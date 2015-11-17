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

void SvLBoxButtonData::InitData( bool bImagesFromDefault, bool _bRadioBtn, const Control* pCtrl )
{
    nWidth = nHeight = 0;

    aBmps.resize((int)SvBmp::STATICIMAGE+1);

    bDataOk = false;
    eState = SV_BUTTON_UNCHECKED;
    pImpl->bDefaultImages = bImagesFromDefault;
    pImpl->bShowRadioButton = _bRadioBtn;

    if ( bImagesFromDefault )
        SetDefaultImages( pCtrl );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings )
    : pImpl( new SvLBoxButtonData_Impl )
{
    InitData( true, false, pControlForSettings );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn )
    : pImpl( new SvLBoxButtonData_Impl )
{
    InitData( true, _bRadioBtn, pControlForSettings );
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


void SvLBoxButtonData::StoreButtonState( SvTreeListEntry* pActEntry, SvItemStateFlags nItemFlags )
{
    pImpl->pEntry = pActEntry;
    eState = ConvertToButtonState( nItemFlags );
}

SvButtonState SvLBoxButtonData::ConvertToButtonState( SvItemStateFlags nItemFlags )
{
    nItemFlags &= (SvItemStateFlags::UNCHECKED |
                   SvItemStateFlags::CHECKED |
                   SvItemStateFlags::TRISTATE);
    switch( nItemFlags )
    {
        case SvItemStateFlags::UNCHECKED:
            return SV_BUTTON_UNCHECKED;
        case SvItemStateFlags::CHECKED:
            return SV_BUTTON_CHECKED;
        case SvItemStateFlags::TRISTATE:
            return SV_BUTTON_TRISTATE;
        default:
            return SV_BUTTON_UNCHECKED;
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


SvLBoxString::SvLBoxString(SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& rStr)
    : SvLBoxItem(pEntry, nFlags)
{
    SetText(rStr);
}

SvLBoxString::SvLBoxString() : SvLBoxItem()
{
}

SvLBoxString::~SvLBoxString()
{
}

sal_uInt16 SvLBoxString::GetType() const
{
    return SV_ITEM_ID_LBOXSTRING;
}

void SvLBoxString::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    DrawTextFlags nStyle = rDev.IsEnabled() ? DrawTextFlags::NONE : DrawTextFlags::Disable;
    if (rDev.IsEntryMnemonicsEnabled())
        nStyle |= DrawTextFlags::Mnemonic;
    rRenderContext.DrawText(Rectangle(rPos, GetSize(&rDev, &rEntry)), maText, nStyle);
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
// class SvLBoxBmp
// ***************************************************************


SvLBoxBmp::SvLBoxBmp() : SvLBoxItem()
{
}

SvLBoxBmp::~SvLBoxBmp()
{
}

sal_uInt16 SvLBoxBmp::GetType() const
{
    return SV_ITEM_ID_LBOXBMP;
}

void SvLBoxBmp::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry,
    SvViewDataItem* pViewData)
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    pViewData->maSize = aBmp.GetSizePixel();
}

void SvLBoxBmp::Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                      const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    DrawImageFlags nStyle = rDev.IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable;
    rRenderContext.DrawImage(rPos, aBmp ,nStyle);
}

SvLBoxItem* SvLBoxBmp::Create() const
{
    return new SvLBoxBmp;
}

void SvLBoxBmp::Clone( SvLBoxItem* pSource )
{
    aBmp = static_cast<SvLBoxBmp*>(pSource)->aBmp;
}

// ***************************************************************
// class SvLBoxButton
// ***************************************************************


SvLBoxButton::SvLBoxButton( SvTreeListEntry* pEntry, SvLBoxButtonKind eTheKind,
                            sal_uInt16 nFlags, SvLBoxButtonData* pBData )
    : SvLBoxItem( pEntry, nFlags )
    , isVis(true)
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
    , eKind(SvLBoxButtonKind_enabledCheckbox)
    , nItemFlags(SvItemStateFlags::NONE)
{
    SetStateUnchecked();
}

SvLBoxButton::~SvLBoxButton()
{
}

sal_uInt16 SvLBoxButton::GetType() const
{
    return SV_ITEM_ID_LBOXBUTTON;
}

bool SvLBoxButton::ClickHdl( SvTreeListBox*, SvTreeListEntry* pEntry )
{
    if ( CheckModification() )
    {
        if ( IsStateChecked() )
            SetStateUnchecked();
        else
            SetStateChecked();
        pData->StoreButtonState( pEntry, nItemFlags );
        pData->CallLink();
    }
    return false;
}

void SvLBoxButton::Paint(
    const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
    const SvViewDataEntry* /*pView*/, const SvTreeListEntry& /*rEntry*/)
{
    SvBmp nIndex = eKind == SvLBoxButtonKind_staticImage ? SvBmp::STATICIMAGE : SvLBoxButtonData::GetIndex(nItemFlags);
    DrawImageFlags nStyle = eKind != SvLBoxButtonKind_disabledCheckbox && rDev.IsEnabled() ? DrawImageFlags::NONE : DrawImageFlags::Disable;

    //Native drawing
    bool bNativeOK = false;
    ControlType eCtrlType = (pData->IsRadio())? CTRL_RADIOBUTTON : CTRL_CHECKBOX;
    if ( nIndex != SvBmp::STATICIMAGE && rRenderContext.IsNativeControlSupported( eCtrlType, PART_ENTIRE_CONTROL) )

    {
        Size aSize(pData->Width(), pData->Height());
        ImplAdjustBoxSize(aSize, eCtrlType, rRenderContext);
        ImplControlValue aControlValue;
        Rectangle aCtrlRegion( rPos, aSize );
        ControlState nState = ControlState::NONE;

        //states ControlState::DEFAULT, ControlState::PRESSED and ControlState::ROLLOVER are not implemented
        if (IsStateHilighted())
            nState |= ControlState::FOCUSED;
        if (nStyle != DrawImageFlags::Disable)
            nState |= ControlState::ENABLED;
        if (IsStateChecked())
            aControlValue.setTristateVal(BUTTONVALUE_ON);
        else if (IsStateUnchecked())
            aControlValue.setTristateVal(BUTTONVALUE_OFF);
        else if (IsStateTristate())
            aControlValue.setTristateVal( BUTTONVALUE_MIXED );

        if (isVis)
            bNativeOK = rRenderContext.DrawNativeControl(eCtrlType, PART_ENTIRE_CONTROL,
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
    if (rRenderContext.IsNativeControlSupported( i_eType, PART_ENTIRE_CONTROL) )
    {
        ImplControlValue    aControlValue;
        Rectangle           aCtrlRegion( Point( 0, 0 ), io_rSize );
        ControlState        nState = ControlState::ENABLED;

        aControlValue.setTristateVal( BUTTONVALUE_ON );

        Rectangle aNativeBounds, aNativeContent;
        bool bNativeOK = rRenderContext.GetNativeControlRegion( i_eType,
                                                            PART_ENTIRE_CONTROL,
                                                            aCtrlRegion,
                                                            nState,
                                                            aControlValue,
                                                            OUString(),
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

    ControlType eCtrlType = (pData->IsRadio())? CTRL_RADIOBUTTON : CTRL_CHECKBOX;
    if ( eKind != SvLBoxButtonKind_staticImage && pView )
        ImplAdjustBoxSize(aSize, eCtrlType, *pView);
    pViewData->maSize = aSize;
}

bool SvLBoxButton::CheckModification() const
{
    return eKind == SvLBoxButtonKind_enabledCheckbox;
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

SvLBoxContextBmp::SvLBoxContextBmp(
    SvTreeListEntry* pEntry, sal_uInt16 nItemFlags, Image aBmp1, Image aBmp2,
    bool bExpanded)
    :SvLBoxItem( pEntry, nItemFlags )
    ,m_pImpl( new SvLBoxContextBmp_Impl )
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
    delete m_pImpl;
}

sal_uInt16 SvLBoxContextBmp::GetType() const
{
    return SV_ITEM_ID_LBOXCONTEXTBMP;
}

bool SvLBoxContextBmp::SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2 )
{
    bool bSuccess = true;
    m_pImpl->m_aImage1 = _rBitmap1;
    m_pImpl->m_aImage2 = _rBitmap2;
    return bSuccess;
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
