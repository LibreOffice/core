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

#if defined (UNX) || defined (__GNUC__)
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <errno.h>
#include <time.h>
#include <cstdarg>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined ( WNT )
#include <windows.h>
#endif

#include "com/sun/star/task/ErrorCodeIOException.hpp"
#include <tools/debug.hxx>
#include <rtl/string.h>
#include <sal/log.hxx>
#include <sal/macros.h>

#include <vector>

#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#ifdef DBG_UTIL

struct DebugData
{
    DbgData                 aDbgData;
    bool                    bInit;
    DbgPrintLine            pDbgPrintMsgBox;
    DbgPrintLine            pDbgPrintWindow;
    DbgPrintLine            pDbgAbort;
    ::std::vector< DbgPrintLine >
                            aDbgPrintUserChannels;
    DbgTestSolarMutexProc   pDbgTestSolarMutex;

    DebugData()
        :bInit( false )
        ,pDbgPrintMsgBox( NULL )
        ,pDbgPrintWindow( NULL )
        ,pDbgAbort( NULL )
        ,pDbgTestSolarMutex( NULL )
    {
        aDbgData.nTestFlags = DBG_TEST_RESOURCE;
        aDbgData.bOverwrite = true;
#ifdef UNX
        aDbgData.nErrorOut = DBG_OUT_SHELL;
#else
        aDbgData.nErrorOut = DBG_OUT_MSGBOX;
#endif
        aDbgData.aDebugName[0] = 0;
        aDbgData.aInclFilter[0] = 0;
        aDbgData.aExclFilter[0] = 0;
        aDbgData.aInclClassFilter[0] = 0;
        aDbgData.aExclClassFilter[0] = 0;
        aDbgData.aDbgWinState[0] = 0;
    }
};

static DebugData aDebugData;
static sal_Char aCurPath[260];
static bool bDbgImplInMain = false;

#if defined( WNT )
static CRITICAL_SECTION aImplCritDbgSection;
#endif

static bool bImplCritDbgSectionInit = false;

void ImplDbgInitLock()
{
#if defined( WNT )
    InitializeCriticalSection( &aImplCritDbgSection );
#endif
    bImplCritDbgSectionInit = true;
}

void ImplDbgDeInitLock()
{
#if defined( WNT )
    DeleteCriticalSection( &aImplCritDbgSection );
#endif
    bImplCritDbgSectionInit = false;
}

void ImplDbgLock()
{
    if ( !bImplCritDbgSectionInit )
        return;

#if defined( WNT )
    EnterCriticalSection( &aImplCritDbgSection );
#endif
}

void ImplDbgUnlock()
{
    if ( !bImplCritDbgSectionInit )
        return;

#if defined( WNT )
    LeaveCriticalSection( &aImplCritDbgSection );
#endif
}

#define FILE_LINEEND    "\n"

static bool ImplActivateDebugger( const sal_Char* pMsg )
{
#if defined( WNT )
    static sal_Char aImplDbgOutBuf[DBG_BUF_MAXLEN];
    strcpy( aImplDbgOutBuf, pMsg );
    strcat( aImplDbgOutBuf, "\r\n" );
    OutputDebugString( aImplDbgOutBuf );
    DebugBreak();
    return true;
#else
    (void) pMsg; // avoid warning about unused parameter
    return false;
#endif
}

static bool ImplCoreDump()
{
#if defined( WNT )
    DebugBreak();
#else
    long* pTemp = 0;
    *pTemp = 0xCCCC;
#endif
    return true;
}

typedef FILE*       FILETYPE;
#define FileOpen    fopen
#define FileRead    fread
#define FilePrintF  fprintf
#define FileClose   fclose

namespace
{
    enum ConfigSection
    {
        eOutput,
        eMemory,
        eGUI,
        eTest,

        eUnknown
    };

    void lcl_lineFeed( FILETYPE _pFile )
    {
        FilePrintF( _pFile, "%s", FILE_LINEEND );
    }

    const sal_Char* lcl_getSectionName( ConfigSection _eSection )
    {
        const sal_Char* pSectionName = NULL;
        switch ( _eSection )
        {
            case eOutput    : pSectionName = "output";  break;
            case eMemory    : pSectionName = "memory";  break;
            case eGUI       : pSectionName = "gui";     break;
            case eTest      : pSectionName = "test";    break;
            case eUnknown:
                OSL_ASSERT(false);
                break;
        }
        return pSectionName;
    }

    ConfigSection lcl_getSectionFromName( const sal_Char* _pSectionName, size_t _nSectionNameLength )
    {
        if ( strncmp( _pSectionName, "output",  _nSectionNameLength < 6 ? _nSectionNameLength : 6 ) == 0 )
            return eOutput;
        if ( strncmp( _pSectionName, "memory",  _nSectionNameLength < 6 ? _nSectionNameLength : 6 ) == 0 )
            return eMemory;
        if ( strncmp( _pSectionName, "gui",     _nSectionNameLength < 3 ? _nSectionNameLength : 3 ) == 0 )
            return eGUI;
        if ( strncmp( _pSectionName, "test",    _nSectionNameLength < 4 ? _nSectionNameLength : 4 ) == 0 )
            return eTest;
        return eUnknown;
    }

    void lcl_startSection( FILETYPE _pFile, ConfigSection _eSection )
    {
        FilePrintF( _pFile, "[%s]%s", lcl_getSectionName( _eSection ), FILE_LINEEND );
    }

    void lcl_writeConfigString( FILETYPE _pFile, const sal_Char* _pKeyName, const sal_Char* _pValue )
    {
        FilePrintF( _pFile, "%s=%s%s", _pKeyName, _pValue, FILE_LINEEND );
    }

    void lcl_writeConfigBoolean( FILETYPE _pFile, const sal_Char* _pKeyName, bool _bValue )
    {
        lcl_writeConfigString( _pFile, _pKeyName, _bValue ? "1" : "0" );
    }

    void lcl_writeConfigFlag( FILETYPE _pFile, const sal_Char* _pKeyName, sal_uIntPtr _nAllFlags, sal_uIntPtr _nCheckFlag )
    {
        lcl_writeConfigBoolean( _pFile, _pKeyName, ( _nAllFlags & _nCheckFlag ) != 0 );
    }

    void lcl_writeConfigOutChannel( FILETYPE _pFile, const sal_Char* _pKeyName, sal_uIntPtr _nValue )
    {
        const sal_Char* names[ DBG_OUT_COUNT ] =
        {
            "dev/null", "file", "window", "shell", "messagebox", "testtool", "debugger", "abort"
        };
        lcl_writeConfigString( _pFile, _pKeyName, names[ _nValue ] );
    }

    bool lcl_isConfigSection( const sal_Char* _pLine, size_t _nLineLen )
    {
        if ( _nLineLen < 2 )
            // not even enough space for '[' and ']'
            return false;
        if ( ( _pLine[0] == '[' ) && ( _pLine[ _nLineLen - 1 ] == ']' ) )
            return true;
        return false;
    }

    bool lcl_isConfigKey( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName )
    {
        size_t nKeyLength = strlen( _pKeyName );
        if ( nKeyLength + 1 >= _nLineLen )
            // not even long enough for the key name plus "=" plus a one-character value
            return false;
        if ( ( strncmp( _pLine, _pKeyName, nKeyLength ) == 0 ) && ( _pLine[ nKeyLength ] == '=' ) )
            return true;
        return false;
    }

    sal_Int32 lcl_tryReadConfigString( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName, sal_Char* _pValue, size_t _nValueLen )
    {
        if ( !lcl_isConfigKey( _pLine, _nLineLen, _pKeyName ) )
            return 0;
        size_t nValuePos = strlen( _pKeyName ) + 1;
        size_t nValueLen = _nLineLen - nValuePos;
        const sal_Char* pValue = _pLine + nValuePos;
        strncpy( _pValue, pValue, ( _nValueLen > nValueLen ) ? nValueLen : _nValueLen );
        _pValue[ ( _nValueLen > nValueLen ) ? nValueLen : _nValueLen - 1 ] = 0;
        return strlen( _pValue );
    }

    void lcl_tryReadConfigBoolean( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName, bool* _out_pnValue )
    {
        sal_Char aBuf[2];
        size_t nValueLen = lcl_tryReadConfigString( _pLine, _nLineLen, _pKeyName, aBuf, sizeof( aBuf ) );
        if ( nValueLen )
            *_out_pnValue = strcmp( aBuf, "1" ) == 0 ? true : false;
    }

    void lcl_matchOutputChannel( sal_Char const * i_buffer, sal_uIntPtr* o_value )
    {
        if ( i_buffer == NULL )
            return;
        const sal_Char* names[ DBG_OUT_COUNT ] =
        {
            "dev/null", "file", "window", "shell", "messagebox", "testtool", "debugger", "abort"
        };
        for ( size_t name = 0; name < SAL_N_ELEMENTS( names ); ++name )
        {
            if ( strcmp( i_buffer, names[ name ] ) == 0 )
            {
                *o_value = name;
                return;
            }
        }
    }

    void lcl_tryReadOutputChannel( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName, sal_uIntPtr* _out_pnValue )
    {
        sal_Char aBuf[20];
        size_t nValueLen = lcl_tryReadConfigString( _pLine, _nLineLen, _pKeyName, aBuf, sizeof( aBuf ) );
        if ( nValueLen )
            lcl_matchOutputChannel( aBuf, _out_pnValue );
    }

    void lcl_tryReadConfigFlag( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName, sal_uIntPtr* _out_pnAllFlags, sal_uIntPtr _nCheckFlag )
    {
        sal_Char aBuf[2];
        size_t nValueLen = lcl_tryReadConfigString( _pLine, _nLineLen, _pKeyName, aBuf, sizeof( aBuf ) );
        if ( nValueLen )
        {
            if ( strcmp( aBuf, "1" ) == 0 )
                *_out_pnAllFlags |= _nCheckFlag;
            else
                *_out_pnAllFlags &= ~_nCheckFlag;
        }
    }
}

static void DbgGetDbgFileName( sal_Char* pStr, sal_Int32 nMaxLen )
{
#if defined( UNX )
    const sal_Char* pName = getenv("DBGSV_INIT");
    if ( !pName )
        pName = ".dbgsv.init";
    strncpy( pStr, pName, nMaxLen );
#elif defined( WNT )
    const sal_Char* pName = getenv("DBGSV_INIT");
    if ( pName )
        strncpy( pStr, pName, nMaxLen );
    else
        GetProfileStringA( "sv", "dbgsv", "dbgsv.ini", pStr, nMaxLen );
#else
    strncpy( pStr, "dbgsv.ini", nMaxLen );
#endif
    pStr[ nMaxLen - 1 ] = 0;
}

static void DbgGetLogFileName( sal_Char* pStr )
{
#if defined( UNX )
    const sal_Char* pName = getenv("DBGSV_LOG");
    if ( !pName )
        pName = "dbgsv.log";
    strcpy( pStr, pName );
#elif defined( WNT )
    const sal_Char* pName = getenv("DBGSV_LOG");
    if ( pName )
        strcpy( pStr, pName );
    else
        GetProfileStringA( "sv", "dbgsvlog", "dbgsv.log", pStr, 200 );
#else
    strcpy( pStr, "dbgsv.log" );
#endif
}

static DebugData* GetDebugData()
{
    if ( !aDebugData.bInit )
    {
        aDebugData.bInit = true;

        // set default debug names
        DbgGetLogFileName( aDebugData.aDbgData.aDebugName );

        // DEBUG.INI-File
        sal_Char aBuf[ 4096 ];
        DbgGetDbgFileName( aBuf, sizeof( aBuf ) );
        FILETYPE pIniFile = FileOpen( aBuf, "r" );
        if ( pIniFile != NULL )
        {
            ConfigSection eCurrentSection = eUnknown;

            // no sophisticated algorithm here, assume that the whole file fits into aBuf ...
            sal_uIntPtr nReallyRead = FileRead( aBuf, 1, sizeof( aBuf ) / sizeof( sal_Char ) - 1, pIniFile );
            aBuf[ nReallyRead ] = 0;
            const sal_Char* pLine = aBuf;
            while ( const sal_Char* pNextLine = strstr( pLine, FILE_LINEEND ) )
            {
                size_t nLineLength = pNextLine - pLine;

                if ( lcl_isConfigSection( pLine, nLineLength ) )
                    eCurrentSection = lcl_getSectionFromName( pLine + 1, nLineLength - 2 );

                // elements of the [output] section
                if ( eCurrentSection == eOutput )
                {
                    lcl_tryReadConfigString( pLine, nLineLength, "log_file", aDebugData.aDbgData.aDebugName, sizeof( aDebugData.aDbgData.aDebugName ) );
                    lcl_tryReadConfigBoolean( pLine, nLineLength, "overwrite", &aDebugData.aDbgData.bOverwrite );
                    lcl_tryReadConfigString( pLine, nLineLength, "include", aDebugData.aDbgData.aInclFilter, sizeof( aDebugData.aDbgData.aInclFilter ) );
                    lcl_tryReadConfigString( pLine, nLineLength, "exclude", aDebugData.aDbgData.aExclFilter, sizeof( aDebugData.aDbgData.aExclFilter ) );
                    lcl_tryReadConfigString( pLine, nLineLength, "include_class", aDebugData.aDbgData.aInclClassFilter, sizeof( aDebugData.aDbgData.aInclClassFilter ) );
                    lcl_tryReadConfigString( pLine, nLineLength, "exclude_class", aDebugData.aDbgData.aExclClassFilter, sizeof( aDebugData.aDbgData.aExclClassFilter ) );
                    lcl_tryReadOutputChannel( pLine, nLineLength, "error", &aDebugData.aDbgData.nErrorOut );
                }

                // elements of the [gui] section
                if ( eCurrentSection == eGUI )
                {
                    lcl_tryReadConfigString( pLine, nLineLength, "debug_window_state", aDebugData.aDbgData.aDbgWinState, sizeof( aDebugData.aDbgData.aDbgWinState ) );
                }

                // elements of the [test] section
                if ( eCurrentSection == eTest )
                {
                    lcl_tryReadConfigFlag( pLine, nLineLength, "resources", &aDebugData.aDbgData.nTestFlags, DBG_TEST_RESOURCE );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "dialog", &aDebugData.aDbgData.nTestFlags, DBG_TEST_DIALOG );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "bold_app_font", &aDebugData.aDbgData.nTestFlags, DBG_TEST_BOLDAPPFONT );
                }

                pLine = pNextLine + strlen( FILE_LINEEND );
            }

            FileClose( pIniFile );
        }
        else
        {
            lcl_matchOutputChannel( getenv( "DBGSV_ERROR_OUT" ), &aDebugData.aDbgData.nErrorOut );

        }

        sal_Char* getcwdResult = getcwd( aCurPath, sizeof( aCurPath ) );
        if ( !getcwdResult )
        {
            OSL_TRACE( "getcwd failed with error %s", strerror(errno) );
        }
    }

    return &aDebugData;
}

inline DebugData* ImplGetDebugData()
{
    if ( !aDebugData.bInit )
        return GetDebugData();
    else
        return &aDebugData;
}

static FILETYPE ImplDbgInitFile()
{
    static bool bFileInit = false;

    sal_Char aBuf[4096];
    sal_Char* getcwdResult = getcwd( aBuf, sizeof( aBuf ) );
    if ( !getcwdResult ) {
        OSL_TRACE( "getcwd failed with error = %s", strerror(errno) );
        return NULL;
    }

    int chdirResult = chdir( aCurPath );
    if ( !chdirResult ) {
        OSL_TRACE ( "chdir failed with error = %s", strerror(errno) );
        return NULL;
    }

    DebugData*  pData = GetDebugData();
    FILETYPE    pDebugFile;

    if ( !bFileInit )
    {
        bFileInit = true;

        if ( pData->aDbgData.bOverwrite )
            pDebugFile = FileOpen( pData->aDbgData.aDebugName, "w" );
        else
            pDebugFile = FileOpen( pData->aDbgData.aDebugName, "a" );

        if ( pDebugFile )
        {
            time_t  nTime = time( 0 );
            tm*     pTime;
#ifdef UNX
            tm      aTime;
            pTime = localtime_r( &nTime, &aTime );
#else
            pTime = localtime( &nTime );
#endif

            // print header
            FilePrintF( pDebugFile, "******************************************************************************%s", FILE_LINEEND );
            FilePrintF( pDebugFile, "%s%s", pData->aDbgData.aDebugName, FILE_LINEEND );
            if ( pTime )
                FilePrintF( pDebugFile, "%s%s", asctime( pTime ), FILE_LINEEND );
        }
    }
    else
        pDebugFile = FileOpen( pData->aDbgData.aDebugName, "a" );

    chdirResult = chdir( aBuf );
    if ( !chdirResult )
    {
        OSL_TRACE( "chdir failed with error = %s", strerror(errno) );
    }

    return pDebugFile;
}

static void ImplDbgPrintFile( const sal_Char* pLine )
{
    FILETYPE pDebugFile = ImplDbgInitFile();

    if ( pDebugFile )
    {
        FilePrintF( pDebugFile, "%s%s", pLine, FILE_LINEEND );
        FileClose( pDebugFile );
    }
}

static int ImplStrSearch( const sal_Char* pSearchStr, int nSearchLen,
                          const sal_Char* pStr, int nLen )
{
    int nPos = 0;
    while ( nPos+nSearchLen <= nLen )
    {
        if ( strncmp( pStr+nPos, pSearchStr, nSearchLen ) == 0 )
            return 1;
        nPos++;
    }

    return 0;
}

static bool ImplDbgFilter( const sal_Char* pFilter, const sal_Char* pMsg,
                           bool bEmpty )
{
    int nStrLen = strlen( pFilter );
    if ( !nStrLen )
        return bEmpty;

    int nMsgLen = strlen( pMsg );
    const sal_Char* pTok = pFilter;
    int         nTok = 0;
    while ( pTok[nTok] )
    {
        if ( pTok[nTok] == ';' )
        {
            if ( nTok && ImplStrSearch( pTok, nTok, pMsg, nMsgLen ) )
                return true;

            pTok += nTok+1;
            nTok = 0;
        }

        nTok++;
    }

    if ( nTok && ImplStrSearch( pTok, nTok, pMsg, nMsgLen ) )
        return true;
    else
        return false;
}

static void DebugInit()
{
    bDbgImplInMain = true;
    ImplDbgInitLock();
}

static void DebugDeInit()
{
    // Set everything to false, as global variables
    // may cause a system crash otherwise.
    // Maintain memory flags, as otherwise new/delete calls
    // for global variables will crash,
    // as pointer alignment won't work then.
    DebugData*  pData = GetDebugData();
    pData->aDbgData.nTestFlags = 0;
    pData->aDbgPrintUserChannels.clear();
    pData->pDbgPrintWindow      = NULL;
    ImplDbgDeInitLock();
}

bool ImplDbgFilterMessage( const sal_Char* pMsg )
{
    DebugData*  pData = GetDebugData();
    if ( !ImplDbgFilter( pData->aDbgData.aInclFilter, pMsg, true ) )
        return true;
    if ( ImplDbgFilter( pData->aDbgData.aExclFilter, pMsg, false ) )
        return true;
    return false;
}

void* DbgFunc( sal_uInt16 nAction, void* pParam )
{
    DebugData* pDebugData = ImplGetDebugData();

    if ( nAction == DBG_FUNC_GETDATA )
        return (void*)&(pDebugData->aDbgData);
    else
    {
        switch ( nAction )
        {
            case DBG_FUNC_DEBUGSTART:
                DebugInit();
                break;

            case DBG_FUNC_DEBUGEND:
                DebugDeInit();
                break;

            case DBG_FUNC_SETPRINTMSGBOX:
                pDebugData->pDbgPrintMsgBox = (DbgPrintLine)(long)pParam;
                break;

            case DBG_FUNC_SETPRINTWINDOW:
                pDebugData->pDbgPrintWindow = (DbgPrintLine)(long)pParam;
                break;

            case DBG_FUNC_SET_ABORT:
                pDebugData->pDbgAbort = (DbgPrintLine)(long)pParam;
                break;

            case DBG_FUNC_SAVEDATA:
                {
                const DbgData* pData = static_cast< const DbgData* >( pParam );

                sal_Char aBuf[ 4096 ];
                DbgGetDbgFileName( aBuf, sizeof( aBuf ) );
                FILETYPE pIniFile = FileOpen( aBuf, "w" );
                if ( pIniFile == NULL )
                    break;

                lcl_startSection( pIniFile, eOutput );
                lcl_writeConfigString( pIniFile, "log_file", pData->aDebugName );
                lcl_writeConfigBoolean( pIniFile, "overwrite", pData->bOverwrite );
                lcl_writeConfigString( pIniFile, "include", pData->aInclFilter );
                lcl_writeConfigString( pIniFile, "exclude", pData->aExclFilter );
                lcl_writeConfigString( pIniFile, "include_class", pData->aInclClassFilter );
                lcl_writeConfigString( pIniFile, "exclude_class", pData->aExclClassFilter );
                lcl_writeConfigOutChannel( pIniFile, "error", pData->nErrorOut );

                lcl_lineFeed( pIniFile );
                lcl_startSection( pIniFile, eGUI );
                lcl_writeConfigString( pIniFile, "debug_window_state", pData->aDbgWinState );

                lcl_lineFeed( pIniFile );
                lcl_startSection( pIniFile, eTest );
                lcl_writeConfigFlag( pIniFile, "resources", pData->nTestFlags, DBG_TEST_RESOURCE );
                lcl_writeConfigFlag( pIniFile, "dialog", pData->nTestFlags, DBG_TEST_DIALOG );
                lcl_writeConfigFlag( pIniFile, "bold_app_font", pData->nTestFlags, DBG_TEST_BOLDAPPFONT );

                FileClose( pIniFile );
                }
                break;

            case DBG_FUNC_COREDUMP:
                ImplCoreDump();
                break;

            case DBG_FUNC_ALLERROROUT:
                return (void*)(sal_uIntPtr)sal_True;

            case DBG_FUNC_SETTESTSOLARMUTEX:
                pDebugData->pDbgTestSolarMutex = (DbgTestSolarMutexProc)(long)pParam;
                break;

            case DBG_FUNC_TESTSOLARMUTEX:
                SAL_WARN_IF(
                    pDebugData->pDbgTestSolarMutex == 0, "tools.debug",
                    "no DbgTestSolarMutex function set");
                if ( pDebugData->pDbgTestSolarMutex )
                    pDebugData->pDbgTestSolarMutex();
                break;

            case DBG_FUNC_PRINTFILE:
                ImplDbgPrintFile( (const sal_Char*)pParam );
                break;
       }

        return NULL;
    }
}

DbgChannelId DbgRegisterUserChannel( DbgPrintLine pProc )
{
    DebugData* pData = ImplGetDebugData();
    pData->aDbgPrintUserChannels.push_back( pProc );
    return (DbgChannelId)( pData->aDbgPrintUserChannels.size() - 1 + DBG_OUT_USER_CHANNEL_0 );
}

void DbgOut( const sal_Char* pMsg )
{
    static bool bIn = false;
    if ( bIn )
        return;
    bIn = true;

    DebugData*  pData = GetDebugData();
    sal_Char const *   pStr;
    sal_uIntPtr       nOut;
    int         nBufLen = 0;

    nOut = pData->aDbgData.nErrorOut;
    pStr = "Error: ";

    if ( nOut == DBG_OUT_NULL )
    {
        bIn = false;
        return;
    }

    if ( ImplDbgFilterMessage( pMsg ) )
    {
        bIn = false;
        return;
    }

    ImplDbgLock();

    sal_Char aBufOut[DBG_BUF_MAXLEN];
    if ( pStr )
    {
        strcpy( aBufOut, pStr );
        nBufLen = strlen( pStr );
    }
    else
        aBufOut[0] = '\0';

    int nMsgLen = strlen( pMsg );
    if ( nBufLen+nMsgLen > DBG_BUF_MAXLEN )
    {
        int nCopyLen = DBG_BUF_MAXLEN-nBufLen-4;
        strncpy( &(aBufOut[nBufLen]), pMsg, nCopyLen );
        strcpy( &(aBufOut[nBufLen+nCopyLen]), "..." );
    }
    else
        strcpy( &(aBufOut[nBufLen]), pMsg );

    if ( ( nOut >= DBG_OUT_USER_CHANNEL_0 ) && ( nOut - DBG_OUT_USER_CHANNEL_0 < pData->aDbgPrintUserChannels.size() ) )
    {
        DbgPrintLine pPrinter = pData->aDbgPrintUserChannels[ nOut - DBG_OUT_USER_CHANNEL_0 ];
        if ( pPrinter )
            pPrinter( aBufOut );
        else
            nOut = DBG_OUT_DEBUGGER;
    }

    if ( nOut == DBG_OUT_ABORT )
    {
        if ( pData->pDbgAbort != NULL )
            pData->pDbgAbort( aBufOut );
        abort();
    }

    if ( nOut == DBG_OUT_DEBUGGER )
    {
        if ( !ImplActivateDebugger( aBufOut ) )
            nOut = DBG_OUT_TESTTOOL;
    }

    if ( nOut == DBG_OUT_TESTTOOL )
    {
        nOut = DBG_OUT_MSGBOX;
    }

    if ( nOut == DBG_OUT_MSGBOX )
    {
        if ( pData->pDbgPrintMsgBox )
            pData->pDbgPrintMsgBox( aBufOut );
        else
            nOut = DBG_OUT_WINDOW;
    }

    if ( nOut == DBG_OUT_WINDOW )
    {
        if ( pData->pDbgPrintWindow )
            pData->pDbgPrintWindow( aBufOut );
        else
            nOut = DBG_OUT_FILE;
    }

    switch ( nOut )
    {
    case DBG_OUT_SHELL:
        DbgPrintShell( aBufOut );
        break;
    case DBG_OUT_FILE:
        ImplDbgPrintFile( aBufOut );
        break;
    }

    ImplDbgUnlock();

    bIn = false;
}

void DbgPrintShell(char const * message) {
    fprintf(stderr, "%s\n", message);
#if defined WNT
    OutputDebugStringA(message);
#endif
}

void DbgOutTypef( const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    sal_Char aBuf[DBG_BUF_MAXLEN];
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    DbgOut( aBuf );
}

#else

void* DbgFunc( sal_uInt16, void* ) { return NULL; }

void DbgOutTypef( const sal_Char*, ... ) {}

#endif


TOOLS_DLLPUBLIC void DbgUnhandledException(const css::uno::Any & caught, const char* currentFunction, const char* fileAndLineNo)
{
#if OSL_DEBUG_LEVEL == 0
        (void) caught;
        (void) currentFunction;
        (void) fileAndLineNo;
#else
        OString sMessage( "caught an exception!" );
        sMessage += "\nin function:";
        sMessage += currentFunction;
        sMessage += "\ntype: ";
        sMessage += OUStringToOString( caught.getValueTypeName(), osl_getThreadTextEncoding() );
        ::com::sun::star::uno::Exception exception;
        caught >>= exception;
        if ( !exception.Message.isEmpty() )
        {
            sMessage += "\nmessage: ";
            sMessage += OUStringToOString( exception.Message, osl_getThreadTextEncoding() );
        }
        if ( exception.Context.is() )
        {
            const char* pContext = typeid( *exception.Context.get() ).name();
            sMessage += "\ncontext: ";
            sMessage += pContext;
        }
        {
            ::com::sun::star::configuration::CorruptedConfigurationException
                specialized;
            if ( caught >>= specialized )
            {
                sMessage += "\ndetails: ";
                sMessage += OUStringToOString(
                    specialized.Details, osl_getThreadTextEncoding() );
            }
        }
        {
            ::com::sun::star::task::ErrorCodeIOException specialized;
            if ( caught >>= specialized )
            {
                sMessage += "\ndetails: ";
                sMessage += OString::number( specialized.ErrCode );
            }
        }
        sMessage += "\n";

        SAL_DETAIL_LOG_FORMAT(
            SAL_DETAIL_ENABLE_LOG_WARN, SAL_DETAIL_LOG_LEVEL_WARN,
            "legacy.osl", fileAndLineNo, "%s", sMessage.getStr());
#endif
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
