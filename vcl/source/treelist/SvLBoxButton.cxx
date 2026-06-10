/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <SvLBoxButton.hxx>

#include <vcl/decoview.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/toolkit/button.hxx>

SvLBoxButtonData::SvLBoxButtonData(const Control& rControlForSettings, bool _bRadioBtn)
    : m_pEntry(nullptr)
    , m_pBox(nullptr)
    , m_bShowRadioButton(false)
{
    bDataOk = false;
    m_bShowRadioButton = _bRadioBtn;

    SetDefaultImages(rControlForSettings);
}

SvLBoxButtonData::~SvLBoxButtonData() {}

void SvLBoxButtonData::CallLink() { aLink.Call(this); }

SvBmp SvLBoxButtonData::GetIndex(SvItemStateFlags nItemState)
{
    SvBmp nIdx;
    if (nItemState == SvItemStateFlags::UNCHECKED)
        nIdx = SvBmp::UNCHECKED;
    else if (nItemState == SvItemStateFlags::CHECKED)
        nIdx = SvBmp::CHECKED;
    else if (nItemState == SvItemStateFlags::TRISTATE)
        nIdx = SvBmp::TRISTATE;
    else if (nItemState == (SvItemStateFlags::UNCHECKED | SvItemStateFlags::HIGHLIGHTED))
        nIdx = SvBmp::HIUNCHECKED;
    else if (nItemState == (SvItemStateFlags::CHECKED | SvItemStateFlags::HIGHLIGHTED))
        nIdx = SvBmp::HICHECKED;
    else if (nItemState == (SvItemStateFlags::TRISTATE | SvItemStateFlags::HIGHLIGHTED))
        nIdx = SvBmp::HITRISTATE;
    else
        nIdx = SvBmp::UNCHECKED;
    return nIdx;
}

const Size& SvLBoxButtonData::GetSize()
{
    if (!bDataOk)
    {
        m_aSize = aBmps.at(SvBmp::UNCHECKED).GetSizePixel();
        bDataOk = true;
    }

    return m_aSize;
}

void SvLBoxButtonData::StoreButtonState(SvTreeListEntry* pActEntry, SvLBoxButton* pActBox)
{
    m_pEntry = pActEntry;
    m_pBox = pActBox;
}

SvButtonState SvLBoxButtonData::ConvertToButtonState(SvItemStateFlags nItemFlags)
{
    nItemFlags
        &= SvItemStateFlags::UNCHECKED | SvItemStateFlags::CHECKED | SvItemStateFlags::TRISTATE;
    switch (nItemFlags)
    {
        case SvItemStateFlags::UNCHECKED:
            return SvButtonState::Unchecked;
        case SvItemStateFlags::CHECKED:
            return SvButtonState::Checked;
        case SvItemStateFlags::TRISTATE:
            return SvButtonState::Tristate;
        default:
            return SvButtonState::Unchecked;
    }
}

SvTreeListEntry* SvLBoxButtonData::GetActEntry() const { return m_pEntry; }

SvLBoxButton* SvLBoxButtonData::GetActBox() const { return m_pBox; }

void SvLBoxButtonData::SetDefaultImages(const Control& rCtrl)
{
    const AllSettings& rSettings = rCtrl.GetSettings();

    if (m_bShowRadioButton)
    {
        aBmps[SvBmp::UNCHECKED] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::Default);
        aBmps[SvBmp::CHECKED] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::Checked);
        aBmps[SvBmp::HICHECKED] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed);
        aBmps[SvBmp::HIUNCHECKED] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed);
        aBmps[SvBmp::TRISTATE] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::DontKnow);
        aBmps[SvBmp::HITRISTATE] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed);
    }
    else
    {
        aBmps[SvBmp::UNCHECKED] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::Default);
        aBmps[SvBmp::CHECKED] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::Checked);
        aBmps[SvBmp::HICHECKED] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed);
        aBmps[SvBmp::HIUNCHECKED] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed);
        aBmps[SvBmp::TRISTATE] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::DontKnow);
        aBmps[SvBmp::HITRISTATE] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed);
    }
}

bool SvLBoxButtonData::IsRadio() const { return m_bShowRadioButton; }

SvLBoxButton::SvLBoxButton(SvLBoxButtonData* pBData)
    : m_bIsVis(true)
    , m_pData(pBData)
    , m_nItemFlags(SvItemStateFlags::NONE)
{
    SetStateUnchecked();
}

SvLBoxButton::SvLBoxButton()
    : m_bIsVis(false)
    , m_pData(nullptr)
    , m_nItemFlags(SvItemStateFlags::NONE)
{
    SetStateUnchecked();
}

SvLBoxButton::~SvLBoxButton() {}

SvLBoxItemType SvLBoxButton::GetType() const { return SvLBoxItemType::Button; }

void SvLBoxButton::ClickHdl(SvTreeListEntry* pEntry)
{
    if (IsStateChecked())
        SetStateUnchecked();
    else
        SetStateChecked();
    m_pData->StoreButtonState(pEntry, this);
    m_pData->CallLink();
}

void SvLBoxButton::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                         const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    SvBmp nIndex = SvLBoxButtonData::GetIndex(m_nItemFlags);
    DrawImageFlags nStyle
        = (rDev.IsEnabled() && !mbDisabled) ? DrawImageFlags::NONE : DrawImageFlags::Disable;

    //Native drawing
    bool bNativeOK = false;
    ControlType eCtrlType = (m_pData->IsRadio()) ? ControlType::Radiobutton : ControlType::Checkbox;
    if (rRenderContext.IsNativeControlSupported(eCtrlType, ControlPart::Entire))
    {
        Size aSize = m_pData->GetSize();
        ImplAdjustBoxSize(aSize, eCtrlType, rRenderContext);
        ImplControlValue aControlValue;
        tools::Rectangle aCtrlRegion(rPos, aSize);
        ControlState nState = ControlState::NONE;

        //states ControlState::DEFAULT, ControlState::PRESSED and ControlState::ROLLOVER are not implemented
        if (IsStateHilighted())
            nState |= ControlState::FOCUSED;
        if (nStyle != DrawImageFlags::Disable)
            nState |= ControlState::ENABLED;
        if (IsStateChecked())
            aControlValue.setTristateVal(ButtonValue::On);
        else if (IsStateUnchecked())
            aControlValue.setTristateVal(ButtonValue::Off);
        else if (IsStateTristate())
            aControlValue.setTristateVal(ButtonValue::Mixed);

        if (m_bIsVis)
            bNativeOK = rRenderContext.DrawNativeControl(
                eCtrlType, ControlPart::Entire, aCtrlRegion, nState, aControlValue, OUString());
    }

    if (!bNativeOK && m_bIsVis)
        rRenderContext.DrawImage(rPos, m_pData->GetImage(nIndex), nStyle);
}

std::unique_ptr<SvLBoxItem> SvLBoxButton::Clone(SvLBoxItem const* pSource) const
{
    std::unique_ptr<SvLBoxButton> pNew(new SvLBoxButton);
    pNew->m_pData = static_cast<SvLBoxButton const*>(pSource)->m_pData;
    return pNew;
}

void SvLBoxButton::ImplAdjustBoxSize(Size& io_rSize, ControlType i_eType,
                                     vcl::RenderContext const& rRenderContext)
{
    if (!rRenderContext.IsNativeControlSupported(i_eType, ControlPart::Entire))
        return;

    ImplControlValue aControlValue;
    tools::Rectangle aCtrlRegion(Point(0, 0), io_rSize);

    aControlValue.setTristateVal(ButtonValue::On);

    tools::Rectangle aNativeBounds, aNativeContent;
    bool bNativeOK = rRenderContext.GetNativeControlRegion(
        i_eType, ControlPart::Entire, aCtrlRegion, ControlState::ENABLED, aControlValue,
        aNativeBounds, aNativeContent);
    if (bNativeOK)
    {
        Size aContentSize(aNativeContent.GetSize());
        // leave a little space around the box image (looks better)
        if (aContentSize.Height() + 2 > io_rSize.Height())
            io_rSize.setHeight(aContentSize.Height() + 2);
        if (aContentSize.Width() + 2 > io_rSize.Width())
            io_rSize.setWidth(aContentSize.Width() + 2);
    }
}

void SvLBoxButton::InitViewData(SvTreeListBox& rView, SvTreeListEntry* pEntry,
                                SvViewDataItem* pViewData)
{
    if (!pViewData)
        pViewData = &rView.GetViewDataItem(pEntry, *this);
    Size aSize = m_pData->GetSize();

    ControlType eCtrlType = (m_pData->IsRadio()) ? ControlType::Radiobutton : ControlType::Checkbox;
    ImplAdjustBoxSize(aSize, eCtrlType, *rView.GetOutDev());
    pViewData->mnWidth = aSize.Width();
    pViewData->mnHeight = aSize.Height();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
