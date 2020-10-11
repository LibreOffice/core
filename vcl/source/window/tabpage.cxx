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

#include <vcl/accel.hxx>
#include <vcl/event.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/settings.hxx>

void TabPage::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;

    Window::ImplInit( pParent, nStyle, nullptr );

    ImplInitSettings();

    // if the tabpage is drawn (ie filled) by a native widget, make sure all controls will have transparent background
    // otherwise they will paint with a wrong background
    if( IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL) )
        EnableChildTransparentMode();
}

void TabPage::ImplInitSettings()
{
    vcl::Window* pParent = GetParent();
    if (pParent && pParent->IsChildTransparentModeEnabled() && !IsControlBackground())
    {
        EnableChildTransparentMode();
        SetParentClipMode( ParentClipMode::NoClip );
        SetPaintTransparent( true );
        SetBackground();
    }
    else
    {
        EnableChildTransparentMode( false );
        SetParentClipMode();
        SetPaintTransparent( false );

        if (IsControlBackground() || !pParent)
            SetBackground( GetControlBackground() );
        else
            SetBackground( pParent->GetBackground() );
    }
}

TabPage::TabPage( vcl::Window* pParent, WinBits nStyle ) :
    Window( WindowType::TABPAGE )
    , IContext()
{
    ImplInit( pParent, nStyle );
}

TabPage::~TabPage()
{
    disposeOnce();
}

void TabPage::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == StateChangedType::InitShow )
    {
        if (GetSettings().GetStyleSettings().GetAutoMnemonic())
            Accelerator::GenerateAutoMnemonicsOnHierarchy(this);
        // FIXME: no layouting, workaround some clipping issues
        ImplAdjustNWFSizes();
    }
    else if ( nType == StateChangedType::ControlBackground )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void TabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void TabPage::Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& )
{
    // draw native tabpage only inside tabcontrols, standalone tabpages look ugly (due to bad dialog design)
    if( !(IsNativeControlSupported(ControlType::TabBody, ControlPart::Entire) && GetParent() && (GetParent()->GetType() == WindowType::TABCONTROL)) )
        return;

    const ImplControlValue aControlValue;

    ControlState nState = ControlState::ENABLED;
    if ( !IsEnabled() )
        nState &= ~ControlState::ENABLED;
    if ( HasFocus() )
        nState |= ControlState::FOCUSED;
    // pass the whole window region to NWF as the tab body might be a gradient or bitmap
    // that has to be scaled properly, clipping makes sure that we do not paint too much
    tools::Rectangle aCtrlRegion( Point(), GetOutputSizePixel() );
    rRenderContext.DrawNativeControl( ControlType::TabBody, ControlPart::Entire, aCtrlRegion, nState,
            aControlValue, OUString() );
}

void TabPage::Draw( OutputDevice* pDev, const Point& rPos, DrawFlags )
{
    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = GetSizePixel();

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
        pDev->DrawRect( tools::Rectangle( aPos, aSize ) );
    }

    pDev->Pop();
}

Size TabPage::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));
    return getLegacyBestSizeForChildren(*this);
}

void TabPage::SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation)
{
    Window::SetPosSizePixel(rAllocPos, rAllocation);
    if (isLayoutEnabled(this) && rAllocation.Width() && rAllocation.Height())
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), rAllocation);
}

void TabPage::SetSizePixel(const Size& rAllocation)
{
    Window::SetSizePixel(rAllocation);
    if (isLayoutEnabled(this) && rAllocation.Width() && rAllocation.Height())
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), rAllocation);
}

void TabPage::SetPosPixel(const Point& rAllocPos)
{
    Window::SetPosPixel(rAllocPos);
    Size aAllocation(GetOutputSizePixel());
    if (isLayoutEnabled(this) && aAllocation.Width() && aAllocation.Height())
    {
        VclContainer::setLayoutAllocation(*GetWindow(GetWindowType::FirstChild), Point(0, 0), aAllocation);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
