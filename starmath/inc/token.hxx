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
#ifndef INCLUDED_STARMATH_INC_TOKEN_HXX
#define INCLUDED_STARMATH_INC_TOKEN_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>

// TokenGroups
#define TGOPER          0x00000001
#define TGRELATION      0x00000002
#define TGSUM           0x00000004
#define TGPRODUCT       0x00000008
#define TGUNOPER        0x00000010
#define TGPOWER         0x00000020
#define TGATTRIBUT      0x00000040
#define TGALIGN         0x00000080
#define TGFUNCTION      0x00000100
#define TGBLANK         0x00000200
#define TGLBRACES       0x00000400
#define TGRBRACES       0x00000800
#define TGCOLOR         0x00001000
#define TGFONT          0x00002000
#define TGSTANDALONE    0x00004000
#define TGDISCARDED     0x00008000
#define TGLIMIT         0x00010000
#define TGFONTATTR      0x00020000


enum SmTokenType
{
/*  0*/ TEND,           TLGROUP,        TRGROUP,        TLPARENT,       TRPARENT,
/*  5*/ TLBRACKET,      TRBRACKET,      TPLUS,          TMINUS,         TMULTIPLY,
/* 10*/ TDIVIDEBY,      TASSIGN,        TPOUND,         TSPECIAL,       TSLASH,
/* 15*/ TBACKSLASH,     TBLANK,         TSBLANK,        TRSUB,          TRSUP,
/* 20*/ TCSUB,          TCSUP,          TLSUB,          TLSUP,          TGT,
/* 25*/ TLT,            TAND,           TOR,            TINTERSECT,     TUNION,
/* 30*/ TNEWLINE,       TBINOM,         TFROM,          TTO,            TINT,
/* 35*/ TSUM,           TOPER,          TABS,           TSQRT,          TFACT,
/* 40*/ TNROOT,         TOVER,          TTIMES,         TGE,            TLE,
/* 45*/ TGG,            TLL,            TDOTSAXIS,      TDOTSLOW,       TDOTSVERT,
/* 50*/ TDOTSDIAG,      TDOTSUP,        TDOTSDOWN,      TACUTE,         TBAR,
/* 55*/ TBREVE,         TCHECK,         TCIRCLE,        TDOT,           TDDOT,
/* 60*/ TDDDOT,         TGRAVE,         THAT,           TTILDE,         TVEC,
/* 65*/ TUNDERLINE,     TOVERLINE,      TOVERSTRIKE,    TITALIC,        TNITALIC,
/* 70*/ TBOLD,          TNBOLD,         TPHANTOM,       TFONT,          TSIZE,
/* 75*/ TCOLOR,         TALIGNL,        TALIGNC,        TALIGNR,        TLEFT,
/* 80*/ TRIGHT,         TLANGLE,        TLBRACE,        TLLINE,         TLDLINE,
/* 85*/ TLCEIL,         TLFLOOR,        TNONE,          TMLINE,         TRANGLE,
/* 90*/ TRBRACE,        TRLINE,         TRDLINE,        TRCEIL,         TRFLOOR,
/* 95*/ TSIN,           TCOS,           TTAN,           TCOT,           TFUNC,
/*100*/ TSTACK,         TMATRIX,        TMATFORM,       TDPOUND,        TPLACE,
/*105*/ TTEXT,          TNUMBER,        TCHARACTER,     TIDENT,         TNEQ,
/*110*/ TEQUIV,         TDEF,           TPROP,          TSIM,           TSIMEQ,
/*115*/ TAPPROX,        TPARALLEL,      TORTHO,         TIN,            TNOTIN,
/*120*/ TSUBSET,        TSUBSETEQ,      TSUPSET,        TSUPSETEQ,      TPLUSMINUS,
/*125*/ TMINUSPLUS,     TOPLUS,         TOMINUS,        TDIV,           TOTIMES,
/*130*/ TODIVIDE,       TTRANSL,        TTRANSR,        TIINT,          TIIINT,
/*135*/ TLINT,          TLLINT,         TLLLINT,        TPROD,          TCOPROD,
/*140*/ TFORALL,        TEXISTS,        TNOTEXISTS,     TLIM,           TNABLA,
/*145*/ TTOWARD,        TSINH,          TCOSH,          TTANH,          TCOTH,
/*150*/ TASIN,          TACOS,          TATAN,          TLN,            TLOG,
/*155*/ TUOPER,         TBOPER,         TBLACK,         TWHITE,         TRED,
/*160*/ TGREEN,         TBLUE,          TCYAN,          TMAGENTA,       TYELLOW,
/*165*/ TFIXED,         TSANS,          TSERIF,         TPOINT,         TASINH,
/*170*/ TACOSH,         TATANH,         TACOTH,         TACOT,          TEXP,
/*175*/ TCDOT,          TODOT,          TLESLANT,       TGESLANT,       TNSUBSET,
/*180*/ TNSUPSET,       TNSUBSETEQ,     TNSUPSETEQ,     TPARTIAL,       TNEG,
/*185*/ TNI,            TBACKEPSILON,   TALEPH,         TIM,            TRE,
/*190*/ TWP,            TEMPTYSET,      TINFINITY,      TESCAPE,        TLIMSUP,
/*195*/ TLIMINF,        TNDIVIDES,      TDRARROW,       TDLARROW,       TDLRARROW,
/*200*/ TUNDERBRACE,    TOVERBRACE,     TCIRC,          TTOP,           THBAR,
/*205*/ TLAMBDABAR,     TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,
/*210*/ TDIVIDES,       TNDIBVIDES,     TSETN,          TSETZ,          TSETQ,
/*215*/ TSETR,          TSETC,          TWIDEVEC,       TWIDETILDE,     TWIDEHAT,
/*220*/ TWIDESLASH,     TWIDEBACKSLASH, TLDBRACKET,     TRDBRACKET,     TNOSPACE,
/*225*/ TUNKNOWN,       TPRECEDES,      TSUCCEEDS,      TPRECEDESEQUAL, TSUCCEEDSEQUAL,
/*230*/ TPRECEDESEQUIV, TSUCCEEDSEQUIV, TNOTPRECEDES,   TNOTSUCCEEDS,   TSILVER,
/*235*/ TGRAY,          TMAROON,        TPURPLE,        TLIME,          TOLIVE,
/*240*/ TNAVY,          TTEAL,          TAQUA,          TFUCHSIA,       TINTD
};

struct SmToken
{

    OUString        aText;      // token text
    SmTokenType     eType;      // token info
    sal_Unicode cMathChar;

    // parse-help info
    sal_uLong       nGroup;
    sal_uInt16      nLevel;

    // token position
    sal_Int32      nRow;
    sal_Int32      nCol;

    SmToken();
    SmToken(SmTokenType eTokenType,
            sal_Unicode cMath,
            const sal_Char* pText,
            sal_uLong nTokenGroup = 0,
            sal_uInt16 nTokenLevel = 0);
};

struct SmTokenTableEntry
{
    const sal_Char* pIdent;
    SmTokenType     eType;
    sal_Unicode     cMathChar;
    sal_uLong       nGroup;
    sal_uInt16      nLevel;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
