/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/** Parses the starmath code and creates the nodes.
  * We rely on compiler's auto-inline functions.
  */

#pragma once

#include <unotools/charclass.hxx>

#include "parsebase.hxx"

class SmParser6 : public AbstractSmParser
{
private:
    // Buffer string (starmath code) and position on it
    OUString m_aBufferString;
    int_fast32_t m_nBufferIndex; // 0-based
    int_fast32_t m_nTokenIndex; // 0-based
    int_fast32_t m_nRow; // 1-based
    int_fast32_t m_nColOff; // 0-based

    // Actual token
    SmToken m_aCurToken;

    // Depth controllers
    int_fast32_t nDepth;

private:
    /**
      * Moves to the next token on normal conditions.
      */
    void NextToken();

    /**
      * Method for doing the parser
      */
    void ParseTokens();

public:
    SmParser6();
    virtual ~SmParser6() {}

    /** Parse rBuffer to formula tree */
    virtual std::unique_ptr<SmTableNode> Parse(const OUString& rBuffer) = 0;
    /** Parse rBuffer to formula subtree that constitutes an expression */
    virtual std::unique_ptr<SmNode> ParseExpression(const OUString& rBuffer) = 0;

    /**
      * Gives the text being parsed.
      * @return parsing text
      */
    virtual const OUString& GetText() const;

    virtual bool IsImportSymbolNames() const = 0;
    virtual void SetImportSymbolNames(bool bVal) = 0;
    virtual bool IsExportSymbolNames() const = 0;
    virtual void SetExportSymbolNames(bool bVal) = 0;

    virtual const SmErrorDesc* NextError() = 0;
    virtual const SmErrorDesc* PrevError() = 0;
    virtual const SmErrorDesc* GetError() const = 0;
    virtual const std::set<OUString>& GetUsedSymbols() const = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
