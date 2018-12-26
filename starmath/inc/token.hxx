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
    TEND,           TLGROUP,        TRGROUP,        TLPARENT,       TRPARENT,
    TLBRACKET,      TRBRACKET,      TPLUS,          TMINUS,         TMULTIPLY,
    TDIVIDEBY,      TASSIGN,        TPOUND,         TSPECIAL,       TSLASH,
    TBACKSLASH,     TBLANK,         TSBLANK,        TRSUB,          TRSUP,
    TCSUB,          TCSUP,          TLSUB,          TLSUP,          TGT,
    TLT,            TAND,           TOR,            TINTERSECT,     TUNION,
    TNEWLINE,       TBINOM,         TFROM,          TTO,            TINT,
    TSUM,           TOPER,          TABS,           TSQRT,          TFACT,
    TNROOT,         TOVER,          TTIMES,         TGE,            TLE,
    TGG,            TLL,            TDOTSAXIS,      TDOTSLOW,       TDOTSVERT,
    TDOTSDIAG,      TDOTSUP,        TDOTSDOWN,      TACUTE,         TBAR,
    TBREVE,         TCHECK,         TCIRCLE,        TDOT,           TDDOT,
    TDDDOT,         TGRAVE,         THAT,           TTILDE,         TVEC,
    TUNDERLINE,     TOVERLINE,      TOVERSTRIKE,    TITALIC,        TNITALIC,
    TBOLD,          TNBOLD,         TPHANTOM,       TFONT,          TSIZE,
    TCOLOR,         TALIGNL,        TALIGNC,        TALIGNR,        TLEFT,
    TRIGHT,         TLANGLE,        TLBRACE,        TLLINE,         TLDLINE,
    TLCEIL,         TLFLOOR,        TNONE,          TMLINE,         TRANGLE,
    TRBRACE,        TRLINE,         TRDLINE,        TRCEIL,         TRFLOOR,
    TSIN,           TCOS,           TTAN,           TCOT,           TFUNC,
    TSTACK,         TMATRIX,        TDPOUND,        TPLACE,
    TTEXT,          TNUMBER,        TCHARACTER,     TIDENT,         TNEQ,
    TEQUIV,         TDEF,           TPROP,          TSIM,           TSIMEQ,
    TAPPROX,        TPARALLEL,      TORTHO,         TIN,            TNOTIN,
    TSUBSET,        TSUBSETEQ,      TSUPSET,        TSUPSETEQ,      TPLUSMINUS,
    TMINUSPLUS,     TOPLUS,         TOMINUS,        TDIV,           TOTIMES,
    TODIVIDE,       TTRANSL,        TTRANSR,        TIINT,          TIIINT,
    TLINT,          TLLINT,         TLLLINT,        TPROD,          TCOPROD,
    TFORALL,        TEXISTS,        TNOTEXISTS,     TLIM,           TNABLA,
    TTOWARD,        TSINH,          TCOSH,          TTANH,          TCOTH,
    TASIN,          TACOS,          TATAN,          TLN,            TLOG,
    TUOPER,         TBOPER,         TBLACK,         TWHITE,         TRED,
    TGREEN,         TBLUE,          TCYAN,          TMAGENTA,       TYELLOW,
    TFIXED,         TSANS,          TSERIF,         TASINH,
    TACOSH,         TATANH,         TACOTH,         TACOT,          TEXP,
    TCDOT,          TODOT,          TLESLANT,       TGESLANT,       TNSUBSET,
    TNSUPSET,       TNSUBSETEQ,     TNSUPSETEQ,     TPARTIAL,       TNEG,
    TNI,            TBACKEPSILON,   TALEPH,         TIM,            TRE,
    TWP,            TEMPTYSET,      TINFINITY,      TESCAPE,        TLIMSUP,
    TLIMINF,        TNDIVIDES,      TDRARROW,       TDLARROW,       TDLRARROW,
    TUNDERBRACE,    TOVERBRACE,     TCIRC,          THBAR,
    TLAMBDABAR,     TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,
    TDIVIDES,       TSETN,          TSETZ,          TSETQ,
    TSETR,          TSETC,          TWIDEVEC,       TWIDETILDE,     TWIDEHAT,
    TWIDESLASH,     TWIDEBACKSLASH, TLDBRACKET,     TRDBRACKET,     TNOSPACE,
    TUNKNOWN,       TPRECEDES,      TSUCCEEDS,      TPRECEDESEQUAL, TSUCCEEDSEQUAL,
    TPRECEDESEQUIV, TSUCCEEDSEQUIV, TNOTPRECEDES,   TNOTSUCCEEDS,   TSILVER,
    TGRAY,          TMAROON,        TPURPLE,        TLIME,          TOLIVE,
    TNAVY,          TTEAL,          TAQUA,          TFUCHSIA,       TINTD
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
    SmTokenType const     eType;
    sal_Unicode const     cMathChar;
    TG const              nGroup;
    sal_uInt16 const      nLevel;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
