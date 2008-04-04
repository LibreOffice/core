/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: window3.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 11:02:34 $
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

#define _SV_WINDOW_CXX

#include "svsys.h"
#include "vcl/window.h"
#include "vcl/window.hxx"
#include "vcl/waitobj.hxx"
#include "vcl/salgdi.hxx"
#include "vcl/salnativewidgets.hxx"
#include "vcl/salctrlhandle.hxx"
#include "rtl/ustring.hxx"
#include "vcl/button.hxx"

using namespace rtl;

// -----------------------------------------------------------------------
// These functions are mainly passthrough functions that allow access to
// the SalFrame behind a Window object for native widget rendering purposes.
// -----------------------------------------------------------------------

void Window::ImplInitSalControlHandle()
{
    // create SalControlHandle on demand
    // not needed for ordinary windows
    // TODO: move creation to SalGraphics
    //if( !ImplGetWinData()->mpSalControlHandle )
    //    ImplGetWinData()->mpSalControlHandle = new SalControlHandle;
}

// -----------------------------------------------------------------------

BOOL Window::IsNativeControlSupported( ControlType nType, ControlPart nPart )
{
    if( !IsNativeWidgetEnabled() )
        return FALSE;

    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return FALSE;

    return( mpGraphics->IsNativeControlSupported(nType, nPart) );
}


// -----------------------------------------------------------------------

BOOL Window::HitTestNativeControl( ControlType nType,
                              ControlPart nPart,
                              const Region& rControlRegion,
                              const Point& aPos,
                              BOOL& rIsInside )
{
    if( !IsNativeWidgetEnabled() )
        return FALSE;

    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return FALSE;

    ImplInitSalControlHandle();

    Point aWinOffs;
    aWinOffs = OutputToScreenPixel( aWinOffs );
    Region screenRegion( rControlRegion );
    screenRegion.Move( aWinOffs.X(), aWinOffs.Y());

    return( mpGraphics->HitTestNativeControl(nType, nPart, screenRegion, OutputToScreenPixel( aPos ), *ImplGetWinData()->mpSalControlHandle, rIsInside, this ) );
}

// -----------------------------------------------------------------------

void Window::ImplMoveControlValue( ControlType nType, const ImplControlValue& aValue, const Point& rDelta ) const
{
    if( aValue.getOptionalVal() )
    {
        switch( nType )
        {
            case CTRL_SCROLLBAR:
            {
                ScrollbarValue* pScVal = reinterpret_cast<ScrollbarValue*>(aValue.getOptionalVal());
                pScVal->maThumbRect.Move( rDelta.X(), rDelta.Y() );
                pScVal->maButton1Rect.Move( rDelta.X(), rDelta.Y() );
                pScVal->maButton2Rect.Move( rDelta.X(), rDelta.Y() );
            }
            break;
            case CTRL_SPINBOX:
            case CTRL_SPINBUTTONS:
            {
                SpinbuttonValue* pSpVal = reinterpret_cast<SpinbuttonValue*>(aValue.getOptionalVal());
                pSpVal->maUpperRect.Move( rDelta.X(), rDelta.Y() );
                pSpVal->maLowerRect.Move( rDelta.X(), rDelta.Y() );
            }
            break;
            case CTRL_TOOLBAR:
            {
                ToolbarValue* pTVal = reinterpret_cast<ToolbarValue*>(aValue.getOptionalVal());
                pTVal->maGripRect.Move( rDelta.X(), rDelta.Y() );
            }
            break;
        }
    }
}

BOOL Window::DrawNativeControl( ControlType nType,
                            ControlPart nPart,
                            const Region& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            OUString aCaption )
{
    if( !IsNativeWidgetEnabled() )
        return FALSE;

    /*
    if( !IsInPaint() && IsPaintTransparent() )
    {
        // only required if called directly (ie, we're not in Paint() ):
        // force redraw (Paint()) for transparent controls
        // to trigger a repaint of the background
        Region aClipRgn( GetClipRegion() );
        if( !rControlRegion.IsEmpty() )
            aClipRgn.Intersect( rControlRegion );
        Invalidate( aClipRgn, INVALIDATE_UPDATE );
        return TRUE;
    }
    */

    ImplInitSalControlHandle();

    // make sure the current clip region is initialized correctly
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return FALSE;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return TRUE;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // Convert the coordinates from relative to Window-absolute, so we draw
    // in the correct place in platform code
    Point aWinOffs;
    aWinOffs = OutputToScreenPixel( aWinOffs );
    Region screenRegion( rControlRegion );
    screenRegion.Move( aWinOffs.X(), aWinOffs.Y());

    // do so for ImplControlValue members, also
    ImplMoveControlValue( nType, aValue, aWinOffs );

    Region aTestRegion( GetActiveClipRegion() );
    aTestRegion.Intersect( rControlRegion );
    if( aTestRegion == rControlRegion )
        nState |= CTRL_CACHING_ALLOWED;   // control is not clipped, caching allowed

    BOOL bRet = mpGraphics->DrawNativeControl(nType, nPart, screenRegion, nState, aValue, *ImplGetWinData()->mpSalControlHandle, aCaption, this );

    // transform back ImplControlValue members
    ImplMoveControlValue( nType, aValue, Point()-aWinOffs );

    return bRet;
}


// -----------------------------------------------------------------------

BOOL Window::DrawNativeControlText(ControlType nType,
                            ControlPart nPart,
                            const Region& rControlRegion,
                            ControlState nState,
                            const ImplControlValue& aValue,
                            OUString aCaption )
{
    if( !IsNativeWidgetEnabled() )
        return FALSE;

    ImplInitSalControlHandle();

    // make sure the current clip region is initialized correctly
    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return false;

    if ( mbInitClipRegion )
        ImplInitClipRegion();
    if ( mbOutputClipped )
        return true;

    if ( mbInitLineColor )
        ImplInitLineColor();
    if ( mbInitFillColor )
        ImplInitFillColor();

    // Convert the coordinates from relative to Window-absolute, so we draw
    // in the correct place in platform code
    Point aWinOffs;
    aWinOffs = OutputToScreenPixel( aWinOffs );
    Region screenRegion( rControlRegion );
    screenRegion.Move( aWinOffs.X(), aWinOffs.Y());
    ImplMoveControlValue( nType, aValue, aWinOffs );

    BOOL bRet = mpGraphics->DrawNativeControlText(nType, nPart, screenRegion, nState, aValue, *ImplGetWinData()->mpSalControlHandle, aCaption, this );

    // transform back ImplControlValue members
    ImplMoveControlValue( nType, aValue, Point()-aWinOffs );

    return bRet;
}


// -----------------------------------------------------------------------

BOOL Window::GetNativeControlRegion(  ControlType nType,
                                ControlPart nPart,
                                const Region& rControlRegion,
                                ControlState nState,
                                const ImplControlValue& aValue,
                                OUString aCaption,
                                Region &rNativeBoundingRegion,
                                Region &rNativeContentRegion )
{
    if( !IsNativeWidgetEnabled() )
        return FALSE;

    if ( !mpGraphics )
        if ( !ImplGetGraphics() )
            return FALSE;

    ImplInitSalControlHandle();

    // Convert the coordinates from relative to Window-absolute, so we draw
    // in the correct place in platform code
    Point aWinOffs;
    aWinOffs = OutputToScreenPixel( aWinOffs );
    Region screenRegion( rControlRegion );
    screenRegion.Move( aWinOffs.X(), aWinOffs.Y());
    ImplMoveControlValue( nType, aValue, aWinOffs );

    BOOL bRet = mpGraphics->GetNativeControlRegion(nType, nPart, screenRegion, nState, aValue,
                                *ImplGetWinData()->mpSalControlHandle, aCaption, rNativeBoundingRegion,
                                rNativeContentRegion, this );
    if( bRet )
    {
        // transform back native regions
        rNativeBoundingRegion.Move( -aWinOffs.X(), -aWinOffs.Y() );
        rNativeContentRegion.Move( -aWinOffs.X(), -aWinOffs.Y() );
    }
    // transform back ImplControlValue members
    ImplMoveControlValue( nType, aValue, Point()-aWinOffs );

    return bRet;
}


// -----------------------------------------------------------------------

WaitObject::~WaitObject()
{
    if ( mpWindow )
        mpWindow->LeaveWait();
}

// -----------------------------------------------------------------------

Size Window::GetOptimalSize(WindowSizeType eType) const
{
    switch (eType) {
    case WINDOWSIZE_MINIMUM:
        return Size();
    case WINDOWSIZE_PREFERRED:
        return GetOptimalSize( WINDOWSIZE_MINIMUM );
    case WINDOWSIZE_MAXIMUM:
    default:
        return Size( LONG_MAX, LONG_MAX );
    }
}

// -----------------------------------------------------------------------

void Window::ImplAdjustNWFSizes()
{
    switch( GetType() )
    {
    case WINDOW_CHECKBOX:
        ((CheckBox*)this)->ImplSetMinimumNWFSize();
        break;
    case WINDOW_RADIOBUTTON:
        ((RadioButton*)this)->ImplSetMinimumNWFSize();
        break;
    default:
        {
            // iterate over children
            Window* pWin = GetWindow( WINDOW_FIRSTCHILD );
            while( pWin )
            {
                pWin->ImplAdjustNWFSizes();
                pWin = pWin->GetWindow( WINDOW_NEXT );
            }
        }
        break;
    }
}
