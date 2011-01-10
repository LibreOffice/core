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
#include "precompiled_basic.hxx"

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "image.hxx"
#include "sbtrace.hxx"
#include <basic/sbobjmod.hxx>


//==========================================================================
// Tracing, for debugging only

// To activate tracing enable in sbtrace.hxx
#ifdef DBG_TRACE_BASIC

#include <hash_map>

// Trace ini file (set NULL to ignore)
static char     GpTraceIniFile[] = "d:\\zBasic.Asm\\BasicTrace.ini";
//static char*  GpTraceIniFile = NULL;


// Trace Settings, used if no ini file / not found in ini file
static char     GpTraceFileNameDefault[] = "d:\\zBasic.Asm\\BasicTrace.txt";
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
        if( strcmp( VarNameBuffer, "GnIndentPerCallLevel") == 0 )
            GnIndentPerCallLevel = strtol( ValBuffer, NULL, 10 );
        else
        if( strcmp( VarNameBuffer, "GnIndentForPCode") == 0 )
            GnIndentForPCode = strtol( ValBuffer, NULL, 10 );
    }
    fclose( pFile );
}

struct TraceTextData
{
    rtl::OString m_aTraceStr_STMNT;
    rtl::OString m_aTraceStr_PCode;
};
typedef std::hash_map< sal_Int32, TraceTextData > PCToTextDataMap;
typedef std::hash_map< ::rtl::OUString, PCToTextDataMap*, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > ModuleTraceMap;

ModuleTraceMap      GaModuleTraceMap;
ModuleTraceMap&     rModuleTraceMap = GaModuleTraceMap;

static void lcl_PrepareTraceForModule( SbModule* pModule )
{
    String aModuleName = pModule->GetName();
    ModuleTraceMap::iterator it = rModuleTraceMap.find( aModuleName );
    if( it != rModuleTraceMap.end() )
    {
        PCToTextDataMap* pInnerMap = it->second;
        delete pInnerMap;
        rModuleTraceMap.erase( it );
    }

    String aDisassemblyStr;
    pModule->Disassemble( aDisassemblyStr );
}

static void lcl_lineOut( const char* pFileName, const char* pStr, const char* pPreStr = NULL )
{
    const char* pPrintFirst = (pPreStr != NULL) ? pPreStr : "";
    FILE* pFile = fopen( pFileName, "a+" );
    if( pFile != NULL )
    {
        fprintf( pFile, "%s%s\n", pPrintFirst, pStr );
        fclose( pFile );
    }
}

const char* lcl_getSpaces( int nSpaceCount )
{
    static sal_Char Spaces[] = "                                                                                                    "
        "                                                                                                    "
        "                                                                                                    ";
    static int nAvailableSpaceCount = strlen( Spaces );
    static sal_Char* pSpacesEnd = Spaces + nAvailableSpaceCount;

    if( nSpaceCount > nAvailableSpaceCount )
        nSpaceCount = nAvailableSpaceCount;

    return pSpacesEnd - nSpaceCount;
}

static rtl::OString lcl_toOStringSkipLeadingWhites( const String& aStr )
{
    static sal_Char Buffer[1000];

    rtl::OString aOStr = OUStringToOString( rtl::OUString( aStr ), RTL_TEXTENCODING_ASCII_US );
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

    rtl::OString aORetStr( Buffer );
    return aORetStr;
}

String lcl_dumpMethodParameters( SbMethod* pMethod )
{
    String aStr;
    if( pMethod == NULL )
        return aStr;

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
            if ( pVar->GetName().Len() )
                aStr += pVar->GetName();
            else if ( pInfo )
            {
                const SbxParamInfo* pParam = pInfo->GetParam( nParam );
                if ( pParam )
                    aStr += pParam->aName;
            }
            aStr += '=';
            SbxDataType eType = pVar->GetType();
            if( eType & SbxARRAY )
                aStr += String( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            else if( eType != SbxOBJECT )
                aStr += pVar->GetString();
            if ( nParam < ( pParams->Count() - 1 ) )
                aStr += String( RTL_CONSTASCII_USTRINGPARAM( ", " ) );
        }
        aStr += ')';
    }

    SbxBase::ResetError();
    if( eOld != SbxERR_OK )
        SbxBase::SetError( eOld );

    return aStr;
}


// Public functions

static bool GbSavTraceOn = false;
void dbg_InitTrace( void )
{
    if( GpTraceIniFile != NULL )
        lcl_ReadIniFile( GpTraceIniFile );

    FILE* pFile = fopen( GpTraceFileName, "w" );
    if( pFile != NULL )
        fclose( pFile );
    GbSavTraceOn = GbTraceOn;
    if( !GbTraceOn )
        lcl_lineOut( GpTraceFileName, "### Program started with trace off ###" );
}

void dbg_DeInitTrace( void )
{
    GbTraceOn = GbSavTraceOn;
}

static sal_Int32 GnLastCallLvl = 0;

void dbg_traceStep( SbModule* pModule, sal_uInt32 nPC, sal_Int32 nCallLvl )
{
    if( !GbTraceOn )
        return;
    GnLastCallLvl = nCallLvl;

    SbModule* pTraceMod = pModule;
    if( pTraceMod->ISA(SbClassModuleObject) )
    {
        SbClassModuleObject* pClassModuleObj = (SbClassModuleObject*)(SbxBase*)pTraceMod;
        pTraceMod = pClassModuleObj->getClassModule();
    }

    String aModuleName = pTraceMod->GetName();
    ModuleTraceMap::iterator it = rModuleTraceMap.find( aModuleName );
    if( it == rModuleTraceMap.end() )
    {
        const char* pModuleNameStr = OUStringToOString( rtl::OUString( aModuleName ), RTL_TEXTENCODING_ASCII_US ).getStr();
        char Buffer[200];
        sprintf( Buffer, "TRACE ERROR: Unknown module \"%s\"", pModuleNameStr );
        lcl_lineOut( GpTraceFileName, Buffer );
        return;
    }

    PCToTextDataMap* pInnerMap = it->second;
    if( pInnerMap == NULL )
    {
        lcl_lineOut( GpTraceFileName, "TRACE INTERNAL ERROR: No inner map" );
        return;
    }

    PCToTextDataMap::iterator itInner = pInnerMap->find( nPC );
    if( itInner == pInnerMap->end() )
    {
        const char* pModuleNameStr = OUStringToOString( rtl::OUString( aModuleName ), RTL_TEXTENCODING_ASCII_US ).getStr();
        char Buffer[200];
        sprintf( Buffer, "TRACE ERROR: No info for PC = %d in module \"%s\"", (int)nPC, pModuleNameStr );
        lcl_lineOut( GpTraceFileName, Buffer );
        return;
    }

    int nIndent = nCallLvl * GnIndentPerCallLevel;

    const TraceTextData& rTraceTextData = itInner->second;
    const rtl::OString& rStr_STMNT = rTraceTextData.m_aTraceStr_STMNT;
    if( rStr_STMNT.getLength() )
        lcl_lineOut( GpTraceFileName, rStr_STMNT.getStr(), lcl_getSpaces( nIndent ) );

    if( !GbIncludePCodes )
        return;

    nIndent += GnIndentForPCode;
    const rtl::OString& rStr_PCode = rTraceTextData.m_aTraceStr_PCode;
    if( rStr_PCode.getLength() )
        lcl_lineOut( GpTraceFileName, rStr_PCode.getStr(), lcl_getSpaces( nIndent ) );
}

void dbg_traceNotifyCall( SbModule* pModule, SbMethod* pMethod, sal_Int32 nCallLvl, bool bLeave )
{
    static const char* pSeparator = "' ================================================================================";

    if( !GbTraceOn )
        return;
    GnLastCallLvl = nCallLvl;

    SbModule* pTraceMod = pModule;
    SbClassModuleObject* pClassModuleObj = NULL;
    if( pTraceMod->ISA(SbClassModuleObject) )
    {
        pClassModuleObj = (SbClassModuleObject*)(SbxBase*)pTraceMod;
        pTraceMod = pClassModuleObj->getClassModule();
    }

    if( nCallLvl > 0 )
        nCallLvl--;
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    if( !bLeave )
    {
        lcl_lineOut( GpTraceFileName, "" );
        lcl_lineOut( GpTraceFileName, pSeparator, lcl_getSpaces( nIndent ) );
    }

    String aStr;
    if( bLeave )
    {
        lcl_lineOut( GpTraceFileName, "}", lcl_getSpaces( nIndent ) );
        aStr.AppendAscii( "' Leaving " );
    }
    else
    {
        aStr.AppendAscii( "Entering " );
    }
    String aModuleName = pTraceMod->GetName();
    aStr += aModuleName;
    if( pMethod != NULL )
    {
        aStr.AppendAscii( "::" );
        String aMethodName = pMethod->GetName();
        aStr += aMethodName;
    }
    else
    {
        aStr.AppendAscii( "/RunInit" );
    }

    if( pClassModuleObj != NULL )
    {
        aStr.AppendAscii( "[this=" );
        aStr += pClassModuleObj->GetName();
        aStr.AppendAscii( "]" );
    }
    if( !bLeave )
        aStr += lcl_dumpMethodParameters( pMethod );

    lcl_lineOut( GpTraceFileName, OUStringToOString( rtl::OUString( aStr ), RTL_TEXTENCODING_ASCII_US ).getStr(), lcl_getSpaces( nIndent ) );
    if( !bLeave )
        lcl_lineOut( GpTraceFileName, "{", lcl_getSpaces( nIndent ) );

    if( bLeave )
        lcl_lineOut( GpTraceFileName, "" );
}

void dbg_traceNotifyError( SbError nTraceErr, const String& aTraceErrMsg, bool bTraceErrHandled, sal_Int32 nCallLvl )
{
    if( !GbTraceOn )
        return;
    GnLastCallLvl = nCallLvl;

    rtl::OString aOTraceErrMsg = OUStringToOString( rtl::OUString( aTraceErrMsg ), RTL_TEXTENCODING_ASCII_US );

    char Buffer[200];
    const char* pHandledStr = bTraceErrHandled ? " / HANDLED" : "";
    sprintf( Buffer, "*** ERROR%s, Id = %d, Msg = \"%s\" ***", pHandledStr, (int)nTraceErr, aOTraceErrMsg.getStr() );
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    lcl_lineOut( GpTraceFileName, Buffer, lcl_getSpaces( nIndent ) );
}

void dbg_RegisterTraceTextForPC( SbModule* pModule, sal_uInt32 nPC,
    const String& aTraceStr_STMNT, const String& aTraceStr_PCode )
{
    String aModuleName = pModule->GetName();
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

    rtl::OString aOTraceStr_STMNT = lcl_toOStringSkipLeadingWhites( aTraceStr_STMNT );
    aData.m_aTraceStr_STMNT = aOTraceStr_STMNT;

    rtl::OString aOTraceStr_PCode = lcl_toOStringSkipLeadingWhites( aTraceStr_PCode );
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

    String aCommand = rPar.Get(1)->GetString();

    if( aCommand.EqualsIgnoreCaseAscii( "TraceOn" ) )
        GbTraceOn = true;
    else
    if( aCommand.EqualsIgnoreCaseAscii( "TraceOff" ) )
        GbTraceOn = false;
    else
    if( aCommand.EqualsIgnoreCaseAscii( "PCodeOn" ) )
        GbIncludePCodes = true;
    else
    if( aCommand.EqualsIgnoreCaseAscii( "PCodeOff" ) )
        GbIncludePCodes = false;
    else
    if( aCommand.EqualsIgnoreCaseAscii( "Print" ) )
    {
        if ( rPar.Count() < 3 )
        {
            StarBASIC::Error( SbERR_BAD_ARGUMENT );
            return;
        }

        SbxError eOld = SbxBase::GetError();
        if( eOld != SbxERR_OK )
            SbxBase::ResetError();

        String aValStr = rPar.Get(2)->GetString();
        SbxError eErr = SbxBase::GetError();
        if( eErr != SbxERR_OK )
        {
            aValStr = String( RTL_CONSTASCII_USTRINGPARAM( "<ERROR converting value to String>" ) );
            SbxBase::ResetError();
        }

        char Buffer[500];
        const char* pValStr = OUStringToOString( rtl::OUString( aValStr ), RTL_TEXTENCODING_ASCII_US ).getStr();

        sprintf( Buffer, "### TRACE_PRINT: %s ###", pValStr );
        int nIndent = GnLastCallLvl * GnIndentPerCallLevel;
        lcl_lineOut( GpTraceFileName, Buffer, lcl_getSpaces( nIndent ) );

        if( eOld != SbxERR_OK )
            SbxBase::SetError( eOld );
    }
}

#endif


//==========================================================================
// For debugging only
//#define DBG_SAVE_DISASSEMBLY

#ifdef DBG_SAVE_DISASSEMBLY
static bool dbg_bDisassemble = true;
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/io/XTextOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

using namespace comphelper;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;

void dbg_SaveDisassembly( SbModule* pModule )
{
    bool bDisassemble = dbg_bDisassemble;
    if( bDisassemble )
    {
        Reference< XSimpleFileAccess3 > xSFI;
        Reference< XTextOutputStream > xTextOut;
        Reference< XOutputStream > xOut;
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            Reference< XSimpleFileAccess3 > xSFI = Reference< XSimpleFileAccess3 >( xSMgr->createInstance
                ( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
            if( xSFI.is() )
            {
                String aFile( RTL_CONSTASCII_USTRINGPARAM("file:///d:/zBasic.Asm/Asm_") );
                StarBASIC* pBasic = (StarBASIC*)pModule->GetParent();
                if( pBasic )
                {
                    aFile += pBasic->GetName();
                    aFile.AppendAscii( "_" );
                }
                aFile += pModule->GetName();
                aFile.AppendAscii( ".txt" );

                // String aFile( RTL_CONSTASCII_USTRINGPARAM("file:///d:/BasicAsm.txt") );
                if( xSFI->exists( aFile ) )
                    xSFI->kill( aFile );
                xOut = xSFI->openFileWrite( aFile );
                Reference< XInterface > x = xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.io.TextOutputStream" ) );
                Reference< XActiveDataSource > xADS( x, UNO_QUERY );
                xADS->setOutputStream( xOut );
                xTextOut = Reference< XTextOutputStream >( x, UNO_QUERY );
            }
        }

        if( xTextOut.is() )
        {
            String aDisassemblyStr;
            pModule->Disassemble( aDisassemblyStr );
            xTextOut->writeString( aDisassemblyStr );
        }
        xOut->closeOutput();
    }
}
#endif


// Diese Routine ist hier definiert, damit der Compiler als eigenes Segment
// geladen werden kann.

sal_Bool SbModule::Compile()
{
    if( pImage )
        return sal_True;
    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( !pBasic )
        return sal_False;
    SbxBase::ResetError();
    // Aktuelles Modul!
    SbModule* pOld = pCMOD;
    pCMOD = this;

    SbiParser* pParser = new SbiParser( (StarBASIC*) GetParent(), this );
    while( pParser->Parse() ) {}
    if( !pParser->GetErrors() )
        pParser->aGen.Save();
    delete pParser;
    // fuer den Disassembler
    if( pImage )
        pImage->aOUSource = aOUSource;

    pCMOD = pOld;

    // Beim Compilieren eines Moduls werden die Modul-globalen
    // Variablen aller Module ungueltig
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
        if( pINST == NULL )
        {
            SbxObject* pParent_ = pBasic->GetParent();
            if( pParent_ )
                pBasic = PTR_CAST(StarBASIC,pParent_);
            if( pBasic )
                pBasic->ClearAllModuleVars();
        }
    }

#ifdef DBG_SAVE_DISASSEMBLY
    dbg_SaveDisassembly( this );
#endif

#ifdef DBG_TRACE_BASIC
    lcl_PrepareTraceForModule( this );
#endif

    return bRet;
}

/**************************************************************************
*
*   Syntax-Highlighting
*
**************************************************************************/

void StarBASIC::Highlight( const String& rSrc, SbTextPortions& rList )
{
    SbiTokenizer aTok( rSrc );
    aTok.Hilite( rList );
}

