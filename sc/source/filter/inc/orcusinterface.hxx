/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_ORCUSINTERFACE_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_ORCUSINTERFACE_HXX

#include "address.hxx"
#include "documentimport.hxx"

#include <tools/color.hxx>
#include <tools/fontenum.hxx>

#include "sharedformulagroups.hxx"

#include <rtl/strbuf.hxx>

#define __ORCUS_STATIC_LIB
#include <orcus/spreadsheet/import_interface.hpp>

#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <unordered_map>
#include <vector>

class ScDocumentImport;
class ScOrcusSheet;
class ScOrcusStyles;
class ScOrcusFactory;
class ScRangeData;
class SfxItemSet;

namespace com { namespace sun { namespace star { namespace task {

class XStatusIndicator;

}}}}

class ScOrcusGlobalSettings : public orcus::spreadsheet::iface::import_global_settings
{
    ScDocumentImport& mrDoc;
    orcus::spreadsheet::formula_grammar_t meGrammar;

public:
    ScOrcusGlobalSettings(ScDocumentImport& rDoc);

    virtual void set_origin_date(int year, int month, int day) SAL_OVERRIDE;
    virtual void set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t) SAL_OVERRIDE;
    virtual orcus::spreadsheet::formula_grammar_t get_default_formula_grammar() const SAL_OVERRIDE;
};

class ScOrcusSharedStrings : public orcus::spreadsheet::iface::import_shared_strings
{
    ScOrcusFactory& mrFactory;

    OStringBuffer maCurSegment;
public:
    ScOrcusSharedStrings(ScOrcusFactory& rFactory);

    virtual size_t append(const char* s, size_t n) SAL_OVERRIDE;
    virtual size_t add(const char* s, size_t n) SAL_OVERRIDE;

    virtual void set_segment_bold(bool b) SAL_OVERRIDE;
    virtual void set_segment_italic(bool b) SAL_OVERRIDE;
    virtual void set_segment_font(size_t font_index) SAL_OVERRIDE;
    virtual void set_segment_font_name(const char* s, size_t n) SAL_OVERRIDE;
    virtual void set_segment_font_size(double point) SAL_OVERRIDE;
    virtual void set_segment_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) SAL_OVERRIDE;
    virtual void append_segment(const char* s, size_t n) SAL_OVERRIDE;

    virtual size_t commit_segments() SAL_OVERRIDE;
};

class ScOrcusAutoFilter : public orcus::spreadsheet::iface::import_auto_filter
{
public:
    ScOrcusAutoFilter(ScDocument& rDoc);

    virtual ~ScOrcusAutoFilter();

    virtual void set_range(const char* p_ref, size_t n_ref) SAL_OVERRIDE;

    virtual void set_column(orcus::spreadsheet::col_t col) SAL_OVERRIDE;

    virtual void append_column_match_value(const char* p, size_t n) SAL_OVERRIDE;

    virtual void commit_column() SAL_OVERRIDE;

    virtual void commit() SAL_OVERRIDE;

private:
    ScDocument& mrDoc;

    ScRange maRange;
};

class ScOrcusSheetProperties : public orcus::spreadsheet::iface::import_sheet_properties
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheetProperties(SCTAB nTab, ScDocumentImport& rDoc);
    virtual ~ScOrcusSheetProperties();

    virtual void set_column_width(orcus::spreadsheet::col_t col, double width, orcus::length_unit_t unit) SAL_OVERRIDE;

    virtual void set_column_hidden(orcus::spreadsheet::col_t col, bool hidden) SAL_OVERRIDE;

    virtual void set_row_height(orcus::spreadsheet::row_t row, double height, orcus::length_unit_t unit) SAL_OVERRIDE;

    virtual void set_row_hidden(orcus::spreadsheet::row_t row, bool hidden) SAL_OVERRIDE;

    virtual void set_merge_cell_range(const char* p_range, size_t n_range) SAL_OVERRIDE;
};

class ScOrcusSheet : public orcus::spreadsheet::iface::import_sheet
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
    ScOrcusFactory& mrFactory;
    ScOrcusStyles& mrStyles;
    sc::SharedFormulaGroups maFormulaGroups;
    ScOrcusAutoFilter maAutoFilter;
    ScOrcusSheetProperties maProperties;

    typedef std::map<size_t, ScRangeData*> SharedFormulaContainer;
    SharedFormulaContainer maSharedFormulas;

    int mnCellCount;

    void cellInserted();

public:
    ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory);

    virtual orcus::spreadsheet::iface::import_auto_filter* get_auto_filter() SAL_OVERRIDE { return &maAutoFilter; }
    virtual orcus::spreadsheet::iface::import_sheet_properties* get_sheet_properties() SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_table* get_table() SAL_OVERRIDE;

    // Orcus import interface
    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex) SAL_OVERRIDE;
    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) SAL_OVERRIDE;
    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) SAL_OVERRIDE;
    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, int year, int month, int day, int hour, int minute, double second) SAL_OVERRIDE;

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index) SAL_OVERRIDE;
    virtual void set_format_range(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end , size_t xf_index) SAL_OVERRIDE;

    virtual void set_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) SAL_OVERRIDE;

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula) SAL_OVERRIDE;

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range) SAL_OVERRIDE;

    virtual void set_shared_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex) SAL_OVERRIDE;

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, orcus::spreadsheet::row_t array_rows, orcus::spreadsheet::col_t array_cols) SAL_OVERRIDE;

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, const char* p_range, size_t n_range) SAL_OVERRIDE;

    SCTAB getIndex() const { return mnTab; }
};

class ScOrcusStyles : public orcus::spreadsheet::iface::import_styles
{
private:
    ScDocument& mrDoc;

    struct font
    {
        bool mbBold;
        bool mbItalic;
        OUString maName;
        double mnSize;
        Color maColor;
        FontUnderline meUnderline;

        font();

        void applyToItemSet(SfxItemSet& rSet) const;
    };

    font maCurrentFont;
    std::vector<font> maFonts;

    struct fill
    {
        OUString maPattern;
        Color maFgColor;
        Color maBgColor;

        void applyToItemSet(SfxItemSet& rSet) const;
    };

    fill maCurrentFill;
    std::vector<fill> maFills;

    struct border
    {

        border();
    };

    border maCurrentBorder;
    std::vector<border> maBorders;

    struct protection
    {
        bool mbHidden;
        bool mbLocked;

        protection();
    };

    protection maCurrentProtection;
    std::vector<protection> maProtections;

    struct number_format
    {
        OUString maCode;
    };

    number_format maCurrentNumberFormat;
    std::vector<number_format> maNumberFormats;

    struct xf
    {
        size_t mnFontId;
        size_t mnFillId;
        size_t mnBorderId;
        size_t mnProtectionId;
        size_t mnNumberFormatId;
        size_t mnStyleXf;

        xf();
    };

    xf maCurrentXF;
    std::vector<xf> maCellStyleXfs;
    std::vector<xf> maCellXfs;

    struct cell_style
    {
        OUString maName;
        size_t mnXFId;
        size_t mnBuiltInId;

        cell_style();
    };

    cell_style maCurrentCellStyle;

    void applyXfToItemSet(SfxItemSet& rSet, const xf& rXf);

public:
    ScOrcusStyles(ScDocument& rDoc);

    bool applyXfToItemSet(SfxItemSet& rSet, size_t xfId);

    // font

    virtual void set_font_count(size_t n) SAL_OVERRIDE;
    virtual void set_font_bold(bool b) SAL_OVERRIDE;
    virtual void set_font_italic(bool b) SAL_OVERRIDE;
    virtual void set_font_name(const char* s, size_t n) SAL_OVERRIDE;
    virtual void set_font_size(double point) SAL_OVERRIDE;
    virtual void set_font_underline(orcus::spreadsheet::underline_t e) SAL_OVERRIDE;
    virtual void set_font_color( orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) SAL_OVERRIDE;
    virtual size_t commit_font() SAL_OVERRIDE;

    // fill

    virtual void set_fill_count(size_t n) SAL_OVERRIDE;
    virtual void set_fill_pattern_type(const char* s, size_t n) SAL_OVERRIDE;
    virtual void set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) SAL_OVERRIDE;
    virtual void set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) SAL_OVERRIDE;
    virtual size_t commit_fill() SAL_OVERRIDE;

    // border

    virtual void set_border_count(size_t n) SAL_OVERRIDE;
    virtual void set_border_style(orcus::spreadsheet::border_direction_t dir, const char* s, size_t n) SAL_OVERRIDE;
    virtual void set_border_color(orcus::spreadsheet::border_direction_t dir,
            orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) SAL_OVERRIDE;
    virtual size_t commit_border() SAL_OVERRIDE;

    // cell protection
    virtual void set_cell_hidden(bool b) SAL_OVERRIDE;
    virtual void set_cell_locked(bool b) SAL_OVERRIDE;
    virtual size_t commit_cell_protection() SAL_OVERRIDE;

    // number format
    virtual void set_number_format_count(size_t n) SAL_OVERRIDE;
    virtual void set_number_format_identifier(size_t n) SAL_OVERRIDE;
    virtual void set_number_format_code(const char* s, size_t n) SAL_OVERRIDE;
    virtual size_t commit_number_format() SAL_OVERRIDE;

    // cell style xf

    virtual void set_cell_style_xf_count(size_t n) SAL_OVERRIDE;
    virtual size_t commit_cell_style_xf() SAL_OVERRIDE;

    // cell xf

    virtual void set_cell_xf_count(size_t n) SAL_OVERRIDE;
    virtual size_t commit_cell_xf() SAL_OVERRIDE;

    // xf (cell format) - used both by cell xf and cell style xf.

    virtual void set_xf_number_format(size_t index) SAL_OVERRIDE;
    virtual void set_xf_font(size_t index) SAL_OVERRIDE;
    virtual void set_xf_fill(size_t index) SAL_OVERRIDE;
    virtual void set_xf_border(size_t index) SAL_OVERRIDE;
    virtual void set_xf_protection(size_t index) SAL_OVERRIDE;
    virtual void set_xf_style_xf(size_t index) SAL_OVERRIDE;
    virtual void set_xf_apply_alignment(bool b) SAL_OVERRIDE;
    virtual void set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align) SAL_OVERRIDE;
    virtual void set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align) SAL_OVERRIDE;

    // cell style entry

    virtual void set_cell_style_count(size_t n) SAL_OVERRIDE;
    virtual void set_cell_style_name(const char* s, size_t n) SAL_OVERRIDE;
    virtual void set_cell_style_xf(size_t index) SAL_OVERRIDE;
    virtual void set_cell_style_builtin(size_t index) SAL_OVERRIDE;
    virtual size_t commit_cell_style() SAL_OVERRIDE;
};

class ScOrcusFactory : public orcus::spreadsheet::iface::import_factory
{
    struct StringCellCache
    {
        ScAddress maPos;
        size_t mnIndex;

        StringCellCache(const ScAddress& rPos, size_t nIndex);
    };

    typedef std::unordered_map<OUString, size_t, OUStringHash> StringHashType;
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

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(const char *sheet_name, size_t sheet_name_length) SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char *sheet_name, size_t sheet_name_length) SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(orcus::spreadsheet::sheet_t sheet_index) SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings() SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings() SAL_OVERRIDE;
    virtual orcus::spreadsheet::iface::import_styles* get_styles() SAL_OVERRIDE;
    virtual void finalize() SAL_OVERRIDE;

    size_t appendString(const OUString& rStr);
    size_t addString(const OUString& rStr);

    void pushStringCell(const ScAddress& rPos, size_t nStrIndex);

    void incrementProgress();

    void setStatusIndicator(const com::sun::star::uno::Reference<com::sun::star::task::XStatusIndicator>& rIndicator);
};

class ScOrcusTable : public orcus::spreadsheet::iface::import_table
{
private:
    SCTAB mnTab;
    ScDocument& mrDoc;
    ScOrcusAutoFilter maAutoFilter;

    ScRange maRange;
    OUString maName;

public:
    ScOrcusTable(SCTAB nTab, ScDocument& rDoc);
    virtual ~ScOrcusTable();

    virtual orcus::spreadsheet::iface::import_auto_filter* get_auto_filter() SAL_OVERRIDE;

    virtual void set_identifier(size_t id) SAL_OVERRIDE;

    virtual void set_range(const char* p_ref, size_t n_ref) SAL_OVERRIDE;

    virtual void set_totals_row_count(size_t row_count) SAL_OVERRIDE;

    virtual void set_name(const char* p, size_t n) SAL_OVERRIDE;

    virtual void set_display_name(const char* p, size_t n) SAL_OVERRIDE;

    virtual void set_column_count(size_t n) SAL_OVERRIDE;

    virtual void set_column_identifier(size_t id) SAL_OVERRIDE;
    virtual void set_column_name(const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_column_totals_row_label(const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_column_totals_row_function(orcus::spreadsheet::totals_row_function_t func) SAL_OVERRIDE;
    virtual void commit_column() SAL_OVERRIDE;

    virtual void set_style_name(const char* p, size_t n) SAL_OVERRIDE;
    virtual void set_style_show_first_column(bool b) SAL_OVERRIDE;
    virtual void set_style_show_last_column(bool b) SAL_OVERRIDE;
    virtual void set_style_show_row_stripes(bool b) SAL_OVERRIDE;
    virtual void set_style_show_column_stripes(bool b) SAL_OVERRIDE;

    virtual void commit() SAL_OVERRIDE;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
