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



#include "system.h"
#include "sockimpl.h"
#include "secimpl.h"
//#include "daemimpl.h"

#include <osl/diagnose.h>

#ifndef GCC
sal_uInt32 _System _DLL_InitTerm( sal_uInt32 nModule,
                                     sal_uInt32 nFlag )
{
    switch( nFlag )
    {
        case 0:
        {
            /* initialize C runtime library */
            _CRT_init();
            {
                LONG fhToAdd = 0;
                ULONG fhOld = 0;
                ULONG ngLastError = DosSetRelMaxFH(&fhToAdd, &fhOld);
                if (fhOld < 200)
                    ngLastError = DosSetMaxFH(200);
            }

            /* turn off hardware-errors and exception popups */
            DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);

            break;
        }

        case 1:
        {
            /* unload libs (sockets) */
            ImplFreeTCPIP();

            /* unload libs (security) */
            ImplFreeUPM();

            break;
        }
    }

    return (sal_True);
}
#endif


