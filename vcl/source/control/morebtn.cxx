/*************************************************************************
 *
 *  $RCSfile: morebtn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_MOREBTN_CXX

#ifndef _SV_MOREBTN_HXX
#include <morebtn.hxx>
#endif

#ifndef _SV_RD_H
#include <rc.h>
#endif

#pragma hdrstop

// =======================================================================

// Muss mit der Laenge der folgenden Texte uebereinstimmen
#define EXTRA_TEXTLEN       3

static sal_Char const aImplMoreOpen[]  = " >>";
static sal_Char const aImplMoreClose[] = " <<";

DECLARE_LIST( ImplMoreWindowList, Window* );

// =======================================================================

void MoreButton::ImplInit( Window* pParent, WinBits nStyle )
{
    mpItemList   = NULL;
    mnDelta      = 0;
    meUnit       = MAP_PIXEL;
    mbState      = FALSE;

    PushButton::ImplInit( pParent, nStyle );

    SetText( Button::GetStandardText( BUTTON_MORE ) );
    SetHelpText( Button::GetStandardHelpText( BUTTON_MORE ) );
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

    USHORT nObjMask = ReadShortRes();

    if ( nObjMask & RSC_MOREBUTTON_STATE )
    {
        // Nicht Methode rufen, da Dialog nicht umgeschaltet werden soll
        mbState = (BOOL)ReadShortRes();
        SetText( GetText() );
    }
    if ( nObjMask & RSC_MOREBUTTON_MAPUNIT )
        meUnit = (MapUnit)ReadShortRes();
    if ( nObjMask & RSC_MOREBUTTON_DELTA )
        // Groesse fuer Erweitern des Dialogs
        mnDelta = ReadShortRes();
}

// -----------------------------------------------------------------------

MoreButton::~MoreButton()
{
    if ( mpItemList )
        delete mpItemList;
}

// -----------------------------------------------------------------------

void MoreButton::Click()
{
    Window*     pParent = GetParent();
    Size        aSize( pParent->GetSizePixel() );
    Window*     pWindow = (mpItemList) ? mpItemList->First() : NULL;
    long        nDeltaPixel = LogicToPixel( Size( 0, mnDelta ), meUnit ).Height();

    // Status aendern
    XubString aText = GetText();
    mbState = !mbState;
    SetText( aText );

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
            pWindow = mpItemList->Next();
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
            pWindow = mpItemList->Next();
        }
    }
}

// -----------------------------------------------------------------------

void MoreButton::AddWindow( Window* pWindow )
{
    if ( !mpItemList )
        mpItemList = new ImplMoreWindowList( 1024, 16, 16 );

    mpItemList->Insert( pWindow, LIST_APPEND );

    if ( mbState )
        pWindow->Show();
    else
        pWindow->Hide();
}

// -----------------------------------------------------------------------

void MoreButton::RemoveWindow( Window* pWindow )
{
    if ( mpItemList )
        mpItemList->Remove( pWindow );
}

// -----------------------------------------------------------------------

void MoreButton::SetText( const XubString& rText )
{
    XubString aText = rText;

    if ( !mbState )
        aText.AppendAscii( aImplMoreOpen );
    else
        aText.AppendAscii( aImplMoreClose );

    PushButton::SetText( aText );
}

// -----------------------------------------------------------------------

XubString MoreButton::GetText() const
{
    XubString aText = PushButton::GetText();

    XubString aSubText = aText.Copy( aText.Len()-EXTRA_TEXTLEN, EXTRA_TEXTLEN );
    if ( !mbState )
    {
        if ( aSubText.EqualsAscii( aImplMoreOpen ) )
            aText.Erase( aText.Len()-EXTRA_TEXTLEN, EXTRA_TEXTLEN );
    }
    else
    {
        if ( aSubText.EqualsAscii( aImplMoreClose ) )
            aText.Erase( aText.Len()-EXTRA_TEXTLEN, EXTRA_TEXTLEN );
    }

    return aText;
}
