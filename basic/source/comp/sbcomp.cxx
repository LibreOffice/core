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
#include <basic/sbobjmod.hxx>

// To activate tracing enable in sbtrace.hxx
#ifdef DBG_TRACE_BASIC

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
            if( eType & SbxARRAY )

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
