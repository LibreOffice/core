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



#ifndef _RTL_RANDOM_H_
#define _RTL_RANDOM_H_ "$Revision: 1.7 $"

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*========================================================================
 *
 * rtlRandom interface.
 *
 *======================================================================*/
/** Random Pool opaque type.
 */
typedef void* rtlRandomPool;


/** Error Code enumeration.
 */
enum __rtl_RandomError
{
    rtl_Random_E_None,
    rtl_Random_E_Argument,
    rtl_Random_E_Memory,
    rtl_Random_E_Unknown,
    rtl_Random_E_FORCE_EQUAL_SIZE = SAL_MAX_ENUM
};

/** Error Code type.
 */
typedef enum __rtl_RandomError rtlRandomError;


/** Create a Random Pool.
    @return initialized Random Pool, or NULL upon failure.
 */
rtlRandomPool SAL_CALL rtl_random_createPool (void) SAL_THROW_EXTERN_C();


/** Destroy a Random Pool.
    @param  Pool [in] a Random Pool.
    @return none. Pool is invalid.
 */
void SAL_CALL rtl_random_destroyPool (
    rtlRandomPool Pool
) SAL_THROW_EXTERN_C();


/** Add bytes to a Random Pool.
    @param Pool    [in] a Random Pool.
    @param pBuffer [in] a buffer containing the bytes to add.
    @param nBufLen [in] the number of bytes to read from the buffer.
    @return rtl_Random_E_None upon success.
 */
rtlRandomError SAL_CALL rtl_random_addBytes (
    rtlRandomPool  Pool,
    const void    *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();


/** Retrieve bytes from a Random Pool.
    @param Pool    [in] a Random Pool.
    @param pBuffer [inout] a buffer to receive the random bytes.
    @param nBufLen [in] the number of bytes to write to the buffer.
    @return rtl_Random_E_None upon success.
 */
rtlRandomError SAL_CALL rtl_random_getBytes (
    rtlRandomPool  Pool,
    void          *Buffer,
    sal_Size       Bytes
) SAL_THROW_EXTERN_C();

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#ifdef __cplusplus
}
#endif

#endif /* _RTL_RANDOM_H_ */

