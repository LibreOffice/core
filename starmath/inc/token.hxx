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
    NONE          = 0x00000000,
    Oper          = 0x00000001,
    Relation      = 0x00000002,
    Sum           = 0x00000004,
    Product       = 0x00000008,
    UnOper        = 0x00000010,
    Power         = 0x00000020,
    Attribute     = 0x00000040,
    Align         = 0x00000080,
    Function      = 0x00000100,
    Blank         = 0x00000200,
    LBrace        = 0x00000400,
    RBrace        = 0x00000800,
    Color         = 0x00001000,
    Font          = 0x00002000,
    Standalone    = 0x00004000,
    Limit         = 0x00008000,
    FontAttr      = 0x00010000,
    Character     = 0x00020000,
};

namespace o3tl {
    template<> struct typed_flags<TG> : is_typed_flags<TG, 0x037fff> {};
}

enum SmTokenType
{
    TDIVIDEBY,      TASSIGN,        TSPECIAL,       TLIMSUP,        TNDIVIDESWITH,
    TBACKSLASH,     TLIMINF,        TNDIVIDES,      TNOSPACE,       TEXESS,
    TTRANSL,        TDIVIDES,       TBAR,           TNEWLINE,       TWIDEBACKSLASH,
    TUNDERLINE,     TOVERLINE,      TOVERSTRIKE,    TWIDESLASH,     TTRANSR,
    TCIRC,
    // Logic
    TAND,           TOR,            TNEG,           TNEQ,
    TKNOW,          TADJOINT,       TTOPTEE,        TASSERTION,     TNOTPROOVE,
    TMODELS,        TTRUE,          TNOTTRUE,       TFORCES,        TNOTFORCES,
    TTHEREFORE,     TRATIO,         TPROPORTION,
    // Font
    TFONT,          TTEXT,          TNUMBER,        TIDENT,
    TALIGNL,        TALIGNC,        TALIGNR,        TSANS,          TSERIF,
    TBOLD,          TNBOLD,         TPHANTOM,       TSIZE,          TFIXED,
    TITALIC,        TNITALIC,
    // Font Attributtes
    // Control
    TEND,           TRGROUP,        TLGROUP,        TESCAPE,        TLSUP,
    TRSUB,          TRSUP,          TCSUB,          TCSUP,          TLSUB,
    TUNKNOWN,       TERROR,         TBLANK,         TSBLANK,        TPLACE,
    TNONE,          TUOPER,         TBOPER,         TTOMBSTONE,
    // Tensors
    TOPLUS,         TOMINUS,        TODIVIDE,       TOTIMES,        TODOT,
    TODASH,         TGVECTORPROD,   THERMITANCONJ,  TMONUS,         THOMOTHETY,
    TSQPLUS,        TSQMINUS,       TSQTIMES,       TSQDOT,         TOEQUALS,
    TOMULTIPLY,     TOEQUAL,
    // SimpleOper
    TCOMPOSITION,   TDIV,           TMINUSPLUS,     TPLUS,          TMINUS,
    TMULTIPLY,      TSLASH,         TPLUSDOT,       TOVER,          TTIMES,
    TFACT,          TABS,           TPLUSMINUS,     TCDOT,          TOCOMPOSE,
    // Arrows and dots
    TDOTSDIAG,      TDOTSUP,        TDOTSDOWN,      TDOTSAXIS,      TDOTSLOW,
    TDOTSVERT,      TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,
    TDRARROW,       TDLARROW,       TDLRARROW,
    // Order
    TPRECEDES,      TSUCCEEDS,      TPRECEDESEQUAL, TSUCCEEDSEQUAL,
    TPRECEDESEQUIV, TSUCCEEDSEQUIV, TNOTPRECEDES,   TNOTSUCCEEDS,
    // Compare
    TLL,            TGG,            TLLL,           TGGG,           TEQUAL,
    TLT,            TGT,            TGE,            TLE,            TSIM,
    TLESLANT,       TGESLANT,       TSIMEQ,         TAPPROX,        TEQUIV,
    TDEF,           TPROP,
    // Brackets
    TRIGHT,         TLEFT,
    TRDANGLE,       TLDANGLE,       TRANGLE,        TLANGLE,        TMLINE,
    TRCEIL,         TLCEIL,         TRFLOOR,        TLFLOOR,        TUNDERBRACE,
    TRLINE,         TLLINE,         TRDLINE,        TLDLINE,        TOVERBRACE,
    TRBRACE,        TLBRACE,        TLDBRACKET,     TRDBRACKET,
    TRBRACKET,      TLBRACKET,      TRPARENT,       TLPARENT,
    // Sets
    TSETN,          TSETZ,          TSETQ,          TSETR,          TSETC,
    TEMPTYSET,      TCOMPLEMENT,    TFORALL,        TEXISTS,        TNOTEXISTS,
    TNI,            TIN,            TNOTIN,         TOWNS,          TNOTOWNS,
    TELEMENTOF,     TCONTASMEMBER,
    TUNION,         TSQUNION,       TINTERSECT,     TSQINTERSECT,
    TSUBSET,        TSUBSETEQ,      TNSUBSET,       TNSUBSETEQ,
    TSUPSET,        TSUPSETEQ,      TNSUPSET,       TNSUPSETEQ,
    TSQSUBSET,      TSQSUBSETEQ,    TSQSUPSET,      TSQSUPSETEQ,
    // Operators
    TOPER,          TSUM,           TFROM,          TTO,            TTOWARD,
    TINTD,          TINT,           TIINT,          TIIINT,         TIIIINT,
    TLINT,          TLLINT,         TLLLINT,        TFILINT,        TDBINT,
    TAVERAGE,       TINTERSECTINT,  TOBINT,         TUBINT,         TSUMINT,
    TCLINT,         TACLINT,        TSQPINT,        TCIPINT,        TWPINT,
    TAPLINT,        TQUAINT,        TLARROWINT,     TTIMESINT,      TUNIONINT,
    TOINTERSECT,    TOUNION,        TOOPLUS,        TOODOT,         TOOTIMES,
    TPROD,          TCOPROD,        TOAND,          TOOR,           TLIM,
    // Attributes
    TDOT,           TDDOT,          TDDDOT,         TDDDDOT,        // Derivative dot
    TVEC,           TWIDEVEC,       THARPOON,       TWIDEHARPOON,   // Vec & arpoon
    TTILDE,         TWIDETILDE,     THAT,           TWIDEHAT,       // Tilde & hat
    TACUTE,         TGRAVE,         TBREVE,         TCHECK,         // Some accents
    TCIRCLE,
    // Spetial characters
    TCHAR,          TCHARACTER,     TBACKEPSILON,   TEULERCTE,
    TINFINITY,      THBAR,          TLAMBDABAR,     TEULER,         TPLANK,
    TDIGAMMA,       TINODOT,        TJNODOT,        TAMPERSAND,     TBULLET,
    TALEPH,         TBETH,          TGIMEL,         TETH,           TNATURALEXP,
    // Units and extras
    TLITRE,         TDEGREE,        TAMSTRONG,      TMICRO,         TOUNCE,
    TSIEMENS,       TMORDINALI,     TFORDINALI,     TNUMSIGN,
    TPERCENT,       TPERTHOUSAND,   TPERTENTHOUSAND,
    // Character sets
    TGREEK,         TDIGIT,         TROM,           TROMD,          TCURRENCY,
    TSET,           THCROD,         TVCROD,         TPHOENICIAN,    TLATINCHAR,
    TFRAK,          TSCRIPT,        THEBREW,        TGOTHIC,        TINTERNATIONAL,
    TEASTEREGG,
    // Function
    TFUNC,          TLN,            TLOG,           TEXP,           // Exp - Log
    TSIN,           TCOS,           TTAN,           TCOT,           // Trigo
    TSINH,          TCOSH,          TTANH,          TCOTH,          // Trigo hyperbolic
    TASIN,          TACOS,          TATAN,          TACOT,          // Arctrigo
    TASINH,         TACOSH,         TATANH,         TACOTH,         // Arctrigo hyperbolic
    TSQRT,          TNROOT,
    // Color
    TCOLOR,         TRGB,           TRGBA,          THEX,
    TAQUA,          TBLACK,         TBLUE,          TCYAN,          TFUCHSIA,
    TGRAY,          TGREEN,         TLIME,          TMAGENTA,       TMAROON,
    TNAVY,          TOLIVE,         TPURPLE,        TRED,           TSILVER,
    TTEAL,          TWHITE,         TYELLOW,
    // Geometry
    TPARALLEL,      TORTHO,         TNOTPARALLEL,   TANGLE,         TGEOFIG,
    TWAVE,          TSPHTRIGO,      TPROJECTIVE,    TPERSPECTIVE,   TTRANSVERSAL,
    TPERPENDICULAR, TPERPENDICULARWITHS,
    // Derivative and transforms and complex
    TWP,            TIM,            TRE,            TICOMPLEX,      TJCOMPLEX,
    TFOURIER,       TLAPLACE,       TPRIME,         TDPRIME,        TTPRIME,
    TNABLA,         TPARTIAL,       TUDIFF,         TLDIFF,         TINCREMENT,
    // Table structures //TODO tables of tdf i don't remember
    TBINOM,         TSTACK,         TMATRIX,        TPOUND,         TDPOUND
};

struct SmTokenTableEntry
{
    const char*      pIdent;
    SmTokenType      eType;
    sal_Unicode32    cMathChar;
    TG               nGroup;
    sal_uInt16       nLevel;
};

struct SmToken
{

    OUString        aText;      // token text
    SmTokenType     eType;      // token info
    sal_Unicode32   cMathChar;  // token text

    // parse-help info
    TG              nGroup;
    sal_uInt16      nLevel;

    // token position
    sal_Int32      nRow; // 1-based
    sal_Int32      nCol; // 1-based

    inline SmToken()
    : eType(TUNKNOWN)
    , cMathChar('\0')
    , nGroup(TG::NONE)
    , nLevel(0)
    , nRow(0)
    , nCol(0)
    { };

    inline SmToken( const SmTokenTableEntry &entry, sal_Int32 row = 0, sal_Int32 col = 0 )
    : aText( OUString::createFromAscii(entry.pIdent) )
    , eType( entry.eType )
    , cMathChar( entry.cMathChar )
    , nGroup( entry.nGroup )
    , nLevel( entry.nLevel )
    , nRow(row)
    , nCol(col)
    { };

    inline SmToken( SmTokenType eTokenType, sal_Unicode32 cMath, const char* pText,
                    TG nTokenGroup = TG::NONE, sal_uInt16 nTokenLevel = 0,
                    sal_Int32 row = 0, sal_Int32 col = 0 )
    : aText(OUString::createFromAscii(pText))
    , eType(eTokenType)
    , cMathChar(cMath)
    , nGroup(nTokenGroup)
    , nLevel(nTokenLevel)
    , nRow(row)
    , nCol(col)
    { };

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
