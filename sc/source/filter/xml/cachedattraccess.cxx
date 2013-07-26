/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
