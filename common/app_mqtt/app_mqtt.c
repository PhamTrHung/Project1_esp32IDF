/* MQTT Mutual Authentication Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "app_mqtt.h"

static const char *TAG = "MQTTS_EXAMPLE";
static esp_mqtt_client_handle_t client;

ESP_EVENT_DEFINE_BASE(MQTT_DEV_EVENT);

mqtt_get_data_callback mqtt_handle_data_cb = NULL;


extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/hello/phamtronghung", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            esp_event_post(MQTT_DEV_EVENT, MQTT_DEV_EVENT_CONNECTED, NULL, 0, portMAX_DELAY);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            esp_event_post(MQTT_DEV_EVENT, MQTT_DEV_EVENT_DISCONNECT, NULL, 0, portMAX_DELAY);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/hello/phamtronghung", NULL, 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            esp_event_post(MQTT_DEV_EVENT, MQTT_DEV_EVENT_SUBSCRIBED, NULL, 0, portMAX_DELAY);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            if(event->data != NULL)
            {
                mqtt_handle_data_cb(event->data, event->data_len);
            }
            // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            // printf("DATA=%.*s\r\n", event->data_len, event->data);
            //esp_event_post(MQTT_DEV_EVENT, MQTT_DEV_EVENT_DATA, event->data, 0, portMAX_DELAY);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://test.mosquitto.org:1883",
        .event_handle = mqtt_event_handler,
        // .client_cert_pem = (const char *)client_cert_pem_start,
        // .client_key_pem = (const char *)client_key_pem_start,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void mqtt_set_data_callback(void* cb)
{
    if(cb)
    {
        mqtt_handle_data_cb = cb;
    }
}

void app_mqtt_publish(char* topic, char* data, int len)
{
    esp_mqtt_client_publish(client, topic, data, len, 1, 0);
}


void app_mqtt_subscribe(char* topic)
{
    esp_mqtt_client_subscribe(client, topic, 1);
}

// void app_mqtt_start(void)
// {
//     ESP_LOGI(TAG, "[APP] Startup..");
//     ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
//     ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

//     esp_log_level_set("*", ESP_LOG_INFO);
//     esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
//     esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
//     esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

//     ESP_ERROR_CHECK(nvs_flash_init());
//     ESP_ERROR_CHECK(esp_netif_init());
//     ESP_ERROR_CHECK(esp_event_loop_create_default());

//     /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
//      * Read "Establishing Wi-Fi or Ethernet Connection" section in
//      * examples/protocols/README.md for more information about this function.
//      */
//     ESP_ERROR_CHECK(example_connect());

//     mqtt_app_start();
// }
