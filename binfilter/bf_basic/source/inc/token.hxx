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

#ifndef _TOKEN_HXX
#define _TOKEN_HXX

#ifndef _SCANNER_HXX
#include "scanner.hxx"
#endif
#ifndef _SBDEF_HXX
#include "sbdef.hxx"
#endif

#if defined( SHARED )
#define SbiTokenSHAREDTMPUNDEF
#undef SHARED
#endif

namespace binfilter {

// Der Tokenizer ist stand-alone, d.h. er kann von ueberallher verwendet
// werden. Eine BASIC-Instanz ist fuer Fehlermeldungen notwendig. Ohne
// BASIC werden die Fehler nur gezaehlt. Auch ist Basic notwendig, wenn
// eine erweiterte SBX-Variable zur Erkennung von Datentypen etc. verwendet
// werden soll.

enum SbiToken {
    NIL = 0,
    // Token zwischen 0x20 und 0x3F sind Literale:
    LPAREN = '(', RPAREN = ')', COMMA = ',', DOT = '.', EXCLAM = '!',
    HASH = '#', SEMICOLON = ';',

    // Anweisungen:
    FIRSTKWD = 0x40,
    AS = FIRSTKWD, ALIAS, ASSIGN,
    CALL, CASE, CLOSE, COMPARE, _CONST_,
    DECLARE, DIM, DO,

    // in der Reihenfolge der Datentyp-Enums!
    DEFINT, DEFLNG, DEFSNG, DEFDBL, DEFCUR, DEFDATE, DEFSTR, DEFOBJ,
    DEFERR, DEFBOOL, DEFVAR,
    // in der Reihenfolge der Datentyp-Enums!
    DATATYPE1,
    TINTEGER = DATATYPE1,
    TLONG, TSINGLE, TDOUBLE, TCURRENCY, TDATE, TSTRING, TOBJECT,
    _ERROR_, TBOOLEAN, TVARIANT, TBYTE,
    DATATYPE2 = TBYTE,

    EACH, ELSE,	ELSEIF, END, ERASE, EXIT,
    FOR, FUNCTION,
    GET, GLOBAL, GOSUB, GOTO,
    IF, _IN_, INPUT,
    LET, LINE, LINEINPUT, LOCAL, LOOP, LPRINT, LSET,
    NAME, NEW, NEXT,
    ON, OPEN, OPTION, IMPLEMENTS,
    PRINT, PRIVATE, PROPERTY, PUBLIC,
    REDIM, REM, RESUME, RETURN, RSET,
    SELECT, SET, SHARED, STATIC, STEP, STOP, SUB,
    TEXT, THEN, TO, TYPE, ENUM,
    UNTIL,
    WEND, WHILE, WITH, WRITE,
    ENDENUM, ENDIF, ENDFUNC, ENDPROPERTY, ENDSUB, ENDTYPE, ENDSELECT, ENDWITH,
    // Ende aller Keywords
    LASTKWD = ENDWITH,
    // Statement-Ende
    EOS, EOLN,
    // Operatoren:
    EXPON, NEG, MUL,
    DIV, IDIV, MOD, PLUS, MINUS,
    EQ, NE, LT, GT, LE, GE,
    NOT, AND, OR, XOR, EQV,
    IMP, CAT, LIKE, IS, TYPEOF,
    // Sonstiges:
    FIRSTEXTRA,
    NUMBER=FIRSTEXTRA, FIXSTRING, SYMBOL, _CDECL_, BYVAL, BYREF,
    OUTPUT, RANDOM, APPEND, BINARY, ACCESS,
    LOCK, READ, PRESERVE, BASE, ANY, LIB, _OPTIONAL_,
    EXPLICIT, COMPATIBLE, CLASSMODULE, PARAMARRAY,

    // Ab hier kommen JavaScript-Tokens (gleiches enum, damit gleicher Typ)
    FIRSTJAVA,
    JS_BREAK=FIRSTJAVA, JS_CONTINUE, JS_FOR, JS_FUNCTION, JS_IF, JS_NEW,
    JS_RETURN, JS_THIS, JS_VAR, JS_WHILE, JS_WITH,

    // JavaScript-Operatoren
    // _ASS_ = Assignment
    JS_COMMA, JS_ASSIGNMENT, JS_ASS_PLUS, JS_ASS_MINUS, JS_ASS_MUL,
    JS_ASS_DIV, JS_ASS_MOD, JS_ASS_LSHIFT, JS_ASS_RSHIFT, JS_ASS_RSHIFT_Z,
    JS_ASS_AND, JS_ASS_XOR, JS_ASS_OR,
    JS_COND_QUEST, JS_COND_SEL, JS_LOG_OR, JS_LOG_AND, JS_BIT_OR,
    JS_BIT_XOR, JS_BIT_AND,	JS_EQ, JS_NE, JS_LT, JS_LE,
    JS_GT, JS_GE, JS_LSHIFT, JS_RSHIFT, JS_RSHIFT_Z,
    JS_PLUS, JS_MINUS, JS_MUL, JS_DIV, JS_MOD, JS_LOG_NOT, JS_BIT_NOT,
    JS_INC, JS_DEC, JS_LPAREN, JS_RPAREN, JS_LINDEX, JS_RINDEX
    , VBASUPPORT
};

#ifdef SbiTokenSHAREDTMPUNDEF
#define SHARED
#undef SbiTokenSHAREDTMPUNDEF
#endif

class SbiTokenizer : public SbiScanner {
protected:
    SbiToken eCurTok;				// aktuelles Token
    SbiToken ePush;					// Pushback-Token
    USHORT  nPLine, nPCol1, nPCol2; // Pushback-Location
    BOOL bEof;						// TRUE bei Dateiende
    BOOL bEos;						// TRUE bei Statement-Ende
    BOOL bKeywords;					// TRUE, falls Keywords geparst werden
    BOOL bAs;						// letztes Keyword war AS
    BOOL bErrorIsSymbol;			// Handle Error token as Symbol, not keyword
public:
    SbiTokenizer( const ::rtl::OUString&, StarBASIC* = NULL );
   ~SbiTokenizer();

    inline BOOL IsEof()				{ return bEof; }
    inline BOOL IsEos()				{ return bEos; }

    SbiToken Peek();				 	// das naechste Token lesen
    SbiToken Next();				  	// Ein Token lesen
    void Error( SbError c ) { GenError( c ); }

    void Keywords( BOOL b ) { bKeywords = b; }

    static BOOL IsEoln( SbiToken t )
        { return BOOL( t == EOS || t == EOLN || t == REM ); }
    static BOOL IsKwd( SbiToken t )
        { return BOOL( t >= FIRSTKWD && t <= LASTKWD ); }
    static BOOL IsExtra( SbiToken t )
        { return BOOL( t >= FIRSTEXTRA ); }
};

}

#endif
