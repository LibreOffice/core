/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *         Kohei Yoshida <kyoshida@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "cachedattraccess.hxx"
#include "document.hxx"

ScXMLCachedRowAttrAccess::Cache::Cache() :
    mnTab(-1), mnRow1(-1), mnRow2(-1), mbValue(false) {}

bool ScXMLCachedRowAttrAccess::Cache::hasCache(sal_Int32 nTab, sal_Int32 nRow) const
{
    return mnTab == nTab && mnRow1 <= nRow && nRow <= mnRow2;
}

ScXMLCachedRowAttrAccess::ScXMLCachedRowAttrAccess(ScDocument* pDoc) :
    mpDoc(pDoc) {}

bool ScXMLCachedRowAttrAccess::rowHidden(sal_Int32 nTab, sal_Int32 nRow)
{
    if (!maHidden.hasCache(nTab, nRow))
    {
        SCROW nRow1, nRow2;
        maHidden.mbValue = mpDoc->RowHidden(
            static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), &nRow1, &nRow2);
        maHidden.mnRow1 = static_cast<sal_Int32>(nRow1);
        maHidden.mnRow2 = static_cast<sal_Int32>(nRow2);
    }
    return maHidden.mbValue;
}

bool ScXMLCachedRowAttrAccess::rowFiltered(sal_Int32 nTab, sal_Int32 nRow)
{
    if (!maFiltered.hasCache(nTab, nRow))
    {
        SCROW nRow1, nRow2;
        maFiltered.mbValue = mpDoc->RowFiltered(
            static_cast<SCROW>(nRow), static_cast<SCTAB>(nTab), &nRow1, &nRow2);
        maFiltered.mnRow1 = static_cast<sal_Int32>(nRow1);
        maFiltered.mnRow2 = static_cast<sal_Int32>(nRow2);
    }
    return maFiltered.mbValue;
}

