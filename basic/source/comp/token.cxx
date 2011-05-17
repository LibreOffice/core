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

#include <ctype.h>
#include "sbcomp.hxx"

struct TokenTable { SbiToken t; const char *s; };

static short nToken;                    // Anzahl der Tokens

static TokenTable* pTokTable;

static TokenTable aTokTable_Basic [] = {        // Token-Tabelle:

    { CAT,      "&" },
    { MUL,      "*" },
    { PLUS,     "+" },
    { MINUS,    "-" },
    { DIV,      "/" },
    { EOS,      ":" },
    { ASSIGN,   ":=" },
    { LT,       "<" },
    { LE,       "<=" },
    { NE,       "<>" },
    { EQ,       "=" },
    { GT,       ">" },
    { GE,       ">=" },
    { ACCESS,   "Access" },
    { ALIAS,    "Alias" },
    { AND,      "And" },
    { ANY,      "Any" },
    { APPEND,   "Append" },
    { AS,       "As" },
    { ATTRIBUTE,"Attribute" },
    { BASE,     "Base" },
    { BINARY,   "Binary" },
    { TBOOLEAN, "Boolean" },
    { BYREF,    "ByRef", },
    { TBYTE,    "Byte", },
    { BYVAL,    "ByVal", },
    { CALL,     "Call" },
    { CASE,     "Case" },
    { _CDECL_,  "Cdecl" },
    { CLASSMODULE, "ClassModule" },
    { CLOSE,    "Close" },
    { COMPARE,  "Compare" },
    { COMPATIBLE,"Compatible" },
    { _CONST_,  "Const" },
    { TCURRENCY,"Currency" },
    { TDATE,    "Date" },
    { DECLARE,  "Declare" },
    { DEFBOOL,  "DefBool" },
    { DEFCUR,   "DefCur" },
    { DEFDATE,  "DefDate" },
    { DEFDBL,   "DefDbl" },
    { DEFERR,   "DefErr" },
    { DEFINT,   "DefInt" },
    { DEFLNG,   "DefLng" },
    { DEFOBJ,   "DefObj" },
    { DEFSNG,   "DefSng" },
    { DEFSTR,   "DefStr" },
    { DEFVAR,   "DefVar" },
    { DIM,      "Dim" },
    { DO,       "Do" },
    { TDOUBLE,  "Double" },
    { EACH,     "Each" },
    { ELSE,     "Else" },
    { ELSEIF,   "ElseIf" },
    { END,      "End" },
    { ENDENUM,  "End Enum" },
    { ENDFUNC,  "End Function" },
    { ENDIF,    "End If" },
    { ENDPROPERTY, "End Property" },
    { ENDSELECT,"End Select" },
    { ENDSUB,   "End Sub" },
    { ENDTYPE,  "End Type" },
    { ENDIF,    "EndIf" },
    { ENUM,     "Enum" },
    { EQV,      "Eqv" },
    { ERASE,    "Erase" },
    { _ERROR_,  "Error" },
    { EXIT,     "Exit" },
    { EXPLICIT, "Explicit" },
    { FOR,      "For" },
    { FUNCTION, "Function" },
    { GET,      "Get" },
    { GLOBAL,   "Global" },
    { GOSUB,    "GoSub" },
    { GOTO,     "GoTo" },
    { IF,       "If" },
    { IMP,      "Imp" },
    { IMPLEMENTS, "Implements" },
    { _IN_,     "In" },
    { INPUT,    "Input" },              // auch INPUT #
    { TINTEGER, "Integer" },
    { IS,       "Is" },
    { LET,      "Let" },
    { LIB,      "Lib" },
    { LIKE,     "Like" },
    { LINE,     "Line" },
    { LINEINPUT,"Line Input" },
    { LOCAL,    "Local" },
    { LOCK,     "Lock" },
    { TLONG,    "Long" },
    { LOOP,     "Loop" },
    { LPRINT,   "LPrint" },
    { LSET,     "LSet" }, // JSM
    { MOD,      "Mod" },
    { NAME,     "Name" },
    { NEW,      "New" },
    { NEXT,     "Next" },
    { NOT,      "Not" },
    { TOBJECT,  "Object" },
    { ON,       "On" },
    { OPEN,     "Open" },
    { OPTION,   "Option" },
    { _OPTIONAL_,   "Optional" },
    { OR,       "Or" },
    { OUTPUT,   "Output" },
    { PARAMARRAY,   "ParamArray" },
    { PRESERVE, "Preserve" },
    { PRINT,    "Print" },
    { PRIVATE,  "Private" },
    { PROPERTY, "Property" },
    { PUBLIC,   "Public" },
    { RANDOM,   "Random" },
    { READ,     "Read" },
    { REDIM,    "ReDim" },
    { REM,      "Rem" },
    { RESUME,   "Resume" },
    { RETURN,   "Return" },
    { RSET,     "RSet" }, // JSM
    { SELECT,   "Select" },
    { SET,      "Set" },
#ifdef SHARED
#undef SHARED
#define tmpSHARED
#endif
    { SHARED,   "Shared" },
#ifdef tmpSHARED
#define SHARED
#undef tmpSHARED
#endif
    { TSINGLE,  "Single" },
    { STATIC,   "Static" },
    { STEP,     "Step" },
    { STOP,     "Stop" },
    { TSTRING,  "String" },
    { SUB,      "Sub" },
    { STOP,     "System" },
    { TEXT,     "Text" },
    { THEN,     "Then" },
    { TO,       "To", },
    { TYPE,     "Type" },
    { TYPEOF,   "TypeOf" },
    { UNTIL,    "Until" },
    { TVARIANT, "Variant" },
    { VBASUPPORT,   "VbaSupport" },
    { WEND,     "Wend" },
    { WHILE,    "While" },
    { WITH,     "With" },
    { WITHEVENTS,   "WithEvents" },
    { WRITE,    "Write" },              // auch WRITE #
    { XOR,      "Xor" },
    { NIL,      "" }
};


// #i109076
TokenLabelInfo::TokenLabelInfo( void )
{
    m_pTokenCanBeLabelTab = new bool[VBASUPPORT+1];
    for( int i = 0 ; i <= VBASUPPORT ; ++i )
        m_pTokenCanBeLabelTab[i] = false;

    // Token accepted as label by VBA
    SbiToken eLabelToken[] = { ACCESS, ALIAS, APPEND, BASE, BINARY, CLASSMODULE,
        COMPARE, COMPATIBLE, DEFERR, _ERROR_, EXPLICIT, LIB, LINE, LPRINT, NAME,
        TOBJECT, OUTPUT, PROPERTY, RANDOM, READ, STEP, STOP, TEXT, VBASUPPORT, NIL };
    SbiToken* pTok = eLabelToken;
    SbiToken eTok;
    for( pTok = eLabelToken ; (eTok = *pTok) != NIL ; ++pTok )
        m_pTokenCanBeLabelTab[eTok] = true;
}

TokenLabelInfo::~TokenLabelInfo()
{
    delete[] m_pTokenCanBeLabelTab;
}


// Der Konstruktor ermittelt die Laenge der Token-Tabelle.

SbiTokenizer::SbiTokenizer( const ::rtl::OUString& rSrc, StarBASIC* pb )
           : SbiScanner( rSrc, pb )
{
    pTokTable = aTokTable_Basic;
    TokenTable *tp;
    bEof = bAs = sal_False;
    eCurTok = NIL;
    ePush = NIL;
    bEos = bKeywords = bErrorIsSymbol = sal_True;
    if( !nToken )
        for( nToken = 0, tp = pTokTable; tp->t; nToken++, tp++ ) {}
}

SbiTokenizer::~SbiTokenizer()
{
}

// Wiederablage (Pushback) eines Tokens. (Bis zu 2 Tokens)

void SbiTokenizer::Push( SbiToken t )
{
    if( ePush != NIL )
        Error( SbERR_INTERNAL_ERROR, "PUSH" );
    else ePush = t;
}

void SbiTokenizer::Error( SbError code, const char* pMsg )
{
    aError = String::CreateFromAscii( pMsg );
    Error( code );
}

void SbiTokenizer::Error( SbError code, String aMsg )
{
    aError = aMsg;
    Error( code );
}

void SbiTokenizer::Error( SbError code, SbiToken tok )
{
    aError = Symbol( tok );
    Error( code );
}

// Einlesen des naechsten Tokens, ohne dass das Token geschluckt wird

SbiToken SbiTokenizer::Peek()
{
    if( ePush == NIL )
    {
        sal_uInt16 nOldLine = nLine;
        sal_uInt16 nOldCol1 = nCol1;
        sal_uInt16 nOldCol2 = nCol2;
        ePush = Next();
        nPLine = nLine; nLine = nOldLine;
        nPCol1 = nCol1; nCol1 = nOldCol1;
        nPCol2 = nCol2; nCol2 = nOldCol2;
    }
    return eCurTok = ePush;
}

// Dies ist fuer die Decompilation.
// Zahlen und Symbole liefern einen Leerstring zurueck.

const String& SbiTokenizer::Symbol( SbiToken t )
{
    // Zeichen-Token?
    if( t < FIRSTKWD )
    {
        aSym = (char) t;
        return aSym;
    }
    switch( t )
    {
        case NEG   : aSym = '-'; return aSym;
        case EOS   : aSym = String::CreateFromAscii( ":/CRLF" ); return aSym;
        case EOLN  : aSym = String::CreateFromAscii( "CRLF" ); return aSym;
        default: break;
    }
    TokenTable* tp = pTokTable;
    for( short i = 0; i < nToken; i++, tp++ )
    {
        if( tp->t == t )
        {
            aSym = String::CreateFromAscii( tp->s );
            return aSym;
        }
    }
    const sal_Unicode *p = aSym.GetBuffer();
    if (*p <= ' ') aSym = String::CreateFromAscii( "???" );
    return aSym;
}

// Einlesen des naechsten Tokens und Ablage desselben
// Tokens, die nicht in der Token-Tabelle vorkommen, werden
// direkt als Zeichen zurueckgeliefert.
// Einige Worte werden gesondert behandelt.

SbiToken SbiTokenizer::Next()
{
    if (bEof) return EOLN;
    // Schon eines eingelesen?
    if( ePush != NIL )
    {
        eCurTok = ePush;
        ePush = NIL;
        nLine = nPLine;
        nCol1 = nPCol1;
        nCol2 = nPCol2;
        bEos = IsEoln( eCurTok );
        return eCurTok;
    }
    TokenTable *tp;

    // Sonst einlesen:
    if( !NextSym() )
    {
        bEof = bEos = sal_True;
        return eCurTok = EOLN;
    }
    // Zeilenende?
    if( aSym.GetBuffer()[0] == '\n' )
    {
        bEos = sal_True; return eCurTok = EOLN;
    }
    bEos = sal_False;

    // Zahl?
    if( bNumber )
        return eCurTok = NUMBER;

    // String?
    else if( ( eScanType == SbxDATE || eScanType == SbxSTRING ) && !bSymbol )
        return eCurTok = FIXSTRING;
    // Sonderfaelle von Zeichen, die zwischen "Z" und "a" liegen. ICompare()
    // wertet die Position dieser Zeichen unterschiedlich aus.
    else if( aSym.GetBuffer()[0] == '^' )
        return eCurTok = EXPON;
    else if( aSym.GetBuffer()[0] == '\\' )
        return eCurTok = IDIV;
    else
    {
        // Mit Typkennung oder ein Symbol und keine Keyword-Erkennung?
        // Dann kein Token-Test
        if( eScanType != SbxVARIANT
         || ( !bKeywords && bSymbol ) )
            return eCurTok = SYMBOL;
        // Gueltiges Token?
        short lb = 0;
        short ub = nToken-1;
        short delta;
        do
        {
            delta = (ub - lb) >> 1;
            tp = &pTokTable[ lb + delta ];
            StringCompare res = aSym.CompareIgnoreCaseToAscii( tp->s );
            // Gefunden?
            if( res == COMPARE_EQUAL )
                goto special;
            // Groesser? Dann untere Haelfte
            if( res == COMPARE_LESS )
            {
                if ((ub - lb) == 2) ub = lb;
                else ub = ub - delta;
            }
            // Kleiner? Dann obere Haelfte
            else
            {
                if ((ub -lb) == 2) lb = ub;
                else lb = lb + delta;
            }
        } while( delta );
        // Symbol? Wenn nicht >= Token
        sal_Unicode ch = aSym.GetBuffer()[0];
        if( !BasicSimpleCharClass::isAlpha( ch, bCompatible ) && !bSymbol )
            return eCurTok = (SbiToken) (ch & 0x00FF);
        return eCurTok = SYMBOL;
    }
special:
    // #i92642
    if( eCurTok != NIL && eCurTok != REM && eCurTok != EOLN && (tp->t == NAME || tp->t == LINE) )
        return eCurTok = SYMBOL;
    else if( tp->t == TEXT )
        return eCurTok = SYMBOL;

    // #i92642: Special LINE token handling -> SbiParser::Line()

    // END IF, CASE, SUB, DEF, FUNCTION, TYPE, CLASS, WITH
    if( tp->t == END )
    {
        // AB, 15.3.96, Spezialbehandlung fuer END, beim Peek() geht die
        // aktuelle Zeile verloren, daher alles merken und danach restaurieren
        sal_uInt16 nOldLine = nLine;
        sal_uInt16 nOldCol  = nCol;
        sal_uInt16 nOldCol1 = nCol1;
        sal_uInt16 nOldCol2 = nCol2;
        String aOldSym = aSym;
        SaveLine();             // pLine im Scanner sichern

        eCurTok = Peek();
        switch( eCurTok )
        {
            case IF:       Next(); eCurTok = ENDIF; break;
            case SELECT:   Next(); eCurTok = ENDSELECT; break;
            case SUB:      Next(); eCurTok = ENDSUB; break;
            case FUNCTION: Next(); eCurTok = ENDFUNC; break;
            case PROPERTY: Next(); eCurTok = ENDPROPERTY; break;
            case TYPE:     Next(); eCurTok = ENDTYPE; break;
            case ENUM:     Next(); eCurTok = ENDENUM; break;
            case WITH:     Next(); eCurTok = ENDWITH; break;
            default :      eCurTok = END;
        }
        nCol1 = nOldCol1;
        if( eCurTok == END )
        {
            // Alles zuruecksetzen, damit Token nach END ganz neu gelesen wird
            ePush = NIL;
            nLine = nOldLine;
            nCol  = nOldCol;
            nCol2 = nOldCol2;
            aSym = aOldSym;
            RestoreLine();      // pLine im Scanner restaurieren
        }
        return eCurTok;
    }
    // Sind Datentypen Keywords?
    // Nur nach AS, sonst sind es Symbole!
    // Es gibt ja ERROR(), DATA(), STRING() etc.
    eCurTok = tp->t;
    // AS: Datentypen sind Keywords
    if( tp->t == AS )
        bAs = sal_True;
    else
    {
        if( bAs )
            bAs = sal_False;
        else if( eCurTok >= DATATYPE1 && eCurTok <= DATATYPE2 && (bErrorIsSymbol || eCurTok != _ERROR_) )
            eCurTok = SYMBOL;
    }

    // CLASSMODULE, PROPERTY, GET, ENUM token only visible in compatible mode
    SbiToken eTok = tp->t;
    if( bCompatible )
    {
        // #129904 Suppress system
        if( eTok == STOP && aSym.CompareIgnoreCaseToAscii( "system" ) == COMPARE_EQUAL )
            eCurTok = SYMBOL;
    }
    else
    {
        if( eTok == CLASSMODULE ||
            eTok == IMPLEMENTS ||
            eTok == PARAMARRAY ||
            eTok == ENUM ||
            eTok == PROPERTY ||
            eTok == GET ||
            eTok == TYPEOF )
        {
            eCurTok = SYMBOL;
        }
    }

    bEos = IsEoln( eCurTok );
    return eCurTok;
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// Kann das aktuell eingelesene Token ein Label sein?

sal_Bool SbiTokenizer::MayBeLabel( sal_Bool bNeedsColon )
{
    if( eCurTok == SYMBOL || m_aTokenLabelInfo.canTokenBeLabel( eCurTok ) )
        return bNeedsColon ? DoesColonFollow() : sal_True;
    else
        return sal_Bool( eCurTok == NUMBER
                  && eScanType == SbxINTEGER
                  && nVal >= 0 );
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif


void SbiTokenizer::Hilite( SbTextPortions& rList )
{
    bErrors = sal_False;
    bUsedForHilite = sal_True;
    SbiToken eLastTok = NIL;
    for( ;; )
    {
        Next();
        if( IsEof() )
            break;
        SbTextPortion aRes;
        aRes.nLine = nLine;
        aRes.nStart = nCol1;
        aRes.nEnd = nCol2;
        switch( eCurTok )
        {
            case REM:
                aRes.eType = SB_COMMENT; break;
            case SYMBOL:
                aRes.eType = SB_SYMBOL; break;
            case FIXSTRING:
                aRes.eType = SB_STRING; break;
            case NUMBER:
                aRes.eType = SB_NUMBER; break;
            default:
                if( ( eCurTok >= FIRSTKWD && eCurTok <= LASTKWD )
                 || (eCurTok >= _CDECL_ ) )
                    aRes.eType = SB_KEYWORD;
                else
                    aRes.eType = SB_PUNCTUATION;
        }
        // Die Folge xxx.Keyword sollte nicht als Kwd geflagt werden
        if( aRes.eType == SB_KEYWORD
         && ( eLastTok == DOT|| eLastTok == EXCLAM ) )
            aRes.eType = SB_SYMBOL;
        if( eCurTok != EOLN && aRes.nStart <= aRes.nEnd )
            rList.Insert( aRes, rList.Count() );
        if( aRes.eType == SB_COMMENT )
            break;
        eLastTok = eCurTok;
    }
    bUsedForHilite = sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
