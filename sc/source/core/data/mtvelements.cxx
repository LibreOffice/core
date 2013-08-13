/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mtvelements.hxx"
#include "globalnames.hxx"
#include "document.hxx"

namespace sc {

CellTextAttr::CellTextAttr() :
    mnTextWidth(TEXTWIDTH_DIRTY),
    mnScriptType(SC_SCRIPTTYPE_UNKNOWN) {}

CellTextAttr::CellTextAttr(const CellTextAttr& r) :
    mnTextWidth(r.mnTextWidth),
    mnScriptType(r.mnScriptType) {}

CellTextAttr::CellTextAttr(sal_uInt16 nTextWidth, sal_uInt8 nScriptType) :
    mnTextWidth(nTextWidth),
    mnScriptType(nScriptType) {}

ColumnBlockPositionSet::ColumnBlockPositionSet(ScDocument& rDoc) : mrDoc(rDoc) {}

ColumnBlockPosition* ColumnBlockPositionSet::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    TablesType::iterator itTab = maTables.find(nTab);
    if (itTab == maTables.end())
    {
        std::pair<TablesType::iterator,bool> r =
            maTables.insert(TablesType::value_type(nTab, ColumnsType()));
        if (!r.second)
            // insertion failed.
            return NULL;

        itTab = r.first;
    }

    ColumnsType& rCols = itTab->second;

    ColumnsType::iterator it = rCols.find(nCol);
    if (it != rCols.end())
        // Block position for this column has already been fetched.
        return &it->second;

    std::pair<ColumnsType::iterator,bool> r =
        rCols.insert(
            ColumnsType::value_type(nCol, ColumnBlockPosition()));

    if (!r.second)
        // insertion failed.
        return NULL;

    it = r.first;

    if (!mrDoc.InitColumnBlockPosition(it->second, nTab, nCol))
        return NULL;

    return &it->second;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
