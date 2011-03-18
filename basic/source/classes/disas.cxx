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

#include <stdio.h>
#include <string.h>
#include <tools/stream.hxx>
#include <basic/sbx.hxx>
#include "sb.hxx"
#include "iosys.hxx"
#include "disas.hxx"


static const char* pOp1[] = {
    "NOP",

    // Operators
    // the following operators have the same order as in
    // enum SbxVarOp
    "EXP", "MUL", "DIV", "MOD", "PLUS", "MINUS", "NEG",
    "EQ", "NE", "LT", "GT", "LE", "GE",
    "IDIV", "AND", "OR", "XOR", "EQV", "IMP", "NOT",
    "CAT",
    // End enum SbxVarOp
    "LIKE", "IS",
    // Load/Store
    "ARGC",             // Create new Argv
    "ARGV",             // TOS ==> current Argv
    "INPUT",            // Input ==> TOS
    "LINPUT",           // Line Input ==> TOS
    "GET",              // get TOS
    "SET",              // Save Object TOS ==> TOS-1
    "PUT",              // TOS ==> TOS-1
    "CONST",            // TOS ==> TOS-1, then ReadOnly
    "DIM",              // DIM
    "REDIM",            // REDIM
    "REDIMP",           // REDIM PRESERVE
    "ERASE",            // delete TOS
    // Branch
    "STOP",             // End of program
    "INITFOR",          // FOR-Variable init
    "NEXT",             // FOR-Variable increment
    "CASE",             // Begin CASE
    "ENDCASE",          // End CASE
    "STDERR",           // Default error handling
    "NOERROR",          // No error handling
    "LEAVE",            // leave UP
    // I/O
    "CHANNEL",          // TOS = Channelnumber
    "PRINT",            // print TOS
    "PRINTF",           // print TOS in field
    "WRITE",            // write TOS
    "RENAME",           // Rename Tos+1 to Tos
    "PROMPT",           // TOS = Prompt for Input
    "RESTART",          // Define restart point
    "STDIO",            // Switch to I/O channel 0
    // Misc
    "EMPTY",            // Empty statement to stack
    "ERROR",            // TOS = error code
    "LSET",             // Save object TOS ==> TOS-1
    "RSET",             // Save object TOS ==> TOS-1 (TODO: Same as above?)
    "REDIMP_ERASE",
    "INITFOREACH",
    "VBASET",
    "ERASE_CLEAR",
    "ARRAYACCESS",
    "BYVAL"
};

static const char* pOp2[] = {
    "NUMBER",            // Load a numeric constant (+ID)
    "STRING",            // Load a string constant (+ID)
    "CONSTANT",          // Immediate Load (+value)
    "ARGN",              // Save named args in argv (+StringID)
    "PAD",               // Pad String to defined length (+length)
    // Branches
    "JUMP",              // Jump to target (+Target)
    "JUMP.T",            // evaluate TOS, conditional jump (+Target)
    "JUMP.F",            // evaluate TOS, conditional jump (+Target)
    "ONJUMP",            // evaluate TOS, jump into JUMP-table (+MaxVal)
    "GOSUB",             // UP-Call (+Target)
    "RETURN",            // UP-Return (+0 or Target)
    "TESTFOR",           // Test FOR-Variable, increment (+Endlabel)
    "CASETO",            // Tos+1 <= Case <= Tos, 2xremove (+Target)
    "ERRHDL",            // Error-Handler (+Offset)
    "RESUME",            // Resume after errors (+0 or 1 or Label)
    // I/O
    "CLOSE",             // (+channel/0)
    "PRCHAR",            // (+char)
    // Objects
    "SETCLASS",          // Test Set + Classname (+StringId)
    "TESTCLASS",         // Check TOS class (+StringId)
    "LIB",               // Set Libname for Declare-Procs (+StringId)
    // New since Beta 3 (TODO: Which Beta3?)
    "BASED",             // TOS is incremted about BASE, push BASE before
    "ARGTYP",            // Convert last parameter in argv (+Type)
    "VBASETCLASS",
};

static const char* pOp3[] = {
    // All opcodes with two operands
    "RTL",              // Load from RTL (+StringID+Typ)
    "FIND",             // Load (+StringID+Typ)
    "ELEM",             // Load element (+StringID+Typ)
    "PARAM",            // Parameter (+Offset+Typ)

    // Branching
    "CALL",             // Call DECLARE method (+StringID+Typ)
    "CALL.C",           // Call Cdecl-DECLARE method (+StringID+Typ)
    "CASEIS",           // Case-Test (+Test-Opcode+False-Target)
    "STMNT",            // Start of a statement (+Line+Col)

    // I/O
    "OPEN",             // (+SvStreamFlags+Flags)

    // Objects and variables
    "LOCAL",            // Local variables (+StringID+Typ)
    "PUBLIC",           // Modul global var (+StringID+Typ)
    "GLOBAL",           // Global var (+StringID+Typ)
    "CREATE",           // Create object (+StringId+StringId)
    "STATIC",           // Create static object (+StringId+StringId)
    "TCREATE",          // Create User defined Object (+StringId+StringId)
    "DCREATE",          // Create User defined Object-Array kreieren (+StringId+StringId)
    "GLOBAL_P",         // Define persistent global var (existing after basic restart)
                        // P=PERSIST (+StringID+Typ)
    "FIND_G",           // Searches for global var with special handling due to _GLOBAL_P
    "DCREATE_REDIMP",   // Change dimensions of a user defined Object-Array (+StringId+StringId)
    "FIND_CM",          // Search inside a class module (CM) to enable global search in time
    "PUBLIC_P",         // Module global Variable (persisted between calls)(+StringID+Typ)
    "FIND_STATIC",      // local static var lookup (+StringID+Typ)
};

static const char** pOps[3] = { pOp1, pOp2, pOp3 };

typedef void( SbiDisas::*Func )( String& );  // Processing routines

static const Func pOperand2[] = {
    &SbiDisas::StrOp,   // Load a numeric constant (+ID)
    &SbiDisas::StrOp,   // Load a string constant (+ID)
    &SbiDisas::ImmOp,   // Immediate Load (+Wert)
    &SbiDisas::StrOp,   // Save a named argument (+ID)
    &SbiDisas::ImmOp,   // Strip String to fixed size (+length)

    // Branches
    &SbiDisas::LblOp,   // Jump (+Target)
    &SbiDisas::LblOp,   // eval TOS, conditional jump (+Target)
    &SbiDisas::LblOp,   // eval TOS, conditional jump (+Target)
    &SbiDisas::OnOp,    // eval TOS, jump in JUMP table (+MaxVal)
    &SbiDisas::LblOp,   // UP call (+Target)
    &SbiDisas::ReturnOp,    // UP Return (+0 or Target)
    &SbiDisas::LblOp,   // test FOR-Variable, increment (+Endlabel)
    &SbiDisas::LblOp,   // Tos+1 <= Case <= Tos), 2xremove (+Target)
    &SbiDisas::LblOp,   // Error handler (+Offset)
    &SbiDisas::ResumeOp,    // Resume after errors (+0 or 1 or Label)

    // I/O
    &SbiDisas::CloseOp, // (+channel/0)
    &SbiDisas::CharOp,  // (+char)

    // Objects
    &SbiDisas::StrOp,   // Test classname (+StringId)
    &SbiDisas::StrOp,   // TESTCLASS, Check TOS class (+StringId)
    &SbiDisas::StrOp,   // Set libname for declare procs (+StringId)
    &SbiDisas::ImmOp,   // TOS is incremented about BASE erhoeht, BASE pushed before
    &SbiDisas::TypeOp,  // Convert last parameter to/in(?) argv (+Typ)
    &SbiDisas::StrOp,   // VBASETCLASS (+StringId)
};

static const Func pOperand3[] = {
    // All opcodes with two operands
    &SbiDisas::VarOp,   // Load from RTL (+StringID+Typ)
    &SbiDisas::VarOp,   // Load (+StringID+Typ)
    &SbiDisas::VarOp,   // Load Element (+StringID+Typ)
    &SbiDisas::OffOp,   // Parameter (+Offset+Typ)

    // Branch
    &SbiDisas::VarOp,   // Call DECLARE-Method (+StringID+Typ)
    &SbiDisas::VarOp,   // Call CDecl-DECLARE-Methode (+StringID+Typ)
    &SbiDisas::CaseOp,  // Case-Test (+Test-Opcode+False-Target)
    &SbiDisas::StmntOp, // Statement (+Row+Column)

    // I/O
    &SbiDisas::StrmOp,  // (+SvStreamFlags+Flags)

    // Objects
    &SbiDisas::VarDefOp,   // Define local var (+StringID+Typ)
    &SbiDisas::VarDefOp,   // Define Module global var (+StringID+Typ)
    &SbiDisas::VarDefOp,   // Define global var (+StringID+Typ)
    &SbiDisas::Str2Op,     // Create object (+StringId+StringId)
    &SbiDisas::VarDefOp,   // Define static object (+StringID+Typ)
    &SbiDisas::Str2Op,     // Create User defined Object (+StringId+StringId)
    &SbiDisas::Str2Op,     // Create User defined Object-Array (+StringId+StringId)
    &SbiDisas::VarDefOp,   // Define persistent global var P=PERSIST (+StringID+Typ)
    &SbiDisas::VarOp,    // Searches for global var with special handling due to  _GLOBAL_P
    &SbiDisas::Str2Op,     // Redimensionate User defined Object-Array (+StringId+StringId)
    &SbiDisas::VarOp,    // FIND_CM
    &SbiDisas::VarDefOp, // PUBLIC_P
    &SbiDisas::VarOp,    // FIND_STATIC
};

// TODO: Why as method? Isn't a simple define sufficient?
static const char* _crlf()
{
#if defined (UNX) || defined( PM2 )
    return "\n";
#else
    return "\r\n";
#endif
}

// This method exists because we want to load the file as own segment
sal_Bool SbModule::Disassemble( String& rText )
{
    rText.Erase();
    if( pImage )
    {
        SbiDisas aDisas( this, pImage );
        aDisas.Disas( rText );
    }
    return sal_Bool( rText.Len() != 0 );
}

SbiDisas::SbiDisas( SbModule* p, const SbiImage* q ) : rImg( *q ), pMod( p )
{
    memset( cLabels, 0, 8192 );
    nLine = 0;
    nOff = 0;
    nPC = 0;
    nOp1 = nOp2 = nParts = 0;
    eOp = _NOP;
    // Set Label-Bits
    nOff = 0;
    while( Fetch() )
    {
        switch( eOp )
        {
            case _RESUME: if( nOp1 <= 1 ) break;
            case _RETURN: if( !nOp1 ) break;
            case _JUMP:
            case _JUMPT:
            case _JUMPF:
            case _GOSUB:
            case _TESTFOR:
            case _CASEIS:
            case _CASETO:
            case _ERRHDL:
                cLabels[ (nOp1 & 0xffff) >> 3 ] |= ( 1 << ( nOp1 & 7 ) );
                break;
            default: break;
        }
    }
    nOff = 0;
    // Add the publics
    for( sal_uInt16 i = 0; i < pMod->GetMethods()->Count(); i++ )
    {
        SbMethod* pMeth = PTR_CAST(SbMethod,pMod->GetMethods()->Get( i ));
        if( pMeth )
        {
            sal_uInt16 nPos = (sal_uInt16) (pMeth->GetId());
            cLabels[ nPos >> 3 ] |= ( 1 << ( nPos & 7 ) );
        }
    }
}

// Read current opcode
sal_Bool SbiDisas::Fetch()
{
    nPC = nOff;
    if( nOff >= rImg.GetCodeSize() )
        return sal_False;
    const unsigned char* p = (const unsigned char*)( rImg.GetCode() + nOff );
    eOp = (SbiOpcode) ( *p++ & 0xFF );
    if( eOp <= SbOP0_END )
    {
        nOp1 = nOp2 = 0;
        nParts = 1;
        nOff++;
        return sal_True;
    }
    else if( eOp <= SbOP1_END )
    {
        nOff += 5;
        if( nOff > rImg.GetCodeSize() )
            return sal_False;
        nOp1 = *p++; nOp1 |= *p++ << 8; nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
        nParts = 2;
        return sal_True;
    }
    else if( eOp <= SbOP2_END )
    {
        nOff += 9;
        if( nOff > rImg.GetCodeSize() )
            return sal_False;
        nOp1 = *p++; nOp1 |= *p++ << 8; nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
        nOp2 = *p++; nOp2 |= *p++ << 8; nOp2 |= *p++ << 16; nOp2 |= *p++ << 24;
        nParts = 3;
        return sal_True;
    }
    else
        return sal_False;
}

void SbiDisas::Disas( SvStream& r )
{
    String aText;
    nOff = 0;
    while( DisasLine( aText ) )
    {
        ByteString aByteText( aText, gsl_getSystemTextEncoding() );
        r.WriteLine( aByteText );
    }
}

void SbiDisas::Disas( String& r )
{
    r.Erase();
    String aText;
    nOff = 0;
    while( DisasLine( aText ) )
    {
        r += aText;
        r.AppendAscii( _crlf() );
    }
    aText.ConvertLineEnd();
}

sal_Bool SbiDisas::DisasLine( String& rText )
{
    char cBuf[ 100 ];
    const char* pMask[] = {
        "%08" SAL_PRIXUINT32 "                            ",
        "%08" SAL_PRIXUINT32 " %02X                   ",
        "%08" SAL_PRIXUINT32 " %02X %08X          ",
        "%08" SAL_PRIXUINT32 " %02X %08X %08X " };
    rText.Erase();
    if( !Fetch() )
        return sal_False;

    // New line?
    if( eOp == _STMNT && nOp1 != nLine )
    {
        // Find line
        String aSource = rImg.aOUSource;
        nLine = nOp1;
        sal_uInt16 n = 0;
        sal_uInt16 l = (sal_uInt16)nLine;
        while( --l ) {
            n = aSource.SearchAscii( "\n", n );
            if( n == STRING_NOTFOUND ) break;
            else n++;
        }
        // Show position
        if( n != STRING_NOTFOUND )
        {
            sal_uInt16 n2 = aSource.SearchAscii( "\n", n );
            if( n2 == STRING_NOTFOUND ) n2 = aSource.Len() - n;
            String s( aSource.Copy( n, n2 - n + 1 ) );
            sal_Bool bDone;
            do {
                bDone = sal_True;
                n = s.Search( '\r' );
                if( n != STRING_NOTFOUND ) bDone = sal_False, s.Erase( n, 1 );
                n = s.Search( '\n' );
                if( n != STRING_NOTFOUND ) bDone = sal_False, s.Erase( n, 1 );
            } while( !bDone );
//          snprintf( cBuf, sizeof(cBuf), pMask[ 0 ], nPC );
//          rText += cBuf;
            rText.AppendAscii( "; " );
            rText += s;
            rText.AppendAscii( _crlf() );
        }
    }

    // Label?
    const char* p = "";
    if( cLabels[ nPC >> 3 ] & ( 1 << ( nPC & 7 ) ) )
    {
        // Public?
        ByteString aByteMethName;
        for( sal_uInt16 i = 0; i < pMod->GetMethods()->Count(); i++ )
        {
            SbMethod* pMeth = PTR_CAST(SbMethod,pMod->GetMethods()->Get( i ));
            if( pMeth )
            {
                aByteMethName = ByteString( pMeth->GetName(), gsl_getSystemTextEncoding() );
                if( pMeth->GetId() == nPC )
                {
                    p = aByteMethName.GetBuffer();
                    break;
                }
                if( pMeth->GetId() >= nPC )
                    break;
            }
        }
        snprintf( cBuf, sizeof(cBuf), pMask[ 0 ], nPC );
        rText.AppendAscii( cBuf );
        if( p && *p )
        {
            rText.AppendAscii( p );
        }
        else
        {
            // fix warning (now error) for "Lbl%04lX" format
              snprintf( cBuf, sizeof(cBuf), "Lbl%08" SAL_PRIXUINT32, nPC );
            rText.AppendAscii( cBuf );
        }
        rText += ':';
        rText.AppendAscii( _crlf() );
    }
    snprintf( cBuf, sizeof(cBuf), pMask[ nParts ], nPC, (sal_uInt16) eOp, nOp1, nOp2 );

    String aPCodeStr;
    aPCodeStr.AppendAscii( cBuf );
    int n = eOp;
    if( eOp >= SbOP2_START )
        n -= SbOP2_START;
    else if( eOp >= SbOP1_START )
        n -= SbOP1_START;
    aPCodeStr += '\t';
    aPCodeStr.AppendAscii( pOps[ nParts-1 ][ n ] );
    aPCodeStr += '\t';
    switch( nParts )
    {
        case 2: (this->*( pOperand2[ n ] ) )( aPCodeStr ); break;
        case 3: (this->*( pOperand3[ n ] ) )( aPCodeStr ); break;
    }

    rText += aPCodeStr;

    return sal_True;
}

// Read from StringPool
void SbiDisas::StrOp( String& rText )
{
    String aStr = rImg.GetString( (sal_uInt16)nOp1 );
    ByteString aByteString( aStr, RTL_TEXTENCODING_ASCII_US );
    const char* p = aByteString.GetBuffer();
    if( p )
    {
        rText += '"';
        rText.AppendAscii( p );
        rText += '"';
    }
    else
    {
        rText.AppendAscii( "?String? " );
        rText += (sal_uInt16)nOp1;
    }
}

void SbiDisas::Str2Op( String& rText )
{
    StrOp( rText );
    rText += ',';
    String s;
    nOp1 = nOp2;
    StrOp( s );
    rText += s;
}

// Immediate Operand
void SbiDisas::ImmOp( String& rText )
{
    rText += String::CreateFromInt32(nOp1);
}

// OnGoto Operand
void SbiDisas::OnOp( String& rText )
{
    rText += String::CreateFromInt32(nOp1 & 0x7FFF);
    if( nOp1 & 0x800 )
        rText.AppendAscii( "\t; Gosub" );
}

// Label
void SbiDisas::LblOp( String& rText )
{
    char cBuf[ 10 ];
    snprintf( cBuf, sizeof(cBuf), "Lbl%04" SAL_PRIXUINT32, nOp1 );
    rText.AppendAscii( cBuf );
}

// 0 or Label
void SbiDisas::ReturnOp( String& rText )
{
    if( nOp1 )
        LblOp( rText );
}

// 0, 1 or Label
void SbiDisas::ResumeOp( String& rText )
{
    switch( nOp1 )
    {
        case 1: rText.AppendAscii( "NEXT" ); break;
        case 2: LblOp( rText );
    }
}

// print Prompt
// sal_False/TRUE
void SbiDisas::PromptOp( String& rText )
{
    if( nOp1 )
        rText.AppendAscii( "\"? \"" );
}

// 0 or 1
void SbiDisas::CloseOp( String& rText )
{
    rText.AppendAscii( nOp1 ? "Channel" : "All" );
}

// Print character
void SbiDisas::CharOp( String& rText )
{
    const char* p = NULL;
    switch( nOp1 )
    {
        case  7: p = "'\\a'"; break;
        case  9: p = "'\\t'"; break;
        case 10: p = "'\\n'"; break;
        case 12: p = "'\\f'"; break;
        case 13: p = "'\\r'"; break;
    }
    if( p ) rText.AppendAscii( p );
    else if( nOp1 >= ' ' )
        rText += '\'',
        rText += (char) nOp1,
        rText += '\'';
    else
        rText.AppendAscii( "char " ),
        rText += (sal_uInt16)nOp1;
}

// Print var: String-ID and type
void SbiDisas::VarOp( String& rText )
{
    rText += rImg.GetString( (sal_uInt16)(nOp1 & 0x7FFF) );
    rText.AppendAscii( "\t; " );
    // The type
    sal_uInt32 n = nOp1;
    nOp1 = nOp2;
    TypeOp( rText );
    if( n & 0x8000 )
        rText.AppendAscii( ", Args" );
}

// Define variable: String-ID and type
void SbiDisas::VarDefOp( String& rText )
{
    rText += rImg.GetString( (sal_uInt16)(nOp1 & 0x7FFF) );
    rText.AppendAscii( "\t; " );
    // The Typ
    nOp1 = nOp2;
    TypeOp( rText );
}

// Print variable: Offset and Typ
void SbiDisas::OffOp( String& rText )
{
    rText += String::CreateFromInt32( nOp1 & 0x7FFF );
    rText.AppendAscii( "\t; " );
    // The type
    sal_uInt32 n = nOp1;
    nOp1 = nOp2;
    TypeOp( rText );
    if( n & 0x8000 )
        rText.AppendAscii( ", Args" );
}

// Data type
void SbiDisas::TypeOp( String& rText )
{
    // From 1996-01-19: type can contain flag for BYVAL (StepARGTYP)
    if( nOp1 & 0x8000 )
    {
        nOp1 &= 0x7FFF;     // filter away the flag
        rText.AppendAscii( "BYVAL " );
    }
    if( nOp1 < 13 )
    {
        static char pTypes[][13] = {
            "Empty","Null","Integer","Long","Single","Double",
            "Currency","Date","String","Object","Error","Boolean",
            "Variant" };

        rText.AppendAscii( pTypes[ nOp1 ] );
    }
    else
    {
        rText.AppendAscii( "type " );
        rText += (sal_uInt16)nOp1;
    }
}

// sal_True-Label, condition Opcode
void SbiDisas::CaseOp( String& rText )
{
    LblOp( rText );
    rText += ',';
    rText.AppendAscii( pOp1[ nOp2 - SbxEQ + _EQ ] );
}

// Row, column
void SbiDisas::StmntOp( String& rText )
{
    rText += String::CreateFromInt32( nOp1 );
    rText += ',';
    sal_uInt32 nCol = nOp2 & 0xFF;
    sal_uInt32 nFor = nOp2 / 0x100;
    rText += String::CreateFromInt32( nCol );
    rText.AppendAscii( " (For-Level: " );
    rText += String::CreateFromInt32( nFor );
    rText += ')';
}

// open mode, flags
void SbiDisas::StrmOp( String& rText )
{
    char cBuf[ 10 ];
    snprintf( cBuf, sizeof(cBuf), "%04" SAL_PRIXUINT32, nOp1 );
    rText.AppendAscii( cBuf );
    if( nOp2 & SBSTRM_INPUT )
        rText.AppendAscii( ", Input" );
    if( nOp2 & SBSTRM_OUTPUT )
        rText.AppendAscii( ", Output" );
    if( nOp2 & SBSTRM_APPEND )
        rText.AppendAscii( ", Append" );
    if( nOp2 & SBSTRM_RANDOM )
        rText.AppendAscii( ", Random" );
    if( nOp2 & SBSTRM_BINARY )
        rText.AppendAscii( ", Binary" );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
