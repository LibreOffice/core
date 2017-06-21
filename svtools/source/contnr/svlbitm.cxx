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

#include <svtools/treelistbox.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>
#include <svtools/viewdataentry.hxx>
#include <vcl/svapp.hxx>
#include <vcl/button.hxx>
#include <vcl/decoview.hxx>
#include <vcl/salnativewidgets.hxx>

struct SvLBoxButtonData_Impl
{
    SvTreeListEntry*    pEntry;
    bool                bDefaultImages;
    bool                bShowRadioButton;

    SvLBoxButtonData_Impl() : pEntry( nullptr ), bDefaultImages( false ), bShowRadioButton( false ) {}
};

void SvLBoxButtonData::InitData( bool _bRadioBtn, const Control* pCtrl )
{
    nWidth = nHeight = 0;

    aBmps.resize((int)SvBmp::STATICIMAGE+1);

    bDataOk = false;
    pImpl->bDefaultImages = true;
    pImpl->bShowRadioButton = _bRadioBtn;

    SetDefaultImages( pCtrl );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings )
    : pImpl( new SvLBoxButtonData_Impl )
{
    InitData( false, pControlForSettings );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn )
    : pImpl( new SvLBoxButtonData_Impl )
{
    InitData( _bRadioBtn, pControlForSettings );
}

SvLBoxButtonData::~SvLBoxButtonData()
{
}

void SvLBoxButtonData::CallLink()
{
    aLink.Call( this );
}

SvBmp SvLBoxButtonData::GetIndex( SvItemStateFlags nItemState )
{
    SvBmp nIdx;
    if (nItemState == SvItemStateFlags::UNCHECKED)
        nIdx = SvBmp::UNCHECKED;
    else if (nItemState == SvItemStateFlags::CHECKED)
        nIdx = SvBmp::CHECKED;
    else if (nItemState == SvItemStateFlags::TRISTATE)
        nIdx = SvBmp::TRISTATE;
    else if (nItemState == (SvItemStateFlags::UNCHECKED | SvItemStateFlags::HILIGHTED))
        nIdx = SvBmp::HIUNCHECKED;
    else if (nItemState == (SvItemStateFlags::CHECKED | SvItemStateFlags::HILIGHTED))
        nIdx = SvBmp::HICHECKED;
    else if (nItemState == (SvItemStateFlags::TRISTATE | SvItemStateFlags::HILIGHTED))
        nIdx = SvBmp::HITRISTATE;
    else
        nIdx = SvBmp::UNCHECKED;
    return nIdx;
}

void SvLBoxButtonData::SetWidthAndHeight()
{
    Size aSize = aBmps[(int)SvBmp::UNCHECKED].GetSizePixel();
    nWidth = aSize.Width();
    nHeight = aSize.Height();
    bDataOk = true;
}


void SvLBoxButtonData::StoreButtonState( SvTreeListEntry* pActEntry )
{
    pImpl->pEntry = pActEntry;
}

SvButtonState SvLBoxButtonData::ConvertToButtonState( SvItemStateFlags nItemFlags )
{
    nItemFlags &= (SvItemStateFlags::UNCHECKED |
                   SvItemStateFlags::CHECKED |
                   SvItemStateFlags::TRISTATE);
    switch( nItemFlags )
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

SvTreeListEntry* SvLBoxButtonData::GetActEntry() const
{
    assert(pImpl && "-SvLBoxButtonData::GetActEntry(): don't use me that way!");
    return pImpl->pEntry;
}

void SvLBoxButtonData::SetDefaultImages( const Control* pCtrl )
{
    const AllSettings& rSettings = pCtrl? pCtrl->GetSettings() : Application::GetSettings();

    if ( pImpl->bShowRadioButton )
    {
        SetImage(SvBmp::UNCHECKED,   RadioButton::GetRadioImage( rSettings, DrawButtonFlags::Default ) );
        SetImage(SvBmp::CHECKED,     RadioButton::GetRadioImage( rSettings, DrawButtonFlags::Checked ) );
        SetImage(SvBmp::HICHECKED,   RadioButton::GetRadioImage( rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed ) );
        SetImage(SvBmp::HIUNCHECKED, RadioButton::GetRadioImage( rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed ) );
        SetImage(SvBmp::TRISTATE,    RadioButton::GetRadioImage( rSettings, DrawButtonFlags::DontKnow ) );
        SetImage(SvBmp::HITRISTATE,  RadioButton::GetRadioImage( rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed ) );
    }
    else
    {
        SetImage(SvBmp::UNCHECKED,   CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default ) );
        SetImage(SvBmp::CHECKED,     CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked ) );
        SetImage(SvBmp::HICHECKED,   CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed ) );
        SetImage(SvBmp::HIUNCHECKED, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed ) );
        SetImage(SvBmp::TRISTATE,    CheckBox::GetCheckImage( rSettings, DrawButtonFlags::DontKnow ) );
        SetImage(SvBmp::HITRISTATE,  CheckBox::GetCheckImage( rSettings, DrawButtonFlags::DontKnow | DrawButtonFlags::Pressed ) );
    }
}

bool SvLBoxButtonData::HasDefaultImages() const
{
    return pImpl->bDefaultImages;
}

bool SvLBoxButtonData::IsRadio() {
    return pImpl->bShowRadioButton;
}

// ***************************************************************
// class SvLBoxString
// ***************************************************************


SvLBoxString::SvLBoxString(const OUString& rStr)
{
    SetText(rStr);
}

SvLBoxString::SvLBoxString() : SvLBoxItem()
{
}

SvLBoxString::~SvLBoxString()
{
}

SvLBoxItemType SvLBoxString::GetType() const
{
    return SvLBoxItemType::String;
}

void SvLBoxString::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    Size aSize = GetSize(&rDev, &rEntry);
    DrawTextFlags nStyle = rDev.IsEnabled() ? DrawTextFlags::NONE : DrawTextFlags::Disable;
    if (rDev.IsEntryMnemonicsEnabled())
        nStyle |= DrawTextFlags::Mnemonic;
    if (rDev.TextCenterAndClipEnabled())
    {
        nStyle |= DrawTextFlags::PathEllipsis | DrawTextFlags::Center;
        aSize.Width() = rDev.GetEntryWidth();
    }
    rRenderContext.DrawText(tools::Rectangle(rPos, aSize), maText, nStyle);
}

SvLBoxItem* SvLBoxString::Create() const
{
    return new SvLBoxString;
}

void SvLBoxString::Clone( SvLBoxItem* pSource )
{
    maText = static_cast<SvLBoxString*>(pSource)->maText;
}

void SvLBoxString::InitViewData(
    SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );

    // fdo#72125: GetTextWidth() can get very expensive; let's just count
    // an approximate width using a cached value when we have many entries
    long nTextWidth;
    if (pView->GetEntryCount() > 100)
    {
        static SvTreeListBox *s_pPreviousView = nullptr;
        static float s_fApproximateCharWidth = 0.0;
        if (s_pPreviousView != pView)
        {
            s_pPreviousView = pView;
            s_fApproximateCharWidth = pView->approximate_char_width();
        }
        nTextWidth = maText.getLength() * s_fApproximateCharWidth;
    }
    else
        nTextWidth = pView->GetTextWidth(maText);

    pViewData->maSize = Size(nTextWidth, pView->GetTextHeight());
}

// ***************************************************************
// class SvLBoxButton
// ***************************************************************


SvLBoxButton::SvLBoxButton( SvLBoxButtonKind eTheKind,
                            SvLBoxButtonData* pBData )
    : isVis(true)
    , pData(pBData)
    , eKind(eTheKind)
    , nItemFlags(SvItemStateFlags::NONE)
{
    SetStateUnchecked();
}

SvLBoxButton::SvLBoxButton()
    : SvLBoxItem()
    , isVis(false)
    , pData(nullptr)
    , eKind(SvLBoxButtonKind::EnabledCheckbox)
    , nItemFlags(SvItemStateFlags::NONE)
{
    SetStateUnchecked();
}

SvLBoxButton::~SvLBoxButton()
{
}

SvLBoxItemType SvLBoxButton::GetType() const
{
    return SvLBoxItemType::Button;
}

bool SvLBoxButton::ClickHdl( SvTreeListEntry* pEntry )
{
    if ( CheckModification() )
    {
        if ( IsStateChecked() )
            SetStateUnchecked();
        else
            SetStateChecked();
        pData->StoreButtonState( pEntry );
        pData->CallLink();
    }
    return false;
}

void SvLBoxButton::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    SvBmp nIndex = eKind == SvLBoxButtonKind::StaticImage ? SvBmp::STATICIMAGE : SvLBoxButtonData::GetIndex(nItemFlags);
    DrawImageFlags nStyle = eKind != SvLBoxButtonKind::DisabledCheckbox && rDev.IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable;

    //Native drawing
    bool bNativeOK = false;
    ControlType eCtrlType = (pData->IsRadio())? ControlType::Radiobutton : ControlType::Checkbox;
    if ( nIndex != SvBmp::STATICIMAGE && rRenderContext.IsNativeControlSupported( eCtrlType, ControlPart::Entire) )

    {
        Size aSize(pData->Width(), pData->Height());
        ImplAdjustBoxSize(aSize, eCtrlType, rRenderContext);
        ImplControlValue aControlValue;
        tools::Rectangle aCtrlRegion( rPos, aSize );
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
            aControlValue.setTristateVal( ButtonValue::Mixed );

        if (isVis)
            bNativeOK = rRenderContext.DrawNativeControl(eCtrlType, ControlPart::Entire,
                                                         aCtrlRegion, nState, aControlValue, OUString());
    }

    if (!bNativeOK && isVis)
        rRenderContext.DrawImage(rPos, pData->GetImage(nIndex), nStyle);
}

SvLBoxItem* SvLBoxButton::Create() const
{
    return new SvLBoxButton;
}

void SvLBoxButton::Clone( SvLBoxItem* pSource )
{
    pData = static_cast<SvLBoxButton*>(pSource)->pData;
}

void SvLBoxButton::ImplAdjustBoxSize(Size& io_rSize, ControlType i_eType, vcl::RenderContext& rRenderContext)
{
    if (rRenderContext.IsNativeControlSupported( i_eType, ControlPart::Entire) )
    {
        ImplControlValue    aControlValue;
        tools::Rectangle           aCtrlRegion( Point( 0, 0 ), io_rSize );
        ControlState        nState = ControlState::ENABLED;

        aControlValue.setTristateVal( ButtonValue::On );

        tools::Rectangle aNativeBounds, aNativeContent;
        bool bNativeOK = rRenderContext.GetNativeControlRegion( i_eType,
                                                            ControlPart::Entire,
                                                            aCtrlRegion,
                                                            nState,
                                                            aControlValue,
                                                            aNativeBounds,
                                                            aNativeContent );
        if( bNativeOK )
        {
            Size aContentSize( aNativeContent.GetSize() );
            // leave a little space around the box image (looks better)
            if( aContentSize.Height() + 2 > io_rSize.Height() )
                io_rSize.Height() = aContentSize.Height() + 2;
            if( aContentSize.Width() + 2 > io_rSize.Width() )
                io_rSize.Width() = aContentSize.Width() + 2;
        }
    }
}

void SvLBoxButton::InitViewData(SvTreeListBox* pView,SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    Size aSize( pData->Width(), pData->Height() );

    ControlType eCtrlType = (pData->IsRadio())? ControlType::Radiobutton : ControlType::Checkbox;
    if ( eKind != SvLBoxButtonKind::StaticImage && pView )
        ImplAdjustBoxSize(aSize, eCtrlType, *pView);
    pViewData->maSize = aSize;
}

bool SvLBoxButton::CheckModification() const
{
    return eKind == SvLBoxButtonKind::EnabledCheckbox;
}

void SvLBoxButton::SetStateInvisible()
{
    isVis = false;
}

// ***************************************************************
// class SvLBoxContextBmp
// ***************************************************************

struct SvLBoxContextBmp_Impl
{
    Image       m_aImage1;
    Image       m_aImage2;

    bool        m_bExpanded;
};

// ***************************************************************

SvLBoxContextBmp::SvLBoxContextBmp(const Image& aBmp1, const Image& aBmp2,
    bool bExpanded)
    :m_pImpl( new SvLBoxContextBmp_Impl )
{

    m_pImpl->m_bExpanded = bExpanded;
    SetModeImages( aBmp1, aBmp2 );
}

SvLBoxContextBmp::SvLBoxContextBmp()
    :SvLBoxItem( )
    ,m_pImpl( new SvLBoxContextBmp_Impl )
{
    m_pImpl->m_bExpanded = false;
}

SvLBoxContextBmp::~SvLBoxContextBmp()
{
}

SvLBoxItemType SvLBoxContextBmp::GetType() const
{
    return SvLBoxItemType::ContextBmp;
}

void SvLBoxContextBmp::SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2 )
{
    m_pImpl->m_aImage1 = _rBitmap1;
    m_pImpl->m_aImage2 = _rBitmap2;
}

Image& SvLBoxContextBmp::implGetImageStore( bool _bFirst )
{

    // OJ: #i27071# wrong mode so we just return the normal images
    return _bFirst ? m_pImpl->m_aImage1 : m_pImpl->m_aImage2;
}

void SvLBoxContextBmp::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry,
    SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    pViewData->maSize = m_pImpl->m_aImage1.GetSizePixel();
}

void SvLBoxContextBmp::Paint(
    const Point& _rPos, SvTreeListBox& _rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* pView, const SvTreeListEntry& rEntry)
{

    // get the image.
    const Image& rImage = implGetImageStore(pView->IsExpanded() != m_pImpl->m_bExpanded);

    bool _bSemiTransparent = bool( SvTLEntryFlags::SEMITRANSPARENT & rEntry.GetFlags( ) );
    // draw
    DrawImageFlags nStyle = _rDev.IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable;
    if (_bSemiTransparent)
        nStyle |= DrawImageFlags::SemiTransparent;
    rRenderContext.DrawImage(_rPos, rImage, nStyle);
}

SvLBoxItem* SvLBoxContextBmp::Create() const
{
    return new SvLBoxContextBmp;
}

void SvLBoxContextBmp::Clone( SvLBoxItem* pSource )
{
    m_pImpl->m_aImage1 = static_cast< SvLBoxContextBmp* >( pSource )->m_pImpl->m_aImage1;
    m_pImpl->m_aImage2 = static_cast< SvLBoxContextBmp* >( pSource )->m_pImpl->m_aImage2;
    m_pImpl->m_bExpanded = static_cast<SvLBoxContextBmp*>(pSource)->m_pImpl->m_bExpanded;
}

long SvLBoxButtonData::Width()
{
    if ( !bDataOk )
        SetWidthAndHeight();
    return nWidth;
}

long SvLBoxButtonData::Height()
{
    if ( !bDataOk )
        SetWidthAndHeight();
    return nHeight;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
