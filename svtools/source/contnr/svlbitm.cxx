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

    SvLBoxButtonData_Impl() : pEntry( NULL ), bDefaultImages( false ), bShowRadioButton( false ) {}
};



void SvLBoxButtonData::InitData( bool bImagesFromDefault, bool _bRadioBtn, const Control* pCtrl )
{
    nWidth = nHeight = 0;

    pImpl = new SvLBoxButtonData_Impl;

    bDataOk = false;
    eState = SV_BUTTON_UNCHECKED;
    pImpl->bDefaultImages = bImagesFromDefault;
    pImpl->bShowRadioButton = _bRadioBtn;

    if ( bImagesFromDefault )
        SetDefaultImages( pCtrl );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings )
{
    InitData( true, false, pControlForSettings );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn )
{
    InitData( true, _bRadioBtn, pControlForSettings );
}

SvLBoxButtonData::~SvLBoxButtonData()
{

    delete pImpl;
#ifdef DBG_UTIL
    pImpl = NULL;
#endif
}

void SvLBoxButtonData::CallLink()
{
    aLink.Call( this );
}

sal_uInt16 SvLBoxButtonData::GetIndex( sal_uInt16 nItemState )
{
    nItemState &= 0x000F;
    sal_uInt16 nIdx;
    switch( nItemState )
    {
        case SV_ITEMSTATE_UNCHECKED:
                nIdx = SV_BMP_UNCHECKED; break;
        case SV_ITEMSTATE_CHECKED:
                nIdx = SV_BMP_CHECKED; break;
        case SV_ITEMSTATE_TRISTATE:
                nIdx = SV_BMP_TRISTATE; break;
        case SV_ITEMSTATE_UNCHECKED | SV_ITEMSTATE_HILIGHTED:
                nIdx = SV_BMP_HIUNCHECKED; break;
        case SV_ITEMSTATE_CHECKED | SV_ITEMSTATE_HILIGHTED:
                nIdx = SV_BMP_HICHECKED; break;
        case SV_ITEMSTATE_TRISTATE | SV_ITEMSTATE_HILIGHTED:
                nIdx = SV_BMP_HITRISTATE; break;
        default:
                nIdx = SV_BMP_UNCHECKED;
    }
    return nIdx;
}

void SvLBoxButtonData::SetWidthAndHeight()
{
    Size aSize = aBmps[0].GetSizePixel();
    nWidth = aSize.Width();
    nHeight = aSize.Height();
    bDataOk = true;
}


void SvLBoxButtonData::StoreButtonState( SvTreeListEntry* pActEntry, sal_uInt16 nItemFlags )
{
    pImpl->pEntry = pActEntry;
    eState = ConvertToButtonState( nItemFlags );
}

SvButtonState SvLBoxButtonData::ConvertToButtonState( sal_uInt16 nItemFlags ) const
{
    nItemFlags &= (SV_ITEMSTATE_UNCHECKED |
                   SV_ITEMSTATE_CHECKED |
                   SV_ITEMSTATE_TRISTATE);
    switch( nItemFlags )
    {
        case SV_ITEMSTATE_UNCHECKED:
            return SV_BUTTON_UNCHECKED;

        case SV_ITEMSTATE_CHECKED:
            return SV_BUTTON_CHECKED;

        case SV_ITEMSTATE_TRISTATE:
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
        aBmps[ SV_BMP_UNCHECKED ]   = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_DEFAULT );
        aBmps[ SV_BMP_CHECKED ]     = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_CHECKED );
        aBmps[ SV_BMP_HICHECKED ]   = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_CHECKED | BUTTON_DRAW_PRESSED );
        aBmps[ SV_BMP_HIUNCHECKED ] = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_DEFAULT | BUTTON_DRAW_PRESSED );
        aBmps[ SV_BMP_TRISTATE ]    = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_DONTKNOW );
        aBmps[ SV_BMP_HITRISTATE ]  = RadioButton::GetRadioImage( rSettings, BUTTON_DRAW_DONTKNOW | BUTTON_DRAW_PRESSED );
    }
    else
    {
        aBmps[ SV_BMP_UNCHECKED ]   = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DEFAULT );
        aBmps[ SV_BMP_CHECKED ]     = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_CHECKED );
        aBmps[ SV_BMP_HICHECKED ]   = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_CHECKED | BUTTON_DRAW_PRESSED );
        aBmps[ SV_BMP_HIUNCHECKED ] = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DEFAULT | BUTTON_DRAW_PRESSED );
        aBmps[ SV_BMP_TRISTATE ]    = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DONTKNOW );
        aBmps[ SV_BMP_HITRISTATE ]  = CheckBox::GetCheckImage( rSettings, BUTTON_DRAW_DONTKNOW | BUTTON_DRAW_PRESSED );
    }
}

bool SvLBoxButtonData::HasDefaultImages( void ) const
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
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/,
    const SvTreeListEntry* pEntry)
{
    if (pEntry)
    {
        sal_uInt16 nStyle = rDev.IsEnabled() ? 0 : TEXT_DRAW_DISABLE;
        if ( rDev.IsEntryMnemonicsEnabled() )
            nStyle |= TEXT_DRAW_MNEMONIC;
        rDev.DrawText(Rectangle(rPos, GetSize(&rDev, pEntry)), maText, nStyle);
    }
    else
        rDev.DrawText(rPos, maText);

}

SvLBoxItem* SvLBoxString::Create() const
{
    return new SvLBoxString;
}

void SvLBoxString::Clone( SvLBoxItem* pSource )
{
    maText = ((SvLBoxString*)pSource)->maText;
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
        static SvTreeListBox *pPreviousView = NULL;
        static float fApproximateCharWidth = 0.0;
        if (pPreviousView != pView)
        {
            pPreviousView = pView;
            fApproximateCharWidth = pView->approximate_char_width();
        }
        nTextWidth = maText.getLength() * fApproximateCharWidth;
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

void SvLBoxBmp::Paint(
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/,
    const SvTreeListEntry* /*pEntry*/)
{
    sal_uInt16 nStyle = rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;
    rDev.DrawImage( rPos, aBmp ,nStyle);
}

SvLBoxItem* SvLBoxBmp::Create() const
{
    return new SvLBoxBmp;
}

void SvLBoxBmp::Clone( SvLBoxItem* pSource )
{
    aBmp = ((SvLBoxBmp*)pSource)->aBmp;
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
    , nItemFlags(0)
    , nBaseOffs(0)
{
    SetStateUnchecked();
}

SvLBoxButton::SvLBoxButton()
    : SvLBoxItem()
    , isVis(false)
    , pData(0)
    , eKind(SvLBoxButtonKind_enabledCheckbox)
    , nItemFlags(0)
    , nBaseOffs(0)
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
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/,
    const SvTreeListEntry* /*pEntry*/)
{
    sal_uInt16 nIndex = eKind == SvLBoxButtonKind_staticImage
        ? SV_BMP_STATICIMAGE : pData->GetIndex( nItemFlags );
    sal_uInt16 nStyle = eKind != SvLBoxButtonKind_disabledCheckbox &&
        rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;

    //Native drawing
    bool bNativeOK = false;
    ControlType eCtrlType = (pData->IsRadio())? CTRL_RADIOBUTTON : CTRL_CHECKBOX;
    if ( nIndex != SV_BMP_STATICIMAGE && rDev.IsNativeControlSupported( eCtrlType, PART_ENTIRE_CONTROL) )

    {
        Size aSize(pData->Width(), pData->Height());
        ImplAdjustBoxSize( aSize, eCtrlType, &rDev );
        ImplControlValue    aControlValue;
        Rectangle           aCtrlRegion( rPos, aSize );
        ControlState        nState = 0;

        //states CTRL_STATE_DEFAULT, CTRL_STATE_PRESSED and CTRL_STATE_ROLLOVER are not implemented
        if ( IsStateHilighted() )                   nState |= CTRL_STATE_FOCUSED;
        if ( nStyle != IMAGE_DRAW_DISABLE )         nState |= CTRL_STATE_ENABLED;

        if ( IsStateChecked() )
            aControlValue.setTristateVal( BUTTONVALUE_ON );
        else if ( IsStateUnchecked() )
            aControlValue.setTristateVal( BUTTONVALUE_OFF );
        else if ( IsStateTristate() )
            aControlValue.setTristateVal( BUTTONVALUE_MIXED );

        if( isVis)
            bNativeOK = rDev.DrawNativeControl( eCtrlType, PART_ENTIRE_CONTROL,
                                aCtrlRegion, nState, aControlValue, OUString() );
    }

    if( !bNativeOK && isVis )
        rDev.DrawImage( rPos, pData->aBmps[nIndex + nBaseOffs] ,nStyle);
}

SvLBoxItem* SvLBoxButton::Create() const
{
    return new SvLBoxButton;
}

void SvLBoxButton::Clone( SvLBoxItem* pSource )
{
    pData = ((SvLBoxButton*)pSource)->pData;
}

void SvLBoxButton::ImplAdjustBoxSize( Size& io_rSize, ControlType i_eType, vcl::Window* i_pParent )
{
    if ( i_pParent->IsNativeControlSupported( i_eType, PART_ENTIRE_CONTROL) )
    {
        ImplControlValue    aControlValue;
        Rectangle           aCtrlRegion( Point( 0, 0 ), io_rSize );
        ControlState        nState = CTRL_STATE_ENABLED;

        aControlValue.setTristateVal( BUTTONVALUE_ON );

        Rectangle aNativeBounds, aNativeContent;
        bool bNativeOK = i_pParent->GetNativeControlRegion( i_eType,
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

void SvLBoxButton::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry,
    SvViewDataItem* pViewData )
{
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    Size aSize( pData->Width(), pData->Height() );

    ControlType eCtrlType = (pData->IsRadio())? CTRL_RADIOBUTTON : CTRL_CHECKBOX;
    if ( eKind != SvLBoxButtonKind_staticImage && pView )
        ImplAdjustBoxSize( aSize, eCtrlType, pView );
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
    const Point& _rPos, SvTreeListBox& _rDev,
    const SvViewDataEntry* pView, const SvTreeListEntry* pEntry)
{

    // get the image.
    const Image& rImage = implGetImageStore(pView->IsExpanded() != m_pImpl->m_bExpanded);

    bool _bSemiTransparent = pEntry && ( 0 != ( SV_ENTRYFLAG_SEMITRANSPARENT  & pEntry->GetFlags( ) ) );
    // draw
    sal_uInt16 nStyle = _rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;
    if ( _bSemiTransparent )
        nStyle |= IMAGE_DRAW_SEMITRANSPARENT;
    _rDev.DrawImage( _rPos, rImage, nStyle);
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
