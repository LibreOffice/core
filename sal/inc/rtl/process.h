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


#ifndef _RTL_PROCESS_H_
#define _RTL_PROCESS_H_

#include <sal/types.h>
#include <osl/process.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
    gets a 16-byte fixed size identifier which is guaranteed not to change
    during the current process.

    The current implementation creates a 16-byte uuid without using
    the ethernet address of system. Thus the
    identifier is different from identifiers created
    in other processes with a very probability.

    @param pTargetUUID 16 byte of memory
    @see rtl_createUiid()
 */
void SAL_CALL rtl_getGlobalProcessId( sal_uInt8 *pTargetUUID );

/** Get the nArg-th command-line argument passed to the main-function of this process.

    This functions differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @param nArg [in] The number of the argument to return.
    @param strCommandArg [out] The string receives the nArg-th command-line argument.
    @return osl_Process_E_None or does not return.
    @see osl_getCommandArg()
    @see rtl_getCommandArgCount()
*/
oslProcessError SAL_CALL rtl_getAppCommandArg(sal_uInt32 nArg, rtl_uString **strCommandArg);

/** Returns the number of command line arguments at process start.

    This functions differs from osl_getCommandArg() in filtering any bootstrap values
    given by command args, that means that all arguments starting with "-env:" will be
    ignored by this function.

    @return the number of commandline arguments passed to the main-function of this process.
    @see osl_getCommandArgCount()
    @see rtl_getCommandArg()
*/
sal_uInt32 SAL_CALL rtl_getAppCommandArgCount();

#ifdef __cplusplus
}
#endif

#endif
