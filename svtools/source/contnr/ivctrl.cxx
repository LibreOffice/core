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


#include <ivctrl.hxx>
#include "imivctl.hxx"
#include <vcl/bitmapex.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/mnemonic.hxx>
using namespace ::com::sun::star::accessibility;

/*****************************************************************************
|
| class : SvxIconChoiceCtrlEntry
|
\*****************************************************************************/

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( const OUString& rText, const Image& rImage, sal_uInt16 _nFlags )
{
    aText = rText;
    aImage = rImage;
    pUserData = NULL;

    nFlags = _nFlags;
    eTextMode = IcnShowTextShort;
    pblink = 0;
    pflink = 0;
}

OUString SvxIconChoiceCtrlEntry::GetDisplayText() const
{
    return MnemonicGenerator::EraseAllMnemonicChars( aText );
}

// ----------------------------------------------------------------------------

SvxIconChoiceCtrlColumnInfo::SvxIconChoiceCtrlColumnInfo( const SvxIconChoiceCtrlColumnInfo& rInfo )
    : aColText( rInfo.aColText ), aColImage( rInfo.aColImage )
{
    nWidth = rInfo.nWidth;
    eAlignment = rInfo.eAlignment;
    nSubItem = rInfo.nSubItem;
}

/*****************************************************************************
|
| class : SvtIconChoiceCtrl
|
\*****************************************************************************/

SvtIconChoiceCtrl::SvtIconChoiceCtrl( Window* pParent, WinBits nWinStyle ) :

     // WB_CLIPCHILDREN on, as ScrollBars lie on the window!
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),

    _pCurKeyEvent   ( NULL ),
    _pImp           ( new SvxIconChoiceCtrl_Impl( this, nWinStyle ) ),
    _bAutoFontColor ( sal_False )

{
    SetLineColor();
    _pImp->SetGrid( Size( 100, 70 ) );
    _pImp->InitSettings();
    _pImp->SetPositionMode( IcnViewPositionModeAutoArrange );
}

SvtIconChoiceCtrl::~SvtIconChoiceCtrl()
{
    _pImp->CallEventListeners( VCLEVENT_OBJECT_DYING );
    delete _pImp;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const OUString& rText, const Image& rImage, sal_uLong nPos, const Point* pPos, sal_uInt16 nFlags  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage, nFlags);

    _pImp->InsertEntry( pEntry, nPos, pPos );

    return pEntry;
}

sal_Bool SvtIconChoiceCtrl::EditedEntry( SvxIconChoiceCtrlEntry*, const OUString&, sal_Bool )
{
    return sal_True;
}

sal_Bool SvtIconChoiceCtrl::EditingEntry( SvxIconChoiceCtrlEntry* )
{
    return sal_True;
}

void SvtIconChoiceCtrl::DrawEntryImage( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, OutputDevice& rDev )
{
    rDev.DrawImage( rPos, pEntry->GetImage() );
}

OUString SvtIconChoiceCtrl::GetEntryText( SvxIconChoiceCtrlEntry* pEntry, sal_Bool )
{
    return pEntry->GetText();
}

sal_Bool SvtIconChoiceCtrl::HasBackground() const
{
    return sal_False;
}

sal_Bool SvtIconChoiceCtrl::HasFont() const
{
    return sal_False;
}

sal_Bool SvtIconChoiceCtrl::HasFontTextColor() const
{
    return sal_True;
}

sal_Bool SvtIconChoiceCtrl::HasFontFillColor() const
{
    return sal_True;
}

void SvtIconChoiceCtrl::Paint( const Rectangle& rRect )
{
    _pImp->Paint( rRect );
}

void SvtIconChoiceCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseButtonDown( rMEvt ) )
        Control::MouseButtonDown( rMEvt );
}

void SvtIconChoiceCtrl::MouseButtonUp( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseButtonUp( rMEvt ) )
        Control::MouseButtonUp( rMEvt );
}

void SvtIconChoiceCtrl::MouseMove( const MouseEvent& rMEvt )
{
    if( !_pImp->MouseMove( rMEvt ) )
        Control::MouseMove( rMEvt );
}
void SvtIconChoiceCtrl::ArrangeIcons()
{
    if ( GetStyle() & WB_ALIGN_TOP )
    {
        Size aFullSize;
        Rectangle aEntryRect;

        for ( sal_uLong i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setWidth ( aFullSize.getWidth()+aEntryRect.GetWidth() );
        }

        _pImp->Arrange ( sal_False, aFullSize.getWidth() );
    }
    else if ( GetStyle() & WB_ALIGN_LEFT )
    {
        Size aFullSize;
        Rectangle aEntryRect;

        for ( sal_uLong i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setHeight ( aFullSize.getHeight()+aEntryRect.GetHeight() );
        }

        _pImp->Arrange ( sal_False, 0, aFullSize.getHeight() );
    }
    else
    {
        _pImp->Arrange();
    }
    _pImp->Arrange( sal_False, 0, 1000 );
}
void SvtIconChoiceCtrl::Resize()
{
    _pImp->Resize();
    Control::Resize();
}

Point SvtIconChoiceCtrl::GetPixelPos( const Point& rPosLogic ) const
{
    Point aPos( rPosLogic );
    aPos += GetMapMode().GetOrigin();
    return aPos;
}

void SvtIconChoiceCtrl::DocumentRectChanged()
{
    _aDocRectChangedHdl.Call( this );
}

void SvtIconChoiceCtrl::VisibleRectChanged()
{
    _aVisRectChangedHdl.Call( this );
}

void SvtIconChoiceCtrl::GetFocus()
{
    _pImp->GetFocus();
    Control::GetFocus();
    sal_uLong nPos;
    SvxIconChoiceCtrlEntry* pSelectedEntry = GetSelectedEntry ( nPos );
    if ( pSelectedEntry )
        _pImp->CallEventListeners( VCLEVENT_LISTBOX_SELECT, pSelectedEntry );
}

void SvtIconChoiceCtrl::LoseFocus()
{
    _pImp->LoseFocus();
    Control::LoseFocus();
}

void SvtIconChoiceCtrl::SetFont( const Font& rFont )
{
    if( rFont != GetFont() )
    {
        Control::SetFont( rFont );
        _pImp->FontModified();
    }
}

void SvtIconChoiceCtrl::SetPointFont( const Font& rFont )
{
    if( rFont != GetPointFont() )
    {
        Control::SetPointFont( rFont );
        _pImp->FontModified();
    }
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( const Point& rPixPos, sal_Bool bHit ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvtIconChoiceCtrl*)this)->_pImp->GetEntry( aPos, bHit );
}

void SvtIconChoiceCtrl::SetStyle( WinBits nWinStyle )
{
    _pImp->SetStyle( nWinStyle );
}

WinBits SvtIconChoiceCtrl::GetStyle() const
{
    return _pImp->GetStyle();
}
void SvtIconChoiceCtrl::Command( const CommandEvent& rCEvt )
{
    _pImp->Command( rCEvt );
}

void SvtIconChoiceCtrl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->SetEntryTextMode( eMode, pEntry );
}

sal_uLong SvtIconChoiceCtrl::GetEntryCount() const
{
    return _pImp->GetEntryCount();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( sal_uLong nPos ) const
{
    return _pImp->GetEntry( nPos );
}

void SvtIconChoiceCtrl::CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics )
{
    _pImp->CreateAutoMnemonics( &_rUsedMnemonics );
}

void SvtIconChoiceCtrl::CreateAutoMnemonics( void )
{
    _pImp->CreateAutoMnemonics();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetSelectedEntry( sal_uLong& rPos ) const
{
    return _pImp->GetFirstSelectedEntry( rPos );
}

void SvtIconChoiceCtrl::ClickIcon()
{
    sal_uLong nPos;
    GetSelectedEntry ( nPos );
    _aClickIconHdl.Call( this );
}
sal_Bool SvtIconChoiceCtrl::IsEntryEditing() const
{
    return _pImp->IsEntryEditing();
}

sal_Bool SvtIconChoiceCtrl::SetChoiceWithCursor ( sal_Bool bDo )
{
    return _pImp->SetChoiceWithCursor (bDo);
}

void SvtIconChoiceCtrl::KeyInput( const KeyEvent& rKEvt )
{
    sal_Bool bKeyUsed = DoKeyInput( rKEvt );
    if ( !bKeyUsed )
    {
        _pCurKeyEvent = (KeyEvent*)&rKEvt;
        Control::KeyInput( rKEvt );
        _pCurKeyEvent = NULL;
    }
}
sal_Bool SvtIconChoiceCtrl::DoKeyInput( const KeyEvent& rKEvt )
{
    // under OS/2, we get key up/down even while editing
    if( IsEntryEditing() )
        return sal_True;
    _pCurKeyEvent = (KeyEvent*)&rKEvt;
    sal_Bool bHandled = _pImp->KeyInput( rKEvt );
    _pCurKeyEvent = NULL;
    return bHandled;
}
sal_uLong SvtIconChoiceCtrl::GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImp->GetEntryListPos( pEntry );
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetCursor( ) const
{
    return _pImp->GetCurEntry( );
}
void SvtIconChoiceCtrl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->SetCursor( pEntry );
}
void SvtIconChoiceCtrl::InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->InvalidateEntry( pEntry );
}

void SvtIconChoiceCtrl::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );
}


void SvtIconChoiceCtrl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ((rDCEvt.GetType() == DATACHANGED_SETTINGS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         (rDCEvt.GetType() == DATACHANGED_FONTS) ) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        _pImp->InitSettings();
        Invalidate(INVALIDATE_NOCHILDREN);
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvtIconChoiceCtrl::SetBackground( const Wallpaper& rPaper )
{
    if( rPaper != GetBackground() )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Wallpaper aEmpty;
        if( rPaper == aEmpty )
            Control::SetBackground( rStyleSettings.GetFieldColor() );
        else
        {
            Wallpaper aBackground( rPaper );
            // HACK, as background might be transparent!
            if( !aBackground.IsBitmap() )
                aBackground.SetStyle( WALLPAPER_TILE );

            WallpaperStyle eStyle = aBackground.GetStyle();
            Color aBack( aBackground.GetColor());
            Color aTrans( COL_TRANSPARENT );
            if( aBack == aTrans  && (
                (!aBackground.IsBitmap() ||
                    aBackground.GetBitmap().IsTransparent() ||
                    (eStyle != WALLPAPER_TILE && eStyle != WALLPAPER_SCALE))))
            {
                aBackground.SetColor( rStyleSettings.GetFieldColor() );
            }
            if( aBackground.IsScrollable() )
            {
                Rectangle aRect;
                aRect.SetSize( Size(32765, 32765) );
                aBackground.SetRect( aRect );
            }
            else
            {
                Rectangle aRect( _pImp->GetOutputRect() );
                aBackground.SetRect( aRect );
            }
            Control::SetBackground( aBackground );
        }

        // If text colors are attributed "hard," don't use automatism to select
        // a readable text color.
        Font aFont( GetFont() );
        aFont.SetColor( rStyleSettings.GetFieldTextColor() );
        SetFont( aFont );

        Invalidate(INVALIDATE_NOCHILDREN);
    }
}

void SvtIconChoiceCtrl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !_pImp->RequestHelp( rHEvt ) )
        Control::RequestHelp( rHEvt );
}

void SvtIconChoiceCtrl::SetSelectionMode( SelectionMode eMode )
{
    _pImp->SetSelectionMode( eMode );
}

Rectangle SvtIconChoiceCtrl::GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImp->GetEntryBoundRect( pEntry );
}

void SvtIconChoiceCtrl::FillLayoutData() const
{
    CreateLayoutData();

    SvtIconChoiceCtrl* pNonConstMe = const_cast< SvtIconChoiceCtrl* >( this );

    // loop through all entries
    sal_uInt16 nCount = (sal_uInt16)GetEntryCount();
    sal_uInt16 nPos = 0;
    while ( nPos < nCount )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( nPos );

        Point aPos = _pImp->GetEntryBoundRect( pEntry ).TopLeft();
        String sEntryText = pEntry->GetDisplayText( );
        Rectangle aTextRect = _pImp->CalcTextRect( pEntry, &aPos, sal_False, &sEntryText );

        sal_Bool bLargeIconMode = WB_ICON == ( _pImp->GetStyle() & ( VIEWMODE_MASK ) );
        sal_uInt16 nTextPaintFlags = bLargeIconMode ? PAINTFLAG_HOR_CENTERED : PAINTFLAG_VER_CENTERED;

        _pImp->PaintItem( aTextRect, IcnViewFieldTypeText, pEntry, nTextPaintFlags, pNonConstMe, &sEntryText, GetLayoutData() );

        ++nPos;
    }
}

Rectangle SvtIconChoiceCtrl::GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const
{
    Rectangle aRect;

    Pair aEntryCharacterRange = GetLineStartEnd( _nEntryPos );
    if ( aEntryCharacterRange.A() + _nCharacterIndex < aEntryCharacterRange.B() )
    {
        aRect = GetCharacterBounds( aEntryCharacterRange.A() + _nCharacterIndex );
    }

    return aRect;
}

void SvtIconChoiceCtrl::SetNoSelection()
{
    _pImp->SetNoSelection();
}

void SvtIconChoiceCtrl::CallImplEventListeners(sal_uLong nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}
::com::sun::star::uno::Reference< XAccessible > SvtIconChoiceCtrl::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    ::com::sun::star::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xTemp(GetComponentInterface());
            xAccessible = _pImp->GetAccessibleFactory().createAccessibleIconChoiceCtrl( *this, xAccParent );
        }
    }
    return xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
