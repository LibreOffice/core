/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: splash.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 09:14:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "splash.hxx"
#include <stdio.h>
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <rtl/logfile.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>

#define NOT_LOADED  ((long)-1)

using namespace ::rtl;
using namespace ::com::sun::star::registry;

namespace desktop
{

SplashScreen::SplashScreen(const Reference< XMultiServiceFactory >& rSMgr)
    : IntroWindow()
    , _vdev(*((IntroWindow*)this))
    , _cProgressFrameColor(NOT_LOADED)
    , _cProgressBarColor(NOT_LOADED)
    , _iProgress(0)
    , _iMax(100)
    , _eBitmapMode(BM_DEFAULT)
    , _bPaintBitmap(sal_True)
    , _bPaintProgress(sal_False)
    , _bFullScreenSplash(sal_False)
    , _bProgressEnd(sal_False)
    , _tlx(NOT_LOADED)
    , _tly(NOT_LOADED)
    , _barwidth(NOT_LOADED)
    , _xoffset(12)
    , _yoffset(18)
    , _barheight(NOT_LOADED)
    , _barspace(2)
    , _fXPos(-1.0)
    , _fYPos(-1.0)
    , _fWidth(-1.0)
    , _fHeight(-1.0)
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

void SAL_CALL SplashScreen::start(const OUString& aText, sal_Int32 nRange)
    throw (RuntimeException)
{
    _iMax = nRange;
    if (_bVisible) {
        _bProgressEnd = sal_False;
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( TRUE );
        Show();
        Paint(Rectangle());
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
            ShowFullScreenMode( TRUE );
        Show();
        updateStatus();
    }
}

void SAL_CALL SplashScreen::setText(const OUString& aText)
    throw (RuntimeException)
{
    if (_bVisible && !_bProgressEnd) {
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( TRUE );
        Show();
    }
}

void SAL_CALL SplashScreen::setValue(sal_Int32 nValue)
    throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "::SplashScreen::setValue (lo119109)" );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "value=%d", nValue );

    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if (_bVisible && !_bProgressEnd) {
        if ( _eBitmapMode == BM_FULLSCREEN )
            ShowFullScreenMode( TRUE );
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
        initBitmap();
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

        if ( NOT_LOADED == _cProgressFrameColor.GetColor() )
            _cProgressFrameColor = Color( COL_LIGHTGRAY );

        if ( NOT_LOADED == _cProgressBarColor.GetColor() )
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
    //_bPaintBitmap=sal_False;
    Paint(Rectangle());
    //_bPaintBitmap=sal_True;
}

// internal private methods
IMPL_LINK( SplashScreen, AppEventListenerHdl, VclWindowEvent *, inEvent )
{
    if ( inEvent != 0 )
    {
        // Paint( Rectangle() );
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

OUString implReadBootstrapKey( const ::rtl::Bootstrap& _rIniFile, const OUString& _rKey )
{
    OUString sValue;
    _rIniFile.getFrom( _rKey, sValue );
    return sValue;
}

void SplashScreen::loadConfig()
{
    // detect execute path
    ::vos::OStartupInfo().getExecutableFile( _sExecutePath );
    sal_uInt32 nLastIndex = _sExecutePath.lastIndexOf( '/' );
    if ( nLastIndex > 0 )
        _sExecutePath = _sExecutePath.copy( 0, nLastIndex + 1 );

    // read keys from soffice.ini (sofficerc)
    OUString sIniFileName = _sExecutePath;
    sIniFileName += OUString( RTL_CONSTASCII_USTRINGPARAM( SAL_CONFIGFILE( "soffice" ) ) );
    rtl::Bootstrap aIniFile( sIniFileName );

    OUString sProgressFrameColor = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressFrameColor" ) ) );
    OUString sProgressBarColor = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressBarColor" ) ) );
    OUString sSize = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressSize" ) ) );
    OUString sPosition = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressPosition" ) ) );
    OUString sFullScreenSplash = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "FullScreenSplash" ) ) );

    // Determine full screen splash mode
    _bFullScreenSplash = (( sFullScreenSplash.getLength() > 0 ) &&
                          ( !sFullScreenSplash.equalsAsciiL( "0", 1 )));

    // Try to retrieve the relative values for the progress bar. The current
    // schema uses the screen ratio to retrieve the associated values.
    if ( _bFullScreenSplash )
        determineProgressRatioValues( aIniFile, _fXPos, _fYPos, _fWidth, _fHeight );

    if ( sProgressFrameColor.getLength() )
    {
        UINT8 nRed = 0;
        UINT8 nGreen = 0;
        UINT8 nBlue = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressFrameColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< UINT8 >( temp );
            temp = sProgressFrameColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            nGreen = static_cast< UINT8 >( temp );
            nBlue = static_cast< UINT8 >( sProgressFrameColor.getToken( 0, ',', idx ).toInt32() );
            _cProgressFrameColor = Color( nRed, nGreen, nBlue );
        }
    }

    if ( sProgressBarColor.getLength() )
    {
        UINT8 nRed = 0;
        UINT8 nGreen = 0;
        UINT8 nBlue = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressBarColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< UINT8 >( temp );
            temp = sProgressBarColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            nGreen = static_cast< UINT8 >( temp );
            nBlue = static_cast< UINT8 >( sProgressBarColor.getToken( 0, ',', idx ).toInt32() );
            _cProgressBarColor = Color( nRed, nGreen, nBlue );
        }
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

void SplashScreen::initBitmap()
{
    rtl::OUString aLogo( RTL_CONSTASCII_USTRINGPARAM( "1" ) );
    aLogo = ::utl::Bootstrap::getLogoData( aLogo );
    sal_Bool bShowLogo = (sal_Bool)aLogo.toInt32();

    if ( bShowLogo )
    {
        bool     haveBitmap = false;
        OUString aBmpFileName( RTL_CONSTASCII_USTRINGPARAM( "intro.bmp" ));

        if ( _bFullScreenSplash )
        {
            haveBitmap = findScreenBitmap( _aIntroBmp );
            if ( haveBitmap )
                _eBitmapMode = BM_FULLSCREEN;
            else
                haveBitmap = findAppBitmap( _aIntroBmp );
        }
        if ( !haveBitmap )
            haveBitmap = findBitmap( aBmpFileName, _aIntroBmp );

        if ( !haveBitmap )
        {
            // Save case:
            // Create resource manager for intro bitmap. Due to our problem that we don't have
            // any language specific information, we have to search for the correct resource
            // file. The bitmap resource is language independent.
            const USHORT nResId = RID_DEFAULTINTRO;
            ByteString aMgrName( "iso" );
            aMgrName += ByteString::CreateFromInt32(SUPD); // current build version
            ResMgr* pLabelResMgr = ResMgr::CreateResMgr( aMgrName.GetBuffer() );
            if ( !pLabelResMgr )
            {
                // no "iso" resource -> search for "ooo" resource
                aMgrName = "ooo";
                aMgrName += ByteString::CreateFromInt32(SUPD); // current build version
                pLabelResMgr = ResMgr::CreateResMgr( aMgrName.GetBuffer() );
            }
            if ( pLabelResMgr )
            {
                ResId aIntroBmpRes( nResId, pLabelResMgr );
                _aIntroBmp = Bitmap( aIntroBmpRes );
            }
            delete pLabelResMgr;
        }
    }
}

bool SplashScreen::findBitmap( const rtl::OUString aBmpFileName, Bitmap& aBitmap )
{
    bool haveBitmap = false;

    xub_StrLen nIndex = 0;

    // First, try to use custom bitmap data.
    rtl::OUString value;
    rtl::Bootstrap::get(
        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomDataUrl" ) ), value );
    if ( value.getLength() > 0 )
    {
        if ( value[ value.getLength() - 1 ] != sal_Unicode( '/' ) )
            value += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/program" ) );
        else
            value += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "program" ) );

        INetURLObject aObj( value, INET_PROT_FILE );
        aObj.insertName( aBmpFileName );

        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() )
        {
            // Default case, we load the intro bitmap from a seperate file
            // (e.g. staroffice_intro.bmp or starsuite_intro.bmp)
            aStrm >> _aIntroBmp;
            haveBitmap = true;
        }
    }

    // Then, try to use bitmap located in the same directory as the executable.
    if ( !haveBitmap )
    {
        INetURLObject aObj( _sExecutePath, INET_PROT_FILE );
        aObj.insertName( aBmpFileName );

        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() )
        {
            // Default case, we load the intro bitmap from a seperate file
            // (e.g. staroffice_intro.bmp or starsuite_intro.bmp)
            aStrm >> _aIntroBmp;
            haveBitmap = true;
        }
    }

    return haveBitmap;
}

bool SplashScreen::findScreenBitmap( Bitmap& rBitmap )
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
    OUStringBuffer aStrBuf( 128 );
    aStrBuf.appendAscii( "intro_" );
    if ( _sAppName.getLength() > 0 )
    {
        aStrBuf.append( _sAppName );
        aStrBuf.appendAscii( "_" );
    }
    aStrBuf.append( OUString::valueOf( nWidth ));
    aStrBuf.appendAscii( "x" );
    aStrBuf.append( OUString::valueOf( nHeight ));
    aStrBuf.appendAscii( ".bmp" );
    OUString aBmpFileName = aStrBuf.makeStringAndClear();

    bool haveBitmap = findBitmap( aBmpFileName, rBitmap );
    if ( !haveBitmap )
    {
        aStrBuf.appendAscii( "intro_" );
        aStrBuf.append( OUString::valueOf( nWidth ));
        aStrBuf.appendAscii( "x" );
        aStrBuf.append( OUString::valueOf( nHeight ));
        aStrBuf.appendAscii( ".bmp" );
        aBmpFileName = aStrBuf.makeStringAndClear();

        haveBitmap = findBitmap( aBmpFileName, rBitmap );
    }
    return haveBitmap;
}

bool SplashScreen::findAppBitmap( Bitmap& rBitmap )
{
    bool haveBitmap = false;

    if ( _sAppName.getLength() > 0 )
    {
        OUStringBuffer aStrBuf( 128 );
        aStrBuf.appendAscii( "intro_" );
        aStrBuf.append( _sAppName );
        aStrBuf.appendAscii( ".bmp" );
        OUString aBmpFileName = aStrBuf.makeStringAndClear();
        haveBitmap = findBitmap( aBmpFileName, rBitmap );
    }
    return haveBitmap;
}

void SplashScreen::determineProgressRatioValues(
    rtl::Bootstrap& rIniFile,
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
            rIniFile, OUString::createFromAscii( szFullScreenProgressRatio ) );

        if ( sFullScreenProgressRatio.getLength() > 0 )
        {
            double fRatio = sFullScreenProgressRatio.toDouble();
            sal_Int32 nRatio = sal_Int32( math::round( fRatio, 2 ) * 100 );
            if ( nRatio == nScreenRatio )
            {
                OUString sFullScreenProgressPos = implReadBootstrapKey(
                    rIniFile, OUString::createFromAscii( szFullScreenProgressPos ) );
                OUString sFullScreenProgressSize = implReadBootstrapKey(
                    rIniFile, OUString::createFromAscii( szFullScreenProgressSize ) );

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

void SplashScreen::Paint( const Rectangle& r)
{
    if(!_bVisible) return;

    // draw bitmap
    if (_bPaintBitmap)
        _vdev.DrawBitmap( Point(), _aIntroBmp );

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
        Rectangle aRect(_tlx+_barspace, _tly+_barspace, _tlx+_barspace+length, _tly+_barheight-_barspace);
        _vdev.DrawRect(Rectangle(_tlx+_barspace, _tly+_barspace,
            _tlx+_barspace+length, _tly+_barheight-_barspace));
    }
    Size aSize =  GetOutputSizePixel();
    Size bSize =  _vdev.GetOutputSizePixel();
    //_vdev.Flush();
    //_vdev.DrawOutDev(Point(), GetOutputSize(), Point(), GetOutputSize(), *((IntroWindow*)this) );
    DrawOutDev(Point(), GetOutputSizePixel(), Point(), _vdev.GetOutputSizePixel(), _vdev );
    //Flush();
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
OUString SplashScreen::impl_getImplementationName()
{
    return OUString::createFromAscii(implementationName);
}
Sequence<OUString> SplashScreen::impl_getSupportedServiceNames()
{
    Sequence<OUString> aSequence;
    for (int i=0; supportedServiceNames[i]!=NULL; i++) {
        aSequence.realloc(i+1);
        aSequence[i]=(OUString::createFromAscii(supportedServiceNames[i]));
    }
    return aSequence;
}

}

