/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLExportSheetViews.hxx"
#include "XMLExportDatabaseRanges.hxx"
#include "xmlexprt.hxx"

#include <dbdata.hxx>
#include <document.hxx>
#include <queryparam.hxx>
#include <rangeutl.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>
#include <sortparam.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

using namespace xmloff::token;

namespace
{
/** Writes one coext:hidden-columns element per run of hidden columns on the table. */
void writeHiddenColumns(ScXMLExport& rExport, const ScDocument& rDoc, SCTAB nTab)
{
    for (SCCOL nColumn = 0; nColumn <= rDoc.MaxCol();)
    {
        SCCOL nLastColumn = nColumn;
        bool bHidden = rDoc.ColHidden(nColumn, nTab, nullptr, &nLastColumn);
        if (bHidden)
        {
            rExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_START_COLUMN, OUString::number(nColumn));
            rExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_END_COLUMN,
                                 OUString::number(nLastColumn));
            SvXMLElementExport aElement(rExport, XML_NAMESPACE_CO_EXT, XML_HIDDEN_COLUMNS, true,
                                        true);
        }
        nColumn = nLastColumn + 1;
    }
}
}

ScXMLExportSheetViews::ScXMLExportSheetViews(ScXMLExport& rExport)
    : mrExport(rExport)
{
}

void ScXMLExportSheetViews::WriteSheetViews(ScDocument& rDoc)
{
    SCTAB nTabCount = rDoc.GetTableCount();
    bool bHasSheetViews = false;
    for (SCTAB nTab = 0; nTab < nTabCount && !bHasSheetViews; ++nTab)
        bHasSheetViews = rDoc.HasSheetViews(nTab);
    if (!bHasSheetViews)
        return;

    SvXMLElementExport aSheetViews(mrExport, XML_NAMESPACE_CO_EXT, XML_SHEET_VIEWS, true, true);
    for (SCTAB nTab = 0; nTab < nTabCount; ++nTab)
    {
        // A holder table has no manager of its own, so only the sheet a view belongs to lists it.
        std::shared_ptr<sc::SheetViewManager> pManager = rDoc.GetSheetViewManager(nTab);
        if (!pManager || pManager->isEmpty())
            continue;

        OUString aTableName;
        rDoc.GetName(nTab, aTableName);
        for (sc::SheetView const& rView : pManager->iterateValidSheetViews())
            writeSheetView(rDoc, nTab, aTableName, rView);
    }
}

void ScXMLExportSheetViews::writeSheetView(ScDocument& rDoc, SCTAB nTab, const OUString& rTableName,
                                           const sc::SheetView& rView)
{
    SCTAB nViewTab = rView.getTableNumber();

    mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_NAME, rView.GetName());
    mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_TABLE_NAME, rTableName);
    if (!rView.GetGUID().isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_GUID,
                              OStringToOUString(rView.GetGUID(), RTL_TEXTENCODING_ASCII_US));
    if (!rView.GetFilterGUID().isEmpty())
        mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_FILTER_GUID,
                              OStringToOUString(rView.GetFilterGUID(), RTL_TEXTENCODING_ASCII_US));

    // The holder table is not in the file, so its auto-filter range is written against the
    // sheet the view belongs to.
    const ScDBData* pDBData = rDoc.GetAnonymousDBData(nViewTab);
    ScRange aRange;
    if (pDBData)
    {
        pDBData->GetArea(aRange);
        aRange.aStart.SetTab(nTab);
        aRange.aEnd.SetTab(nTab);
        OUString aRangeString;
        ScRangeStringConverter::GetStringFromRange(aRangeString, aRange, &rDoc,
                                                   formula::FormulaGrammar::CONV_OOO);
        mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_TARGET_RANGE_ADDRESS, aRangeString);

        ScQueryParam aQueryParam;
        pDBData->GetQueryParam(aQueryParam);
        if (!aQueryParam.bHasHeader)
            mrExport.AddAttribute(XML_NAMESPACE_CO_EXT, XML_CONTAINS_HEADER, XML_FALSE);
    }

    SvXMLElementExport aSheetView(mrExport, XML_NAMESPACE_CO_EXT, XML_SHEET_VIEW, true, true);

    writeHiddenColumns(mrExport, rDoc, nViewTab);

    if (pDBData)
    {
        writeFilter(mrExport, rDoc, *pDBData);
        if (const ScSortParam* pSortParam = rView.getSortParam())
            writeSort(mrExport, *pSortParam, aRange, rDoc);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
