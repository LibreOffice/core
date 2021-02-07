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

/** Parses the starmath code and creates the nodes.
  *
  */

#ifndef INCLUDED_STARMATH_INC_PARSE_HXX
#define INCLUDED_STARMATH_INC_PARSE_HXX

#include <unotools/charclass.hxx>
#include <memory>
#include <set>
#include <vector>

#include "token.hxx"

class SmBlankNode;
class SmBinVerNode;
class SmBracebodyNode;
class SmExpressionNode;
class SmGlyphSpecialNode;
class SmNode;
class SmOperNode;
class SmSpecialNode;
class SmStructureNode;
class SmTableNode;
class SmTextNode;

#define DEPTH_LIMIT 1024

// Those are the errors that the parser may encounter.
enum class SmParseError : uint_fast8_t
{
    None = 0,
    UnexpectedChar = 1,
    UnexpectedToken = 2,
    PoundExpected = 3,
    ColorExpected = 4,
    LgroupExpected = 5,
    RgroupExpected = 6,
    LbraceExpected = 7,
    RbraceExpected = 8,
    ParentMismatch = 9,
    RightExpected = 10,
    FontExpected = 11,
    SizeExpected = 12,
    DoubleAlign = 13,
    DoubleSubsupscript = 14,
    NumberExpected = 15
};

struct SmErrorDesc
{
    SmParseError m_eType;
    SmNode* m_pNode;
    OUString m_aText;

    SmErrorDesc(SmParseError eType, SmNode* pNode, OUString aText)
    : m_eType(eType)
    , m_pNode(pNode)
    , m_aText(aText)
    {}

};

namespace starmathdatabase{

// Must be in sync with SmParseError list
extern const char* SmParseErrorDesc[16];

OUString getParseErrorDesc(SmParseError err);

}

class SmParser
{
    OUString        m_aBufferString;
    SmToken         m_aCurToken;
    std::vector<std::unique_ptr<SmErrorDesc>> m_aErrDescList;
    int             m_nCurError;
    sal_Int32       m_nBufferIndex,
                    m_nTokenIndex;
    sal_Int32       m_nRow,    // 1-based
                    m_nColOff; // 0-based
    bool            m_bImportSymNames,
                    m_bExportSymNames;
    sal_Int32       m_nParseDepth;

    class DepthProtect
    {
    private:
        sal_Int32& m_rParseDepth;
    public:
        DepthProtect(sal_Int32& rParseDepth)
            : m_rParseDepth(rParseDepth)
        {
            ++m_rParseDepth;
            if(m_rParseDepth > DEPTH_LIMIT)
                throw std::range_error("parser depth limit");
        }
        ~DepthProtect()
        {
            --m_rParseDepth;
        }
    };

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set< OUString >   m_aUsedSymbols;

    // CharClass representing a locale for parsing numbers
    CharClass m_aNumCC;
    // pointer to System locale's CharClass, which is alive inside SM_MOD()
    const CharClass* m_pSysCC;

    SmParser(const SmParser&) = delete;
    SmParser& operator=(const SmParser&) = delete;

    // Moves between tokens inside starmath code.
    void            NextToken();
    void            NextTokenColor(bool dvipload);
    void            NextTokenFontSize();
    sal_Int32       GetTokenIndex() const   { return m_nTokenIndex; }
    void            Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText );

    inline bool     TokenInGroup( TG nGroup );

    // grammar
    std::unique_ptr<SmTableNode> DoTable();
    std::unique_ptr<SmNode> DoLine();
    std::unique_ptr<SmNode> DoExpression(bool bUseExtraSpaces = true);
    std::unique_ptr<SmNode> DoRelation();
    std::unique_ptr<SmNode> DoSum();
    std::unique_ptr<SmNode> DoProduct();
    std::unique_ptr<SmNode> DoSubSup(TG nActiveGroup, SmNode *pGivenNode);
    std::unique_ptr<SmNode> DoSubSupEvaluate(SmNode *pGivenNode);
    std::unique_ptr<SmNode> DoOpSubSup();
    std::unique_ptr<SmNode> DoPower();
    std::unique_ptr<SmBlankNode> DoBlank();
    std::unique_ptr<SmNode> DoTerm(bool bGroupNumberIdent);
    std::unique_ptr<SmNode> DoEscape();
    std::unique_ptr<SmOperNode> DoOperator();
    std::unique_ptr<SmNode> DoOper();
    std::unique_ptr<SmStructureNode> DoUnOper();
    std::unique_ptr<SmNode> DoAlign(bool bUseExtraSpaces = true);
    std::unique_ptr<SmStructureNode> DoFontAttribute();
    std::unique_ptr<SmStructureNode> DoAttribute();
    std::unique_ptr<SmStructureNode> DoFont();
    std::unique_ptr<SmStructureNode> DoFontSize();
    std::unique_ptr<SmStructureNode> DoColor();
    std::unique_ptr<SmStructureNode> DoBrace();
    std::unique_ptr<SmBracebodyNode> DoBracebody(bool bIsLeftRight);
    std::unique_ptr<SmNode> DoEvaluate();
    std::unique_ptr<SmTextNode> DoFunction();
    std::unique_ptr<SmTableNode> DoBinom();
    std::unique_ptr<SmBinVerNode> DoFrac();
    std::unique_ptr<SmStructureNode> DoStack();
    std::unique_ptr<SmStructureNode> DoMatrix();
    std::unique_ptr<SmSpecialNode> DoSpecial();
    std::unique_ptr<SmGlyphSpecialNode> DoGlyphSpecial();
    std::unique_ptr<SmExpressionNode> DoError(SmParseError Error);
    // end of grammar

public:
                 SmParser();

    /** Parse rBuffer to formula tree */
    std::unique_ptr<SmTableNode> Parse(const OUString &rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    std::unique_ptr<SmNode> ParseExpression(const OUString &rBuffer);

    const OUString & GetText() const { return m_aBufferString; };

    bool        IsImportSymbolNames() const        { return m_bImportSymNames; }
    void        SetImportSymbolNames(bool bVal)    { m_bImportSymNames = bVal; }
    bool        IsExportSymbolNames() const        { return m_bExportSymNames; }
    void        SetExportSymbolNames(bool bVal)    { m_bExportSymNames = bVal; }

    const SmErrorDesc*  NextError();
    const SmErrorDesc*  PrevError();
    const SmErrorDesc*  GetError();
    const std::set< OUString >&   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( TG nGroup)
{
    return bool(m_aCurToken.nGroup & nGroup);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
