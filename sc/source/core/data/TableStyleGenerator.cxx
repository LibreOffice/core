/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <TableStyleGenerator.hxx>

#include <docmodel/theme/ColorSet.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/colritem.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/itemset.hxx>

#include <document.hxx>
#include <patattr.hxx>
#include <tablestyle.hxx>
#include <sc.hrc>

#include "defaulttablestyles.inc"

namespace
{
// OOXML theme index to model::ThemeColorType mapping
// (from spnColorTokens in stylesbuffer.cxx:266-268)
// Index: 0=lt1, 1=dk1, 2=lt2, 3=dk2, 4-9=accent1-6, 10=hlink, 11=folHlink
constexpr model::ThemeColorType aOoxmlIndexToThemeType[] = {
    model::ThemeColorType::Light1, // 0
    model::ThemeColorType::Dark1, // 1
    model::ThemeColorType::Light2, // 2
    model::ThemeColorType::Dark2, // 3
    model::ThemeColorType::Accent1, // 4
    model::ThemeColorType::Accent2, // 5
    model::ThemeColorType::Accent3, // 6
    model::ThemeColorType::Accent4, // 7
    model::ThemeColorType::Accent5, // 8
    model::ThemeColorType::Accent6, // 9
    model::ThemeColorType::Hyperlink, // 10
    model::ThemeColorType::FollowedHyperlink, // 11
};

struct ResolvedColor
{
    ::Color maColor;
    model::ComplexColor maComplexColor;
};

/// Build a ComplexColor from OOXML theme index + tint and resolve it.
/// Tint conversion replicates addExcelTintTransformation() from oox/source/drawingml/color.cxx.
///
/// Note: We resolve the color manually using Color::ApplyLumModOff() rather than
/// ColorSet::resolveColor(), because ComplexColor::applyTransformations() applies
/// LumMod and LumOff as separate sequential operations, but they must be applied
/// together in a single ApplyLumModOff() call to get correct results.
ResolvedColor resolveThemeColor(const model::ColorSet& rColorSet, const ThemeColor& rThemeColor)
{
    ResolvedColor aResult;

    assert(rThemeColor.nThemeId >= 0
           && rThemeColor.nThemeId < static_cast<int>(std::size(aOoxmlIndexToThemeType)));
    model::ThemeColorType eType = aOoxmlIndexToThemeType[rThemeColor.nThemeId];

    // Build the ComplexColor for storage (used for theme-aware round-tripping)
    model::ComplexColor aComplexColor;
    aComplexColor.setThemeColor(eType);

    double fTint = rThemeColor.fTint;
    // OOXML scale: 100'000 = 100% (used for ComplexColor transforms)
    sal_Int32 nLumModOox = 100'000;
    sal_Int32 nLumOffOox = 0;

    if (fTint > 0.0)
    {
        sal_Int32 nValue = std::round(std::abs(fTint) * 100'000.0);
        nLumModOox = 100'000 - nValue;
        nLumOffOox = nValue;
        aComplexColor.addTransformation({ model::TransformationType::LumMod, nLumModOox });
        aComplexColor.addTransformation({ model::TransformationType::LumOff, nLumOffOox });
    }
    else if (fTint < 0.0)
    {
        sal_Int32 nValue = std::round(std::abs(fTint) * 100'000.0);
        nLumModOox = 100'000 - nValue;
        aComplexColor.addTransformation({ model::TransformationType::LumMod, nLumModOox });
    }

    aResult.maComplexColor = aComplexColor;

    // Resolve: get base theme color, then apply LumMod+LumOff together.
    // ApplyLumModOff uses 10'000 = 100% scale (sal_Int16), so divide OOXML values by 10.
    ::Color aColor = rColorSet.getColor(eType);
    sal_Int16 nLumMod = static_cast<sal_Int16>(nLumModOox / 10);
    sal_Int16 nLumOff = static_cast<sal_Int16>(nLumOffOox / 10);
    if (nLumMod != 10'000 || nLumOff != 0)
        aColor.ApplyLumModOff(nLumMod, nLumOff);
    aResult.maColor = aColor;

    return aResult;
}

// Use the exact same path as the oox filter: create a BorderLine2 and call
// SvxBoxItem::LineToSvxLine(). This guarantees identical SvxBorderLine results.

void setBorderLine(editeng::SvxBorderLine& rLine, const ResolvedColor& rColor,
                   BorderElementStyle eStyle)
{
    css::table::BorderLine2 bl2;
    bl2.Color = sal_Int32(rColor.maColor);
    bl2.OuterLineWidth = 0;
    bl2.InnerLineWidth = 0;
    bl2.LineDistance = 0;
    bl2.LineStyle = 0; // css::table::BorderLineStyle::SOLID
    bl2.LineWidth = 0;

    switch (eStyle)
    {
        case BorderElementStyle::THIN:
            bl2.OuterLineWidth = SvxBorderLineWidth::Thin;
            break;
        case BorderElementStyle::MEDIUM:
            bl2.OuterLineWidth = SvxBorderLineWidth::Medium;
            break;
        case BorderElementStyle::THICK:
            bl2.OuterLineWidth = SvxBorderLineWidth::Thick;
            break;
        case BorderElementStyle::DOUBLE:
            bl2.OuterLineWidth = SvxBorderLineWidth::VeryThin;
            bl2.LineDistance = SvxBorderLineWidth::Thin;
            bl2.InnerLineWidth = SvxBorderLineWidth::VeryThin;
            bl2.LineStyle = sal_Int16(css::table::BorderLineStyle::DOUBLE_THIN);
            break;
    }

    SvxBoxItem::LineToSvxLine(bl2, rLine, false);
    rLine.setComplexColor(rColor.maComplexColor);
}

} // end anonymous namespace

void ScTableStyleGenerator::generateDefaultStyles(ScDocument& rDoc,
                                                  const model::ColorSet& rColorSet)
{
    // Clear existing OOXML default styles before regeneration
    ScTableStyles* pTableStyles = rDoc.GetTableStyles();
    if (!pTableStyles)
        return;

    pTableStyles->ClearOOXMLDefaultStyles();

    // Step 1: Resolve all theme colors
    size_t nColors = std::size(aThemeColors);
    std::vector<ResolvedColor> aColors(nColors);
    for (size_t i = 0; i < nColors; ++i)
        aColors[i] = resolveThemeColor(rColorSet, aThemeColors[i]);

    // Step 2: Build fill items (SvxBrushItem)
    size_t nFills = std::size(aFills);
    std::vector<SvxBrushItem> aFillItems(nFills, SvxBrushItem(ATTR_BACKGROUND));
    for (size_t i = 0; i < nFills; ++i)
    {
        const ::Fill& rFill = aFills[i];
        aFillItems[i].SetColor(aColors[rFill.nFgColorId].maColor);
        aFillItems[i].setComplexColor(aColors[rFill.nFgColorId].maComplexColor);
    }

    // Step 3: Build border items (SvxBoxItem + SvxBoxInfoItem)
    size_t nBorders = std::size(aBorders);
    std::vector<SvxBoxItem> aBoxItems(nBorders, SvxBoxItem(ATTR_BORDER));
    std::vector<SvxBoxInfoItem> aBoxInfoItems(nBorders, SvxBoxInfoItem(ATTR_BORDER_INNER));
    for (size_t i = 0; i < nBorders; ++i)
    {
        const ::Border& rBorder = aBorders[i];
        editeng::SvxBorderLine aLine;

        if (rBorder.nTopId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nTopId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxItems[i].SetLine(&aLine, SvxBoxItemLine::TOP);
        }
        if (rBorder.nBottomId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nBottomId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxItems[i].SetLine(&aLine, SvxBoxItemLine::BOTTOM);
        }
        if (rBorder.nLeftId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nLeftId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxItems[i].SetLine(&aLine, SvxBoxItemLine::LEFT);
        }
        if (rBorder.nRightId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nRightId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxItems[i].SetLine(&aLine, SvxBoxItemLine::RIGHT);
        }
        if (rBorder.nVerticalId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nVerticalId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxInfoItems[i].SetLine(&aLine, SvxBoxInfoItemLine::VERT);
        }
        if (rBorder.nHorizontalId >= 0)
        {
            const BorderElement& rElem = aBorderElements[rBorder.nHorizontalId];
            setBorderLine(aLine, aColors[rElem.nColorId], rElem.eBorderStyle);
            aBoxInfoItems[i].SetLine(&aLine, SvxBoxInfoItemLine::HORI);
        }
    }

    // Step 4: Build font data (bold + color)
    size_t nFonts = std::size(aFonts);
    struct FontData
    {
        bool bBold;
        ::Color maColor;
        model::ComplexColor maComplexColor;
    };
    std::vector<FontData> aFontData(nFonts);
    for (size_t i = 0; i < nFonts; ++i)
    {
        const ::Font& rFont = aFonts[i];
        aFontData[i].bBold = rFont.bBold;
        aFontData[i].maColor = aColors[rFont.nThemeColorId].maColor;
        aFontData[i].maComplexColor = aColors[rFont.nThemeColorId].maComplexColor;
    }

    // Step 5: Build DXF patterns (ScPatternAttr)
    size_t nDxfs = std::size(aDxfs);
    std::vector<std::unique_ptr<ScPatternAttr>> aDxfPatterns(nDxfs);
    for (size_t i = 0; i < nDxfs; ++i)
    {
        const ::Dxf& rDxf = aDxfs[i];
        auto pPattern = std::make_unique<ScPatternAttr>(rDoc.getCellAttributeHelper());
        SfxItemSet& rItemSet = pPattern->GetItemSetWritable();

        if (rDxf.nFillId >= 0)
            rItemSet.Put(aFillItems[rDxf.nFillId]);

        if (rDxf.nBorderId >= 0)
        {
            const SvxBoxItem& rBox = aBoxItems[rDxf.nBorderId];
            bool bHasOuter
                = rBox.GetLine(SvxBoxItemLine::TOP) || rBox.GetLine(SvxBoxItemLine::BOTTOM)
                  || rBox.GetLine(SvxBoxItemLine::LEFT) || rBox.GetLine(SvxBoxItemLine::RIGHT);

            // Only put SvxBoxItem if it has outer lines (matching filter behavior)
            if (bHasOuter)
                rItemSet.Put(rBox);

            // Only put SvxBoxInfoItem if it has vertical or horizontal lines
            const SvxBoxInfoItem& rInfo = aBoxInfoItems[rDxf.nBorderId];
            if (rInfo.GetVert() || rInfo.GetHori())
            {
                rItemSet.Put(rInfo);
            }
        }

        if (rDxf.nFontId >= 0)
        {
            const FontData& rFontInfo = aFontData[rDxf.nFontId];
            if (rFontInfo.bBold)
            {
                SvxWeightItem aWeight(WEIGHT_BOLD, ATTR_FONT_WEIGHT);
                rItemSet.Put(aWeight);
                rItemSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CJK_FONT_WEIGHT));
                rItemSet.Put(SvxWeightItem(WEIGHT_BOLD, ATTR_CTL_FONT_WEIGHT));
            }
            SvxColorItem aColorItem(rFontInfo.maColor, rFontInfo.maComplexColor, ATTR_FONT_COLOR);
            rItemSet.Put(aColorItem);
        }

        aDxfPatterns[i] = std::move(pPattern);
    }

    // Step 6: Assemble table styles
    size_t nTableStyles = std::size(aTableStyles);
    for (size_t i = 0; i < nTableStyles; ++i)
    {
        const ::TableStyle& rStyleInfo = aTableStyles[i];
        auto pStyle = std::make_unique<ScTableStyle>(OUString::createFromAscii(rStyleInfo.pName),
                                                     std::optional<OUString>());
        pStyle->SetOOXMLDefault(true);

        for (size_t j = 0; j < rStyleInfo.nElements; ++j)
        {
            const ::TableStyleElement& rElem
                = aTableStyleElements[rStyleInfo.pTableStyleElementIds[j]];
            if (aDxfPatterns[rElem.nDxfId])
            {
                auto pPatternCopy = std::make_unique<ScPatternAttr>(*aDxfPatterns[rElem.nDxfId]);
                pStyle->SetPattern(rElem.eElement, std::move(pPatternCopy));
            }
        }

        pTableStyles->AddTableStyle(std::move(pStyle));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
