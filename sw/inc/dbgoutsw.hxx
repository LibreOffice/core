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
#ifndef __DBGOUTSW_HXX
#define __DBGOUTSW_HXX

#ifdef DBG_UTIL

#include <boost/unordered_map.hpp>
#include <tox.hxx>
#include <cstdio>
#include "tools/string.hxx"

class SwNode;
class SwTxtAttr;
class SwpHints;
class SfxPoolItem;
class SfxItemSet;
struct SwPosition;
class SwPaM;
class SwNodeNum;
class SwUndo;
class SwRect;
class SwFrmFmt;
class SwFrmFmts;
class SwNodes;
class SwRewriter;
class SwNumRuleTbl;
class SwNumRule;
class SwOutlineNodes;
class SwTxtFmtColl;
class SwNodeRange;

extern bool bDbgOutStdErr;
extern bool bDbgOutPrintAttrSet;

SW_DLLPUBLIC const char * dbg_out(const void * pVoid);
SW_DLLPUBLIC const char * dbg_out(const String & aStr);
SW_DLLPUBLIC const char * dbg_out(const OUString & aStr);
SW_DLLPUBLIC const char * dbg_out(const SwRect & rRect);
SW_DLLPUBLIC const char * dbg_out(const SwFrmFmt & rFrmFmt);
SW_DLLPUBLIC const char * dbg_out(const SwNode & rNode);
SW_DLLPUBLIC const char * dbg_out(const SwTxtAttr & rAttr);
SW_DLLPUBLIC const char * dbg_out(const SwpHints &rHints);
SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem & rItem);
SW_DLLPUBLIC const char * dbg_out(const SfxPoolItem * pItem);
SW_DLLPUBLIC const char * dbg_out(const SfxItemSet & rSet);
SW_DLLPUBLIC const char * dbg_out(SwNodes & rNodes);
SW_DLLPUBLIC const char * dbg_out(const SwPosition & rPos);
SW_DLLPUBLIC const char * dbg_out(const SwPaM & rPam);
SW_DLLPUBLIC const char * dbg_out(const SwNodeNum & rNum);
SW_DLLPUBLIC const char * dbg_out(const SwUndo & rUndo);
SW_DLLPUBLIC const char * dbg_out(const SwRewriter & rRewriter);
SW_DLLPUBLIC const char * dbg_out(const SwNumRule & rRule);
SW_DLLPUBLIC const char * dbg_out(const SwTxtFmtColl & rFmt);
SW_DLLPUBLIC const char * dbg_out(const SwFrmFmts & rFrmFmts);
SW_DLLPUBLIC const char * dbg_out(const SwNumRuleTbl & rTbl);
SW_DLLPUBLIC const char * dbg_out(const SwNodeRange & rRange);

template<typename tKey, typename tMember, typename fHashFunction>
String lcl_dbg_out(const boost::unordered_map<tKey, tMember, fHashFunction> & rMap)
{
    String aResult("[", RTL_TEXTENCODING_ASCII_US);

    typename boost::unordered_map<tKey, tMember, fHashFunction>::const_iterator aIt;

    for (aIt = rMap.begin(); aIt != rMap.end(); aIt++)
    {
        if (aIt != rMap.begin())
            aResult += String(", ", RTL_TEXTENCODING_ASCII_US);

        aResult += aIt->first;

        char sBuffer[256];
        sprintf(sBuffer, "(%p)", aIt->second);
        aResult += String(sBuffer, RTL_TEXTENCODING_ASCII_US);
    }

    aResult += String("]", RTL_TEXTENCODING_ASCII_US);

    return aResult;
}

template<typename tKey, typename tMember, typename fHashFunction>
const char * dbg_out(const boost::unordered_map<tKey, tMember, fHashFunction> & rMap)
{
    return dbg_out(lcl_dbg_out(rMap));
}
SW_DLLPUBLIC const char * dbg_out(const SwFormToken & rToken);
SW_DLLPUBLIC const char * dbg_out(const SwFormTokens & rTokens);
#endif // DBG_UTIL
#endif // __DBGOUTSW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
