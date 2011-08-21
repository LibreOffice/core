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
 * the default mail handler.
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
    int     i;
    BOOL        bMailClient = FALSE;

    // check parameters
    if (argc < 5)
    {
    logMessage( "Usage: senddoc --mailclient <client> --attach <uri>");
    dumpArgs( argc, argv);
        return -1;
    }

    // check configuration
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultMailExe", "",
                          szAppFromINI, sizeof(szAppFromINI));
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultMailWorkingDir", "",
                          szDirFromINI, sizeof(szDirFromINI));
    if (*szAppFromINI == 0 || *szDirFromINI == 0)
    {
    logMessage( "Unable to find default mail handler in USER.INI; exiting.");
    dumpArgs( argc, argv);
        return -1;
    }

    // get default parameter list, at leat -compose is required
    rc = PrfQueryProfileString(HINI_USER, "WPURLDEFAULTSETTINGS",
                          "DefaultMailParameters", "",
                          szCmdLine, sizeof(szCmdLine));
    if (strstr( szCmdLine, "-compose") == 0)
    strcat( szCmdLine, " -compose"); // add if missing!

    // parse cmdline arguments
    for( i=1; i<argc; i++)
    {
    if (!strcmp( argv[i], "--mailclient")) {
        // we support only Thunderbird/Mozilla command line options, check exe name
        if (strstr( argv[i+1], "thunderbird") == 0
         && strstr( argv[i+1], "mozilla") == 0
         && strstr( argv[i+1], "seamonkey") == 0)
        {
        logMessage( "Only Thunderbird/Mozilla is currently supported. Exiting.");
        dumpArgs( argc, argv);
        return -1;
        }
        // mail client found
        bMailClient = TRUE;
        i++;
    } else if (!strcmp( argv[i], "--attach")) {
        strcat( szCmdLine, " attachment=file://");
        strcat( szCmdLine, argv[i+1]);
        i++;
    }
    // ignore other options (BTW currently none)
    }
    if (bMailClient == FALSE)
    {
    logMessage( "No mail client specified. Exiting.");
    dumpArgs( argc, argv);
    return -1;
    }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
