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

#include <vcl/dockingarea.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

#include <svdata.hxx>

#include <map>

class DockingAreaWindow::ImplData
{
public:
    ImplData();

    WindowAlign meAlign;
};

DockingAreaWindow::ImplData::ImplData()
{
    meAlign = WindowAlign::Top;
}

DockingAreaWindow::DockingAreaWindow( vcl::Window* pParent ) :
    Window( WindowType::DOCKINGAREA )
{
    ImplInit( pParent, WB_CLIPCHILDREN|WB_3DLOOK, nullptr );

    mpImplData.reset(new ImplData);
}

DockingAreaWindow::~DockingAreaWindow()
{
    disposeOnce();
}

void DockingAreaWindow::dispose()
{
    mpImplData.reset();
    Window::dispose();
}

void DockingAreaWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        Invalidate();
    }
}

static void ImplInvalidateMenubar( DockingAreaWindow const * pThis )
{
    // due to a possible common gradient covering menubar and top dockingarea
    // the menubar must be repainted if the top dockingarea changes size or visibility
    if( ImplGetSVData()->maNWFData.mbMenuBarDockingAreaCommonBG &&
        (pThis->GetAlign() == WindowAlign::Top)
        && pThis->IsNativeControlSupported( ControlType::Toolbar, ControlPart::Entire )
        && pThis->IsNativeControlSupported( ControlType::Menubar, ControlPart::Entire ) )
    {
        SystemWindow *pSysWin = pThis->GetSystemWindow();
        if( pSysWin && pSysWin->GetMenuBar() )
        {
            vcl::Window *pMenubarWin = pSysWin->GetMenuBar()->GetWindow();
            if( pMenubarWin )
                pMenubarWin->Invalidate();
        }
    }
}

void DockingAreaWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::Visible )
        ImplInvalidateMenubar( this );
}

bool DockingAreaWindow::IsHorizontal() const
{
    return ( mpImplData->meAlign == WindowAlign::Top || mpImplData->meAlign == WindowAlign::Bottom );
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

void DockingAreaWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    const StyleSettings rSetting = rRenderContext.GetSettings().GetStyleSettings();
    const BitmapEx& rPersonaBitmap = (GetAlign() == WindowAlign::Top) ? rSetting.GetPersonaHeader() : rSetting.GetPersonaFooter();

    if (!rPersonaBitmap.IsEmpty() && (GetAlign() == WindowAlign::Top || GetAlign()==WindowAlign::Bottom))
    {
        Wallpaper aWallpaper(rPersonaBitmap);
        if (GetAlign() == WindowAlign::Top)
            aWallpaper.SetStyle(WallpaperStyle::TopRight);
        else
            aWallpaper.SetStyle(WallpaperStyle::BottomRight);
        aWallpaper.SetColor(rSetting.GetWorkspaceColor());

        // we need to shift the bitmap vertically so that it spans over the
        // menubar conveniently
        SystemWindow* pSysWin = GetSystemWindow();
        MenuBar* pMenuBar = pSysWin ? pSysWin->GetMenuBar() : nullptr;
        int nMenubarHeight = pMenuBar ? pMenuBar->GetMenuBarHeight() : 0;
        aWallpaper.SetRect(tools::Rectangle(Point(0, -nMenubarHeight),
                           Size(rRenderContext.GetOutputWidthPixel(),
                                rRenderContext.GetOutputHeightPixel() + nMenubarHeight)));

        rRenderContext.SetBackground(aWallpaper);
    }
    else if (!rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Entire))
    {
        Wallpaper aWallpaper;
        aWallpaper.SetStyle(WallpaperStyle::ApplicationGradient);
        rRenderContext.SetBackground(aWallpaper);
    }
    else
        rRenderContext.SetBackground(Wallpaper(rSetting.GetFaceColor()));

}

void DockingAreaWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    const StyleSettings rSetting = rRenderContext.GetSettings().GetStyleSettings();

    EnableNativeWidget(); // only required because the toolkit currently switches this flag off
    if (rRenderContext.IsNativeControlSupported(ControlType::Toolbar, ControlPart::Entire))
    {
        ToolbarValue aControlValue;

        if (GetAlign() == WindowAlign::Top && ImplGetSVData()->maNWFData.mbMenuBarDockingAreaCommonBG)
        {
            // give NWF a hint that this dockingarea is adjacent to the menubar
            // useful for special gradient effects that should cover both windows
            aControlValue.mbIsTopDockingArea = true;
        }

        ControlState nState = ControlState::ENABLED;
        const bool isFooter = GetAlign() == WindowAlign::Bottom && !rSetting.GetPersonaFooter().IsEmpty();

        if ((GetAlign() == WindowAlign::Top && !rSetting.GetPersonaHeader().IsEmpty() ) || isFooter)
            Erase(rRenderContext);
        else if (!ImplGetSVData()->maNWFData.mbDockingAreaSeparateTB)
        {
            // draw a single toolbar background covering the whole docking area
            tools::Rectangle aCtrlRegion(Point(), GetOutputSizePixel());

            rRenderContext.DrawNativeControl(ControlType::Toolbar, IsHorizontal() ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert,
                                             aCtrlRegion, nState, aControlValue, OUString() );

            if (!ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames)
            {
                // each toolbar gets a thin border to better recognize its borders on the homogeneous docking area
                sal_uInt16 nChildren = GetChildCount();
                for (sal_uInt16 n = 0; n < nChildren; n++)
                {
                    vcl::Window* pChild = GetChild(n);
                    if (pChild->IsVisible())
                    {
                        Point aPos = pChild->GetPosPixel();
                        Size aSize = pChild->GetSizePixel();
                        tools::Rectangle aRect(aPos, aSize);

                        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetLightColor());
                        rRenderContext.DrawLine(aRect.TopLeft(), aRect.TopRight());
                        rRenderContext.DrawLine(aRect.TopLeft(), aRect.BottomLeft());

                        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetSeparatorColor());
                        rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
                        rRenderContext.DrawLine(aRect.TopRight(), aRect.BottomRight());
                    }
                }
            }
        }
        else
        {
            // create map to find toolbar lines
            Size aOutSz(GetOutputSizePixel());
            std::map<int, int> ranges;
            sal_uInt16 nChildren = GetChildCount();
            for (sal_uInt16 n = 0; n < nChildren; n++)
            {
                vcl::Window* pChild = GetChild(n);
                Point aPos = pChild->GetPosPixel();
                Size aSize = pChild->GetSizePixel();
                if (IsHorizontal())
                    ranges[aPos.Y()] = aSize.Height();
                else
                    ranges[aPos.X()] = aSize.Width();
            }

            // draw multiple toolbar backgrounds, i.e., one for each toolbar line
            for (auto const& range : ranges)
            {
                tools::Rectangle aTBRect;
                if (IsHorizontal())
                {
                    aTBRect.SetLeft( 0 );
                    aTBRect.SetRight( aOutSz.Width() - 1 );
                    aTBRect.SetTop( range.first );
                    aTBRect.SetBottom( range.first + range.second - 1 );
                }
                else
                {
                    aTBRect.SetLeft( range.first );
                    aTBRect.SetRight( range.first + range.second - 1 );
                    aTBRect.SetTop( 0 );
                    aTBRect.SetBottom( aOutSz.Height() - 1 );
                }
                rRenderContext.DrawNativeControl(ControlType::Toolbar, IsHorizontal() ? ControlPart::DrawBackgroundHorz : ControlPart::DrawBackgroundVert,
                                                 aTBRect, nState, aControlValue, OUString());
            }
        }
    }
}

void DockingAreaWindow::Resize()
{
    ImplInvalidateMenubar( this );
    if (IsNativeControlSupported(ControlType::Toolbar, ControlPart::Entire))
        Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
