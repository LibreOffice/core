/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "splash.hxx"
#include <stdio.h>
#include <unotools/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/stream.hxx>
#include <sfx2/sfx.hrc>
#include <vcl/svapp.hxx>
#include <vcl/salnativewidgets.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <rtl/bootstrap.hxx>
#include <rtl/logfile.hxx>
#include <rtl/locale.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/math.hxx>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>

#define NOT_LOADED  ((long)-1)

using namespace ::rtl;
using namespace ::com::sun::star::registry;

namespace desktop
{

SplashScreen::SplashScreen(const Reference< XMultiServiceFactory >& rSMgr)
    : IntroWindow()
    , _vdev(*((IntroWindow*)this))
    , _cProgressFrameColor(sal::static_int_cast< ColorData >(NOT_LOADED))
    , _cProgressBarColor(sal::static_int_cast< ColorData >(NOT_LOADED))
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
    , _fXPos(-1.0)
    , _fYPos(-1.0)
    , _fWidth(-1.0)
    , _fHeight(-1.0)
    , _xoffset(12)
    , _yoffset(18)
{
    _rFactory = rSMgr;

    loadConfig();
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
    RTL_LOGFILE_CONTEXT( aLog, "::SplashScreen::setValue (lo119109)" );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "value=%d", nValue );

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

        Application::AddEventListener(
            LINK( this, SplashScreen, AppEventListenerHdl ) );

        SetBackgroundBitmap( _aIntroBmp );
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

// Read keys from edition/edition.ini or soffice{.ini|rc}:
OUString implReadBootstrapKey( const OUString& _rKey )
{
    OUString sValue(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM(
                "${.override:${BRAND_BASE_DIR}/program/edition/edition.ini:")) +
        _rKey + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("}")));
    rtl::Bootstrap::expandMacros(sValue);
    return sValue;
}

void SplashScreen::loadConfig()
{
    _bShowLogo = !implReadBootstrapKey(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Logo"))).
        equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("0"));

    OUString sProgressFrameColor = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressFrameColor" ) ) );
    OUString sProgressBarColor = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressBarColor" ) ) );
    OUString sSize = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressSize" ) ) );
    OUString sPosition = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressPosition" ) ) );
    OUString sFullScreenSplash = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "FullScreenSplash" ) ) );
    OUString sNativeProgress = implReadBootstrapKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM( "NativeProgress" ) ) );


    // Determine full screen splash mode
    _bFullScreenSplash = (( sFullScreenSplash.getLength() > 0 ) &&
                          ( !sFullScreenSplash.equalsAsciiL( "0", 1 )));

    // Try to retrieve the relative values for the progress bar. The current
    // schema uses the screen ratio to retrieve the associated values.
    if ( _bFullScreenSplash )
        determineProgressRatioValues( _fXPos, _fYPos, _fWidth, _fHeight );

    if ( sProgressFrameColor.getLength() )
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

    if ( sProgressBarColor.getLength() )
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

    if( sNativeProgress.getLength() )
    {
        _bNativeProgress = sNativeProgress.toBoolean();
    }

    if ( sSize.getLength() )
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

    if ( sPosition.getLength() )
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
    if ( _sAppName.getLength() > 0 )
    {
        aStrBuf.append( OUStringToOString(_sAppName, RTL_TEXTENCODING_UTF8) );
        aStrBuf.append( "_" );
    }
    aResBuf.append( OString::valueOf( nWidth ));
    aResBuf.append( "x" );
    aResBuf.append( OString::valueOf( nHeight ));

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

        if ( sFullScreenProgressRatio.getLength() > 0 )
        {
            double fRatio = sFullScreenProgressRatio.toDouble();
            sal_Int32 nRatio = sal_Int32( math::round( fRatio, 2 ) * 100 );
            if ( nRatio == nScreenRatio )
            {
                OUString sFullScreenProgressPos = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressPos ) );
                OUString sFullScreenProgressSize = implReadBootstrapKey(
                    OUString::createFromAscii( szFullScreenProgressSize ) );

                if ( sFullScreenProgressPos.getLength() )
                {
                    sal_Int32 idx = 0;
                    double temp = sFullScreenProgressPos.getToken( 0, ',', idx ).toDouble();
                    if ( idx != -1 )
                    {
                        rXRelPos = temp;
                        rYRelPos = sFullScreenProgressPos.getToken( 0, ',', idx ).toDouble();
                    }
                }

                if ( sFullScreenProgressSize.getLength() )
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
                                             CTRL_STATE_ENABLED, aValue, rtl::OUString(),
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
        _vdev.DrawText( Rectangle(_tlx, _tly+_barheight+5, _tlx+_barwidth, _tly+_barheight+5+20), _sProgressText, TEXT_DRAW_CENTER );
    }
    DrawOutDev(Point(), GetOutputSizePixel(), Point(), _vdev.GetOutputSizePixel(), _vdev );
}


// get service instance...
SplashScreen *SplashScreen::_pINSTANCE = NULL;
osl::Mutex SplashScreen::_aMutex;

Reference< XInterface > SplashScreen::getInstance(const Reference< XMultiServiceFactory >& rSMgr)
{
    if ( _pINSTANCE == 0 )
    {
        osl::MutexGuard guard(_aMutex);
        if (_pINSTANCE == 0)
            return (XComponent*)new SplashScreen(rSMgr);
    }

    return (XComponent*)0;
}

// static service info...
const char* SplashScreen::interfaces[] =
{
    "com.sun.star.task.XStartusIndicator",
    "com.sun.star.lang.XInitialization",
    NULL,
};
const sal_Char *SplashScreen::serviceName = "com.sun.star.office.SplashScreen";
const sal_Char *SplashScreen::implementationName = "com.sun.star.office.comp.SplashScreen";
const sal_Char *SplashScreen::supportedServiceNames[] = {"com.sun.star.office.SplashScreen", NULL};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
