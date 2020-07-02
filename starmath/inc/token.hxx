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
    TDIVIDEBY,      TASSIGN,        TPOUND,         TSPECIAL,       TSLASH,
    TBACKSLASH,     TBLANK,         TSBLANK,        TRSUB,          TRSUP,
    TCSUB,          TCSUP,          TLSUB,          TLSUP,          TGT,
    TLT,            TAND,           TOR,            TNEWLINE,       TBINOM,
    TOVER,          TTIMES,         TGE,            TLE,
    TGG,            TLL,            TDOTSAXIS,      TDOTSLOW,       TDOTSVERT,
    TDOTSDIAG,      TDOTSUP,        TDOTSDOWN,      TACUTE,         TBAR,
    TBREVE,         TCHECK,         TCIRCLE,        TDOT,
    TDDDOT,         TGRAVE,         THAT,           TTILDE,         TVEC,
    THARPOON,       TUNDERLINE,     TOVERLINE,      TOVERSTRIKE,
    TSTACK,         TMATRIX,        TDPOUND,        TPLACE,
    TTEXT,          TNUMBER,        TCHARACTER,     TIDENT,         TNEQ,
    TAPPROX,        TPARALLEL,      TORTHO,         THEX,
    TOPLUS,         TOMINUS,        TDIV,           TOTIMES,
    TODIVIDE,       TTRANSL,        TTRANSR,
    TFORALL,        TEXISTS,        TNOTEXISTS,     TNABLA,
    TUOPER,         TBOPER,         TNEG,           TDDOT,
    TODOT,          TLESLANT,       TGESLANT,       TPARTIAL,
    TNI,            TBACKEPSILON,   TALEPH,         TIM,            TRE,
    TWP,            TINFINITY,      TESCAPE,        TLIMSUP,
    TLIMINF,        TNDIVIDES,      TDRARROW,       TDLARROW,       TDLRARROW,
    TUNDERBRACE,    TOVERBRACE,     TCIRC,          THBAR,
    TLAMBDABAR,     TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,
    TWIDEHAT,       TWIDEVEC,       TWIDEHARPOON,   TWIDETILDE,     TDIVIDES,
    TWIDESLASH,     TWIDEBACKSLASH, TLDBRACKET,     TRDBRACKET,     TNOSPACE,
    TUNKNOWN,       TPRECEDES,      TSUCCEEDS,      TPRECEDESEQUAL, TSUCCEEDSEQUAL,
    TPRECEDESEQUIV, TSUCCEEDSEQUIV, TNOTPRECEDES,   TNOTSUCCEEDS,
    TLAPLACE,
    // Basic math chars
    TMINUS,         TPLUS,          TMULTIPLY,      TPLUSMINUS,     TMINUSPLUS,
    TCDOT,          TEQUIV,         TDEF,           TPROP,          TSIM,
    TSIMEQ,
    // Color
    TCOLOR,         TRGB,           TAQUA,          TFUCHSIA,       TTEAL,
    TGRAY,          TMAROON,        TPURPLE,        TLIME,          TOLIVE,
    TGREEN,         TBLUE,          TCYAN,          TMAGENTA,       TYELLOW,
    TNAVY,          TSILVER,        TBLACK,         TWHITE,         TRED,
    // Font formats
    TITALIC,        TNITALIC,       TALIGNL,        TALIGNC,        TALIGNR,
    TBOLD,          TNBOLD,         TPHANTOM,       TFONT,          TSIZE,
    TFIXED,         TSANS,          TSERIF,
    //Sets
    TSETN,          TSETZ,          TSETQ,          TSETR,          TSETC,
    TEMPTYSET,      TSUBSET,        TSUBSETEQ,      TSUPSET,        TSUPSETEQ,
    TNSUBSET,       TNSUPSET,       TNSUBSETEQ,     TNSUPSETEQ,     TIN,
    TINTERSECT,     TUNION,         TOINTERSECT,    TOUNION,        TNOTIN,
    // Sums, Pros, Coprods, Ints, Limits, Opers
    TOPER,          TFROM,          TTO,            TTOWARD,        TLIM,
    TSUM,           TPROD,          TCOPROD,        TINT,           TINTD,
    TIINT,          TIIINT,         TLINT,          TLLINT,         TLLLINT,
    TLATRSUP,       TLATRSUB,
    // Functions
    TFUNC,          TLOG,           TLN,            TEXP,           TNROOT,
    TSIN,           TCOS,           TTAN,           TCOT,           // Trigo
    TSINH,          TCOSH,          TTANH,          TCOTH,          // Trigo hyperbolic
    TASIN,          TACOS,          TATAN,          TACOT,          // Arctrigo
    TASINH,         TACOSH,         TATANH,         TACOTH,         // Arctrigo hyperbolic
    TABS,           TSQRT,          TFACT,
    // Brackets
    TRIGHT,         TLANGLE,        TLBRACE,        TLLINE,         TLDLINE,
    TLCEIL,         TLFLOOR,        TNONE,          TMLINE,         TRANGLE,
    TRBRACE,        TRLINE,         TRDLINE,        TRCEIL,         TRFLOOR,
    TLEFT,          TLBRACKET,      TRBRACKET
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
            const char* pText,
            TG nTokenGroup = TG::NONE,
            sal_uInt16 nTokenLevel = 0);
};

struct SmTokenTableEntry
{
    const char* pIdent;
    SmTokenType      eType;
    sal_Unicode      cMathChar;
    TG               nGroup;
    sal_uInt16       nLevel;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
