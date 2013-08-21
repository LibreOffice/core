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


#include "basiccharclass.hxx"
#include "sbcomp.hxx"

struct TokenTable { SbiToken t; const char *s; };

static short nToken;                    // number of tokens

static const TokenTable* pTokTable;

static const TokenTable aTokTable_Basic [] = {
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
    { INPUT,    "Input" },              // also INPUT #
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
    { WRITE,    "Write" },              // also WRITE #
    { XOR,      "Xor" },
    { NIL,      "" }
};


// #i109076
TokenLabelInfo::TokenLabelInfo( void )
{
    m_pTokenCanBeLabelTab = new bool[VBASUPPORT+1];
    for( int i = 0 ; i <= VBASUPPORT ; ++i )
    {
        m_pTokenCanBeLabelTab[i] = false;
    }
    // Token accepted as label by VBA
    SbiToken eLabelToken[] = { ACCESS, ALIAS, APPEND, BASE, BINARY, CLASSMODULE,
                               COMPARE, COMPATIBLE, DEFERR, _ERROR_, EXPLICIT, LIB, LINE, LPRINT, NAME,
                               TOBJECT, OUTPUT, PROPERTY, RANDOM, READ, STEP, STOP, TEXT, VBASUPPORT, NIL };
    SbiToken* pTok = eLabelToken;
    SbiToken eTok;
    for( pTok = eLabelToken ; (eTok = *pTok) != NIL ; ++pTok )
    {
        m_pTokenCanBeLabelTab[eTok] = true;
    }
}

TokenLabelInfo::~TokenLabelInfo()
{
    delete[] m_pTokenCanBeLabelTab;
}


// the constructor detects the length of the token table

SbiTokenizer::SbiTokenizer( const OUString& rSrc, StarBASIC* pb )
           : SbiScanner( rSrc, pb )
{
    pTokTable = aTokTable_Basic;
    bEof = bAs = false;
    eCurTok = NIL;
    ePush = NIL;
    bEos = bKeywords = bErrorIsSymbol = true;
    if( !nToken )
    {
        const TokenTable *tp;
        for( nToken = 0, tp = pTokTable; tp->t; nToken++, tp++ )
        {}
    }
}

SbiTokenizer::~SbiTokenizer()
{
}


void SbiTokenizer::Push( SbiToken t )
{
    if( ePush != NIL )
        Error( SbERR_INTERNAL_ERROR, "PUSH" );
    else ePush = t;
}

void SbiTokenizer::Error( SbError code, const char* pMsg )
{
    aError = OUString::createFromAscii( pMsg );
    Error( code );
}

void SbiTokenizer::Error( SbError code, const OUString &aMsg )
{
    aError = aMsg;
    Error( code );
}

void SbiTokenizer::Error( SbError code, SbiToken tok )
{
    aError = Symbol( tok );
    Error( code );
}

// reading in the next token without absorbing it

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

// For decompilation. Numbers and symbols return an empty string.

const OUString& SbiTokenizer::Symbol( SbiToken t )
{
    // character token?
    if( t < FIRSTKWD )
    {
        aSym = OUString(sal::static_int_cast<sal_Unicode>(t));
        return aSym;
    }
    switch( t )
    {
    case NEG   :
        aSym = OUString("-");
        return aSym;
    case EOS   :
        aSym = OUString(":/CRLF");
        return aSym;
    case EOLN  :
        aSym = OUString("CRLF");
        return aSym;
    default:
        break;
    }
    const TokenTable* tp = pTokTable;
    for( short i = 0; i < nToken; i++, tp++ )
    {
        if( tp->t == t )
        {
            aSym = OStringToOUString(tp->s, RTL_TEXTENCODING_ASCII_US);
            return aSym;
        }
    }
    const sal_Unicode *p = aSym.getStr();
    if (*p <= ' ')
    {
        aSym = OUString("???");
    }
    return aSym;
}

// Reading in the next token and put it down.
// Tokens that don't appear in the token table
// are directly returned as a character.
// Some words are treated in a special way.

SbiToken SbiTokenizer::Next()
{
    if (bEof)
    {
        return EOLN;
    }
    // have read in one already?
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
    const TokenTable *tp;

    if( !NextSym() )
    {
        bEof = bEos = true;
        return eCurTok = EOLN;
    }

    if( aSym[0] == '\n' )
    {
        bEos = true; return eCurTok = EOLN;
    }
    bEos = false;

    if( bNumber )
    {
        return eCurTok = NUMBER;
    }
    else if( ( eScanType == SbxDATE || eScanType == SbxSTRING ) && !bSymbol )
    {
        return eCurTok = FIXSTRING;
    }
    // Special cases of characters that are between "Z" and "a". ICompare()
    // evaluates the position of these characters in different ways.
    else if( aSym[0] == '^' )
    {
        return eCurTok = EXPON;
    }
    else if( aSym[0] == '\\' )
    {
        return eCurTok = IDIV;
    }
    else
    {
        if( eScanType != SbxVARIANT
         || ( !bKeywords && bSymbol ) )
            return eCurTok = SYMBOL;
        // valid token?
        short lb = 0;
        short ub = nToken-1;
        short delta;
        do
        {
            delta = (ub - lb) >> 1;
            tp = &pTokTable[ lb + delta ];
            sal_Int32 res = aSym.compareToIgnoreAsciiCaseAscii( tp->s );

            if( res == 0 )
            {
                goto special;
            }
            if( res < 0 )
            {
                if ((ub - lb) == 2)
                {
                    ub = lb;
                }
                else
                {
                    ub = ub - delta;
                }
            }
            else
            {
                if ((ub -lb) == 2)
                {
                    lb = ub;
                }
                else
                {
                    lb = lb + delta;
                }
            }
        }
        while( delta );
        // Symbol? if not >= token
        sal_Unicode ch = aSym[0];
        if( !theBasicCharClass::get().isAlpha( ch, bCompatible ) && !bSymbol )
        {
            return eCurTok = (SbiToken) (ch & 0x00FF);
        }
        return eCurTok = SYMBOL;
    }
special:
    // #i92642
    bool bStartOfLine = (eCurTok == NIL || eCurTok == REM || eCurTok == EOLN);
    if( !bStartOfLine && (tp->t == NAME || tp->t == LINE) )
    {
        return eCurTok = SYMBOL;
    }
    else if( tp->t == TEXT )
    {
        return eCurTok = SYMBOL;
    }
    // maybe we can expand this for other statements that have parameters
    // that are keywords ( and those keywords are only used within such
    // statements )
    // what's happening here is that if we come across 'append' ( and we are
    // not in the middle of parsing a special statement ( like 'Open')
    // we just treat keyword 'append' as a normal 'SYMBOL'.
    // Also we accept Dim APPEND
    else if ( ( !bInStatement || eCurTok == DIM ) && tp->t == APPEND )
    {
        return eCurTok = SYMBOL;
    }
    // #i92642: Special LINE token handling -> SbiParser::Line()

    // END IF, CASE, SUB, DEF, FUNCTION, TYPE, CLASS, WITH
    if( tp->t == END )
    {
        // from 15.3.96, special treatment for END, at Peek() the current
        // time is lost, so memorize everything and restore after
        sal_uInt16 nOldLine = nLine;
        sal_uInt16 nOldCol  = nCol;
        sal_uInt16 nOldCol1 = nCol1;
        sal_uInt16 nOldCol2 = nCol2;
        OUString aOldSym = aSym;
        SaveLine();             // save pLine in the scanner

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
        default :      eCurTok = END; break;
        }
        nCol1 = nOldCol1;
        if( eCurTok == END )
        {
            // reset everything so that token is read completely newly after END
            ePush = NIL;
            nLine = nOldLine;
            nCol  = nOldCol;
            nCol2 = nOldCol2;
            aSym = aOldSym;
            RestoreLine();
        }
        return eCurTok;
    }
    // are data types keywords?
    // there is ERROR(), DATA(), STRING() etc.
    eCurTok = tp->t;
    // AS: data types are keywords
    if( tp->t == AS )
    {
        bAs = true;
    }
    else
    {
        if( bAs )
        {
            bAs = false;
        }
        else if( eCurTok >= DATATYPE1 && eCurTok <= DATATYPE2 && (bErrorIsSymbol || eCurTok != _ERROR_) )
        {
            eCurTok = SYMBOL;
        }
    }

    // CLASSMODULE, PROPERTY, GET, ENUM token only visible in compatible mode
    SbiToken eTok = tp->t;
    if( bCompatible )
    {
        // #129904 Suppress system
        if( eTok == STOP && aSym.equalsIgnoreAsciiCase("system") )
        {
            eCurTok = SYMBOL;
        }
        if( eTok == GET && bStartOfLine )
        {
            eCurTok = SYMBOL;
        }
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

bool SbiTokenizer::MayBeLabel( bool bNeedsColon )
{
    if( eCurTok == SYMBOL || m_aTokenLabelInfo.canTokenBeLabel( eCurTok ) )
    {
        return bNeedsColon ? DoesColonFollow() : true;
    }
    else
    {
        return ( eCurTok == NUMBER
                  && eScanType == SbxINTEGER
                  && nVal >= 0 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
