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

#include <o3tl/string_view.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/math.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>

#define NOT_LOADED (tools::Long(-1))
#define NOT_LOADED_COLOR (Color(ColorTransparency, 0xffffffff))

SplashScreenWindow::SplashScreenWindow(const OUString& rAppName)
    : m_nProgress(0)
    , m_aProgressFrameColor(NOT_LOADED_COLOR)
    , m_aProgressBarColor(NOT_LOADED_COLOR)
    , m_aProgressTextColor(NOT_LOADED_COLOR)
    , m_bNativeProgress(true)
    , m_sAppName(rAppName)
    , m_bShowLogo(true)
    , m_nHeight(0)
    , m_nWidth(0)
    , m_nTlx(NOT_LOADED)
    , m_nTly(NOT_LOADED)
    , m_nBarwidth(NOT_LOADED)
    , m_nBarheight(NOT_LOADED)
    , m_nBarspace(2)
    , m_nTextBaseline(NOT_LOADED)
    , m_pVDev(VclPtr<VirtualDevice>::Create(*GetOutDev()))
{
    // set window title
    SetText(utl::ConfigManager::getProductName());

    loadConfig();

    // start to determine bitmap and all other required value
    if (m_bShowLogo)
        SetScreenBitmap(m_aIntroBmp);
    Size aSize = m_aIntroBmp.GetSizePixel();
    SetOutputSizePixel(aSize);
    m_nHeight = aSize.Height();
    m_nWidth = aSize.Width();
    if (m_nWidth > 500)
    {
        Point xtopleft(212, 216);
        if (NOT_LOADED == m_nTlx || NOT_LOADED == m_nTly)
        {
            m_nTlx = xtopleft.X(); // top-left x
            m_nTly = xtopleft.Y(); // top-left y
        }
        if (NOT_LOADED == m_nBarwidth)
            m_nBarwidth = 263;
        if (NOT_LOADED == m_nBarheight)
            m_nBarheight = 8;
    }
    else
    {
        if (NOT_LOADED == m_nBarwidth)
            m_nBarwidth = m_nWidth - (2 * _xoffset);
        if (NOT_LOADED == m_nBarheight)
            m_nBarheight = 6;
        if (NOT_LOADED == m_nTlx || NOT_LOADED == m_nTly)
        {
            m_nTlx = _xoffset; // top-left x
            m_nTly = m_nHeight - _yoffset; // top-left y
        }
    }

    if (NOT_LOADED == m_nTextBaseline)
        m_nTextBaseline = m_nHeight;

    if (NOT_LOADED_COLOR == m_aProgressFrameColor)
        m_aProgressFrameColor = COL_LIGHTGRAY;

    if (NOT_LOADED_COLOR == m_aProgressBarColor)
    {
        // progress bar: new color only for big bitmap format
        if (m_nWidth > 500)
            m_aProgressBarColor = Color(157, 202, 18);
        else
            m_aProgressBarColor = COL_BLUE;
    }

    if (NOT_LOADED_COLOR == m_aProgressTextColor)
        m_aProgressTextColor = COL_BLACK;

    m_pVDev->EnableRTL(IsRTLEnabled());
}

void SplashScreenWindow::SetOutputSizePixel(const Size& rNewSize)
{
    IntroWindow::SetOutputSizePixel(rNewSize);
    m_pVDev->SetOutputSizePixel(rNewSize);
}

void SplashScreenWindow::Redraw()
{
    Invalidate();

    // Trigger event processing to ensure drawing happens
    Scheduler::ProcessEventsToIdle();
}

void SplashScreenWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (isDisposed())
        return;

    //native drawing
    // in case of native controls we need to draw directly to the window
    if (m_bNativeProgress
        && rRenderContext.IsNativeControlSupported(ControlType::IntroProgress, ControlPart::Entire))
    {
        rRenderContext.DrawBitmap(Point(), m_aIntroBmp);

        ImplControlValue aValue(m_nProgress * m_nBarwidth / PROGRESS_MAX);
        tools::Rectangle aDrawRect(Point(m_nTlx, m_nTly), Size(m_nBarwidth, m_nBarheight));
        tools::Rectangle aNativeControlRegion, aNativeContentRegion;

        if (rRenderContext.GetNativeControlRegion(ControlType::IntroProgress, ControlPart::Entire,
                                                  aDrawRect, ControlState::ENABLED, aValue,
                                                  aNativeControlRegion, aNativeContentRegion))
        {
            tools::Long nProgressHeight = aNativeControlRegion.GetHeight();
            aDrawRect.AdjustTop(-((nProgressHeight - m_nBarheight) / 2));
            aDrawRect.AdjustBottom((nProgressHeight - m_nBarheight) / 2);
        }

        if (rRenderContext.DrawNativeControl(ControlType::IntroProgress, ControlPart::Entire,
                                             aDrawRect, ControlState::ENABLED, aValue,
                                             m_sProgressText))
        {
            return;
        }
    }

    // non native drawing
    // draw bitmap
    m_pVDev->DrawBitmap(Point(), m_aIntroBmp);

    // draw progress...
    tools::Long length = (m_nProgress * m_nBarwidth / PROGRESS_MAX) - (2 * m_nBarspace);
    if (length < 0)
        length = 0;

    // border
    m_pVDev->SetFillColor();
    m_pVDev->SetLineColor(m_aProgressFrameColor);
    m_pVDev->DrawRect(
        tools::Rectangle(m_nTlx, m_nTly, m_nTlx + m_nBarwidth, m_nTly + m_nBarheight));
    m_pVDev->SetFillColor(m_aProgressBarColor);
    m_pVDev->SetLineColor();
    m_pVDev->DrawRect(tools::Rectangle(m_nTlx + m_nBarspace, m_nTly + m_nBarspace,
                                       m_nTlx + m_nBarspace + length,
                                       m_nTly + m_nBarheight - m_nBarspace));
    vcl::Font aFont;
    aFont.SetFontSize(Size(0, 12));
    aFont.SetAlignment(ALIGN_BASELINE);
    m_pVDev->SetFont(aFont);
    m_pVDev->SetTextColor(m_aProgressTextColor);
    m_pVDev->DrawText(Point(m_nTlx, m_nTextBaseline), m_sProgressText);

    rRenderContext.DrawOutDev(Point(), GetOutputSizePixel(), Point(), m_pVDev->GetOutputSizePixel(),
                              *m_pVDev);
}

// Read keys from soffice{.ini|rc}:
static OUString implReadBootstrapKey(const OUString& _rKey)
{
    OUString sValue;
    rtl::Bootstrap::get(_rKey, sValue);
    return sValue;
}

void SplashScreenWindow::loadConfig()
{
    m_bShowLogo = implReadBootstrapKey(u"Logo"_ustr) != "0";

    OUString sProgressFrameColor = implReadBootstrapKey(u"ProgressFrameColor"_ustr);
    OUString sProgressBarColor = implReadBootstrapKey(u"ProgressBarColor"_ustr);
    OUString sProgressTextColor = implReadBootstrapKey(u"ProgressTextColor"_ustr);
    OUString sProgressTextBaseline = implReadBootstrapKey(u"ProgressTextBaseline"_ustr);
    OUString sSize = implReadBootstrapKey(u"ProgressSize"_ustr);
    OUString sPosition = implReadBootstrapKey(u"ProgressPosition"_ustr);
    OUString sNativeProgress = implReadBootstrapKey(u"NativeProgress"_ustr);

    if (!sProgressFrameColor.isEmpty())
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx));
        if (idx != -1)
        {
            nRed = static_cast<sal_uInt8>(temp);
            temp = o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx));
        }
        if (idx != -1)
        {
            sal_uInt8 nGreen = static_cast<sal_uInt8>(temp);
            sal_uInt8 nBlue = static_cast<sal_uInt8>(
                o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx)));
            m_aProgressFrameColor = Color(nRed, nGreen, nBlue);
        }
    }

    if (!sProgressBarColor.isEmpty())
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx));
        if (idx != -1)
        {
            nRed = static_cast<sal_uInt8>(temp);
            temp = o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx));
        }
        if (idx != -1)
        {
            sal_uInt8 nGreen = static_cast<sal_uInt8>(temp);
            sal_uInt8 nBlue = static_cast<sal_uInt8>(
                o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx)));
            m_aProgressBarColor = Color(nRed, nGreen, nBlue);
        }
    }

    if (!sProgressTextColor.isEmpty())
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx));
        if (idx != -1)
        {
            nRed = static_cast<sal_uInt8>(temp);
            temp = o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx));
        }
        if (idx != -1)
        {
            sal_uInt8 nGreen = static_cast<sal_uInt8>(temp);
            sal_uInt8 nBlue = static_cast<sal_uInt8>(
                o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx)));
            m_aProgressTextColor = Color(nRed, nGreen, nBlue);
        }
    }

    if (!sProgressTextBaseline.isEmpty())
    {
        m_nTextBaseline = sProgressTextBaseline.toInt32();
    }

    if (!sNativeProgress.isEmpty())
    {
        m_bNativeProgress = sNativeProgress.toBoolean();
    }

    if (!sSize.isEmpty())
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx));
        if (idx != -1)
        {
            m_nBarwidth = temp;
            m_nBarheight = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx));
        }
    }

    if (m_nBarheight >= 10)
        m_nBarspace = 3; // more space between frame and bar

    if (!sPosition.isEmpty())
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx));
        if (idx != -1)
        {
            m_nTlx = temp;
            m_nTly = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx));
        }
    }
}

void SplashScreenWindow::SetScreenBitmap(Bitmap& rBitmap)
{
    sal_Int32 nWidth(0);
    sal_Int32 nHeight(0);

    // determine desktop resolution
    sal_uInt32 nCount = Application::GetScreenCount();
    if (nCount > 0)
    {
        // retrieve size from first screen
        AbsoluteScreenPixelRectangle aScreenArea
            = Application::GetScreenPosSizePixel(static_cast<unsigned int>(0));
        nWidth = aScreenArea.GetWidth();
        nHeight = aScreenArea.GetHeight();
    }

    // create file name from screen resolution information
    OUString aResBuf = "_" + OUString::number(nWidth) + "x" + OUString::number(nHeight);
    if (!m_sAppName.isEmpty())
        if (Application::LoadBrandBitmap(Concat2View("intro_" + m_sAppName + aResBuf), rBitmap))
            return;

    if (Application::LoadBrandBitmap(Concat2View("intro" + aResBuf), rBitmap))
        return;

    (void)Application::LoadBrandBitmap(u"intro", rBitmap);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
