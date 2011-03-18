/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include <sal/main.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>

#include "../../../source/inc/exithelper.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
