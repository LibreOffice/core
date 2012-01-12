/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _RTL_CRC_H_
#define _RTL_CRC_H_ "$Revision: 1.4 $"

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtl_crc32 interface.
 *
 *======================================================================*/
/** Evaluate CRC32 over given data.
    @descr This function evaluates the CRC polynomial 0xEDB88320.

    @param  Crc    [in] CRC32 over previous data or zero.
    @param  Data   [in] data buffer.
    @param  DatLen [in] data buffer length.
    @return new CRC32 value.
 */
sal_uInt32 SAL_CALL rtl_crc32 (
    sal_uInt32  Crc,
    const void *Data, sal_uInt32 DatLen
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_CRC_H_ */

