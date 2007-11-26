/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabpage.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 16:42:46 $
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

#include <tools/ref.hxx>

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <vcl/svdata.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_TABCTRL_HXX
#include <vcl/tabctrl.hxx>
#endif
#include <vcl/bitmapex.hxx>

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif




// =======================================================================

void TabPage::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Window::ImplInit( pParent, nStyle, NULL );

    ImplInitSettings();

    // if the tabpage is drawn (ie filled) by a native widget, make sure all contols will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(CTRL_TAB_BODY, PART_ENTIRE_CONTROL) && GetParent() && (GetParent()->GetType() == WINDOW_TABCONTROL) )
        EnableChildTransparentMode( TRUE );
}

// -----------------------------------------------------------------------

void TabPage::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
    {
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( FALSE );
        SetParentClipMode( 0 );
        SetPaintTransparent( FALSE );

        if ( IsControlBackground() )
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

// -----------------------------------------------------------------------

TabPage::TabPage( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_TABPAGE )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

TabPage::TabPage( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_TABPAGE )
{
    rResId.SetRT( RSC_TABPAGE );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

void TabPage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( GetSettings().GetStyleSettings().GetAutoMnemonic() )
            ImplWindowAutoMnemonic( this );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void TabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void TabPage::Paint( const Rectangle& )
{
    // draw native tabpage only inside tabcontrols, standalone tabpages look ugly (due to bad dialog design)
    if( IsNativeControlSupported(CTRL_TAB_BODY, PART_ENTIRE_CONTROL) && GetParent() && (GetParent()->GetType() == WINDOW_TABCONTROL) )
    {
        const ImplControlValue aControlValue( BUTTONVALUE_DONTKNOW, rtl::OUString(), 0 );

        ControlState nState = CTRL_STATE_ENABLED;
        int part = PART_ENTIRE_CONTROL;
        if ( !IsEnabled() )
            nState &= ~CTRL_STATE_ENABLED;
        if ( HasFocus() )
            nState |= CTRL_STATE_FOCUSED;
        Point aPoint;
        // pass the whole window region to NWF as the tab body might be a gradient or bitmap
        // that has to be scaled properly, clipping makes sure that we do not paint too much
        Region aCtrlRegion( Rectangle( aPoint, GetOutputSizePixel() ) );
        DrawNativeControl( CTRL_TAB_BODY, part, aCtrlRegion, nState,
                aControlValue, rtl::OUString() );
    }
}

// -----------------------------------------------------------------------
void TabPage::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG )
{
    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );

    Wallpaper aWallpaper = GetBackground();
    if ( !aWallpaper.IsBitmap() )
        ImplInitSettings();

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetLineColor();

    if ( aWallpaper.IsBitmap() )
        pDev->DrawBitmapEx( aPos, aSize, aWallpaper.GetBitmap() );
    else
    {
        if( aWallpaper.GetColor() == COL_AUTO )
            pDev->SetFillColor( GetSettings().GetStyleSettings().GetDialogColor() );
        else
            pDev->SetFillColor( aWallpaper.GetColor() );
        pDev->DrawRect( Rectangle( aPos, aSize ) );
    }

    pDev->Pop();
}

// -----------------------------------------------------------------------

void TabPage::ActivatePage()
{
}

// -----------------------------------------------------------------------

void TabPage::DeactivatePage()
{
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > TabPage::CreateAccessible()
{
    // TODO: remove this method (incompatible)

    return Window::CreateAccessible();
}
