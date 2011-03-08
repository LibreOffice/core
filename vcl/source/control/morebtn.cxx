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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/morebtn.hxx>

#include <tools/rc.h>
#include <vector>

// =======================================================================

typedef ::std::vector< Window* > ImplMoreWindowList;

struct ImplMoreButtonData
{
    ImplMoreWindowList *mpItemList;
    XubString           maMoreText;
    XubString           maLessText;
};

// =======================================================================

void MoreButton::ImplInit( Window* pParent, WinBits nStyle )
{
    mpMBData     = new ImplMoreButtonData;
    mnDelta      = 0;
    meUnit       = MAP_PIXEL;
    mbState      = FALSE;

    mpMBData->mpItemList = NULL;

    PushButton::ImplInit( pParent, nStyle );

    mpMBData->maMoreText = Button::GetStandardText( BUTTON_MORE );
    mpMBData->maLessText = Button::GetStandardText( BUTTON_LESS );

    SetHelpText( Button::GetStandardHelpText( BUTTON_MORE ) );

    ShowState();

    SetSymbolAlign( SYMBOLALIGN_RIGHT );
    ImplSetSmallSymbol( TRUE );

    if ( ! ( nStyle & ( WB_RIGHT | WB_LEFT ) ) )
    {
        nStyle |= WB_CENTER;
        SetStyle( nStyle );
    }
}

// -----------------------------------------------------------------------
void MoreButton::ShowState()
{
    if ( mbState )
    {
        SetSymbol( SYMBOL_PAGEUP );
        SetText( mpMBData->maLessText );
    }
    else
    {
        SetSymbol( SYMBOL_PAGEDOWN );
        SetText( mpMBData->maMoreText );
    }
}

// -----------------------------------------------------------------------

MoreButton::MoreButton( Window* pParent, WinBits nStyle ) :
    PushButton( WINDOW_MOREBUTTON )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

MoreButton::MoreButton( Window* pParent, const ResId& rResId ) :
    PushButton( WINDOW_MOREBUTTON )
{
    rResId.SetRT( RSC_MOREBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void MoreButton::ImplLoadRes( const ResId& rResId )
{
    PushButton::ImplLoadRes( rResId );

    ULONG nObjMask = ReadLongRes();

    if ( nObjMask & RSC_MOREBUTTON_STATE )
    {
        // Nicht Methode rufen, da Dialog nicht umgeschaltet werden soll
        mbState = (BOOL)ReadShortRes();
        // SetText( GetText() );
        ShowState();
    }
    if ( nObjMask & RSC_MOREBUTTON_MAPUNIT )
        meUnit = (MapUnit)ReadLongRes();
    if ( nObjMask & RSC_MOREBUTTON_DELTA )
        // Groesse fuer Erweitern des Dialogs
        mnDelta = ReadShortRes();
}

// -----------------------------------------------------------------------

MoreButton::~MoreButton()
{
    if ( mpMBData->mpItemList )
        delete mpMBData->mpItemList;
    delete mpMBData;
}

// -----------------------------------------------------------------------

void MoreButton::Click()
{
    Window*     pParent = GetParent();
    Size        aSize( pParent->GetSizePixel() );
    long        nDeltaPixel = LogicToPixel( Size( 0, mnDelta ), meUnit ).Height();

    // Status aendern
    mbState = !mbState;
    ShowState();

    // Hier den Click-Handler rufen, damit vorher die Controls initialisiert
    // werden koennen
    PushButton::Click();

    // Je nach Status die Fenster updaten
    if ( mbState )
    {
        // Fenster anzeigen
        if ( mpMBData->mpItemList ) {
            for ( size_t i = 0, n = mpMBData->mpItemList->size(); i < n; ++i ) {
                (*mpMBData->mpItemList)[ i ]->Show();
            }
        }

        // Dialogbox anpassen
        Point aPos( pParent->GetPosPixel() );
        Rectangle aDeskRect( pParent->ImplGetFrameWindow()->GetDesktopRectPixel() );

        aSize.Height() += nDeltaPixel;
        if ( (aPos.Y()+aSize.Height()) > aDeskRect.Bottom() )
        {
            aPos.Y() = aDeskRect.Bottom()-aSize.Height();

            if ( aPos.Y() < aDeskRect.Top() )
                aPos.Y() = aDeskRect.Top();

            pParent->SetPosSizePixel( aPos, aSize );
        }
        else
            pParent->SetSizePixel( aSize );
    }
    else
    {
        // Dialogbox anpassen
        aSize.Height() -= nDeltaPixel;
        pParent->SetSizePixel( aSize );

        // Fenster nicht mehr anzeigen
        if ( mpMBData->mpItemList ) {
            for ( size_t i = 0, n = mpMBData->mpItemList->size(); i < n; ++i ) {
                (*mpMBData->mpItemList)[ i ]->Hide();
            }
        }
    }
}

// -----------------------------------------------------------------------

void MoreButton::AddWindow( Window* pWindow )
{
    if ( !mpMBData->mpItemList )
        mpMBData->mpItemList = new ImplMoreWindowList();

    mpMBData->mpItemList->push_back( pWindow );

    if ( mbState )
        pWindow->Show();
    else
        pWindow->Hide();
}

// -----------------------------------------------------------------------

void MoreButton::RemoveWindow( Window* pWindow )
{
    if ( mpMBData->mpItemList ) {
        for ( ImplMoreWindowList::iterator it = mpMBData->mpItemList->begin();
              it < mpMBData->mpItemList->end();
              ++it
        ) {
            if ( *it == pWindow ) {
                mpMBData->mpItemList->erase( it );
                break;
            }
        }
    }
}

// -----------------------------------------------------------------------

void MoreButton::SetText( const XubString& rText )
{
    PushButton::SetText( rText );
}

// -----------------------------------------------------------------------

XubString MoreButton::GetText() const
{
    return PushButton::GetText();
}

// -----------------------------------------------------------------------
void MoreButton::SetMoreText( const XubString& rText )
{
    if ( mpMBData )
        mpMBData->maMoreText = rText;

    if ( !mbState )
        SetText( rText );
}

// -----------------------------------------------------------------------
XubString MoreButton::GetMoreText() const
{
    if ( mpMBData )
        return mpMBData->maMoreText;
    else
        return PushButton::GetText();
}

// -----------------------------------------------------------------------
void MoreButton::SetLessText( const XubString& rText )
{
    if ( mpMBData )
        mpMBData->maLessText = rText;

    if ( mbState )
        SetText( rText );
}

// -----------------------------------------------------------------------
XubString MoreButton::GetLessText() const
{
    if ( mpMBData )
        return mpMBData->maLessText;
    else
        return PushButton::GetText();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
