/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tablestyle.hxx>
#include <sc.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/lokhelper.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <editeng/colritem.hxx>
#include <editeng/borderline.hxx>
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

bool ScTableStyle::HasFontAttrSet(const ScPatternAttr* pPattern)
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
    if (bHasHeader && lookupPattern(ScTableStyleElement::LastHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::LastHeaderCell)))
            return &lookupPattern(ScTableStyleElement::LastHeaderCell)->GetItemSet();
    }

    if (bHasHeader && lookupPattern(ScTableStyleElement::FirstHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::FirstHeaderCell)))
            return &lookupPattern(ScTableStyleElement::FirstHeaderCell)->GetItemSet();
    }

    if (bHasTotal && lookupPattern(ScTableStyleElement::TotalRow) && nRow == aRange.aEnd.Row())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::TotalRow)))
            return &lookupPattern(ScTableStyleElement::TotalRow)->GetItemSet();
    }

    if (bHasHeader && lookupPattern(ScTableStyleElement::HeaderRow) && nRow == aRange.aStart.Row())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::HeaderRow)))
            return &lookupPattern(ScTableStyleElement::HeaderRow)->GetItemSet();
    }

    if (pParam->mbFirstColumn && lookupPattern(ScTableStyleElement::FirstColumn) && nCol == aRange.aStart.Col())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::FirstColumn)))
            return &lookupPattern(ScTableStyleElement::FirstColumn)->GetItemSet();
    }

    if (pParam->mbLastColumn && lookupPattern(ScTableStyleElement::LastColumn) && nCol == aRange.aEnd.Col())
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::LastColumn)))
            return &lookupPattern(ScTableStyleElement::LastColumn)->GetItemSet();
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            if (lookupPattern(ScTableStyleElement::SecondRowStripe) && !bFirstRowStripe)
            {
                if (HasFontAttrSet(lookupPattern(ScTableStyleElement::SecondRowStripe)))
                    return &lookupPattern(ScTableStyleElement::SecondRowStripe)->GetItemSet();
            }

            if (lookupPattern(ScTableStyleElement::FirstRowStripe) && bFirstRowStripe)
            {
                if (HasFontAttrSet(lookupPattern(ScTableStyleElement::FirstRowStripe)))
                    return &lookupPattern(ScTableStyleElement::FirstRowStripe)->GetItemSet();
            }
        }

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (lookupPattern(ScTableStyleElement::SecondColumnStripe) && !bFirstColStripe)
            {
                if (HasFontAttrSet(lookupPattern(ScTableStyleElement::SecondColumnStripe)))
                    return &lookupPattern(ScTableStyleElement::SecondColumnStripe)->GetItemSet();
            }

            if (lookupPattern(ScTableStyleElement::FirstColumnStripe) && bFirstColStripe)
            {
                if (HasFontAttrSet(lookupPattern(ScTableStyleElement::FirstColumnStripe)))
                    return &lookupPattern(ScTableStyleElement::FirstColumnStripe)->GetItemSet();
            }
        }
    }

    if (lookupPattern(ScTableStyleElement::WholeTable))
    {
        if (HasFontAttrSet(lookupPattern(ScTableStyleElement::WholeTable)))
            return &lookupPattern(ScTableStyleElement::WholeTable)->GetItemSet();
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
    if (bHasHeader && lookupPattern(ScTableStyleElement::LastHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::LastHeaderCell), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && lookupPattern(ScTableStyleElement::FirstHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstHeaderCell), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasHeader && lookupPattern(ScTableStyleElement::HeaderRow) && nRow == aRange.aStart.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::HeaderRow), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (bHasTotal && lookupPattern(ScTableStyleElement::TotalRow) && nRow == aRange.aEnd.Row())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::TotalRow), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbFirstColumn && lookupPattern(ScTableStyleElement::FirstColumn) && nCol == aRange.aStart.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstColumn), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (pParam->mbLastColumn && lookupPattern(ScTableStyleElement::LastColumn) && nCol == aRange.aEnd.Col())
    {
        const SvxBrushItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::LastColumn), ATTR_BACKGROUND);
        if (pPoolItem)
            return pPoolItem;
    }

    if (!bHasTotal || aRange.aEnd.Row() != nRow)
    {
        if (pParam->mbRowStripes && nRowIndex >= 0)
        {
            sal_Int32 nTotalRowStripPattern = mnFirstRowStripeSize + mnSecondRowStripeSize;
            bool bFirstRowStripe = (nRowIndex % nTotalRowStripPattern) < mnFirstRowStripeSize;
            if (lookupPattern(ScTableStyleElement::SecondRowStripe) && !bFirstRowStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::SecondRowStripe), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }

            if (lookupPattern(ScTableStyleElement::FirstRowStripe) && bFirstRowStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstRowStripe), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }
        }

        if (pParam->mbColumnStripes)
        {
            SCCOL nRelativeCol = nCol - aRange.aStart.Col();
            sal_Int32 nTotalColStripePattern = mnFirstColStripeSize + mnSecondColStripeSize;
            bool bFirstColStripe = (nRelativeCol % nTotalColStripePattern) < mnFirstColStripeSize;
            if (lookupPattern(ScTableStyleElement::SecondColumnStripe) && !bFirstColStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::SecondColumnStripe), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }

            if (lookupPattern(ScTableStyleElement::FirstColumnStripe) && bFirstColStripe)
            {
                const SvxBrushItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstColumnStripe), ATTR_BACKGROUND);
                if (pPoolItem)
                    return pPoolItem;
            }
        }
    }

    if (lookupPattern(ScTableStyleElement::WholeTable))
    {
        const SvxBrushItem* pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BACKGROUND);
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
    if (bHasHeader && lookupPattern(ScTableStyleElement::LastHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::LastHeaderCell), ATTR_BORDER);
        if (lookupPattern(ScTableStyleElement::WholeTable))
        {
            if (const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER))
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

    if (bHasHeader && lookupPattern(ScTableStyleElement::FirstHeaderCell) && nRow == aRange.aStart.Row()
        && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstHeaderCell), ATTR_BORDER);
        if (lookupPattern(ScTableStyleElement::WholeTable))
        {
            if (const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER))
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

    if (bHasHeader && lookupPattern(ScTableStyleElement::HeaderRow) && nRow == aRange.aStart.Row())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::HeaderRow), ATTR_BORDER);
        if (lookupPattern(ScTableStyleElement::WholeTable))
        {
            const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER);
            const SvxBoxInfoItem* pBoxInfoItem
                = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER_INNER);
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

    if (bHasTotal && lookupPattern(ScTableStyleElement::TotalRow) && nRow == aRange.aEnd.Row())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::TotalRow), ATTR_BORDER);
        if (lookupPattern(ScTableStyleElement::WholeTable))
        {
            const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER);
            const SvxBoxInfoItem* pBoxInfoItem
                = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER_INNER);
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

    if (pParam->mbFirstColumn && lookupPattern(ScTableStyleElement::FirstColumn) && nCol == aRange.aStart.Col())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstColumn), ATTR_BORDER);
        if (pPoolItem)
            return std::make_unique<SvxBoxItem>(*pPoolItem);
    }

    if (pParam->mbLastColumn && lookupPattern(ScTableStyleElement::LastColumn) && nCol == aRange.aEnd.Col())
    {
        const SvxBoxItem* pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::LastColumn), ATTR_BORDER);
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
            if (lookupPattern(ScTableStyleElement::FirstRowStripe) && bFirstRowStripe)
                pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstRowStripe), ATTR_BORDER);
            else if (lookupPattern(ScTableStyleElement::SecondRowStripe) && !bFirstRowStripe)
                pPoolItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::SecondRowStripe), ATTR_BORDER);

            if (pPoolItem && lookupPattern(ScTableStyleElement::WholeTable))
            {
                const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER);
                const SvxBoxInfoItem* pBoxInfoItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER_INNER);
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
            if (lookupPattern(ScTableStyleElement::SecondColumnStripe) && !bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::SecondColumnStripe), ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }

            if (lookupPattern(ScTableStyleElement::FirstColumnStripe) && bFirstColStripe)
            {
                const SvxBoxItem* pPoolItem
                    = GetItemFromPattern(lookupPattern(ScTableStyleElement::FirstColumnStripe), ATTR_BORDER);
                if (pPoolItem)
                    return std::make_unique<SvxBoxItem>(*pPoolItem);
            }
        }
    }

    if (lookupPattern(ScTableStyleElement::WholeTable))
    {
        const SvxBoxItem* pBoxItem = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER);
        const SvxBoxInfoItem* pBoxInfoItem
            = GetItemFromPattern(lookupPattern(ScTableStyleElement::WholeTable), ATTR_BORDER_INNER);

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

const ScPatternAttr* ScTableStyle::lookupPattern(ScTableStyleElement eElement) const
{
    auto it = maPatterns.find(eElement);
    return it == maPatterns.end() ? nullptr : it->second.get();
}

void ScTableStyle::SetPattern(ScTableStyleElement eTableStyleElement,
                              std::unique_ptr<ScPatternAttr> pPattern)
{
    if (pPattern)
        maPatterns[eTableStyleElement] = std::move(pPattern);
    else
        maPatterns.erase(eTableStyleElement);
}

void ScTableStyle::SetTablePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::WholeTable, std::move(pPattern));
}

void ScTableStyle::SetFirstColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::FirstColumnStripe, std::move(pPattern));
}

void ScTableStyle::SetSecondColumnStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::SecondColumnStripe, std::move(pPattern));
}

void ScTableStyle::SetFirstRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::FirstRowStripe, std::move(pPattern));
}

void ScTableStyle::SetSecondRowStripePattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::SecondRowStripe, std::move(pPattern));
}

void ScTableStyle::SetLastColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::LastColumn, std::move(pPattern));
}

void ScTableStyle::SetFirstColumnPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::FirstColumn, std::move(pPattern));
}

void ScTableStyle::SetHeaderRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::HeaderRow, std::move(pPattern));
}

void ScTableStyle::SetTotalRowPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::TotalRow, std::move(pPattern));
}

void ScTableStyle::SetFirstHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::FirstHeaderCell, std::move(pPattern));
}

void ScTableStyle::SetLastHeaderCellPattern(std::unique_ptr<ScPatternAttr> pPattern)
{
    SetPattern(ScTableStyleElement::LastHeaderCell, std::move(pPattern));
}

std::map<ScTableStyleElement, const ScPatternAttr*> ScTableStyle::GetSetPatterns() const
{
    std::map<ScTableStyleElement, const ScPatternAttr*> aPatterns;
    for (const auto& [eElement, pPattern] : maPatterns)
    {
        if (pPattern)
            aPatterns.emplace(eElement, pPattern.get());
    }
    return aPatterns;
}

sal_Int32 ScTableStyle::GetFirstRowStripeSize() const { return mnFirstRowStripeSize; }

sal_Int32 ScTableStyle::GetSecondRowStripeSize() const { return mnSecondRowStripeSize; }

sal_Int32 ScTableStyle::GetFirstColumnStripeSize() const { return mnFirstColStripeSize; }

sal_Int32 ScTableStyle::GetSecondColumnStripeSize() const { return mnSecondColStripeSize; }

ScTableStyles::ScTableStyles(SfxBindings* pBindings)
    : mpBindings(pBindings)
{
}

void ScTableStyles::AddTableStyle(std::unique_ptr<ScTableStyle> pTableStyle)
{
    // TODO: insert() won't overwrite an existing entry with the same name.
    // When we add UI support for creating custom table styles, we should
    // either reject duplicates (with UI validation) or use insert_or_assign
    // to replace the existing style.
    maTableStyles.insert({ pTableStyle->GetName(), std::move(pTableStyle) });
    if (mpBindings)
        mpBindings->Invalidate(SID_TABLE_STYLES);
}

void ScTableStyles::DeleteTableStyle(const OUString& rName)
{
    maTableStyles.erase(rName);
    if (mpBindings)
        mpBindings->Invalidate(SID_TABLE_STYLES);
}

void ScTableStyles::ClearOOXMLDefaultStyles()
{
    std::erase_if(maTableStyles,
                  [](const auto& rEntry) { return rEntry.second->IsOOXMLDefault(); });
    if (mpBindings)
        mpBindings->Invalidate(SID_TABLE_STYLES);
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

    for (auto& [eElement, pPattern] : maPatterns)
    {
        (void)eElement;
        if (pPattern)
            updatePatternThemedColors(*pPattern, rColorSet);
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
        case ScTableStyleElement::PageFieldLabels:
            return "PageFieldLabels";
        case ScTableStyleElement::PageFieldValues:
            return "PageFieldValues";
        case ScTableStyleElement::FirstSubtotalRow:
            return "FirstSubtotalRow";
        case ScTableStyleElement::SecondSubtotalRow:
            return "SecondSubtotalRow";
        case ScTableStyleElement::ThirdSubtotalRow:
            return "ThirdSubtotalRow";
        case ScTableStyleElement::FirstSubtotalColumn:
            return "FirstSubtotalColumn";
        case ScTableStyleElement::SecondSubtotalColumn:
            return "SecondSubtotalColumn";
        case ScTableStyleElement::ThirdSubtotalColumn:
            return "ThirdSubtotalColumn";
        case ScTableStyleElement::FirstColumnSubheading:
            return "FirstColumnSubheading";
        case ScTableStyleElement::SecondColumnSubheading:
            return "SecondColumnSubheading";
        case ScTableStyleElement::ThirdColumnSubheading:
            return "ThirdColumnSubheading";
        case ScTableStyleElement::FirstRowSubheading:
            return "FirstRowSubheading";
        case ScTableStyleElement::SecondRowSubheading:
            return "SecondRowSubheading";
        case ScTableStyleElement::ThirdRowSubheading:
            return "ThirdRowSubheading";
        case ScTableStyleElement::BlankRow:
            return "BlankRow";
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
