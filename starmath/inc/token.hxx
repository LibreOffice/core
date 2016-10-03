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
#include <o3tl/typed_flags_set.hxx>

// TokenGroups
enum class TG {
    NONE          = 0x000000,
    Oper          = 0x000001,
    Relation      = 0x000002,
    Sum           = 0x000004,
    Product       = 0x000008,
    UnOper        = 0x000010,
    Power         = 0x000020,
    Attribute     = 0x000040,
    Align         = 0x000080,
    Function      = 0x000100,
    Blank         = 0x000200,
    LBrace        = 0x000400,
    RBrace        = 0x000800,
    Color         = 0x001000,
    Font          = 0x002000,
    Standalone    = 0x004000,
    Limit         = 0x010000,
    FontAttr      = 0x020000
};
namespace o3tl {
    template<> struct typed_flags<TG> : is_typed_flags<TG, 0x037fff> {};
}

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
    TG              nGroup;
    sal_uInt16      nLevel;

    // token position
    sal_Int32      nRow; // 1-based
    sal_Int32      nCol; // 1-based

    SmToken();
    SmToken(SmTokenType eTokenType,
            sal_Unicode cMath,
            const sal_Char* pText,
            TG nTokenGroup = TG::NONE,
            sal_uInt16 nTokenLevel = 0);
};

struct SmTokenTableEntry
{
    const sal_Char* pIdent;
    SmTokenType     eType;
    sal_Unicode     cMathChar;
    TG              nGroup;
    sal_uInt16      nLevel;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
