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


#include <sal/log.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salnativewidgets.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/math.hxx>
#include <vcl/introwin.hxx>
#include <vcl/virdev.hxx>
#include <o3tl/string_view.hxx>

#include <mutex>

#define NOT_LOADED  (tools::Long(-1))
#define NOT_LOADED_COLOR  (Color(ColorTransparency, 0xffffffff))

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;

namespace {

class SplashScreen;

class SplashScreenWindow : public IntroWindow
{
public:
    SplashScreen *pSpl;
    ScopedVclPtr<VirtualDevice> _vdev;
    explicit SplashScreenWindow(SplashScreen *);
    virtual ~SplashScreenWindow() override { disposeOnce(); }
    virtual void dispose() override;
    // workwindow
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    void Redraw();

};

class  SplashScreen
    : public ::cppu::WeakImplHelper< XStatusIndicator, XInitialization, XServiceInfo >
{
    friend class SplashScreenWindow;
private:
    VclPtr<SplashScreenWindow> pWindow;

    DECL_LINK( AppEventListenerHdl, VclSimpleEvent&, void );
    virtual ~SplashScreen() override;
    void loadConfig();
    void updateStatus();
    void SetScreenBitmap(BitmapEx &rBitmap);
    static void determineProgressRatioValues( double& rXRelPos, double& rYRelPos, double& rRelWidth, double& rRelHeight );

    BitmapEx        _aIntroBmp;
    Color           _cProgressFrameColor;
    Color           _cProgressBarColor;
    Color           _cProgressTextColor;
    bool            _bNativeProgress;
    OUString        _sAppName;
    OUString        _sProgressText;

    sal_Int32   _iMax;
    sal_Int32   _iProgress;
    bool        _bPaintProgress;
    bool        _bVisible;
    bool        _bShowLogo;
    bool        _bFullScreenSplash;
    bool        _bProgressEnd;
    tools::Long        _height, _width, _tlx, _tly, _barwidth;
    tools::Long        _barheight, _barspace, _textBaseline;
    double      _fXPos, _fYPos;
    double      _fWidth, _fHeight;
    static constexpr tools::Long  _xoffset = 12, _yoffset = 18;

public:
    SplashScreen();

    // XStatusIndicator
    virtual void SAL_CALL end() override;
    virtual void SAL_CALL reset() override;
    virtual void SAL_CALL setText(const OUString& aText) override;
    virtual void SAL_CALL setValue(sal_Int32 nValue) override;
    virtual void SAL_CALL start(const OUString& aText, sal_Int32 nRange) override;

    // XInitialize
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any>& aArguments ) override;

    virtual OUString SAL_CALL getImplementationName() override
    { return u"com.sun.star.office.comp.SplashScreen"_ustr; }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return { u"com.sun.star.office.SplashScreen"_ustr }; }
};

SplashScreenWindow::SplashScreenWindow(SplashScreen *pSplash)
    : pSpl( pSplash )
    , _vdev(VclPtr<VirtualDevice>::Create(*GetOutDev()))
{
    _vdev->EnableRTL(IsRTLEnabled());
}

void SplashScreenWindow::dispose()
{
    pSpl = nullptr;
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
    : pWindow( VclPtr<SplashScreenWindow>::Create(this) )
    , _cProgressFrameColor(NOT_LOADED_COLOR)
    , _cProgressBarColor(NOT_LOADED_COLOR)
    , _cProgressTextColor(NOT_LOADED_COLOR)
    , _bNativeProgress(true)
    , _iMax(100)
    , _iProgress(0)
    , _bPaintProgress(false)
    , _bVisible(true)
    , _bShowLogo(true)
    , _bFullScreenSplash(false)
    , _bProgressEnd(false)
    , _height(0)
    , _width(0)
    , _tlx(NOT_LOADED)
    , _tly(NOT_LOADED)
    , _barwidth(NOT_LOADED)
    , _barheight(NOT_LOADED)
    , _barspace(2)
    , _textBaseline(NOT_LOADED)
    , _fXPos(-1.0)
    , _fYPos(-1.0)
    , _fWidth(-1.0)
    , _fHeight(-1.0)
{
    loadConfig();
}

SplashScreen::~SplashScreen()
{
    Application::RemoveEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
    pWindow->Hide();
    pWindow.disposeAndClear();
}

void SAL_CALL SplashScreen::start(const OUString&, sal_Int32 nRange)
{
    _iMax = nRange;
    if (_bVisible) {
        _bProgressEnd = false;
        SolarMutexGuard aSolarGuard;
        pWindow->Show();
        pWindow->Redraw();
    }
}

void SAL_CALL SplashScreen::end()
{
    _iProgress = _iMax;
    if (_bVisible )
    {
        pWindow->Hide();
    }
    _bProgressEnd = true;
}

void SAL_CALL SplashScreen::reset()
{
    _iProgress = 0;
    if (_bVisible && !_bProgressEnd )
    {
        pWindow->Show();
        updateStatus();
    }
}

void SAL_CALL SplashScreen::setText(const OUString& rText)
{
    SolarMutexGuard aSolarGuard;
    if ( _sProgressText != rText )
    {
        _sProgressText = rText;

        if (_bVisible && !_bProgressEnd)
        {
            pWindow->Show();
            updateStatus();
        }
    }
}

void SAL_CALL SplashScreen::setValue(sal_Int32 nValue)
{
    SAL_INFO( "desktop.splash", "setValue: " << nValue );

    SolarMutexGuard aSolarGuard;
    if (_bVisible && !_bProgressEnd) {
        pWindow->Show();
        if (nValue >= _iMax)
            _iProgress = _iMax;
        else
            _iProgress = nValue;
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

    aArguments[0] >>= _bVisible;
    if (aArguments.getLength() > 1 )
        aArguments[1] >>= _sAppName;

    // start to determine bitmap and all other required value
    if ( _bShowLogo )
        SetScreenBitmap (_aIntroBmp);
    Size aSize = _aIntroBmp.GetSizePixel();
    pWindow->SetOutputSizePixel( aSize );
    pWindow->_vdev->SetOutputSizePixel( aSize );
    _height = aSize.Height();
    _width = aSize.Width();
    if (_width > 500)
    {
        Point xtopleft(212,216);
        if ( NOT_LOADED == _tlx || NOT_LOADED == _tly )
        {
            _tlx = xtopleft.X();    // top-left x
            _tly = xtopleft.Y();    // top-left y
        }
        if ( NOT_LOADED == _barwidth )
            _barwidth = 263;
        if ( NOT_LOADED == _barheight )
            _barheight = 8;
    }
    else
    {
        if ( NOT_LOADED == _barwidth )
            _barwidth  = _width - (2 * _xoffset);
        if ( NOT_LOADED == _barheight )
            _barheight = 6;
        if ( NOT_LOADED == _tlx || NOT_LOADED == _tly )
        {
            _tlx = _xoffset;           // top-left x
            _tly = _height - _yoffset; // top-left y
        }
    }

    if ( NOT_LOADED == _textBaseline )
        _textBaseline = _height;

    if ( NOT_LOADED_COLOR == _cProgressFrameColor )
        _cProgressFrameColor = COL_LIGHTGRAY;

    if ( NOT_LOADED_COLOR == _cProgressBarColor )
    {
        // progress bar: new color only for big bitmap format
        if ( _width > 500 )
            _cProgressBarColor = Color( 157, 202, 18 );
        else
            _cProgressBarColor = COL_BLUE;
    }

    if ( NOT_LOADED_COLOR == _cProgressTextColor )
        _cProgressTextColor = COL_BLACK;

    Application::AddEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
}

void SplashScreen::updateStatus()
{
    if (!_bVisible || _bProgressEnd)
        return;
    if (!_bPaintProgress)
        _bPaintProgress = true;
    pWindow->Redraw();
}

// internal private methods
IMPL_LINK( SplashScreen, AppEventListenerHdl, VclSimpleEvent&, inEvent, void )
{
    if (static_cast<VclWindowEvent&>(inEvent).GetWindow() == pWindow)
    {
        switch ( inEvent.GetId() )
        {
            case VclEventId::WindowShow:
                pWindow->Redraw();
                break;
            default:
                break;
        }
    }
}

// Read keys from soffice{.ini|rc}:
OUString implReadBootstrapKey( const OUString& _rKey )
{
    OUString sValue;
    rtl::Bootstrap::get(_rKey, sValue);
    return sValue;
}

void SplashScreen::loadConfig()
{
    _bShowLogo = implReadBootstrapKey( u"Logo"_ustr ) != "0";

    OUString sProgressFrameColor = implReadBootstrapKey( u"ProgressFrameColor"_ustr );
    OUString sProgressBarColor = implReadBootstrapKey( u"ProgressBarColor"_ustr );
    OUString sProgressTextColor = implReadBootstrapKey( u"ProgressTextColor"_ustr );
    OUString sProgressTextBaseline = implReadBootstrapKey( u"ProgressTextBaseline"_ustr );
    OUString sSize = implReadBootstrapKey( u"ProgressSize"_ustr );
    OUString sPosition = implReadBootstrapKey( u"ProgressPosition"_ustr );
    OUString sFullScreenSplash = implReadBootstrapKey( u"FullScreenSplash"_ustr );
    OUString sNativeProgress = implReadBootstrapKey( u"NativeProgress"_ustr );


    // Determine full screen splash mode
    _bFullScreenSplash = (( !sFullScreenSplash.isEmpty() ) &&
                          ( sFullScreenSplash != "0" ));

    // Try to retrieve the relative values for the progress bar. The current
    // schema uses the screen ratio to retrieve the associated values.
    if ( _bFullScreenSplash )
        determineProgressRatioValues( _fXPos, _fYPos, _fWidth, _fHeight );

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
            _cProgressFrameColor = Color( nRed, nGreen, nBlue );
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
            _cProgressBarColor = Color( nRed, nGreen, nBlue );
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
            _cProgressTextColor = Color( nRed, nGreen, nBlue );
        }
    }

    if ( !sProgressTextBaseline.isEmpty() )
    {
        _textBaseline = sProgressTextBaseline.toInt32();
    }

    if( !sNativeProgress.isEmpty() )
    {
        _bNativeProgress = sNativeProgress.toBoolean();
    }

    if ( !sSize.isEmpty() )
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx ));
        if ( idx != -1 )
        {
            _barwidth = temp;
            _barheight = o3tl::toInt32(o3tl::getToken(sSize, 0, ',', idx ));
        }
    }

    if ( _barheight >= 10 )
        _barspace = 3;  // more space between frame and bar

    if ( !sPosition.isEmpty() )
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx ));
        if ( idx != -1 )
        {
            _tlx = temp;
            _tly = o3tl::toInt32(o3tl::getToken(sPosition, 0, ',', idx ));
        }
    }
}

void SplashScreen::SetScreenBitmap(BitmapEx &rBitmap)
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
    if ( !_sAppName.isEmpty() )
        if (Application::LoadBrandBitmap(Concat2View("intro_" + _sAppName + aResBuf), rBitmap))
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
    if (!pSpl || !pSpl->_bVisible)
        return;

    //native drawing
    // in case of native controls we need to draw directly to the window
    if (pSpl->_bNativeProgress && rRenderContext.IsNativeControlSupported(ControlType::IntroProgress, ControlPart::Entire))
    {
        rRenderContext.DrawBitmapEx(Point(), pSpl->_aIntroBmp);

        ImplControlValue aValue( pSpl->_iProgress * pSpl->_barwidth / pSpl->_iMax);
        tools::Rectangle aDrawRect( Point(pSpl->_tlx, pSpl->_tly), Size( pSpl->_barwidth, pSpl->_barheight));
        tools::Rectangle aNativeControlRegion, aNativeContentRegion;

        if (rRenderContext.GetNativeControlRegion(ControlType::IntroProgress, ControlPart::Entire, aDrawRect,
                                                  ControlState::ENABLED, aValue,
                                                  aNativeControlRegion, aNativeContentRegion))
        {
              tools::Long nProgressHeight = aNativeControlRegion.GetHeight();
              aDrawRect.AdjustTop( -((nProgressHeight - pSpl->_barheight)/2) );
              aDrawRect.AdjustBottom((nProgressHeight - pSpl->_barheight)/2 );
        }

        if (rRenderContext.DrawNativeControl(ControlType::IntroProgress, ControlPart::Entire, aDrawRect,
                                             ControlState::ENABLED, aValue, pSpl->_sProgressText))
        {
            return;
        }
    }

    // non native drawing
    // draw bitmap
    _vdev->DrawBitmapEx(Point(), pSpl->_aIntroBmp);

    if (pSpl->_bPaintProgress) {
        // draw progress...
        tools::Long length = (pSpl->_iProgress * pSpl->_barwidth / pSpl->_iMax) - (2 * pSpl->_barspace);
        if (length < 0) length = 0;

        // border
        _vdev->SetFillColor();
        _vdev->SetLineColor( pSpl->_cProgressFrameColor );
        _vdev->DrawRect(tools::Rectangle(pSpl->_tlx, pSpl->_tly, pSpl->_tlx+pSpl->_barwidth, pSpl->_tly+pSpl->_barheight));
        _vdev->SetFillColor( pSpl->_cProgressBarColor );
        _vdev->SetLineColor();
        _vdev->DrawRect(tools::Rectangle(pSpl->_tlx+pSpl->_barspace, pSpl->_tly+pSpl->_barspace, pSpl->_tlx+pSpl->_barspace+length, pSpl->_tly+pSpl->_barheight-pSpl->_barspace));
        vcl::Font aFont;
        aFont.SetFontSize(Size(0, 12));
        aFont.SetAlignment(ALIGN_BASELINE);
        _vdev->SetFont(aFont);
        _vdev->SetTextColor(pSpl->_cProgressTextColor);
        _vdev->DrawText(Point(pSpl->_tlx, pSpl->_textBaseline), pSpl->_sProgressText);
    }
    rRenderContext.DrawOutDev(Point(), GetOutputSizePixel(), Point(), _vdev->GetOutputSizePixel(), *_vdev);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_SplashScreen_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SplashScreen());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
