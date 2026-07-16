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
#include <sfx2/bindings.hxx>
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
}

namespace
{
// The only font attributes a Table Style can contribute.
constexpr sal_uInt16 aTableStyleFontWhich[] = {
    ATTR_FONT,        ATTR_CJK_FONT,        ATTR_CTL_FONT,
    ATTR_FONT_HEIGHT, ATTR_CJK_FONT_HEIGHT, ATTR_CTL_FONT_HEIGHT,
    ATTR_FONT_WEIGHT, ATTR_CJK_FONT_WEIGHT, ATTR_CTL_FONT_WEIGHT,
    ATTR_FONT_COLOR,
};
}

bool ScTableStyle::HasFontAttrSet(const ScPatternAttr* pPattern)
{
    // TODO: GetFontItemSet returns the first matching element's whole item set and
    // does not merge font attributes across elements. Fine for the ooxml defaults
    // (each element sets font weight and colour together); a custom style that split
    // them across elements would need a merge.
    for (sal_uInt16 nWhich : aTableStyleFontWhich)
    {
        if (pPattern->GetItemSet().GetItemState(nWhich) == SfxItemState::SET)
            return true;
    }
    return false;
}

const SfxItemSet* ScTableStyle::GetFontItemSet(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                               SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasHeader && mpLastHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        if (HasFontAttrSet(mpLastHeaderCellPattern.get()))
            return &mpLastHeaderCellPattern->GetItemSet();
    }

    if (bHasHeader && mpFirstHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        if (HasFontAttrSet(mpFirstHeaderCellPattern.get()))
            return &mpFirstHeaderCellPattern->GetItemSet();
    }

    if (bHasTotal && mpTotalRowPattern && nRow == aRange.aEnd.Row())
    {
        if (HasFontAttrSet(mpTotalRowPattern.get()))
            return &mpTotalRowPattern->GetItemSet();
    }

    if (bHasHeader && mpHeaderRowPattern && nRow == aRange.aStart.Row())
    {
        if (HasFontAttrSet(mpHeaderRowPattern.get()))
            return &mpHeaderRowPattern->GetItemSet();
    }

    if (pParam->mbFirstColumn && mpFirstColumnPattern && nCol == aRange.aStart.Col())
    {
        if (HasFontAttrSet(mpFirstColumnPattern.get()))
            return &mpFirstColumnPattern->GetItemSet();
    }

    if (pParam->mbLastColumn && mpLastColumnPattern && nCol == aRange.aEnd.Col())
    {
        if (HasFontAttrSet(mpLastColumnPattern.get()))
            return &mpLastColumnPattern->GetItemSet();
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            if (mpSecondRowStripePattern && !bFirstRowStripe)
            {
                if (HasFontAttrSet(mpSecondRowStripePattern.get()))
                    return &mpSecondRowStripePattern->GetItemSet();
            }

            if (mpFirstRowStripePattern && bFirstRowStripe)
            {
                if (HasFontAttrSet(mpFirstRowStripePattern.get()))
                    return &mpFirstRowStripePattern->GetItemSet();
            }
        }

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (mpSecondColumnStripePattern && !bFirstColStripe)
            {
                if (HasFontAttrSet(mpSecondColumnStripePattern.get()))
                    return &mpSecondColumnStripePattern->GetItemSet();
            }

            if (mpFirstColumnStripePattern && bFirstColStripe)
            {
                if (HasFontAttrSet(mpFirstColumnStripePattern.get()))
                    return &mpFirstColumnStripePattern->GetItemSet();
            }
        }
    }

    if (mpTablePattern)
    {
        if (HasFontAttrSet(mpTablePattern.get()))
            return &mpTablePattern->GetItemSet();
    }

    return nullptr;
}

const SvxBrushItem* ScTableStyle::GetFillItem(const ScDBData& rDBData, SCCOL nCol, SCROW nRow,
                                              SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasHeader && mpLastHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpLastHeaderCellPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && mpFirstHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpFirstHeaderCellPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && mpHeaderRowPattern && nRow == aRange.aStart.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpHeaderRowPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasTotal && mpTotalRowPattern && nRow == aRange.aEnd.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpTotalRowPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbFirstColumn && mpFirstColumnPattern && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpFirstColumnPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbLastColumn && mpLastColumnPattern && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(mpLastColumnPattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            if (mpSecondRowStripePattern && !bFirstRowStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(mpSecondRowStripePattern.get(), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }

            if (mpFirstRowStripePattern && bFirstRowStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(mpFirstRowStripePattern.get(), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }
        }

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (mpSecondColumnStripePattern && !bFirstColStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(mpSecondColumnStripePattern.get(), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }

            if (mpFirstColumnStripePattern && bFirstColStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(mpFirstColumnStripePattern.get(), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }
        }
    }

    if (mpTablePattern)
    {
        const SvxBrushItem* pPoolItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    return nullptr;
}

std::unique_ptr<SvxBoxItem> ScTableStyle::GetBoxItem(const ScDBData& rDBData, SCCOL nCol,
                                                     SCROW nRow, SCROW nRowIndex) const
{
    const ScTableStyleParam* pParam = rDBData.GetTableStyleInfo();
    ScRange aRange;
    rDBData.GetArea(aRange);

    bool bHasHeader = rDBData.HasHeader();
    bool bHasTotal = rDBData.HasTotals();
    if (bHasHeader && mpLastHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetItemFromPattern(mpLastHeaderCellPattern.get(), ATTR_BORDER);
        if (mpTablePattern)
        {
            if (const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER))
            {
                const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
                const ::editeng::SvxBorderLine* pRLine = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
                const ::editeng::SvxBorderLine* pLLine
                    = nCol == aRange.aStart.Col() ? pBoxItem->GetLine(SvxBoxItemLine::LEFT)
                                                  : nullptr;
                if (pTLine || pRLine || pLLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
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
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasHeader && mpFirstHeaderCellPattern && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetItemFromPattern(mpFirstHeaderCellPattern.get(), ATTR_BORDER);
        if (mpTablePattern)
        {
            if (const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER))
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
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasHeader && mpHeaderRowPattern && nRow == aRange.aStart.Row())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(mpHeaderRowPattern.get(), ATTR_BORDER);
        if (mpTablePattern)
        {
            const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER);
            const SvxBoxInfoItem* pBoxInfoItem
                = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER_INNER);
            if (pBoxItem || pBoxInfoItem)
            {
                if (pBoxItem && nCol == aRange.aStart.Col())
                {
                    const ::editeng::SvxBorderLine* pTLine = pBoxItem->GetLine(SvxBoxItemLine::TOP);
                    const ::editeng::SvxBorderLine* pLLine
                        = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
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
                    const ::editeng::SvxBorderLine* pRLine
                        = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
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
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (bHasTotal && mpTotalRowPattern && nRow == aRange.aEnd.Row())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(mpTotalRowPattern.get(), ATTR_BORDER);
        if (mpTablePattern)
        {
            const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER);
            const SvxBoxInfoItem* pBoxInfoItem
                = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER_INNER);
            if (pBoxItem || pBoxInfoItem)
            {
                if (pBoxItem && nCol == aRange.aStart.Col())
                {
                    const ::editeng::SvxBorderLine* pBLine
                        = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                    const ::editeng::SvxBorderLine* pLLine
                        = pBoxItem->GetLine(SvxBoxItemLine::LEFT);
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
                    const ::editeng::SvxBorderLine* pBLine
                        = pBoxItem->GetLine(SvxBoxItemLine::BOTTOM);
                    const ::editeng::SvxBorderLine* pRLine
                        = pBoxItem->GetLine(SvxBoxItemLine::RIGHT);
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
        }

        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (pParam->mbFirstColumn && mpFirstColumnPattern && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(mpFirstColumnPattern.get(), ATTR_BORDER);
        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (pParam->mbLastColumn && mpLastColumnPattern && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(mpLastColumnPattern.get(), ATTR_BORDER);
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
            if (mpFirstRowStripePattern && bFirstRowStripe)
                pPoolItem = GetItemFromPattern(mpFirstRowStripePattern.get(), ATTR_BORDER);
            else if (mpSecondRowStripePattern && !bFirstRowStripe)
                pPoolItem = GetItemFromPattern(mpSecondRowStripePattern.get(), ATTR_BORDER);

            if (pPoolItem && mpTablePattern)
            {
                const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER);
                const SvxBoxInfoItem* pBoxInfoItem
                    = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER_INNER);
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

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (mpSecondColumnStripePattern && !bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetItemFromPattern(mpSecondColumnStripePattern.get(), ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }

            if (mpFirstColumnStripePattern && bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetItemFromPattern(mpFirstColumnStripePattern.get(), ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }
        }
    }

    if (mpTablePattern)
    {
        const SvxBoxItem* pBoxItem = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER);
        const SvxBoxInfoItem* pBoxInfoItem
            = GetItemFromPattern(mpTablePattern.get(), ATTR_BORDER_INNER);

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
    }

    return nullptr;
}

namespace
{
// The style's font item to bake for nWhich, or nullptr to keep the cell's own value. The cell
// wins only when it sets this attribute directly (not through a named cell style) to a
// non-default value - for fonts the Table Style outranks a cell style. Exception: a direct
// black font colour counts as unset (black is the Table Style's "no colour" marker), so the
// style colour shows. See ScPatternAttr::fillFontOnly / fillColor.
const SfxPoolItem* lcl_fontItemToBake(const SfxItemSet& rCellSet, const SfxItemSet& rStyleSet,
                                      sal_uInt16 nWhich)
{
    const SfxPoolItem* pStyleItem = nullptr;
    if (rStyleSet.GetItemState(nWhich, false, &pStyleItem) != SfxItemState::SET)
        return nullptr;

    const SfxPoolItem* pDirect = nullptr;
    if (rCellSet.GetItemState(nWhich, false, &pDirect) == SfxItemState::SET
        && *pDirect != rCellSet.GetPool()->GetUserOrPoolDefaultItem(nWhich))
    {
        const bool bBlackFontColor
            = nWhich == ATTR_FONT_COLOR
              && static_cast<const SvxColorItem*>(pDirect)->getColor() == COL_BLACK;
        if (!bBlackFontColor)
            return nullptr; // direct cell font wins - nothing to bake
    }

    return pStyleItem;
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
                if (std::unique_ptr<SvxBoxItem> pBox = GetBoxItem(rDBData, nCol, nRow, nRowIndex))
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
    switch (eTableStyleElement)
    {
        case ScTableStyleElement::WholeTable:
            mpTablePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstColumnStripe:
            mpFirstColumnStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::SecondColumnStripe:
            mpSecondColumnStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstRowStripe:
            mpFirstRowStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::SecondRowStripe:
            mpSecondRowStripePattern = std::move(pPattern);
            break;
        case ScTableStyleElement::LastColumn:
            mpLastColumnPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstColumn:
            mpFirstColumnPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::HeaderRow:
            mpHeaderRowPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::TotalRow:
            mpTotalRowPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::FirstHeaderCell:
            mpFirstHeaderCellPattern = std::move(pPattern);
            break;
        case ScTableStyleElement::LastHeaderCell:
            mpLastHeaderCellPattern = std::move(pPattern);
            break;
    }
}

std::map<ScTableStyleElement, const ScPatternAttr*> ScTableStyle::GetSetPatterns() const
{
    std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns;
    if (mpTablePattern)
    {
        aPatterns.emplace(ScTableStyleElement::WholeTable, mpTablePattern.get());
    }

    if (mpFirstColumnStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstColumnStripe, mpFirstColumnStripePattern.get());
    }

    if (mpSecondColumnStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::SecondColumnStripe,
                          mpSecondColumnStripePattern.get());
    }

    if (mpFirstRowStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstRowStripe, mpFirstRowStripePattern.get());
    }

    if (mpSecondRowStripePattern)
    {
        aPatterns.emplace(ScTableStyleElement::SecondRowStripe, mpSecondRowStripePattern.get());
    }

    if (mpLastColumnPattern)
    {
        aPatterns.emplace(ScTableStyleElement::LastColumn, mpLastColumnPattern.get());
    }

    if (mpFirstColumnPattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstColumn, mpFirstColumnPattern.get());
    }

    if (mpHeaderRowPattern)
    {
        aPatterns.emplace(ScTableStyleElement::HeaderRow, mpHeaderRowPattern.get());
    }

    if (mpTotalRowPattern)
    {
        aPatterns.emplace(ScTableStyleElement::TotalRow, mpTotalRowPattern.get());
    }

    if (mpFirstHeaderCellPattern)
    {
        aPatterns.emplace(ScTableStyleElement::FirstHeaderCell, mpFirstHeaderCellPattern.get());
    }

    if (mpLastHeaderCellPattern)
    {
        aPatterns.emplace(ScTableStyleElement::LastHeaderCell, mpLastHeaderCellPattern.get());
    }

    return aPatterns;
}

ScTableStyles::ScTableStyles(ScDocument* pDoc)
    : mpDoc(pDoc)
{
}

void ScTableStyles::AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle)
{
    // TODO: insert() won't overwrite an existing entry with the same name.
    // When we add UI support for creating custom table styles, we should
    // either reject duplicates (with UI validation) or use insert_or_assign
    // to replace the existing style.
    maTableStyles.insert({ pTableStyle->GetName(), std::move(pTableStyle) });
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
    if (!mpDoc)
        return;
    if (SfxBindings* pBindings = mpDoc->GetViewBindings())
        pBindings->Invalidate(SID_TABLE_STYLES);
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

    std::unique_ptr<ScPatternAttr>* aPatterns[]
        = { &mpTablePattern,           &mpFirstColumnStripePattern, &mpSecondColumnStripePattern,
            &mpFirstRowStripePattern,  &mpSecondRowStripePattern,   &mpLastColumnPattern,
            &mpFirstColumnPattern,     &mpHeaderRowPattern,         &mpTotalRowPattern,
            &mpFirstHeaderCellPattern, &mpLastHeaderCellPattern };

    for (auto* pPatternPtr : aPatterns)
    {
        if (*pPatternPtr)
            updatePatternThemedColors(**pPatternPtr, rColorSet);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
