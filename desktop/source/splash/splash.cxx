/*************************************************************************
 *
 *  $RCSfile: splash.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-03-30 08:49:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "splash.hxx"

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
#ifndef _RTL_BOOTSTRAP_HXX_
#include <rtl/bootstrap.hxx>
#endif

#include <rtl/logfile.hxx>


using namespace ::rtl;
using namespace ::com::sun::star::registry;

namespace desktop
{

SplashScreen::SplashScreen(const Reference< XMultiServiceFactory >& rSMgr)
    : IntroWindow()
    , _vdev(*((IntroWindow*)this))
    , _cProgressColor( -1 )
    , _iProgress(0)
    , _iMax(100)
    , _bPaintBitmap(sal_True)
    , _bPaintProgress(sal_False)
    , _tlx(-1)
    , _tly(-1)
    , _barwidth(-1)
    , _xoffset(12)
    , _yoffset(18)
    , _barheight(-1)
    , _barspace(2)
{
    _rFactory = rSMgr;

    loadConfig();
    initBitmap();
    Size aSize = _aIntroBmp.GetSizePixel();
    SetOutputSizePixel( aSize );
    _vdev.SetOutputSizePixel( aSize );
    _height = aSize.Height();
    _width = aSize.Width();
    if (_width > 500)
    {
        /* --> PB 2004-11-17 #118510# new branding
        absolute values,
        see: http://so-doc.germany.sun.com/Teams/StarOffice_Applications/Extras/Design/Splash_About/splash_progressbar.html
        */
        Point xtopleft(212,216);
        if ( -1 == _tlx || -1 == _tly )
        {
            _tlx = xtopleft.X();    // top-left x
            _tly = xtopleft.Y();    // top-left y
        }
        if ( -1 == _barwidth )
            _barwidth = 263;
        if ( -1 == _barheight )
            _barheight = 8;
        // <--
    }
    else
    {
        if ( -1 == _barwidth )
            _barwidth  = _width - (2 * _yoffset);
        if ( -1 == _barheight )
            _barheight = 6;
        if ( -1 == _tlx || -1 == _tly )
        {
            _tlx = _xoffset;           // top-left x
            _tly = _height - _yoffset; // top-left y
        }
    }

    if ( -1 == _cProgressColor.GetColor() )
    {
        // progress bar: new color only for big bitmap format
        if ( _width > 500 )
            _cProgressColor = Color( 157, 202, 18 );
        else
            _cProgressColor = Color( COL_BLUE );
    }

    Application::AddEventListener(
        LINK( this, SplashScreen, AppEventListenerHdl ) );

    SetBackgroundBitmap( _aIntroBmp );
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
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        Show();
        Paint(Rectangle());
    }
}
void SAL_CALL SplashScreen::end()
    throw (RuntimeException)
{
    _iProgress = _iMax;
    updateStatus();
    if (_bVisible) Hide();
}
void SAL_CALL SplashScreen::reset()
    throw (RuntimeException)
{
    _iProgress = 0;
    Show();
    updateStatus();
}

void SAL_CALL SplashScreen::setText(const OUString& aText)
    throw (RuntimeException)
{
    if (_bVisible) {
        Show();
    }
}

void SAL_CALL SplashScreen::setValue(sal_Int32 nValue)
    throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "::SplashScreen::setValue (lo119109)" );
    RTL_LOGFILE_CONTEXT_TRACE1( aLog, "value=%d", nValue );

    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if (_bVisible) {
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
        aArguments[0] >>= _bVisible;
}

void SplashScreen::updateStatus()
{
    if (!_bVisible) return;
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

    OUString sProgressColor = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressColor" ) ) );
    OUString sSize = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressSize" ) ) );
    OUString sPosition = implReadBootstrapKey(
        aIniFile, OUString( RTL_CONSTASCII_USTRINGPARAM( "ProgressPosition" ) ) );

    if ( sProgressColor.getLength() )
    {
        UINT8 nRed = 0;
        UINT8 nGreen = 0;
        UINT8 nBlue = 0;
        sal_Int32 idx = 0;
        sal_Int32 temp = sProgressColor.getToken( 0, ',', idx ).toInt32();
        if ( idx != -1 )
        {
            nRed = static_cast< UINT8 >( temp );
            temp = sProgressColor.getToken( 0, ',', idx ).toInt32();
        }
        if ( idx != -1 )
        {
            nGreen = static_cast< UINT8 >( temp );
            nBlue = static_cast< UINT8 >( sProgressColor.getToken( 0, ',', idx ).toInt32() );
            _cProgressColor = Color( nRed, nGreen, nBlue );
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
    String aBmpFileName;
    OUString aIniPath;
    OUString aLogo( RTL_CONSTASCII_USTRINGPARAM( "1" ) );
    aLogo = ::utl::Bootstrap::getLogoData( aLogo );
    sal_Bool bLogo = (sal_Bool)aLogo.toInt32();
    if ( bLogo )
    {
        xub_StrLen nIndex = 0;
        aBmpFileName += String( DEFINE_CONST_UNICODE("intro.bmp") );
        // retrieve our current installation path
        INetURLObject aObj( _sExecutePath, INET_PROT_FILE );
        aObj.insertName( aBmpFileName );
        SvFileStream aStrm( aObj.PathToFileName(), STREAM_STD_READ );
        if ( !aStrm.GetError() )
        {
            // Default case, we load the intro bitmap from a seperate file
            // (e.g. staroffice_intro.bmp or starsuite_intro.bmp)
            aStrm >> _aIntroBmp;
        }
        else
        {
            // Save case:
            // Create resource manager for intro bitmap. Due to our problem that we don't have
            // any language specific information, we have to search for the correct resource
            // file. The bitmap resource is language independent.
            const USHORT nResId = RID_DEFAULTINTRO;
            String       aMgrName = String::CreateFromAscii( "iso" );
            aMgrName += String::CreateFromInt32(SUPD); // current build version
            ResMgr* pLabelResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));
            if ( !pLabelResMgr )
            {
                // no "iso" resource -> search for "ooo" resource
                aMgrName = String::CreateFromAscii( "ooo" );
                aMgrName += String::CreateFromInt32(SUPD); // current build version
                pLabelResMgr = ResMgr::CreateResMgr( U2S( aMgrName ));
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
        _vdev.SetLineColor( Color( COL_LIGHTGRAY ) );
        _vdev.DrawRect(Rectangle(_tlx, _tly, _tlx+_barwidth, _tly+_barheight));
        _vdev.SetFillColor( _cProgressColor );
        _vdev.SetLineColor();
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

#if 0
// component management stuff...
// ----------------------------------------------------------------------------
extern "C"
{
using namespace desktop;

void SAL_CALL
component_getImplementationEnvironment(const sal_Char **ppEnvironmentTypeName, uno_Environment **ppEnvironment)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL
component_writeInfo(void *pServiceManager, void *pRegistryKey)
{
    Reference< XMultiServiceFactory > xMan(reinterpret_cast< XMultiServiceFactory* >(pServiceManager));
    Reference< XRegistryKey > xKey(reinterpret_cast< XRegistryKey* >(pRegistryKey));

    // register service
    ::rtl::OUString aTempStr;
    ::rtl::OUString aImpl(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SplashScreen::impl_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    Reference< XRegistryKey > xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(SplashScreen::impl_getSupportedServiceNames()[0]);

    return sal_True;
}

void * SAL_CALL
component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void *pRegistryKey)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (desktop::SplashScreen::impl_getImplementationName().compareToAscii( pImplementationName ) == COMPARE_EQUAL )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createOneInstanceFactory(
                    xServiceManager, SplashScreen::impl_getImplementationName(),
                    SplashScreen::getInstance, SplashScreen::impl_getSupportedServiceNames()));
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"
#endif
