/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: morebtn.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:54:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_MOREBTN_HXX
#include <morebtn.hxx>
#endif

#ifndef _SV_RD_H
#include <tools/rc.h>
#endif



// =======================================================================

DECLARE_LIST( ImplMoreWindowList, Window* )

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
    Window*     pWindow = (mpMBData->mpItemList) ? mpMBData->mpItemList->First() : NULL;
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
        while ( pWindow )
        {
            pWindow->Show();
            pWindow = mpMBData->mpItemList->Next();
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
        while ( pWindow )
        {
            pWindow->Hide();
            pWindow = mpMBData->mpItemList->Next();
        }
    }
}

// -----------------------------------------------------------------------

void MoreButton::AddWindow( Window* pWindow )
{
    if ( !mpMBData->mpItemList )
        mpMBData->mpItemList = new ImplMoreWindowList( 1024, 16, 16 );

    mpMBData->mpItemList->Insert( pWindow, LIST_APPEND );

    if ( mbState )
        pWindow->Show();
    else
        pWindow->Hide();
}

// -----------------------------------------------------------------------

void MoreButton::RemoveWindow( Window* pWindow )
{
    if ( mpMBData->mpItemList )
        mpMBData->mpItemList->Remove( pWindow );
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

