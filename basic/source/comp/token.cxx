/*************************************************************************
 *
 *  $RCSfile: token.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <ctype.h>
#include "sbcomp.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBCOMP, SBCOMP_CODE )

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
    { BASE,     "Base" },
    { BINARY,   "Binary" },
    { TBOOLEAN, "Boolean" },
    { BYVAL,    "ByVal", },
    { CALL,     "Call" },
    { CASE,     "Case" },
    { _CDECL_,  "Cdecl" },
    { CLOSE,    "Close" },
    { COMPARE,  "Compare" },
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
    { ENDFUNC,  "End Function" },
    { ENDIF,    "End If" },
    { ENDSELECT,"End Select" },
    { ENDSUB,   "End Sub" },
    { ENDTYPE,  "End Type" },
    { ENDIF,    "EndIf" },
    { EQV,      "Eqv" },
    { ERASE,    "Erase" },
    { _ERROR_,  "Error" },
    { EXIT,     "Exit" },
    { EXPLICIT, "Explicit" },
    { FOR,      "For" },
    { FUNCTION, "Function" },
    { GLOBAL,   "Global" },
    { GOSUB,    "GoSub" },
    { GOTO,     "GoTo" },
    { IF,       "If" },
    { IMP,      "Imp" },
    { _IN_,     "In" },
    { INPUT,    "Input" },              // auch INPUT #
    { TINTEGER, "Integer" },
    { IS,       "Is" },
    { LET,      "Let" },
    { LIB,      "Lib" },
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
    { PRESERVE, "Preserve" },
    { PRINT,    "Print" },
    { PRIVATE,  "Private" },
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
    { UNTIL,    "Until" },
    { TVARIANT, "Variant" },
    { WEND,     "Wend" },
    { WHILE,    "While" },
    { WITH,     "With" },
    { WRITE,    "Write" },              // auch WRITE #
    { XOR,      "Xor" },
    { NIL }
};

/*
TokenTable aTokTable_Java [] = {        // Token-Tabelle:

    { JS_LOG_NOT,   "!" },
    { JS_NE,        "!=" },
    { JS_MOD,       "%" },
    { JS_ASS_MOD,   "%=" },
    { JS_BIT_AND,   "&" },
    { JS_LOG_AND,   "&&" },
    { JS_ASS_AND,   "&=" },
    { JS_LPAREN,    "(" },
    { JS_RPAREN,    ")" },
    { JS_MUL,       "*" },
    { JS_ASS_MUL,   "*=" },
    { JS_PLUS,      "+" },
    { JS_INC,       "++" },
    { JS_ASS_PLUS,  "+=" },
    { JS_COMMA,     "," },
    { JS_MINUS,     "-" },
    { JS_DEC,       "--" },
    { JS_ASS_MINUS, "-=" },
    { JS_DIV,       "/" },
    { JS_ASS_DIV,   "/=" },
    { JS_COND_SEL,  ":" },
    { JS_LT,        "<" },
    { JS_LSHIFT,    "<<" },
    { JS_ASS_LSHIFT,"<<=" },
    { JS_LE,        "<=" },
    { JS_NE,        "<>" },
    { JS_ASSIGNMENT,"=" },
    { JS_EQ,        "==" },
    { JS_GT,        ">" },
    { JS_RSHIFT,    ">>" },
    { JS_ASS_RSHIFT,">>=" },
    { JS_RSHIFT_Z,  ">>>" },
    { JS_ASS_RSHIFT_Z,">>>=" },
    { JS_GE,        ">=" },
    { JS_COND_QUEST,"?" },
    { ACCESS,   "Access" },
    { ALIAS,    "Alias" },
    { AND,      "And" },
    { ANY,      "Any" },
    { APPEND,   "Append" },
    { AS,       "As" },
    { BASE,     "Base" },
    { BINARY,   "Binary" },
    { TBOOLEAN, "Boolean" },
    { BYVAL,    "ByVal", },
    { CALL,     "Call" },
    { CASE,     "Case" },
    { _CDECL_,  "Cdecl" },
    { CLOSE,    "Close" },
    { COMPARE,  "Compare" },
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
    { ENDFUNC,  "End Function" },
    { ENDIF,    "End If" },
    { ENDSELECT,"End Select" },
    { ENDSUB,   "End Sub" },
    { ENDTYPE,  "End Type" },
    { ENDIF,    "EndIf" },
    { EQV,      "Eqv" },
    { ERASE,    "Erase" },
    { _ERROR_,  "Error" },
    { EXIT,     "Exit" },
    { EXPLICIT, "Explicit" },
    { FOR,      "For" },
    { FUNCTION, "Function" },
    { GLOBAL,   "Global" },
    { GOSUB,    "GoSub" },
    { GOTO,     "GoTo" },
    { IF,       "If" },
    { IMP,      "Imp" },
    { _IN_,     "In" },
    { INPUT,    "Input" },              // auch INPUT #
    { TINTEGER, "Integer" },
    { IS,       "Is" },
    { LET,      "Let" },
    { LIB,      "Lib" },
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
    { PRESERVE, "Preserve" },
    { PRINT,    "Print" },
    { PRIVATE,  "Private" },
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
    { SHARED,   "Shared" },
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
    { UNTIL,    "Until" },
    { TVARIANT, "Variant" },
    { WEND,     "Wend" },
    { WHILE,    "While" },
    { WITH,     "With" },
    { WRITE,    "Write" },              // auch WRITE #
    { XOR,      "Xor" },
    { JS_LINDEX,    "[" },
    { JS_RINDEX,    "]" },
    { JS_BIT_XOR,   "^" },
    { JS_ASS_XOR,   "^=" },
    { JS_BIT_OR,    "|" },
    { JS_ASS_OR,    "|=" },
    { JS_LOG_OR,    "||" },
    { JS_BIT_NOT,   "~" },
    { NIL }
};
*/

// Der Konstruktor ermittelt die Laenge der Token-Tabelle.

SbiTokenizer::SbiTokenizer( const String& rSrc, StarBASIC* pb )
           : SbiScanner( rSrc, pb )
{
    pTokTable = aTokTable_Basic;
    //if( StarBASIC::GetGlobalLanguageMode() == SB_LANG_JAVASCRIPT )
    //  pTokTable = aTokTable_Java;
    TokenTable *tp;
    bEof = bAs = FALSE;
    eCurTok = NIL;
    ePush = NIL;
    bEos = bKeywords = TRUE;
    if( !nToken )
        for( nToken = 0, tp = pTokTable; tp->t; nToken++, tp++ ) {}
}

SbiTokenizer::~SbiTokenizer()
{}

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
        USHORT nOldLine = nLine;
        USHORT nOldCol1 = nCol1;
        USHORT nOldCol2 = nCol2;
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
        bEof = bEos = TRUE;
        return eCurTok = EOLN;
    }
    // Zeilenende?
    if( aSym.GetBuffer()[0] == '\n' )
    {
        bEos = TRUE; return eCurTok = EOLN;
    }
    bEos = FALSE;

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
            if( res == COMPARE_EQUAL ) goto special;
            // Groesser? Dann untere Haelfte
            if( res == COMPARE_LESS )
            {
                if ((ub - lb) == 2) ub = lb;
                else ub -= delta;
            }
            // Kleiner? Dann obere Haelfte
            else
            {
                if ((ub -lb) == 2) lb = ub;
                else lb += delta;
            }
        } while( delta );
        // Symbol? Wenn nicht >= Token
        sal_Unicode ch = aSym.GetBuffer()[0];
        if( !isalpha( ch ) && !bSymbol )
            return eCurTok = (SbiToken) (ch & 0x00FF);
        return eCurTok = SYMBOL;
    }
special:
    // LINE INPUT
    if( tp->t == LINE )
    {
        short nC1 = nCol1;
        eCurTok = Peek();
        if( eCurTok == INPUT )
        {
            Next();
            nCol1 = nC1;
            return eCurTok = LINEINPUT;
        }
        else
            return eCurTok = LINE;
    }
    // END IF, CASE, SUB, DEF, FUNCTION, TYPE, CLASS, WITH
    if( tp->t == END )
    {
        // AB, 15.3.96, Spezialbehandlung fuer END, beim Peek() geht die
        // aktuelle Zeile verloren, daher alles merken und danach restaurieren
        USHORT nOldLine = nLine;
        USHORT nOldCol  = nCol;
        USHORT nOldCol1 = nCol1;
        USHORT nOldCol2 = nCol2;
        String aOldSym = aSym;
        SaveLine();             // pLine im Scanner sichern

        eCurTok = Peek();
        switch( eCurTok )
        {
            case IF:       Next(); eCurTok = ENDIF; break;
            case SELECT:   Next(); eCurTok = ENDSELECT; break;
            case SUB:      Next(); eCurTok = ENDSUB; break;
            case FUNCTION: Next(); eCurTok = ENDFUNC; break;
            case TYPE:     Next(); eCurTok = ENDTYPE; break;
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
        bAs = TRUE;
    else
    {
        if( bAs )
            bAs = FALSE;
        else if( eCurTok >= DATATYPE1 && eCurTok <= DATATYPE2 )
            eCurTok = SYMBOL;
    }
    bEos = IsEoln( eCurTok );
    return eCurTok;
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif

// Kann das aktuell eingelesene Token ein Label sein?

BOOL SbiTokenizer::MayBeLabel( BOOL bNeedsColon )
{
    if( eCurTok == SYMBOL )
        return bNeedsColon ? DoesColonFollow() : TRUE;
    else
        return BOOL( eCurTok == NUMBER
                  && eScanType == SbxINTEGER
                  && nVal >= 0 );
}

#ifdef _MSC_VER
#pragma optimize("",off)
#endif


void SbiTokenizer::Hilite( SbTextPortions& rList )
{
    bErrors = FALSE;
    bUsedForHilite = TRUE;
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
        sal_Unicode ch = aSym.GetBuffer()[0];
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
    bUsedForHilite = FALSE;
}

