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


#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "image.hxx"
#include <basic/sbobjmod.hxx>
#include <stdio.h>

// To activate tracing enable in sbtrace.hxx
#ifdef DBG_TRACE_BASIC

// Trace ini file (set NULL to ignore)
// can be overridden with the environment variable OOO_BASICTRACEINI
static char     GpTraceIniFile[] = "~/BasicTrace.ini";
//static char*  GpTraceIniFile = NULL;

// Trace Settings, used if no ini file / not found in ini file
static char     GpTraceFileNameDefault[] = "~/BasicTrace.txt";
static char*    GpTraceFileName = GpTraceFileNameDefault;

// GbTraceOn:
// true = tracing is active, false = tracing is disabled, default = true
// Set to false initially if you want to activate tracing on demand with
// TraceCommand( "TraceOn" ), see below
static bool GbTraceOn = true;

// GbIncludePCodes:
// true = PCodes are written to trace, default = false, correspondents
// with TraceCommand( "PCodeOn" / "PCodeOff" ), see below
static bool GbIncludePCodes = false;

// GbInitOnlyAtOfficeStart:
// true = Tracing is only intialized onces after Office start when
// Basic runs the first time. Further calls to Basic, e.g. via events
// use the same output file. The trace ini file is not read again.
static bool GbInitOnlyAtOfficeStart = false;

static int  GnIndentPerCallLevel = 4;
static int  GnIndentForPCode = 2;

/*
    With trace enabled the runtime function TraceCommand
    can be used to influence the trace functionality
    from within the running Basic macro.

    Format: TraceCommand( command as String [, param as Variant] )

    Supported commands (command is NOT case sensitive):
    TraceCommand "TraceOn"          sets GbTraceOn = true
    TraceCommand "TraceOff"         sets GbTraceOn = false

    TraceCommand "PCodeOn"          sets GbIncludePCodes = true
    TraceCommand "PCodeOff"         sets GbIncludePCodes = false

    TraceCommand "Print", aVal      writes aVal into the trace file as
                                    long as it can be converted to string
*/

#ifdef DBG_TRACE_PROFILING

#include <algorithm>
#include <stack>
#include "canvas/elapsedtime.hxx"

//*** Profiling ***
// GbTimerOn:
// true = including time stamps
static bool GbTimerOn = true;

// GbTimeStampForEachStep:
// true = prints time stamp after each command / pcode (very slow)
static bool GbTimeStampForEachStep = false;

// GbBlockAllAfterFirstFunctionUsage:
// true = everything (commands, pcodes, functions) is only printed
// for the first usage (improves performance when tracing / pro-
// filing large macros)
static bool GbBlockAllAfterFirstFunctionUsage = false;

// GbBlockStepsAfterFirstFunctionUsage:
// true = commands / pcodes are only printed for the first time
// a function is executed. Afterwards only the entering/leaving
// messages are logged (improves performance when tracing / pro-
// filing large macros)
static bool GbBlockStepsAfterFirstFunctionUsage = false;

#endif


static void lcl_skipWhites( char*& rpc )
{
    while( *rpc == ' ' || *rpc == '\t' )
        ++rpc;
}

inline void lcl_findNextLine( char*& rpc, char* pe )
{
    // Find line end
    while( rpc < pe && *rpc != 13 && *rpc != 10 )
        ++rpc;

    // Read all
    while( rpc < pe && (*rpc == 13 || *rpc == 10) )
        ++rpc;
}

inline bool lcl_isAlpha( char c )
{
    bool bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    return bRet;
}

static void lcl_ReadIniFile( const char* pIniFileName )
{
    const int BUF_SIZE = 1000;
    static sal_Char TraceFileNameBuffer[BUF_SIZE];
    sal_Char Buffer[BUF_SIZE];
    sal_Char VarNameBuffer[BUF_SIZE];
    sal_Char ValBuffer[BUF_SIZE];

    FILE* pFile = fopen( pIniFileName ,"rb" );
    if( pFile == NULL )
        return;

    size_t nRead = fread( Buffer, 1, BUF_SIZE, pFile );

    // Scan
    char* pc = Buffer;
    char* pe = Buffer + nRead;
    while( pc < pe )
    {
        lcl_skipWhites( pc ); if( pc == pe ) break;

        // Read variable
        char* pVarStart = pc;
        while( pc < pe && lcl_isAlpha( *pc ) )
            ++pc;
        int nVarLen = pc - pVarStart;
        if( nVarLen == 0 )
        {
            lcl_findNextLine( pc, pe );
            continue;
        }
        strncpy( VarNameBuffer, pVarStart, nVarLen );
        VarNameBuffer[nVarLen] = '\0';

        // Check =
        lcl_skipWhites( pc ); if( pc == pe ) break;
        if( *pc != '=' )
            continue;
        ++pc;
        lcl_skipWhites( pc ); if( pc == pe ) break;

        // Read value
        char* pValStart = pc;
        while( pc < pe && *pc != 13 && *pc != 10 )
            ++pc;
        int nValLen = pc - pValStart;
        if( nValLen == 0 )
        {
            lcl_findNextLine( pc, pe );
            continue;
        }
        strncpy( ValBuffer, pValStart, nValLen );
        ValBuffer[nValLen] = '\0';

        // Match variables
        if( strcmp( VarNameBuffer, "GpTraceFileName") == 0 )
        {
            strcpy( TraceFileNameBuffer, ValBuffer );
            GpTraceFileName = TraceFileNameBuffer;
        }
        else
        if( strcmp( VarNameBuffer, "GbTraceOn") == 0 )
            GbTraceOn = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GbIncludePCodes") == 0 )
            GbIncludePCodes = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GbInitOnlyAtOfficeStart") == 0 )
            GbInitOnlyAtOfficeStart = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GnIndentPerCallLevel") == 0 )
            GnIndentPerCallLevel = strtol( ValBuffer, NULL, 10 );
        else
        if( strcmp( VarNameBuffer, "GnIndentForPCode") == 0 )
            GnIndentForPCode = strtol( ValBuffer, NULL, 10 );
#ifdef DBG_TRACE_PROFILING
        else
        if( strcmp( VarNameBuffer, "GbTimerOn") == 0 )
            GbTimerOn = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GbTimeStampForEachStep") == 0 )
            GbTimeStampForEachStep = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GbBlockAllAfterFirstFunctionUsage") == 0 )
            GbBlockAllAfterFirstFunctionUsage = (strcmp( ValBuffer, "true" ) == 0);
        else
        if( strcmp( VarNameBuffer, "GbBlockStepsAfterFirstFunctionUsage") == 0 )
            GbBlockStepsAfterFirstFunctionUsage = (strcmp( ValBuffer, "true" ) == 0);
#endif
    }
    fclose( pFile );
}

struct TraceTextData
{
    OString m_aTraceStr_STMNT;
    OString m_aTraceStr_PCode;
};
typedef std::hash_map< sal_Int32, TraceTextData > PCToTextDataMap;
typedef std::hash_map< OUString, PCToTextDataMap*, OUStringHash, ::std::equal_to< OUString > > ModuleTraceMap;

ModuleTraceMap      GaModuleTraceMap;
ModuleTraceMap&     rModuleTraceMap = GaModuleTraceMap;

static FILE* GpGlobalFile = NULL;

static void lcl_lineOut( const char* pStr, const char* pPreStr = NULL, const char* pPostStr = NULL )
{
    if( GpGlobalFile != NULL )
    {
        fprintf( GpGlobalFile, "%s%s%s\n", pPreStr ? pPreStr : "", pStr, pPostStr ? pPostStr : "" );
        fflush( GpGlobalFile );
    }
}

const char* lcl_getSpaces( int nSpaceCount )
{
    static const sal_Char Spaces[] = "                                                                                                    "
        "                                                                                                    "
        "                                                                                                    ";
    static const int nAvailableSpaceCount = strlen( Spaces );
    static const sal_Char* pSpacesEnd = Spaces + nAvailableSpaceCount;

    if( nSpaceCount > nAvailableSpaceCount )
        nSpaceCount = nAvailableSpaceCount;

    return pSpacesEnd - nSpaceCount;
}

static OString lcl_toOStringSkipLeadingWhites( const OUString& aStr )
{
    static sal_Char Buffer[1000];

    OString aOStr = OUStringToOString( OUString( aStr ), RTL_TEXTENCODING_ASCII_US );
    const sal_Char* pStr = aOStr.getStr();

    // Skip whitespace
    sal_Char c = *pStr;
    while( c == ' ' || c == '\t' )
    {
        pStr++;
        c = *pStr;
    }

    int nLen = strlen( pStr );
    strncpy( Buffer, pStr, nLen );
    Buffer[nLen] = 0;

    OString aORetStr( Buffer );
    return aORetStr;
}

String lcl_dumpMethodParameters( SbMethod* pMethod )
{
    OUString aStr;
    if( pMethod == NULL )
    {
        return aStr;
    }
    SbxError eOld = SbxBase::GetError();

    SbxArray* pParams = pMethod->GetParameters();
    SbxInfo* pInfo = pMethod->GetInfo();
    if ( pParams )
    {
        aStr += '(';
        // 0 is sub itself
        for ( sal_uInt16 nParam = 1; nParam < pParams->Count(); nParam++ )
        {
            SbxVariable* pVar = pParams->Get( nParam );
            DBG_ASSERT( pVar, "Parameter?!" );
            if ( !pVar->GetName().isEmpty() )
            {
                aStr += pVar->GetName();
            }
            else if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( nParam );
                if ( pParam )
                {
                    aStr += pParam->aName;
                }
            }
            aStr += '=';
            SbxDataType eType = pVar->GetType();
            if( eType & SbxARRAY )
            {
                aStr += "...";
            }
            else if( eType != SbxOBJECT )
            {
                aStr += pVar->GetString();
            }
            if ( nParam < ( pParams->Count() - 1 ) )
            {
                aStr += ", ";
            }
        }
        aStr += ')';
    }

    SbxBase::ResetError();
    if( eOld != SbxERR_OK )
    {
        SbxBase::SetError( eOld );
    }
    return aStr;
}


// Public functions
static bool GbSavTraceOn = false;

#ifdef DBG_TRACE_PROFILING
static canvas::tools::ElapsedTime* GpTimer = NULL;
static double GdStartTime = 0.0;
static double GdLastTime = 0.0;
static bool GbBlockSteps = false;
static bool GbBlockAll = false;

struct FunctionItem
{
    OUString    m_aCompleteFunctionName;
    double      m_dTotalTime;
    double      m_dNetTime;
    int         m_nCallCount;
    bool        m_bBlockAll;
    bool        m_bBlockSteps;

    FunctionItem( void )
        : m_dTotalTime( 0.0 )
        , m_dNetTime( 0.0 )
        , m_nCallCount( 0 )
        , m_bBlockAll( false )
        , m_bBlockSteps( false )
    {}
};
typedef std::hash_map< OUString, FunctionItem*, OUStringHash, ::std::equal_to< OUString > > FunctionItemMap;

static std::stack< double >             GaCallEnterTimeStack;
static std::stack< FunctionItem* >      GaFunctionItemStack;
static FunctionItemMap                  GaFunctionItemMap;

bool compareFunctionNetTime( FunctionItem* p1, FunctionItem* p2 )
{
    return (p1->m_dNetTime > p2->m_dNetTime);
}

void lcl_printTimeOutput( void )
{
    // Overall time output
    lcl_lineOut( "" );
    lcl_lineOut( "***** Time Output *****" );
    char TimeBuffer[500];
    double dTotalTime = GpTimer->getElapsedTime() - GdStartTime;
    sprintf( TimeBuffer, "Total execution time = %f ms", dTotalTime*1000.0 );
    lcl_lineOut( TimeBuffer );
    lcl_lineOut( "" );

    if( GbTimerOn )
    {
        lcl_lineOut( "Functions:" );

        std::vector<FunctionItem*> avFunctionItems;

        FunctionItemMap::iterator it;
        for( it = GaFunctionItemMap.begin() ; it != GaFunctionItemMap.end() ; ++it )
        {
            FunctionItem* pFunctionItem = it->second;
            if( pFunctionItem != NULL )
            {
                avFunctionItems.push_back( pFunctionItem );
            }
        }

        std::sort( avFunctionItems.begin(), avFunctionItems.end(), compareFunctionNetTime );

        std::vector<FunctionItem*>::iterator itv;
        for( itv = avFunctionItems.begin() ; itv != avFunctionItems.end() ; ++itv )
        {
            FunctionItem* pFunctionItem = *itv;
            if( pFunctionItem != NULL )
            {
                OUString aCompleteFunctionName = pFunctionItem->m_aCompleteFunctionName;
                const char* pName = OUStringToOString( aCompleteFunctionName, RTL_TEXTENCODING_ASCII_US ).getStr();
                int nNameLen = aCompleteFunctionName.getLength();

                double dFctTotalTime = pFunctionItem->m_dTotalTime;
                double dFctNetTime = pFunctionItem->m_dNetTime;
                double dFctTotalTimePercent = 100.0 * dFctTotalTime / dTotalTime;
                double dFctNetTimePercent = 100.0 * dFctNetTime / dTotalTime;
                int nSpaceCount = 30 - nNameLen;
                if( nSpaceCount < 0 )
                {
                    nSpaceCount = 2;
                }
                sprintf( TimeBuffer, "%s:%sCalled %d times\t%f ms (%f%%) / net %f (%f%%) ms",
                         pName, lcl_getSpaces( nSpaceCount ), pFunctionItem->m_nCallCount,
                         dFctTotalTime*1000.0, dFctTotalTimePercent, dFctNetTime*1000.0, dFctNetTimePercent );
                lcl_lineOut( TimeBuffer );
            }
        }
    }
}
#endif


static bool GbInitTraceAlreadyCalled = false;

void dbg_InitTrace( void )
{
    if( GbInitOnlyAtOfficeStart && GbInitTraceAlreadyCalled )
    {
#ifdef DBG_TRACE_PROFILING
        if( GbTimerOn )
        {
            GpTimer->continueTimer();
        }
#endif
        GpGlobalFile = fopen( GpTraceFileName, "a+" );
        return;
    }
    GbInitTraceAlreadyCalled = true;

    if( const sal_Char* pcIniFileName = ::getenv( "OOO_BASICTRACEINI" ) )
    {
        lcl_ReadIniFile( pcIniFileName );
    }
    else if( GpTraceIniFile != NULL )
    {
        lcl_ReadIniFile( GpTraceIniFile );
    }
    GpGlobalFile = fopen( GpTraceFileName, "w" );
    GbSavTraceOn = GbTraceOn;
    if( !GbTraceOn )
    {
        lcl_lineOut( "### Program started with trace off ###" );
    }
#ifdef DBG_TRACE_PROFILING
    GpTimer = new canvas::tools::ElapsedTime();
    GdStartTime = GpTimer->getElapsedTime();
    GdLastTime = GdStartTime;
    GbBlockSteps = false;
    GbBlockAll = false;
#endif
}

void dbg_DeInitTrace( void )
{
    GbTraceOn = GbSavTraceOn;

#ifdef DBG_TRACE_PROFILING
    while( !GaCallEnterTimeStack.empty() )
    {
        GaCallEnterTimeStack.pop();
    }
    while( !GaFunctionItemStack.empty() )
    {
        GaFunctionItemStack.pop();
    }
    lcl_printTimeOutput();

    for( FunctionItemMap::iterator it = GaFunctionItemMap.begin() ; it != GaFunctionItemMap.end() ; ++it )
    {
        delete it->second;
    }
    GaFunctionItemMap.clear();

    if( GpGlobalFile )
    {
        fclose( GpGlobalFile );
        GpGlobalFile = NULL;
    }

    if( GbInitOnlyAtOfficeStart )
    {
        if( GbTimerOn )
        {
            GpTimer->pauseTimer();
        }
    }
    else
    {
        delete GpTimer;
    }
#endif
}

static sal_Int32 GnLastCallLvl = 0;

void dbg_tracePrint( const OUString& aStr, sal_Int32 nCallLvl, bool bCallLvlRelativeToCurrent )
{
    if( bCallLvlRelativeToCurrent )
    {
        nCallLvl += GnLastCallLvl;
    }
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    lcl_lineOut( OUStringToOString( OUString( aStr ), RTL_TEXTENCODING_ASCII_US ).getStr(), lcl_getSpaces( nIndent ) );
}

void dbg_traceStep( SbModule* pModule, sal_uInt32 nPC, sal_Int32 nCallLvl )
{
    if( !GbTraceOn )
    {
        return;
    }
#ifdef DBG_TRACE_PROFILING
    if( GbBlockSteps || GbBlockAll )
    {
        return;
    }
    double dCurTime = 0.0;
    bool bPrintTimeStamp = false;
    if( GbTimerOn )
    {
        GpTimer->pauseTimer();
        dCurTime = GpTimer->getElapsedTime();
        bPrintTimeStamp = GbTimeStampForEachStep;
    }
#else
    bool bPrintTimeStamp = false;
#endif

    GnLastCallLvl = nCallLvl;

    SbModule* pTraceMod = pModule;
    if( pTraceMod->ISA(SbClassModuleObject) )
    {
        SbClassModuleObject* pClassModuleObj = (SbClassModuleObject*)(SbxBase*)pTraceMod;
        pTraceMod = pClassModuleObj->getClassModule();
    }

    OUString aModuleName = pTraceMod->GetName();
    ModuleTraceMap::iterator it = rModuleTraceMap.find( aModuleName );
    if( it == rModuleTraceMap.end() )
    {
        const char* pModuleNameStr = OUStringToOString( OUString( aModuleName ), RTL_TEXTENCODING_ASCII_US ).getStr();
        char Buffer[200];
        sprintf( Buffer, "TRACE ERROR: Unknown module \"%s\"", pModuleNameStr );
        lcl_lineOut( Buffer );
        return;
    }

    PCToTextDataMap* pInnerMap = it->second;
    if( pInnerMap == NULL )
    {
        lcl_lineOut( "TRACE INTERNAL ERROR: No inner map" );
        return;
    }

    PCToTextDataMap::iterator itInner = pInnerMap->find( nPC );
    if( itInner == pInnerMap->end() )
    {
        const char* pModuleNameStr = OUStringToOString( OUString( aModuleName ), RTL_TEXTENCODING_ASCII_US ).getStr();
        char Buffer[200];
        sprintf( Buffer, "TRACE ERROR: No info for PC = %d in module \"%s\"", (int)nPC, pModuleNameStr );
        lcl_lineOut( Buffer );
        return;
    }

    int nIndent = nCallLvl * GnIndentPerCallLevel;

    const TraceTextData& rTraceTextData = itInner->second;
    const OString& rStr_STMNT = rTraceTextData.m_aTraceStr_STMNT;
    bool bSTMT = false;
    if( rStr_STMNT.getLength() )
    {
        bSTMT = true;
    }
    char TimeBuffer[200];
    memset (TimeBuffer, 0, size(TimeBuffer));
#ifdef DBG_TRACE_PROFILING
    if( bPrintTimeStamp )
    {
        double dDiffTime = dCurTime - GdLastTime;
        GdLastTime = dCurTime;
        sprintf( TimeBuffer, "\t\t// Time = %f ms / += %f ms", dCurTime*1000.0, dDiffTime*1000.0 );
    }
#endif

    if( bSTMT )
    {
        lcl_lineOut( rStr_STMNT.getStr(), lcl_getSpaces( nIndent ),
                     (bPrintTimeStamp && !GbIncludePCodes) ? TimeBuffer : NULL );
    }

    if( !GbIncludePCodes )
    {
#ifdef DBG_TRACE_PROFILING
        if( GbTimerOn )
        {
            GpTimer->continueTimer();
        }
#endif
        return;
    }

    nIndent += GnIndentForPCode;
    const OString& rStr_PCode = rTraceTextData.m_aTraceStr_PCode;
    if( rStr_PCode.getLength() )
    {
        lcl_lineOut( rStr_PCode.getStr(), lcl_getSpaces( nIndent ),
                     bPrintTimeStamp ? TimeBuffer : NULL );
    }

#ifdef DBG_TRACE_PROFILING
    if( GbTimerOn )
    {
        GpTimer->continueTimer();
    }
#endif
}


void dbg_traceNotifyCall( SbModule* pModule, SbMethod* pMethod, sal_Int32 nCallLvl, bool bLeave )
{

    if( !GbTraceOn )
    {
        return;
    }
#ifdef DBG_TRACE_PROFILING
    double dCurTime = 0.0;
    double dExecutionTime = 0.0;
    if( GbTimerOn )
    {
        dCurTime = GpTimer->getElapsedTime();
        GpTimer->pauseTimer();
    }
#endif

    GnLastCallLvl = nCallLvl;

    SbModule* pTraceMod = pModule;
    SbClassModuleObject* pClassModuleObj = NULL;
    if( pTraceMod->ISA(SbClassModuleObject) )
    {
        pClassModuleObj = (SbClassModuleObject*)(SbxBase*)pTraceMod;
        pTraceMod = pClassModuleObj->getClassModule();
    }

    OUString aCompleteFunctionName = pTraceMod->GetName();
    if( pMethod != NULL )
    {
        aCompleteFunctionName +=  "::";
        OUString aMethodName = pMethod->GetName();
        aCompleteFunctionName += aMethodName;
    }
    else
    {
        aCompleteFunctionName += "/RunInit";
    }

    bool bOwnBlockSteps = false;
#ifdef DBG_TRACE_PROFILING
    bool bOwnBlockAll = false;
    FunctionItem* pFunctionItem = NULL;
    if( GbTimerOn )
    {
        FunctionItemMap::iterator itFunctionItem = GaFunctionItemMap.find( aCompleteFunctionName );
        if( itFunctionItem != GaFunctionItemMap.end() )
        {
            pFunctionItem = itFunctionItem->second;
        }
        if( pFunctionItem == NULL )
        {
            DBG_ASSERT( !bLeave, "No FunctionItem in leave!" );

            pFunctionItem = new FunctionItem();
            pFunctionItem->m_aCompleteFunctionName = aCompleteFunctionName;
            GaFunctionItemMap[ aCompleteFunctionName ] = pFunctionItem;
        }
        else if( GbBlockAllAfterFirstFunctionUsage && !bLeave )
        {
            pFunctionItem->m_bBlockAll = true;
        }
        else if( GbBlockStepsAfterFirstFunctionUsage && !bLeave )
        {
            pFunctionItem->m_bBlockSteps = true;
        }

        if( bLeave )
        {
            bOwnBlockAll = GbBlockAll;
            bOwnBlockSteps = GbBlockSteps;
            GbBlockAll = false;
            GbBlockSteps = false;

            dExecutionTime = dCurTime - GaCallEnterTimeStack.top();
            GaCallEnterTimeStack.pop();

            pFunctionItem->m_dTotalTime += dExecutionTime;
            pFunctionItem->m_dNetTime += dExecutionTime;
            pFunctionItem->m_nCallCount++;

            GaFunctionItemStack.pop();
            if( !GaFunctionItemStack.empty() )
            {
                FunctionItem* pParentItem = GaFunctionItemStack.top();
                if( pParentItem != NULL )
                {
                    pParentItem->m_dNetTime -= dExecutionTime;

                    GbBlockSteps = pParentItem->m_bBlockSteps;
                    GbBlockAll = pParentItem->m_bBlockAll;
                }
            }
        }
        else
        {
            GbBlockSteps = bOwnBlockSteps = pFunctionItem->m_bBlockSteps;
            GbBlockAll = bOwnBlockAll = pFunctionItem->m_bBlockAll;

            GaCallEnterTimeStack.push( dCurTime );
            GaFunctionItemStack.push( pFunctionItem );
        }
    }

    if( bOwnBlockAll )
    {
        if( GbTimerOn )
        {
            GpTimer->continueTimer();
        }
        return;
    }
#endif

    if( nCallLvl > 0 )
    {
        nCallLvl--;
    }
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    if( !bLeave && !bOwnBlockSteps )
    {
        static const char* pSeparator = "' ================================================================================";
        lcl_lineOut( "" );
        lcl_lineOut( pSeparator, lcl_getSpaces( nIndent ) );
    }

    OUString aStr;
    if( bLeave )
    {
        if( !bOwnBlockSteps )
        {
            lcl_lineOut( "}", lcl_getSpaces( nIndent ) );
            aStr = "' Leaving ";
        }
    }
    else
    {
        aStr = "Entering " ;
    }
    if( !bLeave || !bOwnBlockSteps )
    {
        aStr += aCompleteFunctionName;
    }
    if( !bOwnBlockSteps && pClassModuleObj != NULL )
    {
        aStr += "[this=";
        aStr += pClassModuleObj->GetName();
        aStr += "]" ;
    }
    if( !bLeave )
    {
        aStr += lcl_dumpMethodParameters( pMethod );
    }
    const char* pPostStr = NULL;
#ifdef DBG_TRACE_PROFILING
    char TimeBuffer[200];
    if( GbTimerOn && bLeave )
    {
        sprintf( TimeBuffer, "    // Execution Time = %f ms", dExecutionTime*1000.0 );
        pPostStr = TimeBuffer;
    }
#endif
    lcl_lineOut( (!bLeave || !bOwnBlockSteps) ? OUStringToOString( OUString( aStr ), RTL_TEXTENCODING_ASCII_US ).getStr() : "}",
                 lcl_getSpaces( nIndent ), pPostStr );
    if( !bLeave )
    {
        lcl_lineOut( "{", lcl_getSpaces( nIndent ) );
    }
    if( bLeave && !bOwnBlockSteps )
    {
        lcl_lineOut( "" );
    }
#ifdef DBG_TRACE_PROFILING
    if( GbTimerOn )
    {
        GpTimer->continueTimer();
    }
#endif
}

void dbg_traceNotifyError( SbError nTraceErr, const OUString& aTraceErrMsg,
                           bool bTraceErrHandled, sal_Int32 nCallLvl )
{
    if( !GbTraceOn )
    {
        return;
    }
#ifdef DBG_TRACE_PROFILING
    if( GbBlockSteps || GbBlockAll )
    {
        return;
    }
#endif
    GnLastCallLvl = nCallLvl;

    OString aOTraceErrMsg = OUStringToOString( OUString( aTraceErrMsg ), RTL_TEXTENCODING_ASCII_US );

    char Buffer[200];
    const char* pHandledStr = bTraceErrHandled ? " / HANDLED" : "";
    sprintf( Buffer, "*** ERROR%s, Id = %d, Msg = \"%s\" ***", pHandledStr, (int)nTraceErr, aOTraceErrMsg.getStr() );
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    lcl_lineOut( Buffer, lcl_getSpaces( nIndent ) );
}

void dbg_RegisterTraceTextForPC( SbModule* pModule, sal_uInt32 nPC,
                                 const OUString& aTraceStr_STMNT, const OUString& aTraceStr_PCode )
{
    OUString aModuleName = pModule->GetName();
    ModuleTraceMap::iterator it = rModuleTraceMap.find( aModuleName );
    PCToTextDataMap* pInnerMap;
    if( it == rModuleTraceMap.end() )
    {
        pInnerMap = new PCToTextDataMap();
        rModuleTraceMap[ aModuleName ] = pInnerMap;
    }
    else
    {
        pInnerMap = it->second;
    }

    TraceTextData aData;

    OString aOTraceStr_STMNT = lcl_toOStringSkipLeadingWhites( aTraceStr_STMNT );
    aData.m_aTraceStr_STMNT = aOTraceStr_STMNT;

    OString aOTraceStr_PCode = lcl_toOStringSkipLeadingWhites( aTraceStr_PCode );
    aData.m_aTraceStr_PCode = aOTraceStr_PCode;

    (*pInnerMap)[nPC] = aData;
}

void RTL_Impl_TraceCommand( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

    if ( rPar.Count() < 2 )
    {
        StarBASIC::Error( SbERR_BAD_ARGUMENT );
        return;
    }

    OUString aCommand = rPar.Get(1)->GetString();

    if( aCommand.equalsIngoreAsciiCase( "TraceOn" ) )
    {
        GbTraceOn = true;
    }
    else if( aCommand.equalsIngoreAsciiCase( "TraceOff" ) )
    {
        GbTraceOn = false;
    }
    else if( aCommand.equalsIngoreAsciiCase( "PCodeOn" ) )
    {
        GbIncludePCodes = true;
    }
    else if( aCommand.equalsIngoreAsciiCase( "PCodeOff" ) )
    {
        GbIncludePCodes = false;
    }
    else if( aCommand.equalsIngoreAsciiCase( "Print" ) )
    {
        if ( rPar.Count() < 3 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }

        SbxError eOld = SbxBase::GetError();
        if( eOld != SbxERR_OK )
            SbxBase::ResetError();

        OUString aValStr = rPar.Get(2)->GetString();
        SbxError eErr = SbxBase::GetError();
        if( eErr != SbxERR_OK )
        {
            aValStr = "<ERROR converting value to String>";
            SbxBase::ResetError();
        }

        char Buffer[500];
        const char* pValStr = OUStringToOString( OUString( aValStr ), RTL_TEXTENCODING_ASCII_US ).getStr();

        sprintf( Buffer, "### TRACE_PRINT: %s ###", pValStr );
        int nIndent = GnLastCallLvl * GnIndentPerCallLevel;
        lcl_lineOut( Buffer, lcl_getSpaces( nIndent ) );

        if( eOld != SbxERR_OK )
        {
            SbxBase::SetError( eOld );
        }
    }
}

#endif

// This routine is defined here, so that the
// compiler can be loaded as a discrete segment.

sal_Bool SbModule::Compile()
{
    if( pImage )
        return sal_True;
    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( !pBasic )
        return sal_False;
    SbxBase::ResetError();

    SbModule* pOld = GetSbData()->pCompMod;
    GetSbData()->pCompMod = this;

    SbiParser* pParser = new SbiParser( (StarBASIC*) GetParent(), this );
    while( pParser->Parse() ) {}
    if( !pParser->GetErrors() )
        pParser->aGen.Save();
    delete pParser;
    // for the disassembler
    if( pImage )
        pImage->aOUSource = aOUSource;

    GetSbData()->pCompMod = pOld;

    // compiling a module, the module-global
    // variables of all modules become invalid
    sal_Bool bRet = IsCompiled();
    if( bRet )
    {
        if( !this->ISA(SbObjModule) )
            pBasic->ClearAllModuleVars();
        RemoveVars(); // remove 'this' Modules variables
        // clear all method statics
        for( sal_uInt16 i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* p = PTR_CAST(SbMethod,pMethods->Get( i ) );
            if( p )
                p->ClearStatics();
        }

        // #i31510 Init other libs only if Basic isn't running
        if( GetSbData()->pInst == NULL )
        {
            SbxObject* pParent_ = pBasic->GetParent();
            if( pParent_ )
                pBasic = PTR_CAST(StarBASIC,pParent_);
            if( pBasic )
                pBasic->ClearAllModuleVars();
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
