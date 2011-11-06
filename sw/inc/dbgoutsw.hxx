/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef __DBGOUTSW_HXX
#define __DBGOUTSW_HXX

#ifdef DEBUG

#include <hash_map>
#include <tox.hxx>
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
// const char * dbg_out(SwOutlineNodes & rNodes);
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
SW_DLLPUBLIC const char * dbg_out(const SwFormToken & rToken);
SW_DLLPUBLIC const char * dbg_out(const SwFormTokens & rTokens);
#endif // DEBUG
#endif // __DBGOUTSW_HXX
