/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcusfiltersimpl.hxx"

#include "document.hxx"

#include "tools/urlobj.hxx"
#include "svtools/svtreebx.hxx"

#define __ORCUS_STATIC_LIB
#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/orcus_csv.hpp>
#include <orcus/xml_structure_tree.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/global.hpp>

#include <boost/ptr_container/ptr_vector.hpp>

using orcus::spreadsheet::row_t;
using orcus::spreadsheet::col_t;
using orcus::spreadsheet::formula_grammar_t;

#ifdef WNT
#define SYSTEM_PATH INetURLObject::FSYS_DOS
#else
#define SYSTEM_PATH INetURLObject::FSYS_UNX
#endif

namespace {

class ScOrcusSheet;

class ScOrcusFactory : public orcus::spreadsheet::iface::import_factory
{
    ScDocument& mrDoc;
    boost::ptr_vector<ScOrcusSheet> maSheets;

public:
    ScOrcusFactory(ScDocument& rDoc);

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings();
    virtual orcus::spreadsheet::iface::import_styles* get_styles();
};

class ScOrcusSheet : public orcus::spreadsheet::iface::import_sheet
{
    ScDocument& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheet(ScDocument& rDoc, SCTAB nTab);

    virtual void set_auto(row_t row, col_t col, const char* p, size_t n);
    virtual void set_format(row_t row, col_t col, size_t xf_index);
    virtual void set_formula(row_t row, col_t col, formula_grammar_t grammar, const char* p, size_t n);
    virtual void set_formula_result(row_t row, col_t col, const char* p, size_t n);
    virtual void set_shared_formula(
        row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula);
    virtual void set_shared_formula(
        row_t row, col_t col, formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range);
    virtual void set_shared_formula(row_t row, col_t col, size_t sindex);
    virtual void set_string(row_t row, col_t col, size_t sindex);
    virtual void set_value(row_t row, col_t col, double value);
};

ScOrcusFactory::ScOrcusFactory(ScDocument& rDoc) : mrDoc(rDoc) {}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::append_sheet(const char* sheet_name, size_t sheet_name_length)
{
    OUString aTabName(sheet_name, sheet_name_length, RTL_TEXTENCODING_UTF8);
    if (!mrDoc.InsertTab(SC_TAB_APPEND, aTabName))
        return NULL;

    SCTAB nTab = mrDoc.GetTableCount() - 1;
    maSheets.push_back(new ScOrcusSheet(mrDoc, nTab));
    return &maSheets.back();
}

orcus::spreadsheet::iface::import_sheet* ScOrcusFactory::get_sheet(const char* /*sheet_name*/, size_t /*sheet_name_length*/)
{
    // TODO: Implement this.
    return NULL;
}

orcus::spreadsheet::iface::import_shared_strings* ScOrcusFactory::get_shared_strings()
{
    // We don't support it yet.
    return NULL;
}

orcus::spreadsheet::iface::import_styles* ScOrcusFactory::get_styles()
{
    // We don't support it yet.
    return NULL;
}

ScOrcusSheet::ScOrcusSheet(ScDocument& rDoc, SCTAB nTab) :
    mrDoc(rDoc), mnTab(nTab) {}

void ScOrcusSheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    OUString aVal(p, n, RTL_TEXTENCODING_UTF8);
    mrDoc.SetString(col, row, mnTab, aVal);
}

void ScOrcusSheet::set_format(row_t /*row*/, col_t /*col*/, size_t /*xf_index*/)
{
}

void ScOrcusSheet::set_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, const char* /*p*/, size_t /*n*/)
{
}

void ScOrcusSheet::set_formula_result(row_t /*row*/, col_t /*col*/, const char* /*p*/, size_t /*n*/)
{
}

void ScOrcusSheet::set_shared_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, size_t /*sindex*/,
    const char* /*p_formula*/, size_t /*n_formula*/)
{
}

void ScOrcusSheet::set_shared_formula(
    row_t /*row*/, col_t /*col*/, formula_grammar_t /*grammar*/, size_t /*sindex*/,
    const char* /*p_formula*/, size_t /*n_formula*/, const char* /*p_range*/, size_t /*n_range*/)
{
}

void ScOrcusSheet::set_shared_formula(row_t /*row*/, col_t /*col*/, size_t /*sindex*/)
{
}

void ScOrcusSheet::set_string(row_t /*row*/, col_t /*col*/, size_t /*sindex*/)
{
}

void ScOrcusSheet::set_value(row_t /*row*/, col_t /*col*/, double /*value*/)
{
}

} // anonymous namespace

bool ScOrcusFiltersImpl::importCSV(ScDocument& rDoc, const OUString& rPath) const
{
    ScOrcusFactory aFactory(rDoc);
    INetURLObject aURL(rPath);
    OString aSysPath = rtl::OUStringToOString(aURL.getFSysPath(SYSTEM_PATH), RTL_TEXTENCODING_UTF8);
    const char* path = aSysPath.getStr();

    try
    {
        orcus::orcus_csv filter(&aFactory);
        filter.read_file(path);
    }
    catch (const std::exception&)
    {
        rDoc.InsertTab(SC_TAB_APPEND, OUString("Foo"));
        rDoc.SetString(0, 0, 0, "Failed to load!!!");
        return false;
    }
    return true;
}

namespace {

void populateTree(
   SvTreeListBox& rTreeCtrl, orcus::xml_structure_tree::walker& rWalker,
   const orcus::xml_structure_tree::entity_name& rElemName, bool bRepeat,
   const Image& rImgRepeatElem, const Image& rImgElemAttr,
   SvLBoxEntry* pParent)
{
    OUString aName(rElemName.name.get(), rElemName.name.size(), RTL_TEXTENCODING_UTF8);
    SvLBoxEntry* pEntry = rTreeCtrl.InsertEntry(aName, pParent);
    if (bRepeat)
    {
        rTreeCtrl.SetExpandedEntryBmp(pEntry, rImgRepeatElem);
        rTreeCtrl.SetCollapsedEntryBmp(pEntry, rImgRepeatElem);
    }

    if (pParent)
        rTreeCtrl.Expand(pParent);

    orcus::xml_structure_tree::entity_names_type aNames;

    rWalker.get_attributes(aNames);
    orcus::xml_structure_tree::entity_names_type::const_iterator it = aNames.begin();
    orcus::xml_structure_tree::entity_names_type::const_iterator itEnd = aNames.end();
    for (; it != itEnd; ++it)
    {
        orcus::xml_structure_tree::entity_name aAttrName = *it;
        SvLBoxEntry* pAttr = rTreeCtrl.InsertEntry(OUString(aAttrName.name.get(), aAttrName.name.size(), RTL_TEXTENCODING_UTF8), pEntry);
        rTreeCtrl.SetExpandedEntryBmp(pAttr, rImgElemAttr);
        rTreeCtrl.SetCollapsedEntryBmp(pAttr, rImgElemAttr);
    }
    rTreeCtrl.Expand(pEntry);

    rWalker.get_children(aNames);

    for (it = aNames.begin(), itEnd = aNames.end(); it != itEnd; ++it)
    {
        orcus::xml_structure_tree::element aElem = rWalker.descend(*it);
        populateTree(rTreeCtrl, rWalker, *it, aElem.repeat, rImgRepeatElem, rImgElemAttr, pEntry);
        rWalker.ascend();
    }
}

class TreeUpdateSwitch
{
    SvTreeListBox& mrTreeCtrl;
public:
    TreeUpdateSwitch(SvTreeListBox& rTreeCtrl) : mrTreeCtrl(rTreeCtrl)
    {
        mrTreeCtrl.SetUpdateMode(false);
    }

    ~TreeUpdateSwitch()
    {
        mrTreeCtrl.SetUpdateMode(true);
    }
};

}

bool ScOrcusFiltersImpl::loadXMLStructure(
   SvTreeListBox& rTreeCtrl, const rtl::OUString& rPath,
   const Image& rImgDefaultElem, const Image& rImgRepeatElem, const Image& rImgElemAttr) const
{
    INetURLObject aURL(rPath);
    OString aSysPath = rtl::OUStringToOString(aURL.getFSysPath(SYSTEM_PATH), RTL_TEXTENCODING_UTF8);
    const char* path = aSysPath.getStr();

    // TODO: Use our own stream loading call instead of one from orcus.
    std::string aStrm;
    orcus::load_file_content(path, aStrm);

    if (aStrm.empty())
        return false;

    orcus::xmlns_repository aNsRepo; // xml namespace repository.
    orcus::xml_structure_tree aXmlTree(aNsRepo);
    try
    {
        aXmlTree.parse(&aStrm[0], aStrm.size());

        TreeUpdateSwitch aSwitch(rTreeCtrl);
        rTreeCtrl.Clear();
        rTreeCtrl.SetDefaultCollapsedEntryBmp(rImgDefaultElem);
        rTreeCtrl.SetDefaultExpandedEntryBmp(rImgDefaultElem);

        orcus::xml_structure_tree::walker aWalker = aXmlTree.get_walker();

        // Root element.
        orcus::xml_structure_tree::element aElem = aWalker.root();
        populateTree(rTreeCtrl, aWalker, aElem.name, aElem.repeat, rImgRepeatElem, rImgElemAttr, NULL);
    }
    catch (const std::exception&)
    {
        // Parsing of this XML file failed.
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
