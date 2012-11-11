/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <svtools/treelistbox.hxx>
#include <svtools/svlbitm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/button.hxx>
#include <vcl/decoview.hxx>
#include <vcl/salnativewidgets.hxx>

struct SvLBoxButtonData_Impl
{
    SvTreeListEntry*    pEntry;
    sal_Bool            bDefaultImages;
    sal_Bool            bShowRadioButton;

    SvLBoxButtonData_Impl() : pEntry( NULL ), bDefaultImages( sal_False ), bShowRadioButton( sal_False ) {}
};


DBG_NAME(SvLBoxButtonData)

void SvLBoxButtonData::InitData( sal_Bool bImagesFromDefault, bool _bRadioBtn, const Control* pCtrl )
{
    pImpl = new SvLBoxButtonData_Impl;

    bDataOk = sal_False;
    eState = SV_BUTTON_UNCHECKED;
    pImpl->bDefaultImages = bImagesFromDefault;
    pImpl->bShowRadioButton = ( _bRadioBtn != false );

    if ( bImagesFromDefault )
        SetDefaultImages( pCtrl );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings )
{
    DBG_CTOR(SvLBoxButtonData,0);

    InitData( sal_True, false, pControlForSettings );
}

SvLBoxButtonData::SvLBoxButtonData( const Control* pControlForSettings, bool _bRadioBtn )
{
    DBG_CTOR(SvLBoxButtonData,0);

    InitData( sal_True, _bRadioBtn, pControlForSettings );
}

SvLBoxButtonData::~SvLBoxButtonData()
{
    DBG_DTOR(SvLBoxButtonData,0);

    delete pImpl;
#ifdef DBG_UTIL
    pImpl = NULL;
#endif
}

void SvLBoxButtonData::CallLink()
{
    DBG_CHKTHIS(SvLBoxButtonData,0);
    aLink.Call( this );
}

sal_uInt16 SvLBoxButtonData::GetIndex( sal_uInt16 nItemState )
{
    DBG_CHKTHIS(SvLBoxButtonData,0);
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
    DBG_CHKTHIS(SvLBoxButtonData,0);
    Size aSize = aBmps[0].GetSizePixel();
    nWidth = aSize.Width();
    nHeight = aSize.Height();
    bDataOk = sal_True;
}


void SvLBoxButtonData::StoreButtonState( SvTreeListEntry* pActEntry, sal_uInt16 nItemFlags )
{
    DBG_CHKTHIS(SvLBoxButtonData,0);
    pImpl->pEntry = pActEntry;
    eState = ConvertToButtonState( nItemFlags );
}

SvButtonState SvLBoxButtonData::ConvertToButtonState( sal_uInt16 nItemFlags ) const
{
    DBG_CHKTHIS(SvLBoxButtonData,0);
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
    DBG_ASSERT( pImpl, "-SvLBoxButtonData::GetActEntry(): don't use me that way!" );
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

sal_Bool SvLBoxButtonData::HasDefaultImages( void ) const
{
    return pImpl->bDefaultImages;
}

sal_Bool SvLBoxButtonData::IsRadio() {
    return pImpl->bShowRadioButton;
}

// ***************************************************************
// class SvLBoxString
// ***************************************************************

DBG_NAME(SvLBoxString);

SvLBoxString::SvLBoxString(SvTreeListEntry* pEntry, sal_uInt16 nFlags, const rtl::OUString& rStr)
    : SvLBoxItem(pEntry, nFlags)
{
    DBG_CTOR(SvLBoxString,0);
    SetText(rStr);
}

SvLBoxString::SvLBoxString() : SvLBoxItem()
{
    DBG_CTOR(SvLBoxString,0);
}

SvLBoxString::~SvLBoxString()
{
    DBG_DTOR(SvLBoxString,0);
}

sal_uInt16 SvLBoxString::IsA()
{
    DBG_CHKTHIS(SvLBoxString,0);
    return SV_ITEM_ID_LBOXSTRING;
}

void SvLBoxString::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /* nFlags */,
    SvTreeListEntry* _pEntry)
{
    DBG_CHKTHIS(SvLBoxString,0);
    if ( _pEntry )
    {
        sal_uInt16 nStyle = rDev.IsEnabled() ? 0 : TEXT_DRAW_DISABLE;
        if ( rDev.IsEntryMnemonicsEnabled() )
            nStyle |= TEXT_DRAW_MNEMONIC;
        rDev.DrawText(Rectangle(rPos, GetSize(&rDev,_pEntry)), maText, nStyle);
    }
    else
        rDev.DrawText(rPos, maText);

}

SvLBoxItem* SvLBoxString::Create() const
{
    DBG_CHKTHIS(SvLBoxString,0);
    return new SvLBoxString;
}

void SvLBoxString::Clone( SvLBoxItem* pSource )
{
    DBG_CHKTHIS(SvLBoxString,0);
    maText = ((SvLBoxString*)pSource)->maText;
}

void SvLBoxString::InitViewData(
    SvTreeListBox* pView, SvTreeListEntry* pEntry, SvViewDataItem* pViewData)
{
    DBG_CHKTHIS(SvLBoxString,0);
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    pViewData->aSize = Size(pView->GetTextWidth(maText), pView->GetTextHeight());
}

// ***************************************************************
// class SvLBoxBmp
// ***************************************************************

DBG_NAME(SvLBoxBmp);

SvLBoxBmp::SvLBoxBmp() : SvLBoxItem()
{
    DBG_CTOR(SvLBoxBmp,0);
}

SvLBoxBmp::~SvLBoxBmp()
{
    DBG_DTOR(SvLBoxBmp,0);
}

sal_uInt16 SvLBoxBmp::IsA()
{
    DBG_CHKTHIS(SvLBoxBmp,0);
    return SV_ITEM_ID_LBOXBMP;
}

void SvLBoxBmp::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry,
    SvViewDataItem* pViewData)
{
    DBG_CHKTHIS(SvLBoxBmp,0);
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    pViewData->aSize = aBmp.GetSizePixel();
}

void SvLBoxBmp::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /* nFlags */,
                        SvTreeListEntry* )
{
    DBG_CHKTHIS(SvLBoxBmp,0);
    sal_uInt16 nStyle = rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;
    rDev.DrawImage( rPos, aBmp ,nStyle);
}

SvLBoxItem* SvLBoxBmp::Create() const
{
    DBG_CHKTHIS(SvLBoxBmp,0);
    return new SvLBoxBmp;
}

void SvLBoxBmp::Clone( SvLBoxItem* pSource )
{
    DBG_CHKTHIS(SvLBoxBmp,0);
    aBmp = ((SvLBoxBmp*)pSource)->aBmp;
}

// ***************************************************************
// class SvLBoxButton
// ***************************************************************

DBG_NAME(SvLBoxButton);

SvLBoxButton::SvLBoxButton( SvTreeListEntry* pEntry, SvLBoxButtonKind eTheKind,
                            sal_uInt16 nFlags, SvLBoxButtonData* pBData )
    : SvLBoxItem( pEntry, nFlags )
{
    DBG_CTOR(SvLBoxButton,0);
    eKind = eTheKind;
    nBaseOffs = 0;
    nItemFlags = 0;
    SetStateUnchecked();
    pData = pBData;
}

SvLBoxButton::SvLBoxButton() : SvLBoxItem()
{
    DBG_CTOR(SvLBoxButton,0);
    eKind = SvLBoxButtonKind_enabledCheckbox;
    nItemFlags = 0;
    SetStateUnchecked();
}

SvLBoxButton::~SvLBoxButton()
{
    DBG_DTOR(SvLBoxButton,0);
}

sal_uInt16 SvLBoxButton::IsA()
{
    DBG_CHKTHIS(SvLBoxButton,0);
    return SV_ITEM_ID_LBOXBUTTON;
}

sal_Bool SvLBoxButton::ClickHdl( SvTreeListBox*, SvTreeListEntry* pEntry )
{
    DBG_CHKTHIS(SvLBoxButton,0);
    if ( CheckModification() )
    {
        if ( IsStateChecked() )
            SetStateUnchecked();
        else
            SetStateChecked();
        pData->StoreButtonState( pEntry, nItemFlags );
        pData->CallLink();
    }
    return sal_False;
}

void SvLBoxButton::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 /* nFlags */,
                            SvTreeListEntry* /*pEntry*/ )
{
    DBG_CHKTHIS(SvLBoxButton,0);
    sal_uInt16 nIndex = eKind == SvLBoxButtonKind_staticImage
        ? SV_BMP_STATICIMAGE : pData->GetIndex( nItemFlags );
    sal_uInt16 nStyle = eKind != SvLBoxButtonKind_disabledCheckbox &&
        rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;

///
//Native drawing
///
    sal_Bool bNativeOK = sal_False;
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

        bNativeOK = rDev.DrawNativeControl( eCtrlType, PART_ENTIRE_CONTROL,
                                aCtrlRegion, nState, aControlValue, rtl::OUString() );
    }

    if( !bNativeOK)
        rDev.DrawImage( rPos, pData->aBmps[nIndex + nBaseOffs] ,nStyle);
}

SvLBoxItem* SvLBoxButton::Create() const
{
    DBG_CHKTHIS(SvLBoxButton,0);
    return new SvLBoxButton;
}

void SvLBoxButton::Clone( SvLBoxItem* pSource )
{
    DBG_CHKTHIS(SvLBoxButton,0);
    pData = ((SvLBoxButton*)pSource)->pData;
}

void SvLBoxButton::ImplAdjustBoxSize( Size& io_rSize, ControlType i_eType, Window* i_pParent )
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
                                                            rtl::OUString(),
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
    DBG_CHKTHIS(SvLBoxButton,0);
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    Size aSize( pData->Width(), pData->Height() );

    ControlType eCtrlType = (pData->IsRadio())? CTRL_RADIOBUTTON : CTRL_CHECKBOX;
    if ( eKind != SvLBoxButtonKind_staticImage && pView )
        ImplAdjustBoxSize( aSize, eCtrlType, pView );
    pViewData->aSize = aSize;
}

bool SvLBoxButton::CheckModification() const
{
    return eKind == SvLBoxButtonKind_enabledCheckbox;
}

// ***************************************************************
// class SvLBoxContextBmp
// ***************************************************************

struct SvLBoxContextBmp_Impl
{
    Image       m_aImage1;
    Image       m_aImage2;

    sal_uInt16      m_nB2IndicatorFlags;
};

// ***************************************************************
DBG_NAME(SvLBoxContextBmp)

SvLBoxContextBmp::SvLBoxContextBmp( SvTreeListEntry* pEntry, sal_uInt16 nItemFlags,
    Image aBmp1, Image aBmp2, sal_uInt16 nEntryFlags )
    :SvLBoxItem( pEntry, nItemFlags )
    ,m_pImpl( new SvLBoxContextBmp_Impl )
{
    DBG_CTOR(SvLBoxContextBmp,0);

    m_pImpl->m_nB2IndicatorFlags = nEntryFlags;
    SetModeImages( aBmp1, aBmp2 );
}

SvLBoxContextBmp::SvLBoxContextBmp()
    :SvLBoxItem( )
    ,m_pImpl( new SvLBoxContextBmp_Impl )
{
    m_pImpl->m_nB2IndicatorFlags = 0;
    DBG_CTOR(SvLBoxContextBmp,0);
}

SvLBoxContextBmp::~SvLBoxContextBmp()
{
    delete m_pImpl;
    DBG_DTOR(SvLBoxContextBmp,0);
}

sal_uInt16 SvLBoxContextBmp::IsA()
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);
    return SV_ITEM_ID_LBOXCONTEXTBMP;
}

sal_Bool SvLBoxContextBmp::SetModeImages( const Image& _rBitmap1, const Image& _rBitmap2 )
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);

    sal_Bool bSuccess = sal_True;
    m_pImpl->m_aImage1 = _rBitmap1;
    m_pImpl->m_aImage2 = _rBitmap2;
    return bSuccess;
}

Image& SvLBoxContextBmp::implGetImageStore( sal_Bool _bFirst )
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);

    // OJ: #i27071# wrong mode so we just return the normal images
    return _bFirst ? m_pImpl->m_aImage1 : m_pImpl->m_aImage2;
}

void SvLBoxContextBmp::InitViewData( SvTreeListBox* pView,SvTreeListEntry* pEntry,
    SvViewDataItem* pViewData)
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);
    if( !pViewData )
        pViewData = pView->GetViewDataItem( pEntry, this );
    pViewData->aSize = m_pImpl->m_aImage1.GetSizePixel();
}

void SvLBoxContextBmp::Paint( const Point& _rPos, SvTreeListBox& _rDev,
    sal_uInt16 _nViewDataEntryFlags, SvTreeListEntry* _pEntry )
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);

    // get the image
    const Image& rImage = implGetImageStore( 0 == ( _nViewDataEntryFlags & m_pImpl->m_nB2IndicatorFlags ) );

    sal_Bool _bSemiTransparent = _pEntry && ( 0 != ( SV_ENTRYFLAG_SEMITRANSPARENT  & _pEntry->GetFlags( ) ) );
    // draw
    sal_uInt16 nStyle = _rDev.IsEnabled() ? 0 : IMAGE_DRAW_DISABLE;
    if ( _bSemiTransparent )
        nStyle |= IMAGE_DRAW_SEMITRANSPARENT;
    _rDev.DrawImage( _rPos, rImage, nStyle);
}

SvLBoxItem* SvLBoxContextBmp::Create() const
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);
    return new SvLBoxContextBmp;
}

void SvLBoxContextBmp::Clone( SvLBoxItem* pSource )
{
    DBG_CHKTHIS(SvLBoxContextBmp,0);
    m_pImpl->m_aImage1 = static_cast< SvLBoxContextBmp* >( pSource )->m_pImpl->m_aImage1;
    m_pImpl->m_aImage2 = static_cast< SvLBoxContextBmp* >( pSource )->m_pImpl->m_aImage2;
    m_pImpl->m_nB2IndicatorFlags = static_cast< SvLBoxContextBmp* >( pSource )->m_pImpl->m_nB2IndicatorFlags;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
