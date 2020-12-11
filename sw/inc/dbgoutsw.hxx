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
#ifndef INCLUDED_SW_INC_DBGOUTSW_HXX
#define INCLUDED_SW_INC_DBGOUTSW_HXX

#ifdef DBG_UTIL

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include "tox.hxx"
#include <cstdio>
#include <string_view>
#include <unordered_map>

class SwContentNode;
class SwNode;
class SwTextAttr;
class SwTextNode;
class SwpHints;
class SfxPoolItem;
class SfxItemSet;
struct SwPosition;
class SwPaM;
class SwNodeNum;
class SwUndo;
class SwRect;
class SwFrameFormat;
class SwFrameFormats;
class SwNumRuleTable;
class SwNumRule;
class SwOutlineNodes;
class SwTextFormatColl;
class SwNodeRange;

extern bool bDbgOutStdErr;
extern bool bDbgOutPrintAttrSet;

const char* dbg_out(const void* pVoid);
const char* dbg_out(std::u16string_view aStr);
const char* dbg_out(const SwRect& rRect);
const char* dbg_out(const SwFrameFormat& rFrameFormat);
SW_DLLPUBLIC const char* dbg_out(const SwNode& rNode);
SW_DLLPUBLIC const char* dbg_out(const SwNode* pNode);
const char* dbg_out(const SwContentNode* pNode);
const char* dbg_out(const SwTextNode* pNode);
const char* dbg_out(const SwTextAttr& rAttr);
const char* dbg_out(const SwpHints& rHints);
const char* dbg_out(const SfxPoolItem& rItem);
const char* dbg_out(const SfxPoolItem* pItem);
const char* dbg_out(const SfxItemSet& rSet);
const char* dbg_out(const SwPosition& rPos);
const char* dbg_out(const SwPaM& rPam);
const char* dbg_out(const SwNodeNum& rNum);
const char* dbg_out(const SwUndo& rUndo);
const char* dbg_out(SwOutlineNodes const& rNodes);
const char* dbg_out(const SwNumRule& rRule);
const char* dbg_out(const SwTextFormatColl& rFormat);
const char* dbg_out(const SwFrameFormats& rFrameFormats);
const char* dbg_out(const SwNumRuleTable& rTable);
const char* dbg_out(const SwNodeRange& rRange);

template <typename tKey, typename tMember, typename fHashFunction>
OUString lcl_dbg_out(const std::unordered_map<tKey, tMember, fHashFunction>& rMap)
{
    OUStringBuffer aResult("[");

    typename std::unordered_map<tKey, tMember, fHashFunction>::const_iterator aIt;

    for (aIt = rMap.begin(); aIt != rMap.end(); ++aIt)
    {
        if (aIt != rMap.begin())
            aResult.append(", ");

        aResult += aIt->first;

        char sBuffer[256];
        sprintf(sBuffer, "(%p)", aIt->second);
        aResult.appendAscii(sBuffer);
    }

    aResult.append("]");

    return aResult.makeStringAndClear();
}

template <typename tKey, typename tMember, typename fHashFunction>
const char* dbg_out(const std::unordered_map<tKey, tMember, fHashFunction>& rMap)
{
    return dbg_out(lcl_dbg_out(rMap));
}
const char* dbg_out(const SwFormToken& rToken);
const char* dbg_out(const SwFormTokens& rTokens);
#endif // DBG_UTIL
#endif // INCLUDED_SW_INC_DBGOUTSW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
