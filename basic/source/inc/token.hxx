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

#ifndef _TOKEN_HXX
#define _TOKEN_HXX

#include "scanner.hxx"
#include <basic/sbdef.hxx>

#if defined( SHARED )
#define SbiTokenSHAREDTMPUNDEF
#undef SHARED
#endif

// The tokenizer is stand-alone, i. e. he can be used from everywhere.
// A BASIC-instance is necessary for error messages. Without BASIC the
// errors are only counted. The BASIC is also necessary when an advanced
// SBX-variable shall be used for recognition of data types etc.


enum SbiToken {
    NIL = 0,
    // tokens between 0x20 and 0x3F are literals:
    LPAREN = '(', RPAREN = ')', COMMA = ',', DOT = '.', EXCLAM = '!',
    HASH = '#', SEMICOLON = ';',

    // commands:
    FIRSTKWD = 0x40,
    AS = FIRSTKWD, ALIAS, ASSIGN,
    CALL, CASE, CLOSE, COMPARE, _CONST_,
    DECLARE, DIM, DO,

    // in the order of the data type enums!
    DEFINT, DEFLNG, DEFSNG, DEFDBL, DEFCUR, DEFDATE, DEFSTR, DEFOBJ,
    DEFERR, DEFBOOL, DEFVAR,
    // in the order of the data type enums!
    DATATYPE1,
    TINTEGER = DATATYPE1,
    TLONG, TSINGLE, TDOUBLE, TCURRENCY, TDATE, TSTRING, TOBJECT,
    _ERROR_, TBOOLEAN, TVARIANT, TBYTE,
    DATATYPE2 = TBYTE,

    EACH, ELSE, ELSEIF, END, ERASE, EXIT,
    FOR, FUNCTION,
    GET, GLOBAL, GOSUB, GOTO,
    IF, _IN_, INPUT,
    LET, LINE, LINEINPUT, LOCAL, LOOP, LPRINT, LSET,
    NAME, NEW, NEXT,
    ON, OPEN, OPTION, ATTRIBUTE, IMPLEMENTS,
    PRINT, PRIVATE, PROPERTY, PUBLIC,
    REDIM, REM, RESUME, RETURN, RSET,
    SELECT, SET, SHARED, STATIC, STEP, STOP, SUB,
    TEXT, THEN, TO, TYPE, ENUM,
    UNTIL,
    WEND, WHILE, WITH, WRITE,
    ENDENUM, ENDIF, ENDFUNC, ENDPROPERTY, ENDSUB, ENDTYPE, ENDSELECT, ENDWITH,
    // end of all keywords
    LASTKWD = ENDWITH,
    // statement end
    EOS, EOLN,
    // operators:
    EXPON, NEG, MUL,
    DIV, IDIV, MOD, PLUS, MINUS,
    EQ, NE, LT, GT, LE, GE,
    NOT, AND, OR, XOR, EQV,
    IMP, CAT, LIKE, IS, TYPEOF,
    // miscellaneous:
    FIRSTEXTRA,
    NUMBER=FIRSTEXTRA, FIXSTRING, SYMBOL, _CDECL_, BYVAL, BYREF,
    OUTPUT, RANDOM, APPEND, BINARY, ACCESS,
    LOCK, READ, PRESERVE, BASE, ANY, LIB, _OPTIONAL_,
    EXPLICIT, COMPATIBLE, CLASSMODULE, PARAMARRAY, WITHEVENTS,

    // from here there are JavaScript-tokens (same enum so that same type)
    FIRSTJAVA,
    JS_BREAK=FIRSTJAVA, JS_CONTINUE, JS_FOR, JS_FUNCTION, JS_IF, JS_NEW,
    JS_RETURN, JS_THIS, JS_VAR, JS_WHILE, JS_WITH,

    // JavaScript-operators
    // _ASS_ = Assignment
    JS_COMMA, JS_ASSIGNMENT, JS_ASS_PLUS, JS_ASS_MINUS, JS_ASS_MUL,
    JS_ASS_DIV, JS_ASS_MOD, JS_ASS_LSHIFT, JS_ASS_RSHIFT, JS_ASS_RSHIFT_Z,
    JS_ASS_AND, JS_ASS_XOR, JS_ASS_OR,
    JS_COND_QUEST, JS_COND_SEL, JS_LOG_OR, JS_LOG_AND, JS_BIT_OR,
    JS_BIT_XOR, JS_BIT_AND, JS_EQ, JS_NE, JS_LT, JS_LE,
    JS_GT, JS_GE, JS_LSHIFT, JS_RSHIFT, JS_RSHIFT_Z,
    JS_PLUS, JS_MINUS, JS_MUL, JS_DIV, JS_MOD, JS_LOG_NOT, JS_BIT_NOT,
    JS_INC, JS_DEC, JS_LPAREN, JS_RPAREN, JS_LINDEX, JS_RINDEX
    , VBASUPPORT
};

#ifdef SbiTokenSHAREDTMPUNDEF
#define SHARED
#undef SbiTokenSHAREDTMPUNDEF
#endif

// #i109076
class TokenLabelInfo
{
    bool* m_pTokenCanBeLabelTab;

public:
    TokenLabelInfo( void );
    TokenLabelInfo( const TokenLabelInfo& rInfo )
        : m_pTokenCanBeLabelTab( NULL )
            { (void)rInfo; }
    ~TokenLabelInfo();

    bool canTokenBeLabel( SbiToken eTok )
        { return m_pTokenCanBeLabelTab[eTok]; }
};

class SbiTokenizer : public SbiScanner {
    TokenLabelInfo  m_aTokenLabelInfo;

protected:
    SbiToken eCurTok;
    SbiToken ePush;
    sal_uInt16  nPLine, nPCol1, nPCol2; // pushback location
    bool bEof;
    bool bEos;
    bool bKeywords;                 // true, if keywords are parsed
    bool bAs;                       // last keyword was AS
    bool bErrorIsSymbol;            // Handle Error token as Symbol, not keyword
public:
    SbiTokenizer( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiTokenizer();

    inline bool IsEof()             { return bEof; }
    inline bool IsEos()             { return bEos; }

    void  Push( SbiToken );
    const ::rtl::OUString& Symbol( SbiToken );   // reconversion

    SbiToken Peek();                    // read the next token
    SbiToken Next();                    // read a token
    bool MayBeLabel( bool= false );

    void Error( SbError c ) { GenError( c ); }
    void Error( SbError, SbiToken );
    void Error( SbError, const char* );
    void Error( SbError, const ::rtl::OUString &);

    static bool IsEoln( SbiToken t )
        { return t == EOS || t == EOLN || t == REM; }
    static bool IsKwd( SbiToken t )
        { return t >= FIRSTKWD && t <= LASTKWD; }
    static bool IsExtra( SbiToken t )
        { return t >= FIRSTEXTRA; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
