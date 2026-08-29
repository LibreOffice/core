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

#include "xmlsheetviewi.hxx"
#include "xmlfilti.hxx"
#include "xmlimprt.hxx"

#include <document.hxx>
#include <rangeutl.hxx>

#include <sax/fastattribs.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLSheetViewsContext::ScXMLSheetViewsContext(ScXMLImport& rImport)
    : ScXMLImportContext(rImport)
{
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLSheetViewsContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    if (nElement != XML_ELEMENT(CO_EXT, XML_SHEET_VIEW))
        return nullptr;

    sax_fastparser::FastAttributeList* pAttribList
        = &sax_fastparser::castToFastAttributeList(xAttrList);
    return new ScXMLSheetViewContext(GetScImport(), pAttribList);
}

ScXMLSheetViewContext::ScXMLSheetViewContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList)
    : ScXMLImportContext(rImport)
{
    // A default ScQueryParam has no header row, and coext:contains-header is only written
    // when the range has none, so an absent attribute means the range has one.
    maData.maQueryParam.bHasHeader = true;

    OUString aRangeString;
    if (rAttrList.is())
    {
        for (auto& aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(CO_EXT, XML_NAME):
                    maData.maName = aIter.toString();
                    break;
                case XML_ELEMENT(CO_EXT, XML_TABLE_NAME):
                    maData.maTableName = aIter.toString();
                    break;
                case XML_ELEMENT(CO_EXT, XML_GUID):
                    maData.maGUID = OString(aIter.toView());
                    break;
                case XML_ELEMENT(CO_EXT, XML_FILTER_GUID):
                    maData.maFilterGUID = OString(aIter.toView());
                    break;
                case XML_ELEMENT(CO_EXT, XML_TARGET_RANGE_ADDRESS):
                    aRangeString = aIter.toString();
                    break;
                case XML_ELEMENT(CO_EXT, XML_CONTAINS_HEADER):
                    maData.maQueryParam.bHasHeader = IsXMLToken(aIter, XML_TRUE);
                    break;
            }
        }
    }

    // The filter conditions that follow give their field numbers relative to this range, so
    // the query has to know the range before the first condition is read.
    ScDocument* pDoc = GetScImport().GetDocument();
    if (aRangeString.isEmpty() || !pDoc)
        return;

    ScRange aRange;
    sal_Int32 nOffset = 0;
    if (!ScRangeStringConverter::GetRangeFromString(aRange, aRangeString, *pDoc,
                                                    formula::FormulaGrammar::CONV_OOO, nOffset))
        return;

    maData.mbHasFilterRange = true;
    maData.maQueryParam.nTab = aRange.aStart.Tab();
    maData.maQueryParam.nCol1 = aRange.aStart.Col();
    maData.maQueryParam.nRow1 = aRange.aStart.Row();
    maData.maQueryParam.nCol2 = aRange.aEnd.Col();
    maData.maQueryParam.nRow2 = aRange.aEnd.Row();
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLSheetViewContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    sax_fastparser::FastAttributeList* pAttribList
        = &sax_fastparser::castToFastAttributeList(xAttrList);

    switch (nElement)
    {
        case XML_ELEMENT(CO_EXT, XML_HIDDEN_COLUMNS):
            return new ScXMLSheetViewHiddenColumnsContext(GetScImport(), pAttribList, *this);
        case XML_ELEMENT(TABLE, XML_FILTER):
            // The auto-filter of a sheet view keeps all of its conditions inline.
            return new ScXMLFilterContext(GetScImport(), pAttribList, maData.maQueryParam, nullptr);
        case XML_ELEMENT(TABLE, XML_SORT):
            return new ScXMLSortContext(GetScImport(), pAttribList, this);
    }
    return nullptr;
}

void SAL_CALL ScXMLSheetViewContext::endFastElement(sal_Int32 /*nElement*/)
{
    if (maData.maTableName.isEmpty())
        return;
    GetScImport().AddSheetView(std::move(maData));
}

void ScXMLSheetViewContext::SetSortSequence(
    const cpo::uno::Sequence<beans::PropertyValue>& rSortSequence)
{
    maData.maSortSequence = rSortSequence;
    maData.mbHasSort = true;
}

void ScXMLSheetViewContext::AddHiddenColumns(SCCOL nStartColumn, SCCOL nEndColumn)
{
    maData.maHiddenColumns.emplace_back(nStartColumn, nEndColumn);
}

ScXMLSheetViewHiddenColumnsContext::ScXMLSheetViewHiddenColumnsContext(
    ScXMLImport& rImport, const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
    ScXMLSheetViewContext& rSheetView)
    : ScXMLImportContext(rImport)
{
    if (!rAttrList.is())
        return;

    SCCOL nStartColumn = -1;
    SCCOL nEndColumn = -1;
    for (auto& aIter : *rAttrList)
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(CO_EXT, XML_START_COLUMN):
                nStartColumn = static_cast<SCCOL>(aIter.toInt32());
                break;
            case XML_ELEMENT(CO_EXT, XML_END_COLUMN):
                nEndColumn = static_cast<SCCOL>(aIter.toInt32());
                break;
        }
    }

    const ScDocument* pDoc = GetScImport().GetDocument();
    if (!pDoc || nStartColumn < 0 || nEndColumn < nStartColumn || nEndColumn > pDoc->MaxCol())
        return;

    rSheetView.AddHiddenColumns(nStartColumn, nEndColumn);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
