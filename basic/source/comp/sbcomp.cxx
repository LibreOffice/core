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
#include "precompiled_basic.hxx"

#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include "image.hxx"
#include "sbtrace.hxx"


//==========================================================================
// Tracing, for debugging only

// To activate tracing enable in sbtrace.hxx
#ifdef DBG_TRACE_BASIC

#include <hash_map>

// Trace Settings
static const char*  GpTraceFileName = "d:\\zBasic.Asm\\BasicTrace.txt";
static const bool   GbIncludePCodes = false;
static const int    GnIndentPerCallLevel = 4;
static const int    GnIndentForPCode = 2;

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

String dumpMethodParameters( SbMethod* pMethod )
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
        for ( USHORT nParam = 1; nParam < pParams->Count(); nParam++ )
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
            if( pVar->GetType() & SbxARRAY )
                aStr += String( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            else
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
void dbg_InitTrace( void )
{
    FILE* pFile = fopen( GpTraceFileName, "w" );
    if( pFile != NULL )
        fclose( pFile );
}

void dbg_traceStep( SbModule* pModule, UINT32 nPC, INT32 nCallLvl )
{
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
        sprintf( Buffer, "TRACE ERROR: No info for PC = %d in module \"%s\"", nPC, pModuleNameStr );
        lcl_lineOut( GpTraceFileName, Buffer );
        return;
    }

    //nCallLvl--;
    //if( nCallLvl < 0 )
    //  nCallLvl = 0;
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

void dbg_traceNotifyCall( SbModule* pModule, SbMethod* pMethod, INT32 nCallLvl, bool bLeave )
{
    static const char* pSeparator = "' ================================================================================";

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
        aStr += dumpMethodParameters( pMethod );

    lcl_lineOut( GpTraceFileName, OUStringToOString( rtl::OUString( aStr ), RTL_TEXTENCODING_ASCII_US ).getStr(), lcl_getSpaces( nIndent ) );
    if( !bLeave )
        lcl_lineOut( GpTraceFileName, "{", lcl_getSpaces( nIndent ) );

    if( bLeave )
        lcl_lineOut( GpTraceFileName, "" );
}

void dbg_traceNotifyError( SbError nTraceErr, const String& aTraceErrMsg, bool bTraceErrHandled, INT32 nCallLvl )
{
    rtl::OString aOTraceErrMsg = OUStringToOString( rtl::OUString( aTraceErrMsg ), RTL_TEXTENCODING_ASCII_US );

    char Buffer[200];
    const char* pHandledStr = bTraceErrHandled ? " / HANDLED" : "";
    sprintf( Buffer, "*** ERROR%s, Id = %d, Msg = \"%s\" ***", pHandledStr, (int)nTraceErr, aOTraceErrMsg.getStr() );
    int nIndent = nCallLvl * GnIndentPerCallLevel;
    lcl_lineOut( GpTraceFileName, Buffer, lcl_getSpaces( nIndent ) );
}

void dbg_RegisterTraceTextForPC( SbModule* pModule, UINT32 nPC,
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
                ( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )) ), UNO_QUERY );
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
                Reference< XInterface > x = xSMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.TextOutputStream" )) );
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

BOOL SbModule::Compile()
{
    if( pImage )
        return TRUE;
    StarBASIC* pBasic = PTR_CAST(StarBASIC,GetParent());
    if( !pBasic )
        return FALSE;
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
    BOOL bRet = IsCompiled();
    if( bRet )
    {
        pBasic->ClearAllModuleVars();
        RemoveVars(); // remove 'this' Modules variables
        // clear all method statics
        for( USHORT i = 0; i < pMethods->Count(); i++ )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
