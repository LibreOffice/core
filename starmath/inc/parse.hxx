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
#ifndef PARSE_HXX
#define PARSE_HXX

#include <vcl/svapp.hxx>

#include <set>
#include <stack>
#include <list>

#include "types.hxx"

#include <vector>

class SmNode;

//////////////////////////////////////////////////////////////////////

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
/*225*/ TUNKNOWN,       TDEBUG,         TPRECEDES,      TSUCCEEDS,      TPRECEDESEQUAL,
/*230*/ TSUCCEEDSEQUAL, TPRECEDESEQUIV, TSUCCEEDSEQUIV, TNOTPRECEDES,   TNOTSUCCEEDS
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
    sal_uInt16      nRow;
    xub_StrLen      nCol;

    SmToken();
    SmToken(SmTokenType eTokenType,
            sal_Unicode cMath,
            const sal_Char* pText,
            sal_uLong nTokenGroup = 0,
            sal_uInt16 nTokenLevel = 0);
};


enum SmParseError
{
    PE_NONE,                    PE_UNEXPECTED_END_OF_INPUT,
    PE_UNEXPECTED_CHAR,         PE_UNEXPECTED_TOKEN,
    PE_FUNC_EXPECTED,           PE_UNOPER_EXPECTED,
    PE_BINOPER_EXPECTED,        PE_SYMBOL_EXPECTED,
    PE_IDENTIFIER_EXPECTED,     PE_POUND_EXPECTED,
    PE_COLOR_EXPECTED,          PE_LGROUP_EXPECTED,
    PE_RGROUP_EXPECTED,         PE_LBRACE_EXPECTED,
    PE_RBRACE_EXPECTED,         PE_PARENT_MISMATCH,
    PE_RIGHT_EXPECTED,          PE_FONT_EXPECTED,
    PE_SIZE_EXPECTED,           PE_DOUBLE_ALIGN,
    PE_DOUBLE_SUBSUPSCRIPT
};


struct SmErrorDesc
{
    SmParseError  Type;
    SmNode       *pNode;
    String        Text;
};


typedef ::std::stack< SmNode* > SmNodeStack;
typedef ::std::vector< SmErrorDesc* > SmErrDescList;

/**************************************************************************/

// defines possible conversions of the formula text from the format of
// one release to the one of another.
enum SmConvert
{
    CONVERT_NONE,
    CONVERT_40_TO_50,
    CONVERT_50_TO_60,
    CONVERT_60_TO_50
};

struct SmTokenTableEntry
{
    const sal_Char* pIdent;
    SmTokenType     eType;
    sal_Unicode     cMathChar;
    sal_uLong       nGroup;
    sal_uInt16      nLevel;
};

class SmParser
{
    String          m_aBufferString;
    SmToken         m_aCurToken;
    SmNodeStack     m_aNodeStack;
    SmErrDescList   m_aErrDescList;
    int             m_nCurError;
    LanguageType    m_nLang;
    xub_StrLen      m_nBufferIndex,
                    m_nTokenIndex;
    sal_uInt16          m_Row,
                    m_nColOff;
    SmConvert       m_eConversion;
    bool            bImportSymNames,
                    m_bExportSymNames;

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set< OUString >   m_aUsedSymbols;

    //! locale where '.' is decimal separator!
    ::com::sun::star::lang::Locale m_aDotLoc;

    // declare copy-constructor and assignment-operator private
    SmParser(const SmParser &);
    SmParser & operator = (const SmParser &);

protected:
#if OSL_DEBUG_LEVEL > 1
    bool            IsDelimiter( const String &rTxt, xub_StrLen nPos );
#endif
    void            NextToken();
    xub_StrLen      GetTokenIndex() const   { return m_nTokenIndex; }
    void            Insert(const String &rText, sal_uInt16 nPos);
    void            Replace( sal_uInt16 nPos, sal_uInt16 nLen, const String &rText );

    inline bool     TokenInGroup( sal_uLong nGroup );

    // grammar
    void    Table();
    void    Line();
    void    Expression();
    void    Relation();
    void    Sum();
    void    Product();
    void    SubSup(sal_uLong nActiveGroup);
    void    OpSubSup();
    void    Power();
    void    Blank();
    void    Term(bool bGroupNumberIdent);
    void    Escape();
    void    Operator();
    void    Oper();
    void    UnOper();
    void    Align();
    void    FontAttribut();
    void    Attribut();
    void    Font();
    void    FontSize();
    void    Color();
    void    Brace();
    void    Bracebody(bool bIsLeftRight);
    void    Function();
    void    Binom();
    void    Stack();
    void    Matrix();
    void    Special();
    void    GlyphSpecial();
    // end of grammar

    LanguageType    GetLanguage() const { return m_nLang; }
    void            SetLanguage( LanguageType nNewLang ) { m_nLang = nNewLang; }

    void    Error(SmParseError Error);

    void    ClearUsedSymbols()                              { m_aUsedSymbols.clear(); }
    void    AddToUsedSymbols( const String &rSymbolName )   { m_aUsedSymbols.insert( rSymbolName ); }

public:
                 SmParser();

    /** Parse rBuffer to formula tree */
    SmNode      *Parse(const String &rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    SmNode      *ParseExpression(const OUString &rBuffer);

    const String & GetText() const { return m_aBufferString; };

    SmConvert   GetConversion() const              { return m_eConversion; }
    void        SetConversion(SmConvert eConv)     { m_eConversion = eConv; }

    bool        IsImportSymbolNames() const        { return bImportSymNames; }
    void        SetImportSymbolNames(bool bVal)    { bImportSymNames = bVal; }
    bool        IsExportSymbolNames() const        { return m_bExportSymNames; }
    void        SetExportSymbolNames(bool bVal)    { m_bExportSymNames = bVal; }

    size_t      AddError(SmParseError Type, SmNode *pNode);
    const SmErrorDesc*  NextError();
    const SmErrorDesc*  PrevError();
    const SmErrorDesc*  GetError(size_t i = size_t(-1) );
    static const SmTokenTableEntry* GetTokenTableEntry( const String &rName );
    bool    IsUsedSymbol( const String &rSymbolName ) const { return m_aUsedSymbols.find( rSymbolName ) != m_aUsedSymbols.end(); }
    std::set< OUString >   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( sal_uLong nGroup)
{
    return (m_aCurToken.nGroup & nGroup) ? true : false;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
