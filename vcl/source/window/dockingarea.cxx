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
#include <tools/debug.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svdata.hxx>

#include <map>

// =======================================================================

class DockingAreaWindow::ImplData
{
public:
    ImplData();
    ~ImplData();

    WindowAlign meAlign;
};

DockingAreaWindow::ImplData::ImplData()
{
    meAlign = WINDOWALIGN_TOP;
}

DockingAreaWindow::ImplData::~ImplData()
{
}

// =======================================================================

static void ImplInitBackground( DockingAreaWindow* pThis )
{
    if( !pThis->IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
    {
        Wallpaper aWallpaper;
        aWallpaper.SetStyle( WALLPAPER_APPLICATIONGRADIENT );
        pThis->SetBackground( aWallpaper );
    }
    else
        pThis->SetBackground( Wallpaper( pThis->GetSettings().GetStyleSettings().GetFaceColor() ) );
}

DockingAreaWindow::DockingAreaWindow( Window* pParent ) :
    Window( WINDOW_DOCKINGAREA )
{
    ImplInit( pParent, WB_CLIPCHILDREN|WB_3DLOOK, NULL );

    mpImplData = new ImplData;
    ImplInitBackground( this );
}

DockingAreaWindow::~DockingAreaWindow()
{
    delete mpImplData;
}

// -----------------------------------------------------------------------

void DockingAreaWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitBackground( this );
    }
}

// -----------------------------------------------------------------------

static void ImplInvalidateMenubar( DockingAreaWindow* pThis )
{
    // due to a possible comon gradient covering menubar and top dockingarea
    // the menubar must be repainted if the top dockingarea changes size or visibility
    if( ImplGetSVData()->maNWFData.mbMenuBarDockingAreaCommonBG &&
        (pThis->GetAlign() == WINDOWALIGN_TOP)
        && pThis->IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL )
        && pThis->IsNativeControlSupported( CTRL_MENUBAR, PART_ENTIRE_CONTROL ) )
    {
        SystemWindow *pSysWin = pThis->GetSystemWindow();
        if( pSysWin && pSysWin->GetMenuBar() )
        {
            Window *pMenubarWin = pSysWin->GetMenuBar()->GetWindow();
            if( pMenubarWin )
                pMenubarWin->Invalidate();
        }
    }
}

void DockingAreaWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_VISIBLE )
        ImplInvalidateMenubar( this );
}

// -----------------------------------------------------------------------

sal_Bool DockingAreaWindow::IsHorizontal() const
{
    return ( mpImplData->meAlign == WINDOWALIGN_TOP || mpImplData->meAlign == WINDOWALIGN_BOTTOM );
}

void DockingAreaWindow::SetAlign( WindowAlign eNewAlign )
{
    if( eNewAlign != mpImplData->meAlign )
    {
        mpImplData->meAlign = eNewAlign;
        Invalidate();
    }
}

WindowAlign DockingAreaWindow::GetAlign() const
{
    return mpImplData->meAlign;
}

// -----------------------------------------------------------------------

void DockingAreaWindow::Paint( const Rectangle& )
{
    EnableNativeWidget( sal_True ); // only required because the toolkit curently switches this flag off
    if( IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
    {
        ToolbarValue        aControlValue;

        if( GetAlign() == WINDOWALIGN_TOP && ImplGetSVData()->maNWFData.mbMenuBarDockingAreaCommonBG )
        {
            // give NWF a hint that this dockingarea is adjacent to the menubar
            // useful for special gradient effects that should cover both windows
            aControlValue.mbIsTopDockingArea = sal_True;
        }
        ControlState        nState = CTRL_STATE_ENABLED;

        if( !ImplGetSVData()->maNWFData.mbDockingAreaSeparateTB )
        {
            // draw a single toolbar background covering the whole docking area
            Point tmp;
            Rectangle aCtrlRegion( tmp, GetOutputSizePixel() );

            DrawNativeControl( CTRL_TOOLBAR, IsHorizontal() ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT,
                               aCtrlRegion, nState, aControlValue, rtl::OUString() );

            // each toolbar gets a thin border to better recognize its borders on the homogeneous docking area
            sal_uInt16 nChildren = GetChildCount();
            for( sal_uInt16 n = 0; n < nChildren; n++ )
            {
                Window* pChild = GetChild( n );
                if ( pChild->IsVisible() )
                {
                    Point aPos = pChild->GetPosPixel();
                    Size aSize = pChild->GetSizePixel();
                    Rectangle aRect( aPos, aSize );

                    SetLineColor( GetSettings().GetStyleSettings().GetLightColor() );
                    DrawLine( aRect.TopLeft(), aRect.TopRight() );
                    DrawLine( aRect.TopLeft(), aRect.BottomLeft() );

                    SetLineColor( GetSettings().GetStyleSettings().GetSeparatorColor() );
                    DrawLine( aRect.BottomLeft(), aRect.BottomRight() );
                    DrawLine( aRect.TopRight(), aRect.BottomRight() );
                }
            }
        }
        else
        {
            // create map to find toolbar lines
            Size aOutSz = GetOutputSizePixel();
            std::map< int, int > ranges;
            sal_uInt16 nChildren = GetChildCount();
            for( sal_uInt16 n = 0; n < nChildren; n++ )
            {
                Window* pChild = GetChild( n );
                Point aPos = pChild->GetPosPixel();
                Size aSize = pChild->GetSizePixel();
                if( IsHorizontal() )
                    ranges[ aPos.Y() ] = aSize.Height();
                else
                    ranges[ aPos.X() ] = aSize.Width();
            }


            // draw multiple toolbar backgrounds, i.e., one for each toolbar line
            for( std::map<int,int>::const_iterator it = ranges.begin(); it != ranges.end(); ++it )
            {
                Rectangle aTBRect;
                if( IsHorizontal() )
                {
                    aTBRect.Left()      = 0;
                    aTBRect.Right()     = aOutSz.Width() - 1;
                    aTBRect.Top()       = it->first;
                    aTBRect.Bottom()    = it->first + it->second - 1;
                }
                else
                {
                    aTBRect.Left()      = it->first;
                    aTBRect.Right()     = it->first + it->second - 1;
                    aTBRect.Top()       = 0;
                    aTBRect.Bottom()    = aOutSz.Height() - 1;
                }
                DrawNativeControl( CTRL_TOOLBAR, IsHorizontal() ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT,
                                   aTBRect, nState, aControlValue, rtl::OUString() );
            }
        }
    }
}

void DockingAreaWindow::Resize()
{
    ImplInvalidateMenubar( this );
    if( IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
        Invalidate();
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
