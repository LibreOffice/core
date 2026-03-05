/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <export/NamedSheetViews.hxx>
#include <export/ExportTools.hxx>

#include <document.hxx>
#include <dbdata.hxx>
#include <queryentry.hxx>
#include <queryparam.hxx>
#include <sortparam.hxx>
#include <SheetView.hxx>
#include <SheetViewManager.hxx>

#include <xestream.hxx>
#include <xestyle.hxx>

#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/export/utils.hxx>
#include <comphelper/xmltools.hxx>

using namespace oox;

namespace
{
/** Writes filter XML elements for query entries on a single column. */
class ColumnFilterWriter
{
public:
    ColumnFilterWriter(const sax_fastparser::FSHelperPtr& pStream,
                       const std::vector<const ScQueryEntry*>& rEntries, const XclExpDxfs& rDxfs)
        : mpStream(pStream)
        , mrEntries(rEntries)
        , mrDxfs(rDxfs)
    {
    }

    void write()
    {
        if (mrEntries.empty())
            return;

        const ScQueryEntry& rFirst = *mrEntries[0];

        if (rFirst.eOp == SC_TOPVAL || rFirst.eOp == SC_BOTVAL || rFirst.eOp == SC_TOPPERC
            || rFirst.eOp == SC_BOTPERC)
            writeTop10(rFirst);
        else if (rFirst.IsQueryByTextColor() || rFirst.IsQueryByBackgroundColor())
            writeColor(rFirst);
        else if (rFirst.eOp == SC_EQUAL)
            writeDiscrete();
        else
            writeCustom();
    }

private:
    static const char* getOperator(ScQueryOp eOp)
    {
        switch (eOp)
        {
            case SC_EQUAL:
                return "equal";
            case SC_LESS:
                return "lessThan";
            case SC_GREATER:
                return "greaterThan";
            case SC_LESS_EQUAL:
                return "lessThanOrEqual";
            case SC_GREATER_EQUAL:
                return "greaterThanOrEqual";
            case SC_NOT_EQUAL:
                return "notEqual";
            default:
                return nullptr;
        }
    }

    void writeTop10(const ScQueryEntry& rEntry)
    {
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (rItems.empty())
            return;

        bool bTop = (rEntry.eOp == SC_TOPVAL || rEntry.eOp == SC_TOPPERC);
        bool bPercent = (rEntry.eOp == SC_TOPPERC || rEntry.eOp == SC_BOTPERC);
        mpStream->singleElement(FSNS(XML_x, XML_top10), XML_top, ToPsz(bTop), XML_percent,
                                ToPsz(bPercent), XML_val,
                                OString::number(sal_Int32(rItems[0].mfVal)));
    }

    void writeColor(const ScQueryEntry& rEntry)
    {
        const ScQueryEntry::QueryItemsType& rItems = rEntry.GetQueryItems();
        if (rItems.empty())
            return;

        bool bCellColor = rEntry.IsQueryByBackgroundColor();
        sal_Int32 nDxfId = mrDxfs.GetDxfByColor(rItems[0].maColor);
        mpStream->singleElement(FSNS(XML_x, XML_colorFilter), XML_dxfId, OString::number(nDxfId),
                                XML_cellColor, OString::number(bCellColor ? 1 : 0));
    }

    void writeDiscrete()
    {
        const ScQueryEntry& rFirst = *mrEntries[0];

        bool bHasBlank = false;
        std::vector<OString> aValues;
        std::vector<OString> aDateValues;

        for (const auto* pEntry : mrEntries)
        {
            const ScQueryEntry::QueryItemsType& rItems = pEntry->GetQueryItems();
            for (const auto& rItem : rItems)
            {
                if (rItem.maString.isEmpty())
                    bHasBlank = true;
                else if (rItem.meType == ScQueryEntry::ByDate)
                    aDateValues.push_back(
                        OUStringToOString(rItem.maString.getString(), RTL_TEXTENCODING_UTF8));
                else
                    aValues.push_back(
                        OUStringToOString(rItem.maString.getString(), RTL_TEXTENCODING_UTF8));
            }
        }

        if (rFirst.IsQueryByEmpty())
            bHasBlank = true;

        if (bHasBlank)
            mpStream->startElement(FSNS(XML_x, XML_filters), XML_blank, "1");
        else
            mpStream->startElement(FSNS(XML_x, XML_filters));

        for (const auto& rValue : aValues)
        {
            mpStream->singleElement(FSNS(XML_x, XML_filter), XML_val, rValue);
        }

        for (const auto& rDateValue : aDateValues)
        {
            oox::xls::writeDateGroupItem(mpStream, FSNS(XML_x, XML_dateGroupItem), rDateValue);
        }

        mpStream->endElement(FSNS(XML_x, XML_filters));
    }

    void writeCustom()
    {
        const char* pOperator = getOperator(mrEntries[0]->eOp);
        if (!pOperator)
            return;

        bool bAnd = (mrEntries.size() > 1 && mrEntries[1]->eConnect == SC_AND);
        mpStream->startElement(FSNS(XML_x, XML_customFilters), XML_and, ToPsz(bAnd));

        for (const auto* pEntry : mrEntries)
        {
            const char* pEntryOperator = getOperator(pEntry->eOp);
            if (!pEntryOperator)
                continue;

            const ScQueryEntry::QueryItemsType& rItems = pEntry->GetQueryItems();
            OString aValue;
            if (!rItems.empty())
                aValue = OUStringToOString(rItems[0].maString.getString(), RTL_TEXTENCODING_UTF8);

            mpStream->singleElement(FSNS(XML_x, XML_customFilter), XML_operator, pEntryOperator,
                                    XML_val, aValue);
        }

        mpStream->endElement(FSNS(XML_x, XML_customFilters));
    }

    const sax_fastparser::FSHelperPtr& mpStream;
    const std::vector<const ScQueryEntry*>& mrEntries;
    const XclExpDxfs& mrDxfs;
};

} // anonymous namespace

xcl::exp::NamedSheetViews::NamedSheetViews(const XclExpRoot& rRoot, SCTAB nTab)
    : XclExpRoot(rRoot)
    , mnTab(nTab)
    , mbHasSheetViews(false)
{
    auto pManager = GetDoc().GetSheetViewManager(nTab);
    if (pManager && !pManager->isEmpty())
        mbHasSheetViews = true;
}

void xcl::exp::NamedSheetViews::saveColumnFilters(const sax_fastparser::FSHelperPtr& pStream,
                                                  const ScQueryParam& rQueryParam)
{
    // Group query entries by column
    std::map<SCCOLROW, std::vector<const ScQueryEntry*>> aColumnEntries;
    bool bHasFilters = false;

    for (SCSIZE i = 0; i < rQueryParam.GetEntryCount(); ++i)
    {
        const ScQueryEntry& rEntry = rQueryParam.GetEntry(i);
        if (!rEntry.bDoQuery)
            break;
        aColumnEntries[rEntry.nField].push_back(&rEntry);
        bHasFilters = true;
    }

    if (!bHasFilters)
        return;

    for (const auto & [ nCol, rEntries ] : aColumnEntries)
    {
        pStream->startElement(XML_columnFilter, XML_colId, OString::number(nCol));
        pStream->startElement(XML_filter, XML_colId, OString::number(nCol));
        ColumnFilterWriter(pStream, rEntries, GetDxfs()).write();
        pStream->endElement(XML_filter);
        pStream->endElement(XML_columnFilter);
    }
}

void xcl::exp::NamedSheetViews::saveSortRules(const sax_fastparser::FSHelperPtr& pStream,
                                              const ScRange& rRange, SCTAB nViewTab,
                                              const ScSortParam& rSortParam)
{
    bool bHasSort = false;
    for (const auto& rKey : rSortParam.maKeyState)
    {
        if (rKey.bDoSort)
        {
            bHasSort = true;
            break;
        }
    }

    if (!bHasSort)
        return;

    pStream->startElement(XML_sortRules, XML_caseSensitive, ToPsz(rSortParam.bCaseSens));

    for (const auto& rKey : rSortParam.maKeyState)
    {
        if (!rKey.bDoSort)
            continue;

        OString aSortRuleGUID = comphelper::xml::generateGUIDString();

        pStream->startElement(XML_sortRule, XML_colId, OString::number(rKey.nField), XML_id,
                              aSortRuleGUID);

        // Build the column range reference for sortCondition
        ScRange aSortColRange;
        aSortColRange.aStart.SetCol(SCCOL(rKey.nField));
        aSortColRange.aStart.SetRow(rRange.aStart.Row());
        aSortColRange.aStart.SetTab(nViewTab);
        aSortColRange.aEnd.SetCol(SCCOL(rKey.nField));
        aSortColRange.aEnd.SetRow(rRange.aEnd.Row());
        aSortColRange.aEnd.SetTab(nViewTab);

        OString aSortReference = XclXmlUtils::ToOString(GetDoc(), aSortColRange);

        std::optional<OString> sDescending;
        if (!rKey.bAscending)
            sDescending = "1";

        pStream->singleElement(XML_sortCondition, XML_ref, aSortReference, XML_descending,
                               sDescending);

        pStream->endElement(XML_sortRule);
    }

    pStream->endElement(XML_sortRules);
}

void xcl::exp::NamedSheetViews::saveSheetView(const sax_fastparser::FSHelperPtr& pStream,
                                              const sc::SheetView& rSheetView)
{
    OString aGUID = comphelper::xml::generateGUIDString();

    pStream->startElement(XML_namedSheetView, XML_name, rSheetView.GetName().toUtf8(), XML_id,
                          aGUID);

    // Get the view tab to access its filter/sort data
    SCTAB nViewTab = rSheetView.getTableNumber();
    ScDBData* pDBData = GetDoc().GetAnonymousDBData(nViewTab);

    if (pDBData)
    {
        ScRange aRange;
        pDBData->GetArea(aRange);
        aRange.aStart.SetTab(nViewTab);
        aRange.aEnd.SetTab(nViewTab);

        OString aRangeString = XclXmlUtils::ToOString(GetDoc(), aRange);
        OString aFilterGUID = comphelper::xml::generateGUIDString();

        pStream->startElement(XML_nsvFilter, XML_filterId, aFilterGUID, XML_ref, aRangeString,
                              XML_tableId, "0");

        ScQueryParam aQueryParam;
        pDBData->GetQueryParam(aQueryParam);
        saveColumnFilters(pStream, aQueryParam);

        ScSortParam aSortParam;
        pDBData->GetSortParam(aSortParam);
        saveSortRules(pStream, aRange, nViewTab, aSortParam);

        pStream->endElement(XML_nsvFilter);
    }

    pStream->endElement(XML_namedSheetView);
}

void xcl::exp::NamedSheetViews::SaveXml(XclExpXmlStream& rStream)
{
    if (!mbHasSheetViews)
        return;

    auto pManager = GetDoc().GetSheetViewManager(mnTab);
    if (!pManager || pManager->isEmpty())
        return;

    // Create output stream for the namedSheetView part
    sax_fastparser::FSHelperPtr pWorksheetStream = rStream.GetCurrentStream();

    OUString sFullPath
        = XclXmlUtils::GetStreamName("xl/namedSheetViews/", "namedSheetView", mnTab + 1);
    OUString sRelPath
        = XclXmlUtils::GetStreamName("../namedSheetViews/", "namedSheetView", mnTab + 1);

    sax_fastparser::FSHelperPtr pNamedSheetViewStream = rStream.CreateOutputStream(
        sFullPath, sRelPath, pWorksheetStream->getOutputStream(),
        "application/vnd.ms-excel.namedSheetViews+xml",
        u"http://schemas.microsoft.com/office/2019/04/relationships/namedSheetView");

    rStream.PushStream(pNamedSheetViewStream);

    pNamedSheetViewStream->startElement(
        XML_namedSheetViews, XML_xmlns, rStream.getNamespaceURL(OOX_NS(xnsv)),
        FSNS(XML_xmlns, XML_x), rStream.getNamespaceURL(OOX_NS(xls)), FSNS(XML_xmlns, XML_x14),
        rStream.getNamespaceURL(OOX_NS(xls14Lst)));

    for (auto& rSheetView : pManager->iterateValidSheetViews())
        saveSheetView(pNamedSheetViewStream, rSheetView);

    pNamedSheetViewStream->endElement(XML_namedSheetViews);

    rStream.PopStream();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
