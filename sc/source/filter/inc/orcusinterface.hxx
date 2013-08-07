/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_ORCUSINTERFACE_HXX__
#define __SC_ORCUSINTERFACE_HXX__

#include "address.hxx"
#include "documentimport.hxx"

#include "sharedformulagroups.hxx"

#include "rtl/strbuf.hxx"

#define __ORCUS_STATIC_LIB
#include <orcus/spreadsheet/import_interface.hpp>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/unordered_map.hpp>

#include <map>

class ScDocumentImport;
class ScOrcusSheet;
class ScOrcusFactory;
class ScRangeData;

namespace com { namespace sun { namespace star { namespace task {

class XStatusIndicator;

}}}}

class ScOrcusGlobalSettings : public orcus::spreadsheet::iface::import_global_settings
{
    ScDocumentImport& mrDoc;

public:
    ScOrcusGlobalSettings(ScDocumentImport& rDoc);

    virtual void set_origin_date(int year, int month, int day);
};

class ScOrcusSharedStrings : public orcus::spreadsheet::iface::import_shared_strings
{
    ScOrcusFactory& mrFactory;

    OStringBuffer maCurSegment;
public:
    ScOrcusSharedStrings(ScOrcusFactory& rFactory);

    virtual size_t append(const char* s, size_t n);
    virtual size_t add(const char* s, size_t n);

    virtual void set_segment_bold(bool b);
    virtual void set_segment_italic(bool b);
    virtual void set_segment_font_name(const char* s, size_t n);
    virtual void set_segment_font_size(double point);
    virtual void append_segment(const char* s, size_t n);

    virtual size_t commit_segments();
};

class ScOrcusSheet : public orcus::spreadsheet::iface::import_sheet
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
    ScOrcusFactory& mrFactory;
    sc::SharedFormulaGroups maFormulaGroups;

    typedef std::map<size_t, ScRangeData*> SharedFormulaContainer;
    SharedFormulaContainer maSharedFormulas;

    int mnCellCount;

    void cellInserted();

public:
    ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory);

    // Orcus import interface
    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n);
    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex);
    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value);
    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value);
    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, int year, int month, int day, int hour, int minute, double second);

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index);

    virtual void set_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, const char* p, size_t n);
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n);

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula);

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range);

    virtual void set_shared_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex);

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, orcus::spreadsheet::row_t array_rows, orcus::spreadsheet::col_t array_cols);

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, const char* p_range, size_t n_range);

    SCTAB getIndex() const { return mnTab; }
};

class ScOrcusStyles : public orcus::spreadsheet::iface::import_styles
{

public:
    // font

    virtual void set_font_count(size_t n);
    virtual void set_font_bold(bool b);
    virtual void set_font_italic(bool b);
    virtual void set_font_name(const char* s, size_t n);
    virtual void set_font_size(double point);
    virtual void set_font_underline(orcus::spreadsheet::underline_t e);
    virtual size_t commit_font();

    // fill

    virtual void set_fill_count(size_t n);
    virtual void set_fill_pattern_type(const char* s, size_t n);
    virtual void set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue);
    virtual void set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue);
    virtual size_t commit_fill();

    // border

    virtual void set_border_count(size_t n);
    virtual void set_border_style(orcus::spreadsheet::border_direction_t dir, const char* s, size_t n);
    virtual size_t commit_border();

    // cell protection
    virtual void set_cell_hidden(bool b);
    virtual void set_cell_locked(bool b);
    virtual size_t commit_cell_protection();

    // number format
    virtual void set_number_format(const char* s, size_t n);
    virtual size_t commit_number_format();

    // cell style xf

    virtual void set_cell_style_xf_count(size_t n);
    virtual size_t commit_cell_style_xf();

    // cell xf

    virtual void set_cell_xf_count(size_t n);
    virtual size_t commit_cell_xf();

    // xf (cell format) - used both by cell xf and cell style xf.

    virtual void set_xf_number_format(size_t index);
    virtual void set_xf_font(size_t index);
    virtual void set_xf_fill(size_t index);
    virtual void set_xf_border(size_t index);
    virtual void set_xf_protection(size_t index);
    virtual void set_xf_style_xf(size_t index);

    // cell style entry

    virtual void set_cell_style_count(size_t n);
    virtual void set_cell_style_name(const char* s, size_t n);
    virtual void set_cell_style_xf(size_t index);
    virtual void set_cell_style_builtin(size_t index);
    virtual size_t commit_cell_style();
};

class ScOrcusFactory : public orcus::spreadsheet::iface::import_factory
{
    struct StringCellCache
    {
        ScAddress maPos;
        size_t mnIndex;

        StringCellCache(const ScAddress& rPos, size_t nIndex);
    };

    typedef boost::unordered_map<OUString, size_t, OUStringHash> StringHashType;
    typedef std::vector<StringCellCache> StringCellCaches;

    ScDocumentImport maDoc;

    std::vector<OUString> maStrings;
    StringHashType maStringHash;

    StringCellCaches maStringCells;
    ScOrcusGlobalSettings maGlobalSettings;
    ScOrcusSharedStrings maSharedStrings;
    boost::ptr_vector<ScOrcusSheet> maSheets;
    ScOrcusStyles maStyles;

    int mnProgress;

    com::sun::star::uno::Reference<com::sun::star::task::XStatusIndicator> mxStatusIndicator;

public:
    ScOrcusFactory(ScDocument& rDoc);

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char *sheet_name, size_t sheet_name_length);
    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings();
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings();
    virtual orcus::spreadsheet::iface::import_styles* get_styles();
    virtual void finalize();

    size_t appendString(const OUString& rStr);
    size_t addString(const OUString& rStr);

    void pushStringCell(const ScAddress& rPos, size_t nStrIndex);

    void incrementProgress();

    void setStatusIndicator(const com::sun::star::uno::Reference<com::sun::star::task::XStatusIndicator>& rIndicator);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
