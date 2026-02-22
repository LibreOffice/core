/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <autoform.hxx>
#include <svx/svxtableitems.hxx>

#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <svx/algitem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/rotmodit.hxx>
#include <svx/strings.hrc>
#include <editeng/adjustitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star;

ScAutoFormatDataField::ScAutoFormatDataField()
{
    // need to set default instances for base class AutoFormatBase here
    // due to resource defines (e.g. ATTR_FONT) which are not available
    // in svx and different in the different usages of derivations
    m_aFont = std::make_unique<SvxFontItem>(ATTR_FONT);
    m_aHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_FONT_HEIGHT);
    m_aWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_FONT_WEIGHT);
    m_aPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_FONT_POSTURE);
    m_aCJKFont = std::make_unique<SvxFontItem>(ATTR_CJK_FONT);
    m_aCJKHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_CJK_FONT_HEIGHT);
    m_aCJKWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT);
    m_aCJKPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_CJK_FONT_POSTURE);
    m_aCTLFont = std::make_unique<SvxFontItem>(ATTR_CTL_FONT);
    m_aCTLHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_CTL_FONT_HEIGHT);
    m_aCTLWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT);
    m_aCTLPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_CTL_FONT_POSTURE);
    m_aUnderline = std::make_unique<SvxUnderlineItem>(LINESTYLE_NONE, ATTR_FONT_UNDERLINE);
    m_aOverline = std::make_unique<SvxOverlineItem>(LINESTYLE_NONE, ATTR_FONT_OVERLINE);
    m_aCrossedOut = std::make_unique<SvxCrossedOutItem>(STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT);
    m_aContour = std::make_unique<SvxContourItem>(false, ATTR_FONT_CONTOUR);
    m_aShadowed = std::make_unique<SvxShadowedItem>(false, ATTR_FONT_SHADOWED);
    m_aColor = std::make_unique<SvxColorItem>(ATTR_FONT_COLOR);
    m_aBox = std::make_unique<SvxBoxItem>(ATTR_BORDER);
    m_aTLBR = std::make_unique<SvxLineItem>(ATTR_BORDER_TLBR);
    m_aBLTR = std::make_unique<SvxLineItem>(ATTR_BORDER_BLTR);
    m_aBackground = std::make_unique<SvxBrushItem>(ATTR_BACKGROUND);
    m_aAdjust = std::make_unique<SvxAdjustItem>(SvxAdjust::Left, 0);
    m_aHorJustify
        = std::make_unique<SvxHorJustifyItem>(SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY);
    m_aVerJustify
        = std::make_unique<SvxVerJustifyItem>(SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY);
    m_aStacked = std::make_unique<ScVerticalStackCell>(ATTR_STACKED);
    m_aMargin = std::make_unique<SvxMarginItem>(ATTR_MARGIN);
    m_aLinebreak = std::make_unique<ScLineBreakCell>(ATTR_LINEBREAK);
    m_aRotateAngle = std::make_unique<ScRotateValueItem>(0_deg100);
    m_aRotateMode = std::make_unique<SvxRotateModeItem>(SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE);
}

ScAutoFormatDataField::ScAutoFormatDataField(const ScAutoFormatDataField& rCopy)
    : SvxAutoFormatDataField(rCopy)
{
}

ScAutoFormatDataField::ScAutoFormatDataField(const SvxAutoFormatDataField& rCopy)
    : SvxAutoFormatDataField(rCopy)
{
}

ScAutoFormatDataField::~ScAutoFormatDataField() {}

ScAutoFormatData::ScAutoFormatData()
{
    for (size_t n = 0; n < ELEMENT_COUNT; n++)
        mpDataField[n] = std::make_unique<ScAutoFormatDataField>();
}

ScAutoFormatData::ScAutoFormatData(const ScAutoFormatData& rData)
    : SvxAutoFormatData(rData)
{
    for (size_t n = 0; n < ELEMENT_COUNT; n++)
        mpDataField[n].reset(new ScAutoFormatDataField(*rData.GetField(n)));
}

ScAutoFormatData::ScAutoFormatData(const SvxAutoFormatData& rData)
    : SvxAutoFormatData(rData)
{
    for (size_t n = 0; n < ELEMENT_COUNT; n++)
        mpDataField[n].reset(new ScAutoFormatDataField(*rData.GetField(n)));
}

ScAutoFormatDataField* ScAutoFormatData::GetField(size_t nIndex)
{
    if (nIndex < ELEMENT_COUNT)
        return mpDataField[nIndex].get();

    SAL_WARN("sc", "ScAutoFormatData::GetField - index out of bounds: " << nIndex);
    return nullptr;
}

const ScAutoFormatDataField* ScAutoFormatData::GetField(size_t nIndex) const
{
    if (nIndex < ELEMENT_COUNT)
        return mpDataField[nIndex].get();

    SAL_WARN("sc", "ScAutoFormatData::GetField - index out of bounds: " << nIndex);
    return nullptr;
}

bool ScAutoFormatData::SetField(size_t nIndex, const SvxAutoFormatDataField& aField)
{
    if (nIndex < ELEMENT_COUNT)
    {
        mpDataField[nIndex] = std::make_unique<ScAutoFormatDataField>(aField);
        return true;
    }

    SAL_WARN("sc", "ScAutoFormatData::SetField - index out of bounds: " << nIndex);
    return false;
}

const SfxPoolItem* ScAutoFormatData::GetItem(sal_uInt8 nIndex, sal_uInt16 nWhich) const
{
    const ScAutoFormatDataField& rField = *GetField(nIndex);
    switch (nWhich)
    {
        case ATTR_FONT:
            return &rField.GetFont();
        case ATTR_FONT_HEIGHT:
            return &rField.GetHeight();
        case ATTR_FONT_WEIGHT:
            return &rField.GetWeight();
        case ATTR_FONT_POSTURE:
            return &rField.GetPosture();
        case ATTR_CJK_FONT:
            return &rField.GetCJKFont();
        case ATTR_CJK_FONT_HEIGHT:
            return &rField.GetCJKHeight();
        case ATTR_CJK_FONT_WEIGHT:
            return &rField.GetCJKWeight();
        case ATTR_CJK_FONT_POSTURE:
            return &rField.GetCJKPosture();
        case ATTR_CTL_FONT:
            return &rField.GetCTLFont();
        case ATTR_CTL_FONT_HEIGHT:
            return &rField.GetCTLHeight();
        case ATTR_CTL_FONT_WEIGHT:
            return &rField.GetCTLWeight();
        case ATTR_CTL_FONT_POSTURE:
            return &rField.GetCTLPosture();
        case ATTR_FONT_UNDERLINE:
            return &rField.GetUnderline();
        case ATTR_FONT_OVERLINE:
            return &rField.GetOverline();
        case ATTR_FONT_CROSSEDOUT:
            return &rField.GetCrossedOut();
        case ATTR_FONT_CONTOUR:
            return &rField.GetContour();
        case ATTR_FONT_SHADOWED:
            return &rField.GetShadowed();
        case ATTR_FONT_COLOR:
            return &rField.GetColor();
        case ATTR_BORDER:
            return &rField.GetBox();
        case ATTR_BORDER_TLBR:
            return &rField.GetTLBR();
        case ATTR_BORDER_BLTR:
            return &rField.GetBLTR();
        case ATTR_BACKGROUND:
            return &rField.GetBackground();
        case ATTR_HOR_JUSTIFY:
            return &rField.GetHorJustify();
        case ATTR_VER_JUSTIFY:
            return &rField.GetVerJustify();
        case ATTR_STACKED:
            return &rField.GetStacked();
        case ATTR_MARGIN:
            return &rField.GetMargin();
        case ATTR_LINEBREAK:
            return &rField.GetLinebreak();
        case ATTR_ROTATE_VALUE:
            return &rField.GetRotateAngle();
        case ATTR_ROTATE_MODE:
            return &rField.GetRotateMode();
    }
    return nullptr;
}

void ScAutoFormatData::PutItem(size_t nIndex, const SfxPoolItem& rItem)
{
    ScAutoFormatDataField* rField = GetField(nIndex);
    switch (rItem.Which())
    {
        case ATTR_FONT:
            rField->SetFont(rItem.StaticWhichCast(ATTR_FONT));
            break;
        case ATTR_FONT_HEIGHT:
            rField->SetHeight(rItem.StaticWhichCast(ATTR_FONT_HEIGHT));
            break;
        case ATTR_FONT_WEIGHT:
            rField->SetWeight(rItem.StaticWhichCast(ATTR_FONT_WEIGHT));
            break;
        case ATTR_FONT_POSTURE:
            rField->SetPosture(rItem.StaticWhichCast(ATTR_FONT_POSTURE));
            break;
        case ATTR_CJK_FONT:
            rField->SetCJKFont(rItem.StaticWhichCast(ATTR_CJK_FONT));
            break;
        case ATTR_CJK_FONT_HEIGHT:
            rField->SetCJKHeight(rItem.StaticWhichCast(ATTR_CJK_FONT_HEIGHT));
            break;
        case ATTR_CJK_FONT_WEIGHT:
            rField->SetCJKWeight(rItem.StaticWhichCast(ATTR_CJK_FONT_WEIGHT));
            break;
        case ATTR_CJK_FONT_POSTURE:
            rField->SetCJKPosture(rItem.StaticWhichCast(ATTR_CJK_FONT_POSTURE));
            break;
        case ATTR_CTL_FONT:
            rField->SetCTLFont(rItem.StaticWhichCast(ATTR_CTL_FONT));
            break;
        case ATTR_CTL_FONT_HEIGHT:
            rField->SetCTLHeight(rItem.StaticWhichCast(ATTR_CTL_FONT_HEIGHT));
            break;
        case ATTR_CTL_FONT_WEIGHT:
            rField->SetCTLWeight(rItem.StaticWhichCast(ATTR_CTL_FONT_WEIGHT));
            break;
        case ATTR_CTL_FONT_POSTURE:
            rField->SetCTLPosture(rItem.StaticWhichCast(ATTR_CTL_FONT_POSTURE));
            break;
        case ATTR_FONT_UNDERLINE:
            rField->SetUnderline(rItem.StaticWhichCast(ATTR_FONT_UNDERLINE));
            break;
        case ATTR_FONT_OVERLINE:
            rField->SetOverline(rItem.StaticWhichCast(ATTR_FONT_OVERLINE));
            break;
        case ATTR_FONT_CROSSEDOUT:
            rField->SetCrossedOut(rItem.StaticWhichCast(ATTR_FONT_CROSSEDOUT));
            break;
        case ATTR_FONT_CONTOUR:
            rField->SetContour(rItem.StaticWhichCast(ATTR_FONT_CONTOUR));
            break;
        case ATTR_FONT_SHADOWED:
            rField->SetShadowed(rItem.StaticWhichCast(ATTR_FONT_SHADOWED));
            break;
        case ATTR_FONT_COLOR:
            rField->SetColor(rItem.StaticWhichCast(ATTR_FONT_COLOR));
            break;
        case ATTR_BORDER:
            rField->SetBox(rItem.StaticWhichCast(ATTR_BORDER));
            break;
        case ATTR_BORDER_TLBR:
            rField->SetTLBR(rItem.StaticWhichCast(ATTR_BORDER_TLBR));
            break;
        case ATTR_BORDER_BLTR:
            rField->SetBLTR(rItem.StaticWhichCast(ATTR_BORDER_BLTR));
            break;
        case ATTR_BACKGROUND:
            rField->SetBackground(rItem.StaticWhichCast(ATTR_BACKGROUND));
            break;
        case ATTR_HOR_JUSTIFY:
            rField->SetHorJustify(rItem.StaticWhichCast(ATTR_HOR_JUSTIFY));
            break;
        case ATTR_VER_JUSTIFY:
            rField->SetVerJustify(rItem.StaticWhichCast(ATTR_VER_JUSTIFY));
            break;
        case ATTR_STACKED:
            rField->SetStacked(rItem.StaticWhichCast(ATTR_STACKED));
            break;
        case ATTR_MARGIN:
            rField->SetMargin(rItem.StaticWhichCast(ATTR_MARGIN));
            break;
        case ATTR_LINEBREAK:
            rField->SetLinebreak(rItem.StaticWhichCast(ATTR_LINEBREAK));
            break;
        case ATTR_ROTATE_VALUE:
            rField->SetRotateAngle(rItem.StaticWhichCast(ATTR_ROTATE_VALUE));
            break;
        case ATTR_ROTATE_MODE:
            rField->SetRotateMode(rItem.StaticWhichCast(ATTR_ROTATE_MODE));
            break;
    }
}

void ScAutoFormatData::FillToItemSet(size_t nIndex, SfxItemSet& rItemSet) const
{
    SvxAutoFormatData::FillToItemSet(nIndex, rItemSet);

    ScAutoFormatDataField rField = *GetField(nIndex), rDefault;
    if (IsFrame())
    {
        if (rField.GetBox() != rDefault.GetBox())
        {
            SvxBoxItem aNewBox(rField.GetBox());
            SvxBoxItem aOldBox(rItemSet.Get(ATTR_BORDER));

            if (!aNewBox.GetTop())
                aNewBox.SetLine(aOldBox.GetTop(), SvxBoxItemLine::TOP);
            if (!aNewBox.GetBottom())
                aNewBox.SetLine(aOldBox.GetBottom(), SvxBoxItemLine::BOTTOM);
            if (!aNewBox.GetLeft())
                aNewBox.SetLine(aOldBox.GetLeft(), SvxBoxItemLine::LEFT);
            if (!aNewBox.GetRight())
                aNewBox.SetLine(aOldBox.GetRight(), SvxBoxItemLine::RIGHT);
            if (!aNewBox.GetDistance(SvxBoxItemLine::RIGHT))
                aNewBox.SetAllDistances(aOldBox.GetDistance(SvxBoxItemLine::RIGHT));

            rItemSet.Put(aNewBox);
        }
    }
}

struct ScAutoFormat::Impl
{
    std::vector<std::unique_ptr<ScAutoFormatData>> maAutoFormats;
};

ScAutoFormat::ScAutoFormat()
    : mpImpl(new Impl)
{
    SvxAutoFormat::Load(false);
}

ScAutoFormat::~ScAutoFormat() = default;

const ScAutoFormatData* ScAutoFormat::GetData(size_t nIndex) const
{
    return &*mpImpl->maAutoFormats[nIndex];
}

ScAutoFormatData* ScAutoFormat::GetData(size_t nIndex) { return &*mpImpl->maAutoFormats[nIndex]; }

bool ScAutoFormat::InsertAutoFormat(SvxAutoFormatData* pFormat)
{
    if (!pFormat)
    {
        SAL_WARN("sc", "Attempting to insert null format into ScAutoFormat");
        return false;
    }
    auto pNew = std::make_unique<ScAutoFormatData>(*pFormat);
    mpImpl->maAutoFormats.emplace_back(std::move(pNew));
    return true;
}

ScAutoFormatData* ScAutoFormat::ReleaseAutoFormat(const OUString& rName)
{
    ScAutoFormatData* pFormat = FindAutoFormat(rName);

    std::unique_ptr<ScAutoFormatData> pRet;
    auto iter = std::find_if(mpImpl->maAutoFormats.begin(), mpImpl->maAutoFormats.end(),
                             [&pFormat](const std::unique_ptr<ScAutoFormatData>& rpFormat)
                             { return rpFormat->GetName() == pFormat->GetName(); });
    if (iter != mpImpl->maAutoFormats.end())
    {
        pRet = std::move(*iter);
        mpImpl->maAutoFormats.erase(iter);
    }
    ResetParent(rName);
    Save();
    return pRet.release();
}

ScAutoFormatData* ScAutoFormat::FindAutoFormat(const OUString& rName) const
{
    for (const auto& rFormat : mpImpl->maAutoFormats)
    {
        if (rFormat->GetName() == rName)
            return rFormat.get();
    }
    return nullptr;
}

const ScAutoFormatData* ScAutoFormat::GetResolvedStyle(const SvxAutoFormatData* rData) const
{
    auto pData = dynamic_cast<const ScAutoFormatData*>(rData);
    if (!pData)
        return nullptr;

    if (pData->GetParent().getLength())
    {
        if (auto pParent = GetResolvedStyle(FindAutoFormat(pData->GetParent())))
        {
            auto pNew = new ScAutoFormatData(*pData);
            pNew->MergeStyle(*pParent);
            return pNew;
        }
    }

    return pData;
}

size_t ScAutoFormat::size() const { return mpImpl->maAutoFormats.size(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
