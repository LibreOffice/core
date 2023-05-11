/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <ThemeColorChanger.hxx>

#include <sal/config.h>

#include <docmodel/uno/UnoComplexColor.hxx>
#include <docmodel/theme/Theme.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>

#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <address.hxx>
#include <dociter.hxx>

namespace sc
{
ThemeColorChanger::ThemeColorChanger(ScDocShell& rDocShell)
    : m_rDocShell(rDocShell)
{
}

ThemeColorChanger::~ThemeColorChanger() = default;

namespace
{
void changeCellItems(SfxItemSet& rItemSet, model::ColorSet const& rColorSet)
{
    const SfxPoolItem* pItem = nullptr;

    if (rItemSet.HasItem(ATTR_FONT_COLOR, &pItem))
    {
        auto const* pColorItem = static_cast<const SvxColorItem*>(pItem);
        model::ComplexColor const& rComplexColor = pColorItem->getComplexColor();
        if (rComplexColor.meType == model::ColorType::Scheme)
        {
            auto eThemeType = rComplexColor.meSchemeType;
            if (eThemeType != model::ThemeColorType::Unknown)
            {
                Color aColor = rColorSet.getColor(eThemeType);
                aColor = rComplexColor.applyTransformations(aColor);

                SvxColorItem aColorItem(*pColorItem);
                aColorItem.setColor(aColor);
                rItemSet.Put(aColorItem);
            }
        }
    }
    if (rItemSet.HasItem(ATTR_BACKGROUND, &pItem))
    {
        auto const* pBrushItem = static_cast<const SvxBrushItem*>(pItem);
        model::ComplexColor const& rComplexColor = pBrushItem->getComplexColor();
        if (rComplexColor.meType == model::ColorType::Scheme)
        {
            auto eThemeType = rComplexColor.meSchemeType;
            if (eThemeType != model::ThemeColorType::Unknown)
            {
                Color aColor = rColorSet.getColor(eThemeType);
                aColor = rComplexColor.applyTransformations(aColor);

                SvxBrushItem aNewBrushItem(*pBrushItem);
                aNewBrushItem.SetColor(aColor);
                rItemSet.Put(aNewBrushItem);
            }
        }
    }
}
} // end anonymous ns

void ThemeColorChanger::apply(std::shared_ptr<model::ColorSet> const& pColorSet)
{
    auto& rDocument = m_rDocShell.GetDocument();
    ScStyleSheetPool* pPool = rDocument.GetStyleSheetPool();
    ScStyleSheet* pStyle;

    // Paragraph style color change
    pStyle = static_cast<ScStyleSheet*>(pPool->First(SfxStyleFamily::Para));
    while (pStyle)
    {
        auto& rItemSet = pStyle->GetItemSet();
        changeCellItems(rItemSet, *pColorSet);
        pStyle = static_cast<ScStyleSheet*>(pPool->Next());
    }

    // Change Cell / Text attributes
    for (SCTAB nTab = 0; nTab < rDocument.GetTableCount(); nTab++)
    {
        ScDocAttrIterator aAttributeIterator(rDocument, nTab, 0, 0, rDocument.MaxCol(),
                                             rDocument.MaxRow());
        SCCOL nCol = 0;
        SCROW nRow1 = 0;
        SCROW nRow2 = 0;

        while (const ScPatternAttr* pPattern = aAttributeIterator.GetNext(nCol, nRow1, nRow2))
        {
            auto* pNonConstPattern = const_cast<ScPatternAttr*>(pPattern);
            auto& rItemSet = pNonConstPattern->GetItemSet();
            changeCellItems(rItemSet, *pColorSet);
        }
    }
}

} // end sw namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
