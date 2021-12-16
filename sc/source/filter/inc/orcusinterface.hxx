/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <address.hxx>
#include <documentimport.hxx>

#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <editeng/svxenum.hxx>

#include "sharedformulagroups.hxx"

#include <conditio.hxx>

#include <rtl/strbuf.hxx>
#include <editeng/borderline.hxx>

#include <orcus/spreadsheet/import_interface.hpp>

#include <memory>
#include <map>
#include <unordered_map>
#include <vector>

class ScOrcusSheet;
class ScOrcusStyles;
class ScOrcusFactory;
class SfxItemSet;
namespace com::sun::star::task { class XStatusIndicator; }

class ScOrcusGlobalSettings : public orcus::spreadsheet::iface::import_global_settings
{
    ScDocumentImport& mrDoc;
    formula::FormulaGrammar::Grammar meCalcGrammar;
    orcus::spreadsheet::formula_grammar_t meOrcusGrammar;
    rtl_TextEncoding mnTextEncoding;

public:
    ScOrcusGlobalSettings(ScDocumentImport& rDoc);

    virtual void set_origin_date(int year, int month, int day) override;
    virtual void set_character_set(orcus::character_set_t cs) override;

    virtual void set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t grammar) override;
    virtual orcus::spreadsheet::formula_grammar_t get_default_formula_grammar() const override;

    formula::FormulaGrammar::Grammar getCalcGrammar() const
    {
        return meCalcGrammar;
    }

    rtl_TextEncoding getTextEncoding() const
    {
        return mnTextEncoding;
    }

    ScDocumentImport& getDoc() const
    {
        return mrDoc;
    }
};

class ScOrcusRefResolver : public orcus::spreadsheet::iface::import_reference_resolver
{
    const ScOrcusGlobalSettings& mrGlobalSettings;

public:
    ScOrcusRefResolver( const ScOrcusGlobalSettings& rGS );

    orcus::spreadsheet::src_address_t resolve_address(std::string_view address) override;
    orcus::spreadsheet::src_range_t resolve_range(std::string_view range) override;
};

class ScOrcusNamedExpression : public orcus::spreadsheet::iface::import_named_expression
{
    ScDocumentImport& mrDoc;
    const ScOrcusGlobalSettings& mrGlobalSettings;
    ScAddress maBasePos;
    OUString maName;
    OUString maExpr;
    const SCTAB mnTab; //< negative if global, else >= 0 for sheet-local named expressions.

public:
    ScOrcusNamedExpression( ScDocumentImport& rDoc, const ScOrcusGlobalSettings& rGS, SCTAB nTab = -1 );

    void reset();

    virtual void set_base_position(const orcus::spreadsheet::src_address_t& pos) override;
    virtual void set_named_expression(std::string_view name, std::string_view expression) override;
    virtual void set_named_range(std::string_view name, std::string_view range) override;
    virtual void commit() override;
};

class ScOrcusSharedStrings : public orcus::spreadsheet::iface::import_shared_strings
{
    ScOrcusFactory& mrFactory;

    OStringBuffer maCurSegment;
public:
    ScOrcusSharedStrings(ScOrcusFactory& rFactory);

    virtual size_t append(std::string_view s) override;
    virtual size_t add(std::string_view s) override;

    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_font_name(std::string_view s) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual void append_segment(std::string_view s) override;

    virtual size_t commit_segments() override;
};

class ScOrcusConditionalFormat : public orcus::spreadsheet::iface::import_conditional_format
{
public:
    ScOrcusConditionalFormat(SCTAB nTab, ScDocument& rDoc);
    virtual ~ScOrcusConditionalFormat() override;

    virtual void set_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;

    virtual void set_formula(std::string_view formula) override;

    virtual void set_condition_type(orcus::spreadsheet::condition_type_t type) override;

    virtual void set_date(orcus::spreadsheet::condition_date_t date) override;

    virtual void commit_condition() override;

    virtual void set_icon_name(std::string_view name) override;

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

    virtual void set_range(std::string_view range) override;

    virtual void set_range(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end) override;

    virtual void commit_format() override;

private:

    SCTAB mnTab;
    ScDocument& mrDoc;

    std::unique_ptr<ScConditionalFormat> mpCurrentFormat;

    ScFormatEntry::Type meEntryType;
};

class ScOrcusAutoFilter : public orcus::spreadsheet::iface::import_auto_filter
{
public:
    ScOrcusAutoFilter( const ScOrcusGlobalSettings& rGS );

    virtual ~ScOrcusAutoFilter() override;

    virtual void set_range(const orcus::spreadsheet::range_t& range) override;

    virtual void set_column(orcus::spreadsheet::col_t col) override;

    virtual void append_column_match_value(std::string_view value) override;

    virtual void commit_column() override;

    virtual void commit() override;

private:
    const ScOrcusGlobalSettings& mrGlobalSettings;
    ScRange maRange;
};

class ScOrcusSheetProperties : public orcus::spreadsheet::iface::import_sheet_properties
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheetProperties(SCTAB nTab, ScDocumentImport& rDoc);
    virtual ~ScOrcusSheetProperties() override;

    virtual void set_column_width(orcus::spreadsheet::col_t col, double width, orcus::length_unit_t unit) override;

    virtual void set_column_hidden(orcus::spreadsheet::col_t col, bool hidden) override;

    virtual void set_row_height(orcus::spreadsheet::row_t row, double height, orcus::length_unit_t unit) override;

    virtual void set_row_hidden(orcus::spreadsheet::row_t row, bool hidden) override;

    virtual void set_merge_cell_range(const orcus::spreadsheet::range_t& range) override;
};

class ScOrcusFormula : public orcus::spreadsheet::iface::import_formula
{
    enum class ResultType { NotSet, String, Value, Empty };

    friend class ScOrcusSheet;

    ScOrcusSheet& mrSheet;

    SCCOL mnCol;
    SCROW mnRow;
    OUString maFormula;
    formula::FormulaGrammar::Grammar meGrammar;
    size_t mnSharedFormulaIndex;
    bool mbShared;

    ResultType meResType;
    OUString maResult; // result string.
    double mfResult;

    void reset();

public:
    ScOrcusFormula( ScOrcusSheet& rSheet );
    virtual ~ScOrcusFormula() override;

    virtual void set_position(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col) override;
    virtual void set_formula(orcus::spreadsheet::formula_grammar_t grammar, std::string_view formula) override;
    virtual void set_shared_formula_index(size_t index) override;
    virtual void set_result_value(double value) override;
    virtual void set_result_string(std::string_view value) override;
    virtual void set_result_empty() override;
    virtual void set_result_bool(bool value) override;
    virtual void commit() override;
};

class ScOrcusArrayFormula : public orcus::spreadsheet::iface::import_array_formula
{
    friend class ScOrcusSheet;

    ScOrcusSheet& mrSheet;

    SCCOL mnCol;
    SCROW mnRow;
    uint32_t mnColRange;
    uint32_t mnRowRange;
    OUString maFormula;
    formula::FormulaGrammar::Grammar meGrammar;

    void reset();

public:
    ScOrcusArrayFormula( ScOrcusSheet& rSheet );
    virtual ~ScOrcusArrayFormula() override;

    virtual void set_range(const orcus::spreadsheet::range_t& range) override;
    virtual void set_formula(orcus::spreadsheet::formula_grammar_t grammar, std::string_view formula) override;
    virtual void set_result_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) override;
    virtual void set_result_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, std::string_view value) override;
    virtual void set_result_empty(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col) override;
    virtual void set_result_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) override;
    virtual void commit() override;
};

class ScOrcusSheet : public orcus::spreadsheet::iface::import_sheet
{
    friend class ScOrcusFormula;
    friend class ScOrcusArrayFormula;

    ScDocumentImport& mrDoc;
    SCTAB mnTab;
    ScOrcusFactory& mrFactory;
    ScOrcusStyles& mrStyles;
    sc::SharedFormulaGroups maFormulaGroups;

    ScOrcusAutoFilter maAutoFilter;
    ScOrcusSheetProperties maProperties;
    ScOrcusConditionalFormat maConditionalFormat;
    ScOrcusNamedExpression maNamedExpressions;
    ScOrcusFormula maFormula;
    ScOrcusArrayFormula maArrayFormula;

    int mnCellCount;

    void cellInserted();

    ScDocumentImport& getDoc();

public:
    ScOrcusSheet(ScDocumentImport& rDoc, SCTAB nTab, ScOrcusFactory& rFactory);

    virtual orcus::spreadsheet::iface::import_auto_filter* get_auto_filter() override;
    virtual orcus::spreadsheet::iface::import_table* get_table() override;
    virtual orcus::spreadsheet::iface::import_sheet_properties* get_sheet_properties() override;
    virtual orcus::spreadsheet::iface::import_conditional_format* get_conditional_format() override;
    virtual orcus::spreadsheet::iface::import_named_expression* get_named_expression() override;
    virtual orcus::spreadsheet::iface::import_formula* get_formula() override;
    virtual orcus::spreadsheet::iface::import_array_formula* get_array_formula() override;

    // Orcus import interface
    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, std::string_view value) override;
    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::string_id_t sindex) override;
    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) override;
    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) override;
    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, int year, int month, int day, int hour, int minute, double second) override;

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index) override;
    virtual void set_format(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end, size_t xf_index) override;

    virtual orcus::spreadsheet::range_size_t get_sheet_size() const override;

    virtual void fill_down_cells(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::row_t range_size) override;

    SCTAB getIndex() const { return mnTab; }

    const sc::SharedFormulaGroups& getSharedFormulaGroups() const;
    sc::SharedFormulaGroups& getSharedFormulaGroups();
    ScOrcusFactory& getFactory();
};

class ScOrcusStyles : public orcus::spreadsheet::iface::import_styles
{
private:
    ScOrcusFactory& mrFactory;

    struct font
    {
        std::optional<OUString> maName;
        std::optional<double> mnSize;
        std::optional<Color> maColor;
        std::optional<bool> mbBold;
        std::optional<bool> mbItalic;
        std::optional<FontLineStyle> meUnderline;
        std::optional<Color> maUnderlineColor;
        std::optional<FontStrikeout> meStrikeout;

        void applyToItemSet(SfxItemSet& rSet) const;
    };

    font maCurrentFont;
    std::vector<font> maFonts;

    struct fill
    {
        orcus::spreadsheet::fill_pattern_t mePattern;
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
            SvxBorderLineStyle meStyle;
            Color maColor;
            double mnWidth;

            border_line();
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
        void applyToItemSet(SfxItemSet& rSet, const ScDocument& rDoc) const;
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
        bool mbAlignment;

        SvxCellHorJustify meHorAlignment;
        SvxCellVerJustify meVerAlignment;
        SvxCellJustifyMethod meHorAlignMethod;
        SvxCellJustifyMethod meVerAlignMethod;

        xf();
    };

    xf maCurrentXF;
    std::vector<xf> maCellStyleXfs;
    std::vector<xf> maCellXfs;

    struct cell_style
    {
        OUString maName;
        OUString maParentName;
        size_t mnXFId;
        size_t mnBuiltInId;

        cell_style();
    };

    cell_style maCurrentCellStyle;

    void applyXfToItemSet(SfxItemSet& rSet, const xf& rXf);

public:
    ScOrcusStyles( ScOrcusFactory& rFactory, bool bSkipDefaultStyles=false );

    void applyXfToItemSet(SfxItemSet& rSet, size_t xfId);

    // font

    virtual void set_font_count(size_t n) override;
    virtual void set_font_bold(bool b) override;
    virtual void set_font_italic(bool b) override;
    virtual void set_font_name(std::string_view name) override;
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
    virtual void set_strikethrough_style(orcus::spreadsheet::strikethrough_style_t s) override;
    virtual void set_strikethrough_type(orcus::spreadsheet::strikethrough_type_t s) override;
    virtual void set_strikethrough_width(orcus::spreadsheet::strikethrough_width_t s) override;
    virtual void set_strikethrough_text(orcus::spreadsheet::strikethrough_text_t s) override;
    virtual size_t commit_font() override;

    // fill

    virtual void set_fill_count(size_t n) override;
    virtual void set_fill_pattern_type(orcus::spreadsheet::fill_pattern_t fp) override;
    virtual void set_fill_fg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;
    virtual void set_fill_bg_color(orcus::spreadsheet::color_elem_t alpha, orcus::spreadsheet::color_elem_t red, orcus::spreadsheet::color_elem_t green, orcus::spreadsheet::color_elem_t blue) override;
    virtual size_t commit_fill() override;

    // border

    virtual void set_border_count(size_t n) override;

    virtual void set_border_style(orcus::spreadsheet::border_direction_t dir, orcus::spreadsheet::border_style_t style) override;
    virtual void set_border_color(orcus::spreadsheet::border_direction_t dir,
            orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;
    virtual void set_border_width(orcus::spreadsheet::border_direction_t dir, double val, orcus::length_unit_t unit) override;
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
    virtual void set_number_format_code(std::string_view s) override;
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
    virtual void set_cell_style_name(std::string_view name) override;
    virtual void set_cell_style_xf(size_t index) override;
    virtual void set_cell_style_builtin(size_t index) override;
    virtual void set_cell_style_parent_name(std::string_view name) override;
    virtual size_t commit_cell_style() override;
};

class ScOrcusFactory : public orcus::spreadsheet::iface::import_factory
{
    struct CellStoreToken
    {
        enum class Type : sal_uInt8
        {
            Auto,
            Numeric,
            String,
            Formula,
            FormulaWithResult,
            SharedFormula,
            SharedFormulaWithResult,
            Matrix,
            FillDownCells
        };


        OUString maStr1;
        OUString maStr2;
        double mfValue;

        ScAddress maPos;
        Type meType;

        uint32_t mnIndex1;
        uint32_t mnIndex2;
        formula::FormulaGrammar::Grammar meGrammar;

        CellStoreToken( const ScAddress& rPos, Type eType );
        CellStoreToken( const ScAddress& rPos, double fValue );
        CellStoreToken( const ScAddress& rPos, uint32_t nIndex );
        CellStoreToken( const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar );
    };

    typedef std::unordered_map<OUString, size_t> StringHashType;
    typedef std::vector<CellStoreToken> CellStoreTokensType;

    ScDocumentImport maDoc;

    std::vector<OUString> maStrings;
    StringHashType maStringHash;

    CellStoreTokensType maCellStoreTokens;
    ScOrcusGlobalSettings maGlobalSettings;
    ScOrcusSharedStrings maSharedStrings;
    ScOrcusNamedExpression maNamedExpressions;
    std::vector<std::unique_ptr<ScOrcusSheet>> maSheets;
    ScOrcusStyles maStyles;

    int mnProgress;

    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

public:
    ScOrcusFactory(ScDocument& rDoc, bool bSkipDefaultStyles=false);

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(
        orcus::spreadsheet::sheet_t sheet_index, std::string_view sheet_name) override;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(std::string_view sheet_name) override;
    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(orcus::spreadsheet::sheet_t sheet_index) override;
    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings() override;
    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings() override;
    virtual orcus::spreadsheet::iface::import_named_expression* get_named_expression() override;
    virtual orcus::spreadsheet::iface::import_styles* get_styles() override;
    virtual void finalize() override;

    ScDocumentImport& getDoc();

    size_t appendString(const OUString& rStr);
    size_t addString(const OUString& rStr);
    const OUString* getString(size_t nIndex) const;

    void pushCellStoreAutoToken( const ScAddress& rPos, const OUString& rVal );
    void pushCellStoreToken( const ScAddress& rPos, uint32_t nStrIndex );
    void pushCellStoreToken( const ScAddress& rPos, double fValue );
    void pushCellStoreToken(
        const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar );
    void pushFillDownCellsToken( const ScAddress& rPos, uint32_t nFillSize );

    void pushSharedFormulaToken( const ScAddress& rPos, uint32_t nIndex );
    void pushMatrixFormulaToken(
        const ScAddress& rPos, const OUString& rFormula, formula::FormulaGrammar::Grammar eGrammar,
        uint32_t nRowRange, uint32_t nColRange );

    void pushFormulaResult( const ScAddress& rPos, double fValue );
    void pushFormulaResult( const ScAddress& rPos, const OUString& rValue );

    void incrementProgress();

    void setStatusIndicator(const css::uno::Reference<css::task::XStatusIndicator>& rIndicator);

    const ScOrcusGlobalSettings& getGlobalSettings() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
