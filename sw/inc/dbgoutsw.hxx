/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef __DBGOUTSW_HXX
#define __DBGOUTSW_HXX

#ifdef DEBUG

#include <boost/unordered_map.hpp>
#include <tox.hxx>
#include <cstdio>
class String;

namespace rtl
{
class OUString;
}
class SwNode;
class SwTxtAttr;
class SwpHints;
class SfxPoolItem;
class SfxItemSet;
struct SwPosition;
class SwPaM;
class SwNodeNum;
class SwUndo;
class SwUndos;
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

#define DBG_OUT_HERE printf("%s(%d):", __FILE__, __LINE__)
#define DBG_OUT_HERE_FN printf("%s(%d) %s:", __FILE__, __LINE__, __FUNCTION__)
#define DBG_OUT_HERE_LN printf("%s(%d)\n", __FILE__, __LINE__)
#define DBG_OUT_HERE_FN_LN printf("%s(%d) %s\n", __FILE__, __LINE__, __FUNCTION__)
#define DBG_OUT(x) printf("%s\n", dbg_out(x))
#define DBG_OUT_LN(x) printf("%s(%d): %s\n", __FILE__, __LINE__, dbg_out(x))
#define DBG_OUT_FN_LN(x) printf("%s: %s\n", __FUNCTION__, dbg_out(x))

extern bool bDbgOutStdErr;
extern bool bDbgOutPrintAttrSet;

SW_DLLPUBLIC const char * dbg_out(const void * pVoid);
SW_DLLPUBLIC const char * dbg_out(const String & aStr);
SW_DLLPUBLIC const char * dbg_out(const ::rtl::OUString & aStr);
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
SW_DLLPUBLIC const char * dbg_out(const SwUndos & rUndos);
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
#endif // DEBUG
#endif // __DBGOUTSW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
