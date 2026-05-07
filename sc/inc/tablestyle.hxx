/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <unordered_map>

#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <tools/json_writer.hxx>
#include "document.hxx"
#include "scdllapi.h"
#include "dbdata.hxx"

class ScPatternAttr;
namespace model
{
class ColorSet;
}

namespace tools
{
class JsonWriter;
}

// Mirrors the OOXML ST_TableStyleType enumeration (ECMA-376 §18.18.83). The
// first 11 values are the regular-table-style elements; the rest are needed
// for full pivot-table-style parity. Levels (first/second/third) for
// subheadings and subtotals follow the row-axis or column-axis field index;
// fields beyond level 2 wrap modulo 3 (Excel's documented behaviour).
enum class ScTableStyleElement
{
    WholeTable,
    FirstColumnStripe,
    SecondColumnStripe,
    FirstRowStripe,
    SecondRowStripe,
    LastColumn,
    FirstColumn,
    HeaderRow,
    TotalRow,
    FirstHeaderCell,
    LastHeaderCell,
    // Table-style-only additions
    FirstTotalCell,
    LastTotalCell,
    // Pivot-style-only additions
    PageFieldLabels,
    PageFieldValues,
    FirstSubtotalRow,
    SecondSubtotalRow,
    ThirdSubtotalRow,
    FirstSubtotalColumn,
    SecondSubtotalColumn,
    ThirdSubtotalColumn,
    FirstColumnSubheading,
    SecondColumnSubheading,
    ThirdColumnSubheading,
    FirstRowSubheading,
    SecondRowSubheading,
    ThirdRowSubheading,
    BlankRow,
};

template <class T> const T* GetItemFromPattern(const ScPatternAttr* pPattern, TypedWhichId<T> nWhich)
{
    return pPattern->GetItemSet().GetItemIfSet(nWhich);
}

class SC_DLLPUBLIC ScTableStyle
{
private:
    ScTableStyle(ScTableStyle const&) = delete;
    ScTableStyle(ScTableStyle&&) = delete;
    void operator=(ScTableStyle const&) = delete;
    void operator=(ScTableStyle&&) = delete;

    // Element -> pattern map. Replaces a fixed set of named unique_ptr members so
    // pivot-style elements (subtotals, subheadings, page-field cells, etc.) can be
    // stored without ballooning the class with one member per element.
    std::unordered_map<ScTableStyleElement, std::unique_ptr<ScPatternAttr>> maPatterns;

    /// Returns the pattern for an element, or nullptr if not set.
    const ScPatternAttr* lookupPattern(ScTableStyleElement eElement) const;

    sal_Int32 mnFirstRowStripeSize;
    sal_Int32 mnSecondRowStripeSize;
    sal_Int32 mnFirstColStripeSize;
    sal_Int32 mnSecondColStripeSize;

    OUString maStyleName;
    std::optional<OUString> maUIName;
    bool mbIsOOXMLDefault;

public:
    ScTableStyle(const OUString& rName, const std::optional<OUString>& rUIName);

    static bool HasFontAttrSet(const ScPatternAttr* pPattern);
    const SfxItemSet* GetFontItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                     SCROW nRowIndex) const;
    const SvxBrushItem* GetFillItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                    SCROW nRowIndex) const;
    std::unique_ptr<SvxBoxItem> GetBoxItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                           SCROW nRowIndex) const;

    void SetRowStripeSize(sal_Int32 nFirstRowStripeSize, sal_Int32 nSecondRowStripeSize);
    void SetColStripeSize(sal_Int32 nFirstColStripeSize, sal_Int32 nSecondColStripeSize);

    void SetPattern(ScTableStyleElement eTableStyleElement,
                    std::unique_ptr<ScPatternAttr> pPattern);
    void SetTablePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetSecondColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetSecondRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetLastColumnPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstColumnPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetHeaderRowPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetTotalRowPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetFirstHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern);
    void SetLastHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern);

    std::map<ScTableStyleElement, const ScPatternAttr*> GetSetPatterns() const;

    sal_Int32 GetFirstRowStripeSize() const;
    sal_Int32 GetFirstColumnStripeSize() const;
    sal_Int32 GetSecondRowStripeSize() const;
    sal_Int32 GetSecondColumnStripeSize() const;

    void SetOOXMLDefault(bool bDefault);
    bool IsOOXMLDefault() const;

    /// Re-resolve themed colors in custom style patterns against a new ColorSet
    void UpdateThemedColors(const model::ColorSet& rColorSet);

    const OUString& GetName() const;
    const OUString& GetUIName() const;
};

class SC_DLLPUBLIC ScTableStyles
{
    SfxBindings* mpBindings;

private:
    ScTableStyles(ScTableStyles const&) = delete;
    ScTableStyles(ScTableStyles&&) = delete;
    void operator=(ScTableStyles const&) = delete;
    void operator=(ScTableStyles&&) = delete;

    std::unordered_map<OUString, std::unique_ptr<ScTableStyle>> maTableStyles;

public:
    ScTableStyles(SfxBindings* pBindings);

    void AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle);
    void DeleteTableStyle(const OUString& rName);
    const ScTableStyle* GetTableStyle(const OUString& rName) const;
    bool HasTableStyle() const { return !maTableStyles.empty(); }

    /// Remove all styles marked as OOXML defaults (for regeneration after theme change)
    void ClearOOXMLDefaultStyles();

    /// Update themed colors in custom (non-default) styles after a theme change
    void UpdateCustomStyleThemedColors(const model::ColorSet& rColorSet);

    void generateJSON(tools::JsonWriter& rWriter) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
