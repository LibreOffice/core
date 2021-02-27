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

#ifndef INCLUDED_STARMATH_INC_PARSE6_HXX
#define INCLUDED_STARMATH_INC_PARSE6_HXX

#include "parsebase.hxx"

class SmParser6 : public AbstractSmParser
{
    OUString m_aBufferString;
    SmToken m_aCurToken;
    std::vector<SmErrorDesc> m_aErrDescList;
    int m_nCurError;
    sal_Int32 m_nBufferIndex, m_nTokenIndex;
    sal_Int32 m_nRow, // 1-based
        m_nColOff; // 0-based
    bool m_bImportSymNames, m_bExportSymNames;
    sal_Int32 m_nParseDepth;

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set<OUString> m_aUsedSymbols;

    // CharClass representing a locale for parsing numbers
    CharClass m_aNumCC;
    // pointer to System locale's CharClass, which is alive inside SM_MOD()
    const CharClass* m_pSysCC;

    SmParser6(const SmParser6&) = delete;
    SmParser6& operator=(const SmParser6&) = delete;

    // Moves between tokens inside starmath code.
    sal_Int32 GetTokenIndex() const { return m_nTokenIndex; }
    void Replace(sal_Int32 nPos, sal_Int32 nLen, const OUString& rText);

    inline bool TokenInGroup(TG nGroup);

    // grammar
    // end of grammar

public:
    SmParser6();
    virtual ~SmParser6();

    /** Parse rBuffer to formula tree */
    std::unique_ptr<SmTableNode> Parse(const OUString& rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    std::unique_ptr<SmNode> ParseExpression(const OUString& rBuffer);

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

inline bool SmParser6::TokenInGroup(TG nGroup) { return bool(m_aCurToken.nGroup & nGroup); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
