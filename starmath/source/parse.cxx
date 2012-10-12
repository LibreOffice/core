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

#include <stdio.h>

#define SMDLL 1

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <i18npool/lang.h>
#include <unotools/charclass.hxx>
#include <editeng/unolingu.hxx>
#include <unotools/syslocale.hxx>
#include <sal/macros.h>
#include "parse.hxx"
#include "starmath.hrc"
#include "smdll.hxx"
#include "smmod.hxx"
#include "config.hxx"

#include "node.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;

///////////////////////////////////////////////////////////////////////////

namespace {
template < typename T >
T* lcl_popOrZero( ::std::stack<T*> & rStack )
{
    if (rStack.empty())
        return 0;
    T* pTmp = rStack.top();
    rStack.pop();
    return pTmp;
}
}

static inline bool strnccmp(const String &u1, xub_StrLen nIdx,
                              const sal_Char *s2, xub_StrLen nLen)
{
    return u1.EqualsIgnoreCaseAscii( s2, nIdx, nLen );
}

static const sal_Unicode aDelimiterTable[] =
{
    ' ',    '\t',   '\n',   '\r',   '+',    '-',    '*',    '/',    '=',    '#',
    '%',    '\\',   '"',    '~',    '`',    '>',    '<',    '&',    '|',    '(',
    ')',    '{',    '}',    '[',    ']',    '^',    '_',
    '\0'    // end of list symbol
};


static inline bool IsDigit( sal_Unicode cChar )
{
    return '0' <= cChar && cChar <= '9';
}

///////////////////////////////////////////////////////////////////////////

SmToken::SmToken() :
    eType       (TUNKNOWN),
    cMathChar   ('\0')
{
    nGroup = nCol = nRow = nLevel = 0;
}

SmToken::SmToken(SmTokenType eTokenType,
                 sal_Unicode cMath,
                 const sal_Char* pText,
                 sal_uLong nTokenGroup,
                 sal_uInt16 nTokenLevel) {
    eType = eTokenType;
    cMathChar = cMath;
    aText.AssignAscii(pText);
    nGroup = nTokenGroup;
    nLevel = nTokenLevel;
    nCol = nRow = 0;
}

///////////////////////////////////////////////////////////////////////////


static const SmTokenTableEntry aTokenTable[] =
{
    { "Im" , TIM, MS_IM, TGSTANDALONE, 5 },
    { "MZ23", TDEBUG, '\0', TGATTRIBUT, 0 },
    { "Re" , TRE, MS_RE, TGSTANDALONE, 5 },
    { "abs", TABS, '\0', TGUNOPER, 13 },
    { "arcosh", TACOSH, '\0', TGFUNCTION, 5 },
    { "arcoth", TACOTH, '\0', TGFUNCTION, 5 },
    { "acute", TACUTE, MS_ACUTE, TGATTRIBUT, 5 },
    { "aleph" , TALEPH, MS_ALEPH, TGSTANDALONE, 5 },
    { "alignb", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
    { "alignc", TALIGNC, '\0', TGALIGN, 0},
    { "alignl", TALIGNL, '\0', TGALIGN, 0},
    { "alignm", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
    { "alignr", TALIGNR, '\0', TGALIGN, 0},
    { "alignt", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
    { "and", TAND, MS_AND, TGPRODUCT, 0},
    { "approx", TAPPROX, MS_APPROX, TGRELATION, 0},
    { "arccos", TACOS, '\0', TGFUNCTION, 5},
    { "arccot", TACOT, '\0', TGFUNCTION, 5},
    { "arcsin", TASIN, '\0', TGFUNCTION, 5},
    { "arctan", TATAN, '\0', TGFUNCTION, 5},
    { "arsinh", TASINH, '\0', TGFUNCTION, 5},
    { "artanh", TATANH, '\0', TGFUNCTION, 5},
    { "backepsilon" , TBACKEPSILON, MS_BACKEPSILON, TGSTANDALONE, 5},
    { "bar", TBAR, MS_BAR, TGATTRIBUT, 5},
    { "binom", TBINOM, '\0', 0, 5 },
    { "black", TBLACK, '\0', TGCOLOR, 0},
    { "blue", TBLUE, '\0', TGCOLOR, 0},
    { "bold", TBOLD, '\0', TGFONTATTR, 5},
    { "boper", TBOPER, '\0', TGPRODUCT, 0},
    { "breve", TBREVE, MS_BREVE, TGATTRIBUT, 5},
    { "bslash", TBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
    { "cdot", TCDOT, MS_CDOT, TGPRODUCT, 0},
    { "check", TCHECK, MS_CHECK, TGATTRIBUT, 5},
    { "circ" , TCIRC, MS_CIRC, TGSTANDALONE, 5},
    { "circle", TCIRCLE, MS_CIRCLE, TGATTRIBUT, 5},
    { "color", TCOLOR, '\0', TGFONTATTR, 5},
    { "coprod", TCOPROD, MS_COPROD, TGOPER, 5},
    { "cos", TCOS, '\0', TGFUNCTION, 5},
    { "cosh", TCOSH, '\0', TGFUNCTION, 5},
    { "cot", TCOT, '\0', TGFUNCTION, 5},
    { "coth", TCOTH, '\0', TGFUNCTION, 5},
    { "csub", TCSUB, '\0', TGPOWER, 0},
    { "csup", TCSUP, '\0', TGPOWER, 0},
    { "cyan", TCYAN, '\0', TGCOLOR, 0},
    { "dddot", TDDDOT, MS_DDDOT, TGATTRIBUT, 5},
    { "ddot", TDDOT, MS_DDOT, TGATTRIBUT, 5},
    { "def", TDEF, MS_DEF, TGRELATION, 0},
    { "div", TDIV, MS_DIV, TGPRODUCT, 0},
    { "divides", TDIVIDES, MS_LINE, TGRELATION, 0},
    { "dlarrow" , TDLARROW, MS_DLARROW, TGSTANDALONE, 5},
    { "dlrarrow" , TDLRARROW, MS_DLRARROW, TGSTANDALONE, 5},
    { "dot", TDOT, MS_DOT, TGATTRIBUT, 5},
    { "dotsaxis", TDOTSAXIS, MS_DOTSAXIS, TGSTANDALONE, 5}, // 5 to continue expression
    { "dotsdiag", TDOTSDIAG, MS_DOTSUP, TGSTANDALONE, 5},   //
    { "dotsdown", TDOTSDOWN, MS_DOTSDOWN, TGSTANDALONE, 5},  //
    { "dotslow", TDOTSLOW, MS_DOTSLOW, TGSTANDALONE, 5},    //
    { "dotsup", TDOTSUP, MS_DOTSUP, TGSTANDALONE, 5},      //
    { "dotsvert", TDOTSVERT, MS_DOTSVERT, TGSTANDALONE, 5}, //
    { "downarrow" , TDOWNARROW, MS_DOWNARROW, TGSTANDALONE, 5},
    { "drarrow" , TDRARROW, MS_DRARROW, TGSTANDALONE, 5},
    { "emptyset" , TEMPTYSET, MS_EMPTYSET, TGSTANDALONE, 5},
    { "equiv", TEQUIV, MS_EQUIV, TGRELATION, 0},
    { "exists", TEXISTS, MS_EXISTS, TGSTANDALONE, 5},
    { "notexists", TNOTEXISTS, MS_NOTEXISTS, TGSTANDALONE, 5},
    { "exp", TEXP, '\0', TGFUNCTION, 5},
    { "fact", TFACT, MS_FACT, TGUNOPER, 5},
    { "fixed", TFIXED, '\0', TGFONT, 0},
    { "font", TFONT, '\0', TGFONTATTR, 5},
    { "forall", TFORALL, MS_FORALL, TGSTANDALONE, 5},
    { "from", TFROM, '\0', TGLIMIT, 0},
    { "func", TFUNC, '\0', TGFUNCTION, 5},
    { "ge", TGE, MS_GE, TGRELATION, 0},
    { "geslant", TGESLANT, MS_GESLANT, TGRELATION, 0 },
    { "gg", TGG, MS_GG, TGRELATION, 0},
    { "grave", TGRAVE, MS_GRAVE, TGATTRIBUT, 5},
    { "green", TGREEN, '\0', TGCOLOR, 0},
    { "gt", TGT, MS_GT, TGRELATION, 0},
    { "hat", THAT, MS_HAT, TGATTRIBUT, 5},
    { "hbar" , THBAR, MS_HBAR, TGSTANDALONE, 5},
    { "iiint", TIIINT, MS_IIINT, TGOPER, 5},
    { "iint", TIINT, MS_IINT, TGOPER, 5},
    { "in", TIN, MS_IN, TGRELATION, 0},
    { "infinity" , TINFINITY, MS_INFINITY, TGSTANDALONE, 5},
    { "infty" , TINFINITY, MS_INFINITY, TGSTANDALONE, 5},
    { "int", TINT, MS_INT, TGOPER, 5},
    { "intersection", TINTERSECT, MS_INTERSECT, TGPRODUCT, 0},
    { "ital", TITALIC, '\0', TGFONTATTR, 5},
    { "italic", TITALIC, '\0', TGFONTATTR, 5},
    { "lambdabar" , TLAMBDABAR, MS_LAMBDABAR, TGSTANDALONE, 5},
    { "langle", TLANGLE, MS_LANGLE, TGLBRACES, 5},
    { "lbrace", TLBRACE, MS_LBRACE, TGLBRACES, 5},
    { "lceil", TLCEIL, MS_LCEIL, TGLBRACES, 5},
    { "ldbracket", TLDBRACKET, MS_LDBRACKET, TGLBRACES, 5},
    { "ldline", TLDLINE, MS_DLINE, TGLBRACES, 5},
    { "le", TLE, MS_LE, TGRELATION, 0},
    { "left", TLEFT, '\0', 0, 5},
    { "leftarrow" , TLEFTARROW, MS_LEFTARROW, TGSTANDALONE, 5},
    { "leslant", TLESLANT, MS_LESLANT, TGRELATION, 0 },
    { "lfloor", TLFLOOR, MS_LFLOOR, TGLBRACES, 5},
    { "lim", TLIM, '\0', TGOPER, 5},
    { "liminf", TLIMINF, '\0', TGOPER, 5},
    { "limsup", TLIMSUP, '\0', TGOPER, 5},
    { "lint", TLINT, MS_LINT, TGOPER, 5},
    { "ll", TLL, MS_LL, TGRELATION, 0},
    { "lline", TLLINE, MS_LINE, TGLBRACES, 5},
    { "llint", TLLINT, MS_LLINT, TGOPER, 5},
    { "lllint", TLLLINT, MS_LLLINT, TGOPER, 5},
    { "ln", TLN, '\0', TGFUNCTION, 5},
    { "log", TLOG, '\0', TGFUNCTION, 5},
    { "lsub", TLSUB, '\0', TGPOWER, 0},
    { "lsup", TLSUP, '\0', TGPOWER, 0},
    { "lt", TLT, MS_LT, TGRELATION, 0},
    { "magenta", TMAGENTA, '\0', TGCOLOR, 0},
    { "matrix", TMATRIX, '\0', 0, 5},
    { "minusplus", TMINUSPLUS, MS_MINUSPLUS, TGUNOPER | TGSUM, 5},
    { "mline", TMLINE, MS_LINE, 0, 0},      //! nicht in TGRBRACES, Level 0
    { "nabla", TNABLA, MS_NABLA, TGSTANDALONE, 5},
    { "nbold", TNBOLD, '\0', TGFONTATTR, 5},
    { "ndivides", TNDIVIDES, MS_NDIVIDES, TGRELATION, 0},
    { "neg", TNEG, MS_NEG, TGUNOPER, 5 },
    { "neq", TNEQ, MS_NEQ, TGRELATION, 0},
    { "newline", TNEWLINE, '\0', 0, 0},
    { "ni", TNI, MS_NI, TGRELATION, 0},
    { "nitalic", TNITALIC, '\0', TGFONTATTR, 5},
    { "none", TNONE, '\0', TGLBRACES | TGRBRACES, 0},
    { "nospace", TNOSPACE, '\0', TGSTANDALONE, 5},
    { "notin", TNOTIN, MS_NOTIN, TGRELATION, 0},
    { "nroot", TNROOT, MS_SQRT, TGUNOPER, 5},
    { "nsubset", TNSUBSET, MS_NSUBSET, TGRELATION, 0 },
    { "nsupset", TNSUPSET, MS_NSUPSET, TGRELATION, 0 },
    { "nsubseteq", TNSUBSETEQ, MS_NSUBSETEQ, TGRELATION, 0 },
    { "nsupseteq", TNSUPSETEQ, MS_NSUPSETEQ, TGRELATION, 0 },
    { "odivide", TODIVIDE, MS_ODIVIDE, TGPRODUCT, 0},
    { "odot", TODOT, MS_ODOT, TGPRODUCT, 0},
    { "ominus", TOMINUS, MS_OMINUS, TGSUM, 0},
    { "oper", TOPER, '\0', TGOPER, 5},
    { "oplus", TOPLUS, MS_OPLUS, TGSUM, 0},
    { "or", TOR, MS_OR, TGSUM, 0},
    { "ortho", TORTHO, MS_ORTHO, TGRELATION, 0},
    { "otimes", TOTIMES, MS_OTIMES, TGPRODUCT, 0},
    { "over", TOVER, '\0', TGPRODUCT, 0},
    { "overbrace", TOVERBRACE, MS_OVERBRACE, TGPRODUCT, 5},
    { "overline", TOVERLINE, '\0', TGATTRIBUT, 5},
    { "overstrike", TOVERSTRIKE, '\0', TGATTRIBUT, 5},
    { "owns", TNI, MS_NI, TGRELATION, 0},
    { "parallel", TPARALLEL, MS_DLINE, TGRELATION, 0},
    { "partial", TPARTIAL, MS_PARTIAL, TGSTANDALONE, 5 },
    { "phantom", TPHANTOM, '\0', TGFONTATTR, 5},
    { "plusminus", TPLUSMINUS, MS_PLUSMINUS, TGUNOPER | TGSUM, 5},
    { "prec", TPRECEDES, MS_PRECEDES, TGRELATION, 0 },
    { "preccurlyeq", TPRECEDESEQUAL, MS_PRECEDESEQUAL, TGRELATION, 0 },
    { "precsim", TPRECEDESEQUIV, MS_PRECEDESEQUIV, TGRELATION, 0 },
    { "nprec", TNOTPRECEDES, MS_NOTPRECEDES, TGRELATION, 0 },
    { "prod", TPROD, MS_PROD, TGOPER, 5},
    { "prop", TPROP, MS_PROP, TGRELATION, 0},
    { "rangle", TRANGLE, MS_RANGLE, TGRBRACES, 0},  //! 0 to terminate expression
    { "rbrace", TRBRACE, MS_RBRACE, TGRBRACES, 0},  //
    { "rceil", TRCEIL, MS_RCEIL, TGRBRACES, 0}, //
    { "rdbracket", TRDBRACKET, MS_RDBRACKET, TGRBRACES, 0}, //
    { "rdline", TRDLINE, MS_DLINE, TGRBRACES, 0},   //
    { "red", TRED, '\0', TGCOLOR, 0},
    { "rfloor", TRFLOOR, MS_RFLOOR, TGRBRACES, 0},  //! 0 to terminate expression
    { "right", TRIGHT, '\0', 0, 0},
    { "rightarrow" , TRIGHTARROW, MS_RIGHTARROW, TGSTANDALONE, 5},
    { "rline", TRLINE, MS_LINE, TGRBRACES, 0},  //! 0 to terminate expression
    { "rsub", TRSUB, '\0', TGPOWER, 0},
    { "rsup", TRSUP, '\0', TGPOWER, 0},
    { "sans", TSANS, '\0', TGFONT, 0},
    { "serif", TSERIF, '\0', TGFONT, 0},
    { "setC" , TSETC, MS_SETC, TGSTANDALONE, 5},
    { "setN" , TSETN, MS_SETN, TGSTANDALONE, 5},
    { "setQ" , TSETQ, MS_SETQ, TGSTANDALONE, 5},
    { "setR" , TSETR, MS_SETR, TGSTANDALONE, 5},
    { "setZ" , TSETZ, MS_SETZ, TGSTANDALONE, 5},
    { "setminus", TBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
    { "sim", TSIM, MS_SIM, TGRELATION, 0},
    { "simeq", TSIMEQ, MS_SIMEQ, TGRELATION, 0},
    { "sin", TSIN, '\0', TGFUNCTION, 5},
    { "sinh", TSINH, '\0', TGFUNCTION, 5},
    { "size", TSIZE, '\0', TGFONTATTR, 5},
    { "slash", TSLASH, MS_SLASH, TGPRODUCT, 0 },
    { "sqrt", TSQRT, MS_SQRT, TGUNOPER, 5},
    { "stack", TSTACK, '\0', 0, 5},
    { "sub", TRSUB, '\0', TGPOWER, 0},
    { "subset", TSUBSET, MS_SUBSET, TGRELATION, 0},
    { "succ", TSUCCEEDS, MS_SUCCEEDS, TGRELATION, 0 },
    { "succcurlyeq", TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, TGRELATION, 0 },
    { "succsim", TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, TGRELATION, 0 },
    { "nsucc", TNOTSUCCEEDS, MS_NOTSUCCEEDS, TGRELATION, 0 },
    { "subseteq", TSUBSETEQ, MS_SUBSETEQ, TGRELATION, 0},
    { "sum", TSUM, MS_SUM, TGOPER, 5},
    { "sup", TRSUP, '\0', TGPOWER, 0},
    { "supset", TSUPSET, MS_SUPSET, TGRELATION, 0},
    { "supseteq", TSUPSETEQ, MS_SUPSETEQ, TGRELATION, 0},
    { "tan", TTAN, '\0', TGFUNCTION, 5},
    { "tanh", TTANH, '\0', TGFUNCTION, 5},
    { "tilde", TTILDE, MS_TILDE, TGATTRIBUT, 5},
    { "times", TTIMES, MS_TIMES, TGPRODUCT, 0},
    { "to", TTO, '\0', TGLIMIT, 0},
    { "toward", TTOWARD, MS_RIGHTARROW, TGRELATION, 0},
    { "transl", TTRANSL, MS_TRANSL, TGRELATION, 0},
    { "transr", TTRANSR, MS_TRANSR, TGRELATION, 0},
    { "underbrace", TUNDERBRACE, MS_UNDERBRACE, TGPRODUCT, 5},
    { "underline", TUNDERLINE, '\0', TGATTRIBUT, 5},
    { "union", TUNION, MS_UNION, TGSUM, 0},
    { "uoper", TUOPER, '\0', TGUNOPER, 5},
    { "uparrow" , TUPARROW, MS_UPARROW, TGSTANDALONE, 5},
    { "vec", TVEC, MS_VEC, TGATTRIBUT, 5},
    { "white", TWHITE, '\0', TGCOLOR, 0},
    { "widebslash", TWIDEBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
    { "widehat", TWIDEHAT, MS_HAT, TGATTRIBUT, 5},
    { "widetilde", TWIDETILDE, MS_TILDE, TGATTRIBUT, 5},
    { "wideslash", TWIDESLASH, MS_SLASH, TGPRODUCT, 0 },
    { "widevec", TWIDEVEC, MS_VEC, TGATTRIBUT, 5},
    { "wp" , TWP, MS_WP, TGSTANDALONE, 5},
    { "yellow", TYELLOW, '\0', TGCOLOR, 0},
    { "", TEND, '\0', 0, 0}
};

const SmTokenTableEntry * SmParser::GetTokenTableEntry( const String &rName )
{
    const SmTokenTableEntry * pRes = 0;
    if (rName.Len())
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(aTokenTable); ++i)
        {
            if (rName.EqualsIgnoreCaseAscii( aTokenTable[i].pIdent ))
            {
                pRes = &aTokenTable[i];
                break;
            }
        }
    }

    return pRes;
}


///////////////////////////////////////////////////////////////////////////

#if OSL_DEBUG_LEVEL > 1

bool SmParser::IsDelimiter( const String &rTxt, xub_StrLen nPos )
    // returns 'true' iff cChar is '\0' or a delimeter
{
    OSL_ENSURE( nPos <= rTxt.Len(), "index out of range" );

    sal_Unicode cChar = rTxt.GetChar( nPos );
    if(!cChar)
        return true;

    // check if 'cChar' is in the delimeter table
    const sal_Unicode *pDelim = &aDelimiterTable[0];
    for ( ;  *pDelim != 0;  pDelim++)
        if (*pDelim == cChar)
            break;


    sal_Int16 nTypJp = SM_MOD()->GetSysLocale().GetCharClass().getType( rTxt, nPos );
    bool bIsDelim = (*pDelim != 0 ||
            nTypJp == com::sun::star::i18n::UnicodeType::SPACE_SEPARATOR ||
            nTypJp == com::sun::star::i18n::UnicodeType::CONTROL);

    return bIsDelim;
}

#endif

void SmParser::Insert(const String &rText, sal_uInt16 nPos)
{
    m_aBufferString.Insert(rText, nPos);

    xub_StrLen  nLen = rText.Len();
    m_nBufferIndex = m_nBufferIndex + nLen;
    m_nTokenIndex  = m_nTokenIndex + nLen;
}


void SmParser::Replace( sal_uInt16 nPos, sal_uInt16 nLen, const String &rText )
{
    OSL_ENSURE( nPos + nLen <= m_aBufferString.Len(), "argument mismatch" );

    m_aBufferString.Replace( nPos, nLen, rText );
    sal_Int16  nChg = rText.Len() - nLen;
    m_nBufferIndex = m_nBufferIndex + nChg;
    m_nTokenIndex = m_nTokenIndex + nChg;
}


// First character may be any alphabetic
const sal_Int32 coStartFlags =
        KParseTokens::ANY_LETTER_OR_NUMBER |
        KParseTokens::IGNORE_LEADING_WS;

// Continuing characters may be any alphanumeric or dot.
const sal_Int32 coContFlags =
    ((coStartFlags | KParseTokens::ASC_DOT) & ~KParseTokens::IGNORE_LEADING_WS)
    | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

// First character for numbers, may be any numeric or dot
const sal_Int32 coNumStartFlags =
        KParseTokens::ASC_DIGIT |
        KParseTokens::ASC_DOT |
        KParseTokens::IGNORE_LEADING_WS;
// Continuing characters for numbers, may be any numeric or dot.
const sal_Int32 coNumContFlags =
    (coNumStartFlags | KParseTokens::ASC_DOT) & ~KParseTokens::IGNORE_LEADING_WS;

void SmParser::NextToken()
{
    static const String aEmptyStr;

    xub_StrLen  nBufLen = m_aBufferString.Len();
    ParseResult aRes;
    xub_StrLen  nRealStart;
    bool        bCont;
    bool        bNumStart = false;
    CharClass   aCC(SM_MOD()->GetSysLocale().GetCharClass().getLocale());
    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR ==
                        aCC.getType( m_aBufferString, m_nBufferIndex ))
           ++m_nBufferIndex;

        sal_Int32 nStartFlags = coStartFlags;
        sal_Int32 nContFlags  = coContFlags;
        sal_Unicode cFirstChar = m_aBufferString.GetChar( m_nBufferIndex );
        aRes = aCC.parseAnyToken( m_aBufferString, m_nBufferIndex,
                                            nStartFlags, aEmptyStr,
                                            nContFlags, aEmptyStr );

        // #i45779# parse numbers correctly
        // i.e. independent from the locale setting.
        // (note that #i11752# remains fixed)
        if ((aRes.TokenType & KParseType::IDENTNAME) && IsDigit( cFirstChar ))
        {
            ParseResult aTmpRes;
            lang::Locale aOldLoc( aCC.getLocale() );
            aCC.setLocale( m_aDotLoc );
            aTmpRes = aCC.parsePredefinedToken(
                            KParseType::ASC_NUMBER,
                            m_aBufferString, m_nBufferIndex,
                            KParseTokens::ASC_DIGIT, aEmptyStr,
                            KParseTokens::ASC_DIGIT | KParseTokens::ASC_DOT, aEmptyStr );
            aCC.setLocale( aOldLoc );
            if (aTmpRes.TokenType & KParseType::ASC_NUMBER)
                aRes.TokenType = aTmpRes.TokenType;
        }

        nRealStart = m_nBufferIndex + sal::static_int_cast< xub_StrLen >(aRes.LeadingWhiteSpace);
        m_nBufferIndex = nRealStart;

        bCont = false;
        if ( aRes.TokenType == 0  &&
                nRealStart < nBufLen &&
                '\n' == m_aBufferString.GetChar( nRealStart ) )
        {
            // keep data needed for tokens row and col entry up to date
            ++m_Row;
            m_nBufferIndex = m_nColOff = nRealStart + 1;
            bCont = true;
        }
        else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
        {
            String aName( m_aBufferString.Copy( nRealStart, 2 ));
            if ( aName.EqualsAscii( "%%" ))
            {
                //SkipComment
                m_nBufferIndex = nRealStart + 2;
                while (m_nBufferIndex < nBufLen  &&
                    '\n' != m_aBufferString.GetChar( m_nBufferIndex ))
                    ++m_nBufferIndex;
                bCont = true;
            }
        }

    } while (bCont);

    // set index of current token
    m_nTokenIndex = m_nBufferIndex;

    m_aCurToken.nRow   = m_Row;
    m_aCurToken.nCol   = nRealStart - m_nColOff + 1;

    bool bHandled = true;
    if (nRealStart >= nBufLen)
    {
        m_aCurToken.eType    = TEND;
        m_aCurToken.cMathChar = '\0';
        m_aCurToken.nGroup       = 0;
        m_aCurToken.nLevel       = 0;
        m_aCurToken.aText.Erase();
    }
    else if ((aRes.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER))
             || (bNumStart && (aRes.TokenType & KParseType::IDENTNAME)))
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        OSL_ENSURE( n >= 0, "length < 0" );
        m_aCurToken.eType      = TNUMBER;
        m_aCurToken.cMathChar  = '\0';
        m_aCurToken.nGroup     = 0;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.Copy( nRealStart, sal::static_int_cast< xub_StrLen >(n) );

#if OSL_DEBUG_LEVEL > 1
        if (!IsDelimiter( m_aBufferString, static_cast< xub_StrLen >(aRes.EndPos) ))
        {
            OSL_FAIL( "identifier really finished? (compatibility!)" );
        }
#endif
    }
    else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
    {
        m_aCurToken.eType      = TTEXT;
        m_aCurToken.cMathChar  = '\0';
        m_aCurToken.nGroup     = 0;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText     = aRes.DequotedNameOrString;
        m_aCurToken.nRow       = m_Row;
        m_aCurToken.nCol       = nRealStart - m_nColOff + 2;
    }
    else if (aRes.TokenType & KParseType::IDENTNAME)
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        OSL_ENSURE( n >= 0, "length < 0" );
        String aName( m_aBufferString.Copy( nRealStart, sal::static_int_cast< xub_StrLen >(n) ) );
        const SmTokenTableEntry *pEntry = GetTokenTableEntry( aName );

        if (pEntry)
        {
            m_aCurToken.eType      = pEntry->eType;
            m_aCurToken.cMathChar  = pEntry->cMathChar;
            m_aCurToken.nGroup     = pEntry->nGroup;
            m_aCurToken.nLevel     = pEntry->nLevel;
            m_aCurToken.aText.AssignAscii( pEntry->pIdent );
        }
        else
        {
            m_aCurToken.eType      = TIDENT;
            m_aCurToken.cMathChar  = '\0';
            m_aCurToken.nGroup     = 0;
            m_aCurToken.nLevel     = 5;
            m_aCurToken.aText      = aName;

#if OSL_DEBUG_LEVEL > 1
            if (!IsDelimiter( m_aBufferString, static_cast< xub_StrLen >(aRes.EndPos) ))
            {
                OSL_FAIL( "identifier really finished? (compatibility!)" );
            }
#endif
        }
    }
    else if (aRes.TokenType == 0  &&  '_' == m_aBufferString.GetChar( nRealStart ))
    {
        m_aCurToken.eType    = TRSUB;
        m_aCurToken.cMathChar = '\0';
        m_aCurToken.nGroup       = TGPOWER;
        m_aCurToken.nLevel       = 0;
        m_aCurToken.aText.AssignAscii( "_" );

        aRes.EndPos = nRealStart + 1;
    }
    else if (aRes.TokenType & KParseType::BOOLEAN)
    {
        sal_Int32   &rnEndPos = aRes.EndPos;
        String  aName( m_aBufferString.Copy( nRealStart,
                        sal::static_int_cast< xub_StrLen >(rnEndPos - nRealStart) ));
        if (2 >= aName.Len())
        {
            sal_Unicode ch = aName.GetChar( 0 );
            switch (ch)
            {
                case '<':
                    {
                        if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "<<" ))
                        {
                            m_aCurToken.eType    = TLL;
                            m_aCurToken.cMathChar = MS_LL;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "<<" );

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "<=" ))
                        {
                            m_aCurToken.eType    = TLE;
                            m_aCurToken.cMathChar = MS_LE;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "<=" );

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "<>" ))
                        {
                            m_aCurToken.eType    = TNEQ;
                            m_aCurToken.cMathChar = MS_NEQ;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "<>" );

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.Copy( nRealStart, 3 ).
                                EqualsAscii( "<?>" ))
                        {
                            m_aCurToken.eType    = TPLACE;
                            m_aCurToken.cMathChar = MS_PLACE;
                            m_aCurToken.nGroup       = 0;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText.AssignAscii( "<?>" );

                            rnEndPos = nRealStart + 3;
                        }
                        else
                        {
                            m_aCurToken.eType    = TLT;
                            m_aCurToken.cMathChar = MS_LT;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "<" );
                        }
                    }
                    break;
                case '>':
                    {
                        if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( ">=" ))
                        {
                            m_aCurToken.eType    = TGE;
                            m_aCurToken.cMathChar = MS_GE;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( ">=" );

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( ">>" ))
                        {
                            m_aCurToken.eType    = TGG;
                            m_aCurToken.cMathChar = MS_GG;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( ">>" );

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TGT;
                            m_aCurToken.cMathChar = MS_GT;
                            m_aCurToken.nGroup       = TGRELATION;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( ">" );
                        }
                    }
                    break;
                default:
                    bHandled = false;
            }
        }
    }
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        sal_Int32   &rnEndPos = aRes.EndPos;
        String  aName( m_aBufferString.Copy( nRealStart,
                            sal::static_int_cast< xub_StrLen >(rnEndPos - nRealStart) ) );

        if (1 == aName.Len())
        {
            sal_Unicode ch = aName.GetChar( 0 );
            switch (ch)
            {
                case '%':
                    {
                        //! modifies aRes.EndPos

                        OSL_ENSURE( rnEndPos >= nBufLen  ||
                                    '%' != m_aBufferString.GetChar( sal::static_int_cast< xub_StrLen >(rnEndPos) ),
                                "unexpected comment start" );

                        // get identifier of user-defined character
                        ParseResult aTmpRes = aCC.parseAnyToken(
                                m_aBufferString, rnEndPos,
                                KParseTokens::ANY_LETTER,
                                aEmptyStr,
                                coContFlags,
                                aEmptyStr );

                        xub_StrLen nTmpStart = sal::static_int_cast< xub_StrLen >(rnEndPos +
                                                    aTmpRes.LeadingWhiteSpace);

                        // default setting for the case that no identifier
                        // i.e. a valid symbol-name is following the '%'
                        // character
                        m_aCurToken.eType      = TTEXT;
                        m_aCurToken.cMathChar  = '\0';
                        m_aCurToken.nGroup     = 0;
                        m_aCurToken.nLevel     = 5;
                        m_aCurToken.aText      = String();
                        m_aCurToken.nRow       = sal::static_int_cast< xub_StrLen >(m_Row);
                        m_aCurToken.nCol       = nTmpStart - m_nColOff;

                        if (aTmpRes.TokenType & KParseType::IDENTNAME)
                        {

                            xub_StrLen n = sal::static_int_cast< xub_StrLen >(aTmpRes.EndPos - nTmpStart);
                            m_aCurToken.eType      = TSPECIAL;
                            m_aCurToken.aText      = m_aBufferString.Copy( sal::static_int_cast< xub_StrLen >(nTmpStart-1), n+1 );

                            OSL_ENSURE( aTmpRes.EndPos > rnEndPos,
                                    "empty identifier" );
                            if (aTmpRes.EndPos > rnEndPos)
                                rnEndPos = aTmpRes.EndPos;
                            else
                                ++rnEndPos;
                        }

                        // if no symbol-name was found we start-over with
                        // finding the next token right afer the '%' sign.
                        // I.e. we leave rnEndPos unmodified.
                    }
                    break;
                case '[':
                    {
                        m_aCurToken.eType    = TLBRACKET;
                        m_aCurToken.cMathChar = MS_LBRACKET;
                        m_aCurToken.nGroup       = TGLBRACES;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText.AssignAscii( "[" );
                    }
                    break;
                case '\\':
                    {
                        m_aCurToken.eType    = TESCAPE;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = 0;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText.AssignAscii( "\\" );
                    }
                    break;
                case ']':
                    {
                        m_aCurToken.eType    = TRBRACKET;
                        m_aCurToken.cMathChar = MS_RBRACKET;
                        m_aCurToken.nGroup       = TGRBRACES;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "]" );
                    }
                    break;
                case '^':
                    {
                        m_aCurToken.eType    = TRSUP;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TGPOWER;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "^" );
                    }
                    break;
                case '`':
                    {
                        m_aCurToken.eType    = TSBLANK;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TGBLANK;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText.AssignAscii( "`" );
                    }
                    break;
                case '{':
                    {
                        m_aCurToken.eType    = TLGROUP;
                        m_aCurToken.cMathChar = MS_LBRACE;
                        m_aCurToken.nGroup       = 0;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText.AssignAscii( "{" );
                    }
                    break;
                case '|':
                    {
                        m_aCurToken.eType    = TOR;
                        m_aCurToken.cMathChar = MS_OR;
                        m_aCurToken.nGroup       = TGSUM;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "|" );
                    }
                    break;
                case '}':
                    {
                        m_aCurToken.eType    = TRGROUP;
                        m_aCurToken.cMathChar = MS_RBRACE;
                        m_aCurToken.nGroup       = 0;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "}" );
                    }
                    break;
                case '~':
                    {
                        m_aCurToken.eType    = TBLANK;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TGBLANK;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText.AssignAscii( "~" );
                    }
                    break;
                case '#':
                    {
                        if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "##" ))
                        {
                            m_aCurToken.eType    = TDPOUND;
                            m_aCurToken.cMathChar = '\0';
                            m_aCurToken.nGroup       = 0;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "##" );

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPOUND;
                            m_aCurToken.cMathChar = '\0';
                            m_aCurToken.nGroup       = 0;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText.AssignAscii( "#" );
                        }
                    }
                    break;
                case '&':
                    {
                        m_aCurToken.eType    = TAND;
                        m_aCurToken.cMathChar = MS_AND;
                        m_aCurToken.nGroup       = TGPRODUCT;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "&" );
                    }
                    break;
                case '(':
                    {
                        m_aCurToken.eType    = TLPARENT;
                        m_aCurToken.cMathChar = MS_LPARENT;
                        m_aCurToken.nGroup       = TGLBRACES;
                        m_aCurToken.nLevel       = 5;     //! 0 to continue expression
                        m_aCurToken.aText.AssignAscii( "(" );
                    }
                    break;
                case ')':
                    {
                        m_aCurToken.eType    = TRPARENT;
                        m_aCurToken.cMathChar = MS_RPARENT;
                        m_aCurToken.nGroup       = TGRBRACES;
                        m_aCurToken.nLevel       = 0;     //! 0 to terminate expression
                        m_aCurToken.aText.AssignAscii( ")" );
                    }
                    break;
                case '*':
                    {
                        m_aCurToken.eType    = TMULTIPLY;
                        m_aCurToken.cMathChar = MS_MULTIPLY;
                        m_aCurToken.nGroup       = TGPRODUCT;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "*" );
                    }
                    break;
                case '+':
                    {
                        if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "+-" ))
                        {
                            m_aCurToken.eType    = TPLUSMINUS;
                            m_aCurToken.cMathChar = MS_PLUSMINUS;
                            m_aCurToken.nGroup       = TGUNOPER | TGSUM;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText.AssignAscii( "+-" );

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPLUS;
                            m_aCurToken.cMathChar = MS_PLUS;
                            m_aCurToken.nGroup       = TGUNOPER | TGSUM;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText.AssignAscii( "+" );
                        }
                    }
                    break;
                case '-':
                    {
                        if (m_aBufferString.Copy( nRealStart, 2 ).
                                EqualsAscii( "-+" ))
                        {
                            m_aCurToken.eType    = TMINUSPLUS;
                            m_aCurToken.cMathChar = MS_MINUSPLUS;
                            m_aCurToken.nGroup       = TGUNOPER | TGSUM;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText.AssignAscii( "-+" );

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TMINUS;
                            m_aCurToken.cMathChar = MS_MINUS;
                            m_aCurToken.nGroup       = TGUNOPER | TGSUM;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText.AssignAscii( "-" );
                        }
                    }
                    break;
                case '.':
                    {
                        // for compatibility with SO5.2
                        // texts like .34 ...56 ... h ...78..90
                        // will be treated as numbers
                        m_aCurToken.eType     = TNUMBER;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = 0;
                        m_aCurToken.nLevel    = 5;

                        xub_StrLen nTxtStart = m_nBufferIndex;
                        sal_Unicode cChar;
                        do
                        {
                            cChar = m_aBufferString.GetChar( ++m_nBufferIndex );
                        }
                        while ( cChar == '.' || IsDigit( cChar ) );

                        m_aCurToken.aText = m_aBufferString.Copy( sal::static_int_cast< xub_StrLen >(nTxtStart),
                                                            sal::static_int_cast< xub_StrLen >(m_nBufferIndex - nTxtStart) );
                        aRes.EndPos = m_nBufferIndex;
                    }
                    break;
                case '/':
                    {
                        m_aCurToken.eType    = TDIVIDEBY;
                        m_aCurToken.cMathChar = MS_SLASH;
                        m_aCurToken.nGroup       = TGPRODUCT;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "/" );
                    }
                    break;
                case '=':
                    {
                        m_aCurToken.eType    = TASSIGN;
                        m_aCurToken.cMathChar = MS_ASSIGN;
                        m_aCurToken.nGroup       = TGRELATION;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText.AssignAscii( "=" );
                    }
                    break;
                default:
                    bHandled = false;
            }
        }
    }
    else
        bHandled = false;

    if (!bHandled)
    {
        m_aCurToken.eType      = TCHARACTER;
        m_aCurToken.cMathChar  = '\0';
        m_aCurToken.nGroup     = 0;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.Copy( nRealStart, 1 );

        aRes.EndPos = nRealStart + 1;
    }

    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = sal::static_int_cast< xub_StrLen >(aRes.EndPos);
}


////////////////////////////////////////
// grammar
//


void SmParser::Table()
{
    SmNodeArray  LineArray;

    Line();
    while (m_aCurToken.eType == TNEWLINE)
    {
        NextToken();
        Line();
    }

    if (m_aCurToken.eType != TEND)
        Error(PE_UNEXPECTED_CHAR);

    sal_uLong n = m_aNodeStack.size();

    LineArray.resize(n);

    for (sal_uLong i = 0; i < n; i++)
    {
        LineArray[n - (i + 1)] = m_aNodeStack.top();
        m_aNodeStack.pop();
    }

    SmStructureNode *pSNode = new SmTableNode(m_aCurToken);
    pSNode->SetSubNodes(LineArray);
    m_aNodeStack.push(pSNode);
}


void SmParser::Align()
    // parse alignment info (if any), then go on with rest of expression
{
    SmStructureNode *pSNode = 0;
    bool    bNeedGroupClose = false;

    if (TokenInGroup(TGALIGN))
    {
        if (CONVERT_40_TO_50 == GetConversion())
            // encapsulate expression to be aligned in group braces
            // (here group-open brace)
        {   Insert(rtl::OUString('{'), GetTokenIndex());
            bNeedGroupClose = true;

            // get first valid align statement in sequence
            // (the dominant one in 4.0) and erase all others (especially old
            // discarded tokens) from command string.
            while (TokenInGroup(TGALIGN))
            {
                if (TokenInGroup(TGDISCARDED) || pSNode)
                {
                    m_nBufferIndex = GetTokenIndex();
                    m_aBufferString.Erase(m_nBufferIndex, m_aCurToken.aText.Len());
                }
                else
                    pSNode = new SmAlignNode(m_aCurToken);

                NextToken();
            }
        }
        else
        {
            pSNode = new SmAlignNode(m_aCurToken);

            NextToken();

            // allow for just one align statement in 5.0
            if (CONVERT_40_TO_50 != GetConversion() && TokenInGroup(TGALIGN))
            {   Error(PE_DOUBLE_ALIGN);
                return;
            }
        }
    }

    Expression();

    if (bNeedGroupClose)
        Insert(rtl::OUString('}'), GetTokenIndex());

    if (pSNode)
    {
        pSNode->SetSubNodes(lcl_popOrZero(m_aNodeStack), 0);
        m_aNodeStack.push(pSNode);
    }
}


void SmParser::Line()
{
    sal_uInt16  n = 0;
    SmNodeArray  ExpressionArray;

    ExpressionArray.resize(n);

    // start with single expression that may have an alignment statement
    // (and go on with expressions that must not have alignment
    // statements in 'while' loop below. See also 'Expression()'.)
    if (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
    {   Align();
        ExpressionArray.resize(++n);
        ExpressionArray[n - 1] = lcl_popOrZero(m_aNodeStack);
    }

    while (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
    {   if (CONVERT_40_TO_50 != GetConversion())
            Expression();
        else
            Align();
        ExpressionArray.resize(++n);
        ExpressionArray[n - 1] = lcl_popOrZero(m_aNodeStack);
    }

    //If there's no expression, add an empty one.
    //this is to avoid a formula tree without any caret
    //positions, in visual formula editor.
    if(ExpressionArray.empty())
        ExpressionArray.push_back(new SmExpressionNode(SmToken()));

    SmStructureNode *pSNode = new SmLineNode(m_aCurToken);
    pSNode->SetSubNodes(ExpressionArray);
    m_aNodeStack.push(pSNode);
}


void SmParser::Expression()
{
    bool bUseExtraSpaces = true;
    if (!m_aNodeStack.empty())
    {
        SmNode *pNode = m_aNodeStack.top();
        m_aNodeStack.pop();
        if (pNode->GetToken().eType == TNOSPACE)
            bUseExtraSpaces = false;
        else
            m_aNodeStack.push(pNode);  // push the node from above again (now to be used as argument to this current 'nospace' node)
    }

    sal_uInt16       n = 0;
    SmNodeArray  RelationArray;

    RelationArray.resize(n);

    Relation();
    RelationArray.resize(++n);
    RelationArray[n - 1] = lcl_popOrZero(m_aNodeStack);

    while (m_aCurToken.nLevel >= 4)
    {   Relation();
        RelationArray.resize(++n);
        RelationArray[n - 1] = lcl_popOrZero(m_aNodeStack);
    }

    SmExpressionNode *pSNode = new SmExpressionNode(m_aCurToken);
    pSNode->SetSubNodes(RelationArray);
    pSNode->SetUseExtraSpaces(bUseExtraSpaces);
    m_aNodeStack.push(pSNode);
}


void SmParser::Relation()
{
    Sum();
    while (TokenInGroup(TGRELATION))
    {
        SmStructureNode *pSNode  = new SmBinHorNode(m_aCurToken);
        SmNode *pFirst = lcl_popOrZero(m_aNodeStack);

        OpSubSup();
        SmNode *pSecond = lcl_popOrZero(m_aNodeStack);

        Sum();

        pSNode->SetSubNodes(pFirst, pSecond, lcl_popOrZero(m_aNodeStack));
        m_aNodeStack.push(pSNode);
    }
}


void SmParser::Sum()
{
    Product();
    while (TokenInGroup(TGSUM))
    {
        SmStructureNode *pSNode  = new SmBinHorNode(m_aCurToken);
        SmNode *pFirst = lcl_popOrZero(m_aNodeStack);

        OpSubSup();
        SmNode *pSecond = lcl_popOrZero(m_aNodeStack);

        Product();

        pSNode->SetSubNodes(pFirst, pSecond, lcl_popOrZero(m_aNodeStack));
        m_aNodeStack.push(pSNode);
    }
}


void SmParser::Product()
{
    Power();

    while (TokenInGroup(TGPRODUCT))
    {   SmStructureNode *pSNode;
        SmNode *pFirst = lcl_popOrZero(m_aNodeStack),
               *pOper;
        bool bSwitchArgs = false;

        SmTokenType eType = m_aCurToken.eType;
        switch (eType)
        {
            case TOVER:
                pSNode = new SmBinVerNode(m_aCurToken);
                pOper = new SmRectangleNode(m_aCurToken);
                NextToken();
                break;

            case TBOPER:
                pSNode = new SmBinHorNode(m_aCurToken);

                NextToken();

                //Let the glyph node know it's a binary operation
                m_aCurToken.eType = TBOPER;
                m_aCurToken.nGroup = TGPRODUCT;

                GlyphSpecial();
                pOper = lcl_popOrZero(m_aNodeStack);
                break;

            case TOVERBRACE :
            case TUNDERBRACE :
                pSNode = new SmVerticalBraceNode(m_aCurToken);
                pOper = new SmMathSymbolNode(m_aCurToken);

                NextToken();
                break;

            case TWIDEBACKSLASH:
            case TWIDESLASH:
            {
                SmBinDiagonalNode *pSTmp = new SmBinDiagonalNode(m_aCurToken);
                pSTmp->SetAscending(eType == TWIDESLASH);
                pSNode = pSTmp;

                pOper = new SmPolyLineNode(m_aCurToken);
                NextToken();

                bSwitchArgs = true;
                break;
            }

            default:
                pSNode = new SmBinHorNode(m_aCurToken);

                OpSubSup();
                pOper = lcl_popOrZero(m_aNodeStack);
        }

        Power();

        if (bSwitchArgs)
        {
            //! vgl siehe SmBinDiagonalNode::Arrange
            pSNode->SetSubNodes(pFirst, lcl_popOrZero(m_aNodeStack), pOper);
        }
        else
        {
            pSNode->SetSubNodes(pFirst, pOper, lcl_popOrZero(m_aNodeStack));
        }
        m_aNodeStack.push(pSNode);
    }
}


void SmParser::SubSup(sal_uLong nActiveGroup)
{
    OSL_ENSURE(nActiveGroup == TGPOWER  ||  nActiveGroup == TGLIMIT,
               "Sm: wrong token group");

    if (!TokenInGroup(nActiveGroup))
        // already finish
        return;

    SmSubSupNode *pNode = new SmSubSupNode(m_aCurToken);
    //! Of course 'm_aCurToken' is just the first sub-/supscript token.
    //! It should be of no further interest. The positions of the
    //! sub-/supscripts will be identified by the corresponding subnodes
    //! index in the 'aSubNodes' array (enum value from 'SmSubSup').

    pNode->SetUseLimits(nActiveGroup == TGLIMIT);

    // initialize subnodes array
    SmNodeArray  aSubNodes;
    aSubNodes.resize(1 + SUBSUP_NUM_ENTRIES);
    aSubNodes[0] = lcl_popOrZero(m_aNodeStack);
    for (sal_uInt16 i = 1;  i < aSubNodes.size();  i++)
        aSubNodes[i] = NULL;

    // process all sub-/supscripts
    int  nIndex = 0;
    while (TokenInGroup(nActiveGroup))
    {   SmTokenType  eType (m_aCurToken.eType);

        // skip sub-/supscript token
        NextToken();

        // get sub-/supscript node on top of stack
        if (eType == TFROM  ||  eType == TTO)
        {
            // parse limits in old 4.0 and 5.0 style
            Relation();
        }
        else
            Term();

        switch (eType)
        {   case TRSUB :    nIndex = (int) RSUB;    break;
            case TRSUP :    nIndex = (int) RSUP;    break;
            case TFROM :
            case TCSUB :    nIndex = (int) CSUB;    break;
            case TTO :
            case TCSUP :    nIndex = (int) CSUP;    break;
            case TLSUB :    nIndex = (int) LSUB;    break;
            case TLSUP :    nIndex = (int) LSUP;    break;
            default :
                OSL_FAIL("Sm: unknown case");
        }
        nIndex++;
        OSL_ENSURE(1 <= nIndex  &&  nIndex <= 1 + SUBSUP_NUM_ENTRIES,
                   "SmParser::Power() : sub-/supscript index falsch");

        // set sub-/supscript if not already done
        if (aSubNodes[nIndex] != NULL)
            Error(PE_DOUBLE_SUBSUPSCRIPT);
        aSubNodes[nIndex] = lcl_popOrZero(m_aNodeStack);
    }

    pNode->SetSubNodes(aSubNodes);
    m_aNodeStack.push(pNode);
}


void SmParser::OpSubSup()
{
    // push operator symbol
    m_aNodeStack.push(new SmMathSymbolNode(m_aCurToken));
    // skip operator token
    NextToken();
    // get sub- supscripts if any
    if (TokenInGroup(TGPOWER))
        SubSup(TGPOWER);
}


void SmParser::Power()
{
    // get body for sub- supscripts on top of stack
    Term();

    SubSup(TGPOWER);
}


void SmParser::Blank()
{
    OSL_ENSURE(TokenInGroup(TGBLANK), "Sm : wrong token");
    SmBlankNode *pBlankNode = new SmBlankNode(m_aCurToken);

    while (TokenInGroup(TGBLANK))
    {
        pBlankNode->IncreaseBy(m_aCurToken);
        NextToken();
    }

    // Blanks am Zeilenende ignorieren wenn die entsprechende Option gesetzt ist
    if ( m_aCurToken.eType == TNEWLINE ||
             (m_aCurToken.eType == TEND && SM_MOD()->GetConfig()->IsIgnoreSpacesRight()) )
    {
        pBlankNode->Clear();
    }

    m_aNodeStack.push(pBlankNode);
}


void SmParser::Term()
{
    switch (m_aCurToken.eType)
    {
        case TESCAPE :
            Escape();
            break;

        case TNOSPACE :
        case TLGROUP :
        {
            bool bNoSpace = m_aCurToken.eType == TNOSPACE;
            if (bNoSpace)   // push 'no space' node and continue to parse expression
            {
                m_aNodeStack.push(new SmExpressionNode(m_aCurToken));
                NextToken();
            }
            if (m_aCurToken.eType != TLGROUP)
            {
                m_aNodeStack.pop();    // get rid of the 'no space' node pushed above
                Term();
            }
            else
            {
                NextToken();

                // allow for empty group
                if (m_aCurToken.eType == TRGROUP)
                {
                    if (bNoSpace)   // get rid of the 'no space' node pushed above
                        m_aNodeStack.pop();
                    SmStructureNode *pSNode = new SmExpressionNode(m_aCurToken);
                    pSNode->SetSubNodes(NULL, NULL);
                    m_aNodeStack.push(pSNode);

                    NextToken();
                }
                else    // go as usual
                {
                    Align();
                    if (m_aCurToken.eType != TRGROUP)
                        Error(PE_RGROUP_EXPECTED);
                    else
                        NextToken();
                }
            }
        }
        break;

        case TLEFT :
            Brace();
            break;

        case TBLANK :
        case TSBLANK :
            Blank();
            break;

        case TTEXT :
            m_aNodeStack.push(new SmTextNode(m_aCurToken, FNT_TEXT));
            NextToken();
            break;
        case TIDENT :
        case TCHARACTER :
            m_aNodeStack.push(new SmTextNode(m_aCurToken, FNT_VARIABLE));
            NextToken();
            break;
        case TNUMBER :
            m_aNodeStack.push(new SmTextNode(m_aCurToken, FNT_NUMBER));
            NextToken();
            break;

        case TLEFTARROW :
        case TRIGHTARROW :
        case TUPARROW :
        case TDOWNARROW :
        case TSETN :
        case TSETZ :
        case TSETQ :
        case TSETR :
        case TSETC :
        case THBAR :
        case TLAMBDABAR :
        case TCIRC :
        case TDRARROW :
        case TDLARROW :
        case TDLRARROW :
        case TBACKEPSILON :
        case TALEPH :
        case TIM :
        case TRE :
        case TWP :
        case TEMPTYSET :
        case TINFINITY :
        case TEXISTS :
        case TNOTEXISTS :
        case TFORALL :
        case TPARTIAL :
        case TNABLA :
        case TTOWARD :
        case TDOTSAXIS :
        case TDOTSDIAG :
        case TDOTSDOWN :
        case TDOTSLOW :
        case TDOTSUP :
        case TDOTSVERT :
            m_aNodeStack.push(new SmMathSymbolNode(m_aCurToken));
            NextToken();
            break;

        case TPLACE:
            m_aNodeStack.push(new SmPlaceNode(m_aCurToken));
            NextToken();
            break;

        case TSPECIAL:
            Special();
            break;

        case TBINOM:
            Binom();
            break;

        case TSTACK:
            Stack();
            break;

        case TMATRIX:
            Matrix();
            break;

        default:
            if (TokenInGroup(TGLBRACES))
            {   Brace();
            }
            else if (TokenInGroup(TGOPER))
            {   Operator();
            }
            else if (TokenInGroup(TGUNOPER))
            {   UnOper();
            }
            else if (    TokenInGroup(TGATTRIBUT)
                     ||  TokenInGroup(TGFONTATTR))
            {   SmStructureNodeArray  aArray;

                bool    bIsAttr;
                sal_uInt16  n = 0;
                while (true == (bIsAttr = TokenInGroup(TGATTRIBUT))
                       ||  TokenInGroup(TGFONTATTR))
                {   aArray.resize(n + 1);

                    if (bIsAttr)
                        Attribut();
                    else
                        FontAttribut();

                    SmNode* pTmp = lcl_popOrZero(m_aNodeStack);

                    // check if casting in following line is ok
                    OSL_ENSURE(pTmp && !pTmp->IsVisible(), "Sm : Ooops...");

                    aArray[n] = (SmStructureNode *) pTmp;
                    n++;
                }

                Power();

                SmNode *pFirstNode = lcl_popOrZero(m_aNodeStack);
                while (n > 0)
                {   aArray[n - 1]->SetSubNodes(0, pFirstNode);
                    pFirstNode = aArray[n - 1];
                    n--;
                }
                m_aNodeStack.push(pFirstNode);
            }
            else if (TokenInGroup(TGFUNCTION))
            {   if (CONVERT_40_TO_50 != GetConversion())
                {   Function();
                }
                else    // encapsulate old 4.0 style parsing in braces
                {
                    // insert opening brace
                    Insert(rtl::OUString('{'), GetTokenIndex());

                    //
                    // parse in 4.0 style
                    //
                    Function();

                    SmNode *pFunc = lcl_popOrZero(m_aNodeStack);

                    if (m_aCurToken.eType == TLPARENT)
                    {   Term();
                    }
                    else
                    {   Align();
                    }

                    // insert closing brace
                    Insert(rtl::OUString('}'), GetTokenIndex());

                    SmStructureNode *pSNode = new SmExpressionNode(pFunc->GetToken());
                    pSNode->SetSubNodes(pFunc, lcl_popOrZero(m_aNodeStack));
                    m_aNodeStack.push(pSNode);
                }
            }
            else
                Error(PE_UNEXPECTED_CHAR);
    }
}


void SmParser::Escape()
{
    NextToken();

    switch (m_aCurToken.eType)
    {
        case TLPARENT :
        case TRPARENT :
        case TLBRACKET :
        case TRBRACKET :
        case TLDBRACKET :
        case TRDBRACKET :
        case TLBRACE :
        case TLGROUP :
        case TRBRACE :
        case TRGROUP :
        case TLANGLE :
        case TRANGLE :
        case TLCEIL :
        case TRCEIL :
        case TLFLOOR :
        case TRFLOOR :
        case TLLINE :
        case TRLINE :
        case TLDLINE :
        case TRDLINE :
            break;
        default:
            Error(PE_UNEXPECTED_TOKEN);
    }

    SmNode *pNode = new SmMathSymbolNode(m_aCurToken);
    m_aNodeStack.push(pNode);

    NextToken();
}


void SmParser::Operator()
{
    if (TokenInGroup(TGOPER))
    {   SmStructureNode *pSNode = new SmOperNode(m_aCurToken);

        // put operator on top of stack
        Oper();

        if (TokenInGroup(TGLIMIT) || TokenInGroup(TGPOWER))
            SubSup(m_aCurToken.nGroup);
        SmNode *pOperator = lcl_popOrZero(m_aNodeStack);

        // get argument
        Power();

        pSNode->SetSubNodes(pOperator, lcl_popOrZero(m_aNodeStack));
        m_aNodeStack.push(pSNode);
    }
}


void SmParser::Oper()
{
    SmTokenType  eType (m_aCurToken.eType);
    SmNode      *pNode = NULL;

    switch (eType)
    {
        case TSUM :
        case TPROD :
        case TCOPROD :
        case TINT :
        case TIINT :
        case TIIINT :
        case TLINT :
        case TLLINT :
        case TLLLINT :
            pNode = new SmMathSymbolNode(m_aCurToken);
            break;

        case TLIM :
        case TLIMSUP :
        case TLIMINF :
            {
                const sal_Char* pLim = 0;
                switch (eType)
                {
                    case TLIM :     pLim = "lim";       break;
                    case TLIMSUP :  pLim = "lim sup";   break;
                    case TLIMINF :  pLim = "lim inf";   break;
                    default:
                        break;
                }
                if( pLim )
                    m_aCurToken.aText.AssignAscii( pLim );
                pNode = new SmTextNode(m_aCurToken, FNT_TEXT);
            }
            break;

        case TOVERBRACE :
        case TUNDERBRACE :
                pNode = new SmMathSymbolNode(m_aCurToken);
            break;

        case TOPER :
            NextToken();

            OSL_ENSURE(m_aCurToken.eType == TSPECIAL, "Sm: wrong token");
            pNode = new SmGlyphSpecialNode(m_aCurToken);
            break;

        default :
            OSL_FAIL("Sm: unknown case");
    }
    m_aNodeStack.push(pNode);

    NextToken();
}


void SmParser::UnOper()
{
    OSL_ENSURE(TokenInGroup(TGUNOPER), "Sm: wrong token");

    SmToken      aNodeToken = m_aCurToken;
    SmTokenType  eType      = m_aCurToken.eType;
    bool         bIsPostfix = eType == TFACT;

    SmStructureNode *pSNode;
    SmNode *pOper   = 0,
           *pExtra  = 0,
           *pArg;

    switch (eType)
    {
        case TABS :
        case TSQRT :
            NextToken();
            break;

        case TNROOT :
            NextToken();
            Power();
            pExtra = lcl_popOrZero(m_aNodeStack);
            break;

        case TUOPER :
            NextToken();
            //Let the glyph know what it is...
            m_aCurToken.eType = TUOPER;
            m_aCurToken.nGroup = TGUNOPER;
            GlyphSpecial();
            pOper = lcl_popOrZero(m_aNodeStack);
            break;

        case TPLUS :
        case TMINUS :
        case TPLUSMINUS :
        case TMINUSPLUS :
        case TNEG :
        case TFACT :
            OpSubSup();
            pOper = lcl_popOrZero(m_aNodeStack);
            break;

        default :
            Error(PE_UNOPER_EXPECTED);
    }

    // get argument
    Power();
    pArg = lcl_popOrZero(m_aNodeStack);

    if (eType == TABS)
    {   pSNode = new SmBraceNode(aNodeToken);
        pSNode->SetScaleMode(SCALE_HEIGHT);

        // build nodes for left & right lines
        // (text, group, level of the used token are of no interrest here)
        // we'll use row & column of the keyword for abs
        aNodeToken.eType = TABS;
        //
        aNodeToken.cMathChar = MS_LINE;
        SmNode* pLeft = new SmMathSymbolNode(aNodeToken);
        //
        aNodeToken.cMathChar = MS_LINE;
        SmNode* pRight = new SmMathSymbolNode(aNodeToken);

        pSNode->SetSubNodes(pLeft, pArg, pRight);
    }
    else if (eType == TSQRT  ||  eType == TNROOT)
    {   pSNode = new SmRootNode(aNodeToken);
        pOper = new SmRootSymbolNode(aNodeToken);
        pSNode->SetSubNodes(pExtra, pOper, pArg);
    }
    else
    {   pSNode = new SmUnHorNode(aNodeToken);

        if (bIsPostfix)
            pSNode->SetSubNodes(pArg, pOper);
        else
            // prefix operator
            pSNode->SetSubNodes(pOper, pArg);
    }

    m_aNodeStack.push(pSNode);
}


void SmParser::Attribut()
{
    OSL_ENSURE(TokenInGroup(TGATTRIBUT), "Sm: wrong token group");

    SmStructureNode *pSNode = new SmAttributNode(m_aCurToken);
    SmNode      *pAttr;
    SmScaleMode  eScaleMode = SCALE_NONE;

    // get appropriate node for the attribut itself
    switch (m_aCurToken.eType)
    {   case TUNDERLINE :
        case TOVERLINE :
        case TOVERSTRIKE :
            pAttr = new SmRectangleNode(m_aCurToken);
            eScaleMode = SCALE_WIDTH;
            break;

        case TWIDEVEC :
        case TWIDEHAT :
        case TWIDETILDE :
            pAttr = new SmMathSymbolNode(m_aCurToken);
            eScaleMode = SCALE_WIDTH;
            break;

        default :
            pAttr = new SmMathSymbolNode(m_aCurToken);
    }

    NextToken();

    pSNode->SetSubNodes(pAttr, 0);
    pSNode->SetScaleMode(eScaleMode);
    m_aNodeStack.push(pSNode);
}


void SmParser::FontAttribut()
{
    OSL_ENSURE(TokenInGroup(TGFONTATTR), "Sm: wrong token group");

    switch (m_aCurToken.eType)
    {
        case TITALIC :
        case TNITALIC :
        case TBOLD :
        case TNBOLD :
        case TPHANTOM :
            m_aNodeStack.push(new SmFontNode(m_aCurToken));
            NextToken();
            break;

        case TSIZE :
            FontSize();
            break;

        case TFONT :
            Font();
            break;

        case TCOLOR :
            Color();
            break;

        default :
            OSL_FAIL("Sm: unknown case");
    }
}


void SmParser::Color()
{
    OSL_ENSURE(m_aCurToken.eType == TCOLOR, "Sm : Ooops...");

    // last color rules, get that one
    SmToken  aToken;
    do
    {   NextToken();

        if (TokenInGroup(TGCOLOR))
        {   aToken = m_aCurToken;
            NextToken();
        }
        else
            Error(PE_COLOR_EXPECTED);
    } while (m_aCurToken.eType == TCOLOR);

    m_aNodeStack.push(new SmFontNode(aToken));
}


void SmParser::Font()
{
    OSL_ENSURE(m_aCurToken.eType == TFONT, "Sm : Ooops...");

    // last font rules, get that one
    SmToken  aToken;
    do
    {   NextToken();

        if (TokenInGroup(TGFONT))
        {   aToken = m_aCurToken;
            NextToken();
        }
        else
            Error(PE_FONT_EXPECTED);
    } while (m_aCurToken.eType == TFONT);

    m_aNodeStack.push(new SmFontNode(aToken));
}


// gets number used as arguments in Math formulas (e.g. 'size' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNumber(const rtl::OUString& rText)
{
    bool bPoint = false;
    const sal_Unicode* pBuffer = rText.getStr();
    for(sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
    {
        const sal_Unicode cChar = *pBuffer;
        if(cChar == '.')
        {
            if(bPoint)
                return false;
            else
                bPoint = true;
        }
        else if ( !IsDigit( cChar ) )
            return false;
    }
    return true;
}

void SmParser::FontSize()
{
    OSL_ENSURE(m_aCurToken.eType == TSIZE, "Sm : Ooops...");

    sal_uInt16   Type;
    SmFontNode *pFontNode = new SmFontNode(m_aCurToken);

    NextToken();

    switch (m_aCurToken.eType)
    {
        case TNUMBER:   Type = FNTSIZ_ABSOLUT;  break;
        case TPLUS:     Type = FNTSIZ_PLUS;     break;
        case TMINUS:    Type = FNTSIZ_MINUS;    break;
        case TMULTIPLY: Type = FNTSIZ_MULTIPLY; break;
        case TDIVIDEBY: Type = FNTSIZ_DIVIDE;   break;

        default:
            delete pFontNode;
            Error(PE_SIZE_EXPECTED);
            return;
    }

    if (Type != FNTSIZ_ABSOLUT)
    {
        NextToken();
        if (m_aCurToken.eType != TNUMBER)
        {
            delete pFontNode;
            Error(PE_SIZE_EXPECTED);
            return;
        }
    }

    // get number argument
    Fraction  aValue( 1L );
    if (lcl_IsNumber( m_aCurToken.aText ))
    {
        double fTmp = rtl::OUString(m_aCurToken.aText).toDouble();
        if (fTmp != 0.0)
        {
            aValue = fTmp;

            //!! keep the numerator and denominator from being to large
            //!! otherwise ongoing multiplications may result in overflows
            //!! (for example in SmNode::SetFontSize the font size calculated
            //!! may become 0 because of this!!! Happens e.g. for ftmp = 2.9 with Linux
            //!! or ftmp = 1.11111111111111111... (11/9) on every platform.)
            if (aValue.GetDenominator() > 1000)
            {
                long nNum   = aValue.GetNumerator();
                long nDenom = aValue.GetDenominator();
                while (nDenom > 1000)
                {
                    nNum    /= 10;
                    nDenom  /= 10;
                }
                aValue = Fraction( nNum, nDenom );
            }
        }
    }

    NextToken();

    pFontNode->SetSizeParameter(aValue, Type);
    m_aNodeStack.push(pFontNode);
}


void SmParser::Brace()
{
    OSL_ENSURE(m_aCurToken.eType == TLEFT  ||  TokenInGroup(TGLBRACES),
        "Sm: kein Klammer Ausdruck");

    SmStructureNode *pSNode  = new SmBraceNode(m_aCurToken);
    SmNode *pBody   = 0,
           *pLeft   = 0,
           *pRight  = 0;
    SmScaleMode   eScaleMode = SCALE_NONE;
    SmParseError  eError     = PE_NONE;

    if (m_aCurToken.eType == TLEFT)
    {   NextToken();

        eScaleMode = SCALE_HEIGHT;

        // check for left bracket
        if (TokenInGroup(TGLBRACES) || TokenInGroup(TGRBRACES))
        {
            pLeft = new SmMathSymbolNode(m_aCurToken);

            NextToken();
            Bracebody(true);
            pBody = lcl_popOrZero(m_aNodeStack);

            if (m_aCurToken.eType == TRIGHT)
            {   NextToken();

                // check for right bracket
                if (TokenInGroup(TGLBRACES) || TokenInGroup(TGRBRACES))
                {
                    pRight = new SmMathSymbolNode(m_aCurToken);
                    NextToken();
                }
                else
                    eError = PE_RBRACE_EXPECTED;
            }
            else
                eError = PE_RIGHT_EXPECTED;
        }
        else
            eError = PE_LBRACE_EXPECTED;
    }
    else
    {
        if (TokenInGroup(TGLBRACES))
        {
            pLeft = new SmMathSymbolNode(m_aCurToken);

            NextToken();
            Bracebody(false);
            pBody = lcl_popOrZero(m_aNodeStack);

            SmTokenType  eExpectedType = TUNKNOWN;
            switch (pLeft->GetToken().eType)
            {   case TLPARENT :     eExpectedType = TRPARENT;   break;
                case TLBRACKET :    eExpectedType = TRBRACKET;  break;
                case TLBRACE :      eExpectedType = TRBRACE;    break;
                case TLDBRACKET :   eExpectedType = TRDBRACKET; break;
                case TLLINE :       eExpectedType = TRLINE;     break;
                case TLDLINE :      eExpectedType = TRDLINE;    break;
                case TLANGLE :      eExpectedType = TRANGLE;    break;
                case TLFLOOR :      eExpectedType = TRFLOOR;    break;
                case TLCEIL :       eExpectedType = TRCEIL;     break;
                default :
                    OSL_FAIL("Sm: unknown case");
            }

            if (m_aCurToken.eType == eExpectedType)
            {
                pRight = new SmMathSymbolNode(m_aCurToken);
                NextToken();
            }
            else
                eError = PE_PARENT_MISMATCH;
        }
        else
            eError = PE_LBRACE_EXPECTED;
    }

    if (eError == PE_NONE)
    {   OSL_ENSURE(pLeft,  "Sm: NULL pointer");
        OSL_ENSURE(pRight, "Sm: NULL pointer");
        pSNode->SetSubNodes(pLeft, pBody, pRight);
        pSNode->SetScaleMode(eScaleMode);
        m_aNodeStack.push(pSNode);
    }
    else
    {   delete pSNode;
        delete pBody;
        delete pLeft;
        delete pRight;

        Error(eError);
    }
}


void SmParser::Bracebody(bool bIsLeftRight)
{
    SmStructureNode *pBody = new SmBracebodyNode(m_aCurToken);
    SmNodeArray      aNodes;
    sal_uInt16           nNum = 0;

    // get body if any
    if (bIsLeftRight)
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                m_aNodeStack.push(new SmMathSymbolNode(m_aCurToken));
                NextToken();
                nNum++;
            }
            else if (m_aCurToken.eType != TRIGHT)
            {   Align();
                nNum++;

                if (m_aCurToken.eType != TMLINE  &&  m_aCurToken.eType != TRIGHT)
                    Error(PE_RIGHT_EXPECTED);
            }
        } while (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TRIGHT);
    }
    else
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                m_aNodeStack.push(new SmMathSymbolNode(m_aCurToken));
                NextToken();
                nNum++;
            }
            else if (!TokenInGroup(TGRBRACES))
            {   Align();
                nNum++;

                if (m_aCurToken.eType != TMLINE  &&  !TokenInGroup(TGRBRACES))
                    Error(PE_RBRACE_EXPECTED);
            }
        } while (m_aCurToken.eType != TEND  &&  !TokenInGroup(TGRBRACES));
    }

    // build argument vector in parsing order
    aNodes.resize(nNum);
    for (sal_uInt16 i = 0;  i < nNum;  i++)
    {
        aNodes[nNum - 1 - i] = lcl_popOrZero(m_aNodeStack);
    }

    pBody->SetSubNodes(aNodes);
    pBody->SetScaleMode(bIsLeftRight ? SCALE_HEIGHT : SCALE_NONE);
    m_aNodeStack.push(pBody);
}


void SmParser::Function()
{
    switch (m_aCurToken.eType)
    {
        case TFUNC:
            NextToken();    // skip "FUNC"-statement
            // fall through

        case TSIN :
        case TCOS :
        case TTAN :
        case TCOT :
        case TASIN :
        case TACOS :
        case TATAN :
        case TACOT :
        case TSINH :
        case TCOSH :
        case TTANH :
        case TCOTH :
        case TASINH :
        case TACOSH :
        case TATANH :
        case TACOTH :
        case TLN :
        case TLOG :
        case TEXP :
            m_aNodeStack.push(new SmTextNode(m_aCurToken, FNT_FUNCTION));
            NextToken();
            break;

        default:
            Error(PE_FUNC_EXPECTED);
    }
}


void SmParser::Binom()
{
    SmNodeArray  ExpressionArray;
    SmStructureNode *pSNode = new SmTableNode(m_aCurToken);

    NextToken();

    Sum();
    Sum();

    ExpressionArray.resize(2);

    for (int i = 0;  i < 2;  i++)
    {
        ExpressionArray[2 - (i + 1)] = lcl_popOrZero(m_aNodeStack);
    }

    pSNode->SetSubNodes(ExpressionArray);
    m_aNodeStack.push(pSNode);
}


void SmParser::Stack()
{
    SmNodeArray  ExpressionArray;
    NextToken();
    if (m_aCurToken.eType == TLGROUP)
    {
        sal_uInt16 n = 0;

        do
        {
            NextToken();
            Align();
            n++;
        }
        while (m_aCurToken.eType == TPOUND);

        ExpressionArray.resize(n);

        for (sal_uInt16 i = 0; i < n; i++)
        {
            ExpressionArray[n - (i + 1)] = lcl_popOrZero(m_aNodeStack);
        }

        if (m_aCurToken.eType != TRGROUP)
            Error(PE_RGROUP_EXPECTED);

        NextToken();

        //We need to let the table node know it context
        //it's used in SmNodeToTextVisitor
        SmToken aTok = m_aCurToken;
        aTok.eType = TSTACK;
        SmStructureNode *pSNode = new SmTableNode(aTok);
        pSNode->SetSubNodes(ExpressionArray);
        m_aNodeStack.push(pSNode);
    }
    else
        Error(PE_LGROUP_EXPECTED);
}


void SmParser::Matrix()
{
    SmNodeArray  ExpressionArray;

    NextToken();
    if (m_aCurToken.eType == TLGROUP)
    {
        sal_uInt16 c = 0;

        do
        {
            NextToken();
            Align();
            c++;
        }
        while (m_aCurToken.eType == TPOUND);

        sal_uInt16 r = 1;

        while (m_aCurToken.eType == TDPOUND)
        {
            NextToken();
            for (sal_uInt16 i = 0; i < c; i++)
            {
                Align();
                if (i < (c - 1))
                {
                    if (m_aCurToken.eType == TPOUND)
                    {
                        NextToken();
                    }
                    else
                        Error(PE_POUND_EXPECTED);
                }
            }

            r++;
        }

        long nRC = r * c;

        ExpressionArray.resize(nRC);

        for (sal_uInt16 i = 0; i < (nRC); i++)
        {
            ExpressionArray[(nRC) - (i + 1)] = lcl_popOrZero(m_aNodeStack);
        }

        if (m_aCurToken.eType != TRGROUP)
            Error(PE_RGROUP_EXPECTED);

        NextToken();

        SmMatrixNode *pMNode = new SmMatrixNode(m_aCurToken);
        pMNode->SetSubNodes(ExpressionArray);
        pMNode->SetRowCol(r, c);
        m_aNodeStack.push(pMNode);
    }
    else
        Error(PE_LGROUP_EXPECTED);
}


void SmParser::Special()
{
    bool bReplace = false;
    String &rName = m_aCurToken.aText;
    String aNewName;

    if (CONVERT_NONE == GetConversion())
    {
        // conversion of symbol names for 6.0 (XML) file format
        // (name change on import / export.
        // UI uses localized names XML file format does not.)
        if( rName.Len() && rName.GetChar( 0 ) == sal_Unicode( '%' ) )
        {
            if (IsImportSymbolNames())
            {
                const SmLocalizedSymbolData &rLSD = SM_MOD()->GetLocSymbolData();
                aNewName = rLSD.GetUiSymbolName( rName.Copy( 1 ) );
                bReplace = true;
            }
            else if (IsExportSymbolNames())
            {
                const SmLocalizedSymbolData &rLSD = SM_MOD()->GetLocSymbolData();
                aNewName = rLSD.GetExportSymbolName( rName.Copy( 1 ) );
                bReplace = true;
            }
        }
        if( aNewName.Len() )
            aNewName.Insert( '%', 0 );
    }
    else    // 5.0 <-> 6.0 formula text (symbol name) conversion
    {
        LanguageType nLanguage = GetLanguage();
        SmLocalizedSymbolData &rData = SM_MOD()->GetLocSymbolData();
        const ResStringArray *pFrom = 0;
        const ResStringArray *pTo   = 0;
        if (CONVERT_50_TO_60 == GetConversion())
        {
            pFrom = rData.Get50NamesArray( nLanguage );
            pTo   = rData.Get60NamesArray( nLanguage );
        }
        else if (CONVERT_60_TO_50 == GetConversion())
        {
            pFrom = rData.Get60NamesArray( nLanguage );
            pTo   = rData.Get50NamesArray( nLanguage );
        }
        if (pFrom  &&  pTo)
        {
            OSL_ENSURE( pFrom->Count() == pTo->Count(),
                    "array length mismatch" );
            sal_uInt16 nCount = sal::static_int_cast< sal_uInt16 >(pFrom->Count());
            for (sal_uInt16 i = 0;  i < nCount;  ++i)
            {
                if (pFrom->GetString(i).equals(rName))
                {
                    aNewName = pTo->GetString(i);
                    bReplace = true;
                }
            }
        }
        // else:
        // conversion arrays not found or (usually)
        // conversion not necessary
    }

    if (bReplace  &&  aNewName.Len()  &&  rName != aNewName)
    {
        Replace( GetTokenIndex(), rName.Len(), aNewName );
        rName = aNewName;
    }

    // add symbol name to list of used symbols
    const String aSymbolName( m_aCurToken.aText.Copy( 1 ) );
    if (aSymbolName.Len() > 0 )
        AddToUsedSymbols( aSymbolName );

    m_aNodeStack.push(new SmSpecialNode(m_aCurToken));
    NextToken();
}


void SmParser::GlyphSpecial()
{
    m_aNodeStack.push(new SmGlyphSpecialNode(m_aCurToken));
    NextToken();
}


void SmParser::Error(SmParseError eError)
{
    SmStructureNode *pSNode = new SmExpressionNode(m_aCurToken);
    SmErrorNode     *pErr   = new SmErrorNode(eError, m_aCurToken);
    pSNode->SetSubNodes(pErr, 0);

    //! put a structure node on the stack (instead of the error node itself)
    //! because sometimes such a node is expected in order to attach some
    //! subnodes
    m_aNodeStack.push(pSNode);

    AddError(eError, pSNode);

    NextToken();
}


// end gramar


SmParser::SmParser()
    : m_aDotLoc( SvxCreateLocale( LANGUAGE_ENGLISH_US ) )
{
    m_eConversion = CONVERT_NONE;
    bImportSymNames = m_bExportSymNames = false;
    m_nLang = Application::GetSettings().GetUILanguage();
}

SmNode *SmParser::Parse(const String &rBuffer)
{
    ClearUsedSymbols();

    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_Row           = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    for ( size_t i = 0, n = m_aErrDescList.size(); i < n; ++i )
        delete m_aErrDescList[ i ];
    m_aErrDescList.clear();

    while ( !m_aNodeStack.empty() )
        m_aNodeStack.pop();

    SetLanguage( Application::GetSettings().GetUILanguage() );
    NextToken();
    Table();

    SmNode* result = lcl_popOrZero(m_aNodeStack);
    return result;
}

SmNode *SmParser::ParseExpression(const OUString &rBuffer)
{
    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_Row           = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    for ( size_t i = 0, n = m_aErrDescList.size(); i < n; ++i )
        delete m_aErrDescList[ i ];
    m_aErrDescList.clear();

    while ( !m_aNodeStack.empty() )
        m_aNodeStack.pop();

    SetLanguage( Application::GetSettings().GetUILanguage() );
    NextToken();
    Expression();

    SmNode* result = lcl_popOrZero(m_aNodeStack);
    return result;
}


size_t SmParser::AddError(SmParseError Type, SmNode *pNode)
{
    SmErrorDesc *pErrDesc = new SmErrorDesc;

    pErrDesc->Type  = Type;
    pErrDesc->pNode = pNode;
    pErrDesc->Text  = String(SmResId(RID_ERR_IDENT));

    sal_uInt16  nRID;
    switch (Type)
    {
        case PE_UNEXPECTED_CHAR:     nRID = RID_ERR_UNEXPECTEDCHARACTER;    break;
        case PE_LGROUP_EXPECTED:     nRID = RID_ERR_LGROUPEXPECTED;         break;
        case PE_RGROUP_EXPECTED:     nRID = RID_ERR_RGROUPEXPECTED;         break;
        case PE_LBRACE_EXPECTED:     nRID = RID_ERR_LBRACEEXPECTED;         break;
        case PE_RBRACE_EXPECTED:     nRID = RID_ERR_RBRACEEXPECTED;         break;
        case PE_FUNC_EXPECTED:       nRID = RID_ERR_FUNCEXPECTED;           break;
        case PE_UNOPER_EXPECTED:     nRID = RID_ERR_UNOPEREXPECTED;         break;
        case PE_BINOPER_EXPECTED:    nRID = RID_ERR_BINOPEREXPECTED;        break;
        case PE_SYMBOL_EXPECTED:     nRID = RID_ERR_SYMBOLEXPECTED;         break;
        case PE_IDENTIFIER_EXPECTED: nRID = RID_ERR_IDENTEXPECTED;          break;
        case PE_POUND_EXPECTED:      nRID = RID_ERR_POUNDEXPECTED;          break;
        case PE_COLOR_EXPECTED:      nRID = RID_ERR_COLOREXPECTED;          break;
        case PE_RIGHT_EXPECTED:      nRID = RID_ERR_RIGHTEXPECTED;          break;

        default:
            nRID = RID_ERR_UNKOWN;
    }
    pErrDesc->Text += SM_RESSTR(nRID);

    m_aErrDescList.push_back( pErrDesc );

    return m_aErrDescList.size()-1;
}


const SmErrorDesc *SmParser::NextError()
{
    if ( !m_aErrDescList.empty() )
        if (m_nCurError > 0) return m_aErrDescList[ --m_nCurError ];
        else
        {
            m_nCurError = 0;
            return m_aErrDescList[ m_nCurError ];
        }
    else return NULL;
}


const SmErrorDesc *SmParser::PrevError()
{
    if ( !m_aErrDescList.empty() )
        if (m_nCurError < (int) (m_aErrDescList.size() - 1)) return m_aErrDescList[ ++m_nCurError ];
        else
        {
            m_nCurError = (int) (m_aErrDescList.size() - 1);
            return m_aErrDescList[ m_nCurError ];
        }
    else return NULL;
}


const SmErrorDesc *SmParser::GetError(size_t i)
{
    if ( i < m_aErrDescList.size() )
        return m_aErrDescList[ i ];

    if ( (size_t)m_nCurError < m_aErrDescList.size() )
        return m_aErrDescList[ m_nCurError ];

    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
