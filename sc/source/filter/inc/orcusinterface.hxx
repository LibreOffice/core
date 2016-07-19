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

#include "conditio.hxx"

#include <rtl/strbuf.hxx>

#define __ORCUS_STATIC_LIB
#include <orcus/spreadsheet/import_interface.hpp>

#include <memory>
#include <map>
#include <unordered_map>
#include <vector>

class ScDocumentImport;
class ScOrcusSheet;
class ScOrcusStyles;
class ScOrcusFactory;
class ScRangeData;
class SfxItemSet;

typedef sal_Int16 SvxBorderStyle;

namespace com { namespace sun { namespace star { namespace task {

class XStatusIndicator;

}}}}

class ScOrcusGlobalSettings : public orcus::spreadsheet::iface::import_global_settings
{
    ScDocumentImport& mrDoc;

public:
    ScOrcusGlobalSettings(ScDocumentImport& rDoc);

    virtual void set_origin_date(int year, int month, int day) override;

    virtual void set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t grammar) override;
    virtual orcus::spreadsheet::formula_grammar_t get_default_formula_grammar() const override;
};

class ScOrcusSharedStrings : public orcus::spreadsheet::iface::import_shared_strings
{
    ScOrcusFactory& mrFactory;

    OStringBuffer maCurSegment;
public:
    ScOrcusSharedStrings(ScOrcusFactory& rFactory);

    virtual size_t append(const char* s, size_t n) override;
    virtual size_t add(const char* s, size_t n) override;

    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_font_name(const char* s, size_t n) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual void append_segment(const char* s, size_t n) override;

    virtual size_t commit_segments() override;
};

class ScOrcusConditionalFormat : public orcus::spreadsheet::iface::import_conditional_format
{
public:
    ScOrcusConditionalFormat(SCTAB nTab, ScDocument& rDoc);
    virtual ~ScOrcusConditionalFormat();

    virtual void set_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;

    virtual void set_formula(const char* p, size_t n) override;

    virtual void set_condition_type(orcus::spreadsheet::condition_type_t type) override;

    virtual void set_date(orcus::spreadsheet::condition_date_t date) override;

    virtual void commit_condition() override;

    virtual void set_icon_name(const char* p, size_t n) override;

    virtual void set_databar_gradient(bool gradient) override;

    virtual void set_databar_axis(orcus::spreadsheet::databar_axis_t axis) override;

    virtual void set_databar_color_positive(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;

    virtual void set_databar_color_negative(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;

    virtual void set_min_databar_length(double length) override;

    virtual void set_max_databar_length(double length) override;

    virtual void set_show_value(bool show) override;

    virtual void set_iconset_reverse(bool reverse) override;

    virtual void set_xf_id(size_t xf) override;

    virtual void set_operator(orcus::spreadsheet::condition_operator_t condition_type) override;

    virtual void set_type(orcus::spreadsheet::conditional_format_t type) override;

    virtual void commit_entry() override;

    virtual void set_range(const char* p, size_t n) override;

    virtual void set_range(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end) override;

    virtual void commit_format() override;

private:

    SCTAB mnTab;
    ScDocument& mrDoc;

    std::unique_ptr<ScConditionalFormat> mpCurrentFormat;

    condformat::ScFormatEntryType meEntryType;
};

class ScOrcusAutoFilter : public orcus::spreadsheet::iface::import_auto_filter
{
public:
    ScOrcusAutoFilter(ScDocument& rDoc);

    virtual ~ScOrcusAutoFilter();

    virtual void set_range(const char* p_ref, size_t n_ref) override;

    virtual void set_column(orcus::spreadsheet::col_t col) override;

    virtual void append_column_match_value(const char* p, size_t n) override;

    virtual void commit_column() override;

    virtual void commit() override;

private:
    ScRange maRange;
};

class ScOrcusSheetProperties : public orcus::spreadsheet::iface::import_sheet_properties
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheetProperties(SCTAB nTab, ScDocumentImport& rDoc);
    virtual ~ScOrcusSheetProperties();

    virtual void set_column_width(orcus::spreadsheet::col_t col, double width, orcus::length_unit_t unit) override;

    virtual void set_column_hidden(orcus::spreadsheet::col_t col, bool hidden) override;

    virtual void set_row_height(orcus::spreadsheet::row_t row, double height, orcus::length_unit_t unit) override;

    virtual void set_row_hidden(orcus::spreadsheet::row_t row, bool hidden) override;

    virtual void set_merge_cell_range(const char* p_range, size_t n_range) override;
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
    ScOrcusConditionalFormat maConditionalFormat;

    int mnCellCount;

    void cellInserted();

public:
    ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory);

    virtual orcus::spreadsheet::iface::import_auto_filter* get_auto_filter() override { return &maAutoFilter; }
    virtual orcus::spreadsheet::iface::import_table* get_table() override;
    virtual orcus::spreadsheet::iface::import_sheet_properties* get_sheet_properties() override;
    virtual orcus::spreadsheet::iface::import_conditional_format* get_conditional_format() override;

    // Orcus import interface
    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n) override;
    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex) override;
    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) override;
    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) override;
    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, int year, int month, int day, int hour, int minute, double second) override;

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index) override;
    virtual void set_format(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end, size_t xf_index) override;

    virtual void set_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, const char* p, size_t n) override;
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n) override;
    virtual void set_formula_result(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double val) override;

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula) override;

    virtual void set_shared_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar, size_t sindex,
        const char* p_formula, size_t n_formula, const char* p_range, size_t n_range) override;

    virtual void set_shared_formula(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex) override;

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, orcus::spreadsheet::row_t array_rows, orcus::spreadsheet::col_t array_cols) override;

    virtual void set_array_formula(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::formula_grammar_t grammar,
        const char* p, size_t n, const char* p_range, size_t n_range) override;

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

        bool mbHasFontAttr;
        bool mbHasUnderlineAttr;

        FontLineStyle meUnderline;
        FontLineStyle meUnderlineType;
        FontLineStyle meUnderlineWidth;
        Color maUnderlineColor;

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

        bool mbHasFillAttr;

        fill();

        void applyToItemSet(SfxItemSet& rSet) const;
    };

    fill maCurrentFill;
    std::vector<fill> maFills;

    struct border
    {
        struct border_line
        {
            SvxBorderStyle mestyle;
            Color maColor;
            double mnWidth;
        };
        std::map<orcus::spreadsheet::border_direction_t, border_line> border_lines;

        bool mbHasBorderAttr;

        border();

        void applyToItemSet(SfxItemSet& rSet) const;
    };

    border maCurrentBorder;
    std::vector<border> maBorders;

    struct protection
    {
        bool mbHidden;
        bool mbLocked;
        bool mbPrintContent;
        bool mbFormulaHidden;

        bool mbHasProtectionAttr;

        protection();
        void applyToItemSet(SfxItemSet& rSet) const;
    };

    protection maCurrentProtection;
    std::vector<protection> maProtections;

    struct number_format
    {
        OUString maCode;

        bool mbHasNumberFormatAttr;

        number_format();
        void applyToItemSet(SfxItemSet& rSet, ScDocument& rDoc) const;
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

    void applyXfToItemSet(SfxItemSet& rSet, size_t xfId);

    // font

    virtual void set_font_count(size_t n) override;
    virtual void set_font_bold(bool b) override;
    virtual void set_font_italic(bool b) override;
    virtual void set_font_name(const char* s, size_t n) override;
    virtual void set_font_size(double point) override;
    virtual void set_font_underline(orcus::spreadsheet::underline_t e) override;
    virtual void set_font_underline_width(orcus::spreadsheet::underline_width_t e) override;
    virtual void set_font_underline_mode(orcus::spreadsheet::underline_mode_t e) override;
    virtual void set_font_underline_type(orcus::spreadsheet::underline_type_t e) override;
    virtual void set_font_underline_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual void set_font_color( orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual size_t commit_font() override;

    // fill

    virtual void set_fill_count(size_t n) override;
    virtual void set_fill_pattern_type(const char* s, size_t n) override;
    virtual void set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;
    virtual void set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;
    virtual size_t commit_fill() override;

    // border

    virtual void set_border_count(size_t n) override;
    virtual void set_border_style(orcus::spreadsheet::border_direction_t dir, const char* s, size_t n) override;
    virtual void set_border_style(orcus::spreadsheet::border_direction_t dir, orcus::spreadsheet::border_style_t style) override;
    virtual void set_border_color(orcus::spreadsheet::border_direction_t dir,
            orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual void set_border_width(orcus::spreadsheet::border_direction_t dir, orcus::length_t width) override;
    virtual size_t commit_border() override;

    // cell protection
    virtual void set_cell_hidden(bool b) override;
    virtual void set_cell_locked(bool b) override;
    virtual void set_cell_print_content(bool b) override;
    virtual void set_cell_formula_hidden(bool b) override;
    virtual size_t commit_cell_protection() override;

    // number format
    virtual void set_number_format_count(size_t n) override;
    virtual void set_number_format_identifier(size_t n) override;
    virtual void set_number_format_code(const char* s, size_t n) override;
    virtual size_t commit_number_format() override;

    // cell style xf

    virtual void set_cell_style_xf_count(size_t n) override;
    virtual size_t commit_cell_style_xf() override;

    // cell xf

    virtual void set_cell_xf_count(size_t n) override;
    virtual size_t commit_cell_xf() override;

    // dxf
    virtual void set_dxf_count(size_t count) override;
    virtual size_t commit_dxf() override;

    // xf (cell format) - used both by cell xf and cell style xf.

    virtual void set_xf_number_format(size_t index) override;
    virtual void set_xf_font(size_t index) override;
    virtual void set_xf_fill(size_t index) override;
    virtual void set_xf_border(size_t index) override;
    virtual void set_xf_protection(size_t index) override;
    virtual void set_xf_style_xf(size_t index) override;
    virtual void set_xf_apply_alignment(bool b) override;
    virtual void set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align) override;
    virtual void set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align) override;

    // cell style entry

    virtual void set_cell_style_count(size_t n) override;
    virtual void set_cell_style_name(const char* s, size_t n) override;
    virtual void set_cell_style_xf(size_t index) override;
    virtual void set_cell_style_builtin(size_t index) override;
    virtual void set_cell_style_parent_name(const char* s, size_t n) override;
    virtual size_t commit_cell_style() override;
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
    std::vector< std::unique_ptr<ScOrcusSheet> > maSheets;
    ScOrcusStyles maStyles;

    int mnProgress;

    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

public:
    ScOrcusFactory(ScDocument& rDoc);

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(const char *sheet_name, size_t sheet_name_length) override;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char *sheet_name, size_t sheet_name_length) override;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(orcus::spreadsheet::sheet_t sheet_index) override;
    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings() override;
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings() override;
    virtual orcus::spreadsheet::iface::import_styles* get_styles() override;
    virtual void finalize() override;

    size_t appendString(const OUString& rStr);
    size_t addString(const OUString& rStr);

    void pushStringCell(const ScAddress& rPos, size_t nStrIndex);

    void incrementProgress();

    void setStatusIndicator(const css::uno::Reference<css::task::XStatusIndicator>& rIndicator);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
