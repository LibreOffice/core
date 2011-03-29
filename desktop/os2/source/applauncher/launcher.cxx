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

#include "launcher.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <process.h>

int main( int argc, char* argv[])
{
    PPIB    pib;
    APIRET   rc;
    RESULTCODES result = {0};
    char     szFail[ _MAX_PATH];

    HAB hab = WinInitialize( 0);
    HMQ hmq = WinCreateMsgQueue( hab, 0);
    ERRORID    erridErrorCode = 0;
    erridErrorCode = WinGetLastError(hab);

    // Calculate application name
    CHAR    szLibpath[_MAX_PATH*2];
    CHAR    szApplicationName[_MAX_PATH];
    CHAR    szDrive[_MAX_PATH];
    CHAR    szDir[_MAX_PATH];
    CHAR    szFileName[_MAX_PATH];
    CHAR    szExt[_MAX_PATH];

    // get executable fullpath
    DosGetInfoBlocks(NULL, &pib);
    DosQueryModuleName(pib->pib_hmte, sizeof(szApplicationName), szApplicationName);

    // adjust libpath
    _splitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    char* basedir = strstr( szDir, "\\PROGRAM\\");
    if (basedir) *basedir = 0;
     sprintf( szLibpath, "\"%s%s\\URE\\BIN\";\"%s%s\\BASIS\\PROGRAM\";%BeginLIBPATH%",
          szDrive, szDir, szDrive, szDir);
    DosSetExtLIBPATH( (PCSZ)szLibpath, BEGIN_LIBPATH);
    // make sure we load DLL from our path only, so multiple instances/versions
    // can be loaded.


    // adjust exe name
    _splitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    _makepath( szApplicationName, szDrive, szDir, OFFICE_IMAGE_NAME, (".bin") );

    // copy command line parameters
    int i, len;
    len = strlen(szApplicationName) + 1 + strlen( APPLICATION_SWITCH) + 1 + 1;
    for( i=1; i<argc; i++)
        len += strlen( argv[i]) + 1;

    char* pszCommandLine, *pszArgs;
    pszCommandLine = (char*) calloc( 1, len);
    strcpy( pszCommandLine, szApplicationName);
    pszArgs = pszCommandLine + strlen(szApplicationName) + 1;
    strcat( pszArgs, APPLICATION_SWITCH);
    strcat( pszArgs, " ");
    for( i=1; i<argc; i++) {
        // add quotes if argument has spaces!
        if (strchr( argv[i], ' '))
            strcat( pszArgs, "\"");
        strcat( pszArgs, argv[i]);
        if (strchr( argv[i], ' '))
            strcat( pszArgs, "\"");
        strcat( pszArgs, " ");
    }
    pszArgs[ strlen( pszArgs) + 0] = 0;

    // execute
    rc = DosExecPgm(szFail, sizeof(szFail),
                   EXEC_SYNC, (PCSZ)pszCommandLine, (PCSZ)NULL, &result,
                   (PCSZ)szApplicationName);
    if (rc) {
        char     szMessage[ _MAX_PATH*2];
        sprintf( szMessage, "Execution failed! Contact technical support.\n\nReturn code: %d\nFailing module:%s\n", rc, szFail);
        rc = WinMessageBox( HWND_DESKTOP, HWND_DESKTOP,
                            (PSZ)szMessage,
                            (PSZ)"Unable to start OpenOffice.org!",
                            0, MB_ERROR | MB_OK);
        WinDestroyMsgQueue( hmq);
        WinTerminate( hab);
        exit(1);
    }

    WinDestroyMsgQueue( hmq);
    WinTerminate( hab);

    exit( result.codeResult);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
