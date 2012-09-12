/* 
   Copyright (C) 2012 by Ronnie Sahlberg <ronniesahlberg@gmail.com>
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include "iscsi.h"
#include "scsi-lowlevel.h"
#include "iscsi-test.h"

int T0400_inquiry_basic(const char *initiator, const char *url, int data_loss, int show_info)
{ 
	struct iscsi_context *iscsi;
	struct scsi_task *task;
	struct scsi_inquiry_standard *inq;
	int ret, lun;
	int full_size;

	printf("0400_inquiry_basic:\n");
	printf("===================\n");
	if (show_info) {
		printf("Test the standard INQUIRY data format.\n");
		printf("1, Check we can read the standard INQUIRY data\n");
		printf("2, Standard data must be at least 36 bytes in size\n");
		printf("3, Device-type must be either of DISK/TAPE/CDROM\n");
		printf("4, Check that peripheral qualifier field is 0\n");
		printf("5, Check that the version field is valid\n");
		printf("6, Check that response data format is valid\n");
		printf("\n");
		return 0;
	}

	iscsi = iscsi_context_login(initiator, url, &lun);
	if (iscsi == NULL) {
		printf("Failed to login to target\n");
		return -1;
	}


	ret = 0;



	printf("Read standard INQUIRY data ... ");
	/* See how big this inquiry data is */
	task = iscsi_inquiry_sync(iscsi, lun, 0, 0, 255);
	if (task == NULL) {
		printf("[FAILED]\n");
		printf("Failed to send INQUIRY command : %s\n", iscsi_get_error(iscsi));
		ret = -1;
		goto finished;
	}
	if (task->status != SCSI_STATUS_GOOD) {
		printf("[FAILED]\n");
		printf("INQUIRY command failed : %s\n", iscsi_get_error(iscsi));
		scsi_free_scsi_task(task);
		ret = -1;
		goto finished;
	}
	full_size = scsi_datain_getfullsize(task);
	if (full_size > task->datain.size) {
		scsi_free_scsi_task(task);

		/* we need more data for the full list */
		if ((task = iscsi_inquiry_sync(iscsi, lun, 0, 0, full_size)) == NULL) {
			printf("[FAILED]\n");
			printf("Inquiry command failed : %s\n", iscsi_get_error(iscsi));
			ret = -1;
			goto finished;
		}
	}
	inq = scsi_datain_unmarshall(task);
	if (inq == NULL) {
		printf("[FAILED]\n");
		printf("failed to unmarshall inquiry datain blob\n");
		scsi_free_scsi_task(task);
		ret = -1;
		goto finished;
	}
	printf("[OK]\n");

test2:
	printf("Check that standard data is >= 36 bytes in size ... ");
	if (full_size < 36) {
		printf("[FAILED]\n");
		printf("Standard INQUIRY data is less than 36 bytes.\n");
		scsi_free_scsi_task(task);
		ret = -1;
		goto finished;
	}
	printf("[OK]\n");


test3:
	printf("Check device-type is either of DISK, TAPE or CD/DVD  ... ");
	switch (inq->device_type) {
	case SCSI_INQUIRY_PERIPHERAL_DEVICE_TYPE_DIRECT_ACCESS:
	case SCSI_INQUIRY_PERIPHERAL_DEVICE_TYPE_SEQUENTIAL_ACCESS:
	case SCSI_INQUIRY_PERIPHERAL_DEVICE_TYPE_MMC:
		break;
	default:
		printf("[FAILED]\n");
		printf("Device-type is not DISK, TAPE or CD/DVD. Device reported:%s\n", scsi_devtype_to_str(inq->device_type));
		ret = -1;
		goto test4;
	}
	printf("[OK]\n");


test4:
	printf("Check PREIPHERAL QUALIFIER FIELD is 0  ... ");
	if (inq->qualifier != 0) {
		printf("[FAILED]\n");
		printf("QUALIFIER was not 0, it was %d\n", inq->qualifier);
		ret = -1;
		goto test5;
	}
	printf("[OK]\n");

test5:
	printf("Check VERSION field is either 0x4, 0x5 or 0x6 ... ");
	switch (inq->version) {
	case 0x4: /* SPC-2 */
	case 0x5: /* SPC-3 */
	case 0x6: /* SPC-4 */
		break;
	default:
		printf("[FAILED]\n");
		printf("Invalid VERSION:%d. Should be 0x4, 0x5 or 0x6\n", inq->version);
		ret = -1;
		goto test6;
	}
	printf("[OK]\n");

test6:
	printf("Check RESPONSE DATA FORMAT is 2 ... ");
	if (inq->response_data_format != 2) {
		printf("[FAILED]\n");
		printf("Invalid RESPONSE_DATA_FORMAT:%d. Should be 2\n", inq->response_data_format);
		ret = -1;
		goto test7;
	}
	printf("[OK]\n");

test7:

finished:
	iscsi_logout_sync(iscsi);
	iscsi_destroy_context(iscsi);
	return ret;
}