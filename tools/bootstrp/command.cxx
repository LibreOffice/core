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
#include "precompiled_tools.hxx"

#ifdef SCO
#define _IOSTREAM_H
#endif

#ifdef PRECOMPILED
#include "first.hxx"
#endif

#include <tools/fsys.hxx>
#include <tools/stream.hxx>
#include "bootstrp/command.hxx"
#include <tools/debug.hxx>
#include "bootstrp/appdef.hxx"

#ifdef _MSC_VER
#pragma warning (push,1)
#endif

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

//#define MH_TEST2  1           // fuers direkte Testen

#if defined(WNT) || defined(OS2)
#ifdef _MSC_VER
#pragma warning (push,1)
#endif
#include <process.h>    // for _SPAWN
#ifdef _MSC_VER
#pragma warning (pop)
#endif
#endif
#ifdef UNX
#include <sys/types.h>
#include <unistd.h>
#if ( defined NETBSD ) || defined (FREEBSD) || defined (AIX) \
    || defined (MACOSX) || defined (OPENBSD) \
    || defined(DRAGONFLY)
#include <sys/wait.h>
#else
#include <wait.h>
#endif
#define P_WAIT 1        // erstmal einen dummz
#endif

#if defined WNT
#include <tools/svwin.h>
#endif

#if defined(WNT) || defined(OS2)
#define     cPathSeperator ';'
#endif
#ifdef UNX
#define     cPathSeperator  ':'
#endif

/*****************************************************************************/
CommandLine::CommandLine(BOOL bWrite)
/*****************************************************************************/
                : bTmpWrite(bWrite)
{
    CommandBuffer = new char [1];
    if (CommandBuffer == NULL) {
        //cout << "Error: nospace" << endl;
        exit(0);
    }
    CommandBuffer[0] = '\0';
    nArgc = 0;
    ppArgv = new char * [1];
    ppArgv[0] = NULL;

    ComShell = new char [128];
    char* pTemp = getenv("COMMAND_SHELL");
    if(!pTemp)
        strcpy(ComShell,COMMAND_SHELL);
    else
        strcpy(ComShell,pTemp);

    strcpy(&ComShell[strlen(ComShell)]," -C ");
}

/*****************************************************************************/
CommandLine::CommandLine(const char *CommandString, BOOL bWrite)
/*****************************************************************************/
                : bTmpWrite(bWrite)
{
    CommandBuffer = new char [1];
    if (CommandBuffer == NULL) {
        exit(0);
    }
    nArgc = 0;
    ppArgv = new char * [1];
    ppArgv[0] = NULL;

    ComShell = new char [128];
    char* pTemp = getenv("COMMAND_SHELL");
    if(!pTemp)
        strcpy(ComShell,COMMAND_SHELL);
    else
        strcpy(ComShell,pTemp);

    strcpy(&ComShell[strlen(ComShell)]," -C ");

    BuildCommand(CommandString);
}

/*****************************************************************************/
CommandLine::CommandLine(const CommandLine& CCommandLine, BOOL bWrite)
/*****************************************************************************/
                : bTmpWrite(bWrite)
{
    CommandBuffer = new char [1];
    if (CommandBuffer == NULL) {
        exit(0);
    }
    nArgc = 0;
    ppArgv = new char * [1];
    ppArgv[0] = NULL;

    ComShell = new char [128];
    char* pTemp = getenv("COMMAND_SHELL");
    if(!pTemp)
        strcpy(ComShell,COMMAND_SHELL);
    else
        strcpy(ComShell,pTemp);

    strcpy(&ComShell[strlen(ComShell)]," -C ");

    BuildCommand(CCommandLine.CommandBuffer);
}

/*****************************************************************************/
CommandLine::~CommandLine()
/*****************************************************************************/
{
    delete [] CommandBuffer;
    delete [] ComShell;
    for (int i = 0; ppArgv[i] != 0; i++) {
        delete [] ppArgv[i];
    }
    delete [] ppArgv;

}

/*****************************************************************************/
CommandLine& CommandLine::operator=(const CommandLine& CCommandLine)
/*****************************************************************************/
{
    strcpy (CommandBuffer, CCommandLine.CommandBuffer);
    for (int i = 0; i != nArgc; i++) {
        delete [] ppArgv[i];
    }
    delete [] ppArgv;
        ppArgv = new char * [1];
        ppArgv[0] = NULL;
    BuildCommand(CommandBuffer);
    return *this;
}

/*****************************************************************************/
CommandLine& CommandLine::operator=(const char *CommandString)
/*****************************************************************************/
{
    strcpy (CommandBuffer, CommandString);
    for (int i = 0; i != nArgc; i++) {
        delete [] ppArgv[i];
    }
    delete [] ppArgv;
        ppArgv = new char * [1];
        ppArgv[0] = NULL;
    BuildCommand(CommandBuffer);

    return *this;
}

/*****************************************************************************/
void CommandLine::Print()
/*****************************************************************************/
{
}

/*****************************************************************************/
void CommandLine::BuildCommand(const char *CommandString)
/*****************************************************************************/
{
    int index = 0, pos=0;
    char buffer[1024];
    char WorkString[1024];

    strcpy(WorkString,CommandString);

    //falls LogWindow -> in tmpfile schreiben
    if(bTmpWrite)
    {
        strcpy(&WorkString[strlen(WorkString)]," >&");
        strcpy(&WorkString[strlen(WorkString)],getenv("TMP"));
        strcpy(&WorkString[strlen(WorkString)],TMPNAME);
    }

    // delete old memory and get some new memory for CommandBuffer
    delete [] CommandBuffer;
    CommandBuffer =  new char [strlen(ComShell)+strlen(WorkString)+1];
    if (CommandBuffer == NULL) {
        exit(0);
    }
    strcpy (CommandBuffer, ComShell);
    strcpy (&CommandBuffer[strlen(ComShell)], WorkString);

    CommandString = CommandBuffer;

    // get the number of tokens
    Strtokens(CommandString);

    // delete the space for the old CommandLine
    for (int i = 0; ppArgv[i] != 0; i++) {
        delete [] ppArgv[i];
    }
    delete [] ppArgv;

    /* get space for the new command line */
    ppArgv = (char **) new char * [nArgc+1];
    if (ppArgv == NULL) {
        //cout << "Error: no space" << endl;
        exit(0);
    }

    // flush the white space
    while ( isspace(*CommandString) )
        CommandString++;

    index = 0;

    // start the loop to build all the individual tokens
    while (*CommandString != '\0') {
        pos = 0;

        // copy the token until white space is found
        while ( !isspace(*CommandString) && *CommandString != '\0') {
            buffer[pos++] = *CommandString++;
        }

        buffer[pos] = '\0';

        // get space for the individual tokens
        ppArgv[index] = (char *) new char [strlen(buffer)+1];
        if (ppArgv[index] == NULL) {
            //cout << "Error: nospace" << endl;
            exit(0);
        }

        // copy the token
        strcpy (ppArgv[index++], buffer);

        // flush while space
        while ( isspace(*CommandString) )
            CommandString++;
    }

    // finish by setting the las pointer to NULL
    ppArgv[nArgc]= NULL;

}

/*****************************************************************************/
void CommandLine::Strtokens(const char *CommandString)
/*****************************************************************************/
{
    int count = 0;
    const char *temp;

    temp = CommandString;

    /* bypass white space */
    while (isspace(*temp)) temp++;

    for (count=0; *temp != '\0'; count++) {
        /* continue until white space of string terminator is found */
        while ((!isspace(*temp)) && (*temp != '\0')) temp++;

        /* bypass white space */
        while (isspace(*temp)) temp++;

    }
    nArgc = count;
}

/*****************************************************************************/
CCommand::CCommand( ByteString &rString )
/*****************************************************************************/
{
    rString.SearchAndReplace( '\t', ' ' );
    aCommand = rString.GetToken( 0, ' ' );
    aCommandLine = Search( "PATH" );
#ifndef UNX
    aCommandLine += " /c ";
#else
    aCommandLine += " -c ";
#endif

    ByteString sCmd( rString.GetToken( 0, ' ' ));
    ByteString sParam( rString.Copy( sCmd.Len()));

    aCommandLine += Search( "PATH", sCmd );
    aCommandLine += sParam;

    ImplInit();
}

/*****************************************************************************/
CCommand::CCommand( const char *pChar )
/*****************************************************************************/
{
    ByteString aString = pChar;
    aString.SearchAndReplace( '\t', ' ' );
    aCommand = aString.GetToken( 0, ' ' );

    aCommandLine = Search( "PATH" );
#ifndef UNX
    aCommandLine += " /c ";
#else
    aCommandLine += " -c ";
#endif
    ByteString rString( pChar );

    ByteString sCmd( rString.GetToken( 0, ' ' ));
    ByteString sParam( rString.Copy( sCmd.Len()));

    aCommandLine += Search( "PATH", sCmd );
    aCommandLine += sParam;

    ImplInit();
}

/*****************************************************************************/
void CCommand::ImplInit()
/*****************************************************************************/
{
    char pTmpStr[255];
    size_t *pPtr;
    char *pChar;
    int nVoid = sizeof( size_t * );
    nArgc = aCommandLine.GetTokenCount(' ');
    ULONG nLen = aCommandLine.Len();

    ppArgv = (char **) new char[ (ULONG)(nLen + nVoid * (nArgc +2) + nArgc ) ];
    pChar = (char *) ppArgv + ( (1+nArgc) * nVoid );
    pPtr = (size_t *) ppArgv;
    for ( xub_StrLen i=0; i<nArgc; i++ )
    {
        (void) strcpy( pTmpStr, aCommandLine.GetToken(i, ' ' ).GetBuffer() );
        size_t nStrLen = strlen( pTmpStr ) + 1;
        strcpy( pChar, pTmpStr );
        *pPtr = (sal_uIntPtr) pChar;
        pChar += nStrLen;
        pPtr += 1;
#ifdef UNX
        if ( i == 1 )
        {
            USHORT nWo = aCommandLine.Search("csh -c ");
            if (nWo != STRING_NOTFOUND)
                aCommandLine.Erase(0, nWo + 7);
            else
                aCommandLine.Erase(0, 16);
            i = nArgc;
            strcpy( pChar, aCommandLine.GetBuffer() );
            *pPtr = (sal_uIntPtr) pChar;
            pPtr += 1;
        }
#endif
    }
    *pPtr = 0;
}

/*****************************************************************************/
CCommand::operator int()
/*****************************************************************************/
{
    int nRet;
#if defined WNT
    nRet = _spawnv( P_WAIT, ppArgv[0], (const char **) ppArgv );
#elif defined OS2
    nRet = _spawnv( P_WAIT, ppArgv[0], ppArgv );
#elif defined UNX
    pid_t pid;

    if (( pid = fork()) < 0 )
        DBG_ASSERT( FALSE, "fork error" );
    else if ( pid == 0 )
        if ( execv( ppArgv[0], (char * const *) ppArgv ) < 0 )
            DBG_ASSERT( FALSE, "execv failed" );
    if ( (nRet = waitpid( pid, NULL, 0 ) < 0) )
        DBG_ASSERT( FALSE, "wait error" );
#endif

    switch ( errno )
    {
        case    E2BIG :
                    nError = COMMAND_TOOBIG;
                    break;
        case    EINVAL :
                    nError = COMMAND_INVALID;
                    break;
        case    ENOENT:
                    nError = COMMAND_NOTFOUND;
                    break;
        case    ENOEXEC :
                    nError = COMMAND_NOEXEC;
                    break;
        case    ENOMEM :
                    nError = COMMAND_NOMEM;
                    break;
        default:
                nError = COMMAND_UNKNOWN;
    }

    if ( nRet )
        fprintf( stderr, "Program returned with errros\n");
    return nRet;
}

/*****************************************************************************/
ByteString CCommand::Search(ByteString aEnv, ByteString sItem)
/*****************************************************************************/
{
    // default wird eine Shell im Path gesucht,
    // wenn aber compsec gestzt ist holen wir uns die
    // Shell von dort
    if ( sItem.Equals( COMMAND_SHELL ))
    {
        ByteString aComspec = GetEnv( "COMSPEC" );
        if ( !aComspec.Equals(""))
            return aComspec;
    }

    DirEntry aItem( String( sItem, RTL_TEXTENCODING_ASCII_US ));
    if ( aItem.Exists())
        return sItem;

    ByteString aEntry, sReturn;
    ByteString sEnv( aEnv );
    ByteString sEnvironment = GetEnv( sEnv.GetBuffer());
    xub_StrLen nCount = sEnvironment.GetTokenCount( cPathSeperator );

    BOOL bFound = FALSE;

    for ( xub_StrLen i=0; i<nCount && !bFound; i++ )
    {
        aEntry = sEnvironment.GetToken(i, cPathSeperator );
#ifndef UNX
        aEntry += '\\';
#else
        aEntry += '/';
#endif
        aEntry += sItem;

        String sEntry( aEntry, RTL_TEXTENCODING_ASCII_US );
        DirEntry aDirEntry( sEntry );
        aDirEntry.ToAbs();
        if ( aDirEntry.Exists()) {
            sReturn = aEntry;
            bFound = TRUE;
        }
    }
    if ( !bFound )
    {
        sEnv = sEnv.ToUpperAscii();
        ByteString sEnvironment2 = GetEnv(sEnv.GetBuffer() );
        xub_StrLen nCount2 = sEnvironment2.GetTokenCount( cPathSeperator );
        for ( xub_StrLen i=0; i<nCount2 && !bFound; i++ )
        {
            aEntry = sEnvironment2.GetToken(i, cPathSeperator );
#ifndef UNX
            aEntry += '\\';
#else
            aEntry += '/';
#endif
            aEntry += sItem;

            String sEntry( aEntry, RTL_TEXTENCODING_ASCII_US );
            DirEntry aDirEntry( sEntry );
            aDirEntry.ToAbs();
            if ( aDirEntry.Exists()) {
                sReturn = aEntry;
                bFound = TRUE;
            }
        }
    }

    if ( sReturn.Equals( "" ))
        sReturn = sItem;

    return sReturn;
}

/*****************************************************************************/
CCommandd::CCommandd( ByteString &rString, CommandBits nBits )
/*****************************************************************************/
                : CCommand( rString ),
                nFlag( nBits )
{
}


/*****************************************************************************/
CCommandd::CCommandd( const char *pChar, CommandBits nBits )
/*****************************************************************************/
                : CCommand( pChar ),
                nFlag( nBits )
{
}

/*****************************************************************************/
CCommandd::operator int()
/*****************************************************************************/
{
    int nRet = 0;

#ifdef WNT
    LPCTSTR lpApplicationName = NULL;
    LPTSTR lpCommandLine = (char *) GetCommandLine_().GetBuffer();
    LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
    LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL;
    BOOL bInheritHandles = TRUE;

    // wie wuenschen wir denn gestartet zu werden ??
    DWORD dwCreationFlags;

    if ( nFlag & COMMAND_EXECUTE_START )
        dwCreationFlags = DETACHED_PROCESS;
    else
        dwCreationFlags = CREATE_NEW_CONSOLE;

    // wir erben vom Vaterprozess
    LPVOID lpEnvironment = NULL;

    // das exe im Pfad suchen
    LPCTSTR lpCurrentDirectory = NULL;

    // in dieser Struktur bekommen wir die erzeugte Processinfo
    // zurueck
    PROCESS_INFORMATION aProcessInformation;

    // weiteres Startupinfo anlegen
    STARTUPINFO aStartupInfo;

    aStartupInfo.cb = sizeof( STARTUPINFO );
    aStartupInfo.lpReserved = NULL;
    aStartupInfo.lpDesktop = NULL;

    // das Fenster bekommt den Namen des Exes
    aStartupInfo.lpTitle = NULL;
    aStartupInfo.dwX = 100;
    aStartupInfo.dwY = 100;
    aStartupInfo.dwXCountChars = 40;
    aStartupInfo.dwYCountChars = 40;

    // Farben setzen
    aStartupInfo.dwFillAttribute = FOREGROUND_RED | BACKGROUND_RED |
                                BACKGROUND_BLUE | BACKGROUND_GREEN;

    aStartupInfo.wShowWindow = SW_SHOWNOACTIVATE;
    aStartupInfo.cbReserved2 = NULL;
    aStartupInfo.lpReserved2 = NULL;

    if ( nFlag & COMMAND_EXECUTE_HIDDEN )
    {
        aStartupInfo.wShowWindow = SW_HIDE;
        aStartupInfo.dwFlags = aStartupInfo.dwFlags | STARTF_USESHOWWINDOW;
    }

    bool bProcess = CreateProcess( lpApplicationName,
                        lpCommandLine, lpProcessAttributes,
                        lpThreadAttributes, bInheritHandles,
                        dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                        &aStartupInfo, &aProcessInformation );

    LPVOID lpMsgBuf;

    if ( bProcess )
    {
        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                GetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL );

        ByteString aErrorString = (char *) lpMsgBuf;

        if ( nFlag & COMMAND_EXECUTE_WAIT )
        {
            DWORD aProcessState = STILL_ACTIVE;
            while(aProcessState == STILL_ACTIVE)
            {
                GetExitCodeProcess(aProcessInformation.hProcess,&aProcessState);
            }
        }
    }
    else
        fprintf( stderr, "Can not start Process !" );

#endif
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
