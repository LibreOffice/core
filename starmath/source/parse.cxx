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

#include <com/sun/star/i18n/UnicodeType.hpp>
#include <i18nlangtag/lang.h>
#include <editeng/unolingu.hxx>
#include <unotools/syslocale.hxx>
#include <sal/macros.h>
#include <o3tl/make_unique.hxx>
#include <vcl/settings.hxx>
#include "parse.hxx"
#include "starmath.hrc"
#include "smdll.hxx"
#include "smmod.hxx"
#include "cfgitem.hxx"
#include <cassert>
#include <stack>

using namespace ::com::sun::star::i18n;


SmToken::SmToken()
    : eType(TUNKNOWN)
    , cMathChar('\0')
    , nGroup(TG::NONE)
    , nLevel(0)
    , nRow(0)
    , nCol(0)
{
}

SmToken::SmToken(SmTokenType eTokenType,
                 sal_Unicode cMath,
                 const sal_Char* pText,
                 TG nTokenGroup,
                 sal_uInt16 nTokenLevel)
    : aText(OUString::createFromAscii(pText))
    , eType(eTokenType)
    , cMathChar(cMath)
    , nGroup(nTokenGroup)
    , nLevel(nTokenLevel)
    , nRow(0)
    , nCol(0)
{
}


static const SmTokenTableEntry aTokenTable[] =
{
    { "Im" , TIM, MS_IM, TG::Standalone, 5 },
    { "Re" , TRE, MS_RE, TG::Standalone, 5 },
    { "abs", TABS, '\0', TG::UnOper, 13 },
    { "arcosh", TACOSH, '\0', TG::Function, 5 },
    { "arcoth", TACOTH, '\0', TG::Function, 5 },
    { "acute", TACUTE, MS_ACUTE, TG::Attribute, 5 },
    { "aleph" , TALEPH, MS_ALEPH, TG::Standalone, 5 },
    { "alignb", TALIGNC, '\0', TG::Align, 0},
    { "alignc", TALIGNC, '\0', TG::Align, 0},
    { "alignl", TALIGNL, '\0', TG::Align, 0},
    { "alignm", TALIGNC, '\0', TG::Align, 0},
    { "alignr", TALIGNR, '\0', TG::Align, 0},
    { "alignt", TALIGNC, '\0', TG::Align, 0},
    { "and", TAND, MS_AND, TG::Product, 0},
    { "approx", TAPPROX, MS_APPROX, TG::Relation, 0},
    { "aqua", TAQUA, '\0', TG::Color, 0},
    { "arccos", TACOS, '\0', TG::Function, 5},
    { "arccot", TACOT, '\0', TG::Function, 5},
    { "arcsin", TASIN, '\0', TG::Function, 5},
    { "arctan", TATAN, '\0', TG::Function, 5},
    { "arsinh", TASINH, '\0', TG::Function, 5},
    { "artanh", TATANH, '\0', TG::Function, 5},
    { "backepsilon" , TBACKEPSILON, MS_BACKEPSILON, TG::Standalone, 5},
    { "bar", TBAR, MS_BAR, TG::Attribute, 5},
    { "binom", TBINOM, '\0', TG::NONE, 5 },
    { "black", TBLACK, '\0', TG::Color, 0},
    { "blue", TBLUE, '\0', TG::Color, 0},
    { "bold", TBOLD, '\0', TG::FontAttr, 5},
    { "boper", TBOPER, '\0', TG::Product, 0},
    { "breve", TBREVE, MS_BREVE, TG::Attribute, 5},
    { "bslash", TBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
    { "cdot", TCDOT, MS_CDOT, TG::Product, 0},
    { "check", TCHECK, MS_CHECK, TG::Attribute, 5},
    { "circ" , TCIRC, MS_CIRC, TG::Standalone, 5},
    { "circle", TCIRCLE, MS_CIRCLE, TG::Attribute, 5},
    { "color", TCOLOR, '\0', TG::FontAttr, 5},
    { "coprod", TCOPROD, MS_COPROD, TG::Oper, 5},
    { "cos", TCOS, '\0', TG::Function, 5},
    { "cosh", TCOSH, '\0', TG::Function, 5},
    { "cot", TCOT, '\0', TG::Function, 5},
    { "coth", TCOTH, '\0', TG::Function, 5},
    { "csub", TCSUB, '\0', TG::Power, 0},
    { "csup", TCSUP, '\0', TG::Power, 0},
    { "cyan", TCYAN, '\0', TG::Color, 0},
    { "dddot", TDDDOT, MS_DDDOT, TG::Attribute, 5},
    { "ddot", TDDOT, MS_DDOT, TG::Attribute, 5},
    { "def", TDEF, MS_DEF, TG::Relation, 0},
    { "div", TDIV, MS_DIV, TG::Product, 0},
    { "divides", TDIVIDES, MS_LINE, TG::Relation, 0},
    { "dlarrow" , TDLARROW, MS_DLARROW, TG::Standalone, 5},
    { "dlrarrow" , TDLRARROW, MS_DLRARROW, TG::Standalone, 5},
    { "dot", TDOT, MS_DOT, TG::Attribute, 5},
    { "dotsaxis", TDOTSAXIS, MS_DOTSAXIS, TG::Standalone, 5}, // 5 to continue expression
    { "dotsdiag", TDOTSDIAG, MS_DOTSUP, TG::Standalone, 5},
    { "dotsdown", TDOTSDOWN, MS_DOTSDOWN, TG::Standalone, 5},
    { "dotslow", TDOTSLOW, MS_DOTSLOW, TG::Standalone, 5},
    { "dotsup", TDOTSUP, MS_DOTSUP, TG::Standalone, 5},
    { "dotsvert", TDOTSVERT, MS_DOTSVERT, TG::Standalone, 5},
    { "downarrow" , TDOWNARROW, MS_DOWNARROW, TG::Standalone, 5},
    { "drarrow" , TDRARROW, MS_DRARROW, TG::Standalone, 5},
    { "emptyset" , TEMPTYSET, MS_EMPTYSET, TG::Standalone, 5},
    { "equiv", TEQUIV, MS_EQUIV, TG::Relation, 0},
    { "exists", TEXISTS, MS_EXISTS, TG::Standalone, 5},
    { "notexists", TNOTEXISTS, MS_NOTEXISTS, TG::Standalone, 5},
    { "exp", TEXP, '\0', TG::Function, 5},
    { "fact", TFACT, MS_FACT, TG::UnOper, 5},
    { "fixed", TFIXED, '\0', TG::Font, 0},
    { "font", TFONT, '\0', TG::FontAttr, 5},
    { "forall", TFORALL, MS_FORALL, TG::Standalone, 5},
    { "from", TFROM, '\0', TG::Limit, 0},
    { "fuchsia", TFUCHSIA, '\0', TG::Color, 0},
    { "func", TFUNC, '\0', TG::Function, 5},
    { "ge", TGE, MS_GE, TG::Relation, 0},
    { "geslant", TGESLANT, MS_GESLANT, TG::Relation, 0 },
    { "gg", TGG, MS_GG, TG::Relation, 0},
    { "grave", TGRAVE, MS_GRAVE, TG::Attribute, 5},
    { "gray", TGRAY, '\0', TG::Color, 0},
    { "green", TGREEN, '\0', TG::Color, 0},
    { "gt", TGT, MS_GT, TG::Relation, 0},
    { "hat", THAT, MS_HAT, TG::Attribute, 5},
    { "hbar" , THBAR, MS_HBAR, TG::Standalone, 5},
    { "iiint", TIIINT, MS_IIINT, TG::Oper, 5},
    { "iint", TIINT, MS_IINT, TG::Oper, 5},
    { "in", TIN, MS_IN, TG::Relation, 0},
    { "infinity" , TINFINITY, MS_INFINITY, TG::Standalone, 5},
    { "infty" , TINFINITY, MS_INFINITY, TG::Standalone, 5},
    { "int", TINT, MS_INT, TG::Oper, 5},
    { "intd", TINTD, MS_INT, TG::Oper, 5},
    { "intersection", TINTERSECT, MS_INTERSECT, TG::Product, 0},
    { "ital", TITALIC, '\0', TG::FontAttr, 5},
    { "italic", TITALIC, '\0', TG::FontAttr, 5},
    { "lambdabar" , TLAMBDABAR, MS_LAMBDABAR, TG::Standalone, 5},
    { "langle", TLANGLE, MS_LMATHANGLE, TG::LBrace, 5},
    { "lbrace", TLBRACE, MS_LBRACE, TG::LBrace, 5},
    { "lceil", TLCEIL, MS_LCEIL, TG::LBrace, 5},
    { "ldbracket", TLDBRACKET, MS_LDBRACKET, TG::LBrace, 5},
    { "ldline", TLDLINE, MS_DVERTLINE, TG::LBrace, 5},
    { "le", TLE, MS_LE, TG::Relation, 0},
    { "left", TLEFT, '\0', TG::NONE, 5},
    { "leftarrow" , TLEFTARROW, MS_LEFTARROW, TG::Standalone, 5},
    { "leslant", TLESLANT, MS_LESLANT, TG::Relation, 0 },
    { "lfloor", TLFLOOR, MS_LFLOOR, TG::LBrace, 5},
    { "lim", TLIM, '\0', TG::Oper, 5},
    { "lime", TLIME, '\0', TG::Color, 0},
    { "liminf", TLIMINF, '\0', TG::Oper, 5},
    { "limsup", TLIMSUP, '\0', TG::Oper, 5},
    { "lint", TLINT, MS_LINT, TG::Oper, 5},
    { "ll", TLL, MS_LL, TG::Relation, 0},
    { "lline", TLLINE, MS_VERTLINE, TG::LBrace, 5},
    { "llint", TLLINT, MS_LLINT, TG::Oper, 5},
    { "lllint", TLLLINT, MS_LLLINT, TG::Oper, 5},
    { "ln", TLN, '\0', TG::Function, 5},
    { "log", TLOG, '\0', TG::Function, 5},
    { "lsub", TLSUB, '\0', TG::Power, 0},
    { "lsup", TLSUP, '\0', TG::Power, 0},
    { "lt", TLT, MS_LT, TG::Relation, 0},
    { "magenta", TMAGENTA, '\0', TG::Color, 0},
    { "maroon", TMAROON, '\0', TG::Color, 0},
    { "matrix", TMATRIX, '\0', TG::NONE, 5},
    { "minusplus", TMINUSPLUS, MS_MINUSPLUS, TG::UnOper | TG::Sum, 5},
    { "mline", TMLINE, MS_VERTLINE, TG::NONE, 0},      //! not in TG::RBrace, Level 0
    { "nabla", TNABLA, MS_NABLA, TG::Standalone, 5},
    { "navy", TNAVY, '\0', TG::Color, 0},
    { "nbold", TNBOLD, '\0', TG::FontAttr, 5},
    { "ndivides", TNDIVIDES, MS_NDIVIDES, TG::Relation, 0},
    { "neg", TNEG, MS_NEG, TG::UnOper, 5 },
    { "neq", TNEQ, MS_NEQ, TG::Relation, 0},
    { "newline", TNEWLINE, '\0', TG::NONE, 0},
    { "ni", TNI, MS_NI, TG::Relation, 0},
    { "nitalic", TNITALIC, '\0', TG::FontAttr, 5},
    { "none", TNONE, '\0', TG::LBrace | TG::RBrace, 0},
    { "nospace", TNOSPACE, '\0', TG::Standalone, 5},
    { "notin", TNOTIN, MS_NOTIN, TG::Relation, 0},
    { "nroot", TNROOT, MS_SQRT, TG::UnOper, 5},
    { "nsubset", TNSUBSET, MS_NSUBSET, TG::Relation, 0 },
    { "nsupset", TNSUPSET, MS_NSUPSET, TG::Relation, 0 },
    { "nsubseteq", TNSUBSETEQ, MS_NSUBSETEQ, TG::Relation, 0 },
    { "nsupseteq", TNSUPSETEQ, MS_NSUPSETEQ, TG::Relation, 0 },
    { "odivide", TODIVIDE, MS_ODIVIDE, TG::Product, 0},
    { "odot", TODOT, MS_ODOT, TG::Product, 0},
    { "olive", TOLIVE, '\0', TG::Color, 0},
    { "ominus", TOMINUS, MS_OMINUS, TG::Sum, 0},
    { "oper", TOPER, '\0', TG::Oper, 5},
    { "oplus", TOPLUS, MS_OPLUS, TG::Sum, 0},
    { "or", TOR, MS_OR, TG::Sum, 0},
    { "ortho", TORTHO, MS_ORTHO, TG::Relation, 0},
    { "otimes", TOTIMES, MS_OTIMES, TG::Product, 0},
    { "over", TOVER, '\0', TG::Product, 0},
    { "overbrace", TOVERBRACE, MS_OVERBRACE, TG::Product, 5},
    { "overline", TOVERLINE, '\0', TG::Attribute, 5},
    { "overstrike", TOVERSTRIKE, '\0', TG::Attribute, 5},
    { "owns", TNI, MS_NI, TG::Relation, 0},
    { "parallel", TPARALLEL, MS_DLINE, TG::Relation, 0},
    { "partial", TPARTIAL, MS_PARTIAL, TG::Standalone, 5 },
    { "phantom", TPHANTOM, '\0', TG::FontAttr, 5},
    { "plusminus", TPLUSMINUS, MS_PLUSMINUS, TG::UnOper | TG::Sum, 5},
    { "prec", TPRECEDES, MS_PRECEDES, TG::Relation, 0 },
    { "preccurlyeq", TPRECEDESEQUAL, MS_PRECEDESEQUAL, TG::Relation, 0 },
    { "precsim", TPRECEDESEQUIV, MS_PRECEDESEQUIV, TG::Relation, 0 },
    { "nprec", TNOTPRECEDES, MS_NOTPRECEDES, TG::Relation, 0 },
    { "prod", TPROD, MS_PROD, TG::Oper, 5},
    { "prop", TPROP, MS_PROP, TG::Relation, 0},
    { "purple", TPURPLE, '\0', TG::Color, 0},
    { "rangle", TRANGLE, MS_RMATHANGLE, TG::RBrace, 0},  //! 0 to terminate expression
    { "rbrace", TRBRACE, MS_RBRACE, TG::RBrace, 0},
    { "rceil", TRCEIL, MS_RCEIL, TG::RBrace, 0},
    { "rdbracket", TRDBRACKET, MS_RDBRACKET, TG::RBrace, 0},
    { "rdline", TRDLINE, MS_DVERTLINE, TG::RBrace, 0},
    { "red", TRED, '\0', TG::Color, 0},
    { "rfloor", TRFLOOR, MS_RFLOOR, TG::RBrace, 0},  //! 0 to terminate expression
    { "right", TRIGHT, '\0', TG::NONE, 0},
    { "rightarrow" , TRIGHTARROW, MS_RIGHTARROW, TG::Standalone, 5},
    { "rline", TRLINE, MS_VERTLINE, TG::RBrace, 0},  //! 0 to terminate expression
    { "rsub", TRSUB, '\0', TG::Power, 0},
    { "rsup", TRSUP, '\0', TG::Power, 0},
    { "sans", TSANS, '\0', TG::Font, 0},
    { "serif", TSERIF, '\0', TG::Font, 0},
    { "setC" , TSETC, MS_SETC, TG::Standalone, 5},
    { "setN" , TSETN, MS_SETN, TG::Standalone, 5},
    { "setQ" , TSETQ, MS_SETQ, TG::Standalone, 5},
    { "setR" , TSETR, MS_SETR, TG::Standalone, 5},
    { "setZ" , TSETZ, MS_SETZ, TG::Standalone, 5},
    { "setminus", TBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
    { "silver", TSILVER, '\0', TG::Color, 0},
    { "sim", TSIM, MS_SIM, TG::Relation, 0},
    { "simeq", TSIMEQ, MS_SIMEQ, TG::Relation, 0},
    { "sin", TSIN, '\0', TG::Function, 5},
    { "sinh", TSINH, '\0', TG::Function, 5},
    { "size", TSIZE, '\0', TG::FontAttr, 5},
    { "slash", TSLASH, MS_SLASH, TG::Product, 0 },
    { "sqrt", TSQRT, MS_SQRT, TG::UnOper, 5},
    { "stack", TSTACK, '\0', TG::NONE, 5},
    { "sub", TRSUB, '\0', TG::Power, 0},
    { "subset", TSUBSET, MS_SUBSET, TG::Relation, 0},
    { "succ", TSUCCEEDS, MS_SUCCEEDS, TG::Relation, 0 },
    { "succcurlyeq", TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, TG::Relation, 0 },
    { "succsim", TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, TG::Relation, 0 },
    { "nsucc", TNOTSUCCEEDS, MS_NOTSUCCEEDS, TG::Relation, 0 },
    { "subseteq", TSUBSETEQ, MS_SUBSETEQ, TG::Relation, 0},
    { "sum", TSUM, MS_SUM, TG::Oper, 5},
    { "sup", TRSUP, '\0', TG::Power, 0},
    { "supset", TSUPSET, MS_SUPSET, TG::Relation, 0},
    { "supseteq", TSUPSETEQ, MS_SUPSETEQ, TG::Relation, 0},
    { "tan", TTAN, '\0', TG::Function, 5},
    { "tanh", TTANH, '\0', TG::Function, 5},
    { "teal", TTEAL, '\0', TG::Color, 0},
    { "tilde", TTILDE, MS_TILDE, TG::Attribute, 5},
    { "times", TTIMES, MS_TIMES, TG::Product, 0},
    { "to", TTO, '\0', TG::Limit, 0},
    { "toward", TTOWARD, MS_RIGHTARROW, TG::Relation, 0},
    { "transl", TTRANSL, MS_TRANSL, TG::Relation, 0},
    { "transr", TTRANSR, MS_TRANSR, TG::Relation, 0},
    { "underbrace", TUNDERBRACE, MS_UNDERBRACE, TG::Product, 5},
    { "underline", TUNDERLINE, '\0', TG::Attribute, 5},
    { "union", TUNION, MS_UNION, TG::Sum, 0},
    { "uoper", TUOPER, '\0', TG::UnOper, 5},
    { "uparrow" , TUPARROW, MS_UPARROW, TG::Standalone, 5},
    { "vec", TVEC, MS_VEC, TG::Attribute, 5},
    { "white", TWHITE, '\0', TG::Color, 0},
    { "widebslash", TWIDEBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
    { "widehat", TWIDEHAT, MS_HAT, TG::Attribute, 5},
    { "widetilde", TWIDETILDE, MS_TILDE, TG::Attribute, 5},
    { "wideslash", TWIDESLASH, MS_SLASH, TG::Product, 0 },
    { "widevec", TWIDEVEC, MS_VEC, TG::Attribute, 5},
    { "wp" , TWP, MS_WP, TG::Standalone, 5},
    { "yellow", TYELLOW, '\0', TG::Color, 0}
};

const SmTokenTableEntry * SmParser::GetTokenTableEntry( const OUString &rName )
{
    if (!rName.isEmpty())
    {
        for (auto const &token : aTokenTable)
        {
            if (rName.equalsIgnoreAsciiCaseAscii( token.pIdent ))
                return &token;
        }
    }
    return nullptr;
}

namespace {

bool IsDelimiter( const OUString &rTxt, sal_Int32 nPos )
    // returns 'true' iff cChar is '\0' or a delimiter
{
    assert(nPos <= rTxt.getLength()); //index out of range

    if (nPos == rTxt.getLength())
        return true;

    sal_Unicode cChar = rTxt[nPos];

    // check if 'cChar' is in the delimiter table
    static const sal_Unicode aDelimiterTable[] =
    {
        ' ',  '\t', '\n', '\r', '+',  '-',  '*',  '/',  '=',  '#',
        '%',  '\\', '"',  '~',  '`',  '>',  '<',  '&',  '|',  '(',
        ')',  '{',  '}',  '[',  ']',  '^',  '_'
    };
    for (auto const &cDelimiter : aDelimiterTable)
    {
        if (cDelimiter == cChar)
            return true;
    }

    sal_Int16 nTypJp = SM_MOD()->GetSysLocale().GetCharClass().getType( rTxt, nPos );
    return ( nTypJp == css::i18n::UnicodeType::SPACE_SEPARATOR ||
             nTypJp == css::i18n::UnicodeType::CONTROL);
}

}

void SmParser::Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText )
{
    assert( nPos + nLen <= m_aBufferString.getLength() );

    m_aBufferString = m_aBufferString.replaceAt( nPos, nLen, rText );
    sal_Int32 nChg = rText.getLength() - nLen;
    m_nBufferIndex = m_nBufferIndex + nChg;
    m_nTokenIndex = m_nTokenIndex + nChg;
}

void SmParser::NextToken()
{
    // First character may be any alphabetic
    static const sal_Int32 coStartFlags =
        KParseTokens::ANY_LETTER |
        KParseTokens::IGNORE_LEADING_WS;

    // Continuing characters may be any alphabetic
    static const sal_Int32 coContFlags =
        (coStartFlags & ~KParseTokens::IGNORE_LEADING_WS)
        | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

    // user-defined char continuing characters may be any alphanumeric or dot.
    static const sal_Int32 coUserDefinedCharContFlags =
        KParseTokens::ANY_LETTER_OR_NUMBER |
        KParseTokens::ASC_DOT |
        KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

    // First character for numbers, may be any numeric or dot
    static const sal_Int32 coNumStartFlags =
        KParseTokens::ASC_DIGIT |
        KParseTokens::ASC_DOT |
        KParseTokens::IGNORE_LEADING_WS;

    // Continuing characters for numbers, may be any numeric or dot.
    static const sal_Int32 coNumContFlags =
        coNumStartFlags & ~KParseTokens::IGNORE_LEADING_WS;

    sal_Int32   nBufLen = m_aBufferString.getLength();
    ParseResult aRes;
    sal_Int32   nRealStart;
    bool        bCont;
    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR ==
                        m_pSysCC->getType( m_aBufferString, m_nBufferIndex ))
           ++m_nBufferIndex;

        // Try to parse a number in a locale-independent manner using
        // '.' as decimal separator.
        // See https://bz.apache.org/ooo/show_bug.cgi?id=45779
        aRes = m_aNumCC.parsePredefinedToken(KParseType::ASC_NUMBER,
                                        m_aBufferString, m_nBufferIndex,
                                        coNumStartFlags, "",
                                        coNumContFlags, "");

        if (aRes.TokenType == 0)
        {
            // Try again with the default token parsing.
            aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex,
                                     coStartFlags, "",
                                     coContFlags, "");
        }

        nRealStart = m_nBufferIndex + aRes.LeadingWhiteSpace;
        m_nBufferIndex = nRealStart;

        bCont = false;
        if ( aRes.TokenType == 0  &&
                nRealStart < nBufLen &&
                '\n' == m_aBufferString[ nRealStart ] )
        {
            // keep data needed for tokens row and col entry up to date
            ++m_nRow;
            m_nBufferIndex = m_nColOff = nRealStart + 1;
            bCont = true;
        }
        else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
        {
            if (nRealStart + 2 <= nBufLen && m_aBufferString.match("%%", nRealStart))
            {
                //SkipComment
                m_nBufferIndex = nRealStart + 2;
                while (m_nBufferIndex < nBufLen  &&
                    '\n' != m_aBufferString[ m_nBufferIndex ])
                    ++m_nBufferIndex;
                bCont = true;
            }
        }

    } while (bCont);

    // set index of current token
    m_nTokenIndex = m_nBufferIndex;

    m_aCurToken.nRow   = m_nRow;
    m_aCurToken.nCol   = nRealStart - m_nColOff + 1;

    bool bHandled = true;
    if (nRealStart >= nBufLen)
    {
        m_aCurToken.eType    = TEND;
        m_aCurToken.cMathChar = '\0';
        m_aCurToken.nGroup       = TG::NONE;
        m_aCurToken.nLevel       = 0;
        m_aCurToken.aText.clear();
    }
    else if (aRes.TokenType & KParseType::ANY_NUMBER)
    {
        assert(aRes.EndPos > 0);
        if ( m_aBufferString[aRes.EndPos-1] == ',' &&
             aRes.EndPos < nBufLen &&
             m_pSysCC->getType( m_aBufferString, aRes.EndPos ) != UnicodeType::SPACE_SEPARATOR )
        {
            // Comma followed by a non-space char is unlikely for decimal/thousands separator.
            --aRes.EndPos;
        }
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        m_aCurToken.eType      = TNUMBER;
        m_aCurToken.cMathChar  = '\0';
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.copy( nRealStart, n );

        SAL_WARN_IF( !IsDelimiter( m_aBufferString, aRes.EndPos ), "starmath", "identifier really finished? (compatibility!)" );
    }
    else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
    {
        m_aCurToken.eType      = TTEXT;
        m_aCurToken.cMathChar  = '\0';
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText     = aRes.DequotedNameOrString;
        m_aCurToken.nRow       = m_nRow;
        m_aCurToken.nCol       = nRealStart - m_nColOff + 2;
    }
    else if (aRes.TokenType & KParseType::IDENTNAME)
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        OUString aName( m_aBufferString.copy( nRealStart, n ) );
        const SmTokenTableEntry *pEntry = GetTokenTableEntry( aName );

        if (pEntry)
        {
            m_aCurToken.eType      = pEntry->eType;
            m_aCurToken.cMathChar  = pEntry->cMathChar;
            m_aCurToken.nGroup     = pEntry->nGroup;
            m_aCurToken.nLevel     = pEntry->nLevel;
            m_aCurToken.aText      = OUString::createFromAscii( pEntry->pIdent );
        }
        else
        {
            m_aCurToken.eType      = TIDENT;
            m_aCurToken.cMathChar  = '\0';
            m_aCurToken.nGroup     = TG::NONE;
            m_aCurToken.nLevel     = 5;
            m_aCurToken.aText      = aName;

            SAL_WARN_IF(!IsDelimiter(m_aBufferString, aRes.EndPos),"starmath", "identifier really finished? (compatibility!)");
        }
    }
    else if (aRes.TokenType == 0  &&  '_' == m_aBufferString[ nRealStart ])
    {
        m_aCurToken.eType    = TRSUB;
        m_aCurToken.cMathChar = '\0';
        m_aCurToken.nGroup       = TG::Power;
        m_aCurToken.nLevel       = 0;
        m_aCurToken.aText = "_";

        aRes.EndPos = nRealStart + 1;
    }
    else if (aRes.TokenType & KParseType::BOOLEAN)
    {
        sal_Int32   &rnEndPos = aRes.EndPos;
        if (rnEndPos - nRealStart <= 2)
        {
            sal_Unicode ch = m_aBufferString[ nRealStart ];
            switch (ch)
            {
                case '<':
                    {
                        if (m_aBufferString.match("<<", nRealStart))
                        {
                            m_aCurToken.eType    = TLL;
                            m_aCurToken.cMathChar = MS_LL;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<<";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<=", nRealStart))
                        {
                            m_aCurToken.eType    = TLE;
                            m_aCurToken.cMathChar = MS_LE;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<=";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<-", nRealStart))
                        {
                            m_aCurToken.eType    = TLEFTARROW;
                            m_aCurToken.cMathChar = MS_LEFTARROW;
                            m_aCurToken.nGroup       = TG::Standalone;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "<-";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<>", nRealStart))
                        {
                            m_aCurToken.eType    = TNEQ;
                            m_aCurToken.cMathChar = MS_NEQ;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<>";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<?>", nRealStart))
                        {
                            m_aCurToken.eType    = TPLACE;
                            m_aCurToken.cMathChar = MS_PLACE;
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "<?>";

                            rnEndPos = nRealStart + 3;
                        }
                        else
                        {
                            m_aCurToken.eType    = TLT;
                            m_aCurToken.cMathChar = MS_LT;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<";
                        }
                    }
                    break;
                case '>':
                    {
                        if (m_aBufferString.match(">=", nRealStart))
                        {
                            m_aCurToken.eType    = TGE;
                            m_aCurToken.cMathChar = MS_GE;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = ">=";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match(">>", nRealStart))
                        {
                            m_aCurToken.eType    = TGG;
                            m_aCurToken.cMathChar = MS_GG;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = ">>";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TGT;
                            m_aCurToken.cMathChar = MS_GT;
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = ">";
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
        if (rnEndPos - nRealStart == 1)
        {
            sal_Unicode ch = m_aBufferString[ nRealStart ];
            switch (ch)
            {
                case '%':
                    {
                        //! modifies aRes.EndPos

                        OSL_ENSURE( rnEndPos >= nBufLen  ||
                                    '%' != m_aBufferString[ rnEndPos ],
                                "unexpected comment start" );

                        // get identifier of user-defined character
                        ParseResult aTmpRes = m_pSysCC->parseAnyToken(
                                m_aBufferString, rnEndPos,
                                KParseTokens::ANY_LETTER,
                                "",
                                coUserDefinedCharContFlags,
                                "" );

                        sal_Int32 nTmpStart = rnEndPos + aTmpRes.LeadingWhiteSpace;

                        // default setting for the case that no identifier
                        // i.e. a valid symbol-name is following the '%'
                        // character
                        m_aCurToken.eType      = TTEXT;
                        m_aCurToken.cMathChar  = '\0';
                        m_aCurToken.nGroup     = TG::NONE;
                        m_aCurToken.nLevel     = 5;
                        m_aCurToken.aText      ="%";
                        m_aCurToken.nRow       = m_nRow;
                        m_aCurToken.nCol       = nTmpStart - m_nColOff;

                        if (aTmpRes.TokenType & KParseType::IDENTNAME)
                        {

                            sal_Int32 n = aTmpRes.EndPos - nTmpStart;
                            m_aCurToken.eType      = TSPECIAL;
                            m_aCurToken.aText      = m_aBufferString.copy( nTmpStart-1, n+1 );

                            OSL_ENSURE( aTmpRes.EndPos > rnEndPos,
                                    "empty identifier" );
                            if (aTmpRes.EndPos > rnEndPos)
                                rnEndPos = aTmpRes.EndPos;
                            else
                                ++rnEndPos;
                        }

                        // if no symbol-name was found we start-over with
                        // finding the next token right after the '%' sign.
                        // I.e. we leave rnEndPos unmodified.
                    }
                    break;
                case '[':
                    {
                        m_aCurToken.eType    = TLBRACKET;
                        m_aCurToken.cMathChar = MS_LBRACKET;
                        m_aCurToken.nGroup       = TG::LBrace;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "[";
                    }
                    break;
                case '\\':
                    {
                        m_aCurToken.eType    = TESCAPE;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "\\";
                    }
                    break;
                case ']':
                    {
                        m_aCurToken.eType    = TRBRACKET;
                        m_aCurToken.cMathChar = MS_RBRACKET;
                        m_aCurToken.nGroup       = TG::RBrace;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "]";
                    }
                    break;
                case '^':
                    {
                        m_aCurToken.eType    = TRSUP;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TG::Power;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "^";
                    }
                    break;
                case '`':
                    {
                        m_aCurToken.eType    = TSBLANK;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TG::Blank;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "`";
                    }
                    break;
                case '{':
                    {
                        m_aCurToken.eType    = TLGROUP;
                        m_aCurToken.cMathChar = MS_LBRACE;
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "{";
                    }
                    break;
                case '|':
                    {
                        m_aCurToken.eType    = TOR;
                        m_aCurToken.cMathChar = MS_OR;
                        m_aCurToken.nGroup       = TG::Sum;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "|";
                    }
                    break;
                case '}':
                    {
                        m_aCurToken.eType    = TRGROUP;
                        m_aCurToken.cMathChar = MS_RBRACE;
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "}";
                    }
                    break;
                case '~':
                    {
                        m_aCurToken.eType    = TBLANK;
                        m_aCurToken.cMathChar = '\0';
                        m_aCurToken.nGroup       = TG::Blank;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "~";
                    }
                    break;
                case '#':
                    {
                        if (m_aBufferString.match("##", nRealStart))
                        {
                            m_aCurToken.eType    = TDPOUND;
                            m_aCurToken.cMathChar = '\0';
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "##";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPOUND;
                            m_aCurToken.cMathChar = '\0';
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "#";
                        }
                    }
                    break;
                case '&':
                    {
                        m_aCurToken.eType    = TAND;
                        m_aCurToken.cMathChar = MS_AND;
                        m_aCurToken.nGroup       = TG::Product;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "&";
                    }
                    break;
                case '(':
                    {
                        m_aCurToken.eType    = TLPARENT;
                        m_aCurToken.cMathChar = MS_LPARENT;
                        m_aCurToken.nGroup       = TG::LBrace;
                        m_aCurToken.nLevel       = 5;     //! 0 to continue expression
                        m_aCurToken.aText = "(";
                    }
                    break;
                case ')':
                    {
                        m_aCurToken.eType    = TRPARENT;
                        m_aCurToken.cMathChar = MS_RPARENT;
                        m_aCurToken.nGroup       = TG::RBrace;
                        m_aCurToken.nLevel       = 0;     //! 0 to terminate expression
                        m_aCurToken.aText = ")";
                    }
                    break;
                case '*':
                    {
                        m_aCurToken.eType    = TMULTIPLY;
                        m_aCurToken.cMathChar = MS_MULTIPLY;
                        m_aCurToken.nGroup       = TG::Product;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "*";
                    }
                    break;
                case '+':
                    {
                        if (m_aBufferString.match("+-", nRealStart))
                        {
                            m_aCurToken.eType    = TPLUSMINUS;
                            m_aCurToken.cMathChar = MS_PLUSMINUS;
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "+-";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPLUS;
                            m_aCurToken.cMathChar = MS_PLUS;
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "+";
                        }
                    }
                    break;
                case '-':
                    {
                        if (m_aBufferString.match("-+", nRealStart))
                        {
                            m_aCurToken.eType    = TMINUSPLUS;
                            m_aCurToken.cMathChar = MS_MINUSPLUS;
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "-+";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("->", nRealStart))
                        {
                            m_aCurToken.eType    = TRIGHTARROW;
                            m_aCurToken.cMathChar = MS_RIGHTARROW;
                            m_aCurToken.nGroup       = TG::Standalone;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "->";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TMINUS;
                            m_aCurToken.cMathChar = MS_MINUS;
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "-";
                        }
                    }
                    break;
                case '.':
                    {
                        // Only one character? Then it can't be a number.
                        if (m_nBufferIndex < m_aBufferString.getLength() - 1)
                        {
                            // for compatibility with SO5.2
                            // texts like .34 ...56 ... h ...78..90
                            // will be treated as numbers
                            m_aCurToken.eType     = TNUMBER;
                            m_aCurToken.cMathChar = '\0';
                            m_aCurToken.nGroup    = TG::NONE;
                            m_aCurToken.nLevel    = 5;

                            sal_Int32 nTxtStart = m_nBufferIndex;
                            sal_Unicode cChar;
                            // if the equation ends with dot(.) then increment m_nBufferIndex till end of string only
                            do
                            {
                                cChar = m_aBufferString[ ++m_nBufferIndex ];
                            }
                            while ( (cChar == '.' || rtl::isAsciiDigit( cChar )) &&
                                     ( m_nBufferIndex < m_aBufferString.getLength() - 1 ) );

                            m_aCurToken.aText = m_aBufferString.copy( nTxtStart, m_nBufferIndex - nTxtStart );
                            aRes.EndPos = m_nBufferIndex;
                        }
                        else
                            bHandled = false;
                    }
                    break;
                case '/':
                    {
                        m_aCurToken.eType    = TDIVIDEBY;
                        m_aCurToken.cMathChar = MS_SLASH;
                        m_aCurToken.nGroup       = TG::Product;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "/";
                    }
                    break;
                case '=':
                    {
                        m_aCurToken.eType    = TASSIGN;
                        m_aCurToken.cMathChar = MS_ASSIGN;
                        m_aCurToken.nGroup       = TG::Relation;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "=";
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
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.copy( nRealStart, 1 );

        aRes.EndPos = nRealStart + 1;
    }

    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = aRes.EndPos;
}

// grammar

SmTableNode *SmParser::DoTable()
{
    DoLine();
    while (m_aCurToken.eType == TNEWLINE)
    {
        NextToken();
        DoLine();
    }

    if (m_aCurToken.eType != TEND)
        Error(PE_UNEXPECTED_CHAR);

    SmNodeArray  LineArray(m_aNodeStack.size());
    for (auto rIt = LineArray.rbegin(), rEnd = LineArray.rend(); rIt != rEnd; ++rIt)
    {
        *rIt = popOrZero(m_aNodeStack);
    }

    std::unique_ptr<SmTableNode> pSNode(new SmTableNode(m_aCurToken));
    pSNode->SetSubNodes(LineArray);
    return pSNode.release();
}

void SmParser::DoAlign()
    // parse alignment info (if any), then go on with rest of expression
{
    std::unique_ptr<SmStructureNode> pSNode;

    if (TokenInGroup(TG::Align))
    {
        pSNode.reset(new SmAlignNode(m_aCurToken));

        NextToken();

        // allow for just one align statement in 5.0
        if (TokenInGroup(TG::Align))
        {
            Error(PE_DOUBLE_ALIGN);
            return;
        }
    }

    DoExpression();

    if (pSNode)
    {
        pSNode->SetSubNode(0, popOrZero(m_aNodeStack));
        m_aNodeStack.push_front(std::move(pSNode));
    }
}

void SmParser::DoLine()
{
    SmNodeArray  ExpressionArray;

    // start with single expression that may have an alignment statement
    // (and go on with expressions that must not have alignment
    // statements in 'while' loop below. See also 'Expression()'.)
    if (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
    {
        DoAlign();
        ExpressionArray.push_back(popOrZero(m_aNodeStack));
    }

    while (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
    {
        DoExpression();
        ExpressionArray.push_back(popOrZero(m_aNodeStack));
    }

    //If there's no expression, add an empty one.
    //this is to avoid a formula tree without any caret
    //positions, in visual formula editor.
    if(ExpressionArray.empty())
    {
        SmToken aTok = SmToken();
        aTok.eType = TNEWLINE;
        ExpressionArray.push_back(new SmExpressionNode(aTok));
    }

    std::unique_ptr<SmStructureNode> pSNode(new SmLineNode(m_aCurToken));
    pSNode->SetSubNodes(ExpressionArray);
    m_aNodeStack.push_front(std::move(pSNode));
}

void SmParser::DoExpression()
{
    bool bUseExtraSpaces = true;
    if (!m_aNodeStack.empty())
    {
        if (m_aNodeStack.front()->GetToken().eType == TNOSPACE)
        {
            m_aNodeStack.pop_front();
            bUseExtraSpaces = false;
        }
    }

    SmNodeArray  RelationArray;

    DoRelation();
    RelationArray.push_back(popOrZero(m_aNodeStack));

    while (m_aCurToken.nLevel >= 4)
    {
        DoRelation();
        RelationArray.push_back(popOrZero(m_aNodeStack));
    }

    if (RelationArray.size() > 1)
    {
        std::unique_ptr<SmExpressionNode> pSNode(new SmExpressionNode(m_aCurToken));
        pSNode->SetSubNodes(RelationArray);
        pSNode->SetUseExtraSpaces(bUseExtraSpaces);
        m_aNodeStack.push_front(std::move(pSNode));
    }
    else
    {
        // This expression has only one node so just push this node.
        m_aNodeStack.push_front(std::unique_ptr<SmNode>(RelationArray[0]));
    }
}

void SmParser::DoRelation()
{
    DoSum();
    while (TokenInGroup(TG::Relation))
    {
        std::unique_ptr<SmStructureNode> pSNode(new SmBinHorNode(m_aCurToken));
        SmNode *pFirst = popOrZero(m_aNodeStack);

        DoOpSubSup();
        SmNode *pSecond = popOrZero(m_aNodeStack);

        DoSum();

        pSNode->SetSubNodes(pFirst, pSecond, popOrZero(m_aNodeStack));
        m_aNodeStack.push_front(std::move(pSNode));
    }
}

void SmParser::DoSum()
{
    DoProduct();
    while (TokenInGroup(TG::Sum))
    {
        std::unique_ptr<SmStructureNode> pSNode(new SmBinHorNode(m_aCurToken));
        SmNode *pFirst = popOrZero(m_aNodeStack);

        DoOpSubSup();
        SmNode *pSecond = popOrZero(m_aNodeStack);

        DoProduct();

        pSNode->SetSubNodes(pFirst, pSecond, popOrZero(m_aNodeStack));
        m_aNodeStack.push_front(std::move(pSNode));
    }
}

void SmParser::DoProduct()
{
    DoPower();

    while (TokenInGroup(TG::Product))
    {   SmStructureNode *pSNode;
        SmNode *pFirst = popOrZero(m_aNodeStack),
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
                m_aCurToken.nGroup = TG::Product;

                DoGlyphSpecial();
                pOper = popOrZero(m_aNodeStack);
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

                DoOpSubSup();
                pOper = popOrZero(m_aNodeStack);
        }

        DoPower();

        if (bSwitchArgs)
        {
            //! vgl siehe SmBinDiagonalNode::Arrange
            pSNode->SetSubNodes(pFirst, popOrZero(m_aNodeStack), pOper);
        }
        else
        {
            pSNode->SetSubNodes(pFirst, pOper, popOrZero(m_aNodeStack));
        }
        m_aNodeStack.push_front(std::unique_ptr<SmStructureNode>(pSNode));
    }
}

void SmParser::DoSubSup(TG nActiveGroup)
{
    OSL_ENSURE(nActiveGroup == TG::Power  ||  nActiveGroup == TG::Limit,
               "Sm: wrong token group");

    if (!TokenInGroup(nActiveGroup))
        // already finish
        return;

    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(m_aCurToken));
    //! Of course 'm_aCurToken' is just the first sub-/supscript token.
    //! It should be of no further interest. The positions of the
    //! sub-/supscripts will be identified by the corresponding subnodes
    //! index in the 'aSubNodes' array (enum value from 'SmSubSup').

    pNode->SetUseLimits(nActiveGroup == TG::Limit);

    // initialize subnodes array
    SmNodeArray aSubNodes(1 + SUBSUP_NUM_ENTRIES, nullptr);
    aSubNodes[0] = popOrZero(m_aNodeStack);

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
            DoRelation();
        }
        else
            DoTerm(true);

        switch (eType)
        {
            case TRSUB :    nIndex = static_cast<int>(RSUB);    break;
            case TRSUP :    nIndex = static_cast<int>(RSUP);    break;
            case TFROM :
            case TCSUB :    nIndex = static_cast<int>(CSUB);    break;
            case TTO :
            case TCSUP :    nIndex = static_cast<int>(CSUP);    break;
            case TLSUB :    nIndex = static_cast<int>(LSUB);    break;
            case TLSUP :    nIndex = static_cast<int>(LSUP);    break;
            default :
                SAL_WARN( "starmath", "unknown case");
        }
        nIndex++;
        assert(1 <= nIndex  &&  nIndex <= SUBSUP_NUM_ENTRIES);

        // set sub-/supscript if not already done
        if (aSubNodes[nIndex] != nullptr)
            Error(PE_DOUBLE_SUBSUPSCRIPT);
        aSubNodes[nIndex] = popOrZero(m_aNodeStack);
    }

    pNode->SetSubNodes(aSubNodes);
    m_aNodeStack.push_front(std::move(pNode));
}

void SmParser::DoOpSubSup()
{
    // push operator symbol
    m_aNodeStack.push_front(o3tl::make_unique<SmMathSymbolNode>(m_aCurToken));
    // skip operator token
    NextToken();
    // get sub- supscripts if any
    if (TokenInGroup(TG::Power))
        DoSubSup(TG::Power);
}

void SmParser::DoPower()
{
    // get body for sub- supscripts on top of stack
    DoTerm(false);

    DoSubSup(TG::Power);
}

void SmParser::DoBlank()
{
    OSL_ENSURE(TokenInGroup(TG::Blank), "Sm : wrong token");
    std::unique_ptr<SmBlankNode> pBlankNode(new SmBlankNode(m_aCurToken));

    while (TokenInGroup(TG::Blank))
    {
        pBlankNode->IncreaseBy(m_aCurToken);
        NextToken();
    }

    // Ignore trailing spaces, if corresponding option is set
    if ( m_aCurToken.eType == TNEWLINE ||
             (m_aCurToken.eType == TEND && SM_MOD()->GetConfig()->IsIgnoreSpacesRight()) )
    {
        pBlankNode->Clear();
    }

    m_aNodeStack.push_front(std::move(pBlankNode));
}

void SmParser::DoTerm(bool bGroupNumberIdent)
{
    switch (m_aCurToken.eType)
    {
        case TESCAPE :
            DoEscape();
            break;

        case TNOSPACE :
        case TLGROUP :
        {
            bool bNoSpace = m_aCurToken.eType == TNOSPACE;
            if (bNoSpace)   // push 'no space' node and continue to parse expression
            {
                m_aNodeStack.push_front(o3tl::make_unique<SmExpressionNode>(m_aCurToken));
                NextToken();
            }
            if (m_aCurToken.eType != TLGROUP)
            {
                m_aNodeStack.pop_front();    // get rid of the 'no space' node pushed above
                DoTerm(false);
            }
            else
            {
                NextToken();

                // allow for empty group
                if (m_aCurToken.eType == TRGROUP)
                {
                    if (bNoSpace)   // get rid of the 'no space' node pushed above
                        m_aNodeStack.pop_front();
                    std::unique_ptr<SmStructureNode> pSNode(new SmExpressionNode(m_aCurToken));
                    pSNode->SetSubNodes(nullptr, nullptr);
                    m_aNodeStack.push_front(std::move(pSNode));

                    NextToken();
                }
                else    // go as usual
                {
                    DoAlign();
                    if (m_aCurToken.eType != TRGROUP)
                        Error(PE_RGROUP_EXPECTED);
                    else
                        NextToken();
                }
            }
        }
        break;

        case TLEFT :
            DoBrace();
            break;

        case TBLANK :
        case TSBLANK :
            DoBlank();
            break;

        case TTEXT :
            m_aNodeStack.push_front(o3tl::make_unique<SmTextNode>(m_aCurToken, FNT_TEXT));
            NextToken();
            break;
        case TCHARACTER :
            m_aNodeStack.push_front(o3tl::make_unique<SmTextNode>(m_aCurToken, FNT_VARIABLE));
            NextToken();
            break;
        case TIDENT :
        case TNUMBER :
        {
            m_aNodeStack.push_front(o3tl::make_unique<SmTextNode>(m_aCurToken,
                                             m_aCurToken.eType == TNUMBER ?
                                             FNT_NUMBER :
                                             FNT_VARIABLE));
            if (!bGroupNumberIdent)
            {
                NextToken();
            }
            else
            {
                // Some people want to be able to write "x_2n" for "x_{2n}"
                // although e.g. LaTeX or AsciiMath interpret that as "x_2 n".
                // The tokenizer skips whitespaces so we need some additional
                // work to distinguish from "x_2 n".
                // See https://bz.apache.org/ooo/show_bug.cgi?id=11752 and
                // https://bugs.libreoffice.org/show_bug.cgi?id=55853
                sal_Int32 nBufLen = m_aBufferString.getLength();
                sal_Int32 nTokens = 1;

                // We need to be careful to call NextToken() only after having
                // tested for a whitespace separator (otherwise it will be
                // skipped!)
                bool moveToNextToken = true;
                while (m_nBufferIndex < nBufLen &&
                       m_pSysCC->getType(m_aBufferString, m_nBufferIndex) !=
                       UnicodeType::SPACE_SEPARATOR)
                {
                    NextToken();
                    if (m_aCurToken.eType != TNUMBER &&
                        m_aCurToken.eType != TIDENT)
                    {
                        // Neither a number nor an identifier. We just moved to
                        // the next token, so no need to do that again.
                        moveToNextToken = false;
                        break;
                    }
                    m_aNodeStack.push_front(o3tl::make_unique<SmTextNode>(m_aCurToken,
                                                     m_aCurToken.eType ==
                                                     TNUMBER ?
                                                     FNT_NUMBER :
                                                     FNT_VARIABLE));
                    nTokens++;
                }
                if (moveToNextToken) NextToken();
                if (nTokens > 1)
                {
                    // We have several concatenated identifiers and numbers.
                    // Let's group them into one SmExpressionNode.
                    SmNodeArray nodeArray(nTokens);
                    for (auto rIt = nodeArray.rbegin(), rEnd = nodeArray.rend(); rIt != rEnd; ++rIt)
                    {
                        *rIt = popOrZero(m_aNodeStack);
                    }
                    std::unique_ptr<SmExpressionNode> pNode(new SmExpressionNode(SmToken()));
                    pNode->SetSubNodes(nodeArray);
                    m_aNodeStack.push_front(std::move(pNode));
                }
            }
            break;
        }
        case TLEFTARROW :
        case TRIGHTARROW :
        case TUPARROW :
        case TDOWNARROW :
        case TCIRC :
        case TDRARROW :
        case TDLARROW :
        case TDLRARROW :
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
            m_aNodeStack.push_front(o3tl::make_unique<SmMathSymbolNode>(m_aCurToken));
            NextToken();
            break;

        case TSETN :
        case TSETZ :
        case TSETQ :
        case TSETR :
        case TSETC :
        case THBAR :
        case TLAMBDABAR :
        case TBACKEPSILON :
        case TALEPH :
        case TIM :
        case TRE :
        case TWP :
        case TEMPTYSET :
        case TINFINITY :
            m_aNodeStack.push_front(o3tl::make_unique<SmMathIdentifierNode>(m_aCurToken));
            NextToken();
            break;

        case TPLACE:
            m_aNodeStack.push_front(o3tl::make_unique<SmPlaceNode>(m_aCurToken));
            NextToken();
            break;

        case TSPECIAL:
            DoSpecial();
            break;

        case TBINOM:
            DoBinom();
            break;

        case TSTACK:
            DoStack();
            break;

        case TMATRIX:
            DoMatrix();
            break;

        default:
            if (TokenInGroup(TG::LBrace))
            {
                DoBrace();
            }
            else if (TokenInGroup(TG::Oper))
            {
                DoOperator();
            }
            else if (TokenInGroup(TG::UnOper))
            {
                DoUnOper();
            }
            else if (    TokenInGroup(TG::Attribute)
                     ||  TokenInGroup(TG::FontAttr))
            {
                std::stack<SmStructureNode *> aStack;
                bool    bIsAttr;
                while ( (bIsAttr = TokenInGroup(TG::Attribute))
                       ||  TokenInGroup(TG::FontAttr))
                {
                    if (bIsAttr)
                        DoAttribut();
                    else
                        DoFontAttribut();

                    SmNode* pTmp = popOrZero(m_aNodeStack);
                    assert(pTmp && !pTmp->IsVisible());
                    aStack.push(static_cast<SmStructureNode *>(pTmp));
                }

                DoPower();

                SmNode *pFirstNode = popOrZero(m_aNodeStack);
                while (!aStack.empty())
                {
                    SmStructureNode *pNode = aStack.top();
                    aStack.pop();
                    pNode->SetSubNodes(nullptr, pFirstNode);
                    pFirstNode = pNode;
                }
                m_aNodeStack.push_front(std::unique_ptr<SmNode>(pFirstNode));
            }
            else if (TokenInGroup(TG::Function))
            {
                DoFunction();
            }
            else
                Error(PE_UNEXPECTED_CHAR);
    }
}

void SmParser::DoEscape()
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

    m_aNodeStack.push_front(o3tl::make_unique<SmMathSymbolNode>(m_aCurToken));

    NextToken();
}

void SmParser::DoOperator()
{
    assert(TokenInGroup(TG::Oper));

    std::unique_ptr<SmStructureNode> pSNode(new SmOperNode(m_aCurToken));

    // put operator on top of stack
    DoOper();

    if (TokenInGroup(TG::Limit) || TokenInGroup(TG::Power))
        DoSubSup(m_aCurToken.nGroup);
    SmNode *pOperator = popOrZero(m_aNodeStack);

    // get argument
    DoPower();

    pSNode->SetSubNodes(pOperator, popOrZero(m_aNodeStack));
    m_aNodeStack.push_front(std::move(pSNode));
}

void SmParser::DoOper()
{
    SmTokenType  eType (m_aCurToken.eType);
    std::unique_ptr<SmNode> pNode;

    switch (eType)
    {
        case TSUM :
        case TPROD :
        case TCOPROD :
        case TINT :
        case TINTD :
        case TIINT :
        case TIIINT :
        case TLINT :
        case TLLINT :
        case TLLLINT :
            pNode.reset(new SmMathSymbolNode(m_aCurToken));
            break;

        case TLIM :
        case TLIMSUP :
        case TLIMINF :
            {
                const sal_Char* pLim = nullptr;
                switch (eType)
                {
                    case TLIM :     pLim = "lim";       break;
                    case TLIMSUP :  pLim = "lim sup";   break;
                    case TLIMINF :  pLim = "lim inf";   break;
                    default:
                        break;
                }
                if( pLim )
                    m_aCurToken.aText = OUString::createFromAscii(pLim);
                pNode.reset(new SmTextNode(m_aCurToken, FNT_TEXT));
            }
            break;

        case TOPER :
            NextToken();

            OSL_ENSURE(m_aCurToken.eType == TSPECIAL, "Sm: wrong token");
            pNode.reset(new SmGlyphSpecialNode(m_aCurToken));
            break;

        default :
            assert(false && "unknown case");
    }
    m_aNodeStack.push_front(std::move(pNode));

    NextToken();
}

void SmParser::DoUnOper()
{
    assert(TokenInGroup(TG::UnOper));

    SmToken      aNodeToken = m_aCurToken;
    SmTokenType  eType      = m_aCurToken.eType;
    bool         bIsPostfix = eType == TFACT;

    std::unique_ptr<SmStructureNode> pSNode;
    SmNode *pOper   = nullptr,
           *pExtra  = nullptr,
           *pArg;

    switch (eType)
    {
        case TABS :
        case TSQRT :
            NextToken();
            break;

        case TNROOT :
            NextToken();
            DoPower();
            pExtra = popOrZero(m_aNodeStack);
            break;

        case TUOPER :
            NextToken();
            //Let the glyph know what it is...
            m_aCurToken.eType = TUOPER;
            m_aCurToken.nGroup = TG::UnOper;
            DoGlyphSpecial();
            pOper = popOrZero(m_aNodeStack);
            break;

        case TPLUS :
        case TMINUS :
        case TPLUSMINUS :
        case TMINUSPLUS :
        case TNEG :
        case TFACT :
            DoOpSubSup();
            pOper = popOrZero(m_aNodeStack);
            break;

        default :
            Error(PE_UNOPER_EXPECTED);
    }

    // get argument
    DoPower();
    pArg = popOrZero(m_aNodeStack);

    if (eType == TABS)
    {
        pSNode.reset(new SmBraceNode(aNodeToken));
        pSNode->SetScaleMode(SCALE_HEIGHT);

        // build nodes for left & right lines
        // (text, group, level of the used token are of no interest here)
        // we'll use row & column of the keyword for abs
        aNodeToken.eType = TABS;

        aNodeToken.cMathChar = MS_VERTLINE;
        SmNode* pLeft = new SmMathSymbolNode(aNodeToken);

        aNodeToken.cMathChar = MS_VERTLINE;
        SmNode* pRight = new SmMathSymbolNode(aNodeToken);

        pSNode->SetSubNodes(pLeft, pArg, pRight);
    }
    else if (eType == TSQRT  ||  eType == TNROOT)
    {
        pSNode.reset(new SmRootNode(aNodeToken));
        pOper = new SmRootSymbolNode(aNodeToken);
        pSNode->SetSubNodes(pExtra, pOper, pArg);
    }
    else
    {
        pSNode.reset(new SmUnHorNode(aNodeToken));
        if (bIsPostfix)
            pSNode->SetSubNodes(pArg, pOper);
        else
            // prefix operator
            pSNode->SetSubNodes(pOper, pArg);
    }

    m_aNodeStack.push_front(std::move(pSNode));
}

void SmParser::DoAttribut()
{
    assert(TokenInGroup(TG::Attribute));

    std::unique_ptr<SmStructureNode> pSNode(new SmAttributNode(m_aCurToken));
    SmNode      *pAttr;
    SmScaleMode  eScaleMode = SCALE_NONE;

    // get appropriate node for the attribute itself
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

    pSNode->SetSubNodes(pAttr, nullptr); // the body will be filled later
    pSNode->SetScaleMode(eScaleMode);
    m_aNodeStack.push_front(std::move(pSNode));
}


void SmParser::DoFontAttribut()
{
    assert(TokenInGroup(TG::FontAttr));

    switch (m_aCurToken.eType)
    {
        case TITALIC :
        case TNITALIC :
        case TBOLD :
        case TNBOLD :
        case TPHANTOM :
            m_aNodeStack.push_front(o3tl::make_unique<SmFontNode>(m_aCurToken));
            NextToken();
            break;

        case TSIZE :
            DoFontSize();
            break;

        case TFONT :
            DoFont();
            break;

        case TCOLOR :
            DoColor();
            break;

        default :
            assert(false);
    }
}

void SmParser::DoColor()
{
    assert(m_aCurToken.eType == TCOLOR);

    // last color rules, get that one
    SmToken  aToken;
    do
    {   NextToken();

        if (TokenInGroup(TG::Color))
        {   aToken = m_aCurToken;
            NextToken();
        }
        else
            Error(PE_COLOR_EXPECTED);
    } while (m_aCurToken.eType == TCOLOR);

    m_aNodeStack.push_front(o3tl::make_unique<SmFontNode>(aToken));
}

void SmParser::DoFont()
{
    assert(m_aCurToken.eType == TFONT);

    // last font rules, get that one
    SmToken  aToken;
    do
    {   NextToken();

        if (TokenInGroup(TG::Font))
        {   aToken = m_aCurToken;
            NextToken();
        }
        else
            Error(PE_FONT_EXPECTED);
    } while (m_aCurToken.eType == TFONT);

    m_aNodeStack.push_front(o3tl::make_unique<SmFontNode>(aToken));
}


// gets number used as arguments in Math formulas (e.g. 'size' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNumber(const OUString& rText)
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
        else if ( !rtl::isAsciiDigit( cChar ) )
            return false;
    }
    return true;
}

void SmParser::DoFontSize()
{
    assert(m_aCurToken.eType == TSIZE);

    FontSizeType   Type;
    std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(m_aCurToken));

    NextToken();

    switch (m_aCurToken.eType)
    {
        case TNUMBER:   Type = FontSizeType::ABSOLUT;  break;
        case TPLUS:     Type = FontSizeType::PLUS;     break;
        case TMINUS:    Type = FontSizeType::MINUS;    break;
        case TMULTIPLY: Type = FontSizeType::MULTIPLY; break;
        case TDIVIDEBY: Type = FontSizeType::DIVIDE;   break;

        default:
            Error(PE_SIZE_EXPECTED);
            return;
    }

    if (Type != FontSizeType::ABSOLUT)
    {
        NextToken();
        if (m_aCurToken.eType != TNUMBER)
        {
            Error(PE_SIZE_EXPECTED);
            return;
        }
    }

    // get number argument
    Fraction  aValue( 1L );
    if (lcl_IsNumber( m_aCurToken.aText ))
    {
        double fTmp = OUString(m_aCurToken.aText).toDouble();
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
    m_aNodeStack.push_front(std::move(pFontNode));
}

void SmParser::DoBrace()
{
    assert(m_aCurToken.eType == TLEFT  ||  TokenInGroup(TG::LBrace));

    std::unique_ptr<SmStructureNode> pSNode(new SmBraceNode(m_aCurToken));
    std::unique_ptr<SmNode> pBody, pLeft, pRight;
    SmScaleMode   eScaleMode = SCALE_NONE;
    SmParseError  eError     = PE_NONE;

    if (m_aCurToken.eType == TLEFT)
    {   NextToken();

        eScaleMode = SCALE_HEIGHT;

        // check for left bracket
        if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
        {
            pLeft.reset(new SmMathSymbolNode(m_aCurToken));

            NextToken();
            DoBracebody(true);
            pBody.reset(popOrZero(m_aNodeStack));

            if (m_aCurToken.eType == TRIGHT)
            {   NextToken();

                // check for right bracket
                if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
                {
                    pRight.reset(new SmMathSymbolNode(m_aCurToken));
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
        assert(TokenInGroup(TG::LBrace));

        pLeft.reset(new SmMathSymbolNode(m_aCurToken));

        NextToken();
        DoBracebody(false);
        pBody.reset(popOrZero(m_aNodeStack));

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
                SAL_WARN("starmath", "unknown case");
            }

        if (m_aCurToken.eType == eExpectedType)
        {
            pRight.reset(new SmMathSymbolNode(m_aCurToken));
            NextToken();
        }
        else
            eError = PE_PARENT_MISMATCH;
    }

    if (eError == PE_NONE)
    {
        assert(pLeft);
        assert(pRight);
        pSNode->SetSubNodes(pLeft.release(), pBody.release(), pRight.release());
        pSNode->SetScaleMode(eScaleMode);
        m_aNodeStack.push_front(std::move(pSNode));
    }
    else
        Error(eError);
}

void SmParser::DoBracebody(bool bIsLeftRight)
{
    std::unique_ptr<SmStructureNode> pBody(new SmBracebodyNode(m_aCurToken));
    sal_uInt16           nNum = 0;

    // get body if any
    if (bIsLeftRight)
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                m_aNodeStack.push_front(o3tl::make_unique<SmMathSymbolNode>(m_aCurToken));
                NextToken();
                nNum++;
            }
            else if (m_aCurToken.eType != TRIGHT)
            {
                DoAlign();
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
                m_aNodeStack.push_front(o3tl::make_unique<SmMathSymbolNode>(m_aCurToken));
                NextToken();
                nNum++;
            }
            else if (!TokenInGroup(TG::RBrace))
            {
                DoAlign();
                nNum++;

                if (m_aCurToken.eType != TMLINE  &&  !TokenInGroup(TG::RBrace))
                    Error(PE_RBRACE_EXPECTED);
            }
        } while (m_aCurToken.eType != TEND  &&  !TokenInGroup(TG::RBrace));
    }

    // build argument vector in parsing order
    SmNodeArray aNodes(nNum);
    for (auto rIt = aNodes.rbegin(), rEnd = aNodes.rend(); rIt != rEnd; ++rIt)
    {
        *rIt = popOrZero(m_aNodeStack);
    }

    pBody->SetSubNodes(aNodes);
    pBody->SetScaleMode(bIsLeftRight ? SCALE_HEIGHT : SCALE_NONE);
    m_aNodeStack.push_front(std::move(pBody));
}

void SmParser::DoFunction()
{
    switch (m_aCurToken.eType)
    {
        case TFUNC:
            NextToken();    // skip "FUNC"-statement
            SAL_FALLTHROUGH;

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
            m_aNodeStack.push_front(o3tl::make_unique<SmTextNode>(m_aCurToken, FNT_FUNCTION));
            NextToken();
            break;

        default:
            Error(PE_FUNC_EXPECTED);
    }
}

void SmParser::DoBinom()
{
    std::unique_ptr<SmStructureNode> pSNode(new SmTableNode(m_aCurToken));

    NextToken();

    DoSum();
    DoSum();

    SmNode *pSecond = popOrZero(m_aNodeStack);
    SmNode *pFirst = popOrZero(m_aNodeStack);
    pSNode->SetSubNodes(pFirst, pSecond);
    m_aNodeStack.push_front(std::move(pSNode));
}

void SmParser::DoStack()
{
    std::unique_ptr<SmStructureNode> pSNode(new SmTableNode(m_aCurToken));
    NextToken();
    if (m_aCurToken.eType == TLGROUP)
    {
        sal_uInt16 n = 0;

        do
        {
            NextToken();
            DoAlign();
            n++;
        }
        while (m_aCurToken.eType == TPOUND);

        SmNodeArray ExpressionArray(n);
        for (auto rIt = ExpressionArray.rbegin(), rEnd = ExpressionArray.rend(); rIt != rEnd; ++rIt)
        {
            *rIt = popOrZero(m_aNodeStack);
        }

        if (m_aCurToken.eType != TRGROUP)
            Error(PE_RGROUP_EXPECTED);

        pSNode->SetSubNodes(ExpressionArray);
        m_aNodeStack.push_front(std::move(pSNode));

        NextToken();
    }
    else
        Error(PE_LGROUP_EXPECTED);
}

void SmParser::DoMatrix()
{
    std::unique_ptr<SmMatrixNode> pMNode(new SmMatrixNode(m_aCurToken));
    NextToken();
    if (m_aCurToken.eType == TLGROUP)
    {
        sal_uInt16 c = 0;

        do
        {
            NextToken();
            DoAlign();
            c++;
        }
        while (m_aCurToken.eType == TPOUND);

        sal_uInt16 r = 1;

        while (m_aCurToken.eType == TDPOUND)
        {
            NextToken();
            for (sal_uInt16 i = 0; i < c; i++)
            {
                DoAlign();
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

        size_t nRC = static_cast<size_t>(r) * c;

        SmNodeArray ExpressionArray(nRC);
        for (auto rIt = ExpressionArray.rbegin(), rEnd = ExpressionArray.rend(); rIt != rEnd; ++rIt)
        {
            *rIt = popOrZero(m_aNodeStack);
        }

        if (m_aCurToken.eType != TRGROUP)
            Error(PE_RGROUP_EXPECTED);

        pMNode->SetSubNodes(ExpressionArray);
        pMNode->SetRowCol(r, c);
        m_aNodeStack.push_front(std::move(pMNode));

        NextToken();
    }
    else
        Error(PE_LGROUP_EXPECTED);
}

void SmParser::DoSpecial()
{
    bool bReplace = false;
    OUString &rName = m_aCurToken.aText;
    OUString aNewName;

    // conversion of symbol names for 6.0 (XML) file format
    // (name change on import / export.
    // UI uses localized names XML file format does not.)
    if( rName.startsWith("%") )
    {
        if (IsImportSymbolNames())
        {
            aNewName = SmLocalizedSymbolData::GetUiSymbolName(rName.copy(1));
            bReplace = true;
        }
        else if (IsExportSymbolNames())
        {
            aNewName = SmLocalizedSymbolData::GetExportSymbolName(rName.copy(1));
            bReplace = true;
        }
    }
    if (!aNewName.isEmpty())
        aNewName = "%" + aNewName;


    if (bReplace && !aNewName.isEmpty() && rName != aNewName)
    {
        Replace(GetTokenIndex(), rName.getLength(), aNewName);
        rName = aNewName;
    }

    // add symbol name to list of used symbols
    const OUString aSymbolName(m_aCurToken.aText.copy(1));
    if (!aSymbolName.isEmpty())
        m_aUsedSymbols.insert( aSymbolName );

    m_aNodeStack.push_front(o3tl::make_unique<SmSpecialNode>(m_aCurToken));
    NextToken();
}

void SmParser::DoGlyphSpecial()
{
    m_aNodeStack.push_front(o3tl::make_unique<SmGlyphSpecialNode>(m_aCurToken));
    NextToken();
}

void SmParser::Error(SmParseError eError)
{
    SmStructureNode *pSNode = new SmExpressionNode(m_aCurToken);
    SmErrorNode     *pErr   = new SmErrorNode(m_aCurToken);
    pSNode->SetSubNodes(pErr, nullptr);

    //! put a structure node on the stack (instead of the error node itself)
    //! because sometimes such a node is expected in order to attach some
    //! subnodes
    m_aNodeStack.push_front(std::unique_ptr<SmStructureNode>(pSNode));

    AddError(eError, pSNode);

    NextToken();
}


// end grammar


SmParser::SmParser()
    : m_nCurError( 0 )
    , m_nBufferIndex( 0 )
    , m_nTokenIndex( 0 )
    , m_nRow( 0 )
    , m_nColOff( 0 )
    , m_bImportSymNames( false )
    , m_bExportSymNames( false )
    , m_aNumCC( LanguageTag( LANGUAGE_ENGLISH_US ) )
    , m_pSysCC( SM_MOD()->GetSysLocale().GetCharClassPtr() )
{
}

SmTableNode *SmParser::Parse(const OUString &rBuffer)
{
    m_aUsedSymbols.clear();

    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_nRow          = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    m_aErrDescList.clear();

    m_aNodeStack.clear();

    NextToken();
    return DoTable();
}

SmNode *SmParser::ParseExpression(const OUString &rBuffer)
{
    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_nRow          = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    m_aErrDescList.clear();

    m_aNodeStack.clear();

    NextToken();
    DoExpression();

    SmNode* result = popOrZero(m_aNodeStack);
    return result;
}


void SmParser::AddError(SmParseError Type, SmNode *pNode)
{
    std::unique_ptr<SmErrorDesc> pErrDesc(new SmErrorDesc);

    pErrDesc->m_eType = Type;
    pErrDesc->m_pNode = pNode;
    pErrDesc->m_aText = SM_RESSTR(RID_ERR_IDENT);

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
        case PE_POUND_EXPECTED:      nRID = RID_ERR_POUNDEXPECTED;          break;
        case PE_COLOR_EXPECTED:      nRID = RID_ERR_COLOREXPECTED;          break;
        case PE_RIGHT_EXPECTED:      nRID = RID_ERR_RIGHTEXPECTED;          break;

        default:
            nRID = RID_ERR_UNKNOWN;
    }
    pErrDesc->m_aText += SM_RESSTR(nRID);

    m_aErrDescList.push_back(std::move(pErrDesc));
}


const SmErrorDesc *SmParser::NextError()
{
    if ( !m_aErrDescList.empty() )
        if (m_nCurError > 0) return m_aErrDescList[ --m_nCurError ].get();
        else
        {
            m_nCurError = 0;
            return m_aErrDescList[ m_nCurError ].get();
        }
    else return nullptr;
}


const SmErrorDesc *SmParser::PrevError()
{
    if ( !m_aErrDescList.empty() )
        if (m_nCurError < static_cast<int>(m_aErrDescList.size() - 1)) return m_aErrDescList[ ++m_nCurError ].get();
        else
        {
            m_nCurError = static_cast<int>(m_aErrDescList.size() - 1);
            return m_aErrDescList[ m_nCurError ].get();
        }
    else return nullptr;
}


const SmErrorDesc *SmParser::GetError(size_t i)
{
    if ( i < m_aErrDescList.size() )
        return m_aErrDescList[ i ].get();

    if ( static_cast<size_t>(m_nCurError) < m_aErrDescList.size() )
        return m_aErrDescList[ m_nCurError ].get();

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
