/*************************************************************************
 *
 *  $RCSfile: dbgoutsw.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-01 13:00:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
