/*************************************************************************
 *
 *  $RCSfile: dockingarea.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2004-12-23 09:12:50 $
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
#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_DOCKINGAREA_HXX
#include <dockingarea.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif

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

BOOL DockingAreaWindow::IsHorizontal() const
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

void DockingAreaWindow::Paint( const Rectangle& rRect )
{
    EnableNativeWidget( TRUE ); // only required because the toolkit curently switches this flag off
    if( IsNativeControlSupported( CTRL_TOOLBAR, PART_ENTIRE_CONTROL ) )
    {
        ImplControlValue    aControlValue;
        ToolbarValue        aToolbarValue;

        if( GetAlign() == WINDOWALIGN_TOP && ImplGetSVData()->maNWFData.mbMenuBarDockingAreaCommonBG )
        {
            // give NWF a hint that this dockingarea is adjacent to the menubar
            // useful for special gradient effects that should cover both windows
            aToolbarValue.mbIsTopDockingArea = TRUE;
        }
        aControlValue.setOptionalVal( (void *)(&aToolbarValue) );
        ControlState        nState = CTRL_STATE_ENABLED;
        if( !ImplGetSVData()->maNWFData.mbDockingAreaSeparateTB )
        {
            // draw a single toolbar background covering the whole docking area
            Point tmp;
            Region aCtrlRegion( Rectangle( tmp, GetOutputSizePixel() ) );

            DrawNativeControl( CTRL_TOOLBAR, IsHorizontal() ? PART_DRAW_BACKGROUND_HORZ : PART_DRAW_BACKGROUND_VERT,
                               aCtrlRegion, nState, aControlValue, rtl::OUString() );
        }
        else
        {
            // draw multiple toolbar backgrounds, i.e., one for each toolbar line
            std::map< int, int > ranges;
            int nChildren = GetChildCount();
            for( int n = 0; n < nChildren; n++ )
            {
                Window* pChild = GetChild( n );
                Point aPos = pChild->GetPosPixel();
                Size aSize = pChild->GetSizePixel();
                if( IsHorizontal() )
                    ranges[ aPos.Y() ] = aSize.Height();
                else
                    ranges[ aPos.X() ] = aSize.Width();
            }
            Size aOutSz = GetOutputSizePixel();
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
                                   Region( aTBRect), nState, aControlValue, rtl::OUString() );
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

