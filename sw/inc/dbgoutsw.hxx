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
#include <tox.hxx>
#include <cstdio>
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
class SwNodes;
class SwRewriter;
class SwNumRuleTable;
class SwNumRule;
class SwOutlineNodes;
class SwTextFormatColl;
class SwNodeRange;

extern bool bDbgOutStdErr;
extern bool bDbgOutPrintAttrSet;

SW_DLLPUBLIC const char * dbg_out(const void * pVoid);
SW_DLLPUBLIC const char * dbg_out(const OUString & aStr);
SW_DLLPUBLIC const char * dbg_out(const SwRect & rRect);
SW_DLLPUBLIC const char * dbg_out(const SwFrameFormat & rFrameFormat);
SW_DLLPUBLIC const char * dbg_out(const SwNode & rNode);
SW_DLLPUBLIC const char * dbg_out(const SwNode * pNode);
SW_DLLPUBLIC const char * dbg_out(const SwContentNode * pNode);
SW_DLLPUBLIC const char * dbg_out(const SwTextNode * pNode);
SW_DLLPUBLIC const char * dbg_out(const SwTextAttr & rAttr);
SW_DLLPUBLIC const char * dbg_out(const SwpHints &rHints);
SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem & rItem);
SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem * pItem);
SW_DLLPUBLIC const char * dbg_out(const SfxItemSet & rSet);
SW_DLLPUBLIC const char * dbg_out(SwNodes & rNodes);
SW_DLLPUBLIC const char * dbg_out(const SwPosition & rPos);
SW_DLLPUBLIC const char * dbg_out(const SwPaM & rPam);
SW_DLLPUBLIC const char * dbg_out(const SwNodeNum & rNum);
SW_DLLPUBLIC const char * dbg_out(const SwUndo & rUndo);
SW_DLLPUBLIC const char * dbg_out(SwOutlineNodes & rNodes);
SW_DLLPUBLIC const char * dbg_out(const SwRewriter & rRewriter);
SW_DLLPUBLIC const char * dbg_out(const SwNumRule & rRule);
SW_DLLPUBLIC const char * dbg_out(const SwTextFormatColl & rFormat);
SW_DLLPUBLIC const char * dbg_out(const SwFrameFormats & rFrameFormats);
SW_DLLPUBLIC const char * dbg_out(const SwNumRuleTable & rTable);
SW_DLLPUBLIC const char * dbg_out(const SwNodeRange & rRange);

template<typename tKey, typename tMember, typename fHashFunction>
OUString lcl_dbg_out(const std::unordered_map<tKey, tMember, fHashFunction> & rMap)
{
    OUString aResult("[");

    typename std::unordered_map<tKey, tMember, fHashFunction>::const_iterator aIt;

    for (aIt = rMap.begin(); aIt != rMap.end(); aIt++)
    {
        if (aIt != rMap.begin())
            aResult += ", ";

        aResult += aIt->first;

        char sBuffer[256];
        sprintf(sBuffer, "(%p)", aIt->second);
        aResult += OUString(sBuffer, strlen(sBuffer), RTL_TEXTENCODING_ASCII_US);
    }

    aResult += "]";

    return aResult;
}

template<typename tKey, typename tMember, typename fHashFunction>
const char * dbg_out(const std::unordered_map<tKey, tMember, fHashFunction> & rMap)
{
    return dbg_out(lcl_dbg_out(rMap));
}
SW_DLLPUBLIC const char * dbg_out(const SwFormToken & rToken);
SW_DLLPUBLIC const char * dbg_out(const SwFormTokens & rTokens);
#endif // DBG_UTIL
#endif // INCLUDED_SW_INC_DBGOUTSW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
