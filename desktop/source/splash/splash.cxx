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


#include "splash.hxx"
#include <stdio.h>
#include <unotools/bootstrap.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salnativewidgets.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/math.hxx>
#include <vcl/introwin.hxx>
#include <vcl/virdev.hxx>

#define NOT_LOADED  ((long)-1)

using namespace ::rtl;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;

namespace {

class  SplashScreen
    : public ::cppu::WeakImplHelper2< XStatusIndicator, XInitialization >
    , public IntroWindow
{
private:
    enum BitmapMode { BM_FULLSCREEN, BM_DEFAULTMODE };

    DECL_LINK( AppEventListenerHdl, VclWindowEvent * );
    virtual ~SplashScreen();
    void loadConfig();
    void updateStatus();
    void SetScreenBitmap(BitmapEx &rBitmap);
    void determineProgressRatioValues( double& rXRelPos, double& rYRelPos, double& rRelWidth, double& rRelHeight );

    static osl::Mutex _aMutex;

    VirtualDevice   _vdev;
    BitmapEx        _aIntroBmp;
    Color           _cProgressFrameColor;
    Color           _cProgressBarColor;
    Color           _cProgressTextColor;
    bool            _bNativeProgress;
    OUString        _sAppName;
    OUString        _sProgressText;

    sal_Int32   _iMax;
    sal_Int32   _iProgress;
    BitmapMode  _eBitmapMode;
    sal_Bool    _bPaintBitmap;
    sal_Bool    _bPaintProgress;
    sal_Bool    _bVisible;
    sal_Bool    _bShowLogo;
    sal_Bool    _bFullScreenSplash;
    sal_Bool    _bProgressEnd;
    long _height, _width, _tlx, _tly, _barwidth;
    long _barheight, _barspace, _textBaseline;
    double _fXPos, _fYPos;
    double _fWidth, _fHeight;
    const long _xoffset, _yoffset;

public:
    SplashScreen();

    // XStatusIndicator
    virtual void SAL_CALL end() throw ( RuntimeException );
    virtual void SAL_CALL reset() throw ( RuntimeException );
    virtual void SAL_CALL setText(const OUString& aText) throw ( RuntimeException );
    virtual void SAL_CALL setValue(sal_Int32 nValue) throw ( RuntimeException );
    virtual void SAL_CALL start(const OUString& aText, sal_Int32 nRange) throw ( RuntimeException );

    // XInitialize
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
        throw ( RuntimeException );

    // workwindow
    virtual void Paint( const Rectangle& );

};

SplashScreen::SplashScreen()
    : IntroWindow()
    , _vdev(*((IntroWindow*)this))
    , _cProgressFrameColor(sal::static_int_cast< ColorData >(NOT_LOADED))
    , _cProgressBarColor(sal::static_int_cast< ColorData >(NOT_LOADED))
    , _cProgressTextColor(sal::static_int_cast< ColorData >(NOT_LOADED))
    , _bNativeProgress(true)
    , _iMax(100)
    , _iProgress(0)
    , _eBitmapMode(BM_DEFAULTMODE)
    , _bPaintBitmap(sal_True)
    , _bPaintProgress(sal_False)
    , _bShowLogo(sal_True)
    , _bFullScreenSplash(sal_False)
    , _bProgressEnd(sal_False)
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
    , _xoffset(12)
    , _yoffset(18)
{
    loadConfig();
    _vdev.EnableRTL(IsRTLEnabled());
}

SplashScreen::~SplashScreen()
{
    Application::RemoveEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );
    Hide();

}

void SAL_CALL SplashScreen::start(const OUString&, sal_Int32 nRange)
    throw (RuntimeException)
{
    _iMax = nRange;
    if (_bVisible) {
        _bProgressEnd = sal_False;
        SolarMutexGuard aSolarGuard;
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( sal_True );
        Show();
        Paint(Rectangle());
        Flush();
    }
}

void SAL_CALL SplashScreen::end()
    throw (RuntimeException)
{
    _iProgress = _iMax;
    if (_bVisible )
    {
        if ( _eBitmapMode == BM_FULLSCREEN )
            EndFullScreenMode();
        Hide();
    }
    _bProgressEnd = sal_True;
}

void SAL_CALL SplashScreen::reset()
    throw (RuntimeException)
{
    _iProgress = 0;
    if (_bVisible && !_bProgressEnd )
    {
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( sal_True );
        Show();
        updateStatus();
    }
}

void SAL_CALL SplashScreen::setText(const OUString& rText)
    throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if ( _sProgressText != rText )
    {
        _sProgressText = rText;

        if (_bVisible && !_bProgressEnd)
        {
            if ( _eBitmapMode == BM_FULLSCREEN )
                ShowFullScreenMode( sal_True );
            Show();
            updateStatus();
        }
    }
}

void SAL_CALL SplashScreen::setValue(sal_Int32 nValue)
    throw (RuntimeException)
{
    SAL_INFO( "desktop.splash", "::SplashScreen::setValue (lo119109)" );
    SAL_INFO( "desktop.splash", "value=" << nValue );

    SolarMutexGuard aSolarGuard;
    if (_bVisible && !_bProgressEnd) {
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( sal_True );
        Show();
        if (nValue >= _iMax) _iProgress = _iMax;
    else _iProgress = nValue;
    updateStatus();
    }
}

// XInitialize
void SAL_CALL
SplashScreen::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
    throw (RuntimeException)
{
    ::osl::ClearableMutexGuard  aGuard( _aMutex );
    if (aArguments.getLength() > 0)
    {
        aArguments[0] >>= _bVisible;
        if (aArguments.getLength() > 1 )
            aArguments[1] >>= _sAppName;

        // start to determine bitmap and all other required value
        if ( _bShowLogo )
            SetScreenBitmap (_aIntroBmp);
        Size aSize = _aIntroBmp.GetSizePixel();
        SetOutputSizePixel( aSize );
        _vdev.SetOutputSizePixel( aSize );
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
            if (( _eBitmapMode == BM_FULLSCREEN ) &&
                _bFullScreenSplash )
            {
                if( ( _fXPos >= 0.0 ) && ( _fYPos >= 0.0 ))
                {
                    _tlx = sal_Int32( double( aSize.Width() ) * _fXPos );
                    _tly = sal_Int32( double( aSize.Height() ) * _fYPos );
                }
                if ( _fWidth >= 0.0 )
                    _barwidth  = sal_Int32( double( aSize.Width() ) * _fWidth );
                if ( _fHeight >= 0.0 )
                    _barheight = sal_Int32( double( aSize.Width() ) * _fHeight );
            }
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

        if ( sal::static_int_cast< ColorData >(NOT_LOADED) ==
             _cProgressFrameColor.GetColor() )
            _cProgressFrameColor = Color( COL_LIGHTGRAY );

        if ( sal::static_int_cast< ColorData >(NOT_LOADED) ==
             _cProgressBarColor.GetColor() )
        {
            // progress bar: new color only for big bitmap format
            if ( _width > 500 )
                _cProgressBarColor = Color( 157, 202, 18 );
            else
                _cProgressBarColor = Color( COL_BLUE );
        }

        if ( sal::static_int_cast< ColorData >(NOT_LOADED) ==
             _cProgressTextColor.GetColor() )
            _cProgressTextColor = Color( COL_BLACK );

        Application::AddEventListener(
            LINK( this, SplashScreen, AppEventListenerHdl ) );
    }
}

void SplashScreen::updateStatus()
{
    if (!_bVisible || _bProgressEnd) return;
    if (!_bPaintProgress) _bPaintProgress = sal_True;
    Paint(Rectangle());
    Flush();
}

// internal private methods
IMPL_LINK( SplashScreen, AppEventListenerHdl, VclWindowEvent *, inEvent )
{
    if ( inEvent != 0 )
    {
        switch ( inEvent->GetId() )
        {
            case VCLEVENT_WINDOW_SHOW:
                Paint( Rectangle() );
                break;
            default:
                break;
        }
    }
    return 0;
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
    _bShowLogo = implReadBootstrapKey( "Logo" ) != "0";

    OUString sProgressFrameColor = implReadBootstrapKey( "ProgressFrameColor" );
    OUString sProgressBarColor = implReadBootstrapKey( "ProgressBarColor" );
    OUString sProgressTextColor = implReadBootstrapKey( "ProgressTextColor" );
    OUString sProgressTextBaseline = implReadBootstrapKey( "ProgressTextBaseline" );
    OUString sSize = implReadBootstrapKey( "ProgressSize" );
    OUString sPosition = implReadBootstrapKey( "ProgressPosition" );
    OUString sFullScreenSplash = implReadBootstrapKey( "FullScreenSplash" );
    OUString sNativeProgress = implReadBootstrapKey( "NativeProgress" );


    // Determine full screen splash mode
    _bFullScreenSplash = (( !sFullScreenSplash.isEmpty() ) &&
                          ( !sFullScreenSplash.equals( "0" )));

    // Try to retrieve the relative values for the progress bar. The current
    // schema uses the screen ratio to retrieve the associated values.
    if ( _bFullScreenSplash )
        determineProgressRatioValues( _fXPos, _fYPos, _fWidth, _fHeight );

    if ( !sProgressFrameColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressFrameColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = sProgressFrameColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( sProgressFrameColor.getToken( 0, ',', idx ).toInt32() );
            _cProgressFrameColor = Color( nRed, nGreen, nBlue );
        }
    }

    if ( !sProgressBarColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressBarColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = sProgressBarColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( sProgressBarColor.getToken( 0, ',', idx ).toInt32() );
            _cProgressBarColor = Color( nRed, nGreen, nBlue );
        }
    }

    if ( !sProgressTextColor.isEmpty() )
    {
        sal_uInt8 nRed = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressTextColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< sal_uInt8 >( temp );
            temp = sProgressTextColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            sal_uInt8 nGreen = static_cast< sal_uInt8 >( temp );
            sal_uInt8 nBlue = static_cast< sal_uInt8 >( sProgressTextColor.getToken( 0, ',', idx ).toInt32() );
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
        sal_Int32 temp = sSize.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            _barwidth = temp;
            _barheight = sSize.getToken( 0, ',', idx ).toInt32();
        }
    }

    if ( _barheight >= 10 )
        _barspace = 3;  // more space between frame and bar

    if ( !sPosition.isEmpty() )
    {
        sal_Int32 idx = 0;
        sal_Int32 temp = sPosition.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            _tlx = temp;
            _tly = sPosition.getToken( 0, ',', idx ).toInt32();
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
        Rectangle aScreenArea = Application::GetScreenPosSizePixel((unsigned int)0);
        nWidth  = aScreenArea.GetWidth();
        nHeight = aScreenArea.GetHeight();
    }

    // create file name from screen resolution information
    OStringBuffer aStrBuf( 128 );
    OStringBuffer aResBuf( 32 );
    aStrBuf.append( "intro_" );
    if ( !_sAppName.isEmpty() )
    {
        aStrBuf.append( OUStringToOString(_sAppName, RTL_TEXTENCODING_UTF8) );
        aStrBuf.append( "_" );
    }
    aResBuf.append( OString::number( nWidth ));
    aResBuf.append( "x" );
    aResBuf.append( OString::number( nHeight ));

    aStrBuf.append( aResBuf.getStr() );
    if (Application::LoadBrandBitmap (aStrBuf.makeStringAndClear().getStr(), rBitmap))
        return;

    aStrBuf.append( "intro_" );
    aStrBuf.append( aResBuf.getStr() );
    if (Application::LoadBrandBitmap (aResBuf.makeStringAndClear().getStr(), rBitmap))
        return;

    Application::LoadBrandBitmap ("intro", rBitmap);
}

void SplashScreen::determineProgressRatioValues(
    double& rXRelPos, double& rYRelPos,
    double& rRelWidth, double& rRelHeight )
{
    sal_Int32 nWidth( 0 );
    sal_Int32 nHeight( 0 );
    sal_Int32 nScreenRatio( 0 );

    // determine desktop resolution
    sal_uInt32 nCount = Application::GetScreenCount();
    if ( nCount > 0 )
    {
        // retrieve size from first screen
        Rectangle aScreenArea = Application::GetScreenPosSizePixel((unsigned int)0);
        nWidth  = aScreenArea.GetWidth();
        nHeight = aScreenArea.GetHeight();
        nScreenRatio  = sal_Int32( math::round( double( nWidth ) / double( nHeight ), 2 ) * 100 );
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
            sal_Int32 nRatio = sal_Int32( math::round( fRatio, 2 ) * 100 );
            if ( nRatio == nScreenRatio )
            {
                OUString sFullScreenProgressPos = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressPos ) );
                OUString sFullScreenProgressSize = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressSize ) );

                if ( !sFullScreenProgressPos.isEmpty() )
                {
                    sal_Int32 idx = 0;
                    double temp = sFullScreenProgressPos.getToken( 0, ',', idx ).toDouble();
                    if ( idx != -1 )
                    {
                        rXRelPos = temp;
                        rYRelPos = sFullScreenProgressPos.getToken( 0, ',', idx ).toDouble();
                    }
                }

                if ( !sFullScreenProgressSize.isEmpty() )
                {
                    sal_Int32 idx = 0;
                    double temp = sFullScreenProgressSize.getToken( 0, ',', idx ).toDouble();
                    if ( idx != -1 )
                    {
                        rRelWidth  = temp;
                        rRelHeight = sFullScreenProgressSize.getToken( 0, ',', idx ).toDouble();
                    }
                }
            }
        }
        else
            break;
    }
}

void SplashScreen::Paint( const Rectangle&)
{
    if(!_bVisible) return;

    //native drawing
    sal_Bool bNativeOK = sal_False;

    // in case of native controls we need to draw directly to the window
    if( _bNativeProgress && IsNativeControlSupported( CTRL_INTROPROGRESS, PART_ENTIRE_CONTROL ) )
    {
        DrawBitmapEx( Point(), _aIntroBmp );

        ImplControlValue aValue( _iProgress * _barwidth / _iMax);
        Rectangle aDrawRect( Point(_tlx, _tly), Size( _barwidth, _barheight ) );
        Rectangle aNativeControlRegion, aNativeContentRegion;

        if( GetNativeControlRegion( CTRL_INTROPROGRESS, PART_ENTIRE_CONTROL, aDrawRect,
                                             CTRL_STATE_ENABLED, aValue, OUString(),
                                             aNativeControlRegion, aNativeContentRegion ) )
        {
              long nProgressHeight = aNativeControlRegion.GetHeight();
              aDrawRect.Top() -= (nProgressHeight - _barheight)/2;
              aDrawRect.Bottom() += (nProgressHeight - _barheight)/2;
        }

        if( (bNativeOK = DrawNativeControl( CTRL_INTROPROGRESS, PART_ENTIRE_CONTROL, aDrawRect,
                                            CTRL_STATE_ENABLED, aValue, _sProgressText )) != sal_False )
        {
            return;
        }
    }
    //non native drawing
    // draw bitmap
    if (_bPaintBitmap)
        _vdev.DrawBitmapEx( Point(), _aIntroBmp );

    if (_bPaintProgress) {
        // draw progress...
        long length = (_iProgress * _barwidth / _iMax) - (2 * _barspace);
        if (length < 0) length = 0;

        // border
        _vdev.SetFillColor();
        _vdev.SetLineColor( _cProgressFrameColor );
        _vdev.DrawRect(Rectangle(_tlx, _tly, _tlx+_barwidth, _tly+_barheight));
        _vdev.SetFillColor( _cProgressBarColor );
        _vdev.SetLineColor();
        _vdev.DrawRect(Rectangle(_tlx+_barspace, _tly+_barspace, _tlx+_barspace+length, _tly+_barheight-_barspace));
        Font aFont;
        aFont.SetSize(Size(0, 12));
        aFont.SetAlign(ALIGN_BASELINE);
        _vdev.SetFont(aFont);
        _vdev.SetTextColor(_cProgressTextColor);
        _vdev.DrawText(Point(_tlx, _textBaseline), _sProgressText);
    }
    DrawOutDev(Point(), GetOutputSizePixel(), Point(), _vdev.GetOutputSizePixel(), _vdev );
}


// get service instance...
osl::Mutex SplashScreen::_aMutex;

}

css::uno::Reference< css::uno::XInterface > desktop::splash::create(
    css::uno::Reference< css::uno::XComponentContext > const &)
{
    return static_cast< cppu::OWeakObject * >(new SplashScreen);
}

OUString desktop::splash::getImplementationName() {
    return OUString("com.sun.star.office.comp.SplashScreen");
}

css::uno::Sequence< OUString > desktop::splash::getSupportedServiceNames() {
    OUString name("com.sun.star.office.SplashScreen");
    return css::uno::Sequence< OUString >(&name, 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
