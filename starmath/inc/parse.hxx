/*************************************************************************
 *
 *  $RCSfile: parse.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef PARSE_HXX
#define PARSE_HXX


#include <ctype.h>

#ifndef _STACK_HXX //autogen
#include <tools/stack.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#include "types.hxx"

class SmNode;


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
    TSTACK,         TMATRIX,        TMATFORM,       TDPOUND,        TPLACE,
    TTEXT,          TNUMBER,        TCHARACTER,     TIDENT,         TNEQ,
    TEQUIV,         TDEF,           TPROP,          TSIM,           TSIMEQ,
    TAPPROX,        TPARALLEL,      TORTHO,         TIN,            TNOTIN,
    TSUBSET,        TSUBSETEQ,      TSUPSET,        TSUPSETEQ,      TPLUSMINUS,
    TMINUSPLUS,     TOPLUS,         TOMINUS,        TDIV,           TOTIMES,
    TODIVIDE,       TTRANSL,        TTRANSR,        TIINT,          TIIINT,
    TLINT,          TLLINT,         TLLLINT,        TPROD,          TCOPROD,
    TFORALL,        TEXISTS,        TLIM,           TNABLA,         TTOWARD,
    TSINH,          TCOSH,          TTANH,          TCOTH,          TASIN,
    TACOS,          TATAN,          TLN,            TLOG,           TUOPER,
    TBOPER,         TBLACK,         TWHITE,         TRED,           TGREEN,
    TBLUE,          TCYAN,          TMAGENTA,       TYELLOW,        TFIXED,
    TSANS,          TSERIF,         TPOINT,         TASINH,         TACOSH,
    TATANH,         TACOTH,         TACOT,          TEXP,           TCDOT,
    TODOT,          TLESLANT,       TGESLANT,       TNSUBSET,       TNSUPSET,
    TNSUBSETEQ,     TNSUPSETEQ,     TPARTIAL,       TNEG,           TNI,
    TBACKEPSILON,   TALEPH,         TIM,            TRE,            TWP,
    TEMPTYSET,      TINFINITY,      TESCAPE,        TLIMSUP,        TLIMINF,
    TNDIVIDES,      TDRARROW,       TDLARROW,       TDLRARROW,      TUNDERBRACE,
    TOVERBRACE,     TCIRC,          TTOP,           THBAR,          TLAMBDABAR,
    TLEFTARROW,     TRIGHTARROW,    TUPARROW,       TDOWNARROW,     TDIVIDES,
    TNDIBVIDES,     TSETN,          TSETZ,          TSETQ,          TSETR,
    TSETC,          TWIDEVEC,       TWIDETILDE,     TWIDEHAT,       TWIDESLASH,
    TWIDEBACKSLASH, TLDBRACKET,     TRDBRACKET,
    TUNKNOWN,       TDEBUG
};


struct SmToken
{
    // token text
    XubString   aText;
    // token info
    SmTokenType eType;
    xub_Unicode     cMathChar;
    // parse-help info
    ULONG       nGroup;
    USHORT      nLevel;
    // token position
    USHORT      nRow;
    USHORT      nCol;

    SmToken();
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
    XubString         Text;
};

DECLARE_STACK(SmNodeStack,  SmNode *);
DECLARE_LIST(SmErrDescList, SmErrorDesc *);

/**************************************************************************/

class SmParser
{
    XubString     BufferString;
    SmToken       CurToken;
    SmNodeStack   NodeStack;
    SmErrDescList ErrDescList;
    int           CurError;
    xub_StrLen    BufferIndex,
                  nTokenIndex;
    USHORT        Row,
                  Column;
    xub_Unicode       CharLineEnd;
    BOOL          bConvert40To50;

    // declare copy-constructor and assignment-operator private
    SmParser(const SmParser &);
    SmParser & operator = (const SmParser &);

protected:
    BOOL            IsDelimiter(xub_Unicode cChar);
    void            NextToken();
    xub_StrLen      GetTokenIndex() const   { return nTokenIndex; }
    void            Insert(const XubString &rText, USHORT nPos);

    inline BOOL     TokenInGroup(ULONG nGroup);
    inline BOOL     IsWhiteSpace(const xub_Unicode cChar) const;
    inline BOOL     IsComment(const xub_Unicode *pPos) const;

    const xub_Unicode *     SkipWhiteSpaces(const xub_Unicode *pPos, USHORT &nRow, USHORT &nColumn);
    const xub_Unicode *     SkipComment(const xub_Unicode *pPos, USHORT &nRow, USHORT &nColumn);

    // grammar
    void    Table();
    void    Line();
    void    Expression();
    void    Relation();
    void    Sum();
    void    Product();
    void    SubSup(ULONG nActiveGroup);
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
    void    Bracebody(BOOL bIsLeftRight);
    void    Function();
    void    Binom();
    void    Stack();
    void    Matrix();
    void    Special();
    void    GlyphSpecial();
    // end of grammar

    void    Error(SmParseError Error);

public:
                 SmParser();

    BOOL         CheckSyntax(const XubString &rBuffer);
    SmNode      *Parse(const XubString &rBuffer);

    const XubString & GetText() const { return BufferString; };

    BOOL         IsConvert40To50() const         { return bConvert40To50; }
    void         SetConvert40To50(BOOL bConvert) { bConvert40To50 = bConvert; }

    USHORT       AddError(SmParseError Type, SmNode *pNode);

    const SmErrorDesc * NextError();
    const SmErrorDesc * PrevError();
    const SmErrorDesc * GetError(USHORT i = 0xFFFF);
};


inline BOOL SmParser::TokenInGroup(ULONG nGroup)
{
    return (CurToken.nGroup & nGroup) ? TRUE : FALSE;
}

inline BOOL SmParser::IsWhiteSpace(const xub_Unicode cChar) const
{
    return isspace(cChar) != 0;
}

inline BOOL SmParser::IsComment(const xub_Unicode *pPos) const
{
    return *pPos == '%'  &&  *(pPos + 1) == '%';
}


#endif

