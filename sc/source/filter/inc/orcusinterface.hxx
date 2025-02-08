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
#include <editutil.hxx>

#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editobj.hxx>

#include "sharedformulagroups.hxx"

#include <conditio.hxx>

#include <rtl/strbuf.hxx>
#include <editeng/borderline.hxx>

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/import_interface_underline.hpp>
#include <orcus/spreadsheet/import_interface_strikethrough.hpp>

#include <memory>
#include <map>
#include <unordered_map>
#include <vector>
#include <variant>

class ScOrcusSheet;
class ScOrcusStyles;
class ScOrcusFactory;
class ScOrcusImportFontStyle;
class ScOrcusSharedStrings;
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

struct ScOrcusStrikethrough
{
    std::optional<orcus::spreadsheet::strikethrough_style_t> meStyle;
    std::optional<orcus::spreadsheet::strikethrough_type_t> meType;
    std::optional<orcus::spreadsheet::strikethrough_width_t> meWidth;
    std::optional<orcus::spreadsheet::strikethrough_text_t> meText;

    void reset();
    std::optional<FontStrikeout> toFontStrikeout() const;
};

struct ScOrcusUnderline
{
    std::optional<orcus::spreadsheet::underline_style_t> meStyle;
    std::optional<orcus::spreadsheet::underline_thickness_t> meThickness;
    std::optional<orcus::spreadsheet::underline_spacing_t> meSpacing;
    std::optional<orcus::spreadsheet::underline_count_t> meCount;

    void reset();
    std::optional<FontLineStyle> toFontLineStyle() const;
};

class ScOrcusSegmentStrikethrough : public orcus::spreadsheet::iface::import_strikethrough
{
    friend class ScOrcusSharedStrings;

    SfxItemSet* mpDestFormat = nullptr;
    ScOrcusStrikethrough maAttrs;

    void reset(SfxItemSet* pDestFormat);

public:
    void set_style(orcus::spreadsheet::strikethrough_style_t s) override;
    void set_type(orcus::spreadsheet::strikethrough_type_t s) override;
    void set_width(orcus::spreadsheet::strikethrough_width_t s) override;
    void set_text(orcus::spreadsheet::strikethrough_text_t s) override;
    void commit() override;
};

class ScOrcusSegmentUnderline : public orcus::spreadsheet::iface::import_underline
{
    friend class ScOrcusSharedStrings;

    SfxItemSet* mpDestFormat = nullptr;

    ScOrcusUnderline maAttrs;
    std::optional<Color> maColor;

    void reset(SfxItemSet* pDestFormat);

public:
    void set_style(orcus::spreadsheet::underline_style_t e) override;
    void set_thickness(orcus::spreadsheet::underline_thickness_t e) override;
    void set_spacing(orcus::spreadsheet::underline_spacing_t e) override;
    void set_count(orcus::spreadsheet::underline_count_t e) override;
    void set_color(
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;
    void commit() override;
};

class ScOrcusSharedStrings : public orcus::spreadsheet::iface::import_shared_strings
{
    ScOrcusFactory& mrFactory;
    ScFieldEditEngine& mrEditEngine;

    SfxItemSet maCurFormat;
    std::vector<std::pair<ESelection, SfxItemSet>> maFormatSegments;

    ScOrcusSegmentUnderline maImportUnderline;
    ScOrcusSegmentStrikethrough maImportStrikethrough;

    OUString toOUString(std::string_view s);

public:
    ScOrcusSharedStrings(ScOrcusFactory& rFactory);

    virtual size_t append(std::string_view s) override;
    virtual size_t add(std::string_view s) override;

    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_superscript(bool b) override;
    virtual void set_segment_subscript(bool b) override;
    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_font_name(std::string_view s) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(orcus::spreadsheet::color_elem_t alpha,
            orcus::spreadsheet::color_elem_t red,
            orcus::spreadsheet::color_elem_t green,
            orcus::spreadsheet::color_elem_t blue) override;

    virtual orcus::spreadsheet::iface::import_underline* start_underline() override;
    virtual orcus::spreadsheet::iface::import_strikethrough* start_strikethrough() override;

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

class ScOrcusSheetProperties : public orcus::spreadsheet::iface::import_sheet_properties
{
    ScDocumentImport& mrDoc;
    SCTAB mnTab;
public:
    ScOrcusSheetProperties(SCTAB nTab, ScDocumentImport& rDoc);
    virtual ~ScOrcusSheetProperties() override;

    virtual void set_column_width(
        orcus::spreadsheet::col_t col, orcus::spreadsheet::col_t col_span,
        double width, orcus::length_unit_t unit) override;

    virtual void set_column_hidden(
        orcus::spreadsheet::col_t col, orcus::spreadsheet::col_t col_span,
        bool hidden) override;

    virtual void set_row_height(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::row_t row_span,
        double height, orcus::length_unit_t unit) override;

    virtual void set_row_hidden(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::row_t row_span, bool hidden) override;

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
    virtual void set_column_format(
        orcus::spreadsheet::col_t col, orcus::spreadsheet::col_t col_span, std::size_t xf_index) override;
    virtual void set_row_format(orcus::spreadsheet::row_t row, std::size_t xf_index) override;

    virtual orcus::spreadsheet::range_size_t get_sheet_size() const override;

    virtual void fill_down_cells(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::row_t range_size) override;

    SCTAB getIndex() const { return mnTab; }

    const sc::SharedFormulaGroups& getSharedFormulaGroups() const;
    sc::SharedFormulaGroups& getSharedFormulaGroups();
    ScOrcusFactory& getFactory();
};

struct ScOrcusFont
{
    std::optional<OUString> maName;
    std::optional<OUString> maNameAsian;
    std::optional<OUString> maNameComplex;
    std::optional<double> mnSize;
    std::optional<double> mnSizeAsian;
    std::optional<double> mnSizeComplex;
    std::optional<Color> maColor;
    std::optional<bool> mbBold;
    std::optional<bool> mbBoldAsian;
    std::optional<bool> mbBoldComplex;
    std::optional<bool> mbItalic;
    std::optional<bool> mbItalicAsian;
    std::optional<bool> mbItalicComplex;
    std::optional<FontLineStyle> meUnderline;
    std::optional<Color> maUnderlineColor;
    std::optional<FontStrikeout> meStrikeout;

    void applyToItemSet( SfxItemSet& rSet ) const;
};

struct ScOrcusFill
{
    std::optional<orcus::spreadsheet::fill_pattern_t> mePattern;
    std::optional<Color> maFgColor;
    std::optional<Color> maBgColor; // currently not used.

    void applyToItemSet( SfxItemSet& rSet ) const;
};

struct ScOrcusBorder
{
    struct BorderLine
    {
        std::optional<SvxBorderLineStyle> meStyle;
        std::optional<Color> maColor;
        std::optional<double> mnWidth;
    };

    std::map<orcus::spreadsheet::border_direction_t, BorderLine> maBorders;

    void applyToItemSet( SfxItemSet& rSet ) const;
};

struct ScOrcusProtection
{
    std::optional<bool> mbLocked;
    std::optional<bool> mbHidden;
    std::optional<bool> mbPrintContent;
    std::optional<bool> mbFormulaHidden;

    void applyToItemSet( SfxItemSet& rSet ) const;
};

struct ScOrcusNumberFormat
{
    std::optional<OUString> maCode;

    void applyToItemSet( SfxItemSet& rSet, const ScDocument& rDoc ) const;
};

struct ScOrcusXf
{
    std::size_t mnFontId;
    std::size_t mnFillId;
    std::size_t mnBorderId;
    std::size_t mnProtectionId;
    std::size_t mnNumberFormatId;
    std::size_t mnStyleXf;

    bool mbApplyAlignment;
    std::optional<bool> mbWrapText;
    std::optional<bool> mbShrinkToFit;

    SvxCellHorJustify meHorAlignment;
    SvxCellVerJustify meVerAlignment;
    SvxCellJustifyMethod meHorAlignMethod;
    SvxCellJustifyMethod meVerAlignMethod;

    ScOrcusXf();
};

struct ScOrcusCellStyle
{
    OUString maName;
    OUString maDisplayName;
    OUString maParentName;
    std::size_t mnXFId;
    std::size_t mnBuiltInId;

    ScOrcusCellStyle();
};

class ScOrcusImportFontUnderlineStyle : public orcus::spreadsheet::iface::import_underline
{
    friend class ScOrcusImportFontStyle;

    ScOrcusFont* mpDestFont = nullptr;
    ScOrcusUnderline maAttrs;
    std::optional<Color> maColor;

    void reset(ScOrcusFont* pDest);

public:
    void set_style(orcus::spreadsheet::underline_style_t e) override;
    void set_thickness(orcus::spreadsheet::underline_thickness_t e) override;
    void set_spacing(orcus::spreadsheet::underline_spacing_t e) override;
    void set_count(orcus::spreadsheet::underline_count_t e) override;
    void set_color(
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;
    void commit() override;
};

class ScOrcusImportFontStrikethroughStyle : public orcus::spreadsheet::iface::import_strikethrough
{
    friend class ScOrcusImportFontStyle;

    ScOrcusFont* mpDestFont = nullptr;
    ScOrcusStrikethrough maAttrs;

    void reset(ScOrcusFont* pDest);

public:
    void set_style(orcus::spreadsheet::strikethrough_style_t s) override;
    void set_type(orcus::spreadsheet::strikethrough_type_t s) override;
    void set_width(orcus::spreadsheet::strikethrough_width_t s) override;
    void set_text(orcus::spreadsheet::strikethrough_text_t s) override;
    void commit() override;
};

class ScOrcusImportFontStyle : public orcus::spreadsheet::iface::import_font_style
{
    ScOrcusFont maCurrentFont;
    ScOrcusFactory& mrFactory;
    std::vector<ScOrcusFont>& mrFonts;

    ScOrcusImportFontUnderlineStyle maUnderlineImport;
    ScOrcusImportFontStrikethroughStyle maStrikeoutImport;

public:
    ScOrcusImportFontStyle( ScOrcusFactory& rFactory, std::vector<ScOrcusFont>& rFonts );

    void reset();

    void set_bold(bool b) override;
    void set_bold_asian(bool b) override;
    void set_bold_complex(bool b) override;
    void set_italic(bool b) override;
    void set_italic_asian(bool b) override;
    void set_italic_complex(bool b) override;
    void set_name(std::string_view s) override;
    void set_name_asian(std::string_view s) override;
    void set_name_complex(std::string_view s) override;
    void set_size(double point) override;
    void set_size_asian(double point) override;
    void set_size_complex(double point) override;

    void set_color(
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;

    orcus::spreadsheet::iface::import_underline* start_underline() override;
    orcus::spreadsheet::iface::import_strikethrough* start_strikethrough() override;

    std::size_t commit() override;
};

class ScOrcusImportFillStyle : public orcus::spreadsheet::iface::import_fill_style
{
    ScOrcusFill maCurrentFill;
    std::vector<ScOrcusFill>& mrFills;

public:
    ScOrcusImportFillStyle( std::vector<ScOrcusFill>& rFills );

    void reset();

    void set_pattern_type(orcus::spreadsheet::fill_pattern_t fp) override;
    void set_fg_color(
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;
    void set_bg_color(
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;
    std::size_t commit() override;
};

class ScOrcusImportBorderStyle : public orcus::spreadsheet::iface::import_border_style
{
    ScOrcusBorder maCurrentBorder;
    std::vector<ScOrcusBorder>& mrBorders;

public:
    ScOrcusImportBorderStyle( std::vector<ScOrcusBorder>& rBorders );

    void reset();

    void set_width(
        orcus::spreadsheet::border_direction_t dir, double width, orcus::length_unit_t unit) override;
    void set_style(
        orcus::spreadsheet::border_direction_t dir, orcus::spreadsheet::border_style_t style) override;
    void set_color(
        orcus::spreadsheet::border_direction_t dir,
        orcus::spreadsheet::color_elem_t alpha,
        orcus::spreadsheet::color_elem_t red,
        orcus::spreadsheet::color_elem_t green,
        orcus::spreadsheet::color_elem_t blue) override;
    std::size_t commit() override;
};

class ScOrcusImportCellProtection : public orcus::spreadsheet::iface::import_cell_protection
{
    ScOrcusProtection maCurrentProtection;
    std::vector<ScOrcusProtection>& mrProtections;

public:
    ScOrcusImportCellProtection( std::vector<ScOrcusProtection>& rProtections );

    void reset();

    void set_hidden(bool b) override;
    void set_locked(bool b) override;
    void set_print_content(bool b) override;
    void set_formula_hidden(bool b) override;
    std::size_t commit() override;
};

class ScOrcusImportNumberFormat : public orcus::spreadsheet::iface::import_number_format
{
    ScOrcusNumberFormat maCurrentFormat;
    ScOrcusFactory& mrFactory;
    std::vector<ScOrcusNumberFormat>& mrNumberFormats;

public:
    ScOrcusImportNumberFormat( ScOrcusFactory& rFactory, std::vector<ScOrcusNumberFormat>& rFormats );

    void reset();

    void set_identifier(std::size_t id) override;
    void set_code(std::string_view s) override;
    std::size_t commit() override;
};

class ScOrucsImportCellStyle : public orcus::spreadsheet::iface::import_cell_style
{
    ScOrcusCellStyle maCurrentStyle;
    ScOrcusFactory& mrFactory;
    ScOrcusStyles& mrStyles;
    const std::vector<ScOrcusXf>& mrXfs;

public:
    ScOrucsImportCellStyle(
        ScOrcusFactory& rFactory, ScOrcusStyles& rStyles, const std::vector<ScOrcusXf>& rXfs );

    void reset();

    void set_name(std::string_view s) override;
    void set_display_name(std::string_view s) override;
    void set_xf(std::size_t index) override;
    void set_builtin(std::size_t index) override;
    void set_parent_name(std::string_view s) override;
    void commit() override;
};

class ScOrcusImportXf : public orcus::spreadsheet::iface::import_xf
{
    ScOrcusXf maCurrentXf;
    std::vector<ScOrcusXf>* mpXfs = nullptr;

public:
    void reset( std::vector<ScOrcusXf>& rXfs );

    void set_font(std::size_t index) override;
    void set_fill(std::size_t index) override;
    void set_border(std::size_t index) override;
    void set_protection(std::size_t index) override;
    void set_number_format(std::size_t index) override;
    void set_style_xf(std::size_t index) override;
    void set_apply_alignment(bool b) override;
    void set_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align) override;
    void set_vertical_alignment(orcus::spreadsheet::ver_alignment_t align) override;
    void set_wrap_text(bool b) override;
    void set_shrink_to_fit(bool b) override;
    std::size_t commit() override;
};

class ScOrcusStyles : public orcus::spreadsheet::iface::import_styles
{
private:
    ScOrcusFactory& mrFactory;

    std::vector<ScOrcusFont> maFonts;
    std::vector<ScOrcusFill> maFills;
    std::vector<ScOrcusBorder> maBorders;
    std::vector<ScOrcusProtection> maProtections;
    std::vector<ScOrcusNumberFormat> maNumberFormats;
    std::vector<ScOrcusXf> maCellXfs;
    std::vector<ScOrcusXf> maCellStyleXfs;
    std::vector<ScOrcusXf> maCellDiffXfs;

    ScOrcusImportFontStyle maFontStyle;
    ScOrcusImportFillStyle maFillStyle;
    ScOrcusImportBorderStyle maBorderStyle;
    ScOrcusImportCellProtection maCellProtection;
    ScOrcusImportNumberFormat maNumberFormat;
    ScOrucsImportCellStyle maCellStyle;
    ScOrcusImportXf maXf;

public:
    ScOrcusStyles( ScOrcusFactory& rFactory, bool bSkipDefaultStyles=false );

    void applyXfToItemSet( SfxItemSet& rSet, const ScOrcusXf& rXf );
    void applyXfToItemSet( SfxItemSet& rSet, std::size_t xfId );

    virtual orcus::spreadsheet::iface::import_font_style* start_font_style() override;
    virtual orcus::spreadsheet::iface::import_fill_style* start_fill_style() override;
    virtual orcus::spreadsheet::iface::import_border_style* start_border_style() override;
    virtual orcus::spreadsheet::iface::import_cell_protection* start_cell_protection() override;
    virtual orcus::spreadsheet::iface::import_number_format* start_number_format() override;
    virtual orcus::spreadsheet::iface::import_xf* start_xf(orcus::spreadsheet::xf_category_t cat) override;
    virtual orcus::spreadsheet::iface::import_cell_style* start_cell_style() override;

    virtual void set_font_count(size_t n) override;
    virtual void set_fill_count(size_t n) override;
    virtual void set_border_count(size_t n) override;
    virtual void set_number_format_count(size_t n) override;
    virtual void set_xf_count(orcus::spreadsheet::xf_category_t cat, size_t n) override;
    virtual void set_cell_style_count(size_t n) override;
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
        CellStoreToken( const ScAddress& rPos, OUString aFormula, formula::FormulaGrammar::Grammar eGrammar );
    };

    using StringValueType = std::variant<OUString, std::unique_ptr<EditTextObject>>;
    typedef std::unordered_map<OUString, size_t> StringHashType;
    typedef std::vector<CellStoreToken> CellStoreTokensType;

    ScDocumentImport maDoc;

    std::vector<StringValueType> maStrings;
    StringHashType maStringHash;

    CellStoreTokensType maCellStoreTokens;
    ScOrcusGlobalSettings maGlobalSettings;
    ScOrcusRefResolver maRefResolver;
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
    virtual orcus::spreadsheet::iface::import_reference_resolver* get_reference_resolver(
        orcus::spreadsheet::formula_ref_context_t cxt) override;
    virtual void finalize() override;

    ScDocumentImport& getDoc();

    size_t appendString(const OUString& rStr);
    size_t addString(const OUString& rStr);
    std::size_t appendFormattedString(std::unique_ptr<EditTextObject> pEditText);

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
