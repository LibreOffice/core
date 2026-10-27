/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <splash/splash.hxx>

#include <vcl/salnativewidgets.hxx>

SplashScreenWindow::SplashScreenWindow(SplashScreen* pSplash)
    : m_pSplash(pSplash)
    , m_pVDev(VclPtr<VirtualDevice>::Create(*GetOutDev()))
{
    m_pVDev->EnableRTL(IsRTLEnabled());
}

void SplashScreenWindow::dispose()
{
    m_pSplash = nullptr;
    IntroWindow::dispose();
}

void SplashScreenWindow::SetOutputSizePixel(const Size& rNewSize)
{
    IntroWindow::SetOutputSizePixel(rNewSize);
    m_pVDev->SetOutputSizePixel(rNewSize);
}

void SplashScreenWindow::Redraw()
{
    Invalidate();
    // Trigger direct painting too - otherwise the splash screen won't be
    // shown in some cases (when the idle timer won't be hit).
    Paint(*GetOutDev(), tools::Rectangle());
    GetOutDev()->Flush();
}

void SplashScreenWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (!m_pSplash)
        return;

    //native drawing
    // in case of native controls we need to draw directly to the window
    if (m_pSplash->m_bNativeProgress
        && rRenderContext.IsNativeControlSupported(ControlType::IntroProgress, ControlPart::Entire))
    {
        rRenderContext.DrawBitmap(Point(), m_pSplash->m_aIntroBmp);

        ImplControlValue aValue(m_pSplash->m_nProgress * m_pSplash->m_nBarwidth
                                / m_pSplash->m_nMax);
        tools::Rectangle aDrawRect(Point(m_pSplash->m_nTlx, m_pSplash->m_nTly),
                                   Size(m_pSplash->m_nBarwidth, m_pSplash->m_nBarheight));
        tools::Rectangle aNativeControlRegion, aNativeContentRegion;

        if (rRenderContext.GetNativeControlRegion(ControlType::IntroProgress, ControlPart::Entire,
                                                  aDrawRect, ControlState::ENABLED, aValue,
                                                  aNativeControlRegion, aNativeContentRegion))
        {
            tools::Long nProgressHeight = aNativeControlRegion.GetHeight();
            aDrawRect.AdjustTop(-((nProgressHeight - m_pSplash->m_nBarheight) / 2));
            aDrawRect.AdjustBottom((nProgressHeight - m_pSplash->m_nBarheight) / 2);
        }

        if (rRenderContext.DrawNativeControl(ControlType::IntroProgress, ControlPart::Entire,
                                             aDrawRect, ControlState::ENABLED, aValue,
                                             m_pSplash->m_sProgressText))
        {
            return;
        }
    }

    // non native drawing
    // draw bitmap
    m_pVDev->DrawBitmap(Point(), m_pSplash->m_aIntroBmp);

    if (m_pSplash->m_bPaintProgress)
    {
        // draw progress...
        tools::Long length = (m_pSplash->m_nProgress * m_pSplash->m_nBarwidth / m_pSplash->m_nMax)
                             - (2 * m_pSplash->m_nBarspace);
        if (length < 0)
            length = 0;

        // border
        m_pVDev->SetFillColor();
        m_pVDev->SetLineColor(m_pSplash->m_aProgressFrameColor);
        m_pVDev->DrawRect(tools::Rectangle(m_pSplash->m_nTlx, m_pSplash->m_nTly,
                                           m_pSplash->m_nTlx + m_pSplash->m_nBarwidth,
                                           m_pSplash->m_nTly + m_pSplash->m_nBarheight));
        m_pVDev->SetFillColor(m_pSplash->m_aProgressBarColor);
        m_pVDev->SetLineColor();
        m_pVDev->DrawRect(tools::Rectangle(
            m_pSplash->m_nTlx + m_pSplash->m_nBarspace, m_pSplash->m_nTly + m_pSplash->m_nBarspace,
            m_pSplash->m_nTlx + m_pSplash->m_nBarspace + length,
            m_pSplash->m_nTly + m_pSplash->m_nBarheight - m_pSplash->m_nBarspace));
        vcl::Font aFont;
        aFont.SetFontSize(Size(0, 12));
        aFont.SetAlignment(ALIGN_BASELINE);
        m_pVDev->SetFont(aFont);
        m_pVDev->SetTextColor(m_pSplash->m_aProgressTextColor);
        m_pVDev->DrawText(Point(m_pSplash->m_nTlx, m_pSplash->m_nTextBaseline),
                          m_pSplash->m_sProgressText);
    }
    rRenderContext.DrawOutDev(Point(), GetOutputSizePixel(), Point(), m_pVDev->GetOutputSizePixel(),
                              *m_pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
