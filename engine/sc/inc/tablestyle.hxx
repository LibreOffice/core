/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <o3tl/enumarray.hxx>

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
    FirstHeaderCell, // header row, first column
    LastHeaderCell, // header row, last column
    FirstTotalCell, // total row, first column
    LastTotalCell, // total row, last column
    LAST = LastTotalCell // keep on the last element
};

// The visual family a table style belongs to, derived from its programmatic
// name. The built-in styles are named TableStyleLight<n>, TableStyleMedium<n>
// and TableStyleDark<n>; every other name, custom styles included, is Custom.
// The order matches the order the families are grouped and sorted for display.
enum class ScTableStyleFamily
{
    Light,
    Medium,
    Dark,
    Custom
};

// Classify a table style. Only the app's own built-ins have a Light, Medium or
// Dark family; every other style is Custom. This is the single place that knows
// that rule, so the grouping, the sort order and the swatch colours all agree on
// which family a style belongs to.
ScTableStyleFamily ScGetTableStyleFamily(std::u16string_view rName, bool bIsBuiltin);

class SC_DLLPUBLIC ScTableStyle
{
private:
    ScTableStyle(ScTableStyle const&) = delete;
    ScTableStyle(ScTableStyle&&) = delete;
    void operator=(ScTableStyle const&) = delete;
    void operator=(ScTableStyle&&) = delete;

    o3tl::enumarray<ScTableStyleElement, std::unique_ptr<ScPatternAttr>> maPatterns;

    // Whether that element's pattern sets any of the font attributes
    o3tl::enumarray<ScTableStyleElement, bool> maHasFontAttr;

    sal_Int32 mnFirstRowStripeSize;
    sal_Int32 mnSecondRowStripeSize;
    sal_Int32 mnFirstColStripeSize;
    sal_Int32 mnSecondColStripeSize;

    OUString maStyleName;
    std::optional<OUString> maUIName;
    bool mbIsOOXMLDefault;

    // Font attributes merged across the table elements
    mutable std::unordered_map<sal_uInt32, std::unique_ptr<SfxItemSet>> maMergedFontSets;

    // Borders built per position, keyed by what GetBoxItem's result depends on. A null
    // entry = the style draws no border there. Dropped whenever a pattern changes.
    mutable std::unordered_map<sal_uInt32, std::unique_ptr<SvxBoxItem>> maBoxItems;

    // The bits of rDBData and the cell position that GetBoxItem's outcome turns on
    sal_uInt32 GetBoxCacheKey(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                              SCROW nRowIndex) const;

    std::unique_ptr<SvxBoxItem> BuildBoxItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                             SCROW nRowIndex) const;

    template <class T>
    const T* GetElementItem(ScTableStyleElement eElement, TypedWhichId<T> nWhich) const
    {
        const std::unique_ptr<ScPatternAttr>& rpPattern = maPatterns[eElement];
        return rpPattern ? rpPattern->GetItemSet().GetItemIfSet(nWhich) : nullptr;
    }

public:
    ScTableStyle(const OUString& rName, const std::optional<OUString>& rUIName);

    const SfxItemSet* GetFontItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                     SCROW nRowIndex) const;
    const SvxBrushItem* GetFillItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                    SCROW nRowIndex) const;
    const SvxBoxItem* GetBoxItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                 SCROW nRowIndex) const;

    // Materialize this style's fill/border/font onto the Table's cells as direct cell attributes
    // (Convert to Range), only where the style is what actually renders; attributes that already
    // win (direct, cell style, conditional) are left untouched. Precedence details are at the
    // per-attribute logic in the implementation.
    void BakeInto(ScDocument& rDoc, const ScDBData& rDBData) const;

    void SetRowStripeSize(sal_Int32 nFirstRowStripeSize, sal_Int32 nSecondRowStripeSize);
    void SetColStripeSize(sal_Int32 nFirstColStripeSize, sal_Int32 nSecondColStripeSize);

    void SetPattern(ScTableStyleElement eTableStyleElement,
                    std::unique_ptr<ScPatternAttr> pPattern);

    std::map<ScTableStyleElement, const ScPatternAttr*> GetSetPatterns() const;

    void SetOOXMLDefault(bool bDefault);
    bool IsOOXMLDefault() const;

    /// Re-resolve themed colors in custom style patterns against a new ColorSet
    void UpdateThemedColors(const model::ColorSet& rColorSet);

    const OUString& GetName() const;
    const OUString& GetUIName() const;
};

class SC_DLLPUBLIC ScTableStyles
{
    ScDocument* mpDoc;

private:
    ScTableStyles(ScTableStyles const&) = delete;
    ScTableStyles(ScTableStyles&&) = delete;
    void operator=(ScTableStyles const&) = delete;
    void operator=(ScTableStyles&&) = delete;

    std::unordered_map<OUString, std::unique_ptr<ScTableStyle>> maTableStyles;
    // The display names the styles answer to; a lazily filled cache. The valid flag
    // is cleared whenever maTableStyles changes so the names are gathered again.
    mutable std::unordered_set<OUString> maUINamesInUse;
    mutable bool mbUINamesInUseValid = false;

    // Name of the style applied to newly inserted tables in this document.
    OUString maDefaultStyleName;

    void InvalidateBindings();

public:
    ScTableStyles(ScDocument* pDoc);

    // Register a style under its programmatic name. A style already registered
    // under that name is kept, so the first definition of a name wins.
    void AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle);
    const ScTableStyle* GetTableStyle(const OUString& rName) const;
    bool HasTableStyle() const { return !maTableStyles.empty(); }

    // A programmatic name of the form TableStyleCustom<n> that no style uses yet,
    // so a style registered under it is classified as Custom.
    OUString GetUnusedCustomStyleName() const;

    // A name to show the user that no style goes by yet: rBaseName on its own when
    // it is free, otherwise rBaseName followed by the lowest count that is free.
    OUString GetUnusedUIName(const OUString& rBaseName) const;

    // Register a copy of the style named rSourceName under a fresh Custom
    // programmatic name, and return that name. The name shown to the user starts
    // from rNewUIName and gains a count when a style already goes by it, so two
    // copies of one style stay apart in the gallery. The copy owns its own
    // patterns, so later edits to either style leave the other alone. An empty
    // return means no style is registered under rSourceName and nothing was added.
    OUString DuplicateTableStyle(const OUString& rSourceName, const OUString& rNewUIName);

    // All table styles ordered by programmatic name, giving callers a stable
    // display order over the unordered internal storage.
    std::vector<const ScTableStyle*> GetSortedTableStyles() const;

    const OUString& GetDefaultStyleName() const { return maDefaultStyleName; }
    void SetDefaultStyleName(const OUString& rName) { maDefaultStyleName = rName; }

    /// Remove all styles marked as OOXML defaults (for regeneration after theme change)
    void ClearOOXMLDefaultStyles();

    /// Update themed colors in custom (non-default) styles after a theme change
    void UpdateCustomStyleThemedColors(const model::ColorSet& rColorSet);

    void generateJSON(tools::JsonWriter& rWriter) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
