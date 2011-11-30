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



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <process.h>
#include <time.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_PM
#include <os2.h>

// OOo uses popen() to start us, so we cannot show PM dialogs.
// log message to disk.
void logMessage( char* msg)
{
    PPIB    pib;
    CHAR    szApplicationName[_MAX_PATH];
    CHAR    szDrive[_MAX_PATH];
    CHAR    szDir[_MAX_PATH];
    CHAR    szFileName[_MAX_PATH];
    CHAR    szExt[_MAX_PATH];
    FILE*   log;
    time_t  timeOfDay;
    struct tm* localTime;

    // get executable fullpath
    DosGetInfoBlocks(NULL, &pib);
    DosQueryModuleName(pib->pib_hmte, sizeof(szApplicationName), szApplicationName);
    _splitpath( szApplicationName, szDrive, szDir, szFileName, szExt );
    // log name
    _makepath( szApplicationName, szDrive, szDir, szFileName, (".LOG") );
    log = fopen( szApplicationName, "a");
    if (!log)
    return;
    time( &timeOfDay);
    localTime = localtime( &timeOfDay);
    fprintf( log, "%04d/%02d/%02d %02d:%02d:%02d %s\n",
    localTime->tm_year+1900, localTime->tm_mon+1, localTime->tm_mday,
    localTime->tm_hour, localTime->tm_min, localTime->tm_sec, msg);
    fclose( log);
}

// dump comand line arguments
void dumpArgs( int argc, char *argv[] )
{
    int i;

    logMessage( "Start of command line arguments dump:");
    for( i=0; i<argc; i++)
    logMessage( argv[i]);
}

/*
 * The intended use of this tool is to pass the argument to
 * the default URL exe.
 */
int main(int argc, char *argv[] )
{
    APIRET  rc;
    RESULTCODES result = {0};
    char        szAppFromINI[_MAX_PATH];
    char        szDirFromINI[_MAX_PATH];
    char        szCmdLine[1024];
    char        szFail[ _MAX_PATH];
    ULONG   ulSID;
    PID         pid;

    // check parameters
    if (argc != 2)
    {
    logMessage( "Usage: open-url <url>");
    dumpArgs( argc, argv);
        return -1;
    }

    // check configuration
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultBrowserExe", "",
                          szAppFromINI, sizeof(szAppFromINI));
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultWorkingDir", "",
                          szDirFromINI, sizeof(szDirFromINI));
    if (*szAppFromINI == 0 || *szDirFromINI == 0)
    {
    logMessage( "Unable to find default url handler in USER.INI; exiting.");
    dumpArgs( argc, argv);
        return -1;
    }

    // get default parameter list
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultParameters", "",
                          szCmdLine, sizeof(szCmdLine));
    strcat( szCmdLine, " ");
    strcat( szCmdLine, argv[1]);

    // change default directory
    _chdir( szDirFromINI);

    // start default handler
    STARTDATA   SData;
    CHAR        szObjBuf[CCHMAXPATH];

    SData.Length  = sizeof(STARTDATA);
    SData.Related = SSF_RELATED_INDEPENDENT;
    SData.FgBg    = (1) ? SSF_FGBG_FORE : SSF_FGBG_BACK;
    SData.TraceOpt = SSF_TRACEOPT_NONE;

    SData.PgmTitle = (PSZ)szAppFromINI;

    SData.PgmName = (PSZ)szAppFromINI;
    SData.PgmInputs = (PSZ)szCmdLine;

    SData.TermQ = NULL;
    SData.Environment = 0;
    SData.InheritOpt = SSF_INHERTOPT_PARENT;
    SData.SessionType = SSF_TYPE_PM;
    SData.IconFile = 0;
    SData.PgmHandle = 0;

    SData.PgmControl = SSF_CONTROL_VISIBLE;

    SData.InitXPos  = 30;
    SData.InitYPos  = 40;
    SData.InitXSize = 200;
    SData.InitYSize = 140;
    SData.Reserved = 0;
    SData.ObjectBuffer  = szFail;
    SData.ObjectBuffLen = (ULONG)sizeof(szFail);

    rc = DosStartSession( &SData, &ulSID, &pid);
    // show error dialog in case of problems
    if (rc != NO_ERROR && rc != ERROR_SMG_START_IN_BACKGROUND) {
        char     szMessage[ _MAX_PATH*2];
        sprintf( szMessage, "Execution failed! rc: %d, failing module:%s", rc, szFail);
        logMessage( szMessage);
        dumpArgs( argc, argv);
        return -1;
    }

    // ok
    return 0;
}

