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

#ifndef INCLUDED_STARMATH_INC_PARSEBASE_HXX
#define INCLUDED_STARMATH_INC_PARSEBASE_HXX

#include <unotools/charclass.hxx>
#include <memory>
#include <set>
#include <vector>

#include "token.hxx"
#include "node.hxx"

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
    {
    }
};

class DepthProtect
{
private:
    sal_Int32& m_rParseDepth;

public:
    DepthProtect(sal_Int32& rParseDepth)
        : m_rParseDepth(rParseDepth)
    {
        ++m_rParseDepth;
        if (m_rParseDepth > DEPTH_LIMIT)
            throw std::range_error("parser depth limit");
    }
    ~DepthProtect() { --m_rParseDepth; }
};

namespace starmathdatabase
{
// Must be in sync with SmParseError list
extern const char* SmParseErrorDesc[16];

OUString getParseErrorDesc(SmParseError err);
}

class AbstractSmParser
{
public:
    AbstractSmParser() {}
    virtual ~AbstractSmParser() {}

    /** Parse rBuffer to formula tree */
    virtual std::unique_ptr<SmTableNode> Parse(const OUString& rBuffer) = 0;
    /** Parse rBuffer to formula subtree that constitutes an expression */
    virtual std::unique_ptr<SmNode> ParseExpression(const OUString& rBuffer) = 0;

    virtual const OUString& GetText() const = 0;

    virtual bool IsImportSymbolNames() const = 0;
    virtual void SetImportSymbolNames(bool bVal) = 0;
    virtual bool IsExportSymbolNames() const = 0;
    virtual void SetExportSymbolNames(bool bVal) = 0;

    virtual const SmErrorDesc* NextError() = 0;
    virtual const SmErrorDesc* PrevError() = 0;
    virtual const SmErrorDesc* GetError() const = 0;
    virtual const std::set<OUString>& GetUsedSymbols() const = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
