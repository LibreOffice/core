/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbgoutsw.hxx,v $
 * $Revision: 1.17 $
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

#include <hash_map>
#include <tox.hxx>
class String;
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

#define DBG_OUT_HERE printf("%s(%d):", __FILE__, __LINE__)
#define DBG_OUT_HERE_FN printf("%s(%d) %s:", __FILE__, __LINE__, __FUNCTION__)
#define DBG_OUT_HERE_LN printf("%s(%d)\n", __FILE__, __LINE__)
#define DBG_OUT_HERE_FN_LN printf("%s(%d) %s\n", __FILE__, __LINE__, __FUNCTION__)
#define DBG_OUT(x) printf("%s\n", dbg_out(x))
#define DBG_OUT_LN(x) printf("%s(%d): %s\n", __FILE__, __LINE__, dbg_out(x))
#define DBG_OUT_FN_LN(x) printf("%s: %s\n", __FUNCTION__, dbg_out(x))

extern bool bDbgOutStdErr;
extern bool bDbgOutPrintAttrSet;

const char * dbg_out(const void * pVoid);
const char * dbg_out(const String & aStr);
const char * dbg_out(const SwRect & rRect);
const char * dbg_out(const SwFrmFmt & rFrmFmt);
const char * dbg_out(const SwNode & rNode);
const char * dbg_out(const SwTxtAttr & rAttr);
const char * dbg_out(const SwpHints &rHints);
const char * dbg_out(const SfxPoolItem & rItem);
const char * dbg_out(const SfxPoolItem * pItem);
const char * dbg_out(const SfxItemSet & rSet);
const char * dbg_out(SwNodes & rNodes);
// const char * dbg_out(SwOutlineNodes & rNodes);
const char * dbg_out(const SwPosition & rPos);
const char * dbg_out(const SwPaM & rPam);
const char * dbg_out(const SwNodeNum & rNum);
const char * dbg_out(const SwUndo & rUndo);
const char * dbg_out(const SwUndos & rUndos);
const char * dbg_out(const SwRewriter & rRewriter);
const char * dbg_out(const SwNumRule & rRule);
const char * dbg_out(const SwTxtFmtColl & rFmt);
const char * dbg_out(const SwFrmFmts & rFrmFmts);
const char * dbg_out(const SwNumRuleTbl & rTbl);

template<typename tKey, typename tMember, typename fHashFunction>
String lcl_dbg_out(const std::hash_map<tKey, tMember, fHashFunction> & rMap)
{
    String aResult("[", RTL_TEXTENCODING_ASCII_US);

    typename std::hash_map<tKey, tMember, fHashFunction>::const_iterator aIt;

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
const char * dbg_out(const std::hash_map<tKey, tMember, fHashFunction> & rMap)
{
    return dbg_out(lcl_dbg_out(rMap));
}
const char * dbg_out(const SwFormToken & rToken);
const char * dbg_out(const SwFormTokens & rTokens);
#endif // DEBUG
#endif // __DBGOUTSW_HXX
