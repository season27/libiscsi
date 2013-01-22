/* 
   Copyright (C) 2013 Ronnie Sahlberg <ronniesahlberg@gmail.com>
   
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

#include <CUnit/CUnit.h>

#include "iscsi.h"
#include "scsi-lowlevel.h"
#include "iscsi-test-cu.h"

void
test_read6_0blocks(void)
{
	int ret;

	logging(LOG_VERBOSE, "");
	logging(LOG_VERBOSE, "Test READ6 0-blocks at LBA==0");
	ret = read6(iscsic, tgt_lun, 0, 0, block_size, NULL);
	CU_ASSERT_EQUAL(ret, 0);

	if (num_blocks > 0x1fffff) {
		CU_PASS("[SKIPPED] LUN is too big");
		return;
	}

	logging(LOG_VERBOSE, "Test READ6 0-blocks one block past end-of-LUN");
	ret = read6_lbaoutofrange(iscsic, tgt_lun, num_blocks + 1, 0,
				  block_size, NULL);
	CU_ASSERT_EQUAL(ret, 0);
}