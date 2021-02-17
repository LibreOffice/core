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

#include <parsebase.hxx>
#include <strings.hrc>
#include <smmod.hxx>

const char* starmathdatabase::SmParseErrorDesc[] = {
    // clang-format off
    RID_ERR_NONE,
    RID_ERR_UNEXPECTEDCHARACTER,
    RID_ERR_UNEXPECTEDTOKEN,
    RID_ERR_POUNDEXPECTED,
    RID_ERR_COLOREXPECTED,
    RID_ERR_LGROUPEXPECTED,
    RID_ERR_RGROUPEXPECTED,
    RID_ERR_LBRACEEXPECTED,
    RID_ERR_RBRACEEXPECTED,
    RID_ERR_PARENTMISMATCH,
    RID_ERR_RIGHTEXPECTED,
    RID_ERR_FONTEXPECTED,
    RID_ERR_SIZEEXPECTED,
    RID_ERR_DOUBLEALIGN,
    RID_ERR_DOUBLESUBSUPSCRIPT,
    RID_ERR_NUMBEREXPECTED
    // clang-format on
};

OUString starmathdatabase::getParseErrorDesc(SmParseError err)
{
    return SmResId(starmathdatabase::SmParseErrorDesc[static_cast<uint_fast8_t>(err)]);
}

AbstractSmParser::AbstractSmParser() {}

AbstractSmParser::~AbstractSmParser() {}

std::unique_ptr<SmTableNode> AbstractSmParser::Parse(const OUString&)
{
    return std::unique_ptr<SmTableNode>(nullptr);
}

std::unique_ptr<SmNode> AbstractSmParser::ParseExpression(const OUString&)
{
    return std::unique_ptr<SmTableNode>(nullptr);
}

const OUString m_string(u"");
const OUString& AbstractSmParser::GetText() const { return m_string; }

bool AbstractSmParser::IsImportSymbolNames() const { return false; }

void AbstractSmParser::SetImportSymbolNames(bool) {}

bool AbstractSmParser::IsExportSymbolNames() const { return false; }

void AbstractSmParser::SetExportSymbolNames(bool) {}

const SmErrorDesc* AbstractSmParser::NextError() { return nullptr; }

const SmErrorDesc* AbstractSmParser::PrevError() { return nullptr; }

const SmErrorDesc* AbstractSmParser::GetError() const { return nullptr; }

const std::set<OUString> m_aUsedSymbols;
const std::set<OUString>& AbstractSmParser::GetUsedSymbols() const { return m_aUsedSymbols; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
