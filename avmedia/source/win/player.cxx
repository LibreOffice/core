/*************************************************************************
 *
 *  $RCSfile: player.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date:
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

#include <tools/prewin.h>
#include <windows.h>
#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>
#include <tools/postwin.h>

#include "player.hxx"
#include "window.hxx"

#define AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_DirectX"
#define AVMEDIA_WIN_PLAYER_SERVICENAME "com.sun.star.media.Player_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {

// ----------------
// - Player -
// ----------------

Player::Player() :
    mpGB( NULL ),
    mpOMF( NULL ),
    mpMC( NULL ),
    mpME( NULL ),
    mpMS( NULL ),
    mpMP( NULL ),
    mpBA( NULL ),
    mpBV( NULL ),
    mpVW( NULL ),
    mpEV( NULL ),
    mnUnmutedVolume( 0 ),
    mbMuted( false ),
    mbLooping( false )
{
    ::CoInitialize( NULL );
}

// ------------------------------------------------------------------------------

Player::~Player()
{
    if( mpBA )
        mpBA->Release();

    if( mpBV )
        mpBV->Release();

    if( mpVW )
        mpVW->Release();

    if( mpMP )
        mpMP->Release();

    if( mpMS )
        mpMS->Release();

    if( mpME )
        mpME->Release();

    if( mpMC )
        mpMC->Release();

    if( mpEV )
        mpEV->Release();

    if( mpOMF )
        mpOMF->Release();

    if( mpGB )
        mpGB->Release();

    ::CoUninitialize();
}

// ------------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
    HRESULT hR;
    bool    bRet = false;

    if( SUCCEEDED( hR = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**) &mpGB ) ) )
    {
        // use overlays, if possible
        if( SUCCEEDED( CoCreateInstance( CLSID_OverlayMixer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &mpOMF ) ) )
        {
            mpGB->AddFilter( mpOMF, L"com_sun_star_media_OverlayMixerFilter" );

            if( !SUCCEEDED( mpOMF->QueryInterface( IID_IDDrawExclModeVideo, (void**) &mpEV ) ) )
                mpEV = NULL;
        }

        if( SUCCEEDED( hR = mpGB->RenderFile( rURL, NULL ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaControl, (void**) &mpMC ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaEventEx, (void**) &mpME ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaSeeking, (void**) &mpMS ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaPosition, (void**) &mpMP ) ) )
        {
            // Video interfaces
            mpGB->QueryInterface( IID_IVideoWindow, (void**) &mpVW );
            mpGB->QueryInterface( IID_IBasicVideo, (void**) &mpBV );

            // Audio interface
            mpGB->QueryInterface( IID_IBasicAudio, (void**) &mpBA );

            if( mpBA )
                mpBA->put_Volume( mnUnmutedVolume );

            bRet = true;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------------

const IVideoWindow* Player::getVideoWindow() const
{
    return mpVW;
}

// ------------------------------------------------------------------------------

void Player::setNotifyWnd( int nNotifyWnd )
{
    if( mpME )
        mpME->SetNotifyWindow( (OAHWND) nNotifyWnd, WM_GRAPHNOTIFY, reinterpret_cast< LONG_PTR>( this ) );
}

// ------------------------------------------------------------------------------

void Player::setDDrawParams( IDirectDraw* pDDraw, IDirectDrawSurface* pDDrawSurface )
{
    if( mpEV && pDDraw && pDDrawSurface )
    {
        mpEV->SetDDrawObject( pDDraw );
        mpEV->SetDDrawSurface( pDDrawSurface );
    }
}

// ------------------------------------------------------------------------------

long Player::processEvent()
{
    long nCode, nParam1, nParam2;

    if( mpME && SUCCEEDED( mpME->GetEvent( &nCode, &nParam1, &nParam2, 0 ) ) )
    {
        if( EC_COMPLETE == nCode )
        {
            if( mbLooping )
            {
                setMediaTime( 0.0 );
                start();
            }
            else
            {
                   setMediaTime( getDuration() );
                   stop();
            }
        }

        mpME->FreeEventParams( nCode, nParam1, nParam2 );
    }

    return 0;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start(  )
    throw (uno::RuntimeException)
{
    if( mpMC )
        mpMC->Run();
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
    if( mpMC )
        mpMC->Stop();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    OAFilterState   eFilterState;
    bool            bRet = false;

    if( mpMC && SUCCEEDED( mpMC->GetState( 10, &eFilterState ) ) )
        bRet = ( State_Running == eFilterState );

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration(  )
    throw (uno::RuntimeException)
{
    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_Duration( &aRefTime );

    return aRefTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    if( mpMP  )
    {
        const bool bPlaying = isPlaying();

        mpMP->put_CurrentPosition( fTime );

        if( !bPlaying && mpMC )
            mpMC->StopWhenReady();
    }
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime(  )
    throw (uno::RuntimeException)
{
    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_CurrentPosition( &aRefTime );

    return aRefTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
    throw (uno::RuntimeException)
{
    if( mpMP  )
        mpMP->put_StopTime( fTime );
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getStopTime(  )
    throw (uno::RuntimeException)
{
    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_StopTime( &aRefTime );

    return aRefTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate )
    throw (uno::RuntimeException)
{
    if( mpMP  )
        mpMP->put_Rate( fRate );
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
    double fRet( 0.0 );

    if( mpMP  )
        mpMP->get_Rate( &fRet );

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
    return mbLooping;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    if( mpBA && ( mbMuted != bSet ) )
    {
        mpBA->put_Volume( ( mbMuted = bSet ) ? -10000 : mnUnmutedVolume );
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
    mnUnmutedVolume = static_cast< long >( nVolumeDB ) * 100;

    if( !mbMuted && mpBA )
        mpBA->put_Volume( mnUnmutedVolume );
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB(  )
    throw (uno::RuntimeException)
{
    return( mnUnmutedVolume / 100 );

    return 0;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
    awt::Size aSize( 0, 0 );

    if( mpBV )
    {
        long nWidth = 0, nHeight = 0;

        mpBV->GetVideoSize( &nWidth, &nHeight );
        aSize.Width = nWidth;
        aSize.Height = nHeight;
    }

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::RuntimeException)
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( mpVW && aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::win::Window* pWindow = new ::avmedia::win::Window( *this );

        xRet = pWindow;

        if( !pWindow->create( aArguments ) )
            xRet = uno::Reference< ::media::XPlayerWindow >();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_WIN_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_WIN_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace win
} // namespace avmedia
