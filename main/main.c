#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "connect_wifi.h"

static const char *TAG = "HTTP_CLIENT";
char api_key[] = "replace_with_your_api_key";
char whatsapp_num[] = "replace_with_your_whatsapp_number";
char whatsapp_messgae[] = "WhatsApp Message Test!";

char *url_encode(const char *str)
{
    static const char *hex = "0123456789abcdef";
    static char encoded[1024];
    char *p = encoded;
    while (*str)
    {
        if (isalnum(*str) || *str == '-' || *str == '_' || *str == '.' || *str == '~')
        {
            *p++ = *str;
        }
        else
        {
            *p++ = '%';
            *p++ = hex[*str >> 4];
            *p++ = hex[*str & 15];
        }
        str++;
    }
    *p = '\0';
    return encoded;
}

static void send_whatsapp_message(void *pvParameters)
{
    char callmebot_url[] = "https://api.callmebot.com/whatsapp.php?phone=%s&text=%s&apikey=%s";

    char URL[strlen(callmebot_url)];
    sprintf(URL, callmebot_url, whatsapp_num, url_encode(whatsapp_messgae), api_key);
    ESP_LOGI(TAG, "URL = %s", URL);
    esp_http_client_config_t config = {
        .url = URL,
        .method = HTTP_METHOD_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);

    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    connect_wifi();
    if (wifi_connect_status)
    {
        xTaskCreate(&send_whatsapp_message, "send_whatsapp_message", 8192, NULL, 5, NULL);
    }
}