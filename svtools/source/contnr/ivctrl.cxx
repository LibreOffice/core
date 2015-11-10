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


#include <svtools/ivctrl.hxx>
#include "imivctl.hxx"
#include <vcl/bitmapex.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;

/*****************************************************************************
|
| class : SvxIconChoiceCtrlEntry
|
\*****************************************************************************/

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( const OUString& rText,
                                                const Image& rImage,
                                                SvxIconViewFlags _nFlags )
    : aImage(rImage)
    , aText(rText)
    , pUserData(nullptr)
    , nPos(0)
    , pblink(nullptr)
    , pflink(nullptr)
    , eTextMode(IcnShowTextShort)
    , nX(0)
    , nY(0)
    , nFlags(_nFlags)
{
}

OUString SvxIconChoiceCtrlEntry::GetDisplayText() const
{
    return MnemonicGenerator::EraseAllMnemonicChars( aText );
}



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

SvtIconChoiceCtrl::SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle ) :

     // WB_CLIPCHILDREN on, as ScrollBars lie on the window!
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),

    _pCurKeyEvent   ( nullptr ),
    _pImp           ( new SvxIconChoiceCtrl_Impl( this, nWinStyle ) ),
    _bAutoFontColor ( false )

{
    SetLineColor();
    _pImp->SetGrid( Size( 100, 70 ) );
    _pImp->InitSettings();
    _pImp->SetPositionMode( IcnViewPositionModeAutoArrange );
}

SvtIconChoiceCtrl::~SvtIconChoiceCtrl()
{
    disposeOnce();
}

void SvtIconChoiceCtrl::dispose()
{
    if (_pImp)
    {
        _pImp->CallEventListeners( VCLEVENT_OBJECT_DYING );
        delete _pImp;
        _pImp = nullptr;
    }
    Control::dispose();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const OUString& rText, const Image& rImage, sal_uLong nPos, const Point* pPos, SvxIconViewFlags nFlags  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage, nFlags);

    _pImp->InsertEntry( pEntry, nPos, pPos );

    return pEntry;
}

void SvtIconChoiceCtrl::DrawEntryImage( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, OutputDevice& rDev )
{
    rDev.DrawImage( rPos, pEntry->GetImage() );
}

OUString SvtIconChoiceCtrl::GetEntryText( SvxIconChoiceCtrlEntry* pEntry, bool )
{
    return pEntry->GetText();
}

void SvtIconChoiceCtrl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    _pImp->Paint(rRenderContext, rRect);
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

        for ( sal_Int32 i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setWidth ( aFullSize.getWidth()+aEntryRect.GetWidth() );
        }

        _pImp->Arrange ( false, aFullSize.getWidth() );
    }
    else if ( GetStyle() & WB_ALIGN_LEFT )
    {
        Size aFullSize;
        Rectangle aEntryRect;

        for ( sal_Int32 i = 0; i < GetEntryCount(); i++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
            aEntryRect = _pImp->GetEntryBoundRect ( pEntry );

            aFullSize.setHeight ( aFullSize.getHeight()+aEntryRect.GetHeight() );
        }

        _pImp->Arrange ( false, 0, aFullSize.getHeight() );
    }
    else
    {
        _pImp->Arrange();
    }
    _pImp->Arrange( false, 0, 1000 );
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

void SvtIconChoiceCtrl::GetFocus()
{
    _pImp->GetFocus();
    Control::GetFocus();
    SvxIconChoiceCtrlEntry* pSelectedEntry = GetSelectedEntry();
    if ( pSelectedEntry )
        _pImp->CallEventListeners( VCLEVENT_LISTBOX_SELECT, pSelectedEntry );
}

void SvtIconChoiceCtrl::LoseFocus()
{
    if (_pImp)
        _pImp->LoseFocus();
    Control::LoseFocus();
}

void SvtIconChoiceCtrl::SetFont(const vcl::Font& rFont)
{
    if (rFont != GetFont())
    {
        Control::SetFont(rFont);
        _pImp->FontModified();
    }
}

void SvtIconChoiceCtrl::SetPointFont(const vcl::Font& rFont)
{
    if (rFont != GetPointFont(*this)) //FIXME
    {
        Control::SetPointFont(*this, rFont); //FIXME
        _pImp->FontModified();
    }
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( const Point& rPixPos, bool bHit ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return const_cast<SvtIconChoiceCtrl*>(this)->_pImp->GetEntry( aPos, bHit );
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

#ifdef DBG_UTIL
void SvtIconChoiceCtrl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    _pImp->SetEntryTextMode( eMode, pEntry );
}
#endif

sal_Int32 SvtIconChoiceCtrl::GetEntryCount() const
{
    return _pImp ? _pImp->GetEntryCount() : 0;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( sal_Int32 nPos ) const
{
    return _pImp ? _pImp->GetEntry( nPos ) : nullptr;
}

void SvtIconChoiceCtrl::CreateAutoMnemonics( MnemonicGenerator& _rUsedMnemonics )
{
    _pImp->CreateAutoMnemonics( &_rUsedMnemonics );
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetSelectedEntry() const
{
    return _pImp ? _pImp->GetFirstSelectedEntry() : nullptr;
}

void SvtIconChoiceCtrl::ClickIcon()
{
    GetSelectedEntry();
    _aClickIconHdl.Call( this );
}
bool SvtIconChoiceCtrl::IsEntryEditing() const
{
    return _pImp->IsEntryEditing();
}

bool SvtIconChoiceCtrl::SetChoiceWithCursor ( bool bDo )
{
    return _pImp->SetChoiceWithCursor (bDo);
}

void SvtIconChoiceCtrl::KeyInput( const KeyEvent& rKEvt )
{
    bool bKeyUsed = DoKeyInput( rKEvt );
    if ( !bKeyUsed )
    {
        _pCurKeyEvent = const_cast<KeyEvent*>(&rKEvt);
        Control::KeyInput( rKEvt );
        _pCurKeyEvent = nullptr;
    }
}
bool SvtIconChoiceCtrl::DoKeyInput( const KeyEvent& rKEvt )
{
    // under OS/2, we get key up/down even while editing
    if( IsEntryEditing() )
        return true;
    _pCurKeyEvent = const_cast<KeyEvent*>(&rKEvt);
    bool bHandled = _pImp->KeyInput( rKEvt );
    _pCurKeyEvent = nullptr;
    return bHandled;
}
sal_Int32 SvtIconChoiceCtrl::GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const
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
    if ( ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        _pImp->InitSettings();
        Invalidate(InvalidateFlags::NoChildren);
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
                aBackground.SetStyle( WallpaperStyle::Tile );

            WallpaperStyle eStyle = aBackground.GetStyle();
            Color aBack( aBackground.GetColor());
            Color aTrans( COL_TRANSPARENT );
            if( aBack == aTrans  && (
                (!aBackground.IsBitmap() ||
                    aBackground.GetBitmap().IsTransparent() ||
                    (eStyle != WallpaperStyle::Tile && eStyle != WallpaperStyle::Scale))))
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
        vcl::Font aFont( GetFont() );
        aFont.SetColor( rStyleSettings.GetFieldTextColor() );
        SetFont( aFont );

        Invalidate(InvalidateFlags::NoChildren);
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
    const_cast<SvtIconChoiceCtrl*>(this)->Invalidate();
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
css::uno::Reference< XAccessible > SvtIconChoiceCtrl::CreateAccessible()
{
    vcl::Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    css::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        css::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            css::uno::Reference< css::awt::XWindowPeer > xTemp(GetComponentInterface());
            xAccessible = _pImp->GetAccessibleFactory().createAccessibleIconChoiceCtrl( *this, xAccParent );
        }
    }
    return xAccessible;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
