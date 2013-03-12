/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/main.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>

#include "../../../source/inc/exithelper.h"

using namespace desktop;
using ::rtl::OUString;

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
                bRestart = (EXITHELPER_CRASH_WITH_RESTART == exitcode || EXITHELPER_NORMAL_RESTART == exitcode);
            }
            else
                break;

            osl_freeProcessHandle( process );

        }

        bFirstRun = false;

    } while ( bRestart );

    return exitcode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
