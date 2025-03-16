/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include <documentimport.hxx>
#include <queryentry.hxx>

#include <orcus/spreadsheet/import_interface_auto_filter.hpp>

#include <vector>
#include <functional>

class ScOrcusGlobalSettings;

class ScOrcusAutoFilterMultiValues
    : public orcus::spreadsheet::iface::import_auto_filter_multi_values
{
public:
    using commitFuncType = std::function<void(ScQueryEntry)>;

    ScOrcusAutoFilterMultiValues(ScDocument& rDoc, const ScOrcusGlobalSettings& rGS);

    void add_value(std::string_view value) override;

    void commit() override;

    void reset(commitFuncType func);

private:
    ScDocument& mrDoc;
    const ScOrcusGlobalSettings& mrGlobalSettings;

    ScQueryEntry maEntry;
    commitFuncType maCommitFunc;
};

class ScOrcusAutoFilterNode : public orcus::spreadsheet::iface::import_auto_filter_node
{
public:
    using commitFuncType = std::function<void(std::vector<ScQueryEntry>, bool)>;

    ScOrcusAutoFilterNode(ScDocument& rDoc, const ScOrcusGlobalSettings& rGS);

    virtual void append_item(orcus::spreadsheet::col_t field,
                             orcus::spreadsheet::auto_filter_op_t op, double value) override;

    virtual void append_item(orcus::spreadsheet::col_t field,
                             orcus::spreadsheet::auto_filter_op_t op, std::string_view value,
                             bool regex) override;

    virtual void append_item(orcus::spreadsheet::col_t field,
                             orcus::spreadsheet::auto_filter_op_t op) override;

    virtual orcus::spreadsheet::iface::import_auto_filter_node*
    start_node(orcus::spreadsheet::auto_filter_node_op_t op) override;

    virtual orcus::spreadsheet::iface::import_auto_filter_multi_values*
    start_multi_values(orcus::spreadsheet::col_t field) override;

    virtual void commit() override;

    void reset(SCCOL nStartCol, ScQueryConnect eConn, commitFuncType func);

private:
    ScDocument& mrDoc;
    const ScOrcusGlobalSettings& mrGlobalSettings;
    ScOrcusAutoFilterMultiValues maMultiValues;
    std::unique_ptr<ScOrcusAutoFilterNode> mxChild;

    SCCOL mnStartCol = -1;
    ScQueryConnect meConn = ScQueryConnect::SC_AND;
    bool mbHasRegex = false;

    std::vector<ScQueryEntry> maEntries;
    commitFuncType maCommitFunc;
};

class ScOrcusAutoFilter : public orcus::spreadsheet::iface::import_auto_filter
{
public:
    ScOrcusAutoFilter(ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS, SCTAB nTab);
    ScOrcusAutoFilter(const ScOrcusAutoFilter&) = delete;
    ~ScOrcusAutoFilter();

    virtual orcus::spreadsheet::iface::import_auto_filter_node*
    start_node(orcus::spreadsheet::auto_filter_node_op_t op) override;

    virtual void commit() override;

    void reset(const orcus::spreadsheet::range_t& range);

private:
    ScDocumentImport& mrDoc;

    ScOrcusAutoFilterNode maNode;
    std::unique_ptr<ScDBData> mxData;

    const SCTAB mnTab;
    SCCOL mnCol1;
    SCCOL mnCol2;
    SCROW mnRow1;
    SCROW mnRow2;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
