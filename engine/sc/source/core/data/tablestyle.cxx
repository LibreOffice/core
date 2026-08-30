/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyle.hxx>
#include <sc.hrc>
#include <scresid.hxx>
#include <strings.hrc>
#include <o3tl/string_view.hxx>
#include <algorithm>
#include <o3tl/enumrange.hxx>
#include <sfx2/kit/helper.hxx>
#include <COKit/COKit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/borderline.hxx>
#include <svl/itempool.hxx>
#include <docmodel/color/ComplexColor.hxx>
#include <docmodel/theme/ColorSet.hxx>
#include <patattr.hxx>

ScTableStyle::ScTableStyle(const OUString& rName, const std::optional<OUString>& rUIName)
    : mnFirstRowStripeSize(1)
    , mnSecondRowStripeSize(1)
    , mnFirstColStripeSize(1)
    , mnSecondColStripeSize(1)
    , maStyleName(rName)
    , maUIName(rUIName)
    , mbIsOOXMLDefault(false)
{
    maHasFontAttr.fill(false);
}

namespace
{
// The only font attributes a Table Style can contribute.
constexpr sal_uInt16 aTableStyleFontWhich[] = {
    ATTR_FONT,         ATTR_CJK_FONT,         ATTR_CTL_FONT,
    ATTR_FONT_HEIGHT,  ATTR_CJK_FONT_HEIGHT,  ATTR_CTL_FONT_HEIGHT,
    ATTR_FONT_WEIGHT,  ATTR_CJK_FONT_WEIGHT,  ATTR_CTL_FONT_WEIGHT,
    ATTR_FONT_POSTURE, ATTR_CJK_FONT_POSTURE, ATTR_CTL_FONT_POSTURE,
    ATTR_FONT_COLOR,   ATTR_FONT_UNDERLINE,   ATTR_FONT_CROSSEDOUT,
};

constexpr size_t nTableStyleElementCount = static_cast<size_t>(ScTableStyleElement::LAST) + 1;

bool lcl_hasFontAttrSet(const ScPatternAttr* pPattern)
{
    for (sal_uInt16 nWhich : aTableStyleFontWhich)
    {
        if (pPattern->GetItemSet().GetItemState(nWhich) == SfxItemState::SET)
            return true;
    }
    return false;
}

// The elements that contribute a font to one cell, highest precedence first, plus a key
// identifying the combination so the merged result can be cached per combination.
struct ScTableStyleFontElements
{
    const ScPatternAttr* aPatterns[nTableStyleElementCount] = {};
    size_t nCount = 0;
    sal_uInt32 nKey = 0;

    template <class PatternArray, class FlagArray>
    void Add(const PatternArray& rPatterns, const FlagArray& rHasFontAttr,
             ScTableStyleElement eElement)
    {
        if (!rHasFontAttr[eElement])
            return;

        aPatterns[nCount++] = rPatterns[eElement].get();
        nKey |= sal_uInt32(1) << static_cast<int>(eElement);
    }
};
}

const SfxItemSet* ScTableStyle::GetFontItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                               SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    const bool bHeaderRow = rDBData.HasHeader() && nRow == aRange.aStart.Row();
    const bool bTotalRow = rDBData.HasTotals() && nRow == aRange.aEnd.Row();

    // Collect the elements the cell falls in, highest precedence win.
    ScTableStyleFontElements aElements;
    if (bTotalRow && pParam->mbLastColumn && nCol == aRange.aEnd.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::LastTotalCell);

    if (bTotalRow && pParam->mbFirstColumn && nCol == aRange.aStart.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::FirstTotalCell);

    if (bHeaderRow && pParam->mbLastColumn && nCol == aRange.aEnd.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::LastHeaderCell);

    if (bHeaderRow && pParam->mbFirstColumn && nCol == aRange.aStart.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::FirstHeaderCell);

    if (bTotalRow)
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::TotalRow);

    if (bHeaderRow)
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::HeaderRow);

    if (pParam->mbFirstColumn && nCol == aRange.aStart.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::FirstColumn);

    if (pParam->mbLastColumn && nCol == aRange.aEnd.Col())
        aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::LastColumn);

    if (!bTotalRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            if ((nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize)
                aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::FirstRowStripe);
            else
                aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::SecondRowStripe);
        }

        if (pParam->mbColumnStripes && nRowIndex >= 0)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            if ((nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize)
                aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::FirstColumnStripe);
            else
                aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::SecondColumnStripe);
        }
    }

    aElements.Add(maPatterns, maHasFontAttr, ScTableStyleElement::WholeTable);

    if (!aElements.nCount)
        return nullptr;

    // A single element needs no merge, so hand out its own set.
    if (aElements.nCount == 1)
        return &aElements.aPatterns[0]->GetItemSet();

    // Merged sets are cached: the result depends on the element combination alone, and
    // building one per cell would allocate through the whole paint.
    std::unique_ptr<SfxItemSet>& rpMerged = maMergedFontSets[aElements.nKey];
    if (!rpMerged)
    {
        // The top element brings its whole set, the ones below it only fill in the font
        // attributes still missing.
        rpMerged = std::make_unique<SfxItemSet>(aElements.aPatterns[0]->GetItemSet());
        for (size_t i = 1; i < aElements.nCount; ++i)
        {
            const SfxItemSet& rSet = aElements.aPatterns[i]->GetItemSet();
            for (sal_uInt16 nWhich : aTableStyleFontWhich)
            {
                const SfxPoolItem* pItem = nullptr;
                if (rpMerged->GetItemState(nWhich) != SfxItemState::SET
                    && rSet.GetItemState(nWhich, true, &pItem) == SfxItemState::SET)
                    rpMerged->Put(*pItem);
            }
        }
    }

    return rpMerged.get();
}

const SvxBrushItem* ScTableStyle::GetFillItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                              SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasTotal && pParam->mbLastColumn && nRow == aRange.aEnd.Row() && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::LastTotalCell, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasTotal && pParam->mbFirstColumn && nRow == aRange.aEnd.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::FirstTotalCell, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && pParam->mbLastColumn && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::LastHeaderCell, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && pParam->mbFirstColumn && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::FirstHeaderCell, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && nRow == aRange.aStart.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::HeaderRow, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasTotal && nRow == aRange.aEnd.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::TotalRow, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbFirstColumn && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::FirstColumn, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbLastColumn && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetElementItem(ScTableStyleElement::LastColumn, ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            const SvxBrushItem* pPoolItem
                = GetElementItem(bFirstRowStripe ? ScTableStyleElement::FirstRowStripe
                                                 : ScTableStyleElement::SecondRowStripe,
                                 ATTR_BACKGROUND);
            if (pPoolItem)
                return pPoolItem;
        }

        if (pParam->mbColumnStripes && nRowIndex >= 0)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            const SvxBrushItem* pPoolItem
                = GetElementItem(bFirstColStripe ? ScTableStyleElement::FirstColumnStripe
                                                 : ScTableStyleElement::SecondColumnStripe,
                                 ATTR_BACKGROUND);
            if (pPoolItem)
                return pPoolItem;
        }
    }

    return GetElementItem(ScTableStyleElement::WholeTable, ATTR_BACKGROUND);
}

sal_uInt32 ScTableStyle::GetBoxCacheKey(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                        SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    // Parity only counts where the banding is actually consulted, so cells that never reach
    // it share an entry instead of splitting one per stripe.
    const bool bBanded = nRowIndex >= 0;
    bool bFirstRowStripe = false;
    if (pParam->mbRowStripes && bBanded)
    {
        sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
        bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
    }

    bool bFirstColStripe = false;
    if (pParam->mbColumnStripes && bBanded)
    {
        SCCOL nRelativeCol = nCol - aRange.aStart.Col();
        sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
        bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
    }

    const bool aKeyBits[] = { nCol == aRange.aStart.Col(),
                              nCol == aRange.aEnd.Col(),
                              nRow == aRange.aStart.Row(),
                              nRow == aRange.aEnd.Row(),
                              rDBData.HasHeader(),
                              rDBData.HasTotals(),
                              pParam->mbFirstColumn,
                              pParam->mbLastColumn,
                              pParam->mbRowStripes,
                              pParam->mbColumnStripes,
                              bBanded,
                              bFirstRowStripe,
                              bFirstColStripe };

    sal_uInt32 nKey = 0;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aKeyBits); ++i)
        nKey |= sal_uInt32(aKeyBits[i]) << i;
    return nKey;
}

const SvxBoxItem* ScTableStyle::GetBoxItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                           SCROW nRowIndex) const
{
    const sal_uInt32 nKey = GetBoxCacheKey(rDBData, nCol, nRow, nRowIndex);
    auto aItr = maBoxItems.find(nKey);
    if (aItr == maBoxItems.end())
        aItr = maBoxItems.emplace(nKey, BuildBoxItem(rDBData, nCol, nRow, nRowIndex)).first;

    return aItr->second.get();
}

// Anything this reads about the cell's position or the table's options has to go into
// GetBoxCacheKey too, or GetBoxItem hands a cached border to a cell that should not share it.
std::unique_ptr<SvxBoxItem> ScTableStyle::BuildBoxItem(const ScDBData& rDBData, SCCOL nCol,
                                                       SCROW nRow, SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasTotal && pParam->mbLastColumn && nRow == aRange.aEnd.Row() && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetElementItem(ScTableStyleElement::LastTotalCell, ATTR_BORDER);
        if (const SvxBoxItem* pBoxItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER))
        {
            const ::editeng::SvxBorderLine* pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
            const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
            const ::editeng::SvxBorderLine* pLLine
                = nCol == aRange.aStart.Col() ? pBoxItem->GetLine(SvxBoxItemLine::LEFT) : nullptr;
            if (pBLine || pRLine || pLLine)
            {
                std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone() : nullptr);
                if (!pNewBoxItem)
                    pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                if (pBLine)
                    pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                if (pRLine)
                    pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);
                if (pLLine)
                    pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

                return pNewBoxItem;
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasTotal && pParam->mbFirstColumn && nRow == aRange.aEnd.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetElementItem(ScTableStyleElement::FirstTotalCell, ATTR_BORDER);
        if (const SvxBoxItem* pBoxItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER))
        {
            const ::editeng::SvxBorderLine* pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
            const ::editeng::SvxBorderLine* pLLine = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
            const ::editeng::SvxBorderLine* pRLine
                = nCol == aRange.aEnd.Col() ? pBoxItem->GetLine(SvxBoxItemLine::RIGHT) : nullptr;
            if (pBLine || pLLine || pRLine)
            {
                std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone() : nullptr);
                if (!pNewBoxItem)
                    pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                if (pBLine)
                    pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                if (pLLine)
                    pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
                if (pRLine)
                    pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                return pNewBoxItem;
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasHeader && pParam->mbLastColumn && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetElementItem(ScTableStyleElement::LastHeaderCell, ATTR_BORDER);
        if (const SvxBoxItem* pBoxItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER))
        {
            const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
            const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
            const ::editeng::SvxBorderLine* pLLine
                = nCol == aRange.aStart.Col() ? pBoxItem->GetLine(SvxBoxItemLine::LEFT) : nullptr;
            if (pTLine || pRLine || pLLine)
            {
                std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone() : nullptr);
                if (!pNewBoxItem)
                    pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                if (pTLine)
                    pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                if (pRLine)
                    pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);
                if (pLLine)
                    pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

                return pNewBoxItem;
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasHeader && pParam->mbFirstColumn && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetElementItem(ScTableStyleElement::FirstHeaderCell, ATTR_BORDER);
        if (const SvxBoxItem* pBoxItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER))
        {
            const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
            const ::editeng::SvxBorderLine* pLLine = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
            const ::editeng::SvxBorderLine* pRLine
                = nCol == aRange.aEnd.Col() ? pBoxItem->GetLine(SvxBoxItemLine::RIGHT) : nullptr;
            if (pTLine || pLLine || pRLine)
            {
                std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone() : nullptr);
                if (!pNewBoxItem)
                    pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                if (pTLine)
                    pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                if (pLLine)
                    pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
                if (pRLine)
                    pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                return pNewBoxItem;
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasHeader && nRow == aRange.aStart.Row())
    {
        const SvxBoxItem* pPoolItem = GetElementItem(ScTableStyleElement::HeaderRow, ATTR_BORDER);
        const SvxBoxItem* pBoxItem = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER);
        const SvxBoxInfoItem* pBoxInfoItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER_INNER);
        if (pBoxItem || pBoxInfoItem)
        {
            if (pBoxItem && nCol == aRange.aStart.Col())
            {
                const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
                const ::editeng::SvxBorderLine* pLLine = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
                const ::editeng::SvxBorderLine* pRLine
                    = nCol == aRange.aEnd.Col() ? pBoxItem->GetLine(SvxBoxItemLine::RIGHT)
                                                : nullptr;
                if (pTLine || pLLine || pRLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pTLine)
                        pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                    if (pLLine)
                        pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
                    if (pRLine)
                        pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                    return pNewBoxItem;
                }
            }
            else if (pBoxItem && nCol == aRange.aEnd.Col())
            {
                const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
                const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
                if (pTLine || pRLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pTLine)
                        pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                    if (pRLine)
                        pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                    return pNewBoxItem;
                }
            }
            else
            {
                const ::editeng::SvxBorderLine* pTLine = nullptr;
                if (pBoxItem)
                    pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);

                const ::editeng::SvxBorderLine* pVLine = nullptr;
                if (pBoxInfoItem)
                    pVLine = pBoxInfoItem->GetVert();

                if (pTLine || pVLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pTLine)
                        pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                    if (pVLine)
                    {
                        pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::LEFT);
                        pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::RIGHT);
                    }

                    return pNewBoxItem;
                }
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasTotal && nRow == aRange.aEnd.Row())
    {
        const SvxBoxItem* pPoolItem = GetElementItem(ScTableStyleElement::TotalRow, ATTR_BORDER);
        const SvxBoxItem* pBoxItem = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER);
        const SvxBoxInfoItem* pBoxInfoItem
            = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER_INNER);
        if (pBoxItem || pBoxInfoItem)
        {
            if (pBoxItem && nCol == aRange.aStart.Col())
            {
                const ::editeng::SvxBorderLine* pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                const ::editeng::SvxBorderLine* pLLine = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
                const ::editeng::SvxBorderLine* pRLine
                    = nCol == aRange.aEnd.Col() ? pBoxItem->GetLine(SvxBoxItemLine::RIGHT)
                                                : nullptr;
                if (pBLine || pLLine || pRLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pBLine)
                        pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                    if (pLLine)
                        pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
                    if (pRLine)
                        pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                    return pNewBoxItem;
                }
            }
            else if (pBoxItem && nCol == aRange.aEnd.Col())
            {
                const ::editeng::SvxBorderLine* pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
                if (pBLine || pRLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pBLine)
                        pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                    if (pRLine)
                        pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                    return pNewBoxItem;
                }
            }
            else
            {
                const ::editeng::SvxBorderLine* pBLine = nullptr;
                if (pBoxItem)
                    pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);

                const ::editeng::SvxBorderLine* pVLine = nullptr;
                if (pBoxInfoItem)
                    pVLine = pBoxInfoItem->GetVert();

                if (pBLine || pVLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pBLine)
                        pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                    if (pVLine)
                    {
                        pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::LEFT);
                        pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::RIGHT);
                    }

                    return pNewBoxItem;
                }
            }
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (pParam->mbFirstColumn && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem = GetElementItem(ScTableStyleElement::FirstColumn, ATTR_BORDER);
        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (pParam->mbLastColumn && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem = GetElementItem(ScTableStyleElement::LastColumn, ATTR_BORDER);
        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;

            const SvxBoxItem* pPoolItem = nullptr;
            if (bFirstRowStripe)
                pPoolItem = GetElementItem(ScTableStyleElement::FirstRowStripe, ATTR_BORDER);
            else if (!bFirstRowStripe)
                pPoolItem = GetElementItem(ScTableStyleElement::SecondRowStripe, ATTR_BORDER);

            if (pPoolItem)
            {
                const SvxBoxItem* pBoxItem
                    = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER);
                const SvxBoxInfoItem* pBoxInfoItem
                    = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER_INNER);
                if (pBoxItem || pBoxInfoItem)
                {
                    if (pBoxItem && nCol == aRange.aStart.Col())
                    {
                        const ::editeng::SvxBorderLine* pLLine
                            = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
                        const ::editeng::SvxBorderLine* pBLine = nullptr;
                        if (aRange.aEnd.Row() == nRow)
                            pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                        const ::editeng::SvxBorderLine* pRLine
                            = nCol == aRange.aEnd.Col() ? pBoxItem->GetLine(SvxBoxItemLine::RIGHT)
                                                        : nullptr;
                        if (pLLine || pBLine || pRLine)
                        {
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                              : nullptr);
                            if (!pNewBoxItem)
                                pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                            if (pBLine)
                                pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                            if (pLLine)
                                pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
                            if (pRLine)
                                pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                            return pNewBoxItem;
                        }
                    }
                    else if (pBoxItem && nCol == aRange.aEnd.Col())
                    {
                        const ::editeng::SvxBorderLine* pRLine
                            = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
                        const ::editeng::SvxBorderLine* pBLine = nullptr;
                        if (aRange.aEnd.Row() == nRow)
                            pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                        if (pRLine || pBLine)
                        {
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                              : nullptr);
                            if (!pNewBoxItem)
                                pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                            if (pBLine)
                                pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                            if (pRLine)
                                pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);

                            return pNewBoxItem;
                        }
                    }
                    else
                    {
                        const ::editeng::SvxBorderLine* pBLine = nullptr;
                        if (pBoxItem && aRange.aEnd.Row() == nRow)
                            pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);

                        const ::editeng::SvxBorderLine* pVLine = nullptr;
                        if (pBoxInfoItem)
                            pVLine = pBoxInfoItem->GetVert();

                        if (pBLine || pVLine)
                        {
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                              : nullptr);
                            if (!pNewBoxItem)
                                pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                            if (pBLine)
                                pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                            if (pVLine)
                            {
                                pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::LEFT);
                                pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::RIGHT);
                            }

                            return pNewBoxItem;
                        }
                    }
                }
            }

            if (pPoolItem)
                return std::make_unique<SvxBoxItem>(*pPoolItem);
        }

        if (pParam->mbColumnStripes && nRowIndex >= 0)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (!bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetElementItem(ScTableStyleElement::SecondColumnStripe, ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }

            if (bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetElementItem(ScTableStyleElement::FirstColumnStripe, ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }
        }
    }

    const SvxBoxItem* pBoxItem = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER);
    const SvxBoxInfoItem* pBoxInfoItem
        = GetElementItem(ScTableStyleElement::WholeTable, ATTR_BORDER_INNER);

    if (pBoxItem || pBoxInfoItem)
    {
        std::unique_ptr<SvxBoxItem> pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
        // Start/End col borders
        if (pBoxItem && nCol == aRange.aStart.Col())
        {
            const ::editeng::SvxBorderLine* pLLine = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
            if (pLLine)
            {
                pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);
            }
        }
        if (pBoxItem && nCol == aRange.aEnd.Col())
        {
            const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
            if (pRLine)
            {
                pNewBoxItem->SetLine(pRLine, SvxBoxItemLine::RIGHT);
            }
        }
        // Start/End row borders
        if (pBoxItem && nRow == aRange.aStart.Row())
        {
            const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
            if (pTLine)
            {
                pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
            }
        }
        if (pBoxItem && nRow == aRange.aEnd.Row())
        {
            const ::editeng::SvxBorderLine* pBLine = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
            if (pBLine)
            {
                pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
            }
        }
        // Inner borders
        if (pBoxInfoItem)
        {
            const ::editeng::SvxBorderLine* pHLine = pBoxInfoItem->GetHori();
            if (pHLine)
            {
                pNewBoxItem->SetLine(pHLine, SvxBoxItemLine::TOP);
                pNewBoxItem->SetLine(pHLine, SvxBoxItemLine::BOTTOM);
            }

            const ::editeng::SvxBorderLine* pVLine = pBoxInfoItem->GetVert();
            if (pVLine)
            {
                pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::LEFT);
                pNewBoxItem->SetLine(pVLine, SvxBoxItemLine::RIGHT);
            }
        }

        return pNewBoxItem;
    }

    return nullptr;
}

namespace
{
// The style's font item to bake for nWhich, or nullptr to keep the cell's own value - the same
// rule as the render, see ScPatternAttr::CanApplyTableItemToCell.
const SfxPoolItem* lcl_fontItemToBake(const SfxItemSet& rCellSet, const SfxItemSet& rStyleSet,
                                      sal_uInt16 nWhich)
{
    const SfxPoolItem* pStyleItem = nullptr;
    if (rStyleSet.GetItemState(nWhich, false, &pStyleItem) != SfxItemState::SET)
        return nullptr;

    return ScPatternAttr::CanApplyTableItemToCell(rCellSet, nWhich) ? pStyleItem : nullptr;
}
}

void ScTableStyle::BakeInto(ScDocument& rDoc, const ScDBData& rDBData) const
{
    ScRange aRange;
    rDBData.GetArea(aRange);
    const SCTAB nTab = aRange.aStart.Tab();
    const SCROW nFirstRow = aRange.aStart.Row();
    // Header row = -HasHeader(), first data row = 0. On an unfiltered table (the norm) this
    // equals fillinfo's non-filtered stripe index exactly; an active filter would re-number
    // visible stripes on screen, but baking per absolute row keeps the whole range consistent
    // once the filter is cleared.
    const SCROW nHeaderOffset = static_cast<SCROW>(rDBData.HasHeader());
    auto& rHelper = rDoc.getCellAttributeHelper();

    // Column-major with vertical run-coalescing: consecutive rows that bake to the same items
    // are applied as one merge (ApplyPatternArea only overrides its set items), so a plain or
    // column-striped column costs a couple of calls instead of one per cell. Banded rows
    // alternate and so degrade to one merge per cell - still cheaper than per-item apply.
    for (SCCOL nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); ++nCol)
    {
        std::unique_ptr<ScPatternAttr> pRunPattern;
        SCROW nRunStart = nFirstRow;
        auto flushRun = [&](SCROW nRunEnd) {
            if (pRunPattern)
                rDoc.ApplyPatternAreaTab(nCol, nRunStart, nCol, nRunEnd, nTab, *pRunPattern);
            pRunPattern.reset();
        };

        for (SCROW nRow = aRange.aStart.Row(); nRow <= aRange.aEnd.Row(); ++nRow)
        {
            const SCROW nRowIndex = nRow - nFirstRow - nHeaderOffset;
            const SfxItemSet& rCellSet = rDoc.GetPattern(nCol, nRow, nTab)->GetItemSet();

            ScPatternAttr aBake(rHelper);
            bool bAny = false;

            // Fill: style shows only where the cell sets no background directly or via its
            // cell style (bSrchInParent), exactly as fillinfo decides.
            if (!rCellSet.GetItemIfSet(ATTR_BACKGROUND))
            {
                if (const SvxBrushItem* pFill = GetFillItem(rDBData, nCol, nRow, nRowIndex))
                {
                    aBake.ItemSetPut(*pFill);
                    bAny = true;
                }
            }

            // Border: same, except an "empty" explicit border (no edges) yields to the style.
            const SvxBoxItem* pCellBox = rCellSet.GetItemIfSet(ATTR_BORDER);
            const bool bCellBoxNonEmpty = pCellBox
                                          && (pCellBox->GetTop() || pCellBox->GetBottom()
                                              || pCellBox->GetLeft() || pCellBox->GetRight());
            if (!bCellBoxNonEmpty)
            {
                if (const SvxBoxItem* pBox = GetBoxItem(rDBData, nCol, nRow, nRowIndex))
                {
                    aBake.ItemSetPut(*pBox);
                    bAny = true;
                }
            }

            // Font: table style beats the cell style (direct-only test); see lcl_fontItemToBake.
            if (const SfxItemSet* pFontSet = GetFontItemSet(rDBData, nCol, nRow, nRowIndex))
            {
                for (sal_uInt16 nWhich : aTableStyleFontWhich)
                {
                    if (const SfxPoolItem* pFont = lcl_fontItemToBake(rCellSet, *pFontSet, nWhich))
                    {
                        aBake.ItemSetPut(*pFont);
                        bAny = true;
                    }
                }
            }

            if (!bAny)
            {
                flushRun(nRow - 1);
                continue;
            }
            if (pRunPattern && *pRunPattern == aBake)
                continue; // extend the open run
            flushRun(nRow - 1);
            pRunPattern = std::make_unique<ScPatternAttr>(aBake);
            nRunStart = nRow;
        }
        flushRun(aRange.aEnd.Row());
    }
}

void ScTableStyle::SetRowStripeSize(sal_Int32 nFirstRowStripeSize, sal_Int32 nSecondRowStripeSize)
{
    if (nFirstRowStripeSize >= 1)
        mnFirstRowStripeSize = nFirstRowStripeSize;

    if (nSecondRowStripeSize >= 1)
        mnSecondRowStripeSize = nSecondRowStripeSize;
}

void ScTableStyle::SetColStripeSize(sal_Int32 nFirstColStripeSize, sal_Int32 nSecondColStripeSize)
{
    if (nFirstColStripeSize >= 1)
        mnFirstColStripeSize = nFirstColStripeSize;

    if (nSecondColStripeSize >= 1)
        mnSecondColStripeSize = nSecondColStripeSize;
}

void ScTableStyle::SetOOXMLDefault(bool bDefault) { mbIsOOXMLDefault = bDefault; }

bool ScTableStyle::IsOOXMLDefault() const { return mbIsOOXMLDefault; }

const OUString& ScTableStyle::GetName() const { return maStyleName; }

const OUString& ScTableStyle::GetUIName() const
{
    if (maUIName)
        return *maUIName;

    return maStyleName;
}

void ScTableStyle::SetPattern(ScTableStyleElement eTableStyleElement,
                              std::unique_ptr<ScPatternAttr> pPattern)
{
    maMergedFontSets.clear();
    maBoxItems.clear();

    maPatterns[eTableStyleElement] = std::move(pPattern);
    const ScPatternAttr* pNew = maPatterns[eTableStyleElement].get();
    maHasFontAttr[eTableStyleElement] = pNew && lcl_hasFontAttrSet(pNew);
}

std::map<ScTableStyleElement, const ScPatternAttr*> ScTableStyle::GetSetPatterns() const
{
    std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns;
    for (ScTableStyleElement eElement : o3tl::enumrange<ScTableStyleElement>())
    {
        if (maPatterns[eElement])
            aPatterns.emplace(eElement, maPatterns[eElement].get());
    }

    return aPatterns;
}

ScTableStyles::ScTableStyles(ScDocument* pDoc)
    : mpDoc(pDoc)
    , maDefaultStyleName(u"TableStyleMedium2"_ustr)
{
}

void ScTableStyles::AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle)
{
    // insert() keeps an existing entry with the same name. The create dialog
    // picks a programmatic name that is not yet in use, and OOXML import keeps
    // the first-seen definition, so this never has to overwrite.
    OUString aName = pTableStyle->GetName();
    maTableStyles.insert({ std::move(aName), std::move(pTableStyle) });
    InvalidateBindings();
}

void ScTableStyles::ClearOOXMLDefaultStyles()
{
    std::erase_if(maTableStyles,
                  [](const auto& rEntry) { return rEntry.second->IsOOXMLDefault(); });
    InvalidateBindings();
}

void ScTableStyles::InvalidateBindings()
{
    // The style set just changed, so the gathered display names are stale and
    // the next request for a free name gathers them again.
    mbUINamesInUseValid = false;

    if (!mpDoc)
        return;
    // The style list belongs to the document, so every view of it is told, not just
    // the view the change came from.
    mpDoc->InvalidateSlotInAllViews(SID_TABLE_STYLES);
}

namespace
{
/// Update themed colors in a single pattern's items against a new ColorSet
void updatePatternThemedColors(ScPatternAttr& rPattern, const model::ColorSet& rColorSet)
{
    SfxItemSet& rItemSet = rPattern.GetItemSetWritable();

    // Update background fill color
    if (const SvxBrushItem* pBrush = rItemSet.GetItemIfSet(ATTR_BACKGROUND))
    {
        const model::ComplexColor& rCC = pBrush->getComplexColor();
        if (rCC.getThemeColorType() != model::ThemeColorType::Unknown)
        {
            Color aNewColor = rColorSet.resolveOOXMLColor(rCC);
            SvxBrushItem aNewBrush(aNewColor, ATTR_BACKGROUND);
            aNewBrush.setComplexColor(rCC);
            rItemSet.Put(aNewBrush);
        }
    }

    // Update font color
    if (const SvxColorItem* pColorItem = rItemSet.GetItemIfSet(ATTR_FONT_COLOR))
    {
        const model::ComplexColor& rCC = pColorItem->getComplexColor();
        if (rCC.getThemeColorType() != model::ThemeColorType::Unknown)
        {
            Color aNewColor = rColorSet.resolveOOXMLColor(rCC);
            SvxColorItem aNewColorItem(aNewColor, rCC, ATTR_FONT_COLOR);
            rItemSet.Put(aNewColorItem);
        }
    }

    // Update border line colors (outer borders)
    if (const SvxBoxItem* pBox = rItemSet.GetItemIfSet(ATTR_BORDER))
    {
        SvxBoxItem aNewBox(*pBox);
        bool bChanged = false;
        for (auto eLine : { SvxBoxItemLine::TOP, SvxBoxItemLine::BOTTOM, SvxBoxItemLine::LEFT,
                            SvxBoxItemLine::RIGHT })
        {
            if (const editeng::SvxBorderLine* pLine = aNewBox.GetLine(eLine))
            {
                const model::ComplexColor& rCC = pLine->getComplexColor();
                if (rCC.getThemeColorType() != model::ThemeColorType::Unknown)
                {
                    Color aNewColor = rColorSet.resolveOOXMLColor(rCC);
                    editeng::SvxBorderLine aNewLine(*pLine);
                    aNewLine.SetColor(aNewColor);
                    aNewBox.SetLine(&aNewLine, eLine);
                    bChanged = true;
                }
            }
        }
        if (bChanged)
            rItemSet.Put(aNewBox);
    }

    // Update inner border line colors (vertical/horizontal)
    if (const SvxBoxInfoItem* pBoxInfo = rItemSet.GetItemIfSet(ATTR_BORDER_INNER))
    {
        SvxBoxInfoItem aNewBoxInfo(*pBoxInfo);
        bool bChanged = false;
        if (const editeng::SvxBorderLine* pLine = aNewBoxInfo.GetVert())
        {
            const model::ComplexColor& rCC = pLine->getComplexColor();
            if (rCC.getThemeColorType() != model::ThemeColorType::Unknown)
            {
                Color aNewColor = rColorSet.resolveOOXMLColor(rCC);
                editeng::SvxBorderLine aNewLine(*pLine);
                aNewLine.SetColor(aNewColor);
                aNewBoxInfo.SetLine(&aNewLine, SvxBoxInfoItemLine::VERT);
                bChanged = true;
            }
        }
        if (const editeng::SvxBorderLine* pLine = aNewBoxInfo.GetHori())
        {
            const model::ComplexColor& rCC = pLine->getComplexColor();
            if (rCC.getThemeColorType() != model::ThemeColorType::Unknown)
            {
                Color aNewColor = rColorSet.resolveOOXMLColor(rCC);
                editeng::SvxBorderLine aNewLine(*pLine);
                aNewLine.SetColor(aNewColor);
                aNewBoxInfo.SetLine(&aNewLine, SvxBoxInfoItemLine::HORI);
                bChanged = true;
            }
        }
        if (bChanged)
            rItemSet.Put(aNewBoxInfo);
    }
}

} // anonymous namespace

void ScTableStyle::UpdateThemedColors(const model::ColorSet& rColorSet)
{
    if (mbIsOOXMLDefault)
        return; // defaults are fully regenerated, not updated in-place

    maMergedFontSets.clear();
    maBoxItems.clear();

    for (const std::unique_ptr<ScPatternAttr>& rpPattern : maPatterns)
    {
        if (rpPattern)
            updatePatternThemedColors(*rpPattern, rColorSet);
    }
}

void ScTableStyles::UpdateCustomStyleThemedColors(const model::ColorSet& rColorSet)
{
    for (auto & [ rName, pStyle ] : maTableStyles)
    {
        if (pStyle && !pStyle->IsOOXMLDefault())
            pStyle->UpdateThemedColors(rColorSet);
    }
}

const ScTableStyle* ScTableStyles::GetTableStyle(const OUString& rName) const
{
    if (maTableStyles.find(rName) == maTableStyles.end())
        return nullptr;

    return maTableStyles.find(rName)->second.get();
}

namespace
{
std::string_view tableStyleElementName(ScTableStyleElement eElement)
{
    switch (eElement)
    {
        case ScTableStyleElement::WholeTable:
            return "WholeTable";
        case ScTableStyleElement::FirstColumnStripe:
            return "FirstColumnStripe";
        case ScTableStyleElement::SecondColumnStripe:
            return "SecondColumnStripe";
        case ScTableStyleElement::FirstRowStripe:
            return "FirstRowStripe";
        case ScTableStyleElement::SecondRowStripe:
            return "SecondRowStripe";
        case ScTableStyleElement::LastColumn:
            return "LastColumn";
        case ScTableStyleElement::FirstColumn:
            return "FirstColumn";
        case ScTableStyleElement::HeaderRow:
            return "HeaderRow";
        case ScTableStyleElement::TotalRow:
            return "TotalRow";
        case ScTableStyleElement::FirstHeaderCell:
            return "FirstHeaderCell";
        case ScTableStyleElement::LastHeaderCell:
            return "LastHeaderCell";
        case ScTableStyleElement::FirstTotalCell:
            return "FirstTotalCell";
        case ScTableStyleElement::LastTotalCell:
            return "LastTotalCell";
    }
    return {};
}
}

void ScTableStyles::generateJSON(tools::JsonWriter& rWriter) const
{
    auto aStylesArray = rWriter.startArray("TableStyles");

    for (auto const & [ rName, pStyle ] : maTableStyles)
    {
        auto aStyleStruct = rWriter.startStruct();
        rWriter.put("Name", pStyle->GetName().toUtf8());
        rWriter.put("UIName", pStyle->GetUIName().toUtf8());

        auto aElementsArray = rWriter.startArray("Elements");
        for (auto const & [ eElement, pPattern ] : pStyle->GetSetPatterns())
        {
            const SvxBrushItem* pBrush = pPattern->GetItemSet().GetItemIfSet(ATTR_BACKGROUND);
            if (!pBrush)
                continue;

            auto aElementStruct = rWriter.startStruct();
            rWriter.put("Type", tableStyleElementName(eElement));
            rWriter.put("FillColor", pBrush->GetColor().AsRGBHexString().toUtf8());
        }
    }
}

OUString ScTableStyles::GetUnusedCustomStyleName() const
{
    for (sal_Int32 nCandidate = 1;; ++nCandidate)
    {
        OUString aName = "TableStyleCustom" + OUString::number(nCandidate);
        if (!GetTableStyle(aName))
            return aName;
    }
}

OUString ScTableStyles::GetUnusedUIName(const OUString& rBaseName) const
{
    // Whether a name is free is a question of membership, not of order, so the names
    // in use are gathered rather than sorted, and they are kept until the style set
    // changes so repeated asking costs one lookup each.
    if (!mbUINamesInUseValid)
    {
        maUINamesInUse.clear();
        maUINamesInUse.reserve(maTableStyles.size());
        for (const auto & [ rStyleName, pStyle ] : maTableStyles)
            maUINamesInUse.insert(pStyle->GetUIName());
        mbUINamesInUseValid = true;
    }

    if (!maUINamesInUse.contains(rBaseName))
        return rBaseName;

    for (sal_Int32 nCandidate = 2;; ++nCandidate)
    {
        // The count goes in first so the name goes in last, and a name carrying a
        // per-cent token of its own reaches the result as the user wrote it.
        OUString aName = ScResId(STR_TABLE_STYLE_NAME_NUMBERED)
                             .replaceFirst("%2", OUString::number(nCandidate))
                             .replaceFirst("%1", rBaseName);
        if (!maUINamesInUse.contains(aName))
            return aName;
    }
}

OUString ScTableStyles::DuplicateTableStyle(const OUString& rSourceName, const OUString& rNewUIName)
{
    const ScTableStyle* pSource = GetTableStyle(rSourceName);
    if (!pSource)
        return OUString();

    const OUString aNewName = GetUnusedCustomStyleName();
    auto pCopy = std::make_unique<ScTableStyle>(
        aNewName, std::optional<OUString>(GetUnusedUIName(rNewUIName)));
    for (const auto & [ eElement, pPattern ] : pSource->GetSetPatterns())
        pCopy->SetPattern(eElement, std::make_unique<ScPatternAttr>(*pPattern));
    AddTableStyle(std::move(pCopy));
    return aNewName;
}

ScTableStyleFamily ScGetTableStyleFamily(std::u16string_view rName, bool bIsBuiltin)
{
    // A style has a built-in family only when it is one of the app's generated
    // built-ins; any other style is Custom, whatever its name happens to be. A
    // built-in's name is generated by us (TableStyleLight/Medium/Dark<n>) and is
    // therefore trusted, so its family is taken from the name prefix.
    if (!bIsBuiltin)
        return ScTableStyleFamily::Custom;
    if (o3tl::starts_with(rName, u"TableStyleLight"))
        return ScTableStyleFamily::Light;
    if (o3tl::starts_with(rName, u"TableStyleMedium"))
        return ScTableStyleFamily::Medium;
    if (o3tl::starts_with(rName, u"TableStyleDark"))
        return ScTableStyleFamily::Dark;
    return ScTableStyleFamily::Custom;
}

namespace
{
// Sort key for a table style: the family (Light, then Medium, then Dark, then
// anything else) followed by the trailing number, so styles read "Light 1,
// Light 2, ... Light 10" rather than the lexical "Light 1, Light 10, Light 2".
std::pair<int, sal_Int32> tableStyleSortKey(const ScTableStyle& rStyle)
{
    const OUString& rName = rStyle.GetName();
    const int nFamily = static_cast<int>(ScGetTableStyleFamily(rName, rStyle.IsOOXMLDefault()));

    sal_Int32 nDigitStart = rName.getLength();
    while (nDigitStart > 0 && rName[nDigitStart - 1] >= '0' && rName[nDigitStart - 1] <= '9')
        --nDigitStart;
    const sal_Int32 nNumber
        = nDigitStart < rName.getLength() ? o3tl::toInt32(rName.subView(nDigitStart)) : 0;

    return { nFamily, nNumber };
}
}

std::vector<const ScTableStyle*> ScTableStyles::GetSortedTableStyles() const
{
    std::vector<const ScTableStyle*> aSorted;
    aSorted.reserve(maTableStyles.size());
    for (auto const & [ rName, pStyle ] : maTableStyles)
        aSorted.push_back(pStyle.get());

    std::sort(aSorted.begin(), aSorted.end(),
              [](const ScTableStyle* pLeft, const ScTableStyle* pRight) {
                  const auto aLeft = tableStyleSortKey(*pLeft);
                  const auto aRight = tableStyleSortKey(*pRight);
                  if (aLeft != aRight)
                      return aLeft < aRight;
                  return pLeft->GetName() < pRight->GetName();
              });
    return aSorted;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
