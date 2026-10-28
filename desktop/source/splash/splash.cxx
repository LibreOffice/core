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

#include <splash/splash.hxx>

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

SplashScreen::SplashScreen(const OUString& rAppName)
    : m_pWindow(VclPtr<SplashScreenWindow>::Create(rAppName))
    , m_bProgressEnd(false)
{
    Application::AddEventListener(LINK(this, SplashScreen, AppEventListenerHdl));
}

SplashScreen::~SplashScreen()
{
    Application::RemoveEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
    m_pWindow->Hide();
    m_pWindow.disposeAndClear();
}

void SplashScreen::start()
{
    m_bProgressEnd = false;
    SolarMutexGuard aSolarGuard;
    m_pWindow->Show();
    m_pWindow->Redraw();
}

void SplashScreen::end()
{
    m_pWindow->SetProgress(SplashScreenWindow::PROGRESS_MAX);
    m_pWindow->Hide();
    m_bProgressEnd = true;
}

void SplashScreen::reset()
{
    m_pWindow->SetProgress(0);
    if (!m_bProgressEnd)
    {
        m_pWindow->Show();
        updateStatus();
    }
}

void SplashScreen::setText(const OUString& rText)
{
    SolarMutexGuard aSolarGuard;

    m_pWindow->SetProgressText(rText);
    if (!m_bProgressEnd)
    {
        m_pWindow->Show();
        updateStatus();
    }
}

void SplashScreen::setValue(sal_Int32 nValue)
{
    SAL_INFO( "desktop.splash", "setValue: " << nValue );

    SolarMutexGuard aSolarGuard;
    if (!m_bProgressEnd)
    {
        m_pWindow->Show();
        m_pWindow->SetProgress(std::min(nValue, SplashScreenWindow::PROGRESS_MAX));
        updateStatus();
    }
}

void SplashScreen::updateStatus()
{
    if (!m_bProgressEnd)
        m_pWindow->Redraw();
}

// internal private methods
IMPL_LINK( SplashScreen, AppEventListenerHdl, VclSimpleEvent&, inEvent, void )
{
    if (static_cast<VclWindowEvent&>(inEvent).GetWindow() == m_pWindow)
    {
        switch ( inEvent.GetId() )
        {
            case VclEventId::WindowShow:
                m_pWindow->Redraw();
                break;
            default:
                break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
