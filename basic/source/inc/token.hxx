/*************************************************************************
 *
 *  $RCSfile: token.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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
    _ERROR_, TBOOLEAN, TVARIANT,
    DATATYPE2 = TVARIANT,

    EACH, ELSE, ELSEIF, END, ERASE, EXIT,
    FOR, FUNCTION,
    GLOBAL, GOSUB, GOTO,
    IF, _IN_, INPUT,
    LET, LINE, LINEINPUT, LOCAL, LOOP, LPRINT, LSET,
    NAME, NEW, NEXT,
    ON, OPEN, OPTION,
    PRINT, PRIVATE, PUBLIC,
    REDIM, REM, RESUME, RETURN, RSET,
    SELECT, SET, SHARED, STATIC, STEP, STOP, SUB,
    TEXT, THEN, TO, TYPE,
    UNTIL,
    WEND, WHILE, WITH, WRITE,
    ENDIF, ENDFUNC, ENDSUB, ENDTYPE, ENDSELECT, ENDWITH,
    // Ende aller Keywords
    LASTKWD = ENDWITH,
    // Statement-Ende
    EOS, EOLN,
    // Operatoren:
    EXPON, NEG, MUL,
    DIV, IDIV, MOD, PLUS, MINUS,
    EQ, NE, LT, GT, LE, GE,
    NOT, AND, OR, XOR, EQV,
    IMP, CAT, LIKE, IS,
    // Sonstiges:
    FIRSTEXTRA,
    NUMBER=FIRSTEXTRA, FIXSTRING, SYMBOL, _CDECL_, BYVAL,
    OUTPUT, RANDOM, APPEND, BINARY, ACCESS,
    LOCK, READ, PRESERVE, BASE, ANY, LIB, _OPTIONAL_,
    EXPLICIT,

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
    JS_BIT_XOR, JS_BIT_AND, JS_EQ, JS_NE, JS_LT, JS_LE,
    JS_GT, JS_GE, JS_LSHIFT, JS_RSHIFT, JS_RSHIFT_Z,
    JS_PLUS, JS_MINUS, JS_MUL, JS_DIV, JS_MOD, JS_LOG_NOT, JS_BIT_NOT,
    JS_INC, JS_DEC, JS_LPAREN, JS_RPAREN, JS_LINDEX, JS_RINDEX
};

#ifdef SbiTokenSHAREDTMPUNDEF
#define SHARED
#undef SbiTokenSHAREDTMPUNDEF
#endif

class SbiTokenizer : public SbiScanner {
protected:
    SbiToken eCurTok;               // aktuelles Token
    SbiToken ePush;                 // Pushback-Token
    USHORT  nPLine, nPCol1, nPCol2; // Pushback-Location
    BOOL bEof;                      // TRUE bei Dateiende
    BOOL bEos;                      // TRUE bei Statement-Ende
    BOOL bKeywords;                 // TRUE, falls Keywords geparst werden
    BOOL bAs;                       // letztes Keyword war AS
public:
    SbiTokenizer( const String&, StarBASIC* = NULL );
   ~SbiTokenizer();

    inline BOOL IsEof()             { return bEof; }
    inline BOOL IsEos()             { return bEos; }

    void  Push( SbiToken );             // Pushback eines Tokens
    const String& Symbol( SbiToken );// Rueckumwandlung

    SbiToken Peek();                    // das naechste Token lesen
    SbiToken Next();                    // Ein Token lesen
    BOOL MayBeLabel( BOOL= FALSE ); // Kann es ein Label sein?

    void Hilite( SbTextPortions& ); // Syntax-Highlighting

    void Error( SbError c ) { GenError( c ); }
    void Error( SbError, SbiToken );
    void Error( SbError, const char* );
    void Error( SbError, String );

    void Keywords( BOOL b ) { bKeywords = b; }

    static BOOL IsEoln( SbiToken t )
        { return BOOL( t == EOS || t == EOLN || t == REM ); }
    static BOOL IsKwd( SbiToken t )
        { return BOOL( t >= FIRSTKWD && t <= LASTKWD ); }
    static BOOL IsExtra( SbiToken t )
        { return BOOL( t >= FIRSTEXTRA ); }
};


#endif
