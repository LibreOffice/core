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
#include <vcl/salnativewidgets.hxx>

#include <rtl/bootstrap.hxx>
#include <rtl/math.hxx>
#include <o3tl/string_view.hxx>

#include <mutex>

#define NOT_LOADED  (tools::Long(-1))
#define NOT_LOADED_COLOR  (Color(ColorTransparency, 0xffffffff))

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;

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

void SplashScreenWindow::Redraw()
{
    Invalidate();
    // Trigger direct painting too - otherwise the splash screen won't be
    // shown in some cases (when the idle timer won't be hit).
    Paint(*GetOutDev(), tools::Rectangle());
    GetOutDev()->Flush();
}

SplashScreen::SplashScreen()
    : m_pWindow(VclPtr<SplashScreenWindow>::Create(this))
    , m_aProgressFrameColor(NOT_LOADED_COLOR)
    , m_aProgressBarColor(NOT_LOADED_COLOR)
    , m_aProgressTextColor(NOT_LOADED_COLOR)
    , m_bNativeProgress(true)
    , m_nMax(100)
    , m_nProgress(0)
    , m_bPaintProgress(false)
    , m_bShowLogo(true)
    , m_bFullScreenSplash(false)
    , m_bProgressEnd(false)
    , m_nHeight(0)
    , m_nWidth(0)
    , m_nTlx(NOT_LOADED)
    , m_nTly(NOT_LOADED)
    , m_nBarwidth(NOT_LOADED)
    , m_nBarheight(NOT_LOADED)
    , m_nBarspace(2)
    , m_nTextBaseline(NOT_LOADED)
    , m_fXPos(-1.0)
    , m_fYPos(-1.0)
    , m_fWidth(-1.0)
    , m_fHeight(-1.0)
{
    loadConfig();
}

SplashScreen::~SplashScreen()
{
    Application::RemoveEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
    m_pWindow->Hide();
    m_pWindow.disposeAndClear();
}

void SAL_CALL SplashScreen::start(const OUString&, sal_Int32 nRange)
{
    m_nMax = nRange;
    m_bProgressEnd = false;
    SolarMutexGuard aSolarGuard;
    m_pWindow->Show();
    m_pWindow->Redraw();
}

void SAL_CALL SplashScreen::end()
{
    m_nProgress = m_nMax;
    m_pWindow->Hide();
    m_bProgressEnd = true;
}

void SAL_CALL SplashScreen::reset()
{
    m_nProgress = 0;
    if (!m_bProgressEnd)
    {
        m_pWindow->Show();
        updateStatus();
    }
}

void SAL_CALL SplashScreen::setText(const OUString& rText)
{
    SolarMutexGuard aSolarGuard;
    if (m_sProgressText != rText)
    {
        m_sProgressText = rText;

        if (!m_bProgressEnd)
        {
            m_pWindow->Show();
            updateStatus();
        }
    }
}

void SAL_CALL SplashScreen::setValue(sal_Int32 nValue)
{
    SAL_INFO( "desktop.splash", "setValue: " << nValue );

    SolarMutexGuard aSolarGuard;
    if (!m_bProgressEnd)
    {
        m_pWindow->Show();
        if (nValue >= m_nMax)
            m_nProgress = m_nMax;
        else
            m_nProgress = nValue;
        updateStatus();
    }
}

// XInitialize
void SAL_CALL
SplashScreen::initialize( const css::uno::Sequence< css::uno::Any>& aArguments )
{
    static std::mutex aMutex;
    std::lock_guard  aGuard( aMutex );
    if (!aArguments.hasElements())
        return;

    aArguments[0] >>= m_sAppName;

    // start to determine bitmap and all other required value
    if (m_bShowLogo)
        SetScreenBitmap(m_aIntroBmp);
    Size aSize = m_aIntroBmp.GetSizePixel();
    m_pWindow->SetOutputSizePixel(aSize);
    m_pWindow->m_pVDev->SetOutputSizePixel(aSize);
    m_nHeight = aSize.Height();
    m_nWidth = aSize.Width();
    if (m_nWidth > 500)
    {
        Point xtopleft(212,216);
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

    Application::AddEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
}

void SplashScreen::updateStatus()
{
    if (m_bProgressEnd)
        return;
    if (!m_bPaintProgress)
        m_bPaintProgress = true;
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

// Read keys from soffice{.ini|rc}:
static OUString implReadBootstrapKey(const OUString& _rKey)
{
    OUString sValue;
    rtl::Bootstrap::get(_rKey, sValue);
    return sValue;
}

void SplashScreen::loadConfig()
{
    m_bShowLogo = implReadBootstrapKey(u"Logo"_ustr) != "0";

    OUString sProgressFrameColor = implReadBootstrapKey( u"ProgressFrameColor"_ustr );
    OUString sProgressBarColor = implReadBootstrapKey( u"ProgressBarColor"_ustr );
    OUString sProgressTextColor = implReadBootstrapKey( u"ProgressTextColor"_ustr );
    OUString sProgressTextBaseline = implReadBootstrapKey( u"ProgressTextBaseline"_ustr );
    OUString sSize = implReadBootstrapKey( u"ProgressSize"_ustr );
    OUString sPosition = implReadBootstrapKey( u"ProgressPosition"_ustr );
    OUString sFullScreenSplash = implReadBootstrapKey( u"FullScreenSplash"_ustr );
    OUString sNativeProgress = implReadBootstrapKey( u"NativeProgress"_ustr );


    // Determine full screen splash mode
    m_bFullScreenSplash = ((!sFullScreenSplash.isEmpty()) && (sFullScreenSplash != "0"));

    // Try to retrieve the relative values for the progress bar. The current
    // schema uses the screen ratio to retrieve the associated values.
    if (m_bFullScreenSplash)
        determineProgressRatioValues(m_fXPos, m_fYPos, m_fWidth, m_fHeight);

    if ( !sProgressFrameColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx ));
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx ));
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( o3tl::toInt32(o3tl::getToken(sProgressFrameColor, 0, ',', idx )) );
            m_aProgressFrameColor = Color(nRed, nGreen, nBlue);
        }
    }

    if ( !sProgressBarColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx ));
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx ));
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( o3tl::toInt32(o3tl::getToken(sProgressBarColor, 0, ',', idx )) );
            m_aProgressBarColor = Color(nRed, nGreen, nBlue);
        }
    }

    if ( !sProgressTextColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx ));
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx ));
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( o3tl::toInt32(o3tl::getToken(sProgressTextColor, 0, ',', idx )) );
            m_aProgressTextColor = Color(nRed, nGreen, nBlue);
        }
    }

    if ( !sProgressTextBaseline.isEmpty() )
    {
        m_nTextBaseline = sProgressTextBaseline.toInt32();
    }

    if( !sNativeProgress.isEmpty() )
    {
        m_bNativeProgress = sNativeProgress.toBoolean();
    }

    if ( !sSize.isEmpty() )
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx ));
        if ( idx != -1 )
        {
            m_nBarwidth = temp;
            m_nBarheight = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx));
        }
    }

    if (m_nBarheight >= 10)
        m_nBarspace = 3; // more space between frame and bar

    if ( !sPosition.isEmpty() )
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx ));
        if ( idx != -1 )
        {
            m_nTlx = temp;
            m_nTly = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx));
        }
    }
}

void SplashScreen::SetScreenBitmap(Bitmap& rBitmap)
{
    sal_Int32 nWidth( 0 );
    sal_Int32 nHeight( 0 );

    // determine desktop resolution
    sal_uInt32 nCount = Application::GetScreenCount();
    if ( nCount > 0 )
    {
        // retrieve size from first screen
        AbsoluteScreenPixelRectangle aScreenArea = Application::GetScreenPosSizePixel(static_cast<unsigned int>(0));
        nWidth  = aScreenArea.GetWidth();
        nHeight = aScreenArea.GetHeight();
    }

    // create file name from screen resolution information
    OUString aResBuf = "_" + OUString::number(nWidth) + "x" + OUString::number(nHeight);
    if (!m_sAppName.isEmpty())
        if (Application::LoadBrandBitmap(Concat2View("intro_" + m_sAppName + aResBuf), rBitmap))
            return;

    if (Application::LoadBrandBitmap(Concat2View("intro" + aResBuf), rBitmap))
        return;

    (void)Application::LoadBrandBitmap (u"intro", rBitmap);
}

void SplashScreen::determineProgressRatioValues(
    double& rXRelPos, double& rYRelPos,
    double& rRelWidth, double& rRelHeight )
{
    sal_Int32 nScreenRatio( 0 );

    // determine desktop resolution
    sal_uInt32 nCount = Application::GetScreenCount();
    if ( nCount > 0 )
    {
        // retrieve size from first screen
        AbsoluteScreenPixelRectangle aScreenArea = Application::GetScreenPosSizePixel(static_cast<unsigned int>(0));
        sal_Int32 nWidth  = aScreenArea.GetWidth();
        sal_Int32 nHeight = aScreenArea.GetHeight();
        nScreenRatio  = nHeight ? sal_Int32( rtl::math::round( double( nWidth ) / double( nHeight ), 2 ) * 100 ) :  0;
    }

    char szFullScreenProgressRatio[] = "FullScreenProgressRatio0";
    char szFullScreenProgressPos[]   = "FullScreenProgressPos0";
    char szFullScreenProgressSize[]  = "FullScreenProgressSize0";
    for ( sal_Int32 i = 0; i <= 9; i++ )
    {
        char cNum = '0' + char( i );
        szFullScreenProgressRatio[23] = cNum;
        szFullScreenProgressPos[21]   = cNum;
        szFullScreenProgressSize[22]  = cNum;

        OUString sFullScreenProgressRatio = implReadBootstrapKey(
            OUString::createFromAscii( szFullScreenProgressRatio ) );

        if ( !sFullScreenProgressRatio.isEmpty() )
        {
            double fRatio = sFullScreenProgressRatio.toDouble();
            sal_Int32 nRatio = sal_Int32( rtl::math::round( fRatio, 2 ) * 100 );
            if ( nRatio == nScreenRatio )
            {
                OUString sFullScreenProgressPos = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressPos ) );
                OUString sFullScreenProgressSize = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressSize ) );

                if ( !sFullScreenProgressPos.isEmpty() )
                {
                    sal_Int32 idx = 0;
                    double temp = o3tl::toDouble(o3tl::getToken(sFullScreenProgressPos, 0, ',', idx ));
                    if ( idx != -1 )
                    {
                        rXRelPos = temp;
                        rYRelPos = o3tl::toDouble(o3tl::getToken(sFullScreenProgressPos, 0, ',', idx ));
                    }
                }

                if ( !sFullScreenProgressSize.isEmpty() )
                {
                    sal_Int32 idx = 0;
                    double temp = o3tl::toDouble(o3tl::getToken(sFullScreenProgressSize, 0, ',', idx ));
                    if ( idx != -1 )
                    {
                        rRelWidth  = temp;
                        rRelHeight = o3tl::toDouble(o3tl::getToken(sFullScreenProgressSize, 0, ',', idx ));
                    }
                }
            }
        }
        else
            break;
    }
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

        if (rRenderContext.GetNativeControlRegion(ControlType::IntroProgress, ControlPart::Entire, aDrawRect,
                                                  ControlState::ENABLED, aValue,
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
        if (length < 0) length = 0;

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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_SplashScreen_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SplashScreen());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
