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

#include <com/sun/star/awt/XWindow.hpp>
#include <core_resource.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include "AppTitleWindow.hxx"

namespace dbaui
{

OTitleWindow::OTitleWindow(vcl::Window* pParent, const char* pTitleId)
    : InterimItemWindow(pParent, "dbaccess/ui/titlewindow.ui", "TitleWindow")
    , m_xTitleFrame(m_xBuilder->weld_container("titleparent"))
    , m_xTitle(m_xBuilder->weld_label("title"))
    , m_xChildContainer(m_xBuilder->weld_container("box"))
    , m_xChildParent(m_xChildContainer->CreateChildFrame())
    , m_xChild(nullptr)
{
    setTitle(pTitleId);
    ImplInitSettings();

    m_xTitleFrame->set_title_background();
    m_xTitle->set_label_type(weld::LabelType::Title);
}

OTitleWindow::~OTitleWindow()
{
    disposeOnce();
}

void OTitleWindow::dispose()
{
    if (m_xChild)
        m_xChild->Hide();
    m_xChild.disposeAndClear();
    m_xChildParent->dispose();
    m_xChildParent.clear();
    m_xChildContainer.reset();
    m_xTitle.reset();
    m_xTitleFrame.reset();
    InterimItemWindow::dispose();
}

vcl::Window* OTitleWindow::getChildContainer()
{
    return VCLUnoHelper::GetWindow(m_xChildParent);
}

void OTitleWindow::setChildWindow(vcl::Window* pChild)
{
    m_xChild = pChild;
}

void OTitleWindow::setTitle(const char* pTitleId)
{
    if (!pTitleId)
        return;
    m_xTitle->set_label(DBA_RES(pTitleId));
}

void OTitleWindow::GetFocus()
{
    InterimItemWindow::GetFocus();
    if (m_xChild)
        m_xChild->GrabFocus();
}

long OTitleWindow::GetWidthPixel() const
{
    Size aTextSize = LogicToPixel(Size(12, 0), MapMode(MapUnit::MapAppFont));
    sal_Int32 nWidth = GetTextWidth(m_xTitle->get_label()) + 2*aTextSize.Width();

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
    vcl::Font aFont = rStyleSettings.GetFieldFont();
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
    vcl::Font aFont = rStyleSettings.GetFieldFont();
    aFont.SetColor(rStyleSettings.GetWindowTextColor());
    SetPointFont(*this, aFont);

    rRenderContext.SetTextColor(rStyleSettings.GetFieldTextColor());
    rRenderContext.SetTextFillColor();

    rRenderContext.SetBackground(rStyleSettings.GetFieldColor());
}

} // namespace dbaui
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
