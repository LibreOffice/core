/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyle.hxx>

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

bool ScTableStyle::HasFontAttrSet(ScPatternAttr* pPattern)
{
    // TODO: maybe different pPatterns can have different font attributes, and
    // now we only check if any font attribute is set on a pattern.
    // e.g.: mpFirstRowStripePattern only has ATTR_FONT_WEIGHT set and we will return that one,
    // but mpTablePattern also can have ATTR_FONT_COLOR set (need to merge them and return that
    // one for custom styles, but now it is enough for the ooxml default styles).
    for (sal_Int16 nWhich = ATTR_FONT; nWhich <= ATTR_FONT_RELIEF; nWhich++)
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
                if (pTLine || pLLine)
                {
                    std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                      : nullptr);
                    if (!pNewBoxItem)
                        pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                    if (pTLine)
                        pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                    if (pLLine)
                        pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

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
                    if (pTLine || pLLine)
                    {
                        std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                          : nullptr);
                        if (!pNewBoxItem)
                            pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                        if (pTLine)
                            pNewBoxItem->SetLine(pTLine, SvxBoxItemLine::TOP);
                        if (pLLine)
                            pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

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
                    if (pBLine || pLLine)
                    {
                        std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem ? pPoolItem->Clone()
                                                                          : nullptr);
                        if (!pNewBoxItem)
                            pNewBoxItem = std::make_unique<SvxBoxItem>(ATTR_BORDER);
                        if (pBLine)
                            pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                        if (pLLine)
                            pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

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
                        if (pLLine || pBLine)
                        {
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem->Clone());
                            if (pBLine)
                                pNewBoxItem->SetLine(pBLine, SvxBoxItemLine::BOTTOM);
                            if (pLLine)
                                pNewBoxItem->SetLine(pLLine, SvxBoxItemLine::LEFT);

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
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem->Clone());
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
                            std::unique_ptr<SvxBoxItem> pNewBoxItem(pPoolItem->Clone());
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
            else if (pBoxItem && nCol == aRange.aEnd.Col())
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
            else if (pBoxItem && nRow == aRange.aEnd.Row())
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

void ScTableStyle::SetTablePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpTablePattern = std::move(pPattern);
}

void ScTableStyle::SetFirstColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstColumnStripePattern = std::move(pPattern);
}

void ScTableStyle::SetSecondColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpSecondColumnStripePattern = std::move(pPattern);
}

void ScTableStyle::SetFirstRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstRowStripePattern = std::move(pPattern);
}

void ScTableStyle::SetSecondRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpSecondRowStripePattern = std::move(pPattern);
}

void ScTableStyle::SetLastColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpLastColumnPattern = std::move(pPattern);
}

void ScTableStyle::SetFirstColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstColumnPattern = std::move(pPattern);
}

void ScTableStyle::SetHeaderRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpHeaderRowPattern = std::move(pPattern);
}

void ScTableStyle::SetTotalRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpTotalRowPattern = std::move(pPattern);
}

void ScTableStyle::SetFirstHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpFirstHeaderCellPattern = std::move(pPattern);
}

void ScTableStyle::SetLastHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    mpLastHeaderCellPattern = std::move(pPattern);
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

sal_Int32 ScTableStyle::GetFirstRowStripeSize() const { return mnFirstRowStripeSize; }

sal_Int32 ScTableStyle::GetSecondRowStripeSize() const { return mnSecondRowStripeSize; }

sal_Int32 ScTableStyle::GetFirstColumnStripeSize() const { return mnFirstColStripeSize; }

sal_Int32 ScTableStyle::GetSecondColumnStripeSize() const { return mnSecondColStripeSize; }

ScTableStyles::ScTableStyles() {}

void ScTableStyles::AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle)
{
    maTableStyles.insert({ pTableStyle->GetName(), std::move(pTableStyle) });
}

void ScTableStyles::DeleteTableStyle(const OUString& rName) { maTableStyles.erase(rName); }

const ScTableStyle* ScTableStyles::GetTableStyle(const OUString& rName) const
{
    if (maTableStyles.find(rName) == maTableStyles.end())
        return nullptr;

    return maTableStyles.find(rName)->second.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
