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

#if defined (UNX) || defined (GCC)
#include <unistd.h>
#else
#include <direct.h>
#endif

#include <errno.h>
#include <time.h>
#include <cstdarg>  // combinations
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined ( WNT )
#include <windows.h>
#endif

#include <tools/debug.hxx>
#include <rtl/string.h>
#include <sal/log.hxx>
#include <sal/macros.h>

#include <vector>

#include <osl/diagnose.h>

#ifdef DBG_UTIL

// --- PointerList ---

#define PBLOCKCOUNT     1024

struct PBlock
{
    void*       aData[PBLOCKCOUNT];
    sal_uInt16      nCount;
    PBlock*     pPrev;
    PBlock*     pNext;
};

class PointerList
{
private:
    PBlock*     pFirst;
    PBlock*     pLast;
    sal_uIntPtr       nCount;

public:
                PointerList() { pFirst = NULL; pLast = NULL; nCount = 0; }
                ~PointerList();

    void        Add( const void* p );
    sal_Bool        Remove( const void* p );

    const void* Get( sal_uIntPtr nPos ) const;
    sal_Bool        IsIn( const void* p ) const;
    sal_uIntPtr       Count() const { return nCount; }
};

// data types

#define DBG_MAXNAME     28

struct ProfType
{
    sal_uIntPtr                   nCount;
    sal_uIntPtr                   nTime;
    sal_uIntPtr                   nMinTime;
    sal_uIntPtr                   nMaxTime;
    sal_uIntPtr                   nStart;
    sal_uIntPtr                   nContinueTime;
    sal_uIntPtr                   nContinueStart;
    sal_Char                aName[DBG_MAXNAME+1];
};

struct XtorType
{
    sal_uIntPtr                   nCtorCalls;
    sal_uIntPtr                   nDtorCalls;
    sal_uIntPtr                   nMaxCount;
    sal_uIntPtr                   nStatics;
    sal_Char                aName[DBG_MAXNAME+1];
    sal_Bool                    bTest;
    PointerList             aThisList;
};

struct DebugData
{
    DbgData                 aDbgData;
    sal_uInt16                  bInit;
    DbgPrintLine            pDbgPrintMsgBox;
    DbgPrintLine            pDbgPrintWindow;
    DbgPrintLine            pDbgPrintTestTool;
    DbgPrintLine            pDbgAbort;
    ::std::vector< DbgPrintLine >
                            aDbgPrintUserChannels;
    PointerList*            pProfList;
    PointerList*            pXtorList;
    DbgTestSolarMutexProc   pDbgTestSolarMutex;
    pfunc_osl_printDetailedDebugMessage
                            pOldDebugMessageFunc;
    bool                    bOslIsHooked;

    DebugData()
        :bInit( sal_False )
        ,pDbgPrintMsgBox( NULL )
        ,pDbgPrintWindow( NULL )
        ,pDbgPrintTestTool( NULL )
        ,pDbgAbort( NULL )
        ,pProfList( NULL )
        ,pXtorList( NULL )
        ,pDbgTestSolarMutex( NULL )
        ,pOldDebugMessageFunc( NULL )
        ,bOslIsHooked( false )
    {
        aDbgData.nTestFlags = DBG_TEST_RESOURCE;
        aDbgData.bOverwrite = sal_True;
        aDbgData.nTraceOut = DBG_OUT_NULL;
        aDbgData.nWarningOut = DBG_OUT_NULL;
#ifdef UNX
        aDbgData.nErrorOut = DBG_OUT_SHELL;
#else
        aDbgData.nErrorOut = DBG_OUT_MSGBOX;
#endif
        aDbgData.bHookOSLAssert = sal_True;
        aDbgData.aDebugName[0] = 0;
        aDbgData.aInclFilter[0] = 0;
        aDbgData.aExclFilter[0] = 0;
        aDbgData.aInclClassFilter[0] = 0;
        aDbgData.aExclClassFilter[0] = 0;
        aDbgData.aDbgWinState[0] = 0;
    }
};

#define DBG_TEST_XTOR_EXTRA (DBG_TEST_XTOR_THIS |  DBG_TEST_XTOR_FUNC |               \
                             DBG_TEST_XTOR_EXIT |  DBG_TEST_XTOR_REPORT )

// static maintenance variables

static DebugData aDebugData;
static sal_Char aCurPath[260];
static int bDbgImplInMain = sal_False;

#if defined( WNT )
static CRITICAL_SECTION aImplCritDbgSection;
#endif

static sal_Bool             bImplCritDbgSectionInit = sal_False;

void ImplDbgInitLock()
{
#if defined( WNT )
    InitializeCriticalSection( &aImplCritDbgSection );
#endif
    bImplCritDbgSectionInit = sal_True;
}

void ImplDbgDeInitLock()
{
#if defined( WNT )
    DeleteCriticalSection( &aImplCritDbgSection );
#endif
    bImplCritDbgSectionInit = sal_False;
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

static sal_Bool ImplActivateDebugger( const sal_Char* pMsg )
{
#if defined( WNT )
    static sal_Char aImplDbgOutBuf[DBG_BUF_MAXLEN];
    strcpy( aImplDbgOutBuf, pMsg );
    strcat( aImplDbgOutBuf, "\r\n" );
    OutputDebugString( aImplDbgOutBuf );
    DebugBreak();
    return sal_True;
#else
    (void) pMsg; // avoid warning about unused parameter
    return sal_False;
#endif
}

static sal_Bool ImplCoreDump()
{
#if defined( WNT )
    DebugBreak();
#else
    long* pTemp = 0;
    *pTemp = 0xCCCC;
#endif
    return sal_True;
}

static sal_uIntPtr ImplGetPerfTime()
{
#if defined( WNT )
    return (sal_uIntPtr)GetTickCount();
#else
    static sal_uIntPtr    nImplTicksPerSecond = 0;
    static double   dImplTicksPerSecond;
    sal_uIntPtr           nTicks = (sal_uIntPtr)clock();

    if ( !nImplTicksPerSecond )
    {
        nImplTicksPerSecond = CLOCKS_PER_SEC;
        dImplTicksPerSecond = nImplTicksPerSecond;
    }

    double fTicks = nTicks;
    fTicks *= 1000;
    fTicks /= dImplTicksPerSecond;
    return (sal_uIntPtr)fTicks;
#endif
}

typedef FILE*       FILETYPE;
#define FileOpen    fopen
#define FileRead    fread
#define FileWrite   fwrite
#define FilePrintF  fprintf
#define FileClose   fclose

namespace
{
    enum ConfigSection
    {
        eOutput,
        eMemory,
        eGUI,
        eObjects,
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
            case eObjects   : pSectionName = "objects"; break;
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
        if ( strncmp( _pSectionName, "objects", _nSectionNameLength < 7 ? _nSectionNameLength : 7 ) == 0 )
            return eObjects;
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

    void lcl_tryReadConfigBoolean( const sal_Char* _pLine, size_t _nLineLen, const sal_Char* _pKeyName, sal_uIntPtr* _out_pnValue )
    {
        sal_Char aBuf[2];
        size_t nValueLen = lcl_tryReadConfigString( _pLine, _nLineLen, _pKeyName, aBuf, sizeof( aBuf ) );
        if ( nValueLen )
            *_out_pnValue = strcmp( aBuf, "1" ) == 0 ? sal_True : sal_False;
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

PointerList::~PointerList()
{
    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        PBlock* pNextBlock = pBlock->pNext;
        delete pBlock;
        pBlock = pNextBlock;
    }
}

void PointerList::Add( const void* p )
{
    if ( !pFirst )
    {
        pFirst = new PBlock;
        memset( pFirst->aData, 0, PBLOCKCOUNT * sizeof( void* ) );
        pFirst->nCount = 0;
        pFirst->pPrev  = NULL;
        pFirst->pNext  = NULL;
        pLast = pFirst;
    }

    PBlock* pBlock = pFirst;
    while ( pBlock && (pBlock->nCount == PBLOCKCOUNT) )
        pBlock = pBlock->pNext;

    if ( !pBlock )
    {
        pBlock = new PBlock;
        memset( pBlock->aData, 0, PBLOCKCOUNT * sizeof( void* ) );
        pBlock->nCount = 0;
        pBlock->pPrev  = pLast;
        pBlock->pNext  = NULL;
        pLast->pNext   = pBlock;
        pLast          = pBlock;
    }

    sal_uInt16 i = 0;
    while ( pBlock->aData[i] )
        i++;

    pBlock->aData[i] = (void*)p;
    pBlock->nCount++;
    nCount++;
}

sal_Bool PointerList::Remove( const void* p )
{
    if ( !p )
       return sal_False;

    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        sal_uInt16 i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( ((sal_uIntPtr)p) == ((sal_uIntPtr)pBlock->aData[i]) )
            {
                pBlock->aData[i] = NULL;
                pBlock->nCount--;
                nCount--;

                if ( !pBlock->nCount )
                {
                    if ( pBlock->pPrev )
                        pBlock->pPrev->pNext = pBlock->pNext;
                    if ( pBlock->pNext )
                        pBlock->pNext->pPrev = pBlock->pPrev;
                    if ( pBlock == pFirst )
                        pFirst = pBlock->pNext;
                    if ( pBlock == pLast )
                        pLast = pBlock->pPrev;
                    delete pBlock;
                }

                return sal_True;
            }
            i++;
        }

        pBlock = pBlock->pNext;
    }

    return sal_False;
}

const void* PointerList::Get( sal_uIntPtr nPos ) const
{
    if ( nCount <= nPos )
        return NULL;

    PBlock* pBlock = pFirst;
    sal_uIntPtr   nStart = 0;
    while ( pBlock )
    {
        sal_uInt16 i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( pBlock->aData[i] )
            {
                nStart++;
                if ( (nStart-1) == nPos )
                    return pBlock->aData[i];
            }

            i++;
        }

        pBlock = pBlock->pNext;
    }

    return NULL;
}

sal_Bool PointerList::IsIn( const void* p ) const
{
    if ( !p )
       return sal_False;

    PBlock* pBlock = pFirst;
    while ( pBlock )
    {
        sal_uInt16 i = 0;
        while ( i < PBLOCKCOUNT )
        {
            if ( ((sal_uIntPtr)p) == ((sal_uIntPtr)pBlock->aData[i]) )
                return sal_True;
            i++;
        }

        pBlock = pBlock->pNext;
    }

    return sal_False;
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
        aDebugData.bInit = sal_True;

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
                    lcl_tryReadOutputChannel( pLine, nLineLength, "trace", &aDebugData.aDbgData.nTraceOut );
                    lcl_tryReadOutputChannel( pLine, nLineLength, "warning", &aDebugData.aDbgData.nWarningOut );
                    lcl_tryReadOutputChannel( pLine, nLineLength, "error", &aDebugData.aDbgData.nErrorOut );
                    lcl_tryReadConfigBoolean( pLine, nLineLength, "oslhook", &aDebugData.aDbgData.bHookOSLAssert );
                }

                // elements of the [gui] section
                if ( eCurrentSection == eGUI )
                {
                    lcl_tryReadConfigString( pLine, nLineLength, "debug_window_state", aDebugData.aDbgData.aDbgWinState, sizeof( aDebugData.aDbgData.aDbgWinState ) );
                }

                // elements of the [objects] section
                if ( eCurrentSection == eObjects )
                {
                    lcl_tryReadConfigFlag( pLine, nLineLength, "check_this", &aDebugData.aDbgData.nTestFlags, DBG_TEST_XTOR_THIS );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "check_function", &aDebugData.aDbgData.nTestFlags, DBG_TEST_XTOR_FUNC );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "check_exit", &aDebugData.aDbgData.nTestFlags, DBG_TEST_XTOR_EXIT );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "generate_report", &aDebugData.aDbgData.nTestFlags, DBG_TEST_XTOR_REPORT );
                    lcl_tryReadConfigFlag( pLine, nLineLength, "trace", &aDebugData.aDbgData.nTestFlags, DBG_TEST_XTOR_TRACE );
                }

                // elements of the [test] section
                if ( eCurrentSection == eTest )
                {
                    lcl_tryReadConfigFlag( pLine, nLineLength, "profiling", &aDebugData.aDbgData.nTestFlags, DBG_TEST_PROFILING );
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
            lcl_matchOutputChannel( getenv( "DBGSV_TRACE_OUT" ), &aDebugData.aDbgData.nTraceOut );
            lcl_matchOutputChannel( getenv( "DBGSV_WARNING_OUT" ), &aDebugData.aDbgData.nWarningOut );
            lcl_matchOutputChannel( getenv( "DBGSV_ERROR_OUT" ), &aDebugData.aDbgData.nErrorOut );

        }

        sal_Char* getcwdResult = getcwd( aCurPath, sizeof( aCurPath ) );
        if ( !getcwdResult )
        {
            OSL_TRACE( "getcwd failed with error %s", strerror(errno) );
        }

        // initialize debug data
        if ( aDebugData.aDbgData.nTestFlags & DBG_TEST_XTOR )
            aDebugData.pXtorList = new PointerList;
        if ( aDebugData.aDbgData.nTestFlags & DBG_TEST_PROFILING )
            aDebugData.pProfList = new PointerList;
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
    static sal_Bool bFileInit = sal_False;

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
        bFileInit = sal_True;

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

static int ImplDbgFilter( const sal_Char* pFilter, const sal_Char* pMsg,
                          int bEmpty )
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
                return sal_True;

            pTok += nTok+1;
            nTok = 0;
        }

        nTok++;
    }

    if ( nTok && ImplStrSearch( pTok, nTok, pMsg, nMsgLen ) )
        return sal_True;
    else
        return sal_False;
}

extern "C"
void SAL_CALL dbg_printOslDebugMessage( const sal_Char * pszFileName, sal_Int32 nLine, const sal_Char * pszMessage )
{
    DbgOut( pszMessage ? pszMessage : "assertion failed!", DBG_OUT_ERROR, pszFileName, (sal_uInt16)nLine );
}

static void DebugInit()
{
    bDbgImplInMain = sal_True;
    ImplDbgInitLock();

    DebugData* pData = GetDebugData();
    if( pData->aDbgData.bHookOSLAssert && ! pData->bOslIsHooked )
    {
        pData->pOldDebugMessageFunc = osl_setDetailedDebugMessageFunc( &dbg_printOslDebugMessage );
        pData->bOslIsHooked = true;
    }
}

static void DebugDeInit()
{
    DebugData*  pData = GetDebugData();
    sal_uIntPtr       i;
    sal_uIntPtr       nCount;
    sal_uIntPtr       nOldOut;

    if( pData->bOslIsHooked )
    {
        osl_setDetailedDebugMessageFunc( pData->pOldDebugMessageFunc );
        pData->bOslIsHooked = sal_False;
    }

    // Output statistics trace data to file
    nOldOut = pData->aDbgData.nTraceOut;
    pData->aDbgData.nTraceOut = DBG_OUT_FILE;

    // output Xtor list
    if ( pData->pXtorList && pData->pXtorList->Count() &&
         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_REPORT) )
    {
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "Object Report" );
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "%-27s : %-9s : %-9s : %-7s : %-3s : %-6s :",
                 "XTor-List", "Ctor", "Dtor", "MaxInst", "St.", "Diff." );
        DbgOutf( "----------------------------:-----------:-----------:---------:----:---------:" );
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            if ( pXtorData->bTest )
            {
                // Add static objects
                pXtorData->nDtorCalls += pXtorData->nStatics;
                if ( pXtorData->nStatics && (pXtorData->nDtorCalls > pXtorData->nCtorCalls) )
                    pXtorData->nDtorCalls = pXtorData->nCtorCalls;
                DbgOutf( "%-27s : %9lu : %9lu : %7lu : %3lu : %4lu %-1s :",
                         pXtorData->aName, pXtorData->nCtorCalls, pXtorData->nDtorCalls,
                         pXtorData->nMaxCount, pXtorData->nStatics,
                         pXtorData->nCtorCalls - pXtorData->nDtorCalls,
                         (pXtorData->nCtorCalls - pXtorData->nDtorCalls) ? "!" : " " );
            }
        }
        DbgOutf( "==============================================================================" );
    }

    // free XtorList
    if ( pData->pXtorList )
    {
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            delete pXtorData;
        }
        delete pData->pXtorList;
        pData->pXtorList = NULL;
    }

    // Set everything to sal_False, as global variables
    // may cause a system crash otherwise.
    // Maintain memory flags, as otherwise new/delete calls
    // for global variables will crash,
    // as pointer alignment won't work then.
    pData->aDbgData.nTraceOut   = nOldOut;
    pData->aDbgData.nTestFlags &= DBG_TEST_PROFILING;
    pData->aDbgPrintUserChannels.clear();
    pData->pDbgPrintTestTool    = NULL;
    pData->pDbgPrintWindow      = NULL;
    pData->pOldDebugMessageFunc = NULL;
    ImplDbgDeInitLock();
}

static void DebugGlobalDeInit()
{
    DebugData*  pData = GetDebugData();
    sal_uIntPtr       i;
    sal_uIntPtr       nCount;
    sal_uIntPtr       nOldOut;

    // Output statistics trace data to file
    nOldOut = pData->aDbgData.nTraceOut;
    pData->aDbgData.nTraceOut = DBG_OUT_FILE;

    // output profile liste
    if ( pData->pProfList && pData->pProfList->Count() )
    {
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "Profiling Report" );
        DbgOutf( "------------------------------------------------------------------------------" );
        DbgOutf( "%-25s : %-9s : %-6s : %-6s : %-6s : %-9s :",
                 "Prof-List (ms)", "Time", "Min", "Max", "Ave", "Count" );
        DbgOutf( "--------------------------:-----------:--------:--------:--------:-----------:" );
        for( i = 0, nCount = pData->pProfList->Count(); i < nCount; i++ )
        {
            ProfType* pProfData = (ProfType*)pData->pProfList->Get( i );
            sal_uIntPtr nAve = pProfData->nTime / pProfData->nCount;
            DbgOutf( "%-25s : %9lu : %6lu : %6lu : %6lu : %9lu :",
                     pProfData->aName, pProfData->nTime,
                     pProfData->nMinTime, pProfData->nMaxTime, nAve,
                     pProfData->nCount );
        }
        DbgOutf( "==============================================================================" );
    }

    // free profile list
    if ( pData->pProfList )
    {
        for( i = 0, nCount = pData->pProfList->Count(); i < nCount; i++ )
        {
            ProfType* pProfData = (ProfType*)pData->pProfList->Get( i );
            delete pProfData;
        }
        delete pData->pProfList;
        pData->pProfList = NULL;
    }

    // disable profiling flags
    pData->aDbgData.nTraceOut   = nOldOut;
    pData->aDbgData.nTestFlags &= ~DBG_TEST_PROFILING;
}

void ImpDbgOutfBuf( sal_Char* pBuf, const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    sal_Char aBuf[DBG_BUF_MAXLEN];
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    strcat( pBuf, aBuf );
    strcat( pBuf, "\n" );
}

static void DebugXTorInfo( sal_Char* pBuf )
{
    DebugData*  pData = GetDebugData();
    sal_uIntPtr       i;
    sal_uIntPtr       nCount;

    // output Xtor list
    if ( pData->pXtorList && pData->pXtorList->Count() &&
         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_REPORT) )
    {
        ImpDbgOutfBuf( pBuf, "------------------------------------------------------------------------------" );
        ImpDbgOutfBuf( pBuf, "Object Report" );
        ImpDbgOutfBuf( pBuf, "------------------------------------------------------------------------------" );
        ImpDbgOutfBuf( pBuf, "%-27s : %-9s : %-9s : %-7s : %-3s : %-6s :",
                       "XTor-List", "Ctor", "Dtor", "MaxInst", "St.", "Diff." );
        ImpDbgOutfBuf( pBuf, "----------------------------:-----------:-----------:---------:----:---------:" );
        for( i = 0, nCount = pData->pXtorList->Count(); i < nCount; i++ )
        {
            XtorType* pXtorData = (XtorType*)pData->pXtorList->Get( i );
            if ( pXtorData->bTest )
            {
                ImpDbgOutfBuf( pBuf, "%-27s : %9lu : %9lu : %7lu : %3lu : %6lu :",
                               pXtorData->aName, pXtorData->nCtorCalls, pXtorData->nDtorCalls,
                               pXtorData->nMaxCount, pXtorData->nStatics,
                               pXtorData->nCtorCalls - pXtorData->nDtorCalls );
            }
        }
        ImpDbgOutfBuf( pBuf, "==============================================================================" );
        ImpDbgOutfBuf( pBuf, "" );
    }
}

sal_Bool ImplDbgFilterMessage( const sal_Char* pMsg )
{
    DebugData*  pData = GetDebugData();
    if ( !ImplDbgFilter( pData->aDbgData.aInclFilter, pMsg, sal_True ) )
        return sal_True;
    if ( ImplDbgFilter( pData->aDbgData.aExclFilter, pMsg, sal_False ) )
        return sal_True;
    return sal_False;
}

void* DbgFunc( sal_uInt16 nAction, void* pParam )
{
    DebugData* pDebugData = ImplGetDebugData();

    if ( nAction == DBG_FUNC_GETDATA )
        return (void*)&(pDebugData->aDbgData);
    else if ( nAction == DBG_FUNC_GETPRINTMSGBOX )
        return (void*)(long)(pDebugData->pDbgPrintMsgBox);
    else if ( nAction == DBG_FUNC_FILTERMESSAGE )
        if ( ImplDbgFilterMessage( (const sal_Char*) pParam ) )
            return (void*) -1;
        else
            return (void*) 0;   // aka NULL
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

            case DBG_FUNC_GLOBALDEBUGEND:
                DebugGlobalDeInit();
                break;

            case DBG_FUNC_SETPRINTMSGBOX:
                pDebugData->pDbgPrintMsgBox = (DbgPrintLine)(long)pParam;
                break;

            case DBG_FUNC_SETPRINTWINDOW:
                pDebugData->pDbgPrintWindow = (DbgPrintLine)(long)pParam;
                break;

            case DBG_FUNC_SETPRINTTESTTOOL:
                pDebugData->pDbgPrintTestTool = (DbgPrintLine)(long)pParam;
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
                lcl_writeConfigOutChannel( pIniFile, "trace", pData->nTraceOut );
                lcl_writeConfigOutChannel( pIniFile, "warning", pData->nWarningOut );
                lcl_writeConfigOutChannel( pIniFile, "error", pData->nErrorOut );
                lcl_writeConfigBoolean( pIniFile, "oslhook", pData->bHookOSLAssert );

                lcl_lineFeed( pIniFile );
                lcl_startSection( pIniFile, eGUI );
                lcl_writeConfigString( pIniFile, "debug_window_state", pData->aDbgWinState );

                lcl_lineFeed( pIniFile );
                lcl_startSection( pIniFile, eObjects );
                lcl_writeConfigFlag( pIniFile, "check_this", pData->nTestFlags, DBG_TEST_XTOR_THIS );
                lcl_writeConfigFlag( pIniFile, "check_function", pData->nTestFlags, DBG_TEST_XTOR_FUNC );
                lcl_writeConfigFlag( pIniFile, "check_exit", pData->nTestFlags, DBG_TEST_XTOR_EXIT );
                lcl_writeConfigFlag( pIniFile, "generate_report", pData->nTestFlags, DBG_TEST_XTOR_REPORT );
                lcl_writeConfigFlag( pIniFile, "trace", pData->nTestFlags, DBG_TEST_XTOR_TRACE );

                lcl_lineFeed( pIniFile );
                lcl_startSection( pIniFile, eTest );
                lcl_writeConfigFlag( pIniFile, "profiling", pData->nTestFlags, DBG_TEST_PROFILING );
                lcl_writeConfigFlag( pIniFile, "resources", pData->nTestFlags, DBG_TEST_RESOURCE );
                lcl_writeConfigFlag( pIniFile, "dialog", pData->nTestFlags, DBG_TEST_DIALOG );
                lcl_writeConfigFlag( pIniFile, "bold_app_font", pData->nTestFlags, DBG_TEST_BOLDAPPFONT );

                FileClose( pIniFile );
                }
                break;

            case DBG_FUNC_XTORINFO:
                DebugXTorInfo( (sal_Char*)pParam );
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
                if ( pDebugData->pDbgTestSolarMutex )
                    pDebugData->pDbgTestSolarMutex();
                break;

            case DBG_FUNC_PRINTFILE:
                ImplDbgPrintFile( (const sal_Char*)pParam );
                break;
            case DBG_FUNC_UPDATEOSLHOOK:
            {
                const DbgData* pData = static_cast< const DbgData* >( pParam );
                pDebugData->aDbgData.bHookOSLAssert = pData->bHookOSLAssert;
                if( pDebugData->bOslIsHooked && ! pData->bHookOSLAssert )
                {
                    osl_setDetailedDebugMessageFunc( pDebugData->pOldDebugMessageFunc );
                    pDebugData->bOslIsHooked = sal_False;
                }
                else if( ! pDebugData->bOslIsHooked && pData->bHookOSLAssert )
                {
                    pDebugData->pOldDebugMessageFunc = osl_setDetailedDebugMessageFunc( &dbg_printOslDebugMessage );
                    pDebugData->bOslIsHooked = sal_True;
                }
            }
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

void DbgProf( sal_uInt16 nAction, DbgDataType* pDbgData )
{
    DebugData* pData = ImplGetDebugData();

    if ( !(pData->aDbgData.nTestFlags & DBG_TEST_PROFILING) )
        return;

    ProfType*   pProfData = (ProfType*)pDbgData->pData;
    sal_uIntPtr       nTime;
    if ( (nAction != DBG_PROF_START) && !pProfData )
    {
        SAL_WARN(
            "tools.debug",
            "DBG_PROF...() without DBG_PROFSTART(): " << pDbgData->pName);
        return;
    }

    switch ( nAction )
    {
        case DBG_PROF_START:
            if ( !pDbgData->pData )
            {
                pDbgData->pData = (void*)new ProfType;
                pProfData = (ProfType*)pDbgData->pData;
                strncpy( pProfData->aName, pDbgData->pName, DBG_MAXNAME );
                pProfData->aName[DBG_MAXNAME] = '\0';
                pProfData->nCount           = 0;
                pProfData->nTime            = 0;
                pProfData->nMinTime         = 0xFFFFFFFF;
                pProfData->nMaxTime         = 0;
                pProfData->nStart           = 0xFFFFFFFF;
                pProfData->nContinueTime    = 0;
                pProfData->nContinueStart   = 0xFFFFFFFF;
                pData->pProfList->Add( (void*)pProfData );
            }

            if ( pProfData->nStart == 0xFFFFFFFF )
            {
                pProfData->nStart = ImplGetPerfTime();
                pProfData->nCount++;
            }
            break;

        case DBG_PROF_STOP:
            nTime = ImplGetPerfTime();

            if ( pProfData->nStart == 0xFFFFFFFF )
            {
                SAL_WARN(
                    "tools.debug", "DBG_PROF...() without DBG_PROFSTART()");
                return;
            }

            if ( pProfData->nContinueStart != 0xFFFFFFFF )
            {
                pProfData->nContinueTime += ImplGetPerfTime() - pProfData->nContinueStart;
                pProfData->nContinueStart = 0xFFFFFFFF;
            }

            nTime -= pProfData->nStart;
            nTime -= pProfData->nContinueTime;

            if ( nTime < pProfData->nMinTime )
                pProfData->nMinTime = nTime;

            if ( nTime > pProfData->nMaxTime )
                pProfData->nMaxTime = nTime;

            pProfData->nTime += nTime;

            pProfData->nStart         = 0xFFFFFFFF;
            pProfData->nContinueTime  = 0;
            pProfData->nContinueStart = 0xFFFFFFFF;
            break;

        case DBG_PROF_CONTINUE:
            if ( pProfData->nContinueStart != 0xFFFFFFFF )
            {
                pProfData->nContinueTime += ImplGetPerfTime() - pProfData->nContinueStart;
                pProfData->nContinueStart = 0xFFFFFFFF;
            }
            break;

        case DBG_PROF_PAUSE:
            if ( pProfData->nContinueStart == 0xFFFFFFFF )
                pProfData->nContinueStart = ImplGetPerfTime();
            break;
    }
}

void DbgXtor( DbgDataType* pDbgData, sal_uInt16 nAction, const void* pThis,
              DbgUsr fDbgUsr )
{
    DebugData* pData = ImplGetDebugData();

    // quick test
    if ( !(pData->aDbgData.nTestFlags & DBG_TEST_XTOR) )
        return;

    XtorType* pXtorData = (XtorType*)pDbgData->pData;
    if ( !pXtorData )
    {
        pDbgData->pData = (void*)new XtorType;
        pXtorData = (XtorType*)pDbgData->pData;
        strncpy( pXtorData->aName, pDbgData->pName, DBG_MAXNAME );
        pXtorData->aName[DBG_MAXNAME] = '\0';
        pXtorData->nCtorCalls   = 0;
        pXtorData->nDtorCalls   = 0;
        pXtorData->nMaxCount    = 0;
        pXtorData->nStatics     = 0;
        pXtorData->bTest        = sal_True;
        pData->pXtorList->Add( (void*)pXtorData );

        if ( !ImplDbgFilter( pData->aDbgData.aInclClassFilter, pXtorData->aName, sal_True ) )
            pXtorData->bTest = sal_False;
        if ( ImplDbgFilter( pData->aDbgData.aExclClassFilter, pXtorData->aName, sal_False ) )
            pXtorData->bTest = sal_False;
    }
    if ( !pXtorData->bTest )
        return;

    sal_uInt16      nAct = nAction & ~DBG_XTOR_DTOROBJ;

    SAL_INFO_IF(
        ((pData->aDbgData.nTestFlags & DBG_TEST_XTOR_TRACE)
         && !(nAction & DBG_XTOR_DTOROBJ) && nAct != DBG_XTOR_CHKOBJ),
        "tools.debug",
        (nAct == DBG_XTOR_CTOR ? "Enter Ctor from class "
         : nAct == DBG_XTOR_DTOR ? "Enter Dtor from class "
         : "Enter method from class ") << pDbgData->pName);

    // If some Xtor-tests are still tracing
    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXTRA )
    {
        // call DBG_CTOR before all other DBG_XTOR calls
        if ( ((nAction & ~DBG_XTOR_DTOROBJ) != DBG_XTOR_CTOR) && !pDbgData->pData )
        {
            SAL_WARN(
                "tools.debug",
                "DBG_DTOR() or DBG_CHKTHIS() without DBG_CTOR(): "
                    << pDbgData->pName);
            return;
        }

        // Test if the pointer is still valid
        if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
        {
            if ( (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) ||
                 !(nAction & DBG_XTOR_DTOROBJ) )
            {
                // This-Pointer == NULL
                if ( !pThis )
                {
                    SAL_WARN(
                        "tools.debug",
                        "this == NULL in class " << pDbgData->pName);
                    return;
                }

                if ( (nAction & ~DBG_XTOR_DTOROBJ) != DBG_XTOR_CTOR )
                {
                    SAL_WARN_IF(
                        !pXtorData->aThisList.IsIn(pThis), "tools.debug",
                        "invalid this-Pointer %p in class " << pDbgData->pName);
                }
            }
        }

        // execute function test and update maintenance data
        const sal_Char* pMsg = NULL;
        switch ( nAction & ~DBG_XTOR_DTOROBJ )
        {
            case DBG_XTOR_CTOR:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                else
                {
                    pXtorData->nCtorCalls++;
                    if ( !bDbgImplInMain )
                        pXtorData->nStatics++;
                    if ( (pXtorData->nCtorCalls-pXtorData->nDtorCalls) > pXtorData->nMaxCount )
                        pXtorData->nMaxCount = pXtorData->nCtorCalls - pXtorData->nDtorCalls;

                    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
                        pXtorData->aThisList.Add( pThis );
                }
                break;

            case DBG_XTOR_DTOR:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    pXtorData->nDtorCalls++;
                    if ( pData->aDbgData.nTestFlags & DBG_TEST_XTOR_THIS )
                        pXtorData->aThisList.Remove( pThis );
                }
                else
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                break;

            case DBG_XTOR_CHKTHIS:
            case DBG_XTOR_CHKOBJ:
                if ( nAction & DBG_XTOR_DTOROBJ )
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_EXIT) &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                else
                {
                    if ( fDbgUsr &&
                         (pData->aDbgData.nTestFlags & DBG_TEST_XTOR_FUNC) )
                        pMsg = fDbgUsr( pThis );
                }
                break;
        }

        SAL_WARN_IF(
            pMsg, "tools.debug",
            "Error-Msg from Object " << pThis << " in class "
                << pDbgData->pName << ": " << pMsg);
    }

    SAL_INFO_IF(
        ((pData->aDbgData.nTestFlags & DBG_TEST_XTOR_TRACE)
         && (nAction & DBG_XTOR_DTOROBJ) && nAct != DBG_XTOR_CHKOBJ),
        "tools.debug",
        (nAct == DBG_XTOR_CTOR
         ? "Leave Ctor from class "
         : nAct == DBG_XTOR_DTOR
         ? "Leave Dtor from class "
         : "Leave method from class ") << pDbgData->pName);
}

void DbgOut( const sal_Char* pMsg, sal_uInt16 nDbgOut, const sal_Char* pFile, sal_uInt16 nLine )
{
    static sal_Bool bIn = sal_False;
    if ( bIn )
        return;
    bIn = sal_True;

    DebugData*  pData = GetDebugData();
    sal_Char const *   pStr;
    sal_uIntPtr       nOut;
    int         nBufLen = 0;

    if ( nDbgOut == DBG_OUT_ERROR )
    {
        nOut = pData->aDbgData.nErrorOut;
        pStr = "Error: ";
    }
    else if ( nDbgOut == DBG_OUT_WARNING )
    {
        nOut = pData->aDbgData.nWarningOut;
        pStr = "Warning: ";
    }
    else
    {
        nOut = pData->aDbgData.nTraceOut;
        pStr = NULL;
    }

    if ( nOut == DBG_OUT_NULL )
    {
        bIn = sal_False;
        return;
    }

    if ( ImplDbgFilterMessage( pMsg ) )
    {
        bIn = sal_False;
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

    if ( pFile && nLine && (nBufLen+nMsgLen < DBG_BUF_MAXLEN) )
    {
        if ( nOut == DBG_OUT_MSGBOX )
            strcat( aBufOut, "\n" );
        else
            strcat( aBufOut, " " );
        strcat( aBufOut, "From File " );
        strcat( aBufOut, pFile );
        strcat( aBufOut, " at Line " );

        // Convert line to String and append
        sal_Char    aLine[9];
        sal_Char*   pLine = &aLine[7];
        sal_uInt16      i;
        memset( aLine, 0, sizeof( aLine ) );
        do
        {
            i = nLine % 10;
            pLine--;
            *(pLine) = (sal_Char)i + 48;
            nLine /= 10;
        }
        while ( nLine );
        strcat( aBufOut, pLine );
    }

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
        if ( pData->pDbgPrintTestTool )
            pData->pDbgPrintTestTool( aBufOut );
        else
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

    bIn = sal_False;
}

void DbgPrintShell(char const * message) {
    fprintf(stderr, "%s\n", message);
#if defined WNT
    OutputDebugStringA(message);
#endif
}

void DbgOutTypef( sal_uInt16 nDbgOut, const sal_Char* pFStr, ... )
{
    va_list pList;

    va_start( pList, pFStr );
    sal_Char aBuf[DBG_BUF_MAXLEN];
    vsprintf( aBuf, pFStr, pList );
    va_end( pList );

    DbgOut( aBuf, nDbgOut );
}

void DbgOutf( const sal_Char* pFStr, ... )
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

void DbgProf( sal_uInt16, DbgDataType* ) {}
void DbgXtor( DbgDataType*, sal_uInt16, const void*, DbgUsr ) {}

void DbgOut( const sal_Char*, sal_uInt16, const sal_Char*, sal_uInt16 ) {}
void DbgOutTypef( sal_uInt16, const sal_Char*, ... ) {}
void DbgOutf( const sal_Char*, ... ) {}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
