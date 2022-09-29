/***************************************************************************//**
 * @file
 * @brief Empty NCP-host Example Project.
 *
 * Reference implementation of an NCP (Network Co-Processor) host, which is
 * typically run on a central MCU without radio. It can connect to an NCP via
 * VCOM to access the Bluetooth stack of the NCP and to control it using BGAPI.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "app.h"
#include "gatt_db.h"
#include "ncp_host.h"
#include "app_log.h"
#include "app_log_cli.h"
#include "app_assert.h"
#include "sl_bt_api.h"

// Optstring argument for getopt.
#define OPTSTRING      NCP_HOST_OPTSTRING APP_LOG_OPTSTRING "h"

// Usage info.
#define USAGE          APP_LOG_NL "%s " NCP_HOST_USAGE APP_LOG_USAGE " [-h]" APP_LOG_NL

// Options info.
#define OPTIONS    \
  "\nOPTIONS\n"    \
  NCP_HOST_OPTIONS \
  APP_LOG_OPTIONS  \
  "    -h  Print this help message.\n"

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(int argc, char *argv[])
{
  sl_status_t sc;
  int opt;

  // Process command line options.
  while ((opt = getopt(argc, argv, OPTSTRING)) != -1) {
    switch (opt) {
      // Print help.
      case 'h':
        app_log(USAGE, argv[0]);
        app_log(OPTIONS);
        exit(EXIT_SUCCESS);

      // Process options for other modules.
      default:
        sc = ncp_host_set_option((char)opt, optarg);
        if (sc == SL_STATUS_NOT_FOUND) {
          sc = app_log_set_option((char)opt, optarg);
        }
        if (sc != SL_STATUS_OK) {
          app_log(USAGE, argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
    }
  }

  // Initialize NCP connection.
  sc = ncp_host_init();
  if (sc == SL_STATUS_INVALID_PARAMETER) {
    app_log(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }
  app_assert_status(sc);
  app_log_info("NCP host initialised." APP_LOG_NL);
  app_log_info("Resetting NCP target..." APP_LOG_NL);
  // Reset NCP to ensure it gets into a defined state.
  // Once the chip successfully boots, boot event should be received.
  sl_bt_system_reset(sl_bt_system_boot_mode_normal);

  app_log_info("Press Crtl+C to quit" APP_LOG_NL APP_LOG_NL);

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Deinit.
 *****************************************************************************/
void app_deinit(void)
{
  ncp_host_deinit();

  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application deinit code here!                       //
  // This is called once during termination.                                 //
  /////////////////////////////////////////////////////////////////////////////
}


// typedef struct
// {
// 	uint8_t len_flags;
// 	uint8_t type_flags;
// 	uint8_t val_flags;

// 	uint8_t len_manuf;
// 	uint8_t type_manuf;
// 	// First two bytes must contain the manufacturer ID (little-endian order) 
// 	uint8_t company_LO;
// 	uint8_t company_HI;

// 	// The next bytes are freely configurable - using one byte for counter value and one byte for last button press 
// 	uint8_t num_presses;
// 	uint8_t last_press;

// 	// length of the name AD element is variable, adding it last to keep things simple 
// 	uint8_t len_name;
// 	uint8_t type_name;

//   // NAME_MAX_LENGTH must be sized so that total length of data does not exceed 31 bytes
// 	char name[NAME_MAX_LENGTH]; 

// 	// These values are NOT included in the actual advertising payload, just for bookkeeping 
// 	char dummy;        // Space for null terminator
// 	uint8_t data_size; // Actual length of advertising data
// } CustomAdv_t;


uint8_t ADV_header[]= {0x02,0x01,0x06,0x05,0xFF,0xAA,0xAA};

uint8_t find_adv_header(uint8_t *reference, uint8_t *data)
{

  uint8_t match_counter = 0;
  for (int i = 0; i< 7; i++)
  {
    if(*data++ == *reference++)
    {
      match_counter++;
    }
  }
  

  if (match_counter==7)
  {
    printf("Match Found \n\r");
    return 1;
  }

  else return 0;
  
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;
  uint8_t match_counter = 0;


  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Print boot message.
      app_log_info("Bluetooth stack booted: v%d.%d.%d-b%d" APP_LOG_NL,
                   evt->data.evt_system_boot.major,
                   evt->data.evt_system_boot.minor,
                   evt->data.evt_system_boot.patch,
                   evt->data.evt_system_boot.build);

          printf("Scanning started \n\r");
          sl_bt_scanner_start(1,2);

      break;

      case sl_bt_evt_scanner_scan_report_id:

      //Function to find specific Advertisement that Matches "02010605FFAAAA"   
      //  match_counter = 0;
      //  for (int i = 0; i< 7; i++)
      //  {
      //    if(evt->data.evt_scanner_scan_report.data.data[i] == ADV_header[i])
      //    {
      //      printf("%x", evt->data.evt_scanner_scan_report.data.data[i]);
      //      match_counter++;
      //    }
      //  }
       
      
      //  if (match_counter==7)
      //  {
      //    printf("Specific Advertisement found\r\n");
      //    printf (" rssi:%d  \r\n", evt->data.evt_scanner_scan_report.rssi );
      //    printf (" packet_type: %d  \r\n" ,  evt->data.evt_scanner_scan_report.packet_type);
      //    printf (" address: %x6s \r\n" ,  evt->data.evt_scanner_scan_report.address.addr);
      //    printf (" address_type: %d  \r\n" ,  evt->data.evt_scanner_scan_report.address_type);
      //    printf (" bonding: %d  \r\n" ,  evt->data.evt_scanner_scan_report.bonding);
      //    printf (" data:%s \r\n" ,  evt->data.evt_scanner_scan_report.data.data);
      //  }
    if (find_adv_header(&ADV_header, &evt->data.evt_scanner_scan_report.data.data))
       {
         printf("Specific Advertisement found\r\n");
         printf (" rssi:%d  \r\n", evt->data.evt_scanner_scan_report.rssi );
         printf (" packet_type: %d  \r\n" ,  evt->data.evt_scanner_scan_report.packet_type);
         printf (" address: %x6s \r\n" ,  evt->data.evt_scanner_scan_report.address.addr);
         printf (" address_type: %d  \r\n" ,  evt->data.evt_scanner_scan_report.address_type);
         printf (" bonding: %d  \r\n" ,  evt->data.evt_scanner_scan_report.bonding);
         printf (" data:%s \r\n" ,  evt->data.evt_scanner_scan_report.data.data);
       }



     break;

    // case 	sl_bt_evt_scanner_legacy_advertisement_report_id:
    //
    //   printf("Device found \n\r");
    //
    //   printf("BLE device found\r\n");
    //   printf (" rssi:%d  \r\n", evt->data.evt_scanner_legacy_advertisement_report.rssi );
    //   printf (" address: %x6s \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.address.addr);
    //   printf (" address_type: %d  \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.address_type);
    //   printf (" bonding: %d  \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.bonding);
    //   printf (" data:%xs \r\n" ,  evt->data.evt_scanner_legacy_advertisement_report.data.data);
    //
    // break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      app_log_info("Connection opened." APP_LOG_NL);
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      app_log_info("Connection closed." APP_LOG_NL);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert(sc == SL_STATUS_OK,
                 "[E: 0x%04x] Failed to generate advertising data\n",
                 (int)sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      app_log_info("Started advertising." APP_LOG_NL);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}
