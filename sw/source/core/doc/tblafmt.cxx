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

#include <svx/svxtableitems.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>
#include <doc.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <hintids.hxx>
#include <fmtornt.hxx>
#include <editsh.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <swmodule.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>

#include <editeng/adjustitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/legacyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/algitem.hxx>
#include <svx/rotmodit.hxx>
#include <unostyle.hxx>
#include <names.hxx>

#include <memory>
#include <utility>
#include <vector>

SwBoxAutoFormat* SwTableAutoFormat::s_pDefaultBoxAutoFormat = nullptr;

SwBoxAutoFormat::SwBoxAutoFormat()
{
    // need to set default instances for base class AutoFormatBase here
    // due to resource defines (e.g. RES_CHRATR_FONT) which are not available
    // in svx and different in the different usages of derivations
    m_aFont = std::make_unique<SvxFontItem>(*GetDfltAttr(RES_CHRATR_FONT));
    m_aHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_FONTSIZE);
    m_aWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_WEIGHT);
    m_aPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_POSTURE);
    m_aCJKFont = std::make_unique<SvxFontItem>(*GetDfltAttr(RES_CHRATR_CJK_FONT));
    m_aCJKHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_CJK_FONTSIZE);
    m_aCJKWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT);
    m_aCJKPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_CJK_POSTURE);
    m_aCTLFont = std::make_unique<SvxFontItem>(*GetDfltAttr(RES_CHRATR_CTL_FONT));
    m_aCTLHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_CTL_FONTSIZE);
    m_aCTLWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT);
    m_aCTLPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_CTL_POSTURE);
    m_aUnderline = std::make_unique<SvxUnderlineItem>(LINESTYLE_NONE, RES_CHRATR_UNDERLINE);
    m_aOverline = std::make_unique<SvxOverlineItem>(LINESTYLE_NONE, RES_CHRATR_OVERLINE);
    m_aCrossedOut = std::make_unique<SvxCrossedOutItem>(STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT);
    m_aContour = std::make_unique<SvxContourItem>(false, RES_CHRATR_CONTOUR);
    m_aShadowed = std::make_unique<SvxShadowedItem>(false, RES_CHRATR_SHADOWED);
    m_aColor = std::make_unique<SvxColorItem>(COL_AUTO, RES_CHRATR_COLOR);
    m_aBox = std::make_unique<SvxBoxItem>(RES_BOX);
    m_aTLBR = std::make_unique<SvxLineItem>(0);
    m_aBLTR = std::make_unique<SvxLineItem>(0);
    m_aBackground = std::make_unique<SvxBrushItem>(RES_BACKGROUND);
    m_aAdjust = std::make_unique<SvxAdjustItem>(SvxAdjust::Left, RES_PARATR_ADJUST);
    m_aHorJustify = std::make_unique<SvxHorJustifyItem>(SvxCellHorJustify::Standard, 0);
    m_aVerJustify = std::make_unique<SvxVerJustifyItem>(SvxCellVerJustify::Standard, 0);
    m_aStacked = std::make_unique<SfxBoolItem>(0);
    m_aMargin = std::make_unique<SvxMarginItem>(TypedWhichId<SvxMarginItem>(0));
    m_aLinebreak = std::make_unique<SfxBoolItem>(0);
    m_aRotateAngle = std::make_unique<SfxInt32Item>(0);
    m_aRotateMode = std::make_unique<SvxRotateModeItem>(SVX_ROTATE_MODE_STANDARD,
                                                        TypedWhichId<SvxRotateModeItem>(0));

    m_aTextOrientation
        = std::make_unique<SvxFrameDirectionItem>(SvxFrameDirection::Environment, RES_FRAMEDIR);
    m_aVerticalAlignment = std::make_unique<SwFormatVertOrient>(0, text::VertOrientation::TOP,
                                                                text::RelOrientation::FRAME);
    m_aVerticalAlignment->SetWhich(RES_VERT_ORIENT);
    SetSysLanguage(::GetAppLanguage());
    SetNumFormatLanguage(::GetAppLanguage());
}

SwBoxAutoFormat::SwBoxAutoFormat(const SwBoxAutoFormat& rNew)
    : SvxAutoFormatDataField(rNew)
    , m_aTextOrientation(rNew.m_aTextOrientation->Clone())
    , m_aVerticalAlignment(rNew.m_aVerticalAlignment->Clone())
{
    SetNumFormatString(rNew.GetNumFormatString());
    SetSysLanguage(rNew.GetSysLanguage());
    SetNumFormatLanguage(rNew.GetNumFormatLanguage());
}

SwBoxAutoFormat::SwBoxAutoFormat(const SvxAutoFormatDataField& rNew)
    : SvxAutoFormatDataField(rNew)
    , m_aTextOrientation(
          std::make_unique<SvxFrameDirectionItem>(SvxFrameDirection::Environment, RES_FRAMEDIR))
    , m_aVerticalAlignment(std::make_unique<SwFormatVertOrient>(0, text::VertOrientation::TOP,
                                                                text::RelOrientation::FRAME))
{
    UpdateAlignment();
    SetSysLanguage(::GetAppLanguage());
    SetNumFormatLanguage(::GetAppLanguage());
}

void SwBoxAutoFormat::UpdateAlignment(bool bSvxUpdate)
{
    if (bSvxUpdate)
    {
        SvxHorJustifyItem aHor(GetHorJustify());
        switch (GetAdjust().GetAdjust())
        {
            case SvxAdjust::Right:
                aHor.SetValue(SvxCellHorJustify::Right);
                break;
            case SvxAdjust::Center:
                aHor.SetValue(SvxCellHorJustify::Center);
                break;
            default:
                aHor.SetValue(SvxCellHorJustify::Left);
                break;
        }
        SvxVerJustifyItem aVer(GetVerJustify());
        switch (GetVerticalAlignment().GetVertOrient())
        {
            case text::VertOrientation::BOTTOM:
                aVer.SetValue(SvxCellVerJustify::Bottom);
                break;
            case text::VertOrientation::CENTER:
                aVer.SetValue(SvxCellVerJustify::Center);
                break;
            default:
                aVer.SetValue(SvxCellVerJustify::Top);
                break;
        }
        SetHorJustify(aHor);
        SetVerJustify(aVer);
        return;
    }

    SwFormatVertOrient aVert(GetVerticalAlignment());
    switch (GetVerJustify().GetValue())
    {
        case SvxCellVerJustify::Bottom:
            aVert.SetVertOrient(text::VertOrientation::BOTTOM);
            break;
        case SvxCellVerJustify::Center:
            aVert.SetVertOrient(text::VertOrientation::CENTER);
            break;
        default:
            aVert.SetVertOrient(text::VertOrientation::TOP);
            break;
    }

    SetVerticalAlignment(aVert);
}

SwBoxAutoFormat::~SwBoxAutoFormat() {}

SwBoxAutoFormat& SwBoxAutoFormat::operator=(const SwBoxAutoFormat& rRef)
{
    // check self-assignment
    if (this == &rRef)
    {
        return *this;
    }

    // call baseclass implementation
    AutoFormatBase::operator=(rRef);

    // copy local members - this will use ::Clone() on all involved Items
    SetTextOrientation(rRef.GetTextOrientation());
    SetVerticalAlignment(rRef.GetVerticalAlignment());
    SetNumFormatString(rRef.GetNumFormatString());
    SetSysLanguage(rRef.GetSysLanguage());
    SetNumFormatLanguage(rRef.GetNumFormatLanguage());

    return *this;
}

bool SwBoxAutoFormat::operator==(const SwBoxAutoFormat& rRight) const
{
    return GetBackground().GetColor() == rRight.GetBackground().GetColor();
}

void SwBoxAutoFormat::SetXObject(rtl::Reference<SwXTextCellStyle> const& xObject)
{
    m_xAutoFormatUnoObject = xObject.get();
}

SwTableAutoFormat::SwTableAutoFormat(const TableStyleName& aName)
    : SvxAutoFormatData()
    , m_aName(aName)
    , m_bHidden(false)
    , m_bUserDefined(true)
{
    SvxAutoFormatData::SetName(aName.toString());
    for (size_t i = 0; i < ELEMENT_COUNT; i++)
    {
        m_aBoxAutoFormat[i] = std::make_unique<SwBoxAutoFormat>();
    }
}

SwTableAutoFormat::SwTableAutoFormat(const SwTableAutoFormat& rNew)
    : SvxAutoFormatData(rNew)
    , m_aName(rNew.GetName())
    , m_bHidden(false)
    , m_bUserDefined(rNew.m_bUserDefined)
{
    SvxAutoFormatData::SetName(rNew.GetName().toString());
    for (size_t i = 0; i < ELEMENT_COUNT; i++)
    {
        m_aBoxAutoFormat[i] = std::make_unique<SwBoxAutoFormat>(*rNew.GetField(i));
    }
}

SwTableAutoFormat::SwTableAutoFormat(const SvxAutoFormatData& rNew)
    : SvxAutoFormatData(rNew)
    , m_aName(rNew.GetName())
    , m_bHidden(false)
    , m_bUserDefined(true)
{
    SvxAutoFormatData::SetName(rNew.GetName());
    for (size_t i = 0; i < ELEMENT_COUNT; i++)
    {
        m_aBoxAutoFormat[i] = std::make_unique<SwBoxAutoFormat>(*rNew.GetField(i));
    }
}

SwTableAutoFormat& SwTableAutoFormat::operator=(const SwTableAutoFormat& rNew)
{
    if (&rNew == this)
        return *this;

    m_aName = rNew.m_aName;
    SetParent(rNew.GetParent());
    SetFont(rNew.IsFont());
    SetJustify(rNew.IsJustify());
    SetFrame(rNew.IsFrame());
    SetBackground(rNew.IsBackground());
    SetValueFormat(rNew.IsValueFormat());
    SetWidthHeight(rNew.IsWidthHeight());
    SvxAutoFormatData::SetName(m_aName.toString());
    m_bHidden = rNew.m_bHidden;
    m_bUserDefined = true;

    for (size_t i = 0; i < ELEMENT_COUNT; i++)
    {
        m_aBoxAutoFormat[i] = std::make_unique<SwBoxAutoFormat>(*rNew.m_aBoxAutoFormat[i]);
    }

    return *this;
}

void SwTableAutoFormat::SetName(const TableStyleName& rNew)
{
    m_aName = rNew;
    SvxAutoFormatData::SetName(rNew.toString());
}

void SwTableAutoFormat::SetName(const OUString& rName)
{
    m_aName = TableStyleName(rName);
    SvxAutoFormatData::SetName(rName);
}

void SwTableAutoFormat::SetBoxFormat(const SwBoxAutoFormat& rNew, size_t nIndex)
{
    OSL_ENSURE(nIndex < ELEMENT_COUNT, "wrong area");
    m_aBoxAutoFormat[nIndex] = std::make_unique<SwBoxAutoFormat>(rNew);
}

void SwTableAutoFormat::DisableAll()
{
    SetFont(false);
    SetJustify(false);
    SetFrame(false);
    SetBackground(false);
    SetValueFormat(false);
    SetWidthHeight(false);
}

const SwBoxAutoFormat& SwTableAutoFormat::GetDefaultBoxFormat()
{
    if (!s_pDefaultBoxAutoFormat)
        s_pDefaultBoxAutoFormat = new SwBoxAutoFormat();

    return *s_pDefaultBoxAutoFormat;
}

SwBoxAutoFormat* SwTableAutoFormat::GetField(size_t nIndex)
{
    if (nIndex < ELEMENT_COUNT)
        return m_aBoxAutoFormat[nIndex].get();

    SAL_WARN("sw", "SwTableAutoFormat::GetField - index out of bounds: " << nIndex);
    return nullptr;
}

const SwBoxAutoFormat* SwTableAutoFormat::GetField(size_t nIndex) const
{
    if (nIndex < ELEMENT_COUNT)
        return m_aBoxAutoFormat[nIndex].get();

    SAL_WARN("svx", "SwTableAutoFormat::GetField - index out of bounds: " << nIndex);
    return nullptr;
}

bool SwTableAutoFormat::SetField(size_t nIndex, const SvxAutoFormatDataField& aField)
{
    if (nIndex < ELEMENT_COUNT)
    {
        m_aBoxAutoFormat[nIndex] = std::make_unique<SwBoxAutoFormat>(aField);
        return true;
    }

    SAL_WARN("svx", "SvxAutoFormatData::SetField - index out of bounds: " << nIndex);
    return false;
}

size_t SwTableAutoFormat::GetIndex(const SwBoxAutoFormat& rBoxFormat) const
{
    size_t nIndex = 0;
    for (; nIndex < ELEMENT_COUNT; ++nIndex)
        if (m_aBoxAutoFormat[nIndex].get() == &rBoxFormat)
            break;

    // box format doesn't belong to this table format
    if (nIndex >= ELEMENT_COUNT)
        return ELEMENT_COUNT;
    return nIndex;
}

void SwTableAutoFormat::RestoreToOriginal(SwContentNode* rContentNode, SfxItemSet& aDummySet,
                                          size_t nRow, size_t nCol, size_t nRows,
                                          size_t nCols) const
{
    UpdateToSet(aDummySet, nRow, nCol, nRows, nCols, nullptr);
    SfxItemSet aItemSet = rContentNode->GetSwAttrSet();

    if (aItemSet.Get(RES_CHRATR_FONT) == aDummySet.Get(RES_CHRATR_FONT))
        rContentNode->ResetAttr(RES_CHRATR_FONT);
    if (aItemSet.Get(RES_CHRATR_FONTSIZE) == aDummySet.Get(RES_CHRATR_FONTSIZE))
        rContentNode->ResetAttr(RES_CHRATR_FONTSIZE);
    if (aItemSet.Get(RES_CHRATR_WEIGHT) == aDummySet.Get(RES_CHRATR_WEIGHT))
        rContentNode->ResetAttr(RES_CHRATR_WEIGHT);
    if (aItemSet.Get(RES_CHRATR_POSTURE) == aDummySet.Get(RES_CHRATR_POSTURE))
        rContentNode->ResetAttr(RES_CHRATR_POSTURE);
    if (aItemSet.Get(RES_CHRATR_CJK_FONT) == aDummySet.Get(RES_CHRATR_CJK_FONT))
        rContentNode->ResetAttr(RES_CHRATR_CJK_FONT);
    if (aItemSet.Get(RES_CHRATR_CJK_FONTSIZE) == aDummySet.Get(RES_CHRATR_CJK_FONTSIZE))
        rContentNode->ResetAttr(RES_CHRATR_CJK_FONTSIZE);
    if (aItemSet.Get(RES_CHRATR_CJK_WEIGHT) == aDummySet.Get(RES_CHRATR_CJK_WEIGHT))
        rContentNode->ResetAttr(RES_CHRATR_CJK_WEIGHT);
    if (aItemSet.Get(RES_CHRATR_CJK_POSTURE) == aDummySet.Get(RES_CHRATR_CJK_POSTURE))
        rContentNode->ResetAttr(RES_CHRATR_CJK_POSTURE);
    if (aItemSet.Get(RES_CHRATR_CTL_FONT) == aDummySet.Get(RES_CHRATR_CTL_FONT))
        rContentNode->ResetAttr(RES_CHRATR_CTL_FONT);
    if (aItemSet.Get(RES_CHRATR_CTL_FONTSIZE) == aDummySet.Get(RES_CHRATR_CTL_FONTSIZE))
        rContentNode->ResetAttr(RES_CHRATR_CTL_FONTSIZE);
    if (aItemSet.Get(RES_CHRATR_CTL_WEIGHT) == aDummySet.Get(RES_CHRATR_CTL_WEIGHT))
        rContentNode->ResetAttr(RES_CHRATR_CTL_WEIGHT);
    if (aItemSet.Get(RES_CHRATR_CTL_POSTURE) == aDummySet.Get(RES_CHRATR_CTL_POSTURE))
        rContentNode->ResetAttr(RES_CHRATR_CTL_POSTURE);
    if (aItemSet.Get(RES_CHRATR_UNDERLINE) == aDummySet.Get(RES_CHRATR_UNDERLINE))
        rContentNode->ResetAttr(RES_CHRATR_UNDERLINE);
    if (aItemSet.Get(RES_CHRATR_COLOR) == aDummySet.Get(RES_CHRATR_COLOR))
        rContentNode->ResetAttr(RES_CHRATR_COLOR);
    if (aItemSet.Get(RES_PARATR_ADJUST) == aDummySet.Get(RES_PARATR_ADJUST))
        rContentNode->ResetAttr(RES_PARATR_ADJUST);
}

void SwTableAutoFormat::UpdateToSet(SfxItemSet& aItemSet, size_t nRow, size_t nCol, size_t nRows,
                                    size_t nCols, SvNumberFormatter* pNFormatr) const
{
    bool bFirstRow = nRow == 0, bLastRow = nRow == nRows - 1, bFirstCol = nCol == 0,
         bLastCol = nCol == nCols - 1, bEvenRow = nRow % 2 != 0, bEvenCol = nCol % 2 != 0;

    SwBoxAutoFormat aDefault;
    aItemSet.Put(aDefault.GetBox());
    aItemSet.Put(aDefault.GetBackground());

    // Background & Body
    FillToItemSet(BACKGROUND, aItemSet, pNFormatr);
    FillToItemSet(BODY, aItemSet, pNFormatr);

    if (UseBandedColStyles())
    {
        if (bEvenCol)
            FillToItemSet(EVEN_COL, aItemSet, pNFormatr);
        else
            FillToItemSet(ODD_COL, aItemSet, pNFormatr);
    }

    if (UseBandedRowStyles())
    {
        if (bEvenRow)
            FillToItemSet(EVEN_ROW, aItemSet, pNFormatr);
        else
            FillToItemSet(ODD_ROW, aItemSet, pNFormatr);
    }

    if (bFirstCol && UseFirstColStyles())
        FillToItemSet(FIRST_COL, aItemSet, pNFormatr);
    if (bLastCol && UseLastColStyles())
        FillToItemSet(LAST_COL, aItemSet, pNFormatr);

    // First & Last Row
    if (UseFirstColStyles())
    {
        if (bFirstRow)
            FillToItemSet(FIRST_ROW, aItemSet, pNFormatr);
        if (bFirstRow && bEvenCol)
            FillToItemSet(FIRST_ROW_EVEN_COL, aItemSet, pNFormatr);
        if (bFirstRow && bLastCol)
            FillToItemSet(FIRST_ROW_END_COL, aItemSet, pNFormatr);
        if (bFirstRow && bFirstCol)
            FillToItemSet(FIRST_ROW_START_COL, aItemSet, pNFormatr);
    }
    if (UseLastRowStyles())
    {
        if (bLastRow)
            FillToItemSet(LAST_ROW, aItemSet, pNFormatr);

        if (bLastRow && bEvenCol)
            FillToItemSet(LAST_ROW_EVEN_COL, aItemSet, pNFormatr);
        if (bLastRow && bLastCol)
            FillToItemSet(LAST_ROW_END_COL, aItemSet, pNFormatr);
        if (bLastRow && bFirstCol)
            FillToItemSet(LAST_ROW_START_COL, aItemSet, pNFormatr);
    }
}

void SwTableAutoFormat::UpdateFromSet(size_t nPos, const SfxItemSet& rSet,
                                      SwTableAutoFormatUpdateFlags eFlags,
                                      SvNumberFormatter const* pNFormatr)
{
    OSL_ENSURE(nPos < ELEMENT_COUNT, "wrong area");
    SwBoxAutoFormat* pFormat = m_aBoxAutoFormat[nPos].get();

    if (SwTableAutoFormatUpdateFlags::Char & eFlags)
    {
        pFormat->SetFont(rSet.Get(RES_CHRATR_FONT));
        pFormat->SetHeight(rSet.Get(RES_CHRATR_FONTSIZE));
        pFormat->SetWeight(rSet.Get(RES_CHRATR_WEIGHT));
        pFormat->SetPosture(rSet.Get(RES_CHRATR_POSTURE));
        pFormat->SetCJKFont(rSet.Get(RES_CHRATR_CJK_FONT));
        pFormat->SetCJKHeight(rSet.Get(RES_CHRATR_CJK_FONTSIZE));
        pFormat->SetCJKWeight(rSet.Get(RES_CHRATR_CJK_WEIGHT));
        pFormat->SetCJKPosture(rSet.Get(RES_CHRATR_CJK_POSTURE));
        pFormat->SetCTLFont(rSet.Get(RES_CHRATR_CTL_FONT));
        pFormat->SetCTLHeight(rSet.Get(RES_CHRATR_CTL_FONTSIZE));
        pFormat->SetCTLWeight(rSet.Get(RES_CHRATR_CTL_WEIGHT));
        pFormat->SetCTLPosture(rSet.Get(RES_CHRATR_CTL_POSTURE));
        pFormat->SetUnderline(rSet.Get(RES_CHRATR_UNDERLINE));
        pFormat->SetOverline(rSet.Get(RES_CHRATR_OVERLINE));
        pFormat->SetCrossedOut(rSet.Get(RES_CHRATR_CROSSEDOUT));
        pFormat->SetContour(rSet.Get(RES_CHRATR_CONTOUR));
        pFormat->SetShadowed(rSet.Get(RES_CHRATR_SHADOWED));
        pFormat->SetColor(rSet.Get(RES_CHRATR_COLOR));
        pFormat->SetAdjust(rSet.Get(RES_PARATR_ADJUST));
    }
    if (!(SwTableAutoFormatUpdateFlags::Box & eFlags))
        return;

    pFormat->SetBox(rSet.Get(RES_BOX));
    // FIXME - add attribute IDs for the diagonal line items
    //        pFormat->SetTLBR( (SvxLineItem&)rSet.Get( RES_... ) );
    //        pFormat->SetBLTR( (SvxLineItem&)rSet.Get( RES_... ) );
    pFormat->SetBackground(rSet.Get(RES_BACKGROUND));
    pFormat->SetTextOrientation(rSet.Get(RES_FRAMEDIR));
    pFormat->SetVerticalAlignment(rSet.Get(RES_VERT_ORIENT));

    const SwTableBoxNumFormat* pNumFormatItem;
    const SvNumberformat* pNumFormat = nullptr;
    if (pNFormatr && (pNumFormatItem = rSet.GetItemIfSet(RES_BOXATR_FORMAT))
        && nullptr != (pNumFormat = pNFormatr->GetEntry(pNumFormatItem->GetValue())))
        pFormat->SetValueFormat(pNumFormat->GetFormatstring(), pNumFormat->GetLanguage(),
                                ::GetAppLanguage());
    else
    {
        // default
        pFormat->SetValueFormat(OUString(), LANGUAGE_SYSTEM, ::GetAppLanguage());
    }
}

void SwTableAutoFormat::FillToItemSet(size_t nIndex, SfxItemSet& rItemSet,
                                      SvNumberFormatter* pNFormatr) const
{
    SvxAutoFormatData::FillToItemSet(nIndex, rItemSet);

    const SwBoxAutoFormat aBox = *GetField(nIndex), aDefault = GetDefaultBoxFormat();
    if (IsJustify())
    {
        if (aBox.GetAdjust() != aDefault.GetAdjust())
            rItemSet.Put(aBox.GetAdjust());
        if (aBox.GetVerJustify() != aDefault.GetVerJustify())
            rItemSet.Put(aBox.GetVerticalAlignment());
    }
    if (IsFrame())
    {
        SvxBoxItem aNewBox(aBox.GetBox());
        SvxBoxItem aOldBox(rItemSet.Get(RES_BOX));

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

    if (IsValueFormat() && pNFormatr)
    {
        OUString sFormat;
        LanguageType eLng, eSys;
        aBox.GetValueFormat(sFormat, eLng, eSys);
        if (!sFormat.isEmpty())
        {
            SvNumFormatType nType;
            bool bNew;
            sal_Int32 nCheckPos;
            sal_uInt32 nKey = pNFormatr->GetIndexPuttingAndConverting(sFormat, eLng, eSys, nType,
                                                                      bNew, nCheckPos);
            rItemSet.Put(SwTableBoxNumFormat(nKey));
        }
        else
            rItemSet.ClearItem(RES_BOXATR_FORMAT);
    }
}

bool SwTableAutoFormat::FirstRowEndColumnIsRow()
{
    if (*GetField(FIRST_ROW) != GetDefaultBoxFormat()
        || *GetField(LAST_COL) == GetDefaultBoxFormat())
        return true;
    return false;
}

bool SwTableAutoFormat::FirstRowStartColumnIsRow()
{
    if (*GetField(FIRST_ROW) != GetDefaultBoxFormat()
        || *GetField(FIRST_COL) == GetDefaultBoxFormat())
        return true;
    return false;
}

bool SwTableAutoFormat::LastRowEndColumnIsRow()
{
    if (*GetField(LAST_ROW) != GetDefaultBoxFormat()
        || *GetField(LAST_COL) == GetDefaultBoxFormat())
        return true;
    return false;
}

bool SwTableAutoFormat::LastRowStartColumnIsRow()
{
    if (*GetField(LAST_ROW) != GetDefaultBoxFormat()
        || *GetField(FIRST_COL) == GetDefaultBoxFormat())
        return true;
    return false;
}

bool SwTableAutoFormat::HasHeaderRow() const
{ // Wild guessing for PDF export: is header different from odd or body?
    // It would be vastly better to do like SdrTableObj and have flags that
    // determine which "special" styles apply, instead of horrible guessing.
    return *GetField(FIRST_ROW) != *GetField(BODY);
}

void SwTableAutoFormat::SetXObject(rtl::Reference<SwXTextTableStyle> const& xObject)
{
    m_xUnoTextTableStyle = xObject.get();
}

struct SwTableAutoFormatTable::Impl
{
    std::vector<std::unique_ptr<SwTableAutoFormat>> m_AutoFormats;
};

SwTableAutoFormatTable::SwTableAutoFormatTable()
    : m_pImpl(new Impl)
{
    Load(true);

    for (size_t i = 0; i < size(); i++)
    {
        SwTableAutoFormat* pFormat = GetData(i);
        pFormat->SetUserDefined(false);
        for (size_t j = 0; j < ELEMENT_COUNT; j++)
        {
            SwBoxAutoFormat& aBox = *pFormat->GetField(j);
            aBox.UpdateAlignment();
            aBox.SetSysLanguage(::GetAppLanguage());
            aBox.SetNumFormatLanguage(::GetAppLanguage());
        }
    }
}

SwTableAutoFormatTable::~SwTableAutoFormatTable() = default;

size_t SwTableAutoFormatTable::size() const { return m_pImpl->m_AutoFormats.size(); }

SwTableAutoFormat const& SwTableAutoFormatTable::operator[](size_t const i) const
{
    return *m_pImpl->m_AutoFormats[i];
}

SwTableAutoFormat& SwTableAutoFormatTable::operator[](size_t const i)
{
    return *m_pImpl->m_AutoFormats[i];
}

const SwTableAutoFormat* SwTableAutoFormatTable::GetData(size_t nIndex) const
{
    return &*m_pImpl->m_AutoFormats[nIndex];
}

SwTableAutoFormat* SwTableAutoFormatTable::GetData(size_t nIndex)
{
    return &*m_pImpl->m_AutoFormats[nIndex];
}

void SwTableAutoFormatTable::AddAutoFormat(const SwTableAutoFormat& rTableStyle)
{
    // don't insert when we already have style of this name
    if (FindAutoFormat(rTableStyle.GetName()))
        return;

    InsertAutoFormat(new SwTableAutoFormat(rTableStyle));
}

bool SwTableAutoFormatTable::InsertAutoFormat(SvxAutoFormatData* pFormat)
{
    if (!pFormat)
    {
        SAL_WARN("sw", "Attempting to insert null format into SwTableAutoFormatTable");
        return false;
    }
    auto pNew = std::make_unique<SwTableAutoFormat>(*pFormat);
    m_pImpl->m_AutoFormats.emplace_back(std::move(pNew));
    return true;
}

SwTableAutoFormat* SwTableAutoFormatTable::FindAutoFormat(const OUString& rName) const
{
    for (const auto& rFormat : m_pImpl->m_AutoFormats)
    {
        if (rFormat->SvxAutoFormatData::GetName() == rName)
            return rFormat.get();
    }
    return nullptr;
}

SwTableAutoFormat* SwTableAutoFormatTable::FindAutoFormat(const TableStyleName& rName) const
{
    return FindAutoFormat(rName.toString());
}

static const SwTableAutoFormat* lcl_GetResolvedStyle(const SwTableAutoFormatTable& rTable,
                                                     const SwTableAutoFormat* pData,
                                                     std::unordered_set<OUString>& visited)
{
    if (!pData)
        return nullptr;

    OUString sName = pData->GetName().toString();
    if (visited.find(sName) == visited.end())
    {
        visited.insert(sName);

        const SwTableAutoFormat* pParent = nullptr;
        if (pData->GetParent().getLength())
            pParent
                = lcl_GetResolvedStyle(rTable, rTable.FindAutoFormat(pData->GetParent()), visited);

        if (pParent)
        {
            auto pNew = new SwTableAutoFormat(*pData);
            pNew->MergeStyle(*pParent);
            return pNew;
        }
    }
    else
    {
        SAL_WARN("sw", "Circular dependency in table styles");
        return pData;
    }

    return pData;
}

const SwTableAutoFormat*
SwTableAutoFormatTable::GetResolvedStyle(const SvxAutoFormatData* pStyle) const
{
    auto pData = dynamic_cast<const SwTableAutoFormat*>(pStyle);
    if (!pData)
        return nullptr;

    std::unordered_set<OUString> visited;
    return lcl_GetResolvedStyle(*this, pData, visited);
}

SwTableAutoFormat* SwTableAutoFormatTable::ReleaseAutoFormat(const OUString& rName)
{
    SwTableAutoFormat* pFormat = FindAutoFormat(rName);

    auto iter = std::find_if(m_pImpl->m_AutoFormats.begin(), m_pImpl->m_AutoFormats.end(),
                             [&pFormat](const std::unique_ptr<SwTableAutoFormat>& rpFormat)
                             {
                                 return rpFormat->SvxAutoFormatData::GetName()
                                        == pFormat->SvxAutoFormatData::GetName();
                             });

    SwTableAutoFormat* pRet = nullptr;
    if (iter != m_pImpl->m_AutoFormats.end())
    {
        pRet = iter->release();
        m_pImpl->m_AutoFormats.erase(iter);
    }
    ResetParent(rName);
    return pRet;
}

std::unique_ptr<SwTableAutoFormat>
SwTableAutoFormatTable::ReleaseAutoFormat(const TableStyleName& rName)
{
    std::unique_ptr<SwTableAutoFormat> pRet;
    SwTableAutoFormat* pFormat = ReleaseAutoFormat(rName.toString());
    if (pFormat)
        pRet = std::make_unique<SwTableAutoFormat>(*pFormat);

    return pRet;
}

SwCellStyleTable::SwCellStyleTable() {}

SwCellStyleTable::~SwCellStyleTable() {}

size_t SwCellStyleTable::size() const { return m_aCellStyles.size(); }

void SwCellStyleTable::clear() { m_aCellStyles.clear(); }

SwCellStyleDescriptor SwCellStyleTable::operator[](size_t i) const
{
    return SwCellStyleDescriptor(m_aCellStyles[i]);
}

void SwCellStyleTable::AddBoxFormat(const SwBoxAutoFormat& rBoxFormat, const UIName& sName)
{
    m_aCellStyles.emplace_back(sName, std::make_unique<SwBoxAutoFormat>(rBoxFormat));
}

void SwCellStyleTable::RemoveBoxFormat(const OUString& sName)
{
    auto iter
        = std::find_if(m_aCellStyles.begin(), m_aCellStyles.end(),
                       [&sName](const std::pair<UIName, std::unique_ptr<SwBoxAutoFormat>>& rStyle)
                       { return rStyle.first == sName; });
    if (iter != m_aCellStyles.end())
    {
        m_aCellStyles.erase(iter);
        return;
    }
    SAL_INFO("sw.core", "SwCellStyleTable::RemoveBoxFormat, format with given name doesn't exists");
}

UIName SwCellStyleTable::GetBoxFormatName(const SwBoxAutoFormat& rBoxFormat) const
{
    for (size_t i = 0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].second.get() == &rBoxFormat)
            return m_aCellStyles[i].first;
    }

    // box format not found
    return UIName();
}

SwBoxAutoFormat* SwCellStyleTable::GetBoxFormat(const UIName& sName) const
{
    for (size_t i = 0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].first == sName)
            return m_aCellStyles[i].second.get();
    }

    return nullptr;
}

void SwCellStyleTable::ChangeBoxFormatName(std::u16string_view sFromName, const UIName& sToName)
{
    if (!GetBoxFormat(sToName))
    {
        SAL_INFO("sw.core", "SwCellStyleTable::ChangeBoxName, box with given name already exists");
        return;
    }
    for (size_t i = 0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].first == sFromName)
        {
            m_aCellStyles[i].first = sToName;
            // changed successfully
            return;
        }
    }
    SAL_INFO("sw.core", "SwCellStyleTable::ChangeBoxName, box with given name not found");
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
