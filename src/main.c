#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"

#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"

static const char *TAG = "SERVER";

/* =========================
   WiFi Handler
   ========================= */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        esp_wifi_connect();

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
        ESP_LOGI(TAG, "Connected to WiFi");
}

/* =========================
   WiFi Init
   ========================= */
void wifi_init()
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}

/* =========================
   HTTP Handlers
   ========================= */
esp_err_t root_get_handler(httpd_req_t *req)
{
    const char* response = "<h1>ESP32 Cloud Server Running</h1>";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* =========================
   Start Server
   ========================= */
httpd_handle_t start_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {

        httpd_uri_t root = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = root_get_handler
        };

        httpd_register_uri_handler(server, &root);
    }

    return server;
}

/* =========================
   Main
   ========================= */
void app_main(void)
{
    wifi_init();

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    start_server();

    while (1) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
