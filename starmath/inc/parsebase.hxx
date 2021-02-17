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

#ifndef INCLUDED_STARMATH_INC_PARSE_BASE_HXX
#define INCLUDED_STARMATH_INC_PARSE_BASE_HXX

#include <unotools/charclass.hxx>
#include <memory>
#include <set>
#include <vector>

#include <node.hxx>

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

// Submit error information
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

namespace starmathdatabase
{
// Must be in sync with SmParseError list
extern const char* SmParseErrorDesc[16];

OUString getParseErrorDesc(SmParseError err);
}

// Prevents out of stack errors
class SmDepthProtect
{
private:
    sal_Int32& m_rParseDepth;

public:
    SmDepthProtect(sal_Int32& rParseDepth)
        : m_rParseDepth(rParseDepth)
    {
        ++m_rParseDepth;
        if (m_rParseDepth > DEPTH_LIMIT)
            throw std::range_error("parser depth limit");
    }
    ~SmDepthProtect() { --m_rParseDepth; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
