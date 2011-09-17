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
#ifndef PARSE_HXX
#define PARSE_HXX


#include <vcl/svapp.hxx>
#include <tools/stack.hxx>
#include <tools/string.hxx>

#include <set>
#include <stack>
#include <list>

#include "types.hxx"

#include <vector>

class SmNode;
class SmDocShell;

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
/*140*/ TFORALL,        TEXISTS,        TNOTEXISTS,     TLIM,           TNABLA,         TTOWARD,
/*145*/ TSINH,          TCOSH,          TTANH,          TCOTH,          TASIN,
/*150*/ TACOS,          TATAN,          TLN,            TLOG,           TUOPER,
/*155*/ TBOPER,         TBLACK,         TWHITE,         TRED,           TGREEN,
/*160*/ TBLUE,          TCYAN,          TMAGENTA,       TYELLOW,        TFIXED,
/*165*/ TSANS,          TSERIF,         TPOINT,         TASINH,         TACOSH,
/*170*/ TATANH,         TACOTH,         TACOT,          TEXP,           TCDOT,
/*175*/ TODOT,          TLESLANT,       TGESLANT,       TNSUBSET,       TNSUPSET,
/*180*/ TNSUBSETEQ,     TNSUPSETEQ,     TPARTIAL,       TNEG,           TNI,
/*185*/ TBACKEPSILON,   TALEPH,         TIM,            TRE,            TWP,
/*190*/ TEMPTYSET,      TINFINITY,      TESCAPE,        TLIMSUP,        TLIMINF,
/*195*/ TNDIVIDES,      TDRARROW,       TDLARROW,       TDLRARROW,      TUNDERBRACE,
/*200*/ TOVERBRACE,     TCIRC,          TTOP,           THBAR,          TLAMBDABAR,
/*205*/ TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,     TDIVIDES,
/*210*/ TNDIBVIDES,     TSETN,          TSETZ,          TSETQ,          TSETR,
/*215*/ TSETC,          TWIDEVEC,       TWIDETILDE,     TWIDEHAT,       TWIDESLASH,
/*220*/ TWIDEBACKSLASH, TLDBRACKET,     TRDBRACKET,     TNOSPACE,
/*225*/ TUNKNOWN,       TDEBUG
};


struct SmToken
{

    String          aText;      // token text
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


DECLARE_STACK(SmNodeStack,  SmNode *)
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
    std::set< rtl::OUString >   m_aUsedSymbols;

    //! locale where '.' is decimal seperator!
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
    void    Term();
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
    SmNode      *ParseExpression(const String &rBuffer);

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
    std::set< rtl::OUString >   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( sal_uLong nGroup)
{
    return (m_aCurToken.nGroup & nGroup) ? true : false;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
