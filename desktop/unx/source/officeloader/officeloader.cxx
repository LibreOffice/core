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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <sal/main.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>

#include "../../../source/inc/exithelper.hxx"

using namespace rtl;
using namespace desktop;

SAL_IMPLEMENT_MAIN()
{
    oslProcess      process;
    oslProcessError error;

    OUString    sExecutableFile;
    rtl_uString **pCommandArgs;
    sal_uInt32  nCommandArgs;

    osl_getExecutableFile( &sExecutableFile.pData );

    sExecutableFile += OUString( RTL_CONSTASCII_USTRINGPARAM(".bin") );

    nCommandArgs = osl_getCommandArgCount();
    pCommandArgs = new rtl_uString *[nCommandArgs];

    for ( sal_uInt32 i = 0; i < nCommandArgs; i++ )
    {
        pCommandArgs[i] = NULL;
        osl_getCommandArg( i, &pCommandArgs[i] );
    }

    bool bRestart = false;
    bool bFirstRun = true;
    oslProcessExitCode  exitcode = 255;

    do  {
        error = osl_executeProcess(
            sExecutableFile.pData,
            bFirstRun ? pCommandArgs : NULL,
            bFirstRun ? nCommandArgs : 0,
            0,
            NULL,
            NULL,
            NULL,
            0,
            &process
            );

        if ( osl_Process_E_None == error )
        {
            oslProcessInfo  info;

            info.Size = sizeof(info);

            error = osl_joinProcess( process );
            if ( osl_Process_E_None != error )
                break;

            error = osl_getProcessInfo( process, osl_Process_EXITCODE, &info );
            if ( osl_Process_E_None != error )
                break;

            if ( info.Fields & osl_Process_EXITCODE )
            {
                exitcode = info.Code;
                bRestart = (ExitHelper::E_CRASH_WITH_RESTART == exitcode || ExitHelper::E_NORMAL_RESTART == exitcode);
            }
            else
                break;

            osl_freeProcessHandle( process );

        }

        bFirstRun = false;

    } while ( bRestart );

    return exitcode;
}
