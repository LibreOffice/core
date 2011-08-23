/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdio.h>

#define SMDLL 1

#include <com/sun/star/i18n/UnicodeType.hpp>

#include <bf_svtools/syslocale.hxx>
#include <sal/macros.h>

#include "starmath.hrc"
#include "config.hxx"

#include "node.hxx"
namespace binfilter {

using namespace ::com::sun::star::i18n;

///////////////////////////////////////////////////////////////////////////


/*N*/ static const sal_Unicode aDelimiterTable[] =
/*N*/ {
/*N*/ 	' ',	'\t',	'\n',	'\r',	'+',	'-',	'*',	'/',	'=',	'#',
/*N*/ 	'%',	'\\',	'"',	'~',	'`',	'>',	'<',	'&',	'|',	'(',
/*N*/ 	')',	'{',	'}',	'[',	']',	'^',	'_',
/*N*/ 	'\0'	// end of list symbol
/*N*/ };



///////////////////////////////////////////////////////////////////////////

/*N*/ SmToken::SmToken() :
/*N*/ 	eType		(TUNKNOWN),
/*N*/ 	cMathChar	('\0')
/*N*/ {
/*N*/ 	nRow = nCol = nGroup = nLevel = 0;
/*N*/ }

///////////////////////////////////////////////////////////////////////////

/*?*/ struct SmTokenTableEntry
/*?*/ {
/*?*/ 	const sal_Char*	pIdent;
/*?*/ 	SmTokenType	 	eType;
/*?*/ 	sal_Unicode	 	cMathChar;
/*?*/ 	ULONG 		 	nGroup;
/*?*/ 	USHORT		 	nLevel;
/*?*/ };

/*?*/ static const SmTokenTableEntry aTokenTable[] =
/*?*/ {
/*?*/ //	{ "#", TPOUND, '\0', 0, 0 },
/*?*/ //	{ "##", TDPOUND, '\0', 0, 0 },
/*?*/ //	{ "&", TAND, MS_AND, TGPRODUCT, 0 },
/*?*/ //	{ "(", TLPARENT, MS_LPARENT, TGLBRACES, 5 },	//! 5 to continue expression
/*?*/ //	{ ")", TRPARENT, MS_RPARENT, TGRBRACES, 0 },	//! 0 to terminate expression
/*?*/ //	{ "*", TMULTIPLY, MS_MULTIPLY, TGPRODUCT, 0 },
/*?*/ //	{ "+", TPLUS, MS_PLUS, TGUNOPER | TGSUM, 5 },
/*?*/ //	{ "+-", TPLUSMINUS, MS_PLUSMINUS, TGUNOPER | TGSUM, 5 },
/*?*/ //	{ "-", TMINUS, MS_MINUS, TGUNOPER | TGSUM, 5 },
/*?*/ //	{ "-+", TMINUSPLUS, MS_MINUSPLUS, TGUNOPER | TGSUM, 5 },
/*?*/ //	{ ".", TPOINT, '\0', 0, 0 },
/*?*/ //	{ "/", TDIVIDEBY, MS_SLASH, TGPRODUCT, 0 },
/*?*/ //	{ "<", TLT, MS_LT, TGRELATION, 0 },
/*?*/ //	{ "<<", TLL, MS_LL, TGRELATION, 0 },
/*?*/ //	{ "<=", TLE, MS_LE, TGRELATION, 0 },
/*?*/ //	{ "<>", TNEQ, MS_NEQ, TGRELATION, 0},
/*?*/ //	{ "<?>", TPLACE, MS_PLACE, 0, 5 },
/*?*/ //	{ "=", TASSIGN, MS_ASSIGN, TGRELATION, 0},
/*?*/ //	{ ">", TGT, MS_GT, TGRELATION, 0 },
/*?*/ //	{ ">=", TGE, MS_GE, TGRELATION, 0 },
/*?*/ //	{ ">>", TGG, MS_GG, TGRELATION, 0 },
/*?*/ 	{ "Im" , TIM, MS_IM, TGSTANDALONE, 5 },
/*?*/ 	{ "MZ23", TDEBUG, '\0', TGATTRIBUT, 0 },
/*?*/ 	{ "Re" , TRE, MS_RE, TGSTANDALONE, 5 },
/*?*/ 	{ "abs", TABS, '\0', TGUNOPER, 13 },
/*?*/ 	{ "arcosh", TACOSH, '\0', TGFUNCTION, 5 },
/*?*/ 	{ "arcoth", TACOTH, '\0', TGFUNCTION, 5 },
/*?*/ 	{ "acute", TACUTE, MS_ACUTE, TGATTRIBUT, 5 },
/*?*/ 	{ "aleph" , TALEPH, MS_ALEPH, TGSTANDALONE, 5 },
/*?*/ 	{ "alignb", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
/*?*/ 	{ "alignc", TALIGNC, '\0', TGALIGN, 0},
/*?*/ 	{ "alignl", TALIGNL, '\0', TGALIGN, 0},
/*?*/ 	{ "alignm", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
/*?*/ 	{ "alignr", TALIGNR, '\0', TGALIGN, 0},
/*?*/ 	{ "alignt", TALIGNC, '\0', TGALIGN | TGDISCARDED, 0},
/*?*/ 	{ "and", TAND, MS_AND, TGPRODUCT, 0},
/*?*/ 	{ "approx", TAPPROX, MS_APPROX, TGRELATION, 0},
/*?*/ 	{ "arccos", TACOS, '\0', TGFUNCTION, 5},
/*?*/ 	{ "arccot", TACOT, '\0', TGFUNCTION, 5},
/*?*/ 	{ "arcsin", TASIN, '\0', TGFUNCTION, 5},
/*?*/ 	{ "arctan", TATAN, '\0', TGFUNCTION, 5},
/*?*/ 	{ "arsinh", TASINH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "artanh", TATANH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "backepsilon" , TBACKEPSILON, MS_BACKEPSILON, TGSTANDALONE, 5},
/*?*/ 	{ "bar", TBAR, MS_BAR, TGATTRIBUT, 5},
/*?*/ 	{ "binom", TBINOM, '\0', 0, 5 },
/*?*/ 	{ "black", TBLACK, '\0', TGCOLOR, 0},
/*?*/ 	{ "blue", TBLUE, '\0', TGCOLOR, 0},
/*?*/ 	{ "bold", TBOLD, '\0', TGFONTATTR, 5},
/*?*/ 	{ "boper", TBOPER, '\0', TGPRODUCT, 0},
/*?*/ 	{ "breve", TBREVE, MS_BREVE, TGATTRIBUT, 5},
/*?*/ 	{ "bslash", TBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
/*?*/ 	{ "cdot", TCDOT, MS_CDOT, TGPRODUCT, 0},
/*?*/ 	{ "check", TCHECK, MS_CHECK, TGATTRIBUT, 5},
/*?*/ 	{ "circ" , TCIRC, MS_CIRC, TGSTANDALONE, 5},
/*?*/ 	{ "circle", TCIRCLE, MS_CIRCLE, TGATTRIBUT, 5},
/*?*/ 	{ "color", TCOLOR, '\0', TGFONTATTR, 5},
/*?*/ 	{ "coprod", TCOPROD, MS_COPROD, TGOPER, 5},
/*?*/ 	{ "cos", TCOS, '\0', TGFUNCTION, 5},
/*?*/ 	{ "cosh", TCOSH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "cot", TCOT, '\0', TGFUNCTION, 5},
/*?*/ 	{ "coth", TCOTH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "csub", TCSUB, '\0', TGPOWER, 0},
/*?*/ 	{ "csup", TCSUP, '\0', TGPOWER, 0},
/*?*/ 	{ "cyan", TCYAN, '\0', TGCOLOR, 0},
/*?*/ 	{ "dddot", TDDDOT, MS_DDDOT, TGATTRIBUT, 5},
/*?*/ 	{ "ddot", TDDOT, MS_DDOT, TGATTRIBUT, 5},
/*?*/ 	{ "def", TDEF, MS_DEF, TGRELATION, 0},
/*?*/ 	{ "div", TDIV, MS_DIV, TGPRODUCT, 0},
/*?*/ 	{ "divides", TDIVIDES, MS_LINE, TGRELATION, 0},
/*?*/ 	{ "dlarrow" , TDLARROW, MS_DLARROW, TGSTANDALONE, 5},
/*?*/ 	{ "dlrarrow" , TDLRARROW, MS_DLRARROW, TGSTANDALONE, 5},
/*?*/ 	{ "dot", TDOT, MS_DOT, TGATTRIBUT, 5},
/*?*/ 	{ "dotsaxis", TDOTSAXIS, MS_DOTSAXIS, TGSTANDALONE, 5},	// 5 to continue expression
/*?*/ 	{ "dotsdiag", TDOTSDIAG, MS_DOTSUP, TGSTANDALONE, 5},	//
/*?*/ 	{ "dotsdown", TDOTSDOWN, MS_DOTSDOWN, TGSTANDALONE, 5},  //
/*?*/ 	{ "dotslow", TDOTSLOW, MS_DOTSLOW, TGSTANDALONE, 5},    //
/*?*/ 	{ "dotsup", TDOTSUP, MS_DOTSUP, TGSTANDALONE, 5},      //
/*?*/ 	{ "dotsvert", TDOTSVERT, MS_DOTSVERT, TGSTANDALONE, 5},	//
/*?*/ 	{ "downarrow" , TDOWNARROW, MS_DOWNARROW, TGSTANDALONE, 5},
/*?*/ 	{ "drarrow" , TDRARROW, MS_DRARROW, TGSTANDALONE, 5},
/*?*/ 	{ "emptyset" , TEMPTYSET, MS_EMPTYSET, TGSTANDALONE, 5},
/*?*/ 	{ "equiv", TEQUIV, MS_EQUIV, TGRELATION, 0},
/*?*/ 	{ "exists", TEXISTS, MS_EXISTS, TGSTANDALONE, 5},
/*?*/ 	{ "exp", TEXP, '\0', TGFUNCTION, 5},
/*?*/ 	{ "fact", TFACT, MS_FACT, TGUNOPER, 5},
/*?*/ 	{ "fixed", TFIXED, '\0', TGFONT, 0},
/*?*/ 	{ "font", TFONT, '\0', TGFONTATTR, 5},
/*?*/ 	{ "forall", TFORALL, MS_FORALL, TGSTANDALONE, 5},
/*?*/ 	{ "from", TFROM, '\0', TGLIMIT, 0},
/*?*/ 	{ "func", TFUNC, '\0', TGFUNCTION, 5},
/*?*/ 	{ "ge", TGE, MS_GE, TGRELATION, 0},
/*?*/ 	{ "geslant", TGESLANT, MS_GESLANT, TGRELATION, 0 },
/*?*/ 	{ "gg", TGG, MS_GG, TGRELATION, 0},
/*?*/ 	{ "grave", TGRAVE, MS_GRAVE, TGATTRIBUT, 5},
/*?*/ 	{ "green", TGREEN, '\0', TGCOLOR, 0},
/*?*/ 	{ "gt", TGT, MS_GT, TGRELATION, 0},
/*?*/ 	{ "hat", THAT, MS_HAT, TGATTRIBUT, 5},
/*?*/ 	{ "hbar" , THBAR, MS_HBAR, TGSTANDALONE, 5},
/*?*/ 	{ "iiint", TIIINT, MS_IIINT, TGOPER, 5},
/*?*/ 	{ "iint", TIINT, MS_IINT, TGOPER, 5},
/*?*/ 	{ "in", TIN, MS_IN, TGRELATION, 0},
/*?*/ 	{ "infinity" , TINFINITY, MS_INFINITY, TGSTANDALONE, 5},
/*?*/ 	{ "infty" , TINFINITY, MS_INFINITY, TGSTANDALONE, 5},
/*?*/ 	{ "int", TINT, MS_INT, TGOPER, 5},
/*?*/ 	{ "intersection", TINTERSECT, MS_INTERSECT, TGPRODUCT, 0},
/*?*/ 	{ "ital", TITALIC, '\0', TGFONTATTR, 5},
/*?*/ 	{ "italic", TITALIC, '\0', TGFONTATTR, 5},
/*?*/ 	{ "lambdabar" , TLAMBDABAR, MS_LAMBDABAR, TGSTANDALONE, 5},
/*?*/ 	{ "langle", TLANGLE, MS_LANGLE, TGLBRACES, 5},
/*?*/ 	{ "lbrace", TLBRACE, MS_LBRACE, TGLBRACES, 5},
/*?*/ 	{ "lceil", TLCEIL, MS_LCEIL, TGLBRACES, 5},
/*?*/ 	{ "ldbracket", TLDBRACKET, MS_LDBRACKET, TGLBRACES, 5},
/*?*/ 	{ "ldline", TLDLINE, MS_DLINE, TGLBRACES, 5},
/*?*/ 	{ "le", TLE, MS_LE, TGRELATION, 0},
/*?*/ 	{ "left", TLEFT, '\0', 0, 5},
/*?*/ 	{ "leftarrow" , TLEFTARROW, MS_LEFTARROW, TGSTANDALONE, 5},
/*?*/ 	{ "leslant", TLESLANT, MS_LESLANT, TGRELATION, 0 },
/*?*/ 	{ "lfloor", TLFLOOR, MS_LFLOOR, TGLBRACES, 5},
/*?*/ 	{ "lim", TLIM, '\0', TGOPER, 5},
/*?*/ 	{ "liminf", TLIMINF, '\0', TGOPER, 5},
/*?*/ 	{ "limsup", TLIMSUP, '\0', TGOPER, 5},
/*?*/ 	{ "lint", TLINT, MS_LINT, TGOPER, 5},
/*?*/ 	{ "ll", TLL, MS_LL, TGRELATION, 0},
/*?*/ 	{ "lline", TLLINE, MS_LINE, TGLBRACES, 5},
/*?*/ 	{ "llint", TLLINT, MS_LLINT, TGOPER, 5},
/*?*/ 	{ "lllint", TLLLINT, MS_LLLINT, TGOPER, 5},
/*?*/ 	{ "ln", TLN, '\0', TGFUNCTION, 5},
/*?*/ 	{ "log", TLOG, '\0', TGFUNCTION, 5},
/*?*/ 	{ "lsub", TLSUB, '\0', TGPOWER, 0},
/*?*/ 	{ "lsup", TLSUP, '\0', TGPOWER, 0},
/*?*/ 	{ "lt", TLT, MS_LT, TGRELATION, 0},
/*?*/ 	{ "magenta", TMAGENTA, '\0', TGCOLOR, 0},
/*?*/ 	{ "matrix", TMATRIX, '\0', 0, 5},
/*?*/ 	{ "minusplus", TMINUSPLUS, MS_MINUSPLUS, TGUNOPER | TGSUM, 5},
/*?*/ 	{ "mline", TMLINE, MS_LINE, 0, 0},		//! nicht in TGRBRACES, Level 0
/*?*/ 	{ "nabla", TNABLA, MS_NABLA, TGSTANDALONE, 5},
/*?*/ 	{ "nbold", TNBOLD, '\0', TGFONTATTR, 5},
/*?*/ 	{ "ndivides", TNDIVIDES, MS_NDIVIDES, TGRELATION, 0},
/*?*/ 	{ "neg", TNEG, MS_NEG, TGUNOPER, 5 },
/*?*/ 	{ "neq", TNEQ, MS_NEQ, TGRELATION, 0},
/*?*/ 	{ "newline", TNEWLINE, '\0', 0, 0},
/*?*/ 	{ "ni", TNI, MS_NI, TGRELATION, 0},
/*?*/ 	{ "nitalic", TNITALIC, '\0', TGFONTATTR, 5},
/*?*/ 	{ "none", TNONE, '\0', TGLBRACES | TGRBRACES, 0},
/*?*/ 	{ "notin", TNOTIN, MS_NOTIN, TGRELATION, 0},
/*?*/ 	{ "nsubset", TNSUBSET, MS_NSUBSET, TGRELATION, 0 },
/*?*/ 	{ "nsupset", TNSUPSET, MS_NSUPSET, TGRELATION, 0 },
/*?*/ 	{ "nsubseteq", TNSUBSETEQ, MS_NSUBSETEQ, TGRELATION, 0 },
/*?*/ 	{ "nsupseteq", TNSUPSETEQ, MS_NSUPSETEQ, TGRELATION, 0 },
/*?*/ 	{ "nroot", TNROOT, MS_SQRT, TGUNOPER, 5},
/*?*/ 	{ "odivide", TODIVIDE, MS_ODIVIDE, TGPRODUCT, 0},
/*?*/ 	{ "odot", TODOT, MS_ODOT, TGPRODUCT, 0},
/*?*/ 	{ "ominus", TOMINUS, MS_OMINUS, TGSUM, 0},
/*?*/ 	{ "oper", TOPER, '\0', TGOPER, 5},
/*?*/ 	{ "oplus", TOPLUS, MS_OPLUS, TGSUM, 0},
/*?*/ 	{ "or", TOR, MS_OR, TGSUM, 0},
/*?*/ 	{ "ortho", TORTHO, MS_ORTHO, TGRELATION, 0},
/*?*/ 	{ "otimes", TOTIMES, MS_OTIMES, TGPRODUCT, 0},
/*?*/ 	{ "over", TOVER, '\0', TGPRODUCT, 0},
/*?*/ 	{ "overbrace", TOVERBRACE, MS_OVERBRACE, TGPRODUCT, 5},
/*?*/ 	{ "overline", TOVERLINE, '\0', TGATTRIBUT, 5},
/*?*/ 	{ "overstrike", TOVERSTRIKE, '\0', TGATTRIBUT, 5},
/*?*/ 	{ "owns", TNI, MS_NI, TGRELATION, 0},
/*?*/ 	{ "parallel", TPARALLEL, MS_DLINE, TGRELATION, 0},
/*?*/ 	{ "partial", TPARTIAL, MS_PARTIAL, TGSTANDALONE, 5 },
/*?*/ 	{ "phantom", TPHANTOM, '\0', TGFONTATTR, 5},
/*?*/ 	{ "plusminus", TPLUSMINUS, MS_PLUSMINUS, TGUNOPER | TGSUM, 5},
/*?*/ 	{ "prod", TPROD, MS_PROD, TGOPER, 5},
/*?*/ 	{ "prop", TPROP, MS_PROP, TGRELATION, 0},
/*?*/ 	{ "rangle", TRANGLE, MS_RANGLE, TGRBRACES, 0},	//! 0 to terminate expression
/*?*/ 	{ "rbrace", TRBRACE, MS_RBRACE, TGRBRACES, 0},	//
/*?*/ 	{ "rceil", TRCEIL, MS_RCEIL, TGRBRACES, 0},	//
/*?*/ 	{ "rdbracket", TRDBRACKET, MS_RDBRACKET, TGRBRACES, 0},	//
/*?*/ 	{ "rdline", TRDLINE, MS_DLINE, TGRBRACES, 0},	//
/*?*/ 	{ "red", TRED, '\0', TGCOLOR, 0},
/*?*/ 	{ "rfloor", TRFLOOR, MS_RFLOOR, TGRBRACES, 0},	//! 0 to terminate expression
/*?*/ 	{ "right", TRIGHT, '\0', 0, 0},
/*?*/ 	{ "rightarrow" , TRIGHTARROW, MS_RIGHTARROW, TGSTANDALONE, 5},
/*?*/ 	{ "rline", TRLINE, MS_LINE, TGRBRACES, 0},	//! 0 to terminate expression
/*?*/ 	{ "rsub", TRSUB, '\0', TGPOWER, 0},
/*?*/ 	{ "rsup", TRSUP, '\0', TGPOWER, 0},
/*?*/ 	{ "sans", TSANS, '\0', TGFONT, 0},
/*?*/ 	{ "serif", TSERIF, '\0', TGFONT, 0},
/*?*/ 	{ "setC" , TSETC, MS_SETC, TGSTANDALONE, 5},
/*?*/ 	{ "setN" , TSETN, MS_SETN, TGSTANDALONE, 5},
/*?*/ 	{ "setQ" , TSETQ, MS_SETQ, TGSTANDALONE, 5},
/*?*/ 	{ "setR" , TSETR, MS_SETR, TGSTANDALONE, 5},
/*?*/ 	{ "setZ" , TSETZ, MS_SETZ, TGSTANDALONE, 5},
/*?*/ 	{ "setminus", TBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
/*?*/ 	{ "sim", TSIM, MS_SIM, TGRELATION, 0},
/*?*/ 	{ "simeq", TSIMEQ, MS_SIMEQ, TGRELATION, 0},
/*?*/ 	{ "sin", TSIN, '\0', TGFUNCTION, 5},
/*?*/ 	{ "sinh", TSINH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "size", TSIZE, '\0', TGFONTATTR, 5},
/*?*/ 	{ "slash", TSLASH, MS_SLASH, TGPRODUCT, 0 },
/*?*/ 	{ "sqrt", TSQRT, MS_SQRT, TGUNOPER, 5},
/*?*/ 	{ "stack", TSTACK, '\0', 0, 5},
/*?*/ 	{ "sub", TRSUB, '\0', TGPOWER, 0},
/*?*/ 	{ "subset", TSUBSET, MS_SUBSET, TGRELATION, 0},
/*?*/ 	{ "subseteq", TSUBSETEQ, MS_SUBSETEQ, TGRELATION, 0},
/*?*/ 	{ "sum", TSUM, MS_SUM, TGOPER, 5},
/*?*/ 	{ "sup", TRSUP, '\0', TGPOWER, 0},
/*?*/ 	{ "supset", TSUPSET, MS_SUPSET, TGRELATION, 0},
/*?*/ 	{ "supseteq", TSUPSETEQ, MS_SUPSETEQ, TGRELATION, 0},
/*?*/ 	{ "tan", TTAN, '\0', TGFUNCTION, 5},
/*?*/ 	{ "tanh", TTANH, '\0', TGFUNCTION, 5},
/*?*/ 	{ "tilde", TTILDE, MS_TILDE, TGATTRIBUT, 5},
/*?*/ 	{ "times", TTIMES, MS_TIMES, TGPRODUCT, 0},
/*?*/ 	{ "to", TTO, '\0', TGLIMIT, 0},
/*?*/ 	{ "toward", TTOWARD, MS_RIGHTARROW, TGRELATION, 0},
/*?*/ 	{ "transl", TTRANSL, MS_TRANSL, TGRELATION, 0},
/*?*/ 	{ "transr", TTRANSR, MS_TRANSR, TGRELATION, 0},
/*?*/ 	{ "underbrace", TUNDERBRACE, MS_UNDERBRACE, TGPRODUCT, 5},
/*?*/ 	{ "underline", TUNDERLINE, '\0', TGATTRIBUT, 5},
/*?*/ 	{ "union", TUNION, MS_UNION, TGSUM, 0},
/*?*/ 	{ "uoper", TUOPER, '\0', TGUNOPER, 5},
/*?*/ 	{ "uparrow" , TUPARROW, MS_UPARROW, TGSTANDALONE, 5},
/*?*/ 	{ "vec", TVEC, MS_VEC, TGATTRIBUT, 5},
/*?*/ 	{ "white", TWHITE, '\0', TGCOLOR, 0},
/*?*/ 	{ "widebslash", TWIDEBACKSLASH, MS_BACKSLASH, TGPRODUCT, 0 },
/*?*/ 	{ "widehat", TWIDEHAT, MS_HAT, TGATTRIBUT, 5},
/*?*/ 	{ "widetilde", TWIDETILDE, MS_TILDE, TGATTRIBUT, 5},
/*?*/ 	{ "wideslash", TWIDESLASH, MS_SLASH, TGPRODUCT, 0 },
/*?*/ 	{ "widevec", TWIDEVEC, MS_VEC, TGATTRIBUT, 5},
/*?*/ 	{ "wp" , TWP, MS_WP, TGSTANDALONE, 5},
/*?*/ 	{ "yellow", TYELLOW, '\0', TGCOLOR, 0},
/*?*/ //	{ "[", TLBRACKET, MS_LBRACKET, TGLBRACES, 5},	//! 5 to continue expression
/*?*/ //	{ "\\", TESCAPE, '\0', 0, 5},
/*?*/ //	{ "]", TRBRACKET, MS_RBRACKET, TGRBRACES, 0},	//! 0 to terminate expression
/*?*/ //	{ "^", TRSUP, '\0', TGPOWER, 0},
/*?*/ //	{ "_", TRSUB, '\0', TGPOWER, 0},
/*?*/ //	{ "`", TSBLANK, '\0', TGBLANK, 5},
/*?*/ //	{ "{", TLGROUP, MS_LBRACE, 0, 5},		//! 5 to continue expression
/*?*/ //	{ "|", TOR, MS_OR, TGSUM, 0},
/*?*/ //	{ "}", TRGROUP, MS_RBRACE, 0, 0},		//! 0 to terminate expression
/*?*/ //	{ "~", TBLANK, '\0', TGBLANK, 5},
/*?*/ 	{ "", TEND, '\0', 0, 0}
/*?*/ };


/*N*/ static const SmTokenTableEntry * GetTokenTableEntry( const String &rName )
/*N*/ {
/*N*/ 	const SmTokenTableEntry * pRes = 0;
/*N*/ 	if (rName.Len())
/*N*/ 	{
/*N*/ 		INT32 nEntries = SAL_N_ELEMENTS( aTokenTable );
/*N*/ 		for (INT32 i = 0;  i < nEntries;  ++i)
/*N*/ 		{
/*N*/ 			if (rName.EqualsIgnoreCaseAscii( aTokenTable[i].pIdent ))
/*N*/ 			{
/*N*/ 				pRes = &aTokenTable[i];
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	return pRes;
/*N*/ }


///////////////////////////////////////////////////////////////////////////

#if OSL_DEBUG_LEVEL > 1

/*N*/ BOOL SmParser::IsDelimiter( const String &rTxt, xub_StrLen nPos )
/*N*/ 	// returns 'TRUE' iff cChar is '\0' or a delimeter
/*N*/ {
/*N*/ 	DBG_ASSERT( nPos <= rTxt.Len(), "index out of range" );
/*N*/ 
/*N*/ 	sal_Unicode cChar = rTxt.GetChar( nPos );
/*N*/ 	if(!cChar)
/*N*/ 		return TRUE;
/*N*/ 
/*N*/ 	// check if 'cChar' is in the delimeter table
/*N*/ 	const sal_Unicode *pDelim = &aDelimiterTable[0];
/*N*/ 	for ( ;  *pDelim != 0;  pDelim++)
/*N*/ 		if (*pDelim == cChar)
/*N*/ 			break;
/*N*/ 
/*N*/ 	BOOL bIsDelim = *pDelim != 0;
/*N*/ 
/*N*/ 	INT16 nTypJp = SM_MOD1()->GetSysLocale().GetCharClass().getType( rTxt, nPos );
/*N*/ 	bIsDelim |= nTypJp == ::com::sun::star::i18n::UnicodeType::SPACE_SEPARATOR ||
/*N*/ 				nTypJp == ::com::sun::star::i18n::UnicodeType::CONTROL;
/*N*/ 
/*N*/ 	return bIsDelim;
/*N*/ }

#endif 


/*N*/ void SmParser::Insert(const String &rText, USHORT nPos)
/*N*/ {
/*N*/ 	BufferString.Insert(rText, nPos);
/*N*/ 
/*N*/ 	xub_StrLen  nLen = rText.Len();
/*N*/ 	BufferIndex += nLen;
/*N*/ 	nTokenIndex += nLen;
/*N*/ }


/*N*/ void SmParser::Replace( USHORT nPos, USHORT nLen, const String &rText )
/*N*/ {
/*N*/     DBG_ASSERT( nPos + nLen <= BufferString.Len(), "argument mismatch" );
/*N*/ 
/*N*/     BufferString.Replace( nPos, nLen, rText );
/*N*/     INT16  nChg = rText.Len() - nLen;
/*N*/     BufferIndex += nChg;
/*N*/     nTokenIndex += nChg;
/*N*/ }


// First character may be any alphabetic
/*?*/ const sal_Int32 coStartFlags =
/*?*/ 		KParseTokens::ANY_LETTER_OR_NUMBER |
/*?*/ 		KParseTokens::IGNORE_LEADING_WS;

// Continuing characters may be any alphanumeric or dot.
/*?*/ const sal_Int32 coContFlags =
/*?*/     ( coStartFlags | KParseTokens::ASC_DOT ) & ~KParseTokens::IGNORE_LEADING_WS
/*?*/     | KParseTokens::TWO_DOUBLE_QUOTES_BREAK_STRING;

// First character for numbers, may be any numeric or dot
const sal_Int32 coNumStartFlags =
        KParseTokens::ASC_DIGIT |
        KParseTokens::ASC_DOT |
        KParseTokens::IGNORE_LEADING_WS;
// Continuing characters for numbers, may be any numeric or dot.
const sal_Int32 coNumContFlags =
    ( coNumStartFlags | KParseTokens::ASC_DOT ) & ~KParseTokens::IGNORE_LEADING_WS;

/*N*/ void SmParser::NextToken()
/*N*/ {
/*N*/ 	static const String aEmptyStr;
/*N*/ 
/*N*/ 	xub_StrLen	nBufLen = BufferString.Len();
/*N*/ 	ParseResult	aRes;
/*N*/ 	xub_StrLen	nRealStart;
/*N*/ 	BOOL		bCont;
/*N*/     BOOL        bNumStart;
/*N*/ 	const CharClass& rCC = SM_MOD1()->GetSysLocale().GetCharClass();
/*N*/ 	do
/*N*/ 	{
/*N*/         // skip white spaces
/*N*/         while (UnicodeType::SPACE_SEPARATOR ==
/*N*/                         rCC.getType( BufferString, BufferIndex ))
/*N*/            ++BufferIndex;
/*N*/ 
/*N*/         sal_Int32 nStartFlags = coStartFlags;
/*N*/         sal_Int32 nContFlags  = coContFlags;
/*N*/         sal_Unicode cFirstChar = BufferString.GetChar( BufferIndex );
/*N*/         bNumStart = cFirstChar == '.' || ('0' <= cFirstChar && cFirstChar <= '9');
/*N*/         if (bNumStart)
/*N*/         {
/*N*/             nStartFlags = coNumStartFlags;
/*N*/             nContFlags  = coNumContFlags;
/*N*/         }
/*N*/ 
/*N*/ 		aRes = rCC.parseAnyToken( BufferString, BufferIndex,
/*N*/                                             nStartFlags, aEmptyStr,
/*N*/                                             nContFlags, aEmptyStr );
/*N*/ 
/*N*/ 		nRealStart = BufferIndex + (xub_StrLen) aRes.LeadingWhiteSpace;
/*N*/         BufferIndex = nRealStart;
/*N*/ 
/*N*/ 		bCont = FALSE;
/*N*/ 		if ( aRes.TokenType == 0  &&
/*N*/ 				nRealStart < nBufLen &&
/*N*/ 				'\n' == BufferString.GetChar( nRealStart ) )
/*N*/ 		{
/*N*/ 			// keep data needed for tokens row and col entry up to date
/*N*/ 			++Row;
/*N*/ 			BufferIndex = ColOff = nRealStart + 1;
/*N*/ 			bCont = TRUE;
/*N*/ 		}
/*N*/ 		else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
/*N*/ 		{
/*N*/ 			String aName( BufferString.Copy( nRealStart, 2 ));
/*N*/ 			if ( aName.EqualsAscii( "%%" ))
/*N*/ 			{
/*N*/ 				//SkipComment
/*N*/ 				BufferIndex = nRealStart + 2;
/*N*/ 				while (BufferIndex < nBufLen  &&
/*N*/ 					'\n' != BufferString.GetChar( BufferIndex ))
/*N*/ 					++BufferIndex;
/*N*/ 				bCont = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 	} while (bCont);
/*N*/ 
/*N*/ 	// set index of current token
/*N*/ 	nTokenIndex = BufferIndex;
/*N*/ 
/*N*/ 	CurToken.nRow	= Row;
/*N*/ 	CurToken.nCol	= nRealStart - ColOff + 1;
/*N*/ 
/*N*/ 	BOOL bHandled = TRUE;
/*N*/ 	if (nRealStart >= nBufLen)
/*N*/ 	{
/*N*/ 		CurToken.eType	   = TEND;
/*N*/ 		CurToken.cMathChar = '\0';
/*N*/ 		CurToken.nGroup	   = 0;
/*N*/ 		CurToken.nLevel	   = 0;
/*N*/ 		CurToken.aText.Erase();
/*N*/ 	}
/*N*/     else if ((aRes.TokenType & (KParseType::ASC_NUMBER | KParseType::UNI_NUMBER))
/*N*/              || (bNumStart && (aRes.TokenType & KParseType::IDENTNAME)))
/*N*/ 	{
/*N*/ 		INT32 n = aRes.EndPos - nRealStart;
/*N*/ 		DBG_ASSERT( n >= 0, "length < 0" );
/*N*/ 		CurToken.eType      = TNUMBER;
/*N*/ 		CurToken.cMathChar  = '\0';
/*N*/ 		CurToken.nGroup     = 0;
/*N*/ 		CurToken.nLevel     = 5;
/*N*/ 		CurToken.aText      = BufferString.Copy( nRealStart, (xub_StrLen) n );
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/             if (!IsDelimiter( BufferString, aRes.EndPos ))
/*N*/                 DBG_WARNING( "identifier really finished? (compatibility!)" );
/*N*/ #endif
/*N*/ 	}
/*N*/ 	else if (aRes.TokenType & KParseType::DOUBLE_QUOTE_STRING)
/*N*/ 	{
/*N*/ 		CurToken.eType      = TTEXT;
/*N*/ 		CurToken.cMathChar  = '\0';
/*N*/ 		CurToken.nGroup     = 0;
/*N*/ 		CurToken.nLevel     = 5;
/*N*/ 		CurToken.aText		= aRes.DequotedNameOrString;
/*N*/ 		CurToken.nRow       = Row;
/*N*/ 		CurToken.nCol       = nRealStart - ColOff + 2;
/*N*/ 	}
/*N*/ 	else if (aRes.TokenType & KParseType::IDENTNAME)
/*N*/ 	{
/*N*/ 		INT32 n = aRes.EndPos - nRealStart;
/*N*/ 		DBG_ASSERT( n >= 0, "length < 0" );
/*N*/ 		String aName( BufferString.Copy( nRealStart, (xub_StrLen) n ) );
/*N*/ 		const SmTokenTableEntry *pEntry = GetTokenTableEntry( aName );
/*N*/ 
/*N*/ 		if (pEntry)
/*N*/ 		{
/*N*/ 			CurToken.eType      = pEntry->eType;
/*N*/ 			CurToken.cMathChar  = pEntry->cMathChar;
/*N*/ 			CurToken.nGroup     = pEntry->nGroup;
/*N*/ 			CurToken.nLevel     = pEntry->nLevel;
/*N*/ 			CurToken.aText.AssignAscii( pEntry->pIdent );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			CurToken.eType      = TIDENT;
/*N*/ 			CurToken.cMathChar  = '\0';
/*N*/ 			CurToken.nGroup     = 0;
/*N*/ 			CurToken.nLevel     = 5;
/*N*/ 			CurToken.aText      = aName;
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/             if (!IsDelimiter( BufferString, aRes.EndPos ))
/*N*/                 DBG_WARNING( "identifier really finished? (compatibility!)" );
/*N*/ #endif
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (aRes.TokenType == 0  &&  '_' == BufferString.GetChar( nRealStart ))
/*N*/ 	{
/*N*/ 		CurToken.eType	   = TRSUB;
/*N*/ 		CurToken.cMathChar = '\0';
/*N*/ 		CurToken.nGroup	   = TGPOWER;
/*N*/ 		CurToken.nLevel	   = 0;
/*N*/ 		CurToken.aText.AssignAscii( "_" );
/*N*/ 
/*N*/ 		aRes.EndPos = nRealStart + 1;
/*N*/ 	}
/*N*/ 	else if (aRes.TokenType & KParseType::BOOLEAN)
/*N*/ 	{
/*N*/ 		sal_Int32   &rnEndPos = aRes.EndPos;
/*N*/ 		String	aName( BufferString.Copy( nRealStart, rnEndPos - nRealStart ) );
/*N*/ 		if (2 >= aName.Len())
/*N*/ 		{
/*N*/ 			sal_Unicode ch = aName.GetChar( 0 );
/*N*/ 			switch (ch)
/*N*/ 			{
/*N*/ 				case '<':
/*N*/ 					{
/*N*/ 						if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "<<" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TLL;
/*N*/ 							CurToken.cMathChar = MS_LL;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "<<" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "<=" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TLE;
/*N*/ 							CurToken.cMathChar = MS_LE;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "<=" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "<>" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TNEQ;
/*N*/ 							CurToken.cMathChar = MS_NEQ;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "<>" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else if (BufferString.Copy( nRealStart, 3 ).
/*N*/ 								EqualsAscii( "<?>" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TPLACE;
/*N*/ 							CurToken.cMathChar = MS_PLACE;
/*N*/ 							CurToken.nGroup	   = 0;
/*N*/ 							CurToken.nLevel	   = 5;
/*N*/ 							CurToken.aText.AssignAscii( "<?>" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 3;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TLT;
/*N*/ 							CurToken.cMathChar = MS_LT;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "<" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '>':
/*N*/ 					{
/*N*/ 						if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( ">=" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TGE;
/*N*/ 							CurToken.cMathChar = MS_GE;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( ">=" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( ">>" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TGG;
/*N*/ 							CurToken.cMathChar = MS_GG;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( ">>" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TGT;
/*N*/ 							CurToken.cMathChar = MS_GT;
/*N*/ 							CurToken.nGroup	   = TGRELATION;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( ">" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				default:
/*N*/ 					bHandled = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if (aRes.TokenType & KParseType::ONE_SINGLE_CHAR)
/*N*/ 	{
/*N*/ 		sal_Int32 &rnEndPos = aRes.EndPos;
/*N*/ 		String	aName( BufferString.Copy( nRealStart, rnEndPos - nRealStart ) );
/*N*/ 
/*N*/ 		if (1 == aName.Len())
/*N*/ 		{
/*N*/ 			sal_Unicode ch = aName.GetChar( 0 );
/*N*/ 			switch (ch)
/*N*/ 			{
/*N*/ 				case '%':
/*N*/ 					{
/*N*/ 						//! modifies aRes.EndPos
/*N*/ 
/*N*/ 						DBG_ASSERT( rnEndPos >= nBufLen  ||
/*N*/ 									'%' != BufferString.GetChar( rnEndPos ),
/*N*/ 								"unexpected comment start" );
/*N*/ 
/*N*/ 						// get identifier of user-defined character
/*N*/ 						ParseResult aTmpRes = rCC.parseAnyToken(
/*N*/ 								BufferString, rnEndPos,
/*N*/ 								KParseTokens::ANY_LETTER,
/*N*/ 								aEmptyStr,
/*N*/                                 coContFlags,
/*N*/ 								aEmptyStr );
/*N*/ 
/*N*/                         xub_StrLen nTmpStart = rnEndPos +
/*N*/                                 (xub_StrLen) aTmpRes.LeadingWhiteSpace;
/*N*/ 
/*N*/                         // default setting fo the case that no identifier
/*N*/                         // i.e. a valid symbol-name is following the '%'
/*N*/                         // character
/*N*/                         CurToken.eType      = TTEXT;
/*N*/                         CurToken.cMathChar  = '\0';
/*N*/                         CurToken.nGroup     = 0;
/*N*/                         CurToken.nLevel     = 5;
/*N*/                         CurToken.aText      = String();
/*N*/                         CurToken.nRow       = Row;
/*N*/                         CurToken.nCol       = nTmpStart - ColOff + 1;
/*N*/ 
/*N*/                         if (aTmpRes.TokenType & KParseType::IDENTNAME)
/*N*/                         {
/*N*/ 
/*N*/                             INT32 n = aTmpRes.EndPos - nTmpStart;
/*N*/                             CurToken.eType      = TSPECIAL;
/*N*/                             CurToken.aText      = BufferString.Copy( nTmpStart, n );
/*N*/ 
/*N*/                             DBG_ASSERT( aTmpRes.EndPos > rnEndPos,
/*N*/                                     "empty identifier" );
/*N*/                             if (aTmpRes.EndPos > rnEndPos)
/*N*/                                 rnEndPos = aTmpRes.EndPos;
/*N*/                             else
/*N*/                                 ++rnEndPos;
/*N*/                         }
/*N*/ 
/*N*/                         // if no symbol-name was found we start-over with
/*N*/                         // finding the next token right afer the '%' sign.
/*N*/                         // I.e. we leave rnEndPos unmodified.
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '[':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TLBRACKET;
/*N*/ 						CurToken.cMathChar = MS_LBRACKET;
/*N*/ 						CurToken.nGroup	   = TGLBRACES;
/*N*/ 						CurToken.nLevel	   = 5;
/*N*/ 						CurToken.aText.AssignAscii( "[" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '\\':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TESCAPE;
/*N*/ 						CurToken.cMathChar = '\0';
/*N*/ 						CurToken.nGroup	   = 0;
/*N*/ 						CurToken.nLevel	   = 5;
/*N*/ 						CurToken.aText.AssignAscii( "\\" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case ']':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TRBRACKET;
/*N*/ 						CurToken.cMathChar = MS_RBRACKET;
/*N*/ 						CurToken.nGroup	   = TGRBRACES;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "]" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '^':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TRSUP;
/*N*/ 						CurToken.cMathChar = '\0';
/*N*/ 						CurToken.nGroup	   = TGPOWER;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "^" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '`':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TSBLANK;
/*N*/ 						CurToken.cMathChar = '\0';
/*N*/ 						CurToken.nGroup	   = TGBLANK;
/*N*/ 						CurToken.nLevel	   = 5;
/*N*/ 						CurToken.aText.AssignAscii( "`" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '{':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TLGROUP;
/*N*/ 						CurToken.cMathChar = MS_LBRACE;
/*N*/ 						CurToken.nGroup	   = 0;
/*N*/ 						CurToken.nLevel	   = 5;
/*N*/ 						CurToken.aText.AssignAscii( "{" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '|':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TOR;
/*N*/ 						CurToken.cMathChar = MS_OR;
/*N*/ 						CurToken.nGroup	   = TGSUM;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "|" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '}':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TRGROUP;
/*N*/ 						CurToken.cMathChar = MS_RBRACE;
/*N*/ 						CurToken.nGroup	   = 0;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "}" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '~':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TBLANK;
/*N*/ 						CurToken.cMathChar = '\0';
/*N*/ 						CurToken.nGroup	   = TGBLANK;
/*N*/ 						CurToken.nLevel	   = 5;
/*N*/ 						CurToken.aText.AssignAscii( "~" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '#':
/*N*/ 					{
/*N*/ 						if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "##" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TDPOUND;
/*N*/ 							CurToken.cMathChar = '\0';
/*N*/ 							CurToken.nGroup	   = 0;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "##" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TPOUND;
/*N*/ 							CurToken.cMathChar = '\0';
/*N*/ 							CurToken.nGroup	   = 0;
/*N*/ 							CurToken.nLevel	   = 0;
/*N*/ 							CurToken.aText.AssignAscii( "#" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '&':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TAND;
/*N*/ 						CurToken.cMathChar = MS_AND;
/*N*/ 						CurToken.nGroup	   = TGPRODUCT;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "&" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '(':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TLPARENT;
/*N*/ 						CurToken.cMathChar = MS_LPARENT;
/*N*/ 						CurToken.nGroup	   = TGLBRACES;
/*N*/ 						CurToken.nLevel	   = 5;		//! 0 to continue expression
/*N*/ 						CurToken.aText.AssignAscii( "(" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case ')':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TRPARENT;
/*N*/ 						CurToken.cMathChar = MS_RPARENT;
/*N*/ 						CurToken.nGroup	   = TGRBRACES;
/*N*/ 						CurToken.nLevel	   = 0;		//! 0 to terminate expression
/*N*/ 						CurToken.aText.AssignAscii( ")" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '*':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TMULTIPLY;
/*N*/ 						CurToken.cMathChar = MS_MULTIPLY;
/*N*/ 						CurToken.nGroup	   = TGPRODUCT;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "*" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '+':
/*N*/ 					{
/*N*/ 						if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "+-" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TPLUSMINUS;
/*N*/ 							CurToken.cMathChar = MS_PLUSMINUS;
/*N*/ 							CurToken.nGroup	   = TGUNOPER | TGSUM;
/*N*/ 							CurToken.nLevel	   = 5;
/*N*/ 							CurToken.aText.AssignAscii( "+-" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TPLUS;
/*N*/ 							CurToken.cMathChar = MS_PLUS;
/*N*/ 							CurToken.nGroup	   = TGUNOPER | TGSUM;
/*N*/ 							CurToken.nLevel	   = 5;
/*N*/ 							CurToken.aText.AssignAscii( "+" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '-':
/*N*/ 					{
/*N*/ 						if (BufferString.Copy( nRealStart, 2 ).
/*N*/ 								EqualsAscii( "-+" ))
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TMINUSPLUS;
/*N*/ 							CurToken.cMathChar = MS_MINUSPLUS;
/*N*/ 							CurToken.nGroup	   = TGUNOPER | TGSUM;
/*N*/ 							CurToken.nLevel	   = 5;
/*N*/ 							CurToken.aText.AssignAscii( "-+" );
/*N*/ 
/*N*/ 							rnEndPos = nRealStart + 2;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							CurToken.eType	   = TMINUS;
/*N*/ 							CurToken.cMathChar = MS_MINUS;
/*N*/ 							CurToken.nGroup	   = TGUNOPER | TGSUM;
/*N*/ 							CurToken.nLevel	   = 5;
/*N*/ 							CurToken.aText.AssignAscii( "-" );
/*N*/ 						}
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '.':
/*N*/ 					{
/*N*/                         // for compatibility with SO5.2
/*N*/                         // texts like .34 ...56 ... h ...78..90
/*N*/                         // will be treated as numbers
/*N*/                         CurToken.eType     = TNUMBER;
/*N*/ 						CurToken.cMathChar = '\0';
/*N*/ 						CurToken.nGroup	   = 0;
/*N*/                         CurToken.nLevel    = 5;
/*N*/ 
/*N*/                         xub_StrLen nTxtStart = BufferIndex;
/*N*/                         sal_Unicode cChar;
/*N*/                         do
/*N*/                         {
/*N*/                             cChar = BufferString.GetChar( ++BufferIndex );
/*N*/                         }
/*N*/                         while ( cChar == '.' || ('0' <= cChar && cChar <= '9') );
/*N*/ 
/*N*/                         CurToken.aText = BufferString.Copy( nTxtStart, BufferIndex - nTxtStart );
/*N*/                         aRes.EndPos = BufferIndex;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '/':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TDIVIDEBY;
/*N*/ 						CurToken.cMathChar = MS_SLASH;
/*N*/ 						CurToken.nGroup	   = TGPRODUCT;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "/" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case '=':
/*N*/ 					{
/*N*/ 						CurToken.eType	   = TASSIGN;
/*N*/ 						CurToken.cMathChar = MS_ASSIGN;
/*N*/ 						CurToken.nGroup	   = TGRELATION;
/*N*/ 						CurToken.nLevel	   = 0;
/*N*/ 						CurToken.aText.AssignAscii( "=" );
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				default:
/*N*/ 					bHandled = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bHandled = FALSE;
/*N*/ 
/*N*/ 	if (!bHandled)
/*N*/ 	{
/*N*/ 		CurToken.eType      = TCHARACTER;
/*N*/ 		CurToken.cMathChar  = '\0';
/*N*/ 		CurToken.nGroup     = 0;
/*N*/ 		CurToken.nLevel     = 5;
/*N*/ 		CurToken.aText      = BufferString.Copy( nRealStart, 1 );
/*N*/ 
/*N*/ 		aRes.EndPos = nRealStart + 1;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (TEND != CurToken.eType)
/*N*/ 		BufferIndex = (xub_StrLen)aRes.EndPos;
/*N*/ }
/*N*/ 
/*N*/ 
////////////////////////////////////////
// grammar
//


/*N*/ void SmParser::Table()
/*N*/ {
/*N*/ 	SmNodeArray  LineArray;
/*N*/ 
/*N*/ 	Line();
/*N*/ 	while (CurToken.eType == TNEWLINE)
/*N*/ 	{
/*N*/ 		NextToken();
/*N*/ 		Line();
/*N*/ 	}
/*N*/ 
/*N*/ 	if (CurToken.eType != TEND)
/*?*/ 		Error(PE_UNEXPECTED_CHAR);
/*N*/ 
/*N*/ 	ULONG n = NodeStack.Count();
/*N*/ 
/*N*/ 	LineArray.SetSize(n);
/*N*/ 
/*N*/ 	for (ULONG i = 0; i < n; i++)
/*N*/ 		LineArray.Put(n - (i + 1), NodeStack.Pop());
/*N*/ 
/*N*/ 	SmStructureNode *pSNode = new SmTableNode(CurToken);
/*N*/ 	pSNode->SetSubNodes(LineArray);
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::Align()
/*N*/ 	// parse alignment info (if any), then go on with rest of expression
/*N*/ {
/*N*/ 	SmStructureNode *pSNode = 0;
/*N*/ 	BOOL    bNeedGroupClose = FALSE;
/*N*/ 
/*N*/ 	if (TokenInGroup(TGALIGN))
/*N*/ 	{
/*?*/         if (CONVERT_40_TO_50 == GetConversion())
/*?*/ 			// encapsulate expression to be aligned in group braces
/*?*/ 			// (here group-open brace)
/*?*/ 		{	Insert('{', GetTokenIndex());
/*?*/ 			bNeedGroupClose = TRUE;
/*?*/ 
/*?*/ 			// get first valid align statement in sequence
/*?*/ 			// (the dominant one in 4.0) and erase all others (especially old
/*?*/ 			// discarded tokens) from command string.
/*?*/ 			while (TokenInGroup(TGALIGN))
/*?*/ 			{	if (TokenInGroup(TGDISCARDED) || pSNode)
/*?*/ 				{   BufferIndex = GetTokenIndex();
/*?*/ 					BufferString.Erase(BufferIndex, CurToken.aText.Len());
/*?*/ 				}
/*?*/ 				else
/*?*/                   pSNode = new SmAlignNode(CurToken);
/*?*/ 
/*?*/ 				NextToken();
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/           pSNode = new SmAlignNode(CurToken);
/*?*/ 
/*?*/           NextToken();
/*?*/ 
/*?*/           // allow for just one align statement in 5.0
/*?*/             if (CONVERT_40_TO_50 != GetConversion() && TokenInGroup(TGALIGN))
/*?*/           {   Error(PE_DOUBLE_ALIGN);
/*?*/               return;
/*?*/           }
/*?*/ 		}
/*?*/ 	}
/*N*/ 
/*N*/ 	Expression();
/*N*/ 
/*N*/ 	if (bNeedGroupClose)
/*?*/ 		Insert('}', GetTokenIndex());
/*N*/ 
/*N*/ 	if (pSNode)
/*?*/ 	{	pSNode->SetSubNodes(NodeStack.Pop(), 0);
/*?*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Line()
/*N*/ {
/*N*/ 	USHORT  n = 0;
/*N*/ 	SmNodeArray  ExpressionArray;
/*N*/ 
/*N*/ 	ExpressionArray.SetSize(n);
/*N*/ 
/*N*/ 	// start with single expression that may have an alignment statement
/*N*/ 	// (and go on with expressions that must not have alignment
/*N*/ 	// statements in 'while' loop below. See also 'Expression()'.)
/*N*/ 	if (CurToken.eType != TEND  &&  CurToken.eType != TNEWLINE)
/*N*/ 	{	Align();
/*N*/ 		ExpressionArray.SetSize(++n);
/*N*/ 		ExpressionArray.Put(n - 1, NodeStack.Pop());
/*N*/ 	}
/*N*/ 
/*N*/ 	while (CurToken.eType != TEND  &&  CurToken.eType != TNEWLINE)
/*N*/     {   if (CONVERT_40_TO_50 != GetConversion())
/*N*/ 			Expression();
/*N*/ 		else
/*?*/ 			Align();
/*N*/ 		ExpressionArray.SetSize(++n);
/*N*/ 		ExpressionArray.Put(n - 1, NodeStack.Pop());
/*N*/ 	}
/*N*/ 
/*N*/ 	SmStructureNode *pSNode = new SmLineNode(CurToken);
/*N*/ 	pSNode->SetSubNodes(ExpressionArray);
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::Expression()
/*N*/ {
/*N*/ 	USHORT		 n = 0;
/*N*/ 	SmNodeArray  RelationArray;
/*N*/ 
/*N*/ 	RelationArray.SetSize(n);
/*N*/ 
/*N*/ 	Relation();
/*N*/ 	RelationArray.SetSize(++n);
/*N*/ 	RelationArray.Put(n - 1, NodeStack.Pop());
/*N*/ 
/*N*/ 	while (CurToken.nLevel >= 4)
/*N*/ 	{	Relation();
/*N*/ 		RelationArray.SetSize(++n);
/*N*/ 		RelationArray.Put(n - 1, NodeStack.Pop());
/*N*/ 	}
/*N*/ 
/*N*/ 	SmStructureNode *pSNode = new SmExpressionNode(CurToken);
/*N*/ 	pSNode->SetSubNodes(RelationArray);
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::Relation()
/*N*/ {
/*N*/ 	Sum();
/*N*/ 	while (TokenInGroup(TGRELATION))
/*N*/ 	{
/*N*/ 		SmStructureNode *pSNode  = new SmBinHorNode(CurToken);
/*N*/ 		SmNode *pFirst = NodeStack.Pop();
/*N*/ 
/*N*/ 		OpSubSup();
/*N*/ 		SmNode *pSecond = NodeStack.Pop();
/*N*/ 
/*N*/ 		Sum();
/*N*/ 
/*N*/ 		pSNode->SetSubNodes(pFirst, pSecond, NodeStack.Pop());
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Sum()
/*N*/ {
/*N*/ 	Product();
/*N*/ 	while (TokenInGroup(TGSUM))
/*N*/ 	{
/*N*/ 		SmStructureNode *pSNode  = new SmBinHorNode(CurToken);
/*N*/ 		SmNode *pFirst = NodeStack.Pop();
/*N*/ 
/*N*/ 		OpSubSup();
/*N*/ 		SmNode *pSecond = NodeStack.Pop();
/*N*/ 
/*N*/ 		Product();
/*N*/ 
/*N*/ 		pSNode->SetSubNodes(pFirst, pSecond, NodeStack.Pop());
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Product()
/*N*/ {
/*N*/ 	Power();
/*N*/ 
/*N*/ 	while (TokenInGroup(TGPRODUCT))
/*N*/ 	{	SmStructureNode *pSNode;
/*N*/ 		SmNode *pFirst = NodeStack.Pop(),
/*N*/ 			   *pOper;
/*N*/ 		BOOL bSwitchArgs = FALSE;
/*N*/ 
/*N*/ 		SmTokenType eType = CurToken.eType;
/*N*/ 		switch (eType)
/*N*/ 		{
/*N*/ 			case TOVER:
/*N*/ 				pSNode = new SmBinVerNode(CurToken);
/*N*/ 				pOper = new SmRectangleNode(CurToken);
/*N*/ 				NextToken();
/*N*/ 				break;
/*N*/ 
/*N*/ 			case TBOPER:
/*?*/ 				pSNode = new SmBinHorNode(CurToken);
/*?*/ 
/*?*/ 				NextToken();
/*?*/ 
/*?*/ 				GlyphSpecial();
/*?*/ 				pOper = NodeStack.Pop();
/*?*/ 				break;
/*N*/ 
/*N*/ 			case TOVERBRACE :
/*N*/ 			case TUNDERBRACE :
/*?*/               pSNode = new SmVerticalBraceNode(CurToken);
/*?*/               pOper = new SmMathSymbolNode(CurToken);
/*?*/               NextToken();
/*?*/ 				break;
/*?*/ 
/*?*/ 			case TWIDEBACKSLASH:
/*?*/ 			case TWIDESLASH:
/*?*/ 			{
/*?*/               SmBinDiagonalNode *pSTmp = new SmBinDiagonalNode(CurToken);
/*?*/               pSTmp->SetAscending(eType == TWIDESLASH);
/*?*/               pSNode = pSTmp;
/*?*/ 
/*?*/               pOper = new SmPolyLineNode(CurToken);
/*?*/               NextToken();
/*?*/ 
/*?*/               bSwitchArgs =TRUE;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 
/*N*/ 			default:
/*N*/ 				pSNode = new SmBinHorNode(CurToken);
/*N*/ 
/*N*/ 				OpSubSup();
/*N*/ 				pOper = NodeStack.Pop();
/*N*/ 		}
/*N*/ 
/*N*/ 		Power();
/*N*/ 
/*N*/ 		if (bSwitchArgs)
/*N*/ 			//! vgl siehe SmBinDiagonalNode::Arrange
/*?*/ 			pSNode->SetSubNodes(pFirst, NodeStack.Pop(), pOper);
/*N*/ 		else
/*N*/ 			pSNode->SetSubNodes(pFirst, pOper, NodeStack.Pop());
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::SubSup(ULONG nActiveGroup)
/*N*/ {
/*N*/ 	DBG_ASSERT(nActiveGroup == TGPOWER  ||  nActiveGroup == TGLIMIT,
/*N*/ 			   "Sm: falsche Tokengruppe");
/*N*/ 
/*N*/ 	if (!TokenInGroup(nActiveGroup))
/*N*/ 		// already finish
/*N*/ 		return;
/*N*/ 
/*N*/ 	SmSubSupNode *pNode = new SmSubSupNode(CurToken);
/*N*/ 	//! Of course 'CurToken' ist just the first sub-/supscript token.
/*N*/ 	//! It should be of no further interest. The positions of the
/*N*/ 	//! sub-/supscripts will be identified by the corresponding subnodes
/*N*/ 	//! index in the 'aSubNodes' array (enum value from 'SmSubSup').
/*N*/ 
/*N*/ 	pNode->SetUseLimits(nActiveGroup == TGLIMIT);
/*N*/ 
/*N*/ 	// initialize subnodes array
/*N*/ 	SmNodeArray  aSubNodes;
/*N*/ 	aSubNodes.SetSize(1 + SUBSUP_NUM_ENTRIES);
/*N*/ 	aSubNodes.Put(0, NodeStack.Pop());
/*N*/ 	for (USHORT i = 1;  i < aSubNodes.GetSize();  i++)
/*N*/ 		aSubNodes.Put(i, NULL);
/*N*/ 
/*N*/ 	// process all sub-/supscripts
/*N*/ 	int  nIndex = 0;
/*N*/ 	while (TokenInGroup(nActiveGroup))
/*N*/ 	{	SmTokenType  eType (CurToken.eType);
/*N*/ 
/*N*/ 		// skip sub-/supscript token
/*N*/ 		NextToken();
/*N*/ 
/*N*/ 		// get sub-/supscript node on top of stack
/*N*/ 		if (eType == TFROM  ||  eType == TTO)
/*N*/ 		{
/*N*/ 			// parse limits in old 4.0 and 5.0 style
/*N*/ 			Relation();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			Term();
/*N*/ 
/*N*/ 		switch (eType)
/*N*/ 		{	case TRSUB :	nIndex = (int) RSUB;	break;
/*N*/ 			case TRSUP :	nIndex = (int) RSUP;	break;
/*N*/ 			case TFROM :
/*N*/ 			case TCSUB :	nIndex = (int) CSUB;	break;
/*N*/ 			case TTO :
/*N*/ 			case TCSUP :	nIndex = (int) CSUP;	break;
/*N*/ 			case TLSUB :	nIndex = (int) LSUB;	break;
/*N*/ 			case TLSUP :	nIndex = (int) LSUP;	break;
/*N*/ 			default :
/*N*/ 				DBG_ASSERT(FALSE, "Sm: unbekannter Fall");
/*N*/ 		}
/*N*/ 		nIndex++;
/*N*/ 		DBG_ASSERT(1 <= nIndex	&&	nIndex <= 1 + SUBSUP_NUM_ENTRIES,
/*N*/ 				   "SmParser::Power() : sub-/supscript index falsch");
/*N*/ 
/*N*/ 		// set sub-/supscript if not already done
/*N*/ 		if (aSubNodes.Get(nIndex) != NULL)
/*?*/ 			Error(PE_DOUBLE_SUBSUPSCRIPT);
/*N*/ 		aSubNodes.Put(nIndex, NodeStack.Pop());
/*N*/ 	}
/*N*/ 
/*N*/ 	pNode->SetSubNodes(aSubNodes);
/*N*/ 	NodeStack.Push(pNode);
/*N*/ }


/*N*/ void SmParser::OpSubSup()
/*N*/ {
/*N*/ 	// push operator symbol
/*N*/ 	NodeStack.Push(new SmMathSymbolNode(CurToken));
/*N*/ 	// skip operator token
/*N*/ 	NextToken();
/*N*/ 	// get sub- supscripts if any
/*N*/ 	if (TokenInGroup(TGPOWER))
/*?*/ 		SubSup(TGPOWER);
/*N*/ }


/*N*/ void SmParser::Power()
/*N*/ {
/*N*/ 	// get body for sub- supscripts on top of stack
/*N*/ 	Term();
/*N*/ 
/*N*/ 	SubSup(TGPOWER);
/*N*/ }


/*N*/ void SmParser::Blank()
/*N*/ {
/*N*/ 	DBG_ASSERT(TokenInGroup(TGBLANK), "Sm : falsches Token");
/*N*/ 	SmBlankNode *pBlankNode = new SmBlankNode(CurToken);
/*N*/ 
/*N*/ 	while (TokenInGroup(TGBLANK))
/*N*/ 	{
/*N*/ 		pBlankNode->IncreaseBy(CurToken);
/*N*/ 		NextToken();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Blanks am Zeilenende ignorieren wenn die entsprechende Option gesetzt ist
/*N*/ 	if (CurToken.eType == TNEWLINE  ||  CurToken.eType == TEND
/*N*/         &&  SM_MOD1()->GetConfig()->IsIgnoreSpacesRight())
/*?*/ 		pBlankNode->Clear();
/*N*/ 
/*N*/ 	NodeStack.Push(pBlankNode);
/*N*/ }


/*N*/ void SmParser::Term()
/*N*/ {
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{	case TESCAPE :
/*N*/ 			Escape();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TLGROUP :
/*N*/ 			NextToken();
/*N*/ 
/*N*/ 			// allow for empty group
/*N*/ 			if (CurToken.eType == TRGROUP)
/*N*/ 			{	SmStructureNode *pSNode = new SmExpressionNode(CurToken);
/*N*/ 				pSNode->SetSubNodes(NULL, NULL);
/*N*/ 				NodeStack.Push(pSNode);
/*N*/ 
/*N*/ 				NextToken();
/*N*/ 			}
/*N*/ 			else	// go as usual
/*N*/ 			{	Align();
/*N*/ 				if (CurToken.eType != TRGROUP)
/*N*/ 					Error(PE_RGROUP_EXPECTED);
/*N*/ 				else
/*N*/ 				{	NextToken();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TLEFT :
/*N*/ 			Brace();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TBLANK :
/*N*/ 		case TSBLANK :
/*N*/ 			Blank();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TTEXT :
/*N*/ 			NodeStack.Push(new SmTextNode(CurToken, FNT_TEXT));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 		case TIDENT :
/*N*/ 		case TCHARACTER :
/*N*/ 			NodeStack.Push(new SmTextNode(CurToken, FNT_VARIABLE));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 		case TNUMBER :
/*N*/ 			NodeStack.Push(new SmTextNode(CurToken, FNT_NUMBER));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TLEFTARROW :
/*N*/ 		case TRIGHTARROW :
/*N*/ 		case TUPARROW :
/*N*/ 		case TDOWNARROW :
/*N*/ 		case TSETN :
/*N*/ 		case TSETZ :
/*N*/ 		case TSETQ :
/*N*/ 		case TSETR :
/*N*/ 		case TSETC :
/*N*/ 		case THBAR :
/*N*/ 		case TLAMBDABAR :
/*N*/ 		case TCIRC :
/*N*/ 		case TDRARROW :
/*N*/ 		case TDLARROW :
/*N*/ 		case TDLRARROW :
/*N*/ 		case TBACKEPSILON :
/*N*/ 		case TALEPH :
/*N*/ 		case TIM :
/*N*/ 		case TRE :
/*N*/ 		case TWP :
/*N*/ 		case TEMPTYSET :
/*N*/ 		case TINFINITY :
/*N*/ 		case TEXISTS :
/*N*/ 		case TFORALL :
/*N*/ 		case TPARTIAL :
/*N*/ 		case TNABLA :
/*N*/ 		case TTOWARD :
/*N*/ 		case TDOTSAXIS :
/*N*/ 		case TDOTSDIAG :
/*N*/ 		case TDOTSDOWN :
/*N*/ 		case TDOTSLOW :
/*N*/ 		case TDOTSUP :
/*N*/ 		case TDOTSVERT :
/*N*/ 			NodeStack.Push(new SmMathSymbolNode(CurToken));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TPLACE:
/*N*/ 			NodeStack.Push(new SmPlaceNode(CurToken));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TSPECIAL:
/*N*/ 			Special();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TBINOM:
/*N*/			Binom(); // matrixfett.smf
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TSTACK:
/*N*/			Stack(); // matrixfett.smf
/*?*/ 			break;
/*N*/ 
/*N*/ 		case TMATRIX:
/*N*/ 			Matrix();
/*N*/ 			break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			if (TokenInGroup(TGLBRACES))
/*N*/ 			{	Brace();
/*N*/ 			}
/*N*/ 			else if (TokenInGroup(TGOPER))
/*N*/ 			{	Operator();
/*N*/ 			}
/*N*/ 			else if (TokenInGroup(TGUNOPER))
/*N*/ 			{	UnOper();
/*N*/ 			}
/*N*/ 			else if (    TokenInGroup(TGATTRIBUT)
/*N*/ 					 ||  TokenInGroup(TGFONTATTR))
/*N*/ 			{	SmStructureNodeArray  aArray;
/*N*/ 
/*N*/ 				BOOL    bIsAttr;
/*N*/ 				USHORT  n = 0;
/*N*/ 				while ((bIsAttr = TokenInGroup(TGATTRIBUT))
/*N*/ 					   ||  TokenInGroup(TGFONTATTR))
/*N*/ 				{	aArray.SetSize(n + 1);
/*N*/ 
/*N*/ 					if (bIsAttr)
/*N*/ 						Attribut();
/*N*/ 					else
/*N*/ 						FontAttribut();
/*N*/ 
/*N*/ 					// check if casting in following line is ok
/*N*/ 					DBG_ASSERT(!NodeStack.Top()->IsVisible(), "Sm : Ooops...");
/*N*/ 
/*N*/ 					aArray.Put(n, (SmStructureNode *) NodeStack.Pop());
/*N*/ 					n++;
/*N*/ 				}
/*N*/ 
/*N*/ 				Power();
/*N*/ 
/*N*/ 				SmNode *pFirstNode = NodeStack.Pop();
/*N*/ 				while (n > 0)
/*N*/ 				{	aArray.Get(n - 1)->SetSubNodes(0, pFirstNode);
/*N*/ 					pFirstNode = aArray.Get(n - 1);
/*N*/ 					n--;
/*N*/ 				}
/*N*/ 				NodeStack.Push(pFirstNode);
/*N*/ 			}
/*N*/ 			else if (TokenInGroup(TGFUNCTION))
/*N*/             {   if (CONVERT_40_TO_50 != GetConversion())
/*N*/ 				{	Function();
/*N*/ 				}
/*N*/ 				else	// encapsulate old 4.0 style parsing in braces
/*N*/ 				{
/*N*/ 					// insert opening brace
/*N*/ 					Insert('{', GetTokenIndex());
/*N*/ 
/*N*/ 					//
/*N*/ 					// parse in 4.0 style
/*N*/ 					//
/*N*/ 					Function();
/*N*/ 
/*N*/ 					SmNode *pFunc = NodeStack.Pop();
/*N*/ 
/*N*/ 					if (CurToken.eType == TLPARENT)
/*?*/ 					{	Term();
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{	Align();
/*N*/ 					}
/*N*/ 
/*N*/ 					// insert closing brace
/*N*/ 					Insert('}', GetTokenIndex());
/*N*/ 
/*N*/ 					SmStructureNode *pSNode = new SmExpressionNode(pFunc->GetToken());
/*N*/ 					pSNode->SetSubNodes(pFunc, NodeStack.Pop());
/*N*/ 					NodeStack.Push(pSNode);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				Error(PE_UNEXPECTED_CHAR);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Escape()
/*N*/ {
/*N*/ 	NextToken();
/*N*/ 
/*N*/ 	sal_Unicode	cChar;
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{	case TLPARENT :		cChar = MS_LPARENT;		break;
/*N*/ 		case TRPARENT :		cChar = MS_RPARENT;		break;
/*N*/ 		case TLBRACKET :	cChar = MS_LBRACKET;	break;
/*N*/ 		case TRBRACKET :	cChar = MS_RBRACKET;	break;
/*N*/         case TLDBRACKET :   cChar = MS_LDBRACKET;   break;
/*N*/         case TRDBRACKET :   cChar = MS_RDBRACKET;   break;
/*N*/ 		case TLBRACE :
/*N*/ 		case TLGROUP :		cChar = MS_LBRACE;		break;
/*N*/ 		case TRBRACE :
/*N*/ 		case TRGROUP :		cChar = MS_RBRACE;		break;
/*N*/ 		case TLANGLE :		cChar = MS_LANGLE;		break;
/*N*/ 		case TRANGLE :		cChar = MS_RANGLE;		break;
/*N*/ 		case TLCEIL :		cChar = MS_LCEIL;		break;
/*N*/ 		case TRCEIL :		cChar = MS_RCEIL;		break;
/*N*/ 		case TLFLOOR :		cChar = MS_LFLOOR;		break;
/*N*/ 		case TRFLOOR :		cChar = MS_RFLOOR;		break;
/*N*/ 		case TLLINE :
/*N*/ 		case TRLINE :		cChar = MS_LINE;		break;
/*N*/ 		case TLDLINE :
/*N*/ 		case TRDLINE :		cChar = MS_DLINE;		break;
/*N*/ 		default:
/*N*/ 			Error(PE_UNEXPECTED_TOKEN);
/*N*/ 	}
/*N*/ 
/*N*/ 	SmNode *pNode = new SmMathSymbolNode(CurToken);
/*N*/ 	NodeStack.Push(pNode);
/*N*/ 
/*N*/ 	NextToken();
/*N*/ }


/*N*/ void SmParser::Operator()
/*N*/ {
/*N*/ 	if (TokenInGroup(TGOPER))
/*N*/ 	{	SmStructureNode *pSNode = new SmOperNode(CurToken);
/*N*/ 
/*N*/ 		// put operator on top of stack
/*N*/ 		Oper();
/*N*/ 
/*N*/ 		if (TokenInGroup(TGLIMIT) || TokenInGroup(TGPOWER))
/*N*/ 			SubSup(CurToken.nGroup);
/*N*/ 		SmNode *pOperator = NodeStack.Pop();
/*N*/ 
/*N*/ 		// get argument
/*N*/ 		Power();
/*N*/ 
/*N*/ 		pSNode->SetSubNodes(pOperator, NodeStack.Pop());
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Oper()
/*N*/ {
/*N*/ 	SmTokenType  eType (CurToken.eType);
/*N*/ 	SmNode      *pNode = NULL;
/*N*/ 
/*N*/ 	switch (eType)
/*N*/ 	{
/*N*/ 		case TSUM :
/*N*/ 		case TPROD :
/*N*/ 		case TCOPROD :
/*N*/ 		case TINT :
/*N*/ 		case TIINT :
/*N*/ 		case TIIINT :
/*N*/ 		case TLINT :
/*N*/ 		case TLLINT :
/*N*/ 		case TLLLINT :
/*N*/ 			pNode = new SmMathSymbolNode(CurToken);
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TLIM :
/*N*/ 		case TLIMSUP :
/*N*/ 		case TLIMINF :
/*N*/ 			{
/*N*/ 				const sal_Char* pLim = 0;
/*N*/ 				switch (eType)
/*N*/ 				{
/*N*/ 					case TLIM :		pLim = "lim";		break;
/*N*/ 					case TLIMSUP :  pLim = "lim sup";	break;
/*N*/ 					case TLIMINF :  pLim = "lim inf";	break;
/*N*/ 				}
/*N*/ 				if( pLim )
/*N*/ 					CurToken.aText.AssignAscii( pLim );
/*N*/ 				pNode = new SmTextNode(CurToken, FNT_TEXT);
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TOVERBRACE :
/*N*/ 		case TUNDERBRACE :
/*N*/ 				pNode = new SmMathSymbolNode(CurToken);
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TOPER :
/*?*/ 			NextToken();
/*N*/ 
/*?*/ 			DBG_ASSERT(CurToken.eType == TSPECIAL, "Sm: falsches Token");
/*?*/ 			pNode = new SmGlyphSpecialNode(CurToken);
/*?*/ 			break;
/*N*/ 
/*N*/ 		default :
/*N*/ 			DBG_ASSERT(0, "Sm: unbekannter Fall");
/*N*/ 	}
/*N*/ 	NodeStack.Push(pNode);
/*N*/ 
/*N*/ 	NextToken();
/*N*/ }


/*N*/ void SmParser::UnOper()
/*N*/ {
/*N*/ 	DBG_ASSERT(TokenInGroup(TGUNOPER), "Sm: falsches Token");
/*N*/ 
/*N*/ 	SmToken		 aNodeToken = CurToken;
/*N*/ 	SmTokenType  eType 		= CurToken.eType;
/*N*/ 	BOOL		 bIsPostfix = eType == TFACT;
/*N*/ 
/*N*/ 	SmStructureNode *pSNode;
/*N*/ 	SmNode *pOper,
/*N*/ 		   *pExtra = 0,
/*N*/ 		   *pArg;
/*N*/ 
/*N*/ 	switch (eType)
/*N*/ 	{
/*N*/ 		case TABS :
/*N*/ 		case TSQRT :
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TNROOT :
/*N*/ 			NextToken();
/*N*/ 			Power();
/*N*/ 			pExtra = NodeStack.Pop();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TUOPER :
/*?*/ 			NextToken();
/*?*/ 			GlyphSpecial();
/*?*/ 			pOper = NodeStack.Pop();
/*?*/ 			break;
/*N*/ 
/*N*/ 		case TPLUS :
/*N*/ 		case TMINUS :
/*N*/ 		case TPLUSMINUS :
/*N*/ 		case TMINUSPLUS :
/*N*/ 		case TNEG :
/*N*/ 		case TFACT :
/*N*/ 			OpSubSup();
/*N*/ 			pOper = NodeStack.Pop();
/*N*/ 			break;
/*N*/ 
/*N*/ 		default :
/*N*/ 			Error(PE_UNOPER_EXPECTED);
/*N*/ 	}
/*N*/ 
/*N*/ 	// get argument
/*N*/ 	Power();
/*N*/ 	pArg = NodeStack.Pop();
/*N*/ 
/*N*/ 	if (eType == TABS)
/*N*/ 	{	pSNode = new SmBraceNode(aNodeToken);
/*N*/ 		pSNode->SetScaleMode(SCALE_HEIGHT);
/*N*/ 
/*N*/ 		// build nodes for left & right lines
/*N*/ 		// (text, group, level of the used token are of no interrest here)
/*N*/ 		// we'll use row & column of the keyword for abs
/*N*/ 		aNodeToken.eType = TABS;
/*N*/ 		//
/*N*/ 		aNodeToken.cMathChar = MS_LINE;
/*N*/ 		SmNode* pLeft = new SmMathSymbolNode(aNodeToken);
/*N*/ 		//
/*N*/ 		aNodeToken.cMathChar = MS_LINE;
/*N*/ 		SmNode* pRight = new SmMathSymbolNode(aNodeToken);
/*N*/ 
/*N*/ 		pSNode->SetSubNodes(pLeft, pArg, pRight);
/*N*/ 	}
/*N*/ 	else if (eType == TSQRT  ||  eType == TNROOT)
/*N*/ 	{	pSNode = new SmRootNode(aNodeToken);
/*N*/ 		pOper = new SmRootSymbolNode(aNodeToken);
/*N*/ 		pSNode->SetSubNodes(pExtra, pOper, pArg);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	pSNode = new SmUnHorNode(aNodeToken);
/*N*/ 
/*N*/ 		if (bIsPostfix)
/*N*/ 			pSNode->SetSubNodes(pArg, pOper);
/*N*/ 		else
/*N*/ 			// prefix operator
/*N*/ 			pSNode->SetSubNodes(pOper, pArg);
/*N*/ 	}
/*N*/ 
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::Attribut()
/*N*/ {
/*N*/ 	DBG_ASSERT(TokenInGroup(TGATTRIBUT), "Sm: falsche Tokengruppe");
/*N*/ 
/*N*/ 	SmStructureNode	*pSNode = new SmAttributNode(CurToken);
/*N*/ 	SmNode		*pAttr;
/*N*/ 	SmScaleMode  eScaleMode = SCALE_NONE;
/*N*/ 
/*N*/ 	// get appropriate node for the attribut itself
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{	case TUNDERLINE :
/*N*/ 		case TOVERLINE :
/*N*/ 		case TOVERSTRIKE :
/*?*/ 			pAttr = new SmRectangleNode(CurToken);
/*?*/ 			eScaleMode = SCALE_WIDTH;
/*?*/ 			break;
/*N*/ 
/*N*/ 		case TWIDEVEC :
/*N*/ 		case TWIDEHAT :
/*N*/ 		case TWIDETILDE :
/*N*/ 			pAttr = new SmMathSymbolNode(CurToken);
/*N*/ 			eScaleMode = SCALE_WIDTH;
/*?*/ 			break;
/*N*/ 
/*N*/ 		default :
/*N*/ 			pAttr = new SmMathSymbolNode(CurToken);
/*N*/ 	}
/*N*/ 
/*N*/ 	NextToken();
/*N*/ 
/*N*/ 	pSNode->SetSubNodes(pAttr, 0);
/*N*/ 	pSNode->SetScaleMode(eScaleMode);
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::FontAttribut()
/*N*/ {
/*N*/ 	DBG_ASSERT(TokenInGroup(TGFONTATTR), "Sm: falsche Tokengruppe");
/*N*/ 
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{
/*N*/ 		case TITALIC :
/*N*/ 		case TNITALIC :
/*N*/ 		case TBOLD :
/*N*/ 		case TNBOLD :
/*N*/ 		case TPHANTOM :
/*N*/ 			NodeStack.Push(new SmFontNode(CurToken));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TSIZE :
/*N*/ 			FontSize();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TFONT :
/*N*/ 			Font();
/*N*/ 			break;
/*N*/ 
/*N*/ 		case TCOLOR :
/*N*/ 			Color();
/*N*/ 			break;
/*N*/ 
/*N*/ 		default :
/*N*/ 			DBG_ASSERT(0, "Sm: unbekannter Fall");
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Color()
/*N*/ {
/*N*/     DBG_ASSERT(CurToken.eType == TCOLOR, "Sm : Ooops...");
/*N*/ 
/*N*/     // last color rules, get that one
/*N*/     SmToken  aToken;
/*N*/     do
/*N*/     {   NextToken();
/*N*/ 
/*N*/         if (TokenInGroup(TGCOLOR))
/*N*/         {   aToken = CurToken;
/*N*/             NextToken();
/*N*/         }
/*N*/         else
/*N*/             Error(PE_COLOR_EXPECTED);
/*N*/     } while (CurToken.eType == TCOLOR);
/*N*/ 
/*N*/     NodeStack.Push(new SmFontNode(aToken));
/*N*/ }


/*N*/ void SmParser::Font()
/*N*/ {
/*N*/ 	DBG_ASSERT(CurToken.eType == TFONT, "Sm : Ooops...");
/*N*/ 
/*N*/ 	// last font rules, get that one
/*N*/ 	SmToken  aToken;
/*N*/ 	do
/*N*/ 	{	NextToken();
/*N*/ 
/*N*/ 		if (TokenInGroup(TGFONT))
/*N*/ 		{	aToken = CurToken;
/*N*/ 			NextToken();
/*N*/ 		}
/*N*/ 		else
/*N*/ 			Error(PE_FONT_EXPECTED);
/*N*/ 	} while (CurToken.eType == TFONT);
/*N*/ 
/*N*/ 	NodeStack.Push(new SmFontNode(aToken));
/*N*/ }
/*N*/ BOOL lcl_IsNumber(const UniString& rText)
/*N*/ {
/*N*/ 	BOOL bPoint = FALSE;
/*N*/ 	const sal_Unicode* pBuffer = rText.GetBuffer();
/*N*/ 	for(xub_StrLen nPos = 0; nPos < rText.Len(); nPos++, pBuffer++)
/*N*/ 	{
/*N*/ 		const sal_Unicode cChar = *pBuffer;
/*N*/ 		if(cChar == '.')
/*N*/ 		{
/*N*/ 			if(bPoint)
/*N*/ 				return FALSE;
/*N*/ 			else
/*N*/ 				bPoint = TRUE;
/*N*/ 		}
/*N*/ 		else if ( (cChar < 48) || (cChar > 57) )
/*N*/ 			return FALSE;
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ void SmParser::FontSize()
/*N*/ {
/*N*/ 	DBG_ASSERT(CurToken.eType == TSIZE, "Sm : Ooops...");
/*N*/ 
/*N*/ 	USHORT	   Type;
/*N*/ 	SmFontNode *pFontNode = new SmFontNode(CurToken);
/*N*/ 
/*N*/ 	NextToken();
/*N*/ 
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{
/*N*/ 		case TNUMBER:	Type = FNTSIZ_ABSOLUT;	break;
/*N*/ 		case TPLUS:		Type = FNTSIZ_PLUS;		break;
/*N*/ 		case TMINUS:	Type = FNTSIZ_MINUS;	break;
/*N*/ 		case TMULTIPLY:	Type = FNTSIZ_MULTIPLY;	break;
/*N*/ 		case TDIVIDEBY:	Type = FNTSIZ_DIVIDE;	break;
/*N*/ 
/*N*/ 		default:
/*?*/ 			delete pFontNode;
/*?*/ 			Error(PE_SIZE_EXPECTED);
/*?*/ 			return;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (Type != FNTSIZ_ABSOLUT)
/*N*/ 	{
/*N*/ 		NextToken();
/*N*/ 		if (CurToken.eType != TNUMBER)
/*N*/ 		{
/*?*/ 			delete pFontNode;
/*?*/ 			Error(PE_SIZE_EXPECTED);
/*?*/ 			return;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// get number argument
/*N*/ 	double	  fTmp;
/*N*/ 	Fraction  aValue;
/*N*/ 	if(lcl_IsNumber(CurToken.aText) &&
/*N*/ 		sscanf(ByteString(CurToken.aText, RTL_TEXTENCODING_ASCII_US).GetBuffer(), "%lf", &fTmp) == 1)
/*N*/ 		aValue = fTmp;
/*N*/ 
/*N*/ 	NextToken();
/*N*/ 
/*N*/ 	pFontNode->SetSizeParameter(aValue, Type);
/*N*/ 	NodeStack.Push(pFontNode);
/*N*/ }


/*N*/ void SmParser::Brace()
/*N*/ {
/*N*/ 	DBG_ASSERT(CurToken.eType == TLEFT  ||  TokenInGroup(TGLBRACES),
/*N*/ 		"Sm: kein Klammer Ausdruck");
/*N*/ 
/*N*/ 	SmStructureNode *pSNode  = new SmBraceNode(CurToken);
/*N*/ 	SmNode *pBody   = 0,
/*N*/ 		   *pLeft   = 0,
/*N*/ 		   *pRight  = 0;
/*N*/ 	SmScaleMode   eScaleMode = SCALE_NONE;
/*N*/ 	SmParseError  eError     = PE_NONE;
/*N*/ 
/*N*/ 	if (CurToken.eType == TLEFT)
/*N*/ 	{	NextToken();
/*N*/ 
/*N*/ 		eScaleMode = SCALE_HEIGHT;
/*N*/ 
/*N*/ 		// check for left bracket
/*N*/ 		if (TokenInGroup(TGLBRACES) || TokenInGroup(TGRBRACES))
/*N*/ 		{
/*N*/ 			pLeft = new SmMathSymbolNode(CurToken);
/*N*/ 
/*N*/ 			NextToken();
/*N*/ 			Bracebody(TRUE);
/*N*/ 			pBody = NodeStack.Pop();
/*N*/ 
/*N*/ 			if (CurToken.eType == TRIGHT)
/*N*/ 			{	NextToken();
/*N*/ 
/*N*/ 				// check for right bracket
/*N*/ 				if (TokenInGroup(TGLBRACES) || TokenInGroup(TGRBRACES))
/*N*/ 				{
/*N*/ 					pRight = new SmMathSymbolNode(CurToken);
/*N*/ 					NextToken();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					eError = PE_RBRACE_EXPECTED;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				eError = PE_RIGHT_EXPECTED;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			eError = PE_LBRACE_EXPECTED;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if (TokenInGroup(TGLBRACES))
/*N*/ 		{
/*N*/ 			pLeft = new SmMathSymbolNode(CurToken);
/*N*/ 
/*N*/ 			NextToken();
/*N*/ 			Bracebody(FALSE);
/*N*/ 			pBody = NodeStack.Pop();
/*N*/ 
/*N*/ 			SmTokenType  eExpectedType = TUNKNOWN;
/*N*/ 			switch (pLeft->GetToken().eType)
/*N*/ 			{	case TLPARENT :	 	eExpectedType = TRPARENT;	break;
/*N*/ 				case TLBRACKET : 	eExpectedType = TRBRACKET;	break;
/*N*/ 				case TLBRACE : 	 	eExpectedType = TRBRACE;	break;
/*N*/ 				case TLDBRACKET :	eExpectedType = TRDBRACKET;	break;
/*N*/ 				case TLLINE : 	 	eExpectedType = TRLINE;		break;
/*N*/ 				case TLDLINE :   	eExpectedType = TRDLINE;	break;
/*N*/ 				case TLANGLE :   	eExpectedType = TRANGLE;	break;
/*N*/ 				case TLFLOOR :   	eExpectedType = TRFLOOR;	break;
/*N*/ 				case TLCEIL : 	 	eExpectedType = TRCEIL;		break;
/*N*/ 				default :
/*N*/ 					DBG_ASSERT(0, "Sm: unbekannter Fall");
/*N*/ 			}
/*N*/ 
/*N*/ 			if (CurToken.eType == eExpectedType)
/*N*/ 			{
/*N*/ 				pRight = new SmMathSymbolNode(CurToken);
/*N*/ 				NextToken();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				eError = PE_PARENT_MISMATCH;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			eError = PE_LBRACE_EXPECTED;
/*N*/ 	}
/*N*/ 
/*N*/ 	if (eError == PE_NONE)
/*N*/ 	{   DBG_ASSERT(pLeft,  "Sm: NULL pointer");
/*N*/ 		DBG_ASSERT(pRight, "Sm: NULL pointer");
/*N*/ 		pSNode->SetSubNodes(pLeft, pBody, pRight);
/*N*/ 		pSNode->SetScaleMode(eScaleMode);
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	delete pSNode;
/*N*/ 		delete pBody;
/*N*/ 		delete pLeft;
/*N*/ 		delete pRight;
/*N*/ 
/*N*/ 		Error(eError);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Bracebody(BOOL bIsLeftRight)
/*N*/ {
/*N*/ 	SmStructureNode *pBody = new SmBracebodyNode(CurToken);
/*N*/ 	SmNodeArray  	 aNodes;
/*N*/ 	USHORT		 	 nNum = 0;
/*N*/ 
/*N*/ 	// get body if any
/*N*/ 	if (bIsLeftRight)
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*N*/ 			if (CurToken.eType == TMLINE)
/*N*/ 			{
/*N*/ 				NodeStack.Push(new SmMathSymbolNode(CurToken));
/*N*/                 NextToken();
/*N*/ 				nNum++;
/*N*/ 			}
/*N*/ 			else if (CurToken.eType != TRIGHT)
/*N*/ 			{	Align();
/*N*/ 				nNum++;
/*N*/ 
/*N*/ 				if (CurToken.eType != TMLINE  &&  CurToken.eType != TRIGHT)
/*N*/ 					Error(PE_RIGHT_EXPECTED);
/*N*/ 			}
/*N*/ 		} while (CurToken.eType != TEND  &&  CurToken.eType != TRIGHT);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*N*/ 			if (CurToken.eType == TMLINE)
/*N*/ 			{
/*N*/ 				NodeStack.Push(new SmMathSymbolNode(CurToken));
/*N*/ 				NextToken();
/*N*/ 				nNum++;
/*N*/ 			}
/*N*/ 			else if (!TokenInGroup(TGRBRACES))
/*N*/ 			{	Align();
/*N*/ 				nNum++;
/*N*/ 
/*N*/ 				if (CurToken.eType != TMLINE  &&  !TokenInGroup(TGRBRACES))
/*N*/ 					Error(PE_RBRACE_EXPECTED);
/*N*/ 			}
/*N*/ 		} while (CurToken.eType != TEND  &&  !TokenInGroup(TGRBRACES));
/*N*/ 	}
/*N*/ 
/*N*/ 	// build argument vector in parsing order
/*N*/ 	aNodes.SetSize(nNum);
/*N*/ 	for (USHORT i = 0;  i < nNum;  i++)
/*N*/ 		aNodes.Put(nNum - 1 - i, NodeStack.Pop());
/*N*/ 
/*N*/ 	pBody->SetSubNodes(aNodes);
/*N*/ 	pBody->SetScaleMode(bIsLeftRight ? SCALE_HEIGHT : SCALE_NONE);
/*N*/ 	NodeStack.Push(pBody);
/*N*/ }


/*N*/ void SmParser::Function()
/*N*/ {
/*N*/ 	switch (CurToken.eType)
/*N*/ 	{
/*N*/ 		case TFUNC:
/*N*/ 			NextToken();	// skip "FUNC"-statement
/*N*/ 			// fall through
/*N*/ 
/*N*/ 		case TSIN :
/*N*/ 		case TCOS :
/*N*/ 		case TTAN :
/*N*/ 		case TCOT :
/*N*/ 		case TASIN :
/*N*/ 		case TACOS :
/*N*/ 		case TATAN :
/*N*/ 		case TACOT :
/*N*/ 		case TSINH :
/*N*/ 		case TCOSH :
/*N*/ 		case TTANH :
/*N*/ 		case TCOTH :
/*N*/ 		case TASINH :
/*N*/ 		case TACOSH :
/*N*/ 		case TATANH :
/*N*/ 		case TACOTH :
/*N*/ 		case TLN :
/*N*/ 		case TLOG :
/*N*/ 		case TEXP :
/*N*/ 			NodeStack.Push(new SmTextNode(CurToken, FNT_FUNCTION));
/*N*/ 			NextToken();
/*N*/ 			break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			Error(PE_FUNC_EXPECTED);
/*N*/ 	}
/*N*/ }


/*N*/ void SmParser::Binom() // matrixfett.smf
/*N*/ {
/*N*/ 	SmNodeArray  ExpressionArray;
/*N*/ 	SmStructureNode	*pSNode = new SmTableNode(CurToken);
/*N*/ 
/*N*/ 	NextToken();
/*N*/ 
/*N*/ 	Sum();
/*N*/ 	Sum();
/*N*/ 
/*N*/ 	ExpressionArray.SetSize(2);
/*N*/ 
/*N*/ 	for (int i = 0;  i < 2;  i++)
/*N*/ 		ExpressionArray.Put(2 - (i + 1), NodeStack.Pop());
/*N*/ 
/*N*/ 	pSNode->SetSubNodes(ExpressionArray);
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ }


/*N*/ void SmParser::Stack() // matrixfett.smf
/*N*/ {
/*N*/ 	SmNodeArray  ExpressionArray;
/*N*/ 	NextToken();
/*N*/ 	if (CurToken.eType == TLGROUP)
/*N*/ 	{
/*N*/ 		USHORT n = 0;
/*N*/ 
/*N*/ 		do
/*N*/ 		{
/*N*/ 			NextToken();
/*N*/ 			Align();
/*N*/ 			n++;
/*N*/ 		}
/*N*/ 		while (CurToken.eType == TPOUND);
/*N*/ 
/*N*/ 		ExpressionArray.SetSize(n);
/*N*/ 
/*N*/ 		for (USHORT i = 0; i < n; i++)
/*N*/ 			ExpressionArray.Put(n - (i + 1), NodeStack.Pop());
/*N*/ 
/*N*/ 		if (CurToken.eType != TRGROUP)
/*N*/ 			Error(PE_RGROUP_EXPECTED);
/*N*/ 
/*N*/ 		NextToken();
/*N*/ 
/*N*/ 		SmStructureNode *pSNode = new SmTableNode(CurToken);
/*N*/ 		pSNode->SetSubNodes(ExpressionArray);
/*N*/ 		NodeStack.Push(pSNode);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		Error(PE_LGROUP_EXPECTED);
/*N*/ }


/*N*/ void SmParser::Matrix()
/*N*/ {
/*N*/ 	SmNodeArray  ExpressionArray;
/*N*/ 
/*N*/ 	NextToken();
/*N*/ 	if (CurToken.eType == TLGROUP)
/*N*/ 	{
/*N*/ 		USHORT c = 0;
/*N*/ 
/*N*/ 		do
/*N*/ 		{
/*N*/ 			NextToken();
/*N*/ 			Align();
/*N*/ 			c++;
/*N*/ 		}
/*N*/ 		while (CurToken.eType == TPOUND);
/*N*/ 
/*N*/ 		USHORT r = 1;
/*N*/ 
/*N*/ 		while (CurToken.eType == TDPOUND)
/*N*/ 		{
/*N*/ 			NextToken();
/*N*/ 			for (USHORT i = 0; i < c; i++)
/*N*/ 			{
/*N*/ 				Align();
/*N*/ 				if (i < (c - 1))
/*N*/ 				{
/*N*/ 					if (CurToken.eType == TPOUND)
/*N*/ 					{
/*N*/ 						NextToken();
/*N*/ 					}
/*N*/ 					else
/*N*/ 						Error(PE_POUND_EXPECTED);
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			r++;
/*N*/ 		}
/*N*/ 
/*N*/ 		long nRC = r * c;
/*N*/ 
/*N*/ 		ExpressionArray.SetSize(nRC);
/*N*/ 
/*N*/ 		for (USHORT i = 0; i < (nRC); i++)
/*N*/ 			ExpressionArray.Put((nRC) - (i + 1), NodeStack.Pop());
/*N*/ 
/*N*/ 		if (CurToken.eType != TRGROUP)
/*N*/ 			Error(PE_RGROUP_EXPECTED);
/*N*/ 
/*N*/ 		NextToken();
/*N*/ 
/*N*/ 		SmMatrixNode *pMNode = new SmMatrixNode(CurToken);
/*N*/ 		pMNode->SetSubNodes(ExpressionArray);
/*N*/ 		pMNode->SetRowCol(r, c);
/*N*/ 		NodeStack.Push(pMNode);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		Error(PE_LGROUP_EXPECTED);
/*N*/ }


/*N*/ void SmParser::Special()
/*M*/ {
/*M*/     BOOL bReplace = FALSE;
/*M*/     String &rName = CurToken.aText;
/*M*/     String aNewName;
/*M*/ 
/*M*/     if (CONVERT_NONE == GetConversion())
/*M*/     {
/*M*/         // conversion of symbol names for 6.0 (XML) file format
/*M*/         // (name change on import / export.
/*M*/         // UI uses localized names XML file format does not.)
/*M*/         if (IsImportSymbolNames())
/*M*/         {
/*M*/             const SmLocalizedSymbolData &rLSD = SM_MOD1()->GetLocSymbolData();
/*M*/             aNewName = rLSD.GetUiSymbolName( rName );
/*M*/             bReplace = TRUE;
/*M*/         }
/*M*/         else if (IsExportSymbolNames())
/*M*/         {
/*M*/             const SmLocalizedSymbolData &rLSD = SM_MOD1()->GetLocSymbolData();
/*M*/             aNewName = rLSD.GetExportSymbolName( rName );
/*M*/             bReplace = TRUE;
/*M*/         }
/*M*/     }
/*M*/     else    // 5.0 <-> 6.0 formula text (symbol name) conversion
/*M*/     {
/*M*/         LanguageType nLang = GetLanguage();
/*M*/         SmLocalizedSymbolData &rData = SM_MOD1()->GetLocSymbolData();
/*M*/         const ResStringArray *pFrom = 0;
/*M*/         const ResStringArray *pTo   = 0;
/*M*/         if (CONVERT_50_TO_60 == GetConversion())
/*M*/         {
/*M*/             pFrom = rData.Get50NamesArray( nLang );
/*M*/             pTo   = rData.Get60NamesArray( nLang );
/*M*/         }
/*M*/         else if (CONVERT_60_TO_50 == GetConversion())
/*M*/         {
/*M*/             pFrom = rData.Get60NamesArray( nLang );
/*M*/             pTo   = rData.Get50NamesArray( nLang );
/*M*/         }
/*M*/         if (pFrom  &&  pTo)
/*M*/         {
/*M*/             DBG_ASSERT( pFrom->Count() == pTo->Count(),
/*M*/                     "array length mismatch" );
/*M*/             USHORT nCount = pFrom->Count();
/*M*/             for (USHORT i = 0;  i < nCount;  ++i)
/*M*/             {
/*M*/                 if (pFrom->GetString(i) == rName)
/*M*/                 {
/*M*/                     aNewName = pTo->GetString(i);
/*M*/                     bReplace = TRUE;
/*M*/                 }
/*M*/             }
/*M*/         }
/*M*/         // else:
/*M*/         // conversion arrays not found or (usually)
/*M*/         // conversion not necessary
/*M*/     }
/*M*/ 
/*M*/     if (bReplace  &&  aNewName.Len()  &&  rName != aNewName)
/*M*/     {
/*M*/         Replace( GetTokenIndex() + 1, rName.Len(), aNewName );
/*M*/         rName = aNewName;
/*M*/     }
/*M*/ 
/*M*/ 	NodeStack.Push(new SmSpecialNode(CurToken));
/*M*/ 	NextToken();
/*M*/ }


/*?*/ void SmParser::GlyphSpecial()
/*?*/ {
/*?*/ 	NodeStack.Push(new SmGlyphSpecialNode(CurToken));
/*?*/ 	NextToken();
/*?*/ }


/*N*/ void SmParser::Error(SmParseError eError)
/*N*/ {
/*N*/ 	SmStructureNode *pSNode = new SmExpressionNode(CurToken);
/*N*/ 	SmErrorNode		*pErr   = new SmErrorNode(eError, CurToken);
/*N*/ 	pSNode->SetSubNodes(pErr, 0);
/*N*/ 
/*N*/ 	//! put a structure node on the stack (instead of the error node itself)
/*N*/ 	//! because sometimes such a node is expected in order to attach some
/*N*/ 	//! subnodes
/*N*/ 	NodeStack.Push(pSNode);
/*N*/ 
/*N*/ 	AddError(eError, pSNode);
/*N*/ 
/*N*/ 	NextToken();
/*N*/ }


// end gramar


/*N*/ SmParser::SmParser()
/*N*/ {
/*N*/     eConversion = CONVERT_NONE;
/*N*/     bImportSymNames = bExportSymNames = FALSE;
/*N*/     nLang = Application::GetSettings().GetUILanguage();
/*N*/ }


/*N*/ SmNode *SmParser::Parse(const String &rBuffer)
/*N*/ {
/*N*/ 	BufferString = rBuffer;
/*N*/ 	BufferString.ConvertLineEnd( LINEEND_LF );
/*N*/ 	BufferIndex  =
/*N*/ 	nTokenIndex  = 0;
/*N*/ 	Row 		 = 1;
/*N*/ 	ColOff		 = 0;
/*N*/ 	CurError	 = -1;
/*N*/ 
/*N*/ 	for (USHORT i = 0;  i < ErrDescList.Count();  i++)
/*N*/ 		delete ErrDescList.Remove(i);
/*N*/ 
/*N*/ 	ErrDescList.Clear();
/*N*/ 
/*N*/ 	NodeStack.Clear();
/*N*/ 
/*N*/     SetLanguage( Application::GetSettings().GetUILanguage() );
/*N*/ 	NextToken();
/*N*/ 	Table();
/*N*/ 
/*N*/ 	return NodeStack.Pop();
/*N*/ }


/*N*/ USHORT SmParser::AddError(SmParseError Type, SmNode *pNode)
/*N*/ {
/*N*/ 	SmErrorDesc *pErrDesc = new SmErrorDesc;
/*N*/ 
/*N*/ 	pErrDesc->Type  = Type;
/*N*/ 	pErrDesc->pNode = pNode;
/*N*/ 	pErrDesc->Text  = String(SmResId(RID_ERR_IDENT));
/*N*/ 
/*N*/ 	USHORT  nRID;
/*N*/ 	switch (Type)
/*N*/ 	{
/*N*/ 		case PE_UNEXPECTED_CHAR:	 nRID = RID_ERR_UNEXPECTEDCHARACTER;	break;
/*N*/ 		case PE_LGROUP_EXPECTED:	 nRID = RID_ERR_LGROUPEXPECTED;			break;
/*N*/ 		case PE_RGROUP_EXPECTED:	 nRID = RID_ERR_RGROUPEXPECTED;			break;
/*N*/ 		case PE_LBRACE_EXPECTED:	 nRID = RID_ERR_LBRACEEXPECTED;			break;
/*N*/ 		case PE_RBRACE_EXPECTED:	 nRID = RID_ERR_RBRACEEXPECTED;			break;
/*N*/ 		case PE_FUNC_EXPECTED:		 nRID = RID_ERR_FUNCEXPECTED;			break;
/*N*/ 		case PE_UNOPER_EXPECTED:	 nRID = RID_ERR_UNOPEREXPECTED;			break;
/*N*/ 		case PE_BINOPER_EXPECTED:	 nRID = RID_ERR_BINOPEREXPECTED;		break;
/*N*/ 		case PE_SYMBOL_EXPECTED:	 nRID = RID_ERR_SYMBOLEXPECTED;			break;
/*N*/ 		case PE_IDENTIFIER_EXPECTED: nRID = RID_ERR_IDENTEXPECTED;			break;
/*N*/ 		case PE_POUND_EXPECTED:		 nRID = RID_ERR_POUNDEXPECTED;			break;
/*N*/ 		case PE_COLOR_EXPECTED:		 nRID = RID_ERR_COLOREXPECTED;			break;
/*N*/ 		case PE_RIGHT_EXPECTED:		 nRID = RID_ERR_RIGHTEXPECTED;			break;
/*N*/ 
/*N*/ 		default:
/*N*/ 			nRID = RID_ERR_UNKOWN;
/*N*/ 	}
/*N*/ 	pErrDesc->Text += SmResId(nRID);
/*N*/ 
/*N*/ 	ErrDescList.Insert(pErrDesc);
/*N*/ 
/*N*/ 	return (USHORT) ErrDescList.GetPos(pErrDesc);
/*N*/ }








}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
