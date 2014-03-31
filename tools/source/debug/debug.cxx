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
    DbgTestSolarMutexProc   pDbgTestSolarMutex;

    DebugData()
        :bInit( false )
        ,pDbgTestSolarMutex( NULL )
    {
        aDbgData.nTestFlags = DBG_TEST_RESOURCE;
        aDbgData.aDbgWinState[0] = 0;
    }
};

static DebugData aDebugData;

#define FILE_LINEEND    "\n"

typedef FILE*       FILETYPE;
#define FileOpen    fopen
#define FileRead    fread
#define FilePrintF  fprintf
#define FileClose   fclose

namespace
{
    enum ConfigSection
    {
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

static DebugData* GetDebugData()
{
    if ( !aDebugData.bInit )
    {
        aDebugData.bInit = true;

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

void* DbgFunc( sal_uInt16 nAction, void* pParam )
{
    DebugData* pDebugData = ImplGetDebugData();

    if ( nAction == DBG_FUNC_GETDATA )
        return (void*)&(pDebugData->aDbgData);
    else
    {
        switch ( nAction )
        {
            case DBG_FUNC_SAVEDATA:
                {
                const DbgData* pData = static_cast< const DbgData* >( pParam );

                sal_Char aBuf[ 4096 ];
                DbgGetDbgFileName( aBuf, sizeof( aBuf ) );
                FILETYPE pIniFile = FileOpen( aBuf, "w" );
                if ( pIniFile == NULL )
                    break;

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
       }

        return NULL;
    }
}

#else

void* DbgFunc( sal_uInt16, void* ) { return NULL; }

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
