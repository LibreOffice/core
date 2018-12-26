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

#include "AppTitleWindow.hxx"
#include <core_resource.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>

namespace dbaui
{

OTitleWindow::OTitleWindow(vcl::Window* _pParent, const char* pTitleId, WinBits _nBits, bool _bShift)
: Window(_pParent,_nBits | WB_DIALOGCONTROL)
, m_aSpace1(VclPtr<FixedText>::Create(this))
, m_aSpace2(VclPtr<FixedText>::Create(this))
, m_aTitle(VclPtr<FixedText>::Create(this))
, m_pChild(nullptr)
, m_bShift(_bShift)
{
    setTitle(pTitleId);
    SetBorderStyle(WindowBorderStyle::MONO);
    ImplInitSettings();

    const StyleSettings& rStyle = Application::GetSettings().GetStyleSettings();
    vcl::Window* pWindows[] = { m_aSpace1.get(), m_aSpace2.get(), m_aTitle.get() };
    for (vcl::Window* pWindow : pWindows)
    {
        vcl::Font aFont = pWindow->GetControlFont();
        aFont.SetWeight(WEIGHT_BOLD);
        pWindow->SetControlFont(aFont);
        pWindow->SetControlForeground(rStyle.GetLightColor());
        pWindow->SetControlBackground(rStyle.GetShadowColor());
        pWindow->Show();
    }
}

OTitleWindow::~OTitleWindow()
{
    disposeOnce();
}

void OTitleWindow::dispose()
{
    if ( m_pChild )
    {
        m_pChild->Hide();
    }
    m_pChild.disposeAndClear();
    m_aSpace1.disposeAndClear();
    m_aSpace2.disposeAndClear();
    m_aTitle.disposeAndClear();
    vcl::Window::dispose();
}

void OTitleWindow::setChildWindow(vcl::Window* _pChild)
{
    m_pChild = _pChild;
}

#define SPACE_BORDER    1
void OTitleWindow::Resize()
{
    // parent window dimension
    Size aOutputSize( GetOutputSize() );
    long nOutputWidth   = aOutputSize.Width();
    long nOutputHeight  = aOutputSize.Height();

    Size aTextSize = LogicToPixel(Size(6, 3), MapMode(MapUnit::MapAppFont));
    sal_Int32 nXOffset = aTextSize.Width();
    sal_Int32 nYOffset = aTextSize.Height();
    sal_Int32 nHeight = GetTextHeight() + 2*nYOffset;

    m_aSpace1->SetPosSizePixel(  Point(SPACE_BORDER, SPACE_BORDER ),
                                Size(nXOffset , nHeight - SPACE_BORDER) );
    m_aSpace2->SetPosSizePixel(  Point(nXOffset + SPACE_BORDER, SPACE_BORDER ),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nYOffset) );
    m_aTitle->SetPosSizePixel(   Point(nXOffset + SPACE_BORDER, nYOffset + SPACE_BORDER),
                                Size(nOutputWidth - nXOffset - 2*SPACE_BORDER, nHeight - nYOffset - SPACE_BORDER) );
    if ( m_pChild )
    {
        m_pChild->SetPosSizePixel(  Point(m_bShift ? (nXOffset+SPACE_BORDER) : sal_Int32(SPACE_BORDER), nHeight + nXOffset + SPACE_BORDER),
                                    Size(nOutputWidth - ( m_bShift ? (2*nXOffset - 2*SPACE_BORDER) : sal_Int32(SPACE_BORDER) ), nOutputHeight - nHeight - 2*nXOffset - 2*SPACE_BORDER) );
    }
}

void OTitleWindow::setTitle(const char* pTitleId)
{
    if (pTitleId)
    {
        m_aTitle->SetText(DBA_RES(pTitleId));
    }
}

void OTitleWindow::GetFocus()
{
    Window::GetFocus();
    if ( m_pChild )
        m_pChild->GrabFocus();
}

long OTitleWindow::GetWidthPixel() const
{
    Size aTextSize = LogicToPixel(Size(12, 0), MapMode(MapUnit::MapAppFont));
    sal_Int32 nWidth = GetTextWidth(m_aTitle->GetText()) + 2*aTextSize.Width();

    return nWidth;
}

void OTitleWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
        (rDCEvt.GetType() == DataChangedEventType::DISPLAY) ||
        (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
        ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

void OTitleWindow::ImplInitSettings()
{
    // FIXME RenderContext
    AllSettings aAllSettings = GetSettings();
    StyleSettings aStyle = aAllSettings.GetStyleSettings();
    aStyle.SetMonoColor(aStyle.GetActiveBorderColor());//GetMenuBorderColor());
    aAllSettings.SetStyleSettings(aStyle);
    SetSettings(aAllSettings);

    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    vcl::Font aFont;
    aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    SetPointFont(*this, aFont);

    SetTextColor( rStyleSettings.GetFieldTextColor() );
    SetTextFillColor();

    SetBackground( rStyleSettings.GetFieldColor() );
}

void OTitleWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    // FIXME RenderContext
    AllSettings aAllSettings = rRenderContext.GetSettings();
    StyleSettings aStyle = aAllSettings.GetStyleSettings();
    aStyle.SetMonoColor(aStyle.GetActiveBorderColor());//GetMenuBorderColor());
    aAllSettings.SetStyleSettings(aStyle);
    rRenderContext.SetSettings(aAllSettings);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    vcl::Font aFont;
    aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor(rStyleSettings.GetWindowTextColor());
    SetPointFont(*this, aFont);

    rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
    rRenderContext.SetTextFillColor();

    rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
}

} // namespace dbaui
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
