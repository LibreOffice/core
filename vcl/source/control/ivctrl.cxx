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


#include <utility>
#include <vcl/toolkit/ivctrl.hxx>
#include "imivctl.hxx"
#include <vcl/bitmap.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <accessibility/accessibleiconchoicectrl.hxx>

using namespace ::com::sun::star::accessibility;

SvxIconChoiceCtrlEntry::SvxIconChoiceCtrlEntry( OUString _aText,
                                                Image _aImage )
    : aImage(std::move(_aImage))
    , aText(std::move(_aText))
    , nX(0)
    , nY(0)
    , nFlags(SvxIconViewFlags::NONE)
{
}

OUString SvxIconChoiceCtrlEntry::GetDisplayText() const
{
    return MnemonicGenerator::EraseAllMnemonicChars( aText );
}

SvtIconChoiceCtrl::SvtIconChoiceCtrl( vcl::Window* pParent, WinBits nWinStyle ) :

     // WB_CLIPCHILDREN on, as ScrollBars lie on the window!
    Control( pParent, nWinStyle | WB_CLIPCHILDREN ),

    _pImpl           ( new SvxIconChoiceCtrl_Impl( this, nWinStyle ) ),
    m_nWidth(-1)
{
    GetOutDev()->SetLineColor();
    _pImpl->InitSettings();
}

SvtIconChoiceCtrl::~SvtIconChoiceCtrl()
{
    disposeOnce();
}

void SvtIconChoiceCtrl::dispose()
{
    if (_pImpl)
    {
        _pImpl->CallEventListeners( VclEventId::ObjectDying, nullptr );
        _pImpl.reset();
    }
    Control::dispose();
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::InsertEntry( const OUString& rText, const Image& rImage  )
{
    SvxIconChoiceCtrlEntry* pEntry = new SvxIconChoiceCtrlEntry( rText, rImage);

    _pImpl->InsertEntry(std::unique_ptr<SvxIconChoiceCtrlEntry>(pEntry), _pImpl->GetEntryCount());

    return pEntry;
}

void SvtIconChoiceCtrl::RemoveEntry(sal_Int32 nIndex)
{
    _pImpl->RemoveEntry(nIndex);
}

void SvtIconChoiceCtrl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    _pImpl->Paint(rRenderContext, rRect);
}

void SvtIconChoiceCtrl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( !_pImpl->MouseButtonDown( rMEvt ) )
        Control::MouseButtonDown( rMEvt );
}

void SvtIconChoiceCtrl::MouseMove( const MouseEvent& rMEvt )
{
    if( !_pImpl->MouseMove( rMEvt ) )
        Control::MouseMove( rMEvt );
}
void SvtIconChoiceCtrl::ArrangeIcons()
{
    Size aFullSize;
    tools::Rectangle aEntryRect;

    for ( sal_Int32 i = 0; i < GetEntryCount(); i++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry ( i );
        aEntryRect = _pImpl->GetEntryBoundRect ( pEntry );

        aFullSize.setHeight ( aFullSize.getHeight()+aEntryRect.GetHeight() );
    }

    _pImpl->Arrange(aFullSize.getHeight());

    _pImpl->Arrange(1000);
}

long SvtIconChoiceCtrl::AdjustWidth(const long nWidth)
{
    const long cMargin = 9;

    if (nWidth + cMargin > m_nWidth)
    {
        m_nWidth = nWidth + cMargin;
        this->set_width_request(m_nWidth);
        _pImpl->SetGrid(Size(m_nWidth, 32));
    }
    return m_nWidth - cMargin;
}

void SvtIconChoiceCtrl::Resize()
{
    _pImpl->Resize();
    Control::Resize();
}

void SvtIconChoiceCtrl::GetFocus()
{
    _pImpl->GetFocus();
    Control::GetFocus();
}

void SvtIconChoiceCtrl::LoseFocus()
{
    if (_pImpl)
        _pImpl->LoseFocus();
    Control::LoseFocus();
}

void SvtIconChoiceCtrl::SetFont(const vcl::Font& rFont)
{
    if (rFont != GetFont())
    {
        Control::SetFont(rFont);
        _pImpl->FontModified();
    }
}

void SvtIconChoiceCtrl::SetPointFont(const vcl::Font& rFont)
{
    if (rFont != GetPointFont(*GetOutDev())) //FIXME
    {
        Control::SetPointFont(*GetOutDev(), rFont); //FIXME
        _pImpl->FontModified();
    }
}

void SvtIconChoiceCtrl::Command(const CommandEvent& rCEvt)
{
    _pImpl->Command( rCEvt );
    //pass at least alt press/release to parent impl
    if (rCEvt.GetCommand() == CommandEventId::ModKeyChange)
        Control::Command(rCEvt);
}

sal_Int32 SvtIconChoiceCtrl::GetEntryCount() const
{
    return _pImpl ? _pImpl->GetEntryCount() : 0;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetEntry( sal_Int32 nPos ) const
{
    return _pImpl ? _pImpl->GetEntry( nPos ) : nullptr;
}

SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetSelectedEntry() const
{
    return _pImpl ? _pImpl->GetFirstSelectedEntry() : nullptr;
}

void SvtIconChoiceCtrl::ClickIcon()
{
    GetSelectedEntry();
    _aClickIconHdl.Call( this );
}

void SvtIconChoiceCtrl::KeyInput( const KeyEvent& rKEvt )
{
    bool bKeyUsed = DoKeyInput( rKEvt );
    if ( !bKeyUsed )
    {
        Control::KeyInput( rKEvt );
    }
}
bool SvtIconChoiceCtrl::DoKeyInput( const KeyEvent& rKEvt )
{
    return _pImpl->KeyInput( rKEvt );
}
sal_Int32 SvtIconChoiceCtrl::GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const
{
    return _pImpl->GetEntryListPos( pEntry );
}
SvxIconChoiceCtrlEntry* SvtIconChoiceCtrl::GetCursor( ) const
{
    return _pImpl->GetCurEntry( );
}
void SvtIconChoiceCtrl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    _pImpl->SetCursor( pEntry );
}

void SvtIconChoiceCtrl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTS) ) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        _pImpl->InitSettings();
        Invalidate(InvalidateFlags::NoChildren);
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvtIconChoiceCtrl::SetBackground( const Wallpaper& rPaper )
{
    if( rPaper == GetBackground() )
        return;

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    // if it is the default (empty) wallpaper
    if (rPaper.IsEmpty())
    {
        Control::SetBackground( rStyleSettings.GetFieldColor() );
    }
    else
    {
        Wallpaper aBackground( rPaper );
        // HACK, as background might be transparent!
        if( !aBackground.IsBitmap() )
            aBackground.SetStyle( WallpaperStyle::Tile );

        WallpaperStyle eStyle = aBackground.GetStyle();
        Color aBack( aBackground.GetColor());
        if( aBack == COL_TRANSPARENT &&
            (!aBackground.IsBitmap() ||
             aBackground.GetBitmap().HasAlpha() ||
             (eStyle != WallpaperStyle::Tile && eStyle != WallpaperStyle::Scale)) )
        {
            aBackground.SetColor( rStyleSettings.GetFieldColor() );
        }
        if( aBackground.IsScrollable() )
        {
            tools::Rectangle aRect;
            aRect.SetSize( Size(32765, 32765) );
            aBackground.SetRect( aRect );
        }
        else
        {
            tools::Rectangle aRect( _pImpl->GetOutputRect() );
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

void SvtIconChoiceCtrl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !_pImpl->RequestHelp( rHEvt ) )
        Control::RequestHelp( rHEvt );
}

tools::Rectangle SvtIconChoiceCtrl::GetBoundingBox( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return _pImpl->GetEntryBoundRect( pEntry );
}

void SvtIconChoiceCtrl::FillLayoutData() const
{
    CreateLayoutData();
    const_cast<SvtIconChoiceCtrl*>(this)->Invalidate();
}

tools::Rectangle SvtIconChoiceCtrl::GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const
{
    tools::Rectangle aRect;

    Pair aEntryCharacterRange = GetLineStartEnd( _nEntryPos );
    if ( aEntryCharacterRange.A() + _nCharacterIndex < aEntryCharacterRange.B() )
    {
        aRect = GetCharacterBounds( aEntryCharacterRange.A() + _nCharacterIndex );
    }

    return aRect;
}

void SvtIconChoiceCtrl::CallImplEventListeners(VclEventId nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}

rtl::Reference<comphelper::OAccessible> SvtIconChoiceCtrl::CreateAccessible()
{
    return new AccessibleIconChoiceCtrl(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
