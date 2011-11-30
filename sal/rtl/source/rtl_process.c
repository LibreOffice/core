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


#include <string.h>
#include <osl/mutex.h>
#include <rtl/uuid.h>

/* rtl_getCommandArg, rtl_getCommandArgCount see cmdargs.cxx  */

void SAL_CALL rtl_getGlobalProcessId( sal_uInt8 *pTargetUUID )
{
    static sal_uInt8 *pUuid = 0;
    if( ! pUuid )
    {
        osl_acquireMutex( * osl_getGlobalMutex() );
        if( ! pUuid )
        {
            static sal_uInt8 aUuid[16];
            rtl_createUuid( aUuid , 0 , sal_False );
            pUuid = aUuid;
        }
        osl_releaseMutex( * osl_getGlobalMutex() );
    }
    memcpy( pTargetUUID , pUuid , 16 );
}

