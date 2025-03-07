/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus_autofilter.hxx>
#include <orcusinterface.hxx>
#include <dbdata.hxx>
#include <globalnames.hxx>
#include <queryparam.hxx>

#include <svl/sharedstringpool.hxx>

#include <memory>

namespace os = orcus::spreadsheet;

namespace
{
ScQueryConnect toQueryConnect(os::auto_filter_node_op_t op)
{
    switch (op)
    {
        case os::auto_filter_node_op_t::op_and:
            return SC_AND;
        case os::auto_filter_node_op_t::op_or:
            return SC_OR;
        case os::auto_filter_node_op_t::unspecified:;
    }

    throw std::runtime_error("filter node operator type is not specified");
}

void setQueryOp(ScQueryEntry& rEntry, os::auto_filter_op_t op)
{
    switch (op)
    {
        case os::auto_filter_op_t::empty:
            rEntry.bDoQuery = true;
            rEntry.SetQueryByEmpty();
            break;
        case os::auto_filter_op_t::not_empty:
            rEntry.bDoQuery = true;
            rEntry.SetQueryByNonEmpty();
            break;
        case os::auto_filter_op_t::equal:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_EQUAL;
            break;
        case os::auto_filter_op_t::not_equal:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_NOT_EQUAL;
            break;
        case os::auto_filter_op_t::contain:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_CONTAINS;
            break;
        case os::auto_filter_op_t::not_contain:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_DOES_NOT_CONTAIN;
            break;
        case os::auto_filter_op_t::begin_with:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_BEGINS_WITH;
            break;
        case os::auto_filter_op_t::not_begin_with:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_DOES_NOT_BEGIN_WITH;
            break;
        case os::auto_filter_op_t::end_with:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_ENDS_WITH;
            break;
        case os::auto_filter_op_t::not_end_with:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_DOES_NOT_END_WITH;
            break;
        case os::auto_filter_op_t::greater:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_GREATER;
            break;
        case os::auto_filter_op_t::greater_equal:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_GREATER_EQUAL;
            break;
        case os::auto_filter_op_t::less:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_LESS;
            break;
        case os::auto_filter_op_t::less_equal:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_LESS_EQUAL;
            break;
        case os::auto_filter_op_t::top:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_TOPVAL;
            break;
        case os::auto_filter_op_t::bottom:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_BOTVAL;
            break;
        case os::auto_filter_op_t::top_percent:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_TOPPERC;
            break;
        case os::auto_filter_op_t::bottom_percent:
            rEntry.bDoQuery = true;
            rEntry.eOp = SC_BOTPERC;
            break;
        case os::auto_filter_op_t::top_percent_range:
        case os::auto_filter_op_t::bottom_percent_range:
        case os::auto_filter_op_t::unspecified:
            rEntry.bDoQuery = false;
            break;
    }
}
}

ScOrcusAutoFilterMultiValues::ScOrcusAutoFilterMultiValues(ScDocument& rDoc,
                                                           const ScOrcusGlobalSettings& rGS)
    : mrDoc(rDoc)
    , mrGlobalSettings(rGS)
{
}

void ScOrcusAutoFilterMultiValues::add_value(std::string_view value)
{
    OUString aStr(value.data(), value.size(), mrGlobalSettings.getTextEncoding());

    ScQueryEntry::Item aItem;
    aItem.maString = mrDoc.GetSharedStringPool().intern(aStr);
    aItem.meType = ScQueryEntry::ByString;

    maEntry.GetQueryItems().push_back(aItem);
}

void ScOrcusAutoFilterMultiValues::commit() { maCommitFunc(std::move(maEntry)); }

void ScOrcusAutoFilterMultiValues::reset(commitFuncType func)
{
    maEntry.Clear();
    maEntry.bDoQuery = true;
    maCommitFunc = std::move(func);
}

ScOrcusAutoFilterNode::ScOrcusAutoFilterNode(ScDocument& rDoc, const ScOrcusGlobalSettings& rGS)
    : mrDoc(rDoc)
    , mrGlobalSettings(rGS)
    , maMultiValues(rDoc, rGS)
{
}

void ScOrcusAutoFilterNode::append_item(os::col_t field, os::auto_filter_op_t op, double value)
{
    ScQueryEntry aEntry;
    aEntry.nField = mnStartCol + field;
    aEntry.eConnect = meConn;
    setQueryOp(aEntry, op);
    aEntry.GetQueryItem().meType = ScQueryEntry::ByValue;
    aEntry.GetQueryItem().mfVal = value;

    maEntries.push_back(aEntry);
}

void ScOrcusAutoFilterNode::append_item(os::col_t field, os::auto_filter_op_t op,
                                        std::string_view value, bool regex)
{
    ScQueryEntry aEntry;
    aEntry.nField = mnStartCol + field;
    aEntry.eConnect = meConn;
    setQueryOp(aEntry, op);
    aEntry.GetQueryItem().meType = ScQueryEntry::ByString;

    OUString aStr(value.data(), value.size(), mrGlobalSettings.getTextEncoding());
    aEntry.GetQueryItem().maString = mrDoc.GetSharedStringPool().intern(aStr);

    maEntries.push_back(aEntry);

    if (regex)
        mbHasRegex = true;
}

void ScOrcusAutoFilterNode::append_item(os::col_t field, os::auto_filter_op_t op)
{
    ScQueryEntry aEntry;
    aEntry.nField = mnStartCol + field;
    aEntry.eConnect = meConn;
    setQueryOp(aEntry, op);

    maEntries.push_back(aEntry);
}

os::iface::import_auto_filter_node* ScOrcusAutoFilterNode::start_node(os::auto_filter_node_op_t op)
{
    auto func = [this](std::vector<ScQueryEntry> aEntries, bool bHasRegex) {
        if (aEntries.empty())
            return;

        aEntries[0].eConnect = meConn;

        for (auto& rEntry : aEntries)
            maEntries.push_back(std::move(rEntry));

        if (bHasRegex)
            mbHasRegex = true;
    };

    mxChild = std::make_unique<ScOrcusAutoFilterNode>(mrDoc, mrGlobalSettings);
    mxChild->reset(mnStartCol, toQueryConnect(op), std::move(func));
    return mxChild.get();
}

os::iface::import_auto_filter_multi_values*
ScOrcusAutoFilterNode::start_multi_values(os::col_t field)
{
    auto func = [this, field](ScQueryEntry aEntry) {
        aEntry.nField = mnStartCol + field;
        maEntries.push_back(std::move(aEntry));
    };

    maMultiValues.reset(std::move(func));
    return &maMultiValues;
}

void ScOrcusAutoFilterNode::commit() { maCommitFunc(std::move(maEntries), mbHasRegex); }

void ScOrcusAutoFilterNode::reset(SCCOL nStartCol, ScQueryConnect eConn, commitFuncType func)
{
    mnStartCol = nStartCol;
    meConn = eConn;

    mxChild.reset();
    maEntries.clear();
    maCommitFunc = std::move(func);
}

ScOrcusAutoFilter::ScOrcusAutoFilter(ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS,
                                     SCTAB nTab)
    : mrDoc(rDoc)
    , maNode(rDoc.getDoc(), rGS)
    , mnTab(nTab)
{
}

ScOrcusAutoFilter::~ScOrcusAutoFilter() = default;

os::iface::import_auto_filter_node* ScOrcusAutoFilter::start_node(os::auto_filter_node_op_t op)
{
    ScDBData& rData = *mxData;
    auto func = [&rData](std::vector<ScQueryEntry> aEntries, bool bHasRegex) {
        ScQueryParam aParam;

        if (bHasRegex)
            aParam.eSearchType = utl::SearchParam::SearchType::Regexp;

        for (auto& rEntry : aEntries)
            aParam.AppendEntry() = std::move(rEntry);

        rData.SetQueryParam(aParam);
    };

    maNode.reset(mnCol1, toQueryConnect(op), std::move(func));
    return &maNode;
}

void ScOrcusAutoFilter::commit()
{
    auto& rDoc = mrDoc.getDoc();
    rDoc.SetAnonymousDBData(mnTab, std::move(mxData));

    for (SCCOL nCol = mnCol1; nCol <= mnCol2; ++nCol)
    {
        auto nFlag = rDoc.GetAttr(nCol, mnRow1, mnTab, ATTR_MERGE_FLAG)->GetValue();
        rDoc.ApplyAttr(nCol, mnRow1, mnTab, ScMergeFlagAttr(nFlag | ScMF::Auto));
    }
}

void ScOrcusAutoFilter::reset(const os::range_t& range)
{
    mnCol1 = range.first.column;
    mnCol2 = range.last.column;
    mnRow1 = range.first.row;
    mnRow2 = range.last.row;

    mxData = std::make_unique<ScDBData>(STR_DB_LOCAL_NONAME, mnTab, mnCol1, mnRow1, mnCol2, mnRow2);
    mxData->SetAutoFilter(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
