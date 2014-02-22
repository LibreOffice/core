/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "basiccharclass.hxx"
#include "sbcomp.hxx"

struct TokenTable { SbiToken t; const char *s; };

static short nToken;                    

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
    { BASIC_EXPLICIT, "Explicit" },
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
    { INPUT,    "Input" },              
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
    { LSET,     "LSet" }, 
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
    { RSET,     "RSet" }, 
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
    { WRITE,    "Write" },              
    { XOR,      "Xor" },
    { NIL,      "" }
};



TokenLabelInfo::TokenLabelInfo( void )
{
    m_pTokenCanBeLabelTab = new bool[VBASUPPORT+1];
    for( int i = 0 ; i <= VBASUPPORT ; ++i )
    {
        m_pTokenCanBeLabelTab[i] = false;
    }
    
    SbiToken eLabelToken[] = { ACCESS, ALIAS, APPEND, BASE, BINARY, CLASSMODULE,
                               COMPARE, COMPATIBLE, DEFERR, _ERROR_, BASIC_EXPLICIT, LIB, LINE, LPRINT, NAME,
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




SbiTokenizer::SbiTokenizer( const OUString& rSrc, StarBASIC* pb )
    : SbiScanner(rSrc, pb)
    , eCurTok(NIL)
    , ePush(NIL)
    , nPLine(0)
    , nPCol1(0)
    , nPCol2(0)
    , bEof(false)
    , bEos(true)
    , bKeywords(true)
    , bAs(false)
    , bErrorIsSymbol(true)
{
    pTokTable = aTokTable_Basic;
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



const OUString& SbiTokenizer::Symbol( SbiToken t )
{
    
    if( t < FIRSTKWD )
    {
        aSym = OUString(sal::static_int_cast<sal_Unicode>(t));
        return aSym;
    }
    switch( t )
    {
    case NEG   :
        aSym = "-";
        return aSym;
    case EOS   :
        aSym = ":/CRLF";
        return aSym;
    case EOLN  :
        aSym = "CRLF";
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
        aSym = "???";
    }
    return aSym;
}






SbiToken SbiTokenizer::Next()
{
    if (bEof)
    {
        return EOLN;
    }
    
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

    if( aSym.startsWith("\n") )
    {
        bEos = true;
        return eCurTok = EOLN;
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
    else if( aSym.isEmpty() )
    {
        
        bEof = bEos = true;
        return eCurTok = EOLN;
    }
    
    
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
        
        sal_Unicode ch = aSym[0];
        if( !theBasicCharClass::get().isAlpha( ch, bCompatible ) && !bSymbol )
        {
            return eCurTok = (SbiToken) (ch & 0x00FF);
        }
        return eCurTok = SYMBOL;
    }
special:
    
    bool bStartOfLine = (eCurTok == NIL || eCurTok == REM || eCurTok == EOLN);
    if( !bStartOfLine && (tp->t == NAME || tp->t == LINE) )
    {
        return eCurTok = SYMBOL;
    }
    else if( tp->t == TEXT )
    {
        return eCurTok = SYMBOL;
    }
    
    
    
    
    
    
    
    else if ( ( !bInStatement || eCurTok == DIM ) && tp->t == APPEND )
    {
        return eCurTok = SYMBOL;
    }
    

    
    if( tp->t == END )
    {
        
        
        sal_uInt16 nOldLine = nLine;
        sal_uInt16 nOldCol  = nCol;
        sal_uInt16 nOldCol1 = nCol1;
        sal_uInt16 nOldCol2 = nCol2;
        OUString aOldSym = aSym;
        SaveLine();             

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
            
            ePush = NIL;
            nLine = nOldLine;
            nCol  = nOldCol;
            nCol2 = nOldCol2;
            aSym = aOldSym;
            RestoreLine();
        }
        return eCurTok;
    }
    
    
    eCurTok = tp->t;
    
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

    
    SbiToken eTok = tp->t;
    if( bCompatible )
    {
        
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


OUString SbiTokenizer::GetKeywordCase( const OUString& sKeyword )
{
    if( !nToken )
    {
        const TokenTable *tp;
        for( nToken = 0, tp = pTokTable; tp->t; nToken++, tp++ )
        {}
    }
    const TokenTable* tp = pTokTable;
    for( short i = 0; i < nToken; i++, tp++ )
    {
        OUString sStr = OStringToOUString(tp->s, RTL_TEXTENCODING_ASCII_US);
        if( sStr.equalsIgnoreAsciiCase(sKeyword) )
        {
            return sStr;
        }
    }
    return OUString("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
