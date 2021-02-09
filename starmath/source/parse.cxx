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

#include <memory>
#include <com/sun/star/i18n/UnicodeType.hpp>
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <i18nlangtag/lang.h>
#include <tools/lineend.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <node.hxx>
#include <parse.hxx>
#include <strings.hrc>
#include <smmod.hxx>
#include <cfgitem.hxx>
#include <cassert>
#include <stack>
#include <starmathdatabase.hxx>

using namespace ::com::sun::star::i18n;

const char* starmathdatabase::SmParseErrorDesc[] = {
    // clang-format off
    RID_ERR_NONE,
    RID_ERR_UNEXPECTEDCHARACTER,
    RID_ERR_UNEXPECTEDTOKEN,
    RID_ERR_POUNDEXPECTED,
    RID_ERR_COLOREXPECTED,
    RID_ERR_LGROUPEXPECTED,
    RID_ERR_RGROUPEXPECTED,
    RID_ERR_LBRACEEXPECTED,
    RID_ERR_RBRACEEXPECTED,
    RID_ERR_PARENTMISMATCH,
    RID_ERR_RIGHTEXPECTED,
    RID_ERR_FONTEXPECTED,
    RID_ERR_SIZEEXPECTED,
    RID_ERR_DOUBLEALIGN,
    RID_ERR_DOUBLESUBSUPSCRIPT,
    RID_ERR_NUMBEREXPECTED
    // clang-format on
};

OUString starmathdatabase::getParseErrorDesc(SmParseError err){
    return SmResId(starmathdatabase::SmParseErrorDesc[static_cast<uint_fast8_t>(err)]);
}

//Definition of math keywords
const SmTokenTableEntry aTokenTable[] =
{
    { u"abs", TABS, '\0', TG::UnOper, 13 },
    { u"acute", TACUTE, MS_ACUTE, TG::Attribute, 5 },
    { u"aleph" , TALEPH, MS_ALEPH, TG::Standalone, 5 },
    { u"alignb", TALIGNC, '\0', TG::Align, 0},
    { u"alignc", TALIGNC, '\0', TG::Align, 0},
    { u"alignl", TALIGNL, '\0', TG::Align, 0},
    { u"alignm", TALIGNC, '\0', TG::Align, 0},
    { u"alignr", TALIGNR, '\0', TG::Align, 0},
    { u"alignt", TALIGNC, '\0', TG::Align, 0},
    { u"and", TAND, MS_AND, TG::Product, 0},
    { u"approx", TAPPROX, MS_APPROX, TG::Relation, 0},
    { u"arccos", TACOS, '\0', TG::Function, 5},
    { u"arccot", TACOT, '\0', TG::Function, 5},
    { u"arcosh", TACOSH, '\0', TG::Function, 5 },
    { u"arcoth", TACOTH, '\0', TG::Function, 5 },
    { u"arcsin", TASIN, '\0', TG::Function, 5},
    { u"arctan", TATAN, '\0', TG::Function, 5},
    { u"arsinh", TASINH, '\0', TG::Function, 5},
    { u"artanh", TATANH, '\0', TG::Function, 5},
    { u"backepsilon" , TBACKEPSILON, MS_BACKEPSILON, TG::Standalone, 5},
    { u"bar", TBAR, MS_BAR, TG::Attribute, 5},
    { u"binom", TBINOM, '\0', TG::NONE, 5 },
    { u"bold", TBOLD, '\0', TG::FontAttr, 5},
    { u"boper", TBOPER, '\0', TG::Product, 0},
    { u"breve", TBREVE, MS_BREVE, TG::Attribute, 5},
    { u"bslash", TBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
    { u"cdot", TCDOT, MS_CDOT, TG::Product, 0},
    { u"check", TCHECK, MS_CHECK, TG::Attribute, 5},
    { u"circ" , TCIRC, MS_CIRC, TG::Standalone, 5},
    { u"circle", TCIRCLE, MS_CIRCLE, TG::Attribute, 5},
    { u"color", TCOLOR, '\0', TG::FontAttr, 5},
    { u"coprod", TCOPROD, MS_COPROD, TG::Oper, 5},
    { u"cos", TCOS, '\0', TG::Function, 5},
    { u"cosh", TCOSH, '\0', TG::Function, 5},
    { u"cot", TCOT, '\0', TG::Function, 5},
    { u"coth", TCOTH, '\0', TG::Function, 5},
    { u"csub", TCSUB, '\0', TG::Power, 0},
    { u"csup", TCSUP, '\0', TG::Power, 0},
    { u"dddot", TDDDOT, MS_DDDOT, TG::Attribute, 5},
    { u"ddot", TDDOT, MS_DDOT, TG::Attribute, 5},
    { u"def", TDEF, MS_DEF, TG::Relation, 0},
    { u"div", TDIV, MS_DIV, TG::Product, 0},
    { u"divides", TDIVIDES, MS_LINE, TG::Relation, 0},
    { u"dlarrow" , TDLARROW, MS_DLARROW, TG::Standalone, 5},
    { u"dlrarrow" , TDLRARROW, MS_DLRARROW, TG::Standalone, 5},
    { u"dot", TDOT, MS_DOT, TG::Attribute, 5},
    { u"dotsaxis", TDOTSAXIS, MS_DOTSAXIS, TG::Standalone, 5}, // 5 to continue expression
    { u"dotsdiag", TDOTSDIAG, MS_DOTSUP, TG::Standalone, 5},
    { u"dotsdown", TDOTSDOWN, MS_DOTSDOWN, TG::Standalone, 5},
    { u"dotslow", TDOTSLOW, MS_DOTSLOW, TG::Standalone, 5},
    { u"dotsup", TDOTSUP, MS_DOTSUP, TG::Standalone, 5},
    { u"dotsvert", TDOTSVERT, MS_DOTSVERT, TG::Standalone, 5},
    { u"downarrow" , TDOWNARROW, MS_DOWNARROW, TG::Standalone, 5},
    { u"drarrow" , TDRARROW, MS_DRARROW, TG::Standalone, 5},
    { u"emptyset" , TEMPTYSET, MS_EMPTYSET, TG::Standalone, 5},
    { u"equiv", TEQUIV, MS_EQUIV, TG::Relation, 0},
    { u"evaluate", TEVALUATE, '\0', TG::NONE, 0},
    { u"exists", TEXISTS, MS_EXISTS, TG::Standalone, 5},
    { u"exp", TEXP, '\0', TG::Function, 5},
    { u"fact", TFACT, MS_FACT, TG::UnOper, 5},
    { u"fixed", TFIXED, '\0', TG::Font, 0},
    { u"font", TFONT, '\0', TG::FontAttr, 5},
    { u"forall", TFORALL, MS_FORALL, TG::Standalone, 5},
    { u"fourier", TFOURIER, MS_FOURIER, TG::Standalone, 5},
    { u"frac", TFRAC, '\0', TG::NONE, 5},
    { u"from", TFROM, '\0', TG::Limit, 0},
    { u"func", TFUNC, '\0', TG::Function, 5},
    { u"ge", TGE, MS_GE, TG::Relation, 0},
    { u"geslant", TGESLANT, MS_GESLANT, TG::Relation, 0 },
    { u"gg", TGG, MS_GG, TG::Relation, 0},
    { u"grave", TGRAVE, MS_GRAVE, TG::Attribute, 5},
    { u"gt", TGT, MS_GT, TG::Relation, 0},
    { u"harpoon", THARPOON, MS_HARPOON, TG::Attribute, 5},
    { u"hat", THAT, MS_HAT, TG::Attribute, 5},
    { u"hbar" , THBAR, MS_HBAR, TG::Standalone, 5},
    { u"hex" , THEX, '\0', TG::NONE, 5},
    { u"iiint", TIIINT, MS_IIINT, TG::Oper, 5},
    { u"iint", TIINT, MS_IINT, TG::Oper, 5},
    { u"im" , TIM, MS_IM, TG::Standalone, 5 },
    { u"in", TIN, MS_IN, TG::Relation, 0},
    { u"infinity" , TINFINITY, MS_INFINITY, TG::Standalone, 5},
    { u"infty" , TINFINITY, MS_INFINITY, TG::Standalone, 5},
    { u"int", TINT, MS_INT, TG::Oper, 5},
    { u"intd", TINTD, MS_INT, TG::Oper, 5},
    { u"intersection", TINTERSECT, MS_INTERSECT, TG::Product, 0},
    { u"it", TIT, '\0', TG::Product, 0},
    { u"ital", TITALIC, '\0', TG::FontAttr, 5},
    { u"italic", TITALIC, '\0', TG::FontAttr, 5},
    { u"lambdabar" , TLAMBDABAR, MS_LAMBDABAR, TG::Standalone, 5},
    { u"langle", TLANGLE, MS_LMATHANGLE, TG::LBrace, 5},
    { u"laplace", TLAPLACE, MS_LAPLACE, TG::Standalone, 5},
    { u"lbrace", TLBRACE, MS_LBRACE, TG::LBrace, 5},
    { u"lceil", TLCEIL, MS_LCEIL, TG::LBrace, 5},
    { u"ldbracket", TLDBRACKET, MS_LDBRACKET, TG::LBrace, 5},
    { u"ldline", TLDLINE, MS_DVERTLINE, TG::LBrace, 5},
    { u"le", TLE, MS_LE, TG::Relation, 0},
    { u"left", TLEFT, '\0', TG::NONE, 5},
    { u"leftarrow" , TLEFTARROW, MS_LEFTARROW, TG::Standalone, 5},
    { u"leslant", TLESLANT, MS_LESLANT, TG::Relation, 0 },
    { u"lfloor", TLFLOOR, MS_LFLOOR, TG::LBrace, 5},
    { u"lim", TLIM, '\0', TG::Oper, 5},
    { u"liminf", TLIMINF, '\0', TG::Oper, 5},
    { u"limsup", TLIMSUP, '\0', TG::Oper, 5},
    { u"lint", TLINT, MS_LINT, TG::Oper, 5},
    { u"ll", TLL, MS_LL, TG::Relation, 0},
    { u"lline", TLLINE, MS_VERTLINE, TG::LBrace, 5},
    { u"llint", TLLINT, MS_LLINT, TG::Oper, 5},
    { u"lllint", TLLLINT, MS_LLLINT, TG::Oper, 5},
    { u"ln", TLN, '\0', TG::Function, 5},
    { u"log", TLOG, '\0', TG::Function, 5},
    { u"lrline", TLRLINE, MS_VERTLINE, TG::LBrace | TG::RBrace, 5},
    { u"lrdline", TLRDLINE, MS_VERTLINE, TG::LBrace | TG::RBrace, 5},
    { u"lsub", TLSUB, '\0', TG::Power, 0},
    { u"lsup", TLSUP, '\0', TG::Power, 0},
    { u"lt", TLT, MS_LT, TG::Relation, 0},
    { u"matrix", TMATRIX, '\0', TG::NONE, 5},
    { u"minusplus", TMINUSPLUS, MS_MINUSPLUS, TG::UnOper | TG::Sum, 5},
    { u"mline", TMLINE, MS_VERTLINE, TG::NONE, 0},      //! not in TG::RBrace, Level 0
    { u"nabla", TNABLA, MS_NABLA, TG::Standalone, 5},
    { u"nbold", TNBOLD, '\0', TG::FontAttr, 5},
    { u"ndivides", TNDIVIDES, MS_NDIVIDES, TG::Relation, 0},
    { u"neg", TNEG, MS_NEG, TG::UnOper, 5 },
    { u"neq", TNEQ, MS_NEQ, TG::Relation, 0},
    { u"newline", TNEWLINE, '\0', TG::NONE, 0},
    { u"ni", TNI, MS_NI, TG::Relation, 0},
    { u"nitalic", TNITALIC, '\0', TG::FontAttr, 5},
    { u"none", TNONE, '\0', TG::LBrace | TG::RBrace, 0},
    { u"nospace", TNOSPACE, '\0', TG::Standalone, 5},
    { u"notexists", TNOTEXISTS, MS_NOTEXISTS, TG::Standalone, 5},
    { u"notin", TNOTIN, MS_NOTIN, TG::Relation, 0},
    { u"nprec", TNOTPRECEDES, MS_NOTPRECEDES, TG::Relation, 0 },
    { u"nroot", TNROOT, MS_SQRT, TG::UnOper, 5},
    { u"nsubset", TNSUBSET, MS_NSUBSET, TG::Relation, 0 },
    { u"nsubseteq", TNSUBSETEQ, MS_NSUBSETEQ, TG::Relation, 0 },
    { u"nsucc", TNOTSUCCEEDS, MS_NOTSUCCEEDS, TG::Relation, 0 },
    { u"nsupset", TNSUPSET, MS_NSUPSET, TG::Relation, 0 },
    { u"nsupseteq", TNSUPSETEQ, MS_NSUPSETEQ, TG::Relation, 0 },
    { u"odivide", TODIVIDE, MS_ODIVIDE, TG::Product, 0},
    { u"odot", TODOT, MS_ODOT, TG::Product, 0},
    { u"ominus", TOMINUS, MS_OMINUS, TG::Sum, 0},
    { u"oper", TOPER, '\0', TG::Oper, 5},
    { u"oplus", TOPLUS, MS_OPLUS, TG::Sum, 0},
    { u"or", TOR, MS_OR, TG::Sum, 0},
    { u"ortho", TORTHO, MS_ORTHO, TG::Relation, 0},
    { u"otimes", TOTIMES, MS_OTIMES, TG::Product, 0},
    { u"over", TOVER, '\0', TG::Product, 0},
    { u"overbrace", TOVERBRACE, MS_OVERBRACE, TG::Product, 5},
    { u"overline", TOVERLINE, '\0', TG::Attribute, 5},
    { u"overstrike", TOVERSTRIKE, '\0', TG::Attribute, 5},
    { u"owns", TNI, MS_NI, TG::Relation, 0},
    { u"parallel", TPARALLEL, MS_DLINE, TG::Relation, 0},
    { u"partial", TPARTIAL, MS_PARTIAL, TG::Standalone, 5 },
    { u"phantom", TPHANTOM, '\0', TG::FontAttr, 5},
    { u"plusminus", TPLUSMINUS, MS_PLUSMINUS, TG::UnOper | TG::Sum, 5},
    { u"prec", TPRECEDES, MS_PRECEDES, TG::Relation, 0 },
    { u"preccurlyeq", TPRECEDESEQUAL, MS_PRECEDESEQUAL, TG::Relation, 0 },
    { u"precsim", TPRECEDESEQUIV, MS_PRECEDESEQUIV, TG::Relation, 0 },
    { u"prod", TPROD, MS_PROD, TG::Oper, 5},
    { u"prop", TPROP, MS_PROP, TG::Relation, 0},
    { u"rangle", TRANGLE, MS_RMATHANGLE, TG::RBrace, 0},  //! 0 to terminate expression
    { u"rbrace", TRBRACE, MS_RBRACE, TG::RBrace, 0},
    { u"rceil", TRCEIL, MS_RCEIL, TG::RBrace, 0},
    { u"rdbracket", TRDBRACKET, MS_RDBRACKET, TG::RBrace, 0},
    { u"rdline", TRDLINE, MS_DVERTLINE, TG::RBrace, 0},
    { u"re" , TRE, MS_RE, TG::Standalone, 5 },
    { u"rfloor", TRFLOOR, MS_RFLOOR, TG::RBrace, 0},  //! 0 to terminate expression
    { u"right", TRIGHT, '\0', TG::NONE, 0},
    { u"rightarrow" , TRIGHTARROW, MS_RIGHTARROW, TG::Standalone, 5},
    { u"rline", TRLINE, MS_VERTLINE, TG::RBrace, 0},  //! 0 to terminate expression
    { u"rsub", TRSUB, '\0', TG::Power, 0},
    { u"rsup", TRSUP, '\0', TG::Power, 0},
    { u"sans", TSANS, '\0', TG::Font, 0},
    { u"serif", TSERIF, '\0', TG::Font, 0},
    { u"setC" , TSETC, MS_SETC, TG::Standalone, 5},
    { u"setminus", TSETMINUS, MS_BACKSLASH, TG::Product, 0 },
    { u"setN" , TSETN, MS_SETN, TG::Standalone, 5},
    { u"setQ" , TSETQ, MS_SETQ, TG::Standalone, 5},
    { u"setquotient", TSETQUOTIENT, MS_SLASH, TG::Product, 0 },
    { u"setR" , TSETR, MS_SETR, TG::Standalone, 5},
    { u"setZ" , TSETZ, MS_SETZ, TG::Standalone, 5},
    { u"sim", TSIM, MS_SIM, TG::Relation, 0},
    { u"simeq", TSIMEQ, MS_SIMEQ, TG::Relation, 0},
    { u"sin", TSIN, '\0', TG::Function, 5},
    { u"sinh", TSINH, '\0', TG::Function, 5},
    { u"size", TSIZE, '\0', TG::FontAttr, 5},
    { u"slash", TSLASH, MS_SLASH, TG::Product, 0 },
    { u"sqrt", TSQRT, MS_SQRT, TG::UnOper, 5},
    { u"stack", TSTACK, '\0', TG::NONE, 5},
    { u"sub", TRSUB, '\0', TG::Power, 0},
    { u"subset", TSUBSET, MS_SUBSET, TG::Relation, 0},
    { u"subseteq", TSUBSETEQ, MS_SUBSETEQ, TG::Relation, 0},
    { u"succ", TSUCCEEDS, MS_SUCCEEDS, TG::Relation, 0 },
    { u"succcurlyeq", TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, TG::Relation, 0 },
    { u"succsim", TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, TG::Relation, 0 },
    { u"sum", TSUM, MS_SUM, TG::Oper, 5},
    { u"sup", TRSUP, '\0', TG::Power, 0},
    { u"supset", TSUPSET, MS_SUPSET, TG::Relation, 0},
    { u"supseteq", TSUPSETEQ, MS_SUPSETEQ, TG::Relation, 0},
    { u"tan", TTAN, '\0', TG::Function, 5},
    { u"tanh", TTANH, '\0', TG::Function, 5},
    { u"tilde", TTILDE, MS_TILDE, TG::Attribute, 5},
    { u"times", TTIMES, MS_TIMES, TG::Product, 0},
    { u"to", TTO, '\0', TG::Limit, 0},
    { u"toward", TTOWARD, MS_RIGHTARROW, TG::Relation, 0},
    { u"transl", TTRANSL, MS_TRANSL, TG::Relation, 0},
    { u"transr", TTRANSR, MS_TRANSR, TG::Relation, 0},
    { u"underbrace", TUNDERBRACE, MS_UNDERBRACE, TG::Product, 5},
    { u"underline", TUNDERLINE, '\0', TG::Attribute, 5},
    { u"union", TUNION, MS_UNION, TG::Sum, 0},
    { u"uoper", TUOPER, '\0', TG::UnOper, 5},
    { u"uparrow" , TUPARROW, MS_UPARROW, TG::Standalone, 5},
    { u"vec", TVEC, MS_VEC, TG::Attribute, 5},
    { u"widebslash", TWIDEBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
    { u"wideharpoon", TWIDEHARPOON, MS_HARPOON, TG::Attribute, 5},
    { u"widehat", TWIDEHAT, MS_HAT, TG::Attribute, 5},
    { u"wideslash", TWIDESLASH, MS_SLASH, TG::Product, 0 },
    { u"widetilde", TWIDETILDE, MS_TILDE, TG::Attribute, 5},
    { u"widevec", TWIDEVEC, MS_VEC, TG::Attribute, 5},
    { u"wp" , TWP, MS_WP, TG::Standalone, 5}
};

// First character may be any alphabetic
const sal_Int32 coStartFlags = KParseTokens::ANY_LETTER | KParseTokens::IGNORE_LEADING_WS;

// Continuing characters may be any alphabetic
const sal_Int32 coContFlags = (coStartFlags & ~KParseTokens::IGNORE_LEADING_WS)
                              | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;
// First character for numbers, may be any numeric or dot
const sal_Int32 coNumStartFlags = KParseTokens::ASC_DIGIT | KParseTokens::ASC_DOT
                                  | KParseTokens::IGNORE_LEADING_WS;
// Continuing characters for numbers, may be any numeric or dot or comma.
// tdf#127873: additionally accept ',' comma group separator as too many
// existing documents unwittingly may have used that as decimal separator
// in such locales (though it never was as this is always the en-US locale
// and the group separator is only parsed away).
const sal_Int32 coNumContFlags = (coNumStartFlags & ~KParseTokens::IGNORE_LEADING_WS)
                                 | KParseTokens::GROUP_SEPARATOR_IN_NUMBER;
// First character for numbers hexadecimal
const sal_Int32 coNum16StartFlags = KParseTokens::ASC_DIGIT | KParseTokens::ASC_UPALPHA
                                    | KParseTokens::IGNORE_LEADING_WS;

// Continuing characters for numbers hexadecimal
const sal_Int32 coNum16ContFlags = (coNum16StartFlags & ~KParseTokens::IGNORE_LEADING_WS);
// user-defined char continuing characters may be any alphanumeric or dot.
const sal_Int32 coUserDefinedCharContFlags = KParseTokens::ANY_LETTER_OR_NUMBER
                                             | KParseTokens::ASC_DOT
                                             | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

//Checks if keyword is in the list.
static inline bool findCompare(const SmTokenTableEntry & lhs, const OUString & s)
{
    return s.compareToIgnoreAsciiCase(lhs.pIdent) > 0;
}

//Returns the SmTokenTableEntry for a keyword
static const SmTokenTableEntry * GetTokenTableEntry( const OUString &rName )
{
    if (rName.isEmpty())return nullptr; //avoid null pointer exceptions
    //Looks for the first keyword after or equal to rName in alphabetical order.
    auto findIter = std::lower_bound( std::begin(aTokenTable),
                                      std::end(aTokenTable), rName, findCompare );
    if ( findIter != std::end(aTokenTable) && rName.equalsIgnoreAsciiCase( findIter->pIdent ))
        return &*findIter; //check is equal
    return nullptr; //not found
}

static bool IsDelimiter( const OUString &rTxt, sal_Int32 nPos )
{   // returns 'true' iff cChar is '\0' or a delimiter

    assert(nPos <= rTxt.getLength()); //index out of range
    if (nPos == rTxt.getLength())return true; //This is EOF
    sal_Unicode cChar = rTxt[nPos];

    // check if 'cChar' is in the delimiter table
    static const sal_Unicode aDelimiterTable[] =
    {
        ' ', '{',  '}',  '(',  ')', '\t', '\n', '\r', '+',  '-',
        '*',  '/',  '=',  '[',  ']',  '^',  '_',  '#',
        '%',  '>',  '<',  '&',  '|', '\\', '"',  '~',  '`'
    };//reordered by usage (by eye) for nanoseconds saving.

    //checks the array
    for (auto const &cDelimiter : aDelimiterTable)
    {
        if (cDelimiter == cChar)return true;
    }

    //special chars support
    sal_Int16 nTypJp = SM_MOD()->GetSysLocale().GetCharClass().getType( rTxt, nPos );
    return ( nTypJp == css::i18n::UnicodeType::SPACE_SEPARATOR ||
             nTypJp == css::i18n::UnicodeType::CONTROL);
}

// checks number used as arguments in Math formulas (e.g. 'size' command)
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
            if(bPoint) return false;
            else bPoint = true;
        }
        else if ( !rtl::isAsciiDigit( cChar ) ) return false;
    }
    return true;
}
// checks number used as arguments in Math formulas (e.g. 'size' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNotWholeNumber(const OUString& rText)
{
    const sal_Unicode* pBuffer = rText.getStr();
    for(sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
        if ( !rtl::isAsciiDigit( *pBuffer ) ) return true;
    return false;
}
// checks hex number used as arguments in Math formulas (e.g. 'hex' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNotWholeNumber16(const OUString& rText)
{
    const sal_Unicode* pBuffer = rText.getStr();
    for(sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
        if ( !rtl::isAsciiCanonicHexDigit( *pBuffer ) ) return true;
    return false;
}

//Text replace onto m_aBufferString
void SmParser::Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText )
{
    assert( nPos + nLen <= m_aBufferString.getLength() ); //checks if length allows text replace

    m_aBufferString = m_aBufferString.replaceAt( nPos, nLen, rText ); //replace and reindex
    sal_Int32 nChg = rText.getLength() - nLen;
    m_nBufferIndex = m_nBufferIndex + nChg;
    m_nTokenIndex = m_nTokenIndex + nChg;
}

void SmParser::NextToken() //Central part of the parser
{

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
        m_aCurToken.cMathChar = u"";
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
        m_aCurToken.cMathChar  = u"";
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.copy( nRealStart, n );

        SAL_WARN_IF( !IsDelimiter( m_aBufferString, aRes.EndPos ), "starmath", "identifier really finished? (compatibility!)" );
    }
    else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
    {
        m_aCurToken.eType      = TTEXT;
        m_aCurToken.cMathChar  = u"";
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
            m_aCurToken.setChar(pEntry->cMathChar);
            m_aCurToken.nGroup     = pEntry->nGroup;
            m_aCurToken.nLevel     = pEntry->nLevel;
            m_aCurToken.aText      = pEntry->pIdent;
        }
        else
        {
            m_aCurToken.eType      = TIDENT;
            m_aCurToken.cMathChar  = u"";
            m_aCurToken.nGroup     = TG::NONE;
            m_aCurToken.nLevel     = 5;
            m_aCurToken.aText      = aName;

            SAL_WARN_IF(!IsDelimiter(m_aBufferString, aRes.EndPos),"starmath", "identifier really finished? (compatibility!)");
        }
    }
    else if (aRes.TokenType == 0  &&  '_' == m_aBufferString[ nRealStart ])
    {
        m_aCurToken.eType    = TRSUB;
        m_aCurToken.cMathChar = u"";
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
                            m_aCurToken.setChar(MS_LL);
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<<";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<=", nRealStart))
                        {
                            m_aCurToken.eType    = TLE;
                            m_aCurToken.setChar(MS_LE);
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<=";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<-", nRealStart))
                        {
                            m_aCurToken.eType    = TLEFTARROW;
                            m_aCurToken.setChar(MS_LEFTARROW);
                            m_aCurToken.nGroup       = TG::Standalone;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "<-";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<>", nRealStart))
                        {
                            m_aCurToken.eType    = TNEQ;
                            m_aCurToken.setChar(MS_NEQ);
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "<>";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("<?>", nRealStart))
                        {
                            m_aCurToken.eType    = TPLACE;
                            m_aCurToken.setChar(MS_PLACE);
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "<?>";

                            rnEndPos = nRealStart + 3;
                        }
                        else
                        {
                            m_aCurToken.eType    = TLT;
                            m_aCurToken.setChar(MS_LT);
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
                            m_aCurToken.setChar(MS_GE);
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = ">=";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match(">>", nRealStart))
                        {
                            m_aCurToken.eType    = TGG;
                            m_aCurToken.setChar(MS_GG);
                            m_aCurToken.nGroup       = TG::Relation;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = ">>";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TGT;
                            m_aCurToken.setChar(MS_GT);
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
                        m_aCurToken.cMathChar  = u"";
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
                        m_aCurToken.setChar(MS_LBRACKET);
                        m_aCurToken.nGroup       = TG::LBrace;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "[";
                    }
                    break;
                case '\\':
                    {
                        m_aCurToken.eType    = TESCAPE;
                        m_aCurToken.cMathChar = u"";
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "\\";
                    }
                    break;
                case ']':
                    {
                        m_aCurToken.eType    = TRBRACKET;
                        m_aCurToken.setChar(MS_RBRACKET);
                        m_aCurToken.nGroup       = TG::RBrace;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "]";
                    }
                    break;
                case '^':
                    {
                        m_aCurToken.eType    = TRSUP;
                        m_aCurToken.cMathChar = u"";
                        m_aCurToken.nGroup       = TG::Power;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "^";
                    }
                    break;
                case '`':
                    {
                        m_aCurToken.eType    = TSBLANK;
                        m_aCurToken.cMathChar = u"";
                        m_aCurToken.nGroup       = TG::Blank;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "`";
                    }
                    break;
                case '{':
                    {
                        m_aCurToken.eType    = TLGROUP;
                        m_aCurToken.setChar(MS_LBRACE);
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 5;
                        m_aCurToken.aText = "{";
                    }
                    break;
                case '|':
                    {
                        m_aCurToken.eType    = TOR;
                        m_aCurToken.setChar(MS_OR);
                        m_aCurToken.nGroup       = TG::Sum;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "|";
                    }
                    break;
                case '}':
                    {
                        m_aCurToken.eType    = TRGROUP;
                        m_aCurToken.setChar(MS_RBRACE);
                        m_aCurToken.nGroup       = TG::NONE;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "}";
                    }
                    break;
                case '~':
                    {
                        m_aCurToken.eType    = TBLANK;
                        m_aCurToken.cMathChar = u"";
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
                            m_aCurToken.cMathChar = u"";
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "##";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPOUND;
                            m_aCurToken.cMathChar = u"";
                            m_aCurToken.nGroup       = TG::NONE;
                            m_aCurToken.nLevel       = 0;
                            m_aCurToken.aText = "#";
                        }
                    }
                    break;
                case '&':
                    {
                        m_aCurToken.eType    = TAND;
                        m_aCurToken.setChar(MS_AND);
                        m_aCurToken.nGroup       = TG::Product;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "&";
                    }
                    break;
                case '(':
                    {
                        m_aCurToken.eType    = TLPARENT;
                        m_aCurToken.setChar(MS_LPARENT);
                        m_aCurToken.nGroup       = TG::LBrace;
                        m_aCurToken.nLevel       = 5;     //! 0 to continue expression
                        m_aCurToken.aText = "(";
                    }
                    break;
                case ')':
                    {
                        m_aCurToken.eType    = TRPARENT;
                        m_aCurToken.setChar(MS_RPARENT);
                        m_aCurToken.nGroup       = TG::RBrace;
                        m_aCurToken.nLevel       = 0;     //! 0 to terminate expression
                        m_aCurToken.aText = ")";
                    }
                    break;
                case '*':
                    {
                        m_aCurToken.eType    = TMULTIPLY;
                        m_aCurToken.setChar(MS_MULTIPLY);
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
                            m_aCurToken.setChar(MS_PLUSMINUS);
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "+-";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TPLUS;
                            m_aCurToken.setChar(MS_PLUS);
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
                            m_aCurToken.setChar(MS_MINUSPLUS);
                            m_aCurToken.nGroup       = TG::UnOper | TG::Sum;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "-+";

                            rnEndPos = nRealStart + 2;
                        }
                        else if (m_aBufferString.match("->", nRealStart))
                        {
                            m_aCurToken.eType    = TRIGHTARROW;
                            m_aCurToken.setChar(MS_RIGHTARROW);
                            m_aCurToken.nGroup       = TG::Standalone;
                            m_aCurToken.nLevel       = 5;
                            m_aCurToken.aText = "->";

                            rnEndPos = nRealStart + 2;
                        }
                        else
                        {
                            m_aCurToken.eType    = TMINUS;
                            m_aCurToken.setChar(MS_MINUS);
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
                            m_aCurToken.cMathChar = u"";
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
                        m_aCurToken.setChar(MS_SLASH);
                        m_aCurToken.nGroup       = TG::Product;
                        m_aCurToken.nLevel       = 0;
                        m_aCurToken.aText = "/";
                    }
                    break;
                case '=':
                    {
                        m_aCurToken.eType    = TASSIGN;
                        m_aCurToken.setChar(MS_ASSIGN);
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
        m_aCurToken.cMathChar  = u"";
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;

        // tdf#129372: we may have to deal with surrogate pairs
        // (see https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates)
        // in this case, we must read 2 sal_Unicode instead of 1
        int nOffset(rtl::isSurrogate(m_aBufferString[nRealStart])? 2 : 1);
        m_aCurToken.aText      = m_aBufferString.copy( nRealStart, nOffset );

        aRes.EndPos = nRealStart + nOffset;
    }

    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = aRes.EndPos;
}

void SmParser::NextTokenColor(SmTokenType dvipload)
{

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
        //parse, there are few options, so less strict.
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex,
                                       coStartFlags, "", coContFlags, "");
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
    m_nTokenIndex    = m_nBufferIndex;
    m_aCurToken.nRow = m_nRow;
    m_aCurToken.nCol = nRealStart - m_nColOff + 1;
    if (nRealStart >= nBufLen) m_aCurToken.eType = TEND;
    else if (aRes.TokenType & KParseType::IDENTNAME)
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        OUString aName( m_aBufferString.copy( nRealStart, n ) );
        switch(dvipload)
        {
            case TCOLOR:
                m_aCurToken = starmathdatabase::Identify_ColorName_Parser( aName );
                break;
            case TDVIPSNAMESCOL:
                m_aCurToken = starmathdatabase::Identify_ColorName_DVIPSNAMES( aName );
                break;
            default:
                m_aCurToken = starmathdatabase::Identify_ColorName_Parser( aName );
                break;
        }
    }
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        if( m_aBufferString[ nRealStart ] == '#' && !m_aBufferString.match("##", nRealStart) )
        {
            m_aCurToken.eType    = THEX;
            m_aCurToken.cMathChar = u"";
            m_aCurToken.nGroup       = TG::Color;
            m_aCurToken.nLevel       = 0;
            m_aCurToken.aText = "hex";
        }
    }
    else m_aCurToken.eType         = TNONE;
    if (TEND != m_aCurToken.eType) m_nBufferIndex = aRes.EndPos;
}

void SmParser::NextTokenFontSize()
{

    sal_Int32   nBufLen = m_aBufferString.getLength();
    ParseResult aRes;
    sal_Int32   nRealStart;
    bool        bCont;
    bool        hex = false;

    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR ==
                    m_pSysCC->getType( m_aBufferString, m_nBufferIndex ))
           ++m_nBufferIndex;
        //hexadecimal parser
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex,
                                       coNum16StartFlags, ".", coNum16ContFlags, ".,");
        if (aRes.TokenType == 0)
        {
            // Try again with the default token parsing.
            aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex,
                                     coStartFlags, "", coContFlags, "");
        }
        else hex = true;
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
    m_nTokenIndex      = m_nBufferIndex;
    m_aCurToken.nRow   = m_nRow;
    m_aCurToken.nCol   = nRealStart - m_nColOff + 1;
    if (nRealStart >= nBufLen) m_aCurToken.eType    = TEND;
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        if ( aRes.EndPos - nRealStart == 1 )
        {
            switch ( m_aBufferString[ nRealStart ] )
            {
                case '*':
                    m_aCurToken.eType     = TMULTIPLY;
                    m_aCurToken.setChar(MS_MULTIPLY);
                    m_aCurToken.nGroup    = TG::Product;
                    m_aCurToken.nLevel    = 0;
                    m_aCurToken.aText     = "*";
                    break;
                case '+':
                    m_aCurToken.eType     = TPLUS;
                    m_aCurToken.setChar(MS_PLUS);
                    m_aCurToken.nGroup    = TG::UnOper | TG::Sum;
                    m_aCurToken.nLevel    = 5;
                    m_aCurToken.aText     = "+";
                    break;
                case '-':
                    m_aCurToken.eType     = TMINUS;
                    m_aCurToken.setChar(MS_MINUS);
                    m_aCurToken.nGroup    = TG::UnOper | TG::Sum;
                    m_aCurToken.nLevel    = 5;
                    m_aCurToken.aText     = "-";
                    break;
                case '/':
                    m_aCurToken.eType     = TDIVIDEBY;
                    m_aCurToken.setChar(MS_SLASH);
                    m_aCurToken.nGroup    = TG::Product;
                    m_aCurToken.nLevel    = 0;
                    m_aCurToken.aText     = "/";
                    break;
                default:
                    m_aCurToken.eType     = TNONE;
                    break;
            }
        }
        else m_aCurToken.eType = TNONE;
    }
    else if(hex)
    {
        assert(aRes.EndPos > 0);
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        m_aCurToken.eType      = THEX;
        m_aCurToken.cMathChar  = u"";
        m_aCurToken.nGroup     = TG::NONE;
        m_aCurToken.nLevel     = 5;
        m_aCurToken.aText      = m_aBufferString.copy( nRealStart, n );
    }
    else m_aCurToken.eType     = TNONE;
    if (TEND != m_aCurToken.eType) m_nBufferIndex = aRes.EndPos;
}

namespace
{
    SmNodeArray buildNodeArray(std::vector<std::unique_ptr<SmNode>>& rSubNodes)
    {
        SmNodeArray aSubArray(rSubNodes.size());
        for (size_t i = 0; i < rSubNodes.size(); ++i)
            aSubArray[i] = rSubNodes[i].release();
        return aSubArray;
    }
} //end namespace

// grammar
/*************************************************************************************************/

std::unique_ptr<SmTableNode> SmParser::DoTable()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::vector<std::unique_ptr<SmNode>> aLineArray;
    aLineArray.push_back(DoLine());
    while (m_aCurToken.eType == TNEWLINE)
    {
        NextToken();
        aLineArray.push_back(DoLine());
    }
    assert(m_aCurToken.eType == TEND);
    std::unique_ptr<SmTableNode> xSNode(new SmTableNode(m_aCurToken));
    xSNode->SetSubNodes(buildNodeArray(aLineArray));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser::DoAlign(bool bUseExtraSpaces)
    // parse alignment info (if any), then go on with rest of expression
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmStructureNode> xSNode;

    if (TokenInGroup(TG::Align))
    {
        xSNode.reset(new SmAlignNode(m_aCurToken));

        NextToken();

        // allow for just one align statement in 5.0
        if (TokenInGroup(TG::Align))
            return DoError(SmParseError::DoubleAlign);
    }

    auto pNode = DoExpression(bUseExtraSpaces);

    if (xSNode)
    {
        xSNode->SetSubNode(0, pNode.release());
        return xSNode;
    }
    return pNode;
}

// Postcondition: m_aCurToken.eType == TEND || m_aCurToken.eType == TNEWLINE
std::unique_ptr<SmNode> SmParser::DoLine()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::vector<std::unique_ptr<SmNode>> ExpressionArray;

    // start with single expression that may have an alignment statement
    // (and go on with expressions that must not have alignment
    // statements in 'while' loop below. See also 'Expression()'.)
    if (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
        ExpressionArray.push_back(DoAlign());

    while (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TNEWLINE)
        ExpressionArray.push_back(DoExpression());

    //If there's no expression, add an empty one.
    //this is to avoid a formula tree without any caret
    //positions, in visual formula editor.
    if(ExpressionArray.empty())
    {
        SmToken aTok;
        aTok.eType = TNEWLINE;
        ExpressionArray.emplace_back(std::unique_ptr<SmNode>(new SmExpressionNode(aTok)));
    }

    auto xSNode = std::make_unique<SmLineNode>(m_aCurToken);
    xSNode->SetSubNodes(buildNodeArray(ExpressionArray));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser::DoExpression(bool bUseExtraSpaces)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::vector<std::unique_ptr<SmNode>> RelationArray;
    RelationArray.push_back(DoRelation());
    while (m_aCurToken.nLevel >= 4)
        RelationArray.push_back(DoRelation());

    if (RelationArray.size() > 1)
    {
        std::unique_ptr<SmExpressionNode> xSNode(new SmExpressionNode(m_aCurToken));
        xSNode->SetSubNodes(buildNodeArray(RelationArray));
        xSNode->SetUseExtraSpaces(bUseExtraSpaces);
        return xSNode;
    }
    else
    {
        // This expression has only one node so just push this node.
        return std::move(RelationArray[0]);
    }
}

std::unique_ptr<SmNode> SmParser::DoRelation()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    int nDepthLimit = m_nParseDepth;

    auto xFirst = DoSum();
    while (TokenInGroup(TG::Relation))
    {
        std::unique_ptr<SmStructureNode> xSNode(new SmBinHorNode(m_aCurToken));
        auto xSecond = DoOpSubSup();
        auto xThird = DoSum();
        xSNode->SetSubNodes(std::move(xFirst), std::move(xSecond), std::move(xThird));
        xFirst = std::move(xSNode);

        ++m_nParseDepth;
        DepthProtect bDepthGuard(m_nParseDepth);
    }

    m_nParseDepth = nDepthLimit;

    return xFirst;
}

std::unique_ptr<SmNode> SmParser::DoSum()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    int nDepthLimit = m_nParseDepth;

    auto xFirst = DoProduct();
    while (TokenInGroup(TG::Sum))
    {
        std::unique_ptr<SmStructureNode> xSNode(new SmBinHorNode(m_aCurToken));
        auto xSecond = DoOpSubSup();
        auto xThird = DoProduct();
        xSNode->SetSubNodes(std::move(xFirst), std::move(xSecond), std::move(xThird));
        xFirst = std::move(xSNode);

        ++m_nParseDepth;
        DepthProtect bDepthGuard(m_nParseDepth);
    }

    m_nParseDepth = nDepthLimit;

    return xFirst;
}

std::unique_ptr<SmNode> SmParser::DoProduct()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto xFirst = DoPower();

    int nDepthLimit = 0;

    while (TokenInGroup(TG::Product))
    {
        //this linear loop builds a recursive structure, if it gets
        //too deep then later processing, e.g. releasing the tree,
        //can exhaust stack
        if (m_nParseDepth + nDepthLimit > DEPTH_LIMIT)
            throw std::range_error("parser depth limit");

        std::unique_ptr<SmStructureNode> xSNode;
        std::unique_ptr<SmNode> xOper;

        SmTokenType eType = m_aCurToken.eType;
        switch (eType)
        {
            case TOVER:
                xSNode.reset(new SmBinVerNode(m_aCurToken));
                xOper.reset(new SmRectangleNode(m_aCurToken));
                NextToken();
                break;

            case TBOPER:
                xSNode.reset(new SmBinHorNode(m_aCurToken));

                NextToken();

                //Let the glyph node know it's a binary operation
                m_aCurToken.eType = TBOPER;
                m_aCurToken.nGroup = TG::Product;
                xOper = DoGlyphSpecial();
                break;

            case TOVERBRACE :
            case TUNDERBRACE :
                xSNode.reset(new SmVerticalBraceNode(m_aCurToken));
                xOper.reset(new SmMathSymbolNode(m_aCurToken));

                NextToken();
                break;

            case TWIDEBACKSLASH:
            case TWIDESLASH:
            {
                SmBinDiagonalNode *pSTmp = new SmBinDiagonalNode(m_aCurToken);
                pSTmp->SetAscending(eType == TWIDESLASH);
                xSNode.reset(pSTmp);

                xOper.reset(new SmPolyLineNode(m_aCurToken));
                NextToken();

                break;
            }

            default:
                xSNode.reset(new SmBinHorNode(m_aCurToken));

                xOper = DoOpSubSup();
        }

        auto xArg = DoPower();
        xSNode->SetSubNodesBinMo(std::move(xFirst), std::move(xOper), std::move(xArg));
        xFirst = std::move(xSNode);
        ++nDepthLimit;
    }
    return xFirst;
}

std::unique_ptr<SmNode> SmParser::DoSubSup(TG nActiveGroup, std::unique_ptr<SmNode> xGivenNode)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(nActiveGroup == TG::Power || nActiveGroup == TG::Limit);
    assert(m_aCurToken.nGroup == nActiveGroup);

    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(m_aCurToken));
    //! Of course 'm_aCurToken' is just the first sub-/supscript token.
    //! It should be of no further interest. The positions of the
    //! sub-/supscripts will be identified by the corresponding subnodes
    //! index in the 'aSubNodes' array (enum value from 'SmSubSup').

    pNode->SetUseLimits(nActiveGroup == TG::Limit);

    // initialize subnodes array
    std::vector<std::unique_ptr<SmNode>> aSubNodes(1 + SUBSUP_NUM_ENTRIES);
    aSubNodes[0] = std::move(xGivenNode);

    // process all sub-/supscripts
    int  nIndex = 0;
    while (TokenInGroup(nActiveGroup))
    {
        SmTokenType  eType (m_aCurToken.eType);

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

        std::unique_ptr<SmNode> xENode;
        if (aSubNodes[nIndex]) // if already occupied at earlier iteration
        {
            // forget the earlier one, remember an error instead
            aSubNodes[nIndex].reset();
            xENode = DoError(SmParseError::DoubleSubsupscript); // this also skips current token.
        }
        else
        {
            // skip sub-/supscript token
            NextToken();
        }

        // get sub-/supscript node
        // (even when we saw a double-sub/supscript error in the above
        // in order to minimize mess and continue parsing.)
        std::unique_ptr<SmNode> xSNode;
        if (eType == TFROM  ||  eType == TTO)
        {
            // parse limits in old 4.0 and 5.0 style
            xSNode = DoRelation();
        }
        else
            xSNode = DoTerm(true);

        aSubNodes[nIndex] = std::move(xENode ? xENode : xSNode);
    }

    pNode->SetSubNodes(buildNodeArray(aSubNodes));
    return pNode;
}

std::unique_ptr<SmNode> SmParser::DoSubSupEvaluate(std::unique_ptr<SmNode> xGivenNode)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(m_aCurToken));
    pNode->SetUseLimits(true);

    // initialize subnodes array
    std::vector<std::unique_ptr<SmNode>> aSubNodes(1 + SUBSUP_NUM_ENTRIES);
    aSubNodes[0] = std::move(xGivenNode);

    // process all sub-/supscripts
    int  nIndex = 0;
    while (TokenInGroup(TG::Limit))
    {
        SmTokenType  eType (m_aCurToken.eType);

        switch (eType)
        {
            case TFROM :    nIndex = static_cast<int>(RSUB);    break;
            case TTO   :    nIndex = static_cast<int>(RSUP);    break;
            default :
                SAL_WARN( "starmath", "unknown case");
        }
        nIndex++;
        assert(1 <= nIndex  &&  nIndex <= SUBSUP_NUM_ENTRIES);

        std::unique_ptr<SmNode> xENode;
        if (aSubNodes[nIndex]) // if already occupied at earlier iteration
        {
            // forget the earlier one, remember an error instead
            aSubNodes[nIndex].reset();
            xENode = DoError(SmParseError::DoubleSubsupscript); // this also skips current token.
        }
        else NextToken(); // skip sub-/supscript token

        // get sub-/supscript node
        std::unique_ptr<SmNode> xSNode;
        xSNode = DoTerm(true);

        aSubNodes[nIndex] = std::move(xENode ? xENode : xSNode);
    }

    pNode->SetSubNodes(buildNodeArray(aSubNodes));
    return pNode;
}

std::unique_ptr<SmNode> SmParser::DoOpSubSup()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // get operator symbol
    auto xNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
    // skip operator token
    NextToken();
    // get sub- supscripts if any
    if (m_aCurToken.nGroup == TG::Power)
        return DoSubSup(TG::Power, std::move(xNode));
    return xNode;
}

std::unique_ptr<SmNode> SmParser::DoPower()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // get body for sub- supscripts on top of stack
    std::unique_ptr<SmNode> xNode(DoTerm(false));

    if (m_aCurToken.nGroup == TG::Power)
        return DoSubSup(TG::Power, std::move(xNode));
    return xNode;
}

std::unique_ptr<SmBlankNode> SmParser::DoBlank()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Blank));
    std::unique_ptr<SmBlankNode> pBlankNode(new SmBlankNode(m_aCurToken));

    do
    {
        pBlankNode->IncreaseBy(m_aCurToken);
        NextToken();
    }
    while (TokenInGroup(TG::Blank));

    // Ignore trailing spaces, if corresponding option is set
    if ( m_aCurToken.eType == TNEWLINE ||
             (m_aCurToken.eType == TEND && !utl::ConfigManager::IsFuzzing() && SM_MOD()->GetConfig()->IsIgnoreSpacesRight()) )
    {
        pBlankNode->Clear();
    }
    return pBlankNode;
}

std::unique_ptr<SmNode> SmParser::DoTerm(bool bGroupNumberIdent)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    switch (m_aCurToken.eType)
    {
        case TESCAPE :
            return DoEscape();

        case TNOSPACE :
        case TLGROUP :
        {
            bool bNoSpace = m_aCurToken.eType == TNOSPACE;
            if (bNoSpace)
                NextToken();
            if (m_aCurToken.eType != TLGROUP)
                return DoTerm(false); // nospace is no longer concerned

            NextToken();

            // allow for empty group
            if (m_aCurToken.eType == TRGROUP)
            {
                std::unique_ptr<SmStructureNode> xSNode(new SmExpressionNode(m_aCurToken));
                xSNode->SetSubNodes(nullptr, nullptr);

                NextToken();
                return std::unique_ptr<SmNode>(xSNode.release());
            }

            auto pNode = DoAlign(!bNoSpace);
            if (m_aCurToken.eType == TRGROUP) {
                NextToken();
                return pNode;
            }
            auto xSNode = std::make_unique<SmExpressionNode>(m_aCurToken);
            std::unique_ptr<SmNode> xError(DoError(SmParseError::RgroupExpected));
            xSNode->SetSubNodes(std::move(pNode), std::move(xError));
            return std::unique_ptr<SmNode>(xSNode.release());
        }

        case TLEFT :
            return DoBrace();
                   case TEVALUATE:
            return DoEvaluate();

        case TBLANK :
        case TSBLANK :
            return DoBlank();

        case TTEXT :
            {
                auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_TEXT);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }
        case TCHARACTER :
            {
                auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_VARIABLE);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }
        case TIDENT :
        case TNUMBER :
        {
            auto pTextNode = std::make_unique<SmTextNode>(m_aCurToken,
                                             m_aCurToken.eType == TNUMBER ?
                                             FNT_NUMBER :
                                             FNT_VARIABLE);
            if (!bGroupNumberIdent)
            {
                NextToken();
                return std::unique_ptr<SmNode>(pTextNode.release());
            }
            std::vector<std::unique_ptr<SmNode>> aNodes;
            // Some people want to be able to write "x_2n" for "x_{2n}"
            // although e.g. LaTeX or AsciiMath interpret that as "x_2 n".
            // The tokenizer skips whitespaces so we need some additional
            // work to distinguish from "x_2 n".
            // See https://bz.apache.org/ooo/show_bug.cgi?id=11752 and
            // https://bugs.libreoffice.org/show_bug.cgi?id=55853
            sal_Int32 nBufLen = m_aBufferString.getLength();

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
                aNodes.emplace_back(std::unique_ptr<SmNode>(new SmTextNode(m_aCurToken,
                                                m_aCurToken.eType ==
                                                TNUMBER ?
                                                FNT_NUMBER :
                                                FNT_VARIABLE)));
            }
            if (moveToNextToken)
                NextToken();
            if (aNodes.empty())
                return std::unique_ptr<SmNode>(pTextNode.release());
            // We have several concatenated identifiers and numbers.
            // Let's group them into one SmExpressionNode.
            aNodes.insert(aNodes.begin(), std::move(pTextNode));
            std::unique_ptr<SmExpressionNode> xNode(new SmExpressionNode(SmToken()));
            xNode->SetSubNodes(buildNodeArray(aNodes));
            return std::unique_ptr<SmNode>(xNode.release());
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
        case TLAPLACE :
        case TFOURIER :
        case TTOWARD :
        case TDOTSAXIS :
        case TDOTSDIAG :
        case TDOTSDOWN :
        case TDOTSLOW :
        case TDOTSUP :
        case TDOTSVERT :
            {
                auto pNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }

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
            {
                auto pNode = std::make_unique<SmMathIdentifierNode>(m_aCurToken);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }

        case TPLACE:
            {
                auto pNode = std::make_unique<SmPlaceNode>(m_aCurToken);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }

        case TSPECIAL:
            return DoSpecial();

        case TBINOM:
            return DoBinom();

        case TFRAC:
            return DoFrac();

        case TSTACK:
            return DoStack();

        case TMATRIX:
            return DoMatrix();

        case THEX:
            NextTokenFontSize();
            if( m_aCurToken.eType == THEX )
            {
                auto pTextNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_NUMBER );
                NextToken();
                return pTextNode;
            }
            else return DoError(SmParseError::NumberExpected);
        default:
            if (TokenInGroup(TG::LBrace)) return DoBrace();
            if (TokenInGroup(TG::Oper)) return DoOperator();
            if (TokenInGroup(TG::UnOper)) return DoUnOper();
            if ( TokenInGroup(TG::Attribute) || TokenInGroup(TG::FontAttr) )
            {
                std::stack<std::unique_ptr<SmStructureNode>> aStack;
                bool    bIsAttr;
                for (;;)
                {
                    bIsAttr = TokenInGroup(TG::Attribute);
                    if (!bIsAttr && !TokenInGroup(TG::FontAttr))
                        break;
                    aStack.push(bIsAttr ? DoAttribute() : DoFontAttribute());
                }

                auto xFirstNode = DoPower();
                while (!aStack.empty())
                {
                    std::unique_ptr<SmStructureNode> xNode = std::move(aStack.top());
                    aStack.pop();
                    xNode->SetSubNodes(nullptr, std::move(xFirstNode));
                    xFirstNode = std::move(xNode);
                }
                return xFirstNode;
            }
            if (TokenInGroup(TG::Function))
                return DoFunction();
            return DoError(SmParseError::UnexpectedChar);
    }
}

std::unique_ptr<SmNode> SmParser::DoEscape()
{
    DepthProtect aDepthGuard(m_nParseDepth);

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
            {
                auto pNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
                NextToken();
                return std::unique_ptr<SmNode>(pNode.release());
            }
        default:
            return DoError(SmParseError::UnexpectedToken);
    }
}

std::unique_ptr<SmOperNode> SmParser::DoOperator()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Oper));

    auto xSNode = std::make_unique<SmOperNode>(m_aCurToken);

    // get operator
    auto xOperator = DoOper();

    if (m_aCurToken.nGroup == TG::Limit || m_aCurToken.nGroup == TG::Power)
        xOperator = DoSubSup(m_aCurToken.nGroup, std::move(xOperator));

    // get argument
    auto xArg = DoPower();

    xSNode->SetSubNodes(std::move(xOperator), std::move(xArg));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser::DoOper()
{
    DepthProtect aDepthGuard(m_nParseDepth);

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
                const char* pLim = nullptr;
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
            m_aCurToken.eType = TOPER;
            pNode.reset(new SmGlyphSpecialNode(m_aCurToken));
            break;

        default :
            assert(false && "unknown case");
    }

    NextToken();
    return pNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoUnOper()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::UnOper));

    SmToken      aNodeToken = m_aCurToken;
    SmTokenType  eType      = m_aCurToken.eType;
    bool         bIsPostfix = eType == TFACT;

    std::unique_ptr<SmStructureNode> xSNode;
    std::unique_ptr<SmNode> xOper;
    std::unique_ptr<SmNode> xExtra;
    std::unique_ptr<SmNode> xArg;

    switch (eType)
    {
        case TABS :
        case TSQRT :
            NextToken();
            break;

        case TNROOT :
            NextToken();
            xExtra = DoPower();
            break;

        case TUOPER :
            NextToken();
            //Let the glyph know what it is...
            m_aCurToken.eType = TUOPER;
            m_aCurToken.nGroup = TG::UnOper;
            xOper = DoGlyphSpecial();
            break;

        case TPLUS :
        case TMINUS :
        case TPLUSMINUS :
        case TMINUSPLUS :
        case TNEG :
        case TFACT :
            xOper = DoOpSubSup();
            break;

        default :
            assert(false);
    }

    // get argument
    xArg = DoPower();

    if (eType == TABS)
    {
        xSNode.reset(new SmBraceNode(aNodeToken));
        xSNode->SetScaleMode(SmScaleMode::Height);

        // build nodes for left & right lines
        // (text, group, level of the used token are of no interest here)
        // we'll use row & column of the keyword for abs
        aNodeToken.eType = TABS;

        aNodeToken.setChar(MS_VERTLINE);
        std::unique_ptr<SmNode> xLeft(new SmMathSymbolNode(aNodeToken));
        std::unique_ptr<SmNode> xRight(new SmMathSymbolNode(aNodeToken));

        xSNode->SetSubNodes(std::move(xLeft), std::move(xArg), std::move(xRight));
    }
    else if (eType == TSQRT  ||  eType == TNROOT)
    {
        xSNode.reset(new SmRootNode(aNodeToken));
        xOper.reset(new SmRootSymbolNode(aNodeToken));
        xSNode->SetSubNodes(std::move(xExtra), std::move(xOper), std::move(xArg));
    }
    else
    {
        xSNode.reset(new SmUnHorNode(aNodeToken));
        if (bIsPostfix)
            xSNode->SetSubNodes(std::move(xArg), std::move(xOper));
        else
        {
            // prefix operator
            xSNode->SetSubNodes(std::move(xOper), std::move(xArg));
        }
    }
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoAttribute()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Attribute));

    auto xSNode = std::make_unique<SmAttributeNode>(m_aCurToken);
    std::unique_ptr<SmNode> xAttr;
    SmScaleMode  eScaleMode = SmScaleMode::None;

    // get appropriate node for the attribute itself
    switch (m_aCurToken.eType)
    {   case TUNDERLINE :
        case TOVERLINE :
        case TOVERSTRIKE :
            xAttr.reset(new SmRectangleNode(m_aCurToken));
            eScaleMode = SmScaleMode::Width;
            break;

        case TWIDEVEC :
        case TWIDEHARPOON :
        case TWIDEHAT :
        case TWIDETILDE :
            xAttr.reset(new SmMathSymbolNode(m_aCurToken));
            eScaleMode = SmScaleMode::Width;
            break;

        default :
            xAttr.reset(new SmMathSymbolNode(m_aCurToken));
    }

    NextToken();

    xSNode->SetSubNodes(std::move(xAttr), nullptr); // the body will be filled later
    xSNode->SetScaleMode(eScaleMode);
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoFontAttribute()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::FontAttr));

    switch (m_aCurToken.eType)
    {
        case TITALIC :
        case TNITALIC :
        case TBOLD :
        case TNBOLD :
        case TPHANTOM :
            {
                auto pNode = std::make_unique<SmFontNode>(m_aCurToken);
                NextToken();
                return pNode;
            }

        case TSIZE :
            return DoFontSize();

        case TFONT :
            return DoFont();

        case TCOLOR :
            return DoColor();

        default :
            assert(false);
            return {};
    }
}

std::unique_ptr<SmStructureNode> SmParser::DoColor()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TCOLOR);
    NextTokenColor(TCOLOR);
    SmToken  aToken;

    if( m_aCurToken.eType == TDVIPSNAMESCOL ) NextTokenColor(TDVIPSNAMESCOL);
    if( m_aCurToken.eType == TERROR ) return DoError(SmParseError::ColorExpected);
    if (TokenInGroup(TG::Color))
    {
        aToken = m_aCurToken;
        if( m_aCurToken.eType == TRGB ) //loads r, g and b
        {
            sal_uInt32 nr, ng, nb, nc;
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            nr = m_aCurToken.aText.toUInt32();
            if( nr > 255 )return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            ng = m_aCurToken.aText.toUInt32();
            if( ng > 255 )return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            nb = m_aCurToken.aText.toUInt32();
            if( nb > 255 )return DoError(SmParseError::ColorExpected);
            nc = nb | ng << 8 | nr << 16 | sal_uInt32(0) << 24;
            aToken.aText = OUString::number(nc, 16);
        }
        else if( m_aCurToken.eType == TRGBA ) //loads r, g and b
        {
            sal_uInt32 nr, na, ng, nb, nc;
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            nr = m_aCurToken.aText.toUInt32();
            if( nr > 255 )return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            ng = m_aCurToken.aText.toUInt32();
            if( ng > 255 )return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            nb = m_aCurToken.aText.toUInt32();
            if( nb > 255 )return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            na = m_aCurToken.aText.toUInt32();
            if( na > 255 )return DoError(SmParseError::ColorExpected);
            nc = nb | ng << 8 | nr << 16 | na << 24;
            aToken.aText = OUString::number(nc, 16);
        }
        else if( m_aCurToken.eType == THEX ) //loads hex code
        {
            sal_uInt32 nc;
            NextTokenFontSize();
            if( lcl_IsNotWholeNumber16(m_aCurToken.aText) )
                return DoError(SmParseError::ColorExpected);
            nc = m_aCurToken.aText.toUInt32(16);
            aToken.aText = OUString::number(nc, 16);
        }
        NextToken();
    }
    else return DoError(SmParseError::ColorExpected);

    std::unique_ptr<SmStructureNode> xNode;
    xNode.reset(new SmFontNode(aToken));
    return xNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoFont()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TFONT);

    std::unique_ptr<SmStructureNode> xNode;
    // last font rules, get that one
    SmToken  aToken;
    do
    {   NextToken();

        if (TokenInGroup(TG::Font))
        {   aToken = m_aCurToken;
            NextToken();
        }
        else
        {
            return DoError(SmParseError::FontExpected);
        }
    } while (m_aCurToken.eType == TFONT);

    xNode.reset(new SmFontNode(aToken));
    return xNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoFontSize()
{
    DepthProtect aDepthGuard(m_nParseDepth);
    std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(m_aCurToken));
    NextTokenFontSize();
    FontSizeType Type;

    switch (m_aCurToken.eType)
    {
        case THEX:      Type = FontSizeType::ABSOLUT;  break;
        case TPLUS:     Type = FontSizeType::PLUS;     break;
        case TMINUS:    Type = FontSizeType::MINUS;    break;
        case TMULTIPLY: Type = FontSizeType::MULTIPLY; break;
        case TDIVIDEBY: Type = FontSizeType::DIVIDE;   break;

        default:
            return DoError(SmParseError::SizeExpected);
    }

    if (Type != FontSizeType::ABSOLUT)
    {
        NextTokenFontSize();
        if (m_aCurToken.eType != THEX) return DoError(SmParseError::SizeExpected);
    }

    // get number argument
    Fraction  aValue( 1 );
    if (lcl_IsNumber( m_aCurToken.aText ))
    {
        aValue = m_aCurToken.aText.toDouble();
        //!! Reduce values in order to avoid numerical errors
        if (aValue.GetDenominator() > 1000)
        {
            tools::Long nNum   = aValue.GetNumerator();
            tools::Long nDenom = aValue.GetDenominator();
            while ( nDenom > 1000 ) //remove big denominator
            {
                nNum    /= 10;
                nDenom  /= 10;
            }
            aValue = Fraction( nNum, nDenom );
        }
    }
    else return DoError(SmParseError::SizeExpected);

    pFontNode->SetSizeParameter(aValue, Type);
    NextToken();
    return pFontNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoBrace()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TLEFT  ||  TokenInGroup(TG::LBrace));

    std::unique_ptr<SmStructureNode> xSNode(new SmBraceNode(m_aCurToken));
    std::unique_ptr<SmNode> pBody, pLeft, pRight;
    SmScaleMode   eScaleMode = SmScaleMode::None;
    SmParseError  eError     = SmParseError::None;

    if (m_aCurToken.eType == TLEFT)
    {   NextToken();

        eScaleMode = SmScaleMode::Height;

        // check for left bracket
        if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
        {
            pLeft.reset(new SmMathSymbolNode(m_aCurToken));

            NextToken();
            pBody = DoBracebody(true);

            if (m_aCurToken.eType == TRIGHT)
            {   NextToken();

                // check for right bracket
                if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
                {
                    pRight.reset(new SmMathSymbolNode(m_aCurToken));
                    NextToken();
                }
                else
                    eError = SmParseError::RbraceExpected;
            }
            else
                eError = SmParseError::RightExpected;
        }
        else
            eError = SmParseError::LbraceExpected;
    }
    else
    {
        assert(TokenInGroup(TG::LBrace));

        pLeft.reset(new SmMathSymbolNode(m_aCurToken));

        NextToken();
        pBody = DoBracebody(false);

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
            case TLRLINE :      eExpectedType = TLRLINE;    break;
            case TLRDLINE :     eExpectedType = TLRDLINE;   break;
            default :
                SAL_WARN("starmath", "unknown case");
            }

        if (m_aCurToken.eType == eExpectedType)
        {
            pRight.reset(new SmMathSymbolNode(m_aCurToken));
            NextToken();
        }
        else
            eError = SmParseError::ParentMismatch;
    }

    if (eError == SmParseError::None)
    {
        assert(pLeft);
        assert(pRight);
        xSNode->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
        xSNode->SetScaleMode(eScaleMode);
        return xSNode;
    }
    return DoError(eError);
}

std::unique_ptr<SmBracebodyNode> SmParser::DoBracebody(bool bIsLeftRight)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto pBody = std::make_unique<SmBracebodyNode>(m_aCurToken);

    std::vector<std::unique_ptr<SmNode>> aNodes;
    // get body if any
    if (bIsLeftRight)
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                aNodes.emplace_back(std::make_unique<SmMathSymbolNode>(m_aCurToken));
                NextToken();
            }
            else if (m_aCurToken.eType != TRIGHT)
            {
                aNodes.push_back(DoAlign());
                if (m_aCurToken.eType != TMLINE  &&  m_aCurToken.eType != TRIGHT)
                    aNodes.emplace_back(DoError(SmParseError::RightExpected));
            }
        } while (m_aCurToken.eType != TEND  &&  m_aCurToken.eType != TRIGHT);
    }
    else
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                aNodes.emplace_back(std::make_unique<SmMathSymbolNode>(m_aCurToken));
                NextToken();
            }
            else if (!TokenInGroup(TG::RBrace))
            {
                aNodes.push_back(DoAlign());
                if (m_aCurToken.eType != TMLINE  &&  !TokenInGroup(TG::RBrace))
                    aNodes.emplace_back(DoError(SmParseError::RbraceExpected));
            }
        } while (m_aCurToken.eType != TEND  &&  !TokenInGroup(TG::RBrace));
    }

    pBody->SetSubNodes(buildNodeArray(aNodes));
    pBody->SetScaleMode(bIsLeftRight ? SmScaleMode::Height : SmScaleMode::None);
    return pBody;
}

std::unique_ptr<SmNode> SmParser::DoEvaluate()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // Create node
    std::unique_ptr<SmStructureNode> xSNode(new SmBraceNode(m_aCurToken));
    SmToken aToken( TRLINE, MS_VERTLINE, "evaluate", TG::RBrace, 5);
    aToken.nRow = m_aCurToken.nRow;
    aToken.nCol = m_aCurToken.nCol;

    // Parse body && left none
    NextToken();
    std::unique_ptr<SmNode> pBody = DoPower();
    SmToken bToken( TNONE, '\0', "", TG::LBrace, 5);
    std::unique_ptr<SmNode> pLeft;
    pLeft.reset(new SmMathSymbolNode(bToken));

    // Mount nodes
    std::unique_ptr<SmNode> pRight;
    pRight.reset(new SmMathSymbolNode(aToken));
    xSNode->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
    xSNode->SetScaleMode(SmScaleMode::Height); // scalable line

    // Parse from to
    if ( m_aCurToken.nGroup == TG::Limit )
    {
        std::unique_ptr<SmNode> rSNode;
        rSNode = DoSubSupEvaluate(std::move(xSNode));
        rSNode->GetToken().eType = TEVALUATE;
        return rSNode;
    }

    return xSNode;

}

std::unique_ptr<SmTextNode> SmParser::DoFunction()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    if( m_aCurToken.eType == TFUNC )
    {
        NextToken();    // skip "FUNC"-statement
        m_aCurToken.eType = TFUNC;
        m_aCurToken.nGroup = TG::Function;
    }
    auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_FUNCTION);
    NextToken();
    return pNode;
}

std::unique_ptr<SmTableNode> SmParser::DoBinom()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto xSNode = std::make_unique<SmTableNode>(m_aCurToken);

    NextToken();

    auto xFirst = DoSum();
    auto xSecond = DoSum();
    xSNode->SetSubNodes(std::move(xFirst), std::move(xSecond));
    return xSNode;
}

std::unique_ptr<SmBinVerNode> SmParser::DoFrac()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmBinVerNode> xSNode = std::make_unique<SmBinVerNode>(m_aCurToken);
    std::unique_ptr<SmNode> xOper = std::make_unique<SmRectangleNode>(m_aCurToken);

    NextToken();

    auto xFirst = DoSum();
    auto xSecond = DoSum();
    xSNode->SetSubNodes(std::move(xFirst), std::move(xOper), std::move(xSecond));
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoStack()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmStructureNode> xSNode(new SmTableNode(m_aCurToken));
    NextToken();
    if (m_aCurToken.eType != TLGROUP)
        return DoError(SmParseError::LgroupExpected);
    std::vector<std::unique_ptr<SmNode>> aExprArr;
    do
    {
        NextToken();
        aExprArr.push_back(DoAlign());
    }
    while (m_aCurToken.eType == TPOUND);

    if (m_aCurToken.eType == TRGROUP)
        NextToken();
    else
        aExprArr.emplace_back(DoError(SmParseError::RgroupExpected));

    xSNode->SetSubNodes(buildNodeArray(aExprArr));
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser::DoMatrix()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmMatrixNode> xMNode(new SmMatrixNode(m_aCurToken));
    NextToken();
    if (m_aCurToken.eType != TLGROUP)
        return DoError(SmParseError::LgroupExpected);

    std::vector<std::unique_ptr<SmNode>> aExprArr;
    do
    {
        NextToken();
        aExprArr.push_back(DoAlign());
    }
    while (m_aCurToken.eType == TPOUND);

    size_t nCol = aExprArr.size();
    size_t nRow = 1;
    while (m_aCurToken.eType == TDPOUND)
    {
        NextToken();
        for (size_t i = 0; i < nCol; i++)
        {
            auto xNode = DoAlign();
            if (i < (nCol - 1))
            {
                if (m_aCurToken.eType == TPOUND)
                    NextToken();
                else
                    xNode = DoError(SmParseError::PoundExpected);
            }
            aExprArr.emplace_back(std::move(xNode));
        }
        ++nRow;
    }

    if (m_aCurToken.eType == TRGROUP)
        NextToken();
    else
    {
        std::unique_ptr<SmNode> xENode(DoError(SmParseError::RgroupExpected));
        if (aExprArr.empty())
            nRow = nCol = 1;
        else
            aExprArr.pop_back();
        aExprArr.emplace_back(std::move(xENode));
    }

    xMNode->SetSubNodes(buildNodeArray(aExprArr));
    xMNode->SetRowCol(static_cast<sal_uInt16>(nRow),
                      static_cast<sal_uInt16>(nCol));
    return std::unique_ptr<SmStructureNode>(xMNode.release());
}

std::unique_ptr<SmSpecialNode> SmParser::DoSpecial()
{
    DepthProtect aDepthGuard(m_nParseDepth);

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
            aNewName = SmLocalizedSymbolData::GetExportSymbolName(rName.subView(1));
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

    auto pNode = std::make_unique<SmSpecialNode>(m_aCurToken);
    NextToken();
    return pNode;
}

std::unique_ptr<SmGlyphSpecialNode> SmParser::DoGlyphSpecial()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto pNode = std::make_unique<SmGlyphSpecialNode>(m_aCurToken);
    NextToken();
    return pNode;
}

std::unique_ptr<SmExpressionNode> SmParser::DoError(SmParseError eError)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // Identify error message
    OUStringBuffer sStrBuf(128);
    sStrBuf.append(SmResId(RID_ERR_IDENT));
    sStrBuf.append(starmathdatabase::getParseErrorDesc(eError));

    // Generate error node
    m_aCurToken.eType = TERROR;
    m_aCurToken.cMathChar = sStrBuf.makeStringAndClear();
    auto xSNode = std::make_unique<SmExpressionNode>(m_aCurToken);
    SmErrorNode* pErr(new SmErrorNode(m_aCurToken));
    xSNode->SetSubNode(0, pErr);

    // Append error to the error list
    SmErrorDesc* pErrDesc = new SmErrorDesc(eError, xSNode.get(), m_aCurToken.cMathChar);
    m_aErrDescList.push_back(std::unique_ptr<SmErrorDesc>(pErrDesc));

    NextToken();

    return xSNode;
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
    , m_nParseDepth(0)
    , m_aNumCC( LanguageTag( LANGUAGE_ENGLISH_US ) )
    , m_pSysCC( SM_MOD()->GetSysLocale().GetCharClassPtr() )
{
}

std::unique_ptr<SmTableNode> SmParser::Parse(const OUString &rBuffer)
{
    m_aUsedSymbols.clear();

    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_nRow          = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    m_aErrDescList.clear();

    NextToken();
    return DoTable();
}

std::unique_ptr<SmNode> SmParser::ParseExpression(const OUString &rBuffer)
{
    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex  = 0;
    m_nTokenIndex   = 0;
    m_nRow          = 1;
    m_nColOff       = 0;
    m_nCurError     = -1;

    m_aErrDescList.clear();

    NextToken();
    return DoExpression();
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


const SmErrorDesc *SmParser::GetError()
{
    if ( !m_aErrDescList.empty() )
        return m_aErrDescList.front().get();
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
