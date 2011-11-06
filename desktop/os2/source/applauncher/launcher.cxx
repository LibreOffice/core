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
#if 0
    // YD this feature is not compatible with innowin b20,
    // java cannot load with this flag enabled
    DosSetExtLIBPATH( (PCSZ)"T", LIBPATHSTRICT);
#endif

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
