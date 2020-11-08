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

 /** The tokens contain the information gathered by the parser.
   *
   * They contain:
   *    the data type (~ mathematical operation).
   *    The mathematical char.
   *    The corresponding code or information to recreate it.
   *    Location of the token in the starmath code.
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

// Tokens identifiers. Allow to know what kind of information the node contains.
enum SmTokenType
{
    // Uncategorized
    TEND,           TSPECIAL,       TNONE,          TESCAPE,        TUNKNOWN,
    TBLANK,         TSBLANK,        TPLACE,         TNOSPACE,       TDOTSDOWN,
    TNEWLINE,       TDOTSAXIS,      TDOTSLOW,       TDOTSVERT,      TBACKEPSILON,
    TDOTSDIAG,      TDOTSUP,        TFRAC,
    // Basic
    TPLUS,          TMINUS,         TMULTIPLY,      TDIVIDEBY,      // +-*/
    TGT,            TLT,            TGE,            TLE,            // > < >= <=
    TASSIGN,        TNEQ,           TGG,            TLL,            // = != >>> <<<
    TPARALLEL,      TORTHO,         TEQUIV,                         // Geometry
    TOPER,          TSUM,           TPROD,          TCOPROD,        // Operators
    TIM,            TRE,            THBAR,          TLAMBDABAR,     // Complex and constants
    TPLUSMINUS,     TMINUSPLUS,     TSIM,           TSIMEQ,         // +- -+ ~ ~=
    TLIM,           TLIMSUP,        TLIMINF,        TTOWARD,        // Limits
    TOVER,          TTIMES,         TCDOT,          TDIV,           // Product type
    TSLASH,         TBACKSLASH,     TWIDESLASH,     TWIDEBACKSLASH, //Slash
    // Structure
    TMATRIX,         TPOUND,        TDPOUND,        TSTACK,         TBINOM,
    // Logic
    TAND,           TOR,            TNEG,                           // && || !
    TPRECEDES,      TSUCCEEDS,      TNOTPRECEDES,   TNOTSUCCEEDS,   // Order
    TPRECEDESEQUAL, TSUCCEEDSEQUAL, TPRECEDESEQUIV, TSUCCEEDSEQUIV, // Order eq
    TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,     // Arrows
    TDRARROW,       TDLARROW,       TDLRARROW,      TDEF,           // Double arrows, definition
    TPROP,          TNDIVIDES,      TDIVIDES,       TAPPROX,        // Proportions, approximation
    TLESLANT,       TGESLANT,       TTRANSL,        TTRANSR,        // <= >= corresponds
    // Tensors
    TOPLUS,         TOMINUS,        TOTIMES,        TODIVIDE,       TODOT,
    TCIRC,
    // Positions
    TRSUB,          TRSUP,          TCSUB,          TCSUP,          TLSUB,
    TLSUP,          TFROM,          TTO,            TUOPER,         TBOPER,
    // Set theory
    TSETN,          TSETZ,          TSETQ,          TSETR,          TSETC,
    TIN,            TNOTIN,         TNI,            TEMPTYSET,      // Insideout
    TSUBSET,        TSUBSETEQ,      TSUPSET,        TSUPSETEQ,      // Subsupset
    TNSUBSET,       TNSUPSET,       TNSUBSETEQ,     TNSUPSETEQ,     // Not subsupset
    TINTERSECT,     TUNION,         TSETMINUS,      TSETQUOTIENT,   // +-/
    TALEPH,         TWP,            TINFINITY,                      // Abstract sets
    TFORALL,        TEXISTS,        TNOTEXISTS,                     // Existential
    // Font
    TFONT,          TSIZE,          TCOLOR,         TPHANTOM,       // Basic
    TITALIC,        TNITALIC,       TBOLD,          TNBOLD,         // Bold ital
    TALIGNL,        TALIGNC,        TALIGNR,                        // Align
    TUNDERLINE,     TOVERLINE,      TOVERSTRIKE,    TBAR,           // Lines
    TFIXED,         TSANS,          TSERIF,                         // Types
    TACUTE,         TGRAVE,         THAT,           TBREVE,         // Accents
    TWIDEVEC,       TWIDEHARPOON,   TWIDETILDE,     TWIDEHAT,       // Wide math
    TVEC,           THARPOON,       TTILDE,         TCIRCLE,        // math
    TCHECK,
    TTEXT,          TNUMBER,        TCHARACTER,     TIDENT,         // Content type
    // Brackets
    TLEFT,          TRIGHT,         TUNDERBRACE,    TOVERBRACE,     // Scalable, upsidedown
    TLGROUP,        TRGROUP,        TLPARENT,       TRPARENT,       // Structural
    TLBRACKET,      TRBRACKET,      TLDBRACKET,     TRDBRACKET,     // Bracket x1 & x2
    TLCEIL,         TRCEIL,         TLFLOOR,        TRFLOOR,        // Reals -> Wholes
    TLANGLE,        TRANGLE,        TLBRACE,        TRBRACE,        // <x> {x}
    // Brackets Lines
    TLLINE,         TRLINE,         TLDLINE,        TRDLINE,        TMLINE,
    // Differential calculus
    TNABLA,         TPARTIAL,       TFOURIER,       TLAPLACE,       // Derivative, Transformation
    TINTD,          TINT,           TIINT,          TIIINT,         // Integral
    TLINT,          TLLINT,         TLLLINT,                        // Circuit integral
    TDOT,           TDDOT,          TDDDOT,                         // Derivative dots
    // Function
    TFUNC,          TLN,            TLOG,           TEXP,           // Exp - Log
    TSIN,           TCOS,           TTAN,           TCOT,           // Trigo
    TSINH,          TCOSH,          TTANH,          TCOTH,          // Trigo hyperbolic
    TASIN,          TACOS,          TATAN,          TACOT,          // Arctrigo
    TASINH,         TACOSH,         TATANH,         TACOTH,         // Arctrigo hyperbolic
    TSQRT,          TNROOT,         TFACT,          TABS,           // roots, n! |z|
    // Color
    TRGB,           TRGBA,          THEX,
    TAQUA,          TBLACK,         TBLUE,          TCYAN,          TFUCHSIA,
    TGRAY,          TGREEN,         TLIME,          TMAGENTA,       TMAROON,
    TNAVY,          TOLIVE,         TPURPLE,        TRED,           TSILVER,
    TTEAL,          TWHITE,         TYELLOW
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
