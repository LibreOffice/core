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
#include <com/sun/star/i18n/KParseTokens.hpp>
#include <com/sun/star/i18n/KParseType.hpp>
#include <i18nlangtag/lang.h>
#include <tools/lineend.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/syslocale.hxx>
#include <osl/diagnose.h>
#include <rtl/character.hxx>
#include <parse5.hxx>
#include <strings.hrc>
#include <smmod.hxx>
#include <symbol.hxx>
#include <cfgitem.hxx>
#include <starmathdatabase.hxx>

#include <stack>

using namespace ::com::sun::star::i18n;

//Definition of math keywords
const SmTokenTableEntry aTokenTable[]
    = { { u"abs"_ustr, TABS, '\0', TG::UnOper, 13 },
        { u"acute"_ustr, TACUTE, MS_ACUTE, TG::Attribute, 5 },
        { u"aleph"_ustr, TALEPH, MS_ALEPH, TG::Standalone, 5 },
        { u"alignb"_ustr, TALIGNC, '\0', TG::Align, 0 },
        { u"alignc"_ustr, TALIGNC, '\0', TG::Align, 0 },
        { u"alignl"_ustr, TALIGNL, '\0', TG::Align, 0 },
        { u"alignm"_ustr, TALIGNC, '\0', TG::Align, 0 },
        { u"alignr"_ustr, TALIGNR, '\0', TG::Align, 0 },
        { u"alignt"_ustr, TALIGNC, '\0', TG::Align, 0 },
        { u"and"_ustr, TAND, MS_AND, TG::Product, 0 },
        { u"approx"_ustr, TAPPROX, MS_APPROX, TG::Relation, 0 },
        { u"arccos"_ustr, TACOS, '\0', TG::Function, 5 },
        { u"arccot"_ustr, TACOT, '\0', TG::Function, 5 },
        { u"arcosh"_ustr, TACOSH, '\0', TG::Function, 5 },
        { u"arcoth"_ustr, TACOTH, '\0', TG::Function, 5 },
        { u"arcsin"_ustr, TASIN, '\0', TG::Function, 5 },
        { u"arctan"_ustr, TATAN, '\0', TG::Function, 5 },
        { u"arsinh"_ustr, TASINH, '\0', TG::Function, 5 },
        { u"artanh"_ustr, TATANH, '\0', TG::Function, 5 },
        { u"backepsilon"_ustr, TBACKEPSILON, MS_BACKEPSILON, TG::Standalone, 5 },
        { u"bar"_ustr, TBAR, MS_BAR, TG::Attribute, 5 },
        { u"binom"_ustr, TBINOM, '\0', TG::NONE, 5 },
        { u"bold"_ustr, TBOLD, '\0', TG::FontAttr, 5 },
        { u"boper"_ustr, TBOPER, '\0', TG::Product, 0 },
        { u"breve"_ustr, TBREVE, MS_BREVE, TG::Attribute, 5 },
        { u"bslash"_ustr, TBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
        { u"cdot"_ustr, TCDOT, MS_CDOT, TG::Product, 0 },
        { u"check"_ustr, TCHECK, MS_CHECK, TG::Attribute, 5 },
        { u"circ"_ustr, TCIRC, MS_CIRC, TG::Standalone, 5 },
        { u"circle"_ustr, TCIRCLE, MS_CIRCLE, TG::Attribute, 5 },
        { u"color"_ustr, TCOLOR, '\0', TG::FontAttr, 5 },
        { u"coprod"_ustr, TCOPROD, MS_COPROD, TG::Oper, 5 },
        { u"cos"_ustr, TCOS, '\0', TG::Function, 5 },
        { u"cosh"_ustr, TCOSH, '\0', TG::Function, 5 },
        { u"cot"_ustr, TCOT, '\0', TG::Function, 5 },
        { u"coth"_ustr, TCOTH, '\0', TG::Function, 5 },
        { u"csub"_ustr, TCSUB, '\0', TG::Power, 0 },
        { u"csup"_ustr, TCSUP, '\0', TG::Power, 0 },
        { u"dddot"_ustr, TDDDOT, MS_DDDOT, TG::Attribute, 5 },
        { u"ddot"_ustr, TDDOT, MS_DDOT, TG::Attribute, 5 },
        { u"def"_ustr, TDEF, MS_DEF, TG::Relation, 0 },
        { u"div"_ustr, TDIV, MS_DIV, TG::Product, 0 },
        { u"divides"_ustr, TDIVIDES, MS_LINE, TG::Relation, 0 },
        { u"dlarrow"_ustr, TDLARROW, MS_DLARROW, TG::Standalone, 5 },
        { u"dlrarrow"_ustr, TDLRARROW, MS_DLRARROW, TG::Standalone, 5 },
        { u"dot"_ustr, TDOT, MS_DOT, TG::Attribute, 5 },
        { u"dotsaxis"_ustr, TDOTSAXIS, MS_DOTSAXIS, TG::Standalone, 5 }, // 5 to continue expression
        { u"dotsdiag"_ustr, TDOTSDIAG, MS_DOTSUP, TG::Standalone, 5 },
        { u"dotsdown"_ustr, TDOTSDOWN, MS_DOTSDOWN, TG::Standalone, 5 },
        { u"dotslow"_ustr, TDOTSLOW, MS_DOTSLOW, TG::Standalone, 5 },
        { u"dotsup"_ustr, TDOTSUP, MS_DOTSUP, TG::Standalone, 5 },
        { u"dotsvert"_ustr, TDOTSVERT, MS_DOTSVERT, TG::Standalone, 5 },
        { u"downarrow"_ustr, TDOWNARROW, MS_DOWNARROW, TG::Standalone, 5 },
        { u"drarrow"_ustr, TDRARROW, MS_DRARROW, TG::Standalone, 5 },
        { u"emptyset"_ustr, TEMPTYSET, MS_EMPTYSET, TG::Standalone, 5 },
        { u"equiv"_ustr, TEQUIV, MS_EQUIV, TG::Relation, 0 },
        { u"evaluate"_ustr, TEVALUATE, '\0', TG::NONE, 0 },
        { u"exists"_ustr, TEXISTS, MS_EXISTS, TG::Standalone, 5 },
        { u"exp"_ustr, TEXP, '\0', TG::Function, 5 },
        { u"fact"_ustr, TFACT, MS_FACT, TG::UnOper, 5 },
        { u"fixed"_ustr, TFIXED, '\0', TG::Font, 0 },
        { u"font"_ustr, TFONT, '\0', TG::FontAttr, 5 },
        { u"forall"_ustr, TFORALL, MS_FORALL, TG::Standalone, 5 },
        { u"fourier"_ustr, TFOURIER, MS_FOURIER, TG::Standalone, 5 },
        { u"frac"_ustr, TFRAC, '\0', TG::NONE, 5 },
        { u"from"_ustr, TFROM, '\0', TG::Limit, 0 },
        { u"func"_ustr, TFUNC, '\0', TG::Function, 5 },
        { u"ge"_ustr, TGE, MS_GE, TG::Relation, 0 },
        { u"geslant"_ustr, TGESLANT, MS_GESLANT, TG::Relation, 0 },
        { u"gg"_ustr, TGG, MS_GG, TG::Relation, 0 },
        { u"grave"_ustr, TGRAVE, MS_GRAVE, TG::Attribute, 5 },
        { u"gt"_ustr, TGT, MS_GT, TG::Relation, 0 },
        { u"hadd"_ustr, THADD, MS_HADD, TG::Oper, 5 },
        { u"harpoon"_ustr, THARPOON, MS_HARPOON, TG::Attribute, 5 },
        { u"hat"_ustr, THAT, MS_HAT, TG::Attribute, 5 },
        { u"hbar"_ustr, THBAR, MS_HBAR, TG::Standalone, 5 },
        { u"hex"_ustr, THEX, '\0', TG::NONE, 5 },
        { u"iiint"_ustr, TIIINT, MS_IIINT, TG::Oper, 5 },
        { u"iint"_ustr, TIINT, MS_IINT, TG::Oper, 5 },
        { u"im"_ustr, TIM, MS_IM, TG::Standalone, 5 },
        { u"in"_ustr, TIN, MS_IN, TG::Relation, 0 },
        { u"infinity"_ustr, TINFINITY, MS_INFINITY, TG::Standalone, 5 },
        { u"infty"_ustr, TINFINITY, MS_INFINITY, TG::Standalone, 5 },
        { u"int"_ustr, TINT, MS_INT, TG::Oper, 5 },
        { u"intd"_ustr, TINTD, MS_INT, TG::Oper, 5 },
        { u"intersection"_ustr, TINTERSECT, MS_INTERSECT, TG::Product, 0 },
        { u"it"_ustr, TIT, '\0', TG::Product, 0 },
        { u"ital"_ustr, TITALIC, '\0', TG::FontAttr, 5 },
        { u"italic"_ustr, TITALIC, '\0', TG::FontAttr, 5 },
        { u"lambdabar"_ustr, TLAMBDABAR, MS_LAMBDABAR, TG::Standalone, 5 },
        { u"langle"_ustr, TLANGLE, MS_LMATHANGLE, TG::LBrace, 5 },
        { u"laplace"_ustr, TLAPLACE, MS_LAPLACE, TG::Standalone, 5 },
        { u"lbrace"_ustr, TLBRACE, MS_LBRACE, TG::LBrace, 5 },
        { u"lceil"_ustr, TLCEIL, MS_LCEIL, TG::LBrace, 5 },
        { u"ldbracket"_ustr, TLDBRACKET, MS_LDBRACKET, TG::LBrace, 5 },
        { u"ldline"_ustr, TLDLINE, MS_DVERTLINE, TG::LBrace, 5 },
        { u"le"_ustr, TLE, MS_LE, TG::Relation, 0 },
        { u"left"_ustr, TLEFT, '\0', TG::NONE, 5 },
        { u"leftarrow"_ustr, TLEFTARROW, MS_LEFTARROW, TG::Standalone, 5 },
        { u"leslant"_ustr, TLESLANT, MS_LESLANT, TG::Relation, 0 },
        { u"lfloor"_ustr, TLFLOOR, MS_LFLOOR, TG::LBrace, 5 },
        { u"lim"_ustr, TLIM, '\0', TG::Oper, 5 },
        { u"liminf"_ustr, TLIMINF, '\0', TG::Oper, 5 },
        { u"limsup"_ustr, TLIMSUP, '\0', TG::Oper, 5 },
        { u"lint"_ustr, TLINT, MS_LINT, TG::Oper, 5 },
        { u"ll"_ustr, TLL, MS_LL, TG::Relation, 0 },
        { u"lline"_ustr, TLLINE, MS_VERTLINE, TG::LBrace, 5 },
        { u"llint"_ustr, TLLINT, MS_LLINT, TG::Oper, 5 },
        { u"lllint"_ustr, TLLLINT, MS_LLLINT, TG::Oper, 5 },
        { u"ln"_ustr, TLN, '\0', TG::Function, 5 },
        { u"log"_ustr, TLOG, '\0', TG::Function, 5 },
        { u"lrline"_ustr, TLRLINE, MS_VERTLINE, TG::LBrace | TG::RBrace, 5 },
        { u"lrdline"_ustr, TLRDLINE, MS_VERTLINE, TG::LBrace | TG::RBrace, 5 },
        { u"lsub"_ustr, TLSUB, '\0', TG::Power, 0 },
        { u"lsup"_ustr, TLSUP, '\0', TG::Power, 0 },
        { u"lt"_ustr, TLT, MS_LT, TG::Relation, 0 },
        { u"maj"_ustr, TSUM, MS_MAJ, TG::Oper, 5 },
        { u"matrix"_ustr, TMATRIX, '\0', TG::NONE, 5 },
        { u"minusplus"_ustr, TMINUSPLUS, MS_MINUSPLUS, TG::UnOper | TG::Sum, 5 },
        { u"mline"_ustr, TMLINE, MS_VERTLINE, TG::NONE, 0 }, //! not in TG::RBrace, Level 0
        { u"nabla"_ustr, TNABLA, MS_NABLA, TG::Standalone, 5 },
        { u"nbold"_ustr, TNBOLD, '\0', TG::FontAttr, 5 },
        { u"ndivides"_ustr, TNDIVIDES, MS_NDIVIDES, TG::Relation, 0 },
        { u"neg"_ustr, TNEG, MS_NEG, TG::UnOper, 5 },
        { u"neq"_ustr, TNEQ, MS_NEQ, TG::Relation, 0 },
        { u"newline"_ustr, TNEWLINE, '\0', TG::NONE, 0 },
        { u"ni"_ustr, TNI, MS_NI, TG::Relation, 0 },
        { u"nitalic"_ustr, TNITALIC, '\0', TG::FontAttr, 5 },
        { u"none"_ustr, TNONE, '\0', TG::LBrace | TG::RBrace, 0 },
        { u"nospace"_ustr, TNOSPACE, '\0', TG::Standalone, 5 },
        { u"notexists"_ustr, TNOTEXISTS, MS_NOTEXISTS, TG::Standalone, 5 },
        { u"notin"_ustr, TNOTIN, MS_NOTIN, TG::Relation, 0 },
        { u"nprec"_ustr, TNOTPRECEDES, MS_NOTPRECEDES, TG::Relation, 0 },
        { u"nroot"_ustr, TNROOT, MS_SQRT, TG::UnOper, 5 },
        { u"nsubset"_ustr, TNSUBSET, MS_NSUBSET, TG::Relation, 0 },
        { u"nsubseteq"_ustr, TNSUBSETEQ, MS_NSUBSETEQ, TG::Relation, 0 },
        { u"nsucc"_ustr, TNOTSUCCEEDS, MS_NOTSUCCEEDS, TG::Relation, 0 },
        { u"nsupset"_ustr, TNSUPSET, MS_NSUPSET, TG::Relation, 0 },
        { u"nsupseteq"_ustr, TNSUPSETEQ, MS_NSUPSETEQ, TG::Relation, 0 },
        { u"odivide"_ustr, TODIVIDE, MS_ODIVIDE, TG::Product, 0 },
        { u"odot"_ustr, TODOT, MS_ODOT, TG::Product, 0 },
        { u"ominus"_ustr, TOMINUS, MS_OMINUS, TG::Sum, 0 },
        { u"oper"_ustr, TOPER, '\0', TG::Oper, 5 },
        { u"oplus"_ustr, TOPLUS, MS_OPLUS, TG::Sum, 0 },
        { u"or"_ustr, TOR, MS_OR, TG::Sum, 0 },
        { u"ortho"_ustr, TORTHO, MS_ORTHO, TG::Relation, 0 },
        { u"otimes"_ustr, TOTIMES, MS_OTIMES, TG::Product, 0 },
        { u"over"_ustr, TOVER, '\0', TG::Product, 0 },
        { u"overbrace"_ustr, TOVERBRACE, MS_OVERBRACE, TG::Product, 5 },
        { u"overline"_ustr, TOVERLINE, '\0', TG::Attribute, 5 },
        { u"overstrike"_ustr, TOVERSTRIKE, '\0', TG::Attribute, 5 },
        { u"owns"_ustr, TNI, MS_NI, TG::Relation, 0 },
        { u"parallel"_ustr, TPARALLEL, MS_DLINE, TG::Relation, 0 },
        { u"partial"_ustr, TPARTIAL, MS_PARTIAL, TG::Standalone, 5 },
        { u"phantom"_ustr, TPHANTOM, '\0', TG::FontAttr, 5 },
        { u"plusminus"_ustr, TPLUSMINUS, MS_PLUSMINUS, TG::UnOper | TG::Sum, 5 },
        { u"prec"_ustr, TPRECEDES, MS_PRECEDES, TG::Relation, 0 },
        { u"preccurlyeq"_ustr, TPRECEDESEQUAL, MS_PRECEDESEQUAL, TG::Relation, 0 },
        { u"precsim"_ustr, TPRECEDESEQUIV, MS_PRECEDESEQUIV, TG::Relation, 0 },
        { u"prod"_ustr, TPROD, MS_PROD, TG::Oper, 5 },
        { u"prop"_ustr, TPROP, MS_PROP, TG::Relation, 0 },
        { u"rangle"_ustr, TRANGLE, MS_RMATHANGLE, TG::RBrace, 0 }, //! 0 to terminate expression
        { u"rbrace"_ustr, TRBRACE, MS_RBRACE, TG::RBrace, 0 },
        { u"rceil"_ustr, TRCEIL, MS_RCEIL, TG::RBrace, 0 },
        { u"rdbracket"_ustr, TRDBRACKET, MS_RDBRACKET, TG::RBrace, 0 },
        { u"rdline"_ustr, TRDLINE, MS_DVERTLINE, TG::RBrace, 0 },
        { u"re"_ustr, TRE, MS_RE, TG::Standalone, 5 },
        { u"rfloor"_ustr, TRFLOOR, MS_RFLOOR, TG::RBrace, 0 }, //! 0 to terminate expression
        { u"right"_ustr, TRIGHT, '\0', TG::NONE, 0 },
        { u"rightarrow"_ustr, TRIGHTARROW, MS_RIGHTARROW, TG::Standalone, 5 },
        { u"rline"_ustr, TRLINE, MS_VERTLINE, TG::RBrace, 0 }, //! 0 to terminate expression
        { u"rsub"_ustr, TRSUB, '\0', TG::Power, 0 },
        { u"rsup"_ustr, TRSUP, '\0', TG::Power, 0 },
        { u"sans"_ustr, TSANS, '\0', TG::Font, 0 },
        { u"serif"_ustr, TSERIF, '\0', TG::Font, 0 },
        { u"setC"_ustr, TSETC, MS_SETC, TG::Standalone, 5 },
        { u"setminus"_ustr, TSETMINUS, MS_BACKSLASH, TG::Product, 0 },
        { u"setN"_ustr, TSETN, MS_SETN, TG::Standalone, 5 },
        { u"setQ"_ustr, TSETQ, MS_SETQ, TG::Standalone, 5 },
        { u"setquotient"_ustr, TSETQUOTIENT, MS_SLASH, TG::Product, 0 },
        { u"setR"_ustr, TSETR, MS_SETR, TG::Standalone, 5 },
        { u"setZ"_ustr, TSETZ, MS_SETZ, TG::Standalone, 5 },
        { u"sim"_ustr, TSIM, MS_SIM, TG::Relation, 0 },
        { u"simeq"_ustr, TSIMEQ, MS_SIMEQ, TG::Relation, 0 },
        { u"sin"_ustr, TSIN, '\0', TG::Function, 5 },
        { u"sinh"_ustr, TSINH, '\0', TG::Function, 5 },
        { u"size"_ustr, TSIZE, '\0', TG::FontAttr, 5 },
        { u"slash"_ustr, TSLASH, MS_SLASH, TG::Product, 0 },
        { u"sqrt"_ustr, TSQRT, MS_SQRT, TG::UnOper, 5 },
        { u"stack"_ustr, TSTACK, '\0', TG::NONE, 5 },
        { u"sub"_ustr, TRSUB, '\0', TG::Power, 0 },
        { u"subset"_ustr, TSUBSET, MS_SUBSET, TG::Relation, 0 },
        { u"subseteq"_ustr, TSUBSETEQ, MS_SUBSETEQ, TG::Relation, 0 },
        { u"succ"_ustr, TSUCCEEDS, MS_SUCCEEDS, TG::Relation, 0 },
        { u"succcurlyeq"_ustr, TSUCCEEDSEQUAL, MS_SUCCEEDSEQUAL, TG::Relation, 0 },
        { u"succsim"_ustr, TSUCCEEDSEQUIV, MS_SUCCEEDSEQUIV, TG::Relation, 0 },
        { u"sum"_ustr, TSUM, MS_SUM, TG::Oper, 5 },
        { u"sup"_ustr, TRSUP, '\0', TG::Power, 0 },
        { u"supset"_ustr, TSUPSET, MS_SUPSET, TG::Relation, 0 },
        { u"supseteq"_ustr, TSUPSETEQ, MS_SUPSETEQ, TG::Relation, 0 },
        { u"tan"_ustr, TTAN, '\0', TG::Function, 5 },
        { u"tanh"_ustr, TTANH, '\0', TG::Function, 5 },
        { u"tilde"_ustr, TTILDE, MS_TILDE, TG::Attribute, 5 },
        { u"times"_ustr, TTIMES, MS_TIMES, TG::Product, 0 },
        { u"to"_ustr, TTO, '\0', TG::Limit, 0 },
        { u"toward"_ustr, TTOWARD, MS_RIGHTARROW, TG::Relation, 0 },
        { u"transl"_ustr, TTRANSL, MS_TRANSL, TG::Relation, 0 },
        { u"transr"_ustr, TTRANSR, MS_TRANSR, TG::Relation, 0 },
        { u"underbrace"_ustr, TUNDERBRACE, MS_UNDERBRACE, TG::Product, 5 },
        { u"underline"_ustr, TUNDERLINE, '\0', TG::Attribute, 5 },
        { u"union"_ustr, TUNION, MS_UNION, TG::Sum, 0 },
        { u"uoper"_ustr, TUOPER, '\0', TG::UnOper, 5 },
        { u"uparrow"_ustr, TUPARROW, MS_UPARROW, TG::Standalone, 5 },
        { u"vec"_ustr, TVEC, MS_VEC, TG::Attribute, 5 },
        { u"widebslash"_ustr, TWIDEBACKSLASH, MS_BACKSLASH, TG::Product, 0 },
        { u"wideharpoon"_ustr, TWIDEHARPOON, MS_HARPOON, TG::Attribute, 5 },
        { u"widehat"_ustr, TWIDEHAT, MS_HAT, TG::Attribute, 5 },
        { u"wideslash"_ustr, TWIDESLASH, MS_SLASH, TG::Product, 0 },
        { u"widetilde"_ustr, TWIDETILDE, MS_TILDE, TG::Attribute, 5 },
        { u"widevec"_ustr, TWIDEVEC, MS_VEC, TG::Attribute, 5 },
        { u"wp"_ustr, TWP, MS_WP, TG::Standalone, 5 },
        { u"جا"_ustr, TSIN, '\0', TG::Function, 5 },
        { u"جاز"_ustr, TSINH, '\0', TG::Function, 5 },
        { u"جتا"_ustr, TCOS, '\0', TG::Function, 5 },
        { u"جتاز"_ustr, TCOSH, '\0', TG::Function, 5 },
        { u"حا"_ustr, TSIN, '\0', TG::Function, 5 },
        { u"حاز"_ustr, TSINH, '\0', TG::Function, 5 },
        { u"حتا"_ustr, TCOS, '\0', TG::Function, 5 },
        { u"حتاز"_ustr, TCOSH, '\0', TG::Function, 5 },
        { u"حد"_ustr, THADD, MS_HADD, TG::Oper, 5 },
        { u"طا"_ustr, TTAN, '\0', TG::Function, 5 },
        { u"طاز"_ustr, TTANH, '\0', TG::Function, 5 },
        { u"طتا"_ustr, TCOT, '\0', TG::Function, 5 },
        { u"طتاز"_ustr, TCOTH, '\0', TG::Function, 5 },
        { u"ظا"_ustr, TTAN, '\0', TG::Function, 5 },
        { u"ظاز"_ustr, TTANH, '\0', TG::Function, 5 },
        { u"ظتا"_ustr, TCOT, '\0', TG::Function, 5 },
        { u"ظتاز"_ustr, TCOTH, '\0', TG::Function, 5 },
        { u"قا"_ustr, TSEC, '\0', TG::Function, 5 },
        { u"قاز"_ustr, TSECH, '\0', TG::Function, 5 },
        { u"قتا"_ustr, TCSC, '\0', TG::Function, 5 },
        { u"قتاز"_ustr, TCSCH, '\0', TG::Function, 5 },
        { u"لو"_ustr, TLOG, '\0', TG::Function, 5 },
        { u"مجـ"_ustr, TSUM, MS_MAJ, TG::Oper, 5 },
        { u"نها"_ustr, TNAHA, '\0', TG::Oper, 5 },
        { u"ٯا"_ustr, TSEC, '\0', TG::Function, 5 },
        { u"ٯاز"_ustr, TSECH, '\0', TG::Function, 5 },
        { u"ٯتا"_ustr, TCSC, '\0', TG::Function, 5 },
        { u"ٯتاز"_ustr, TCSCH, '\0', TG::Function, 5 } };

// First character may be any alphabetic
const sal_Int32 coStartFlags = KParseTokens::ANY_LETTER | KParseTokens::IGNORE_LEADING_WS;

// Continuing characters may be any alphabetic
const sal_Int32 coContFlags = (coStartFlags & ~KParseTokens::IGNORE_LEADING_WS)
                              | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;
// First character for numbers, may be any numeric or dot
const sal_Int32 coNumStartFlags
    = KParseTokens::ASC_DIGIT | KParseTokens::ASC_DOT | KParseTokens::IGNORE_LEADING_WS;
// Continuing characters for numbers, may be any numeric or dot or comma.
// tdf#127873: additionally accept ',' comma group separator as too many
// existing documents unwittingly may have used that as decimal separator
// in such locales (though it never was as this is always the en-US locale
// and the group separator is only parsed away).
const sal_Int32 coNumContFlags = (coNumStartFlags & ~KParseTokens::IGNORE_LEADING_WS)
                                 | KParseTokens::GROUP_SEPARATOR_IN_NUMBER;
// First character for numbers hexadecimal
const sal_Int32 coNum16StartFlags
    = KParseTokens::ASC_DIGIT | KParseTokens::ASC_UPALPHA | KParseTokens::IGNORE_LEADING_WS;

// Continuing characters for numbers hexadecimal
const sal_Int32 coNum16ContFlags = (coNum16StartFlags & ~KParseTokens::IGNORE_LEADING_WS);
// user-defined char continuing characters may be any alphanumeric or dot.
const sal_Int32 coUserDefinedCharContFlags = KParseTokens::ANY_LETTER_OR_NUMBER
                                             | KParseTokens::ASC_DOT
                                             | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

//Checks if keyword is in the list.
static inline bool findCompare(const SmTokenTableEntry& lhs, const OUString& s)
{
    return s.compareToIgnoreAsciiCase(lhs.aIdent) > 0;
}

//Returns the SmTokenTableEntry for a keyword
static const SmTokenTableEntry* GetTokenTableEntry(const OUString& rName)
{
    if (rName.isEmpty())
        return nullptr; //avoid null pointer exceptions
    //Looks for the first keyword after or equal to rName in alphabetical order.
    auto findIter
        = std::lower_bound(std::begin(aTokenTable), std::end(aTokenTable), rName, findCompare);
    if (findIter != std::end(aTokenTable) && rName.equalsIgnoreAsciiCase(findIter->aIdent))
        return &*findIter; //check is equal
    return nullptr; //not found
}

static bool IsDelimiter(const OUString& rTxt, sal_Int32 nPos)
{ // returns 'true' iff cChar is '\0' or a delimiter

    assert(nPos <= rTxt.getLength()); //index out of range
    if (nPos == rTxt.getLength())
        return true; //This is EOF
    sal_Unicode cChar = rTxt[nPos];

    // check if 'cChar' is in the delimiter table
    static const sal_Unicode aDelimiterTable[] = {
        ' ', '{', '}', '(', ')', '\t', '\n', '\r', '+', '-',  '*', '/', '=', '[',
        ']', '^', '_', '#', '%', '>',  '<',  '&',  '|', '\\', '"', '~', '`'
    }; //reordered by usage (by eye) for nanoseconds saving.

    //checks the array
    for (auto const& cDelimiter : aDelimiterTable)
    {
        if (cDelimiter == cChar)
            return true;
    }

    //special chars support
    sal_Int16 nTypJp = SM_MOD()->GetSysLocale().GetCharClass().getType(rTxt, nPos);
    return (nTypJp == css::i18n::UnicodeType::SPACE_SEPARATOR
            || nTypJp == css::i18n::UnicodeType::CONTROL);
}

// checks number used as arguments in Math formulas (e.g. 'size' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNumber(const OUString& rText)
{
    bool bPoint = false;
    const sal_Unicode* pBuffer = rText.getStr();
    for (sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
    {
        const sal_Unicode cChar = *pBuffer;
        if (cChar == '.')
        {
            if (bPoint)
                return false;
            else
                bPoint = true;
        }
        else if (!rtl::isAsciiDigit(cChar))
            return false;
    }
    return true;
}
// checks number used as arguments in Math formulas (e.g. 'size' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNotWholeNumber(const OUString& rText)
{
    const sal_Unicode* pBuffer = rText.getStr();
    for (sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
        if (!rtl::isAsciiDigit(*pBuffer))
            return true;
    return false;
}
// checks hex number used as arguments in Math formulas (e.g. 'hex' command)
// Format: no negative numbers, must start with a digit, no exponent notation, ...
static bool lcl_IsNotWholeNumber16(const OUString& rText)
{
    const sal_Unicode* pBuffer = rText.getStr();
    for (sal_Int32 nPos = 0; nPos < rText.getLength(); nPos++, pBuffer++)
        if (!rtl::isAsciiCanonicHexDigit(*pBuffer))
            return true;
    return false;
}

//Text replace onto m_aBufferString
void SmParser5::Replace(sal_Int32 nPos, sal_Int32 nLen, std::u16string_view aText)
{
    assert(nPos + nLen <= m_aBufferString.getLength()); //checks if length allows text replace

    m_aBufferString = m_aBufferString.replaceAt(nPos, nLen, aText); //replace and reindex
    sal_Int32 nChg = aText.size() - nLen;
    m_nBufferIndex = m_nBufferIndex + nChg;
    m_nTokenIndex = m_nTokenIndex + nChg;
}

void SmParser5::NextToken() //Central part of the parser
{
    sal_Int32 nBufLen = m_aBufferString.getLength();
    ParseResult aRes;
    sal_Int32 nRealStart;
    bool bCont;
    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR == m_pSysCC->getType(m_aBufferString, m_nBufferIndex))
            ++m_nBufferIndex;

        // Try to parse a number in a locale-independent manner using
        // '.' as decimal separator.
        // See https://bz.apache.org/ooo/show_bug.cgi?id=45779
        aRes
            = m_aNumCC.parsePredefinedToken(KParseType::ASC_NUMBER, m_aBufferString, m_nBufferIndex,
                                            coNumStartFlags, u""_ustr, coNumContFlags, u""_ustr);

        if (aRes.TokenType == 0)
        {
            // Try again with the default token parsing.
            aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, coStartFlags, u""_ustr,
                                           coContFlags, u""_ustr);
        }

        nRealStart = m_nBufferIndex + aRes.LeadingWhiteSpace;
        m_nBufferIndex = nRealStart;

        bCont = false;
        if (aRes.TokenType == 0 && nRealStart < nBufLen && '\n' == m_aBufferString[nRealStart])
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
                while (m_nBufferIndex < nBufLen && '\n' != m_aBufferString[m_nBufferIndex])
                    ++m_nBufferIndex;
                bCont = true;
            }
        }

    } while (bCont);

    // set index of current token
    m_nTokenIndex = m_nBufferIndex;
    sal_uInt32 nCol = nRealStart - m_nColOff;

    bool bHandled = true;
    if (nRealStart >= nBufLen)
    {
        m_aCurToken.eType = TEND;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nLevel = 0;
        m_aCurToken.aText.clear();
    }
    else if (aRes.TokenType & KParseType::ANY_NUMBER)
    {
        assert(aRes.EndPos > 0);
        if (m_aBufferString[aRes.EndPos - 1] == ',' && aRes.EndPos < nBufLen
            && m_pSysCC->getType(m_aBufferString, aRes.EndPos) != UnicodeType::SPACE_SEPARATOR)
        {
            // Comma followed by a non-space char is unlikely for decimal/thousands separator.
            --aRes.EndPos;
        }
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        m_aCurToken.eType = TNUMBER;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nLevel = 5;
        m_aCurToken.aText = m_aBufferString.copy(nRealStart, n);

        SAL_WARN_IF(!IsDelimiter(m_aBufferString, aRes.EndPos), "starmath",
                    "identifier really finished? (compatibility!)");
    }
    else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
    {
        m_aCurToken.eType = TTEXT;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nLevel = 5;
        m_aCurToken.aText = aRes.DequotedNameOrString;
        nCol++;
    }
    else if (aRes.TokenType & KParseType::IDENTNAME)
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        OUString aName(m_aBufferString.copy(nRealStart, n));
        const SmTokenTableEntry* pEntry = GetTokenTableEntry(aName);

        if (pEntry)
        {
            m_aCurToken.eType = pEntry->eType;
            m_aCurToken.setChar(pEntry->cMathChar);
            m_aCurToken.nGroup = pEntry->nGroup;
            m_aCurToken.nLevel = pEntry->nLevel;
            m_aCurToken.aText = pEntry->aIdent;
        }
        else
        {
            m_aCurToken.eType = TIDENT;
            m_aCurToken.cMathChar = u""_ustr;
            m_aCurToken.nGroup = TG::NONE;
            m_aCurToken.nLevel = 5;
            m_aCurToken.aText = aName;

            SAL_WARN_IF(!IsDelimiter(m_aBufferString, aRes.EndPos), "starmath",
                        "identifier really finished? (compatibility!)");
        }
    }
    else if (aRes.TokenType == 0 && '_' == m_aBufferString[nRealStart])
    {
        m_aCurToken.eType = TRSUB;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::Power;
        m_aCurToken.nLevel = 0;
        m_aCurToken.aText = "_";

        aRes.EndPos = nRealStart + 1;
    }
    else if (aRes.TokenType & KParseType::BOOLEAN)
    {
        sal_Int32& rnEndPos = aRes.EndPos;
        if (rnEndPos - nRealStart <= 2)
        {
            sal_Unicode ch = m_aBufferString[nRealStart];
            switch (ch)
            {
                case '<':
                {
                    if (m_aBufferString.match("<<", nRealStart))
                    {
                        m_aCurToken.eType = TLL;
                        m_aCurToken.setChar(MS_LL);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "<<";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match("<=", nRealStart))
                    {
                        m_aCurToken.eType = TLE;
                        m_aCurToken.setChar(MS_LE);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "<=";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match("<-", nRealStart))
                    {
                        m_aCurToken.eType = TLEFTARROW;
                        m_aCurToken.setChar(MS_LEFTARROW);
                        m_aCurToken.nGroup = TG::Standalone;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "<-";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match("<>", nRealStart))
                    {
                        m_aCurToken.eType = TNEQ;
                        m_aCurToken.setChar(MS_NEQ);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "<>";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match("<?>", nRealStart))
                    {
                        m_aCurToken.eType = TPLACE;
                        m_aCurToken.setChar(MS_PLACE);
                        m_aCurToken.nGroup = TG::NONE;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "<?>";

                        rnEndPos = nRealStart + 3;
                    }
                    else
                    {
                        m_aCurToken.eType = TLT;
                        m_aCurToken.setChar(MS_LT);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "<";
                    }
                }
                break;
                case '>':
                {
                    if (m_aBufferString.match(">=", nRealStart))
                    {
                        m_aCurToken.eType = TGE;
                        m_aCurToken.setChar(MS_GE);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = ">=";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match(">>", nRealStart))
                    {
                        m_aCurToken.eType = TGG;
                        m_aCurToken.setChar(MS_GG);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = ">>";

                        rnEndPos = nRealStart + 2;
                    }
                    else
                    {
                        m_aCurToken.eType = TGT;
                        m_aCurToken.setChar(MS_GT);
                        m_aCurToken.nGroup = TG::Relation;
                        m_aCurToken.nLevel = 0;
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
        sal_Int32& rnEndPos = aRes.EndPos;
        if (rnEndPos - nRealStart == 1)
        {
            sal_Unicode ch = m_aBufferString[nRealStart];
            switch (ch)
            {
                case '%':
                {
                    //! modifies aRes.EndPos

                    OSL_ENSURE(rnEndPos >= nBufLen || '%' != m_aBufferString[rnEndPos],
                               "unexpected comment start");

                    // get identifier of user-defined character
                    ParseResult aTmpRes = m_pSysCC->parseAnyToken(
                        m_aBufferString, rnEndPos, KParseTokens::ANY_LETTER, u""_ustr,
                        coUserDefinedCharContFlags, u""_ustr);

                    sal_Int32 nTmpStart = rnEndPos + aTmpRes.LeadingWhiteSpace;

                    // default setting for the case that no identifier
                    // i.e. a valid symbol-name is following the '%'
                    // character
                    m_aCurToken.eType = TTEXT;
                    m_aCurToken.cMathChar = u""_ustr;
                    m_aCurToken.nGroup = TG::NONE;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "%";

                    if (aTmpRes.TokenType & KParseType::IDENTNAME)
                    {
                        sal_Int32 n = aTmpRes.EndPos - nTmpStart;
                        m_aCurToken.eType = TSPECIAL;
                        m_aCurToken.aText = m_aBufferString.copy(nTmpStart - 1, n + 1);

                        OSL_ENSURE(aTmpRes.EndPos > rnEndPos, "empty identifier");
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
                    m_aCurToken.eType = TLBRACKET;
                    m_aCurToken.setChar(MS_LBRACKET);
                    m_aCurToken.nGroup = TG::LBrace;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "[";
                }
                break;
                case '\\':
                {
                    m_aCurToken.eType = TESCAPE;
                    m_aCurToken.cMathChar = u""_ustr;
                    m_aCurToken.nGroup = TG::NONE;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "\\";
                }
                break;
                case ']':
                {
                    m_aCurToken.eType = TRBRACKET;
                    m_aCurToken.setChar(MS_RBRACKET);
                    m_aCurToken.nGroup = TG::RBrace;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "]";
                }
                break;
                case '^':
                {
                    m_aCurToken.eType = TRSUP;
                    m_aCurToken.cMathChar = u""_ustr;
                    m_aCurToken.nGroup = TG::Power;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "^";
                }
                break;
                case '`':
                {
                    m_aCurToken.eType = TSBLANK;
                    m_aCurToken.cMathChar = u""_ustr;
                    m_aCurToken.nGroup = TG::Blank;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "`";
                }
                break;
                case '{':
                {
                    m_aCurToken.eType = TLGROUP;
                    m_aCurToken.setChar(MS_LBRACE);
                    m_aCurToken.nGroup = TG::NONE;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "{";
                }
                break;
                case '|':
                {
                    m_aCurToken.eType = TOR;
                    m_aCurToken.setChar(MS_OR);
                    m_aCurToken.nGroup = TG::Sum;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "|";
                }
                break;
                case '}':
                {
                    m_aCurToken.eType = TRGROUP;
                    m_aCurToken.setChar(MS_RBRACE);
                    m_aCurToken.nGroup = TG::NONE;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "}";
                }
                break;
                case '~':
                {
                    m_aCurToken.eType = TBLANK;
                    m_aCurToken.cMathChar = u""_ustr;
                    m_aCurToken.nGroup = TG::Blank;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "~";
                }
                break;
                case '#':
                {
                    if (m_aBufferString.match("##", nRealStart))
                    {
                        m_aCurToken.eType = TDPOUND;
                        m_aCurToken.cMathChar = u""_ustr;
                        m_aCurToken.nGroup = TG::NONE;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "##";

                        rnEndPos = nRealStart + 2;
                    }
                    else
                    {
                        m_aCurToken.eType = TPOUND;
                        m_aCurToken.cMathChar = u""_ustr;
                        m_aCurToken.nGroup = TG::NONE;
                        m_aCurToken.nLevel = 0;
                        m_aCurToken.aText = "#";
                    }
                }
                break;
                case '&':
                {
                    m_aCurToken.eType = TAND;
                    m_aCurToken.setChar(MS_AND);
                    m_aCurToken.nGroup = TG::Product;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "&";
                }
                break;
                case '(':
                {
                    m_aCurToken.eType = TLPARENT;
                    m_aCurToken.setChar(MS_LPARENT);
                    m_aCurToken.nGroup = TG::LBrace;
                    m_aCurToken.nLevel = 5; //! 0 to continue expression
                    m_aCurToken.aText = "(";
                }
                break;
                case ')':
                {
                    m_aCurToken.eType = TRPARENT;
                    m_aCurToken.setChar(MS_RPARENT);
                    m_aCurToken.nGroup = TG::RBrace;
                    m_aCurToken.nLevel = 0; //! 0 to terminate expression
                    m_aCurToken.aText = ")";
                }
                break;
                case '*':
                {
                    m_aCurToken.eType = TMULTIPLY;
                    m_aCurToken.setChar(MS_MULTIPLY);
                    m_aCurToken.nGroup = TG::Product;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "*";
                }
                break;
                case '+':
                {
                    if (m_aBufferString.match("+-", nRealStart))
                    {
                        m_aCurToken.eType = TPLUSMINUS;
                        m_aCurToken.setChar(MS_PLUSMINUS);
                        m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "+-";

                        rnEndPos = nRealStart + 2;
                    }
                    else
                    {
                        m_aCurToken.eType = TPLUS;
                        m_aCurToken.setChar(MS_PLUS);
                        m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "+";
                    }
                }
                break;
                case '-':
                {
                    if (m_aBufferString.match("-+", nRealStart))
                    {
                        m_aCurToken.eType = TMINUSPLUS;
                        m_aCurToken.setChar(MS_MINUSPLUS);
                        m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "-+";

                        rnEndPos = nRealStart + 2;
                    }
                    else if (m_aBufferString.match("->", nRealStart))
                    {
                        m_aCurToken.eType = TRIGHTARROW;
                        m_aCurToken.setChar(MS_RIGHTARROW);
                        m_aCurToken.nGroup = TG::Standalone;
                        m_aCurToken.nLevel = 5;
                        m_aCurToken.aText = "->";

                        rnEndPos = nRealStart + 2;
                    }
                    else
                    {
                        m_aCurToken.eType = TMINUS;
                        m_aCurToken.setChar(MS_MINUS);
                        m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                        m_aCurToken.nLevel = 5;
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
                        m_aCurToken.eType = TNUMBER;
                        m_aCurToken.cMathChar = u""_ustr;
                        m_aCurToken.nGroup = TG::NONE;
                        m_aCurToken.nLevel = 5;

                        sal_Int32 nTxtStart = m_nBufferIndex;
                        sal_Unicode cChar;
                        // if the equation ends with dot(.) then increment m_nBufferIndex till end of string only
                        do
                        {
                            cChar = m_aBufferString[++m_nBufferIndex];
                        } while ((cChar == '.' || rtl::isAsciiDigit(cChar))
                                 && (m_nBufferIndex < m_aBufferString.getLength() - 1));

                        m_aCurToken.aText
                            = m_aBufferString.copy(nTxtStart, m_nBufferIndex - nTxtStart);
                        aRes.EndPos = m_nBufferIndex;
                    }
                    else
                        bHandled = false;
                }
                break;
                case '/':
                {
                    m_aCurToken.eType = TDIVIDEBY;
                    m_aCurToken.setChar(MS_SLASH);
                    m_aCurToken.nGroup = TG::Product;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "/";
                }
                break;
                case '=':
                {
                    m_aCurToken.eType = TASSIGN;
                    m_aCurToken.setChar(MS_ASSIGN);
                    m_aCurToken.nGroup = TG::Relation;
                    m_aCurToken.nLevel = 0;
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
        m_aCurToken.eType = TCHARACTER;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nLevel = 5;

        // tdf#129372: we may have to deal with surrogate pairs
        // (see https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates)
        // in this case, we must read 2 sal_Unicode instead of 1
        int nOffset(rtl::isSurrogate(m_aBufferString[nRealStart]) ? 2 : 1);
        m_aCurToken.aText = m_aBufferString.copy(nRealStart, nOffset);

        aRes.EndPos = nRealStart + nOffset;
    }
    m_aCurESelection = ESelection(m_nRow, nCol, m_nRow, nCol + m_aCurToken.aText.getLength());

    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = aRes.EndPos;
}

void SmParser5::NextTokenColor(SmTokenType dvipload)
{
    sal_Int32 nBufLen = m_aBufferString.getLength();
    ParseResult aRes;
    sal_Int32 nRealStart;
    bool bCont;

    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR == m_pSysCC->getType(m_aBufferString, m_nBufferIndex))
            ++m_nBufferIndex;
        //parse, there are few options, so less strict.
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, coStartFlags, u""_ustr,
                                       coContFlags, u""_ustr);
        nRealStart = m_nBufferIndex + aRes.LeadingWhiteSpace;
        m_nBufferIndex = nRealStart;
        bCont = false;
        if (aRes.TokenType == 0 && nRealStart < nBufLen && '\n' == m_aBufferString[nRealStart])
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
                while (m_nBufferIndex < nBufLen && '\n' != m_aBufferString[m_nBufferIndex])
                    ++m_nBufferIndex;
                bCont = true;
            }
        }
    } while (bCont);

    // set index of current token
    m_nTokenIndex = m_nBufferIndex;
    sal_uInt32 nCol = nRealStart - m_nColOff;

    if (nRealStart >= nBufLen)
        m_aCurToken.eType = TEND;
    else if (aRes.TokenType & KParseType::IDENTNAME)
    {
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        OUString aName(m_aBufferString.copy(nRealStart, n));
        switch (dvipload)
        {
            case TCOLOR:
                m_aCurToken = starmathdatabase::Identify_ColorName_Parser(aName);
                break;
            case TDVIPSNAMESCOL:
                m_aCurToken = starmathdatabase::Identify_ColorName_DVIPSNAMES(aName);
                break;
            default:
                m_aCurToken = starmathdatabase::Identify_ColorName_Parser(aName);
                break;
        }
    }
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        if (m_aBufferString[nRealStart] == '#' && !m_aBufferString.match("##", nRealStart))
        {
            m_aCurToken.eType = THEX;
            m_aCurToken.cMathChar = u""_ustr;
            m_aCurToken.nGroup = TG::Color;
            m_aCurToken.nLevel = 0;
            m_aCurToken.aText = "hex";
        }
    }
    else
        m_aCurToken.eType = TNONE;

    m_aCurESelection = ESelection(m_nRow, nCol, m_nRow, nCol + m_aCurToken.aText.getLength());
    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = aRes.EndPos;
}

void SmParser5::NextTokenFontSize()
{
    sal_Int32 nBufLen = m_aBufferString.getLength();
    ParseResult aRes;
    sal_Int32 nRealStart;
    bool bCont;
    bool hex = false;

    do
    {
        // skip white spaces
        while (UnicodeType::SPACE_SEPARATOR == m_pSysCC->getType(m_aBufferString, m_nBufferIndex))
            ++m_nBufferIndex;
        //hexadecimal parser
        aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, coNum16StartFlags,
                                       u"."_ustr, coNum16ContFlags, u".,"_ustr);
        if (aRes.TokenType == 0)
        {
            // Try again with the default token parsing.
            aRes = m_pSysCC->parseAnyToken(m_aBufferString, m_nBufferIndex, coStartFlags, u""_ustr,
                                           coContFlags, u""_ustr);
        }
        else
            hex = true;
        nRealStart = m_nBufferIndex + aRes.LeadingWhiteSpace;
        m_nBufferIndex = nRealStart;
        bCont = false;
        if (aRes.TokenType == 0 && nRealStart < nBufLen && '\n' == m_aBufferString[nRealStart])
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
                while (m_nBufferIndex < nBufLen && '\n' != m_aBufferString[m_nBufferIndex])
                    ++m_nBufferIndex;
                bCont = true;
            }
        }
    } while (bCont);

    // set index of current token
    m_nTokenIndex = m_nBufferIndex;
    sal_uInt32 nCol = nRealStart - m_nColOff;

    if (nRealStart >= nBufLen)
        m_aCurToken.eType = TEND;
    else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
    {
        if (aRes.EndPos - nRealStart == 1)
        {
            switch (m_aBufferString[nRealStart])
            {
                case '*':
                    m_aCurToken.eType = TMULTIPLY;
                    m_aCurToken.setChar(MS_MULTIPLY);
                    m_aCurToken.nGroup = TG::Product;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "*";
                    break;
                case '+':
                    m_aCurToken.eType = TPLUS;
                    m_aCurToken.setChar(MS_PLUS);
                    m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "+";
                    break;
                case '-':
                    m_aCurToken.eType = TMINUS;
                    m_aCurToken.setChar(MS_MINUS);
                    m_aCurToken.nGroup = TG::UnOper | TG::Sum;
                    m_aCurToken.nLevel = 5;
                    m_aCurToken.aText = "-";
                    break;
                case '/':
                    m_aCurToken.eType = TDIVIDEBY;
                    m_aCurToken.setChar(MS_SLASH);
                    m_aCurToken.nGroup = TG::Product;
                    m_aCurToken.nLevel = 0;
                    m_aCurToken.aText = "/";
                    break;
                default:
                    m_aCurToken.eType = TNONE;
                    break;
            }
        }
        else
            m_aCurToken.eType = TNONE;
    }
    else if (hex)
    {
        assert(aRes.EndPos > 0);
        sal_Int32 n = aRes.EndPos - nRealStart;
        assert(n >= 0);
        m_aCurToken.eType = THEX;
        m_aCurToken.cMathChar = u""_ustr;
        m_aCurToken.nGroup = TG::NONE;
        m_aCurToken.nLevel = 5;
        m_aCurToken.aText = m_aBufferString.copy(nRealStart, n);
    }
    else
        m_aCurToken.eType = TNONE;

    m_aCurESelection = ESelection(m_nRow, nCol, m_nRow, nCol + m_aCurToken.aText.getLength());
    if (TEND != m_aCurToken.eType)
        m_nBufferIndex = aRes.EndPos;
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

std::unique_ptr<SmTableNode> SmParser5::DoTable()
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
    xSNode->SetSelection(m_aCurESelection);
    xSNode->SetSubNodes(buildNodeArray(aLineArray));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser5::DoAlign(bool bUseExtraSpaces)
// parse alignment info (if any), then go on with rest of expression
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmStructureNode> xSNode;

    if (TokenInGroup(TG::Align))
    {
        xSNode.reset(new SmAlignNode(m_aCurToken));
        xSNode->SetSelection(m_aCurESelection);

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
std::unique_ptr<SmNode> SmParser5::DoLine()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::vector<std::unique_ptr<SmNode>> ExpressionArray;

    // start with single expression that may have an alignment statement
    // (and go on with expressions that must not have alignment
    // statements in 'while' loop below. See also 'Expression()'.)
    if (m_aCurToken.eType != TEND && m_aCurToken.eType != TNEWLINE)
        ExpressionArray.push_back(DoAlign());

    while (m_aCurToken.eType != TEND && m_aCurToken.eType != TNEWLINE)
        ExpressionArray.push_back(DoExpression());

    //If there's no expression, add an empty one.
    //this is to avoid a formula tree without any caret
    //positions, in visual formula editor.
    if (ExpressionArray.empty())
    {
        SmToken aTok;
        aTok.eType = TNEWLINE;
        ExpressionArray.emplace_back(std::unique_ptr<SmNode>(new SmExpressionNode(aTok)));
    }

    auto xSNode = std::make_unique<SmLineNode>(m_aCurToken);
    xSNode->SetSelection(m_aCurESelection);
    xSNode->SetSubNodes(buildNodeArray(ExpressionArray));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser5::DoExpression(bool bUseExtraSpaces)
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

std::unique_ptr<SmNode> SmParser5::DoRelation()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    int nDepthLimit = m_nParseDepth;

    auto xFirst = DoSum();
    while (TokenInGroup(TG::Relation))
    {
        std::unique_ptr<SmStructureNode> xSNode(new SmBinHorNode(m_aCurToken));
        xSNode->SetSelection(m_aCurESelection);
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

std::unique_ptr<SmNode> SmParser5::DoSum()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    int nDepthLimit = m_nParseDepth;

    auto xFirst = DoProduct();
    while (TokenInGroup(TG::Sum))
    {
        std::unique_ptr<SmStructureNode> xSNode(new SmBinHorNode(m_aCurToken));
        xSNode->SetSelection(m_aCurESelection);
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

std::unique_ptr<SmNode> SmParser5::DoProduct()
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
                xSNode->SetSelection(m_aCurESelection);
                xOper.reset(new SmRectangleNode(m_aCurToken));
                xOper->SetSelection(m_aCurESelection);
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

            case TOVERBRACE:
            case TUNDERBRACE:
                xSNode.reset(new SmVerticalBraceNode(m_aCurToken));
                xSNode->SetSelection(m_aCurESelection);
                xOper.reset(new SmMathSymbolNode(m_aCurToken));
                xOper->SetSelection(m_aCurESelection);

                NextToken();
                break;

            case TWIDEBACKSLASH:
            case TWIDESLASH:
            {
                SmBinDiagonalNode* pSTmp = new SmBinDiagonalNode(m_aCurToken);
                pSTmp->SetAscending(eType == TWIDESLASH);
                xSNode.reset(pSTmp);

                xOper.reset(new SmPolyLineNode(m_aCurToken));
                xOper->SetSelection(m_aCurESelection);
                NextToken();

                break;
            }

            default:
                xSNode.reset(new SmBinHorNode(m_aCurToken));
                xSNode->SetSelection(m_aCurESelection);

                xOper = DoOpSubSup();
        }

        auto xArg = DoPower();
        xSNode->SetSubNodesBinMo(std::move(xFirst), std::move(xOper), std::move(xArg));
        xFirst = std::move(xSNode);
        ++nDepthLimit;
    }
    return xFirst;
}

std::unique_ptr<SmNode> SmParser5::DoSubSup(TG nActiveGroup, std::unique_ptr<SmNode> xGivenNode)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(nActiveGroup == TG::Power || nActiveGroup == TG::Limit);
    assert(m_aCurToken.nGroup == nActiveGroup);

    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(m_aCurToken));
    pNode->SetSelection(m_aCurESelection);
    //! Of course 'm_aCurToken' is just the first sub-/supscript token.
    //! It should be of no further interest. The positions of the
    //! sub-/supscripts will be identified by the corresponding subnodes
    //! index in the 'aSubNodes' array (enum value from 'SmSubSup').

    pNode->SetUseLimits(nActiveGroup == TG::Limit);

    // initialize subnodes array
    std::vector<std::unique_ptr<SmNode>> aSubNodes(1 + SUBSUP_NUM_ENTRIES);
    aSubNodes[0] = std::move(xGivenNode);

    // process all sub-/supscripts
    int nIndex = 0;
    while (TokenInGroup(nActiveGroup))
    {
        SmTokenType eType(m_aCurToken.eType);

        switch (eType)
        {
            case TRSUB:
                nIndex = static_cast<int>(RSUB);
                break;
            case TRSUP:
                nIndex = static_cast<int>(RSUP);
                break;
            case TFROM:
            case TCSUB:
                nIndex = static_cast<int>(CSUB);
                break;
            case TTO:
            case TCSUP:
                nIndex = static_cast<int>(CSUP);
                break;
            case TLSUB:
                nIndex = static_cast<int>(LSUB);
                break;
            case TLSUP:
                nIndex = static_cast<int>(LSUP);
                break;
            default:
                SAL_WARN("starmath", "unknown case");
        }
        nIndex++;
        assert(1 <= nIndex && nIndex <= SUBSUP_NUM_ENTRIES);

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
        if (eType == TFROM || eType == TTO)
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

std::unique_ptr<SmNode> SmParser5::DoSubSupEvaluate(std::unique_ptr<SmNode> xGivenNode)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmSubSupNode> pNode(new SmSubSupNode(m_aCurToken));
    pNode->SetSelection(m_aCurESelection);
    pNode->SetUseLimits(true);

    // initialize subnodes array
    std::vector<std::unique_ptr<SmNode>> aSubNodes(1 + SUBSUP_NUM_ENTRIES);
    aSubNodes[0] = std::move(xGivenNode);

    // process all sub-/supscripts
    int nIndex = 0;
    while (TokenInGroup(TG::Limit))
    {
        SmTokenType eType(m_aCurToken.eType);

        switch (eType)
        {
            case TFROM:
                nIndex = static_cast<int>(RSUB);
                break;
            case TTO:
                nIndex = static_cast<int>(RSUP);
                break;
            default:
                SAL_WARN("starmath", "unknown case");
        }
        nIndex++;
        assert(1 <= nIndex && nIndex <= SUBSUP_NUM_ENTRIES);

        std::unique_ptr<SmNode> xENode;
        if (aSubNodes[nIndex]) // if already occupied at earlier iteration
        {
            // forget the earlier one, remember an error instead
            aSubNodes[nIndex].reset();
            xENode = DoError(SmParseError::DoubleSubsupscript); // this also skips current token.
        }
        else
            NextToken(); // skip sub-/supscript token

        // get sub-/supscript node
        std::unique_ptr<SmNode> xSNode;
        xSNode = DoTerm(true);

        aSubNodes[nIndex] = std::move(xENode ? xENode : xSNode);
    }

    pNode->SetSubNodes(buildNodeArray(aSubNodes));
    return pNode;
}

std::unique_ptr<SmNode> SmParser5::DoOpSubSup()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // get operator symbol
    auto xNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
    xNode->SetSelection(m_aCurESelection);
    // skip operator token
    NextToken();
    // get sub- supscripts if any
    if (m_aCurToken.nGroup == TG::Power)
        return DoSubSup(TG::Power, std::move(xNode));
    return xNode;
}

std::unique_ptr<SmNode> SmParser5::DoPower()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // get body for sub- supscripts on top of stack
    std::unique_ptr<SmNode> xNode(DoTerm(false));

    if (m_aCurToken.nGroup == TG::Power)
        return DoSubSup(TG::Power, std::move(xNode));
    return xNode;
}

std::unique_ptr<SmBlankNode> SmParser5::DoBlank()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Blank));
    std::unique_ptr<SmBlankNode> pBlankNode(new SmBlankNode(m_aCurToken));
    pBlankNode->SetSelection(m_aCurESelection);

    do
    {
        pBlankNode->IncreaseBy(m_aCurToken);
        NextToken();
    } while (TokenInGroup(TG::Blank));

    // Ignore trailing spaces, if corresponding option is set
    if (m_aCurToken.eType == TNEWLINE
        || (m_aCurToken.eType == TEND && !comphelper::IsFuzzing()
            && SM_MOD()->GetConfig()->IsIgnoreSpacesRight()))
    {
        pBlankNode->Clear();
    }
    return pBlankNode;
}

std::unique_ptr<SmNode> SmParser5::DoTerm(bool bGroupNumberIdent)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    switch (m_aCurToken.eType)
    {
        case TESCAPE:
            return DoEscape();

        case TNOSPACE:
        case TLGROUP:
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
                xSNode->SetSelection(m_aCurESelection);
                xSNode->SetSubNodes(nullptr, nullptr);

                NextToken();
                return std::unique_ptr<SmNode>(xSNode.release());
            }

            auto pNode = DoAlign(!bNoSpace);
            if (m_aCurToken.eType == TRGROUP)
            {
                NextToken();
                return pNode;
            }
            auto xSNode = std::make_unique<SmExpressionNode>(m_aCurToken);
            xSNode->SetSelection(m_aCurESelection);
            std::unique_ptr<SmNode> xError(DoError(SmParseError::RgroupExpected));
            xSNode->SetSubNodes(std::move(pNode), std::move(xError));
            return std::unique_ptr<SmNode>(xSNode.release());
        }

        case TLEFT:
            return DoBrace();
        case TEVALUATE:
            return DoEvaluate();

        case TBLANK:
        case TSBLANK:
            return DoBlank();

        case TTEXT:
        {
            auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_TEXT);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return std::unique_ptr<SmNode>(pNode.release());
        }
        case TCHARACTER:
        {
            auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_VARIABLE);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return std::unique_ptr<SmNode>(pNode.release());
        }
        case TIDENT:
        case TNUMBER:
        {
            auto pTextNode = std::make_unique<SmTextNode>(
                m_aCurToken, m_aCurToken.eType == TNUMBER ? FNT_NUMBER : FNT_VARIABLE);
            pTextNode->SetSelection(m_aCurESelection);
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
            while (m_nBufferIndex < nBufLen
                   && m_pSysCC->getType(m_aBufferString, m_nBufferIndex)
                          != UnicodeType::SPACE_SEPARATOR)
            {
                NextToken();
                if (m_aCurToken.eType != TNUMBER && m_aCurToken.eType != TIDENT)
                {
                    // Neither a number nor an identifier. We just moved to
                    // the next token, so no need to do that again.
                    moveToNextToken = false;
                    break;
                }
                aNodes.emplace_back(std::unique_ptr<SmNode>(new SmTextNode(
                    m_aCurToken, m_aCurToken.eType == TNUMBER ? FNT_NUMBER : FNT_VARIABLE)));
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
        case TLEFTARROW:
        case TRIGHTARROW:
        case TUPARROW:
        case TDOWNARROW:
        case TCIRC:
        case TDRARROW:
        case TDLARROW:
        case TDLRARROW:
        case TEXISTS:
        case TNOTEXISTS:
        case TFORALL:
        case TPARTIAL:
        case TNABLA:
        case TLAPLACE:
        case TFOURIER:
        case TTOWARD:
        case TDOTSAXIS:
        case TDOTSDIAG:
        case TDOTSDOWN:
        case TDOTSLOW:
        case TDOTSUP:
        case TDOTSVERT:
        {
            auto pNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return std::unique_ptr<SmNode>(pNode.release());
        }

        case TSETN:
        case TSETZ:
        case TSETQ:
        case TSETR:
        case TSETC:
        case THBAR:
        case TLAMBDABAR:
        case TBACKEPSILON:
        case TALEPH:
        case TIM:
        case TRE:
        case TWP:
        case TEMPTYSET:
        case TINFINITY:
        {
            auto pNode = std::make_unique<SmMathIdentifierNode>(m_aCurToken);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return std::unique_ptr<SmNode>(pNode.release());
        }

        case TPLACE:
        {
            auto pNode = std::make_unique<SmPlaceNode>(m_aCurToken);
            pNode->SetSelection(m_aCurESelection);
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
            if (m_aCurToken.eType == THEX)
            {
                auto pTextNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_NUMBER);
                pTextNode->SetSelection(m_aCurESelection);
                NextToken();
                return pTextNode;
            }
            else
                return DoError(SmParseError::NumberExpected);
        default:
            if (TokenInGroup(TG::LBrace))
                return DoBrace();
            if (TokenInGroup(TG::Oper))
                return DoOperator();
            if (TokenInGroup(TG::UnOper))
                return DoUnOper();
            if (TokenInGroup(TG::Attribute) || TokenInGroup(TG::FontAttr))
            {
                std::stack<std::unique_ptr<SmStructureNode>,
                           std::vector<std::unique_ptr<SmStructureNode>>>
                    aStack;
                bool bIsAttr;
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

std::unique_ptr<SmNode> SmParser5::DoEscape()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    NextToken();

    switch (m_aCurToken.eType)
    {
        case TLPARENT:
        case TRPARENT:
        case TLBRACKET:
        case TRBRACKET:
        case TLDBRACKET:
        case TRDBRACKET:
        case TLBRACE:
        case TLGROUP:
        case TRBRACE:
        case TRGROUP:
        case TLANGLE:
        case TRANGLE:
        case TLCEIL:
        case TRCEIL:
        case TLFLOOR:
        case TRFLOOR:
        case TLLINE:
        case TRLINE:
        case TLDLINE:
        case TRDLINE:
        {
            auto pNode = std::make_unique<SmMathSymbolNode>(m_aCurToken);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return std::unique_ptr<SmNode>(pNode.release());
        }
        default:
            return DoError(SmParseError::UnexpectedToken);
    }
}

std::unique_ptr<SmOperNode> SmParser5::DoOperator()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Oper));

    auto xSNode = std::make_unique<SmOperNode>(m_aCurToken);
    xSNode->SetSelection(m_aCurESelection);

    // get operator
    auto xOperator = DoOper();

    if (m_aCurToken.nGroup == TG::Limit || m_aCurToken.nGroup == TG::Power)
        xOperator = DoSubSup(m_aCurToken.nGroup, std::move(xOperator));

    // get argument
    auto xArg = DoPower();

    xSNode->SetSubNodes(std::move(xOperator), std::move(xArg));
    return xSNode;
}

std::unique_ptr<SmNode> SmParser5::DoOper()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    SmTokenType eType(m_aCurToken.eType);
    std::unique_ptr<SmNode> pNode;

    switch (eType)
    {
        case TSUM:
        case TPROD:
        case TCOPROD:
        case TINT:
        case TINTD:
        case TIINT:
        case TIIINT:
        case TLINT:
        case TLLINT:
        case TLLLINT:
            pNode.reset(new SmMathSymbolNode(m_aCurToken));
            pNode->SetSelection(m_aCurESelection);
            break;

        case TLIM:
        case TLIMSUP:
        case TLIMINF:
        case THADD:
        case TNAHA:
            if (eType == TLIMSUP)
                m_aCurToken.aText = u"lim sup"_ustr;
            else if (eType == TLIMINF)
                m_aCurToken.aText = u"lim inf"_ustr;
            else if (eType == TNAHA)
                m_aCurToken.aText = u"نها"_ustr;
            else if (eType == THADD)
                m_aCurToken.aText = OUString(&MS_HADD, 1);
            else
                m_aCurToken.aText = u"lim"_ustr;
            pNode.reset(new SmTextNode(m_aCurToken, FNT_TEXT));
            pNode->SetSelection(m_aCurESelection);
            break;

        case TOPER:
            NextToken();
            OSL_ENSURE(m_aCurToken.eType == TSPECIAL, "Sm: wrong token");
            m_aCurToken.eType = TOPER;
            pNode.reset(new SmGlyphSpecialNode(m_aCurToken));
            pNode->SetSelection(m_aCurESelection);
            break;

        default:
            assert(false && "unknown case");
    }

    NextToken();
    return pNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoUnOper()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::UnOper));

    SmToken aNodeToken = m_aCurToken;
    ESelection aESelection = m_aCurESelection;
    SmTokenType eType = m_aCurToken.eType;
    bool bIsPostfix = eType == TFACT;

    std::unique_ptr<SmStructureNode> xSNode;
    std::unique_ptr<SmNode> xOper;
    std::unique_ptr<SmNode> xExtra;
    std::unique_ptr<SmNode> xArg;

    switch (eType)
    {
        case TABS:
        case TSQRT:
            NextToken();
            break;

        case TNROOT:
            NextToken();
            xExtra = DoPower();
            break;

        case TUOPER:
            NextToken();
            //Let the glyph know what it is...
            m_aCurToken.eType = TUOPER;
            m_aCurToken.nGroup = TG::UnOper;
            xOper = DoGlyphSpecial();
            break;

        case TPLUS:
        case TMINUS:
        case TPLUSMINUS:
        case TMINUSPLUS:
        case TNEG:
        case TFACT:
            xOper = DoOpSubSup();
            break;

        default:
            assert(false);
    }

    // get argument
    xArg = DoPower();

    if (eType == TABS)
    {
        xSNode.reset(new SmBraceNode(aNodeToken));
        xSNode->SetSelection(aESelection);
        xSNode->SetScaleMode(SmScaleMode::Height);

        // build nodes for left & right lines
        // (text, group, level of the used token are of no interest here)
        // we'll use row & column of the keyword for abs
        aNodeToken.eType = TABS;

        aNodeToken.setChar(MS_VERTLINE);
        std::unique_ptr<SmNode> xLeft(new SmMathSymbolNode(aNodeToken));
        xLeft->SetSelection(aESelection);
        std::unique_ptr<SmNode> xRight(new SmMathSymbolNode(aNodeToken));
        xRight->SetSelection(aESelection);

        xSNode->SetSubNodes(std::move(xLeft), std::move(xArg), std::move(xRight));
    }
    else if (eType == TSQRT || eType == TNROOT)
    {
        xSNode.reset(new SmRootNode(aNodeToken));
        xSNode->SetSelection(aESelection);
        xOper.reset(new SmRootSymbolNode(aNodeToken));
        xOper->SetSelection(aESelection);
        xSNode->SetSubNodes(std::move(xExtra), std::move(xOper), std::move(xArg));
    }
    else
    {
        xSNode.reset(new SmUnHorNode(aNodeToken));
        xSNode->SetSelection(aESelection);
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

std::unique_ptr<SmStructureNode> SmParser5::DoAttribute()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::Attribute));

    auto xSNode = std::make_unique<SmAttributeNode>(m_aCurToken);
    xSNode->SetSelection(m_aCurESelection);
    std::unique_ptr<SmNode> xAttr;
    SmScaleMode eScaleMode = SmScaleMode::None;

    // get appropriate node for the attribute itself
    switch (m_aCurToken.eType)
    {
        case TUNDERLINE:
        case TOVERLINE:
        case TOVERSTRIKE:
            xAttr.reset(new SmRectangleNode(m_aCurToken));
            xAttr->SetSelection(m_aCurESelection);
            eScaleMode = SmScaleMode::Width;
            break;

        case TWIDEVEC:
        case TWIDEHARPOON:
        case TWIDEHAT:
        case TWIDETILDE:
            xAttr.reset(new SmMathSymbolNode(m_aCurToken));
            xAttr->SetSelection(m_aCurESelection);
            eScaleMode = SmScaleMode::Width;
            break;

        default:
            xAttr.reset(new SmMathSymbolNode(m_aCurToken));
            xAttr->SetSelection(m_aCurESelection);
    }

    NextToken();

    xSNode->SetSubNodes(std::move(xAttr), nullptr); // the body will be filled later
    xSNode->SetScaleMode(eScaleMode);
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoFontAttribute()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(TokenInGroup(TG::FontAttr));

    switch (m_aCurToken.eType)
    {
        case TITALIC:
        case TNITALIC:
        case TBOLD:
        case TNBOLD:
        case TPHANTOM:
        {
            auto pNode = std::make_unique<SmFontNode>(m_aCurToken);
            pNode->SetSelection(m_aCurESelection);
            NextToken();
            return pNode;
        }

        case TSIZE:
            return DoFontSize();

        case TFONT:
            return DoFont();

        case TCOLOR:
            return DoColor();

        default:
            assert(false);
            return {};
    }
}

std::unique_ptr<SmStructureNode> SmParser5::DoColor()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TCOLOR);
    sal_Int32 nBufferIndex = m_nBufferIndex;
    NextTokenColor(TCOLOR);
    SmToken aToken;
    ESelection aESelection;

    if (m_aCurToken.eType == TDVIPSNAMESCOL)
        NextTokenColor(TDVIPSNAMESCOL);
    if (m_aCurToken.eType == TERROR)
        return DoError(SmParseError::ColorExpected);
    if (TokenInGroup(TG::Color))
    {
        aToken = m_aCurToken;
        aESelection = m_aCurESelection;
        if (m_aCurToken.eType == TRGB) //loads r, g and b
        {
            sal_uInt32 nr, ng, nb, nc;
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            nr = m_aCurToken.aText.toUInt32();
            if (nr > 255)
                return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            ng = m_aCurToken.aText.toUInt32();
            if (ng > 255)
                return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            nb = m_aCurToken.aText.toUInt32();
            if (nb > 255)
                return DoError(SmParseError::ColorExpected);
            nc = nb | ng << 8 | nr << 16 | sal_uInt32(0) << 24;
            aToken.cMathChar = OUString::number(nc, 16);
        }
        else if (m_aCurToken.eType == TRGBA) //loads r, g and b
        {
            sal_uInt32 nr, na, ng, nb, nc;
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            nr = m_aCurToken.aText.toUInt32();
            if (nr > 255)
                return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            ng = m_aCurToken.aText.toUInt32();
            if (ng > 255)
                return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            nb = m_aCurToken.aText.toUInt32();
            if (nb > 255)
                return DoError(SmParseError::ColorExpected);
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            na = m_aCurToken.aText.toUInt32();
            if (na > 255)
                return DoError(SmParseError::ColorExpected);
            nc = nb | ng << 8 | nr << 16 | na << 24;
            aToken.cMathChar = OUString::number(nc, 16);
        }
        else if (m_aCurToken.eType == THEX) //loads hex code
        {
            sal_uInt32 nc;
            NextTokenFontSize();
            if (lcl_IsNotWholeNumber16(m_aCurToken.aText))
                return DoError(SmParseError::ColorExpected);
            nc = m_aCurToken.aText.toUInt32(16);
            aToken.cMathChar = OUString::number(nc, 16);
        }
        aToken.aText = m_aBufferString.subView(nBufferIndex, m_nBufferIndex - nBufferIndex);
        NextToken();
    }
    else
        return DoError(SmParseError::ColorExpected);

    std::unique_ptr<SmStructureNode> xNode;
    xNode.reset(new SmFontNode(aToken));
    xNode->SetSelection(aESelection);
    return xNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoFont()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TFONT);

    std::unique_ptr<SmStructureNode> xNode;
    // last font rules, get that one
    SmToken aToken;
    ESelection aESelection = m_aCurESelection;
    do
    {
        NextToken();

        if (TokenInGroup(TG::Font))
        {
            aToken = m_aCurToken;
            NextToken();
        }
        else
        {
            return DoError(SmParseError::FontExpected);
        }
    } while (m_aCurToken.eType == TFONT);

    xNode.reset(new SmFontNode(aToken));
    xNode->SetSelection(aESelection);
    return xNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoFontSize()
{
    DepthProtect aDepthGuard(m_nParseDepth);
    std::unique_ptr<SmFontNode> pFontNode(new SmFontNode(m_aCurToken));
    pFontNode->SetSelection(m_aCurESelection);
    NextTokenFontSize();
    FontSizeType Type;

    switch (m_aCurToken.eType)
    {
        case THEX:
            Type = FontSizeType::ABSOLUT;
            break;
        case TPLUS:
            Type = FontSizeType::PLUS;
            break;
        case TMINUS:
            Type = FontSizeType::MINUS;
            break;
        case TMULTIPLY:
            Type = FontSizeType::MULTIPLY;
            break;
        case TDIVIDEBY:
            Type = FontSizeType::DIVIDE;
            break;

        default:
            return DoError(SmParseError::SizeExpected);
    }

    if (Type != FontSizeType::ABSOLUT)
    {
        NextTokenFontSize();
        if (m_aCurToken.eType != THEX)
            return DoError(SmParseError::SizeExpected);
    }

    // get number argument
    Fraction aValue(1);
    if (lcl_IsNumber(m_aCurToken.aText))
    {
        aValue = m_aCurToken.aText.toDouble();
        //!! Reduce values in order to avoid numerical errors
        if (aValue.GetDenominator() > 1000)
        {
            tools::Long nNum = aValue.GetNumerator();
            tools::Long nDenom = aValue.GetDenominator();
            while (nDenom > 1000) //remove big denominator
            {
                nNum /= 10;
                nDenom /= 10;
            }
            aValue = Fraction(nNum, nDenom);
        }
    }
    else
        return DoError(SmParseError::SizeExpected);

    pFontNode->SetSizeParameter(aValue, Type);
    NextToken();
    return pFontNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoBrace()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    assert(m_aCurToken.eType == TLEFT || TokenInGroup(TG::LBrace));

    std::unique_ptr<SmStructureNode> xSNode(new SmBraceNode(m_aCurToken));
    xSNode->SetSelection(m_aCurESelection);
    std::unique_ptr<SmNode> pBody, pLeft, pRight;
    SmScaleMode eScaleMode = SmScaleMode::None;
    SmParseError eError = SmParseError::None;

    if (m_aCurToken.eType == TLEFT)
    {
        NextToken();

        eScaleMode = SmScaleMode::Height;

        // check for left bracket
        if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
        {
            pLeft.reset(new SmMathSymbolNode(m_aCurToken));
            pLeft->SetSelection(m_aCurESelection);

            NextToken();
            pBody = DoBracebody(true);

            if (m_aCurToken.eType == TRIGHT)
            {
                NextToken();

                // check for right bracket
                if (TokenInGroup(TG::LBrace) || TokenInGroup(TG::RBrace))
                {
                    pRight.reset(new SmMathSymbolNode(m_aCurToken));
                    pRight->SetSelection(m_aCurESelection);
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
        pLeft->SetSelection(m_aCurESelection);

        NextToken();
        pBody = DoBracebody(false);

        SmTokenType eExpectedType = TUNKNOWN;
        switch (pLeft->GetToken().eType)
        {
            case TLPARENT:
                eExpectedType = TRPARENT;
                break;
            case TLBRACKET:
                eExpectedType = TRBRACKET;
                break;
            case TLBRACE:
                eExpectedType = TRBRACE;
                break;
            case TLDBRACKET:
                eExpectedType = TRDBRACKET;
                break;
            case TLLINE:
                eExpectedType = TRLINE;
                break;
            case TLDLINE:
                eExpectedType = TRDLINE;
                break;
            case TLANGLE:
                eExpectedType = TRANGLE;
                break;
            case TLFLOOR:
                eExpectedType = TRFLOOR;
                break;
            case TLCEIL:
                eExpectedType = TRCEIL;
                break;
            case TLRLINE:
                eExpectedType = TLRLINE;
                break;
            case TLRDLINE:
                eExpectedType = TLRDLINE;
                break;
            default:
                SAL_WARN("starmath", "unknown case");
        }

        if (m_aCurToken.eType == eExpectedType)
        {
            pRight.reset(new SmMathSymbolNode(m_aCurToken));
            pRight->SetSelection(m_aCurESelection);
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

std::unique_ptr<SmBracebodyNode> SmParser5::DoBracebody(bool bIsLeftRight)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto pBody = std::make_unique<SmBracebodyNode>(m_aCurToken);
    pBody->SetSelection(m_aCurESelection);

    std::vector<std::unique_ptr<SmNode>> aNodes;
    // get body if any
    if (bIsLeftRight)
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                SmMathSymbolNode* pTempNode = new SmMathSymbolNode(m_aCurToken);
                pTempNode->SetSelection(m_aCurESelection);
                aNodes.emplace_back(std::unique_ptr<SmMathSymbolNode>(pTempNode));
                NextToken();
            }
            else if (m_aCurToken.eType != TRIGHT)
            {
                aNodes.push_back(DoAlign());
                if (m_aCurToken.eType != TMLINE && m_aCurToken.eType != TRIGHT)
                    aNodes.emplace_back(DoError(SmParseError::RightExpected));
            }
        } while (m_aCurToken.eType != TEND && m_aCurToken.eType != TRIGHT);
    }
    else
    {
        do
        {
            if (m_aCurToken.eType == TMLINE)
            {
                SmMathSymbolNode* pTempNode = new SmMathSymbolNode(m_aCurToken);
                pTempNode->SetSelection(m_aCurESelection);
                aNodes.emplace_back(std::unique_ptr<SmMathSymbolNode>(pTempNode));
                NextToken();
            }
            else if (!TokenInGroup(TG::RBrace))
            {
                aNodes.push_back(DoAlign());
                if (m_aCurToken.eType != TMLINE && !TokenInGroup(TG::RBrace))
                    aNodes.emplace_back(DoError(SmParseError::RbraceExpected));
            }
        } while (m_aCurToken.eType != TEND && !TokenInGroup(TG::RBrace));
    }

    pBody->SetSubNodes(buildNodeArray(aNodes));
    pBody->SetScaleMode(bIsLeftRight ? SmScaleMode::Height : SmScaleMode::None);
    return pBody;
}

std::unique_ptr<SmNode> SmParser5::DoEvaluate()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // Create node
    std::unique_ptr<SmStructureNode> xSNode(new SmBraceNode(m_aCurToken));
    xSNode->SetSelection(m_aCurESelection);
    SmToken aToken(TRLINE, MS_VERTLINE, u"evaluate"_ustr, TG::RBrace, 5);

    // Parse body && left none
    NextToken();
    std::unique_ptr<SmNode> pBody = DoPower();
    SmToken bToken(TNONE, '\0', u""_ustr, TG::LBrace, 5);
    std::unique_ptr<SmNode> pLeft;
    pLeft.reset(new SmMathSymbolNode(bToken));

    // Mount nodes
    std::unique_ptr<SmNode> pRight;
    pRight.reset(new SmMathSymbolNode(aToken));
    xSNode->SetSubNodes(std::move(pLeft), std::move(pBody), std::move(pRight));
    xSNode->SetScaleMode(SmScaleMode::Height); // scalable line

    // Parse from to
    if (m_aCurToken.nGroup == TG::Limit)
    {
        std::unique_ptr<SmNode> rSNode;
        rSNode = DoSubSupEvaluate(std::move(xSNode));
        rSNode->GetToken().eType = TEVALUATE;
        return rSNode;
    }

    return xSNode;
}

std::unique_ptr<SmTextNode> SmParser5::DoFunction()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    if (m_aCurToken.eType == TFUNC)
    {
        NextToken(); // skip "FUNC"-statement
        m_aCurToken.eType = TFUNC;
        m_aCurToken.nGroup = TG::Function;
    }
    auto pNode = std::make_unique<SmTextNode>(m_aCurToken, FNT_FUNCTION);
    pNode->SetSelection(m_aCurESelection);
    NextToken();
    return pNode;
}

std::unique_ptr<SmTableNode> SmParser5::DoBinom()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto xSNode = std::make_unique<SmTableNode>(m_aCurToken);
    xSNode->SetSelection(m_aCurESelection);

    NextToken();

    auto xFirst = DoSum();
    auto xSecond = DoSum();
    xSNode->SetSubNodes(std::move(xFirst), std::move(xSecond));
    return xSNode;
}

std::unique_ptr<SmBinVerNode> SmParser5::DoFrac()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmBinVerNode> xSNode = std::make_unique<SmBinVerNode>(m_aCurToken);
    xSNode->SetSelection(m_aCurESelection);
    std::unique_ptr<SmNode> xOper = std::make_unique<SmRectangleNode>(m_aCurToken);
    xOper->SetSelection(m_aCurESelection);

    NextToken();

    auto xFirst = DoSum();
    auto xSecond = DoSum();
    xSNode->SetSubNodes(std::move(xFirst), std::move(xOper), std::move(xSecond));
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoStack()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmStructureNode> xSNode(new SmTableNode(m_aCurToken));
    xSNode->SetSelection(m_aCurESelection);
    NextToken();
    if (m_aCurToken.eType != TLGROUP)
        return DoError(SmParseError::LgroupExpected);
    std::vector<std::unique_ptr<SmNode>> aExprArr;
    do
    {
        NextToken();
        aExprArr.push_back(DoAlign());
    } while (m_aCurToken.eType == TPOUND);

    if (m_aCurToken.eType == TRGROUP)
        NextToken();
    else
        aExprArr.emplace_back(DoError(SmParseError::RgroupExpected));

    xSNode->SetSubNodes(buildNodeArray(aExprArr));
    return xSNode;
}

std::unique_ptr<SmStructureNode> SmParser5::DoMatrix()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    std::unique_ptr<SmMatrixNode> xMNode(new SmMatrixNode(m_aCurToken));
    xMNode->SetSelection(m_aCurESelection);
    NextToken();
    if (m_aCurToken.eType != TLGROUP)
        return DoError(SmParseError::LgroupExpected);

    std::vector<std::unique_ptr<SmNode>> aExprArr;
    do
    {
        NextToken();
        aExprArr.push_back(DoAlign());
    } while (m_aCurToken.eType == TPOUND);

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
    xMNode->SetRowCol(static_cast<sal_uInt16>(nRow), static_cast<sal_uInt16>(nCol));
    return std::unique_ptr<SmStructureNode>(xMNode.release());
}

std::unique_ptr<SmSpecialNode> SmParser5::DoSpecial()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    bool bReplace = false;
    OUString& rName = m_aCurToken.aText;
    OUString aNewName;

    // conversion of symbol names for 6.0 (XML) file format
    // (name change on import / export.
    // UI uses localized names XML file format does not.)
    if (rName.startsWith("%"))
    {
        if (IsImportSymbolNames())
        {
            const SmSym* pSym
                = SM_MOD()->GetSymbolManager().GetSymbolByExportName(rName.subView(1));
            if (pSym)
            {
                aNewName = pSym->GetUiName();
                bReplace = true;
            }
        }
        else if (IsExportSymbolNames())
        {
            const SmSym* pSym = SM_MOD()->GetSymbolManager().GetSymbolByUiName(rName.subView(1));
            if (pSym)
            {
                aNewName = pSym->GetExportName();
                bReplace = true;
            }
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
        m_aUsedSymbols.insert(aSymbolName);

    auto pNode = std::make_unique<SmSpecialNode>(m_aCurToken);
    pNode->SetSelection(m_aCurESelection);
    NextToken();
    return pNode;
}

std::unique_ptr<SmGlyphSpecialNode> SmParser5::DoGlyphSpecial()
{
    DepthProtect aDepthGuard(m_nParseDepth);

    auto pNode = std::make_unique<SmGlyphSpecialNode>(m_aCurToken);
    NextToken();
    return pNode;
}

std::unique_ptr<SmExpressionNode> SmParser5::DoError(SmParseError eError)
{
    DepthProtect aDepthGuard(m_nParseDepth);

    // Generate error node
    m_aCurToken.eType = TERROR;
    // Identify error message
    m_aCurToken.cMathChar = SmResId(RID_ERR_IDENT) + starmathdatabase::getParseErrorDesc(eError);
    auto xSNode = std::make_unique<SmExpressionNode>(m_aCurToken);
    SmErrorNode* pErr(new SmErrorNode(m_aCurToken));
    pErr->SetSelection(m_aCurESelection);
    xSNode->SetSubNode(0, pErr);

    // Append error to the error list
    SmErrorDesc aErrDesc(eError, xSNode.get(), m_aCurToken.cMathChar);
    m_aErrDescList.push_back(aErrDesc);

    NextToken();

    return xSNode;
}

// end grammar

SmParser5::SmParser5()
    : m_nCurError(0)
    , m_nBufferIndex(0)
    , m_nTokenIndex(0)
    , m_nRow(0)
    , m_nColOff(0)
    , m_bImportSymNames(false)
    , m_bExportSymNames(false)
    , m_nParseDepth(0)
    , m_aNumCC(LanguageTag(LANGUAGE_ENGLISH_US))
    , m_pSysCC(&SM_MOD()->GetSysLocale().GetCharClass())
{
}

SmParser5::~SmParser5() {}

std::unique_ptr<SmTableNode> SmParser5::Parse(const OUString& rBuffer)
{
    m_aUsedSymbols.clear();

    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex = 0;
    m_nTokenIndex = 0;
    m_nRow = 0;
    m_nColOff = 0;
    m_nCurError = -1;

    m_aErrDescList.clear();

    NextToken();
    return DoTable();
}

std::unique_ptr<SmNode> SmParser5::ParseExpression(const OUString& rBuffer)
{
    m_aBufferString = convertLineEnd(rBuffer, LINEEND_LF);
    m_nBufferIndex = 0;
    m_nTokenIndex = 0;
    m_nRow = 0;
    m_nColOff = 0;
    m_nCurError = -1;

    m_aErrDescList.clear();

    NextToken();
    return DoExpression();
}

const SmErrorDesc* SmParser5::NextError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError > 0)
            return &m_aErrDescList[--m_nCurError];
        else
        {
            m_nCurError = 0;
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser5::PrevError()
{
    if (!m_aErrDescList.empty())
        if (m_nCurError < static_cast<int>(m_aErrDescList.size() - 1))
            return &m_aErrDescList[++m_nCurError];
        else
        {
            m_nCurError = static_cast<int>(m_aErrDescList.size() - 1);
            return &m_aErrDescList[m_nCurError];
        }
    else
        return nullptr;
}

const SmErrorDesc* SmParser5::GetError() const
{
    if (m_aErrDescList.empty())
        return nullptr;
    return &m_aErrDescList.front();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
