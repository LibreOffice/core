/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/** Parses the starmath code and creates the nodes.
  *
  */

#pragma once

#include "parsebase.hxx"

class SmParser6 : public AbstractSmParser
{
    OUString m_aBufferString;
    SmToken m_aCurToken;
    ESelection m_aCurESelection;
    std::vector<SmErrorDesc> m_aErrDescList;
    int m_nCurError;
    bool m_bImportSymNames, m_bExportSymNames;

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set<OUString> m_aUsedSymbols;

    // CharClass representing a locale for parsing numbers
    CharClass m_aNumCC;
    // pointer to System locale's CharClass, which is alive inside SM_MOD()
    const CharClass* m_pSysCC;

    SmParser6(const SmParser6&) = delete;
    SmParser6& operator=(const SmParser6&) = delete;

public:
    SmParser6();
    virtual ~SmParser6();

    /** Parse rBuffer to formula tree */
    std::unique_ptr<SmTableNode> Parse(const OUString& rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    std::unique_ptr<SmNode> ParseExpression(const OUString& rBuffer);

    /** Parse rBuffer to formula tree */
    SmMlElement* ParseML(const OUString& rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    SmMlElement* ParseExpressionML(const OUString& rBuffer);

    const OUString& GetText() const { return m_aBufferString; };

    bool IsImportSymbolNames() const { return m_bImportSymNames; }
    void SetImportSymbolNames(bool bVal) { m_bImportSymNames = bVal; }
    bool IsExportSymbolNames() const { return m_bExportSymNames; }
    void SetExportSymbolNames(bool bVal) { m_bExportSymNames = bVal; }

    const SmErrorDesc* NextError();
    const SmErrorDesc* PrevError();
    const SmErrorDesc* GetError() const;
    const std::set<OUString>& GetUsedSymbols() const { return m_aUsedSymbols; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
