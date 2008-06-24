/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: disas.cxx,v $
 * $Revision: 1.28 $
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

    // Operatoren
    // die folgenden Operatoren sind genauso angeordnet
    // wie der enum SbxVarOp
    "EXP", "MUL", "DIV", "MOD", "PLUS", "MINUS", "NEG",
    "EQ", "NE", "LT", "GT", "LE", "GE",
    "IDIV", "AND", "OR", "XOR", "EQV", "IMP", "NOT",
    "CAT",
    // Ende enum SbxVarOp
    "LIKE", "IS",
    // Laden/speichern
    "ARGC",             // neuen Argv einrichten
    "ARGV",             // TOS ==> aktueller Argv
    "INPUT",            // Input ==> TOS
    "LINPUT",           // Line Input ==> TOS
    "GET",              // TOS anfassen
    "SET",              // Speichern Objekt TOS ==> TOS-1
    "PUT",              // TOS ==> TOS-1
    "CONST",            // TOS ==> TOS-1, dann ReadOnly
    "DIM",              // DIM
    "REDIM",            // REDIM
    "REDIMP",           // REDIM PRESERVE
    "ERASE",            // TOS loeschen
    // Verzweigen
    "STOP",             // Programmende
    "INITFOR",          // FOR-Variable initialisieren
    "NEXT",             // FOR-Variable inkrementieren
    "CASE",             // Anfang CASE
    "ENDCASE",          // Ende CASE
    "STDERR",           // Standard-Fehlerbehandlung
    "NOERROR",          // keine Fehlerbehandlung
    "LEAVE",            // UP verlassen
    // E/A
    "CHANNEL",          // TOS = Kanalnummer
    "PRINT",            // print TOS
    "PRINTF",           // print TOS in field
    "WRITE",            // write TOS
    "RENAME",           // Rename Tos+1 to Tos
    "PROMPT",           // TOS = Prompt for Input
    "RESTART",          // Restartpunkt definieren
    "STDIO",            // E/A-Kanal 0 einstellen
    // Sonstiges
    "EMPTY",            // Leeren Ausdruck auf Stack
    "ERROR",            // TOS = Fehlercode
    "LSET",             // Speichern Objekt TOS ==> TOS-1
    "RSET",             // Speichern Objekt TOS ==> TOS-1
    "REDIMP_ERASE",
    "INITFOREACH",
    "VBASET",
    "ERASE_CLEAR",
    "ARRAYACCESS"
};

static const char* pOp2[] = {
    "NUMBER",           // Laden einer numerischen Konstanten (+ID)
    "STRING",           // Laden einer Stringkonstanten (+ID)
    "CONSTANT",         // Immediate Load (+Wert)
    "ARGN",             // Speichern eines named Args in Argv (+StringID)
    "PAD",              // String auf feste Laenge bringen (+Laenge)
    // Verzweigungen
    "JUMP",             // Sprung (+Target)
    "JUMP.T",           // TOS auswerten, bedingter Sprung (+Target)
    "JUMP.F",           // TOS auswerten, bedingter Sprung (+Target)
    "ONJUMP",           // TOS auswerten, Sprung in JUMP-Tabelle (+MaxVal)
    "GOSUB",            // UP-Aufruf (+Target)
    "RETURN",           // UP-Return (+0 oder Target)
    "TESTFOR",          // FOR-Variable testen, inkrementieren (+Endlabel)
    "CASETO",           // Tos+1 <= Case <= Tos, 2xremove (+Target)
    "ERRHDL",           // Fehler-Handler (+Offset)
    "RESUME",           // Resume nach Fehlern (+0 or 1 or Label)
    // E/A
    "CLOSE",            // (+Kanal/0)
    "PRCHAR",           // (+char)
    // Objekte
    "SETCLASS",         // Set + Klassennamen testen (+StringId)
    "TESTCLASS",        // Check TOS class (+StringId)
    "LIB",              // Libnamen fuer Declare-Procs setzen (+StringId)
    // Neues ab Beta 3
    "BASED",            // TOS wird um BASE erhoeht, BASE davor gepusht
    "ARGTYP",           // Letzten Parameter in Argv konvertieren (+Typ)
    "VBASETCLASS",
};

static const char* pOp3[] = {
    // Alle Opcodes mit zwei Operanden
    "RTL",              // Laden aus RTL (+StringID+Typ)
    "FIND",             // Laden (+StringID+Typ)
    "ELEM",             // Laden Element (+StringID+Typ)
    "PARAM",            // Parameter (+Offset+Typ)
    // Verzweigen
    "CALL",             // DECLARE-Methode rufen (+StringID+Typ)
    "CALL.C",           // Cdecl-DECLARE-Methode rufen (+StringID+Typ)
    "CASEIS",           // Case-Test (+Test-Opcode+False-Target)
    "STMNT",            // Beginn eines Statements (+Line+Col)
    // E/A
    "OPEN",             // (+SvStreamFlags+Flags)
    // Objekte und Variable
    "LOCAL",            // Lokale Variable (+StringID+Typ)
    "PUBLIC",           // Modulglobale Variable (+StringID+Typ)
    "GLOBAL",           // Globale Variable (+StringID+Typ)
    "CREATE",           // Objekt kreieren (+StringId+StringId)
    "STATIC",           // Objekt kreieren (+StringId+StringId)
    "TCREATE",          // User defined Objekt kreieren (+StringId+StringId)
    "DCREATE",          // User defined Objekt-Array kreieren (+StringId+StringId)
    "GLOBAL_P",         // Globale Variable definieren, die beim Neustart von Basic
                        // nicht ueberschrieben wird, P=PERSIST (+StringID+Typ)
    "FIND_G",           // Sucht globale Variable mit Spezialbehandlung wegen _GLOBAL_P
    "DCREATE_REDIMP",   // User defined Objekt-Array redimensionieren (+StringId+StringId)
    "FIND_CM",          // Search inside a class module (CM) to enable global search in time
    "PUBLIC_P",         // Module global Variable (persisted between calls)(+StringID+Typ)
};

static const char** pOps[3] = { pOp1, pOp2, pOp3 };

typedef void( SbiDisas::*Func )( String& );     // Verarbeitungsroutine

static const Func pOperand2[] = {
    &SbiDisas::StrOp,   // Laden einer numerischen Konstanten (+ID)
    &SbiDisas::StrOp,   // Laden einer Stringkonstanten (+ID)
    &SbiDisas::ImmOp,   // Immediate Load (+Wert)
    &SbiDisas::StrOp,   // Speichern eines benannten Arguments(+ID)
    &SbiDisas::ImmOp,   // String auf feste Laenge bringen (+Laenge)
    // Verzweigungen
    &SbiDisas::LblOp,   // Sprung (+Target)
    &SbiDisas::LblOp,   // TOS auswerten), bedingter Sprung (+Target)
    &SbiDisas::LblOp,   // TOS auswerten), bedingter Sprung (+Target)
    &SbiDisas::OnOp,        // TOS auswerten), Sprung in JUMP-Tabelle (+MaxVal)
    &SbiDisas::LblOp,   // UP-Aufruf (+Target)
    &SbiDisas::ReturnOp,    // UP-Return (+0 oder Target)
    &SbiDisas::LblOp,   // FOR-Variable testen), inkrementieren (+Endlabel)
    &SbiDisas::LblOp,   // Tos+1 <= Case <= Tos), 2xremove (+Target)
    &SbiDisas::LblOp,   // Fehler-Handler (+Offset)
    &SbiDisas::ResumeOp,    // Resume nach Fehlern (+0 or 1 or Label)
    // E/A
    &SbiDisas::CloseOp, // (+Kanal/0)
    &SbiDisas::CharOp,  // (+char)
    // Objekte
    &SbiDisas::StrOp,   // Klassennamen testen (+StringId)
    &SbiDisas::StrOp,   // TESTCLASS, Check TOS class (+StringId)
    &SbiDisas::StrOp,   // Libnamen fuer Declare-Procs setzen (+StringId)
    &SbiDisas::ImmOp,   // TOS wird um BASE erhoeht, BASE davor gepusht
    &SbiDisas::TypeOp,  // Letzten Parameter in Argv konvertieren (+Typ)
    &SbiDisas::StrOp,   // VBASETCLASS (+StringId)
};

static const Func pOperand3[] = {
    // Alle Opcodes mit zwei Operanden
    &SbiDisas::VarOp,   // Laden aus RTL (+StringID+Typ)
    &SbiDisas::VarOp,   // Laden (+StringID+Typ)
    &SbiDisas::VarOp,   // Laden Element (+StringID+Typ)
    &SbiDisas::OffOp,   // Parameter (+Offset+Typ)
    // Verzweigen
    &SbiDisas::VarOp,   // DECLARE-Methode rufen (+StringID+Typ)
    &SbiDisas::VarOp,   // CDecl-DECLARE-Methode rufen (+StringID+Typ)
    &SbiDisas::CaseOp,  // Case-Test (+Test-Opcode+False-Target)
    &SbiDisas::StmntOp, // Statement (+Zeilen+Spalte)
    // E/A
    &SbiDisas::StrmOp,  // (+SvStreamFlags+Flags)
    // Objekte
    &SbiDisas::VarDefOp, // Lokale Variable definieren (+StringID+Typ)
    &SbiDisas::VarDefOp, // Modulglobale Variable definieren (+StringID+Typ)
    &SbiDisas::VarDefOp, // Globale Variable definieren (+StringID+Typ)
    &SbiDisas::Str2Op,   // Objekt kreieren (+StringId+StringId)
    &SbiDisas::VarDefOp, // Statische Variable definieren (+StringID+Typ)
    &SbiDisas::Str2Op,   // User defined Objekt kreieren (+StringId+StringId)
    &SbiDisas::Str2Op,   // User defined Objekt-Array kreieren (+StringId+StringId)
    &SbiDisas::VarDefOp, // Globale Variable definieren, die beim Neustart von Basic
                                // nicht ueberschrieben wird, P=PERSIST (+StringID+Typ)
    &SbiDisas::VarOp,    // Sucht globale Variable mit Spezialbehandlung wegen _GLOBAL_P
    &SbiDisas::Str2Op,   // User defined Objekt-Array redimensionieren (+StringId+StringId)
    &SbiDisas::VarOp,    // FIND_CM
    &SbiDisas::VarDefOp, // PUBLIC_P
};


static const char* _crlf()
{
#if defined (UNX) || defined( PM2 )
    return "\n";
#else
    return "\r\n";
#endif
}

// Diese Methode ist hier, damit die Datei als eigenes Segment geladen werden
// kann.

BOOL SbModule::Disassemble( String& rText )
{
    rText.Erase();
    if( pImage )
    {
        SbiDisas aDisas( this, pImage );
        aDisas.Disas( rText );
    }
    return BOOL( rText.Len() != 0 );
}

SbiDisas::SbiDisas( SbModule* p, const SbiImage* q ) : rImg( *q ), pMod( p )
{
    memset( cLabels, 0, 8192 );
    nLine = 0;
    nOff = 0;
    nPC = 0;
    nOp1 = nOp2 = nParts = 0;
    eOp = _NOP;
    // Label-Bits setzen
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
    // Die Publics noch dazu
    for( USHORT i = 0; i < pMod->GetMethods()->Count(); i++ )
    {
        SbMethod* pMeth = PTR_CAST(SbMethod,pMod->GetMethods()->Get( i ));
        if( pMeth )
        {
            USHORT nPos = (USHORT) (pMeth->GetId());
            cLabels[ nPos >> 3 ] |= ( 1 << ( nPos & 7 ) );
        }
    }
}

// Aktuellen Opcode auslesen

BOOL SbiDisas::Fetch()
{
    nPC = nOff;
    if( nOff >= rImg.GetCodeSize() )
        return FALSE;
    const unsigned char* p = (const unsigned char*)( rImg.GetCode() + nOff );
    eOp = (SbiOpcode) ( *p++ & 0xFF );
    if( eOp <= SbOP0_END )
    {
        nOp1 = nOp2 = 0;
        nParts = 1;
        nOff++;
        return TRUE;
    }
    else if( eOp <= SbOP1_END )
    {
        nOff += 5;
        if( nOff > rImg.GetCodeSize() )
            return FALSE;
        nOp1 = *p++; nOp1 |= *p++ << 8; nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
        nParts = 2;
        return TRUE;
    }
    else if( eOp <= SbOP2_END )
    {
        nOff += 9;
        if( nOff > rImg.GetCodeSize() )
            return FALSE;
        nOp1 = *p++; nOp1 |= *p++ << 8; nOp1 |= *p++ << 16; nOp1 |= *p++ << 24;
        nOp2 = *p++; nOp2 |= *p++ << 8; nOp2 |= *p++ << 16; nOp2 |= *p++ << 24;
        nParts = 3;
        return TRUE;
    }
    else
        return FALSE;
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

BOOL SbiDisas::DisasLine( String& rText )
{
    char cBuf[ 100 ];
    const char* pMask[] = {
        "%08" SAL_PRIXUINT32 "                            ",
        "%08" SAL_PRIXUINT32 " %02X                   ",
        "%08" SAL_PRIXUINT32 " %02X %08X          ",
        "%08" SAL_PRIXUINT32 " %02X %08X %08X " };
    rText.Erase();
    if( !Fetch() )
        return FALSE;
    // Neue Zeile?
    if( eOp == _STMNT && nOp1 != nLine )
    {
        // Zeile raussuchen
        String aSource = rImg.aOUSource;
        nLine = nOp1;
        USHORT n = 0;
        USHORT l = (USHORT)nLine;
        while( --l ) {
            n = aSource.SearchAscii( "\n", n );
            if( n == STRING_NOTFOUND ) break;
            else n++;
        }
        // Stelle anzeigen
        if( n != STRING_NOTFOUND )
        {
            USHORT n2 = aSource.SearchAscii( "\n", n );
            if( n2 == STRING_NOTFOUND ) n2 = aSource.Len() - n;
            String s( aSource.Copy( n, n2 - n + 1 ) );
            BOOL bDone;
            do {
                bDone = TRUE;
                n = s.Search( '\r' );
                if( n != STRING_NOTFOUND ) bDone = FALSE, s.Erase( n, 1 );
                n = s.Search( '\n' );
                if( n != STRING_NOTFOUND ) bDone = FALSE, s.Erase( n, 1 );
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
        for( USHORT i = 0; i < pMod->GetMethods()->Count(); i++ )
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
    snprintf( cBuf, sizeof(cBuf), pMask[ nParts ], nPC, (USHORT) eOp, nOp1, nOp2 );
    rText.AppendAscii( cBuf );
    int n = eOp;
    if( eOp >= SbOP2_START )
        n -= SbOP2_START;
    else if( eOp >= SbOP1_START )
        n -= SbOP1_START;
    rText += '\t';
    rText.AppendAscii( pOps[ nParts-1 ][ n ] );
    rText += '\t';
    switch( nParts )
    {
        case 2: (this->*( pOperand2[ n ] ) )( rText ); break;
        case 3: (this->*( pOperand3[ n ] ) )( rText ); break;
    }
    return TRUE;
}


// Auslesen aus StringPool

void SbiDisas::StrOp( String& rText )
{
    String aStr = rImg.GetString( (USHORT)nOp1 );
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
        rText += (USHORT)nOp1;
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

// 0 oder Label

void SbiDisas::ReturnOp( String& rText )
{
    if( nOp1 )
        LblOp( rText );
}

// 0, 1 oder Label

void SbiDisas::ResumeOp( String& rText )
{
    switch( nOp1 )
    {
        case 1: rText.AppendAscii( "NEXT" ); break;
        case 2: LblOp( rText );
    }
}

// Prompt ausgeben
// FALSE/TRUE

void SbiDisas::PromptOp( String& rText )
{
    if( nOp1 )
        rText.AppendAscii( "\"? \"" );
}

// 0 oder 1

void SbiDisas::CloseOp( String& rText )
{
    rText.AppendAscii( nOp1 ? "Channel" : "All" );
}

// Zeichen ausgeben

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
        rText += (USHORT)nOp1;
}

// Variable ausgeben: String-ID und Typ

void SbiDisas::VarOp( String& rText )
{
    rText += rImg.GetString( (USHORT)(nOp1 & 0x7FFF) );
    rText.AppendAscii( "\t; " );
    // Der Typ
    UINT32 n = nOp1;
    nOp1 = nOp2;
    TypeOp( rText );
    if( n & 0x8000 )
        rText.AppendAscii( ", Args" );
}

// Variable definieren: String-ID und Typ

void SbiDisas::VarDefOp( String& rText )
{
    rText += rImg.GetString( (USHORT)(nOp1 & 0x7FFF) );
    rText.AppendAscii( "\t; " );
    // Der Typ
    nOp1 = nOp2;
    TypeOp( rText );
}

// Variable ausgeben: Offset und Typ

void SbiDisas::OffOp( String& rText )
{
    rText += String::CreateFromInt32( nOp1 & 0x7FFF );
    rText.AppendAscii( "\t; " );
    // Der Typ
    UINT32 n = nOp1;
    nOp1 = nOp2;
    TypeOp( rText );
    if( n & 0x8000 )
        rText.AppendAscii( ", Args" );
}

// Datentyp
#ifdef HP9000
static char* SbiDisas_TypeOp_pTypes[13] = {
    "Empty","Null","Integer","Long","Single","Double",
    "Currency","Date","String","Object","Error","Boolean",
    "Variant" };
#define pTypes SbiDisas_TypeOp_pTypes
#endif
void SbiDisas::TypeOp( String& rText )
{
    // AB 19.1.96: Typ kann Flag für BYVAL enthalten (StepARGTYP)
    if( nOp1 & 0x8000 )
    {
        nOp1 &= 0x7FFF;     // Flag wegfiltern
        rText.AppendAscii( "BYVAL " );
    }
    if( nOp1 < 13 )
    {
#ifndef HP9000
        static char pTypes[][13] = {
            "Empty","Null","Integer","Long","Single","Double",
            "Currency","Date","String","Object","Error","Boolean",
            "Variant" };
#endif
        rText.AppendAscii( pTypes[ nOp1 ] );
    }
    else
    {
        rText.AppendAscii( "type " );
        rText += (USHORT)nOp1;
    }
}
#ifdef HP9000
#undef pTypes
#endif

// TRUE-Label, Bedingungs-Opcode

void SbiDisas::CaseOp( String& rText )
{
    LblOp( rText );
    rText += ',';
    rText.AppendAscii( pOp1[ nOp2 - SbxEQ + _EQ ] );
}

// Zeile, Spalte

void SbiDisas::StmntOp( String& rText )
{
    rText += String::CreateFromInt32( nOp1 );
    rText += ',';
    UINT32 nCol = nOp2 & 0xFF;
    UINT32 nFor = nOp2 / 0x100;
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


