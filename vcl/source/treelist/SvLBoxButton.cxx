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
    m_bShowRadioButton = _bRadioBtn;

    SetDefaultImages(rControlForSettings);
}

SvLBoxButtonData::~SvLBoxButtonData() {}

void SvLBoxButtonData::CallLink(SvTreeListEntry* pActEntry, SvLBoxButton* pActBox)
{
    m_pEntry = pActEntry;
    m_pBox = pActBox;
    m_aLink.Call(this);
}

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

const Size& SvLBoxButtonData::GetSize() { return m_aSize; }

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
        m_aBmps[SvBmp::UNCHECKED] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::Default);
        m_aBmps[SvBmp::CHECKED] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::Checked);
        m_aBmps[SvBmp::HICHECKED] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed);
        m_aBmps[SvBmp::HIUNCHECKED] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed);
        m_aBmps[SvBmp::TRISTATE] = RadioButton::GetRadioImage(rSettings, DrawButtonFlags::DontKnow);
        m_aBmps[SvBmp::HITRISTATE] = RadioButton::GetRadioImage(
            rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed);
    }
    else
    {
        m_aBmps[SvBmp::UNCHECKED] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::Default);
        m_aBmps[SvBmp::CHECKED] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::Checked);
        m_aBmps[SvBmp::HICHECKED] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed);
        m_aBmps[SvBmp::HIUNCHECKED] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed);
        m_aBmps[SvBmp::TRISTATE] = CheckBox::GetCheckImage(rSettings, DrawButtonFlags::DontKnow);
        m_aBmps[SvBmp::HITRISTATE] = CheckBox::GetCheckImage(
            rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed);
    }

    m_aSize = m_aBmps.at(SvBmp::UNCHECKED).GetSizePixel();
}

bool SvLBoxButtonData::IsRadio() const { return m_bShowRadioButton; }

SvLBoxButton::SvLBoxButton(SvLBoxButtonData& rBData, bool bIsVis)
    : m_bIsVis(bIsVis)
    , m_rData(rBData)
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

    m_rData.CallLink(pEntry, this);
}

void SvLBoxButton::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                         const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    SvBmp nIndex = SvLBoxButtonData::GetIndex(m_nItemFlags);
    DrawImageFlags nStyle
        = (rDev.IsEnabled() && !mbDisabled) ? DrawImageFlags::NONE : DrawImageFlags::Disable;

    //Native drawing
    bool bNativeOK = false;
    ControlType eCtrlType = (m_rData.IsRadio()) ? ControlType::Radiobutton : ControlType::Checkbox;
    if (rRenderContext.IsNativeControlSupported(eCtrlType, ControlPart::Entire))
    {
        Size aSize = m_rData.GetSize();
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
        rRenderContext.DrawImage(rPos, m_rData.GetImage(nIndex), nStyle);
}

std::unique_ptr<SvLBoxItem> SvLBoxButton::Clone(SvLBoxItem const* pSource) const
{
    return std::make_unique<SvLBoxButton>(static_cast<SvLBoxButton const*>(pSource)->m_rData,
                                          false);
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

void SvLBoxButton::InitViewData(SvTreeListBox& rView, SvTreeListEntry& rEntry,
                                SvViewDataItem* pViewData)
{
    if (!pViewData)
        pViewData = &rView.GetViewDataItem(rEntry, *this);
    Size aSize = m_rData.GetSize();

    ControlType eCtrlType = (m_rData.IsRadio()) ? ControlType::Radiobutton : ControlType::Checkbox;
    ImplAdjustBoxSize(aSize, eCtrlType, *rView.GetOutDev());
    pViewData->mnWidth = aSize.Width();
    pViewData->mnHeight = aSize.Height();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
