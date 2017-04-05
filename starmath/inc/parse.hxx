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
#ifndef INCLUDED_STARMATH_INC_PARSE_HXX
#define INCLUDED_STARMATH_INC_PARSE_HXX

#include <unotools/charclass.hxx>
#include <memory>
#include <set>
#include <vector>

#include "types.hxx"
#include "token.hxx"
#include "error.hxx"
#include "node.hxx"

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

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set< OUString >   m_aUsedSymbols;

    // CharClass representing a locale for parsing numbers
    CharClass m_aNumCC;
    // pointer to System locale's CharClass, which is alive inside SM_MOD()
    const CharClass* m_pSysCC;

    SmParser(const SmParser&) = delete;
    SmParser& operator=(const SmParser&) = delete;

    void            NextToken();
    sal_Int32       GetTokenIndex() const   { return m_nTokenIndex; }
    void            Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText );

    inline bool     TokenInGroup( TG nGroup );

    // grammar
    SmTableNode *DoTable();
    SmLineNode *DoLine();
    SmNode *DoExpression(bool bUseExtraSpaces = true);
    SmNode *DoRelation();
    SmNode *DoSum();
    SmNode *DoProduct();
    SmNode *DoSubSup(TG nActiveGroup, SmNode *pGivenNode);
    SmNode *DoOpSubSup();
    SmNode *DoPower();
    SmBlankNode *DoBlank();
    SmNode *DoTerm(bool bGroupNumberIdent);
    SmNode *DoEscape();
    SmOperNode *DoOperator();
    SmNode *DoOper();
    SmStructureNode *DoUnOper();
    SmNode *DoAlign(bool bUseExtraSpaces = true);
    SmStructureNode *DoFontAttribut();
    SmAttributNode *DoAttribut();
    SmStructureNode *DoFont();
    SmStructureNode *DoFontSize();
    SmStructureNode *DoColor();
    SmStructureNode *DoBrace();
    SmBracebodyNode *DoBracebody(bool bIsLeftRight);
    SmTextNode *DoFunction();
    SmTableNode *DoBinom();
    SmStructureNode *DoStack();
    SmStructureNode *DoMatrix();
    SmSpecialNode *DoSpecial();
    SmGlyphSpecialNode *DoGlyphSpecial();
    SmExpressionNode *DoError(SmParseError Error);
    // end of grammar

public:
                 SmParser();

    /** Parse rBuffer to formula tree */
    SmTableNode *Parse(const OUString &rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    SmNode      *ParseExpression(const OUString &rBuffer);

    const OUString & GetText() const { return m_aBufferString; };

    bool        IsImportSymbolNames() const        { return m_bImportSymNames; }
    void        SetImportSymbolNames(bool bVal)    { m_bImportSymNames = bVal; }
    bool        IsExportSymbolNames() const        { return m_bExportSymNames; }
    void        SetExportSymbolNames(bool bVal)    { m_bExportSymNames = bVal; }

    void        AddError(SmParseError Type, SmNode *pNode);
    const SmErrorDesc*  NextError();
    const SmErrorDesc*  PrevError();
    const SmErrorDesc*  GetError(size_t i);
    static const SmTokenTableEntry* GetTokenTableEntry( const OUString &rName );
    const std::set< OUString >&   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( TG nGroup)
{
    return bool(m_aCurToken.nGroup & nGroup);
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
