/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fixbrd.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:52:53 $
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

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_FIXBRD_HXX
#include <fixbrd.hxx>
#endif



// =======================================================================

void FixedBorder::ImplInit( Window* pParent, WinBits nStyle )
{
    mnType          = FIXEDBORDER_TYPE_DOUBLEOUT;
    mbTransparent   = TRUE;

    nStyle = ImplInitStyle( nStyle );
    Control::ImplInit( pParent, nStyle, NULL );
    ImplInitSettings();
}

// -----------------------------------------------------------------------

WinBits FixedBorder::ImplInitStyle( WinBits nStyle )
{
    if ( !(nStyle & WB_NOGROUP) )
        nStyle |= WB_GROUP;
    return nStyle;
}

// -----------------------------------------------------------------------

void FixedBorder::ImplInitSettings()
{
    Window* pParent = GetParent();
    if ( (pParent->IsChildTransparentModeEnabled() ||
          !(pParent->GetStyle() & WB_CLIPCHILDREN) ) &&
         !IsControlBackground() && mbTransparent )
    {
        SetMouseTransparent( TRUE );
        EnableChildTransparentMode( TRUE );
        SetParentClipMode( PARENTCLIPMODE_NOCLIP );
        SetPaintTransparent( TRUE );
        SetBackground();
    }
    else
    {
        SetMouseTransparent( FALSE );
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

FixedBorder::FixedBorder( Window* pParent, WinBits nStyle ) :
    Control( WINDOW_FIXEDBORDER )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

FixedBorder::FixedBorder( Window* pParent, const ResId& rResId ) :
    Control( WINDOW_FIXEDBORDER )
{
    rResId.SetRT( RSC_CONTROL );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

FixedBorder::~FixedBorder()
{
}

// -----------------------------------------------------------------------

void FixedBorder::ImplDraw( OutputDevice* pDev, ULONG nDrawFlags,
                            const Point& rPos, const Size& rSize )
{
    const StyleSettings&    rStyleSettings = GetSettings().GetStyleSettings();
    Rectangle               aRect( rPos, rSize );
    USHORT                  nBorderStyle = mnType;

    if ( (nDrawFlags & WINDOW_DRAW_MONO) ||
         (rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
        nBorderStyle |= FRAME_DRAW_MONO;

    /*
    // seems only to be used in tools->options around a tabpage (ie, no tabcontrol!)
    // as tabpages that are not embedded in a tabcontrol should not be drawn natively
    // the fixedborder must also not be drawn (reason was, that it looks too ugly, dialogs must be redesigned)
    Window *pWin = pDev->GetOutDevType() == OUTDEV_WINDOW ? (Window*) pDev : NULL;
    if( !(nBorderStyle & FRAME_DRAW_MONO) && pWin && pWin->IsNativeControlSupported( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue aControlValue;
        Point aPt;
        Region aCtrlRegion( Rectangle( aPt, GetOutputSizePixel() ) );
        ControlState nState = IsEnabled() ? CTRL_STATE_ENABLED : 0;
        pWin->DrawNativeControl( CTRL_FIXEDBORDER, PART_ENTIRE_CONTROL, aCtrlRegion, nState,
                                 aControlValue, rtl::OUString() );
    }
    else
    */
    {
        DecorationView  aDecoView( pDev );
        aDecoView.DrawFrame( aRect, nBorderStyle );
    }
}

// -----------------------------------------------------------------------

void FixedBorder::Paint( const Rectangle& )
{
    ImplDraw( this, 0, Point(), GetOutputSizePixel() );
}

// -----------------------------------------------------------------------

void FixedBorder::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize,
                        ULONG nFlags )
{
    Point   aPos  = pDev->LogicToPixel( rPos );
    Size    aSize = pDev->LogicToPixel( rSize );

    pDev->Push();
    pDev->SetMapMode();
    ImplDraw( pDev, nFlags, aPos, aSize );
    pDev->Pop();
}

// -----------------------------------------------------------------------

void FixedBorder::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void FixedBorder::StateChanged( StateChangedType nType )
{
    Control::StateChanged( nType );

    if ( (nType == STATE_CHANGE_DATA) ||
         (nType == STATE_CHANGE_UPDATEMODE) )
    {
        if ( IsUpdateMode() )
            Invalidate();
    }
    else if ( nType == STATE_CHANGE_STYLE )
        SetStyle( ImplInitStyle( GetStyle() ) );
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::SetTransparent( BOOL bTransparent )
{
    if ( mbTransparent != bTransparent )
    {
        mbTransparent = bTransparent;
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void FixedBorder::SetBorderType( USHORT nType )
{
    if ( mnType != nType )
    {
        mnType = nType;
        Invalidate();
    }
}
