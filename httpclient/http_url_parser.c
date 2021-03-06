/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-05-03 20:39:06
 * @LastEditTime: 2020-05-06 09:08:03
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#include <http_log.h>
#include <http_error.h>
#include <http_utils.h>
#include <http_parser.h>
#include <http_url_parser.h>
#include <platform_memory.h>


/* url:<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
 * test:"https://jiejie:test@jiedev.com:8080/test/index.php?who=jiejie#frag"
 */
int http_url_parsing(http_connect_params_t *connect_params, const char *url)  
{
    struct http_parser_url u;

    HTTP_ROBUSTNESS_CHECK((connect_params && url), HTTP_NULL_VALUE_ERROR);

    memset(&u, 0, sizeof(struct http_parser_url));

    if (0 != http_parser_parse_url(url, strlen(url), 0, &u)) { 
        HTTP_LOG_D("error parse url %s", url);
        RETURN_ERROR(HTTP_NULL_VALUE_ERROR);
    }
    
    if (u.field_data[UF_HOST].len) {
        http_utils_assign_string(&connect_params->http_host, url + u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
        HTTP_ROBUSTNESS_CHECK(connect_params->http_host, HTTP_MEM_NOT_ENOUGH_ERROR)
    }

    if (u.field_data[UF_SCHEMA].len) {
        http_utils_assign_string(&connect_params->http_scheme, url + u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
        HTTP_ROBUSTNESS_CHECK(connect_params->http_scheme, HTTP_MEM_NOT_ENOUGH_ERROR);

        if (strcasecmp(connect_params->http_scheme, "https") == 0) {
            connect_params->http_port = DEFAULT_HTTPS_PORT;
        } else if(strcasecmp(connect_params->http_scheme, "http") == 0) {
            connect_params->http_port = DEFAULT_HTTP_PORT;
        }
    }

    if(u.field_set & (1 << UF_PORT)) { 
        connect_params->http_port = u.port;  
    }

    if (0 == connect_params->http_port) {
        RETURN_ERROR(HTTP_NULL_VALUE_ERROR);
    }

    if (u.field_data[UF_USERINFO].len) {
        char *user_info = NULL;
        http_utils_assign_string(&user_info, url + u.field_data[UF_USERINFO].off, u.field_data[UF_USERINFO].len);
        
        if (user_info) {
            char *username = user_info;
            char *password = strchr(user_info, ':');

            if (password) {
                *password = 0;
                password ++;
                http_utils_assign_string(&connect_params->http_password, password, 0);
                HTTP_ROBUSTNESS_CHECK(connect_params->http_password, HTTP_MEM_NOT_ENOUGH_ERROR);
            }
            http_utils_assign_string(&connect_params->http_username, username, 0);
            HTTP_ROBUSTNESS_CHECK(connect_params->http_username, HTTP_MEM_NOT_ENOUGH_ERROR);
            
            http_utils_release_string(user_info);
        } else {
            RETURN_ERROR(HTTP_MEM_NOT_ENOUGH_ERROR);
        }
    }

    if (u.field_data[UF_PATH].len) {
        http_utils_assign_string(&connect_params->http_path, url + u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    } else {
        http_utils_assign_string(&connect_params->http_path, "/", 0);
    }
    
    HTTP_ROBUSTNESS_CHECK(connect_params->http_path, HTTP_MEM_NOT_ENOUGH_ERROR);

    if (u.field_data[UF_QUERY].len) {
        http_utils_assign_string(&connect_params->http_query, url + u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);
        HTTP_ROBUSTNESS_CHECK(connect_params->http_query, HTTP_MEM_NOT_ENOUGH_ERROR);
    } else if (connect_params->http_query) {
        http_utils_release_string(connect_params->http_query);
        connect_params->http_query = NULL;
    }

    if (u.field_data[UF_FRAGMENT].len) {
        http_utils_assign_string(&connect_params->http_farg, url + u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);
        HTTP_ROBUSTNESS_CHECK(connect_params->http_farg, HTTP_MEM_NOT_ENOUGH_ERROR);
    } else if (connect_params->http_farg) {
        http_utils_release_string(connect_params->http_farg);
        connect_params->http_farg = NULL;
    }

    RETURN_ERROR(HTTP_SUCCESS_ERROR);
}





