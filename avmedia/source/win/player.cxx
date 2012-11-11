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

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"

#define AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_DirectX"
#define AVMEDIA_WIN_PLAYER_SERVICENAME "com.sun.star.media.Player_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {

LRESULT CALLBACK MediaPlayerWndProc_2( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    Player* pPlayer = (Player*) ::GetWindowLong( hWnd, 0 );
    bool    bProcessed = true;

    if( pPlayer )
    {
        switch( nMsg )
        {
            case( WM_GRAPHNOTIFY ):
                pPlayer->processEvent();
            break;
            default:
                bProcessed = false;
            break;
        }
    }
    else
        bProcessed = false;

    return( bProcessed ? 0 : DefWindowProc( hWnd, nMsg, nPar1, nPar2 ) );
}


bool isWindowsVistaOrHigher()
{
    // POST: return true if we are at least on Windows Vista
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    return  osvi.dwMajorVersion >= 6;
}

// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    Player_BASE(m_aMutex),
    mxMgr( rxMgr ),
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
    mnFrameWnd( 0 ),
    mbMuted( false ),
    mbLooping( false ),
    mbAddWindow( sal_True )
{
    ::CoInitialize( NULL );
}

// ------------------------------------------------------------------------------

Player::~Player()
{
    if( mnFrameWnd )
        ::DestroyWindow( (HWND) mnFrameWnd );

    ::CoUninitialize();
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    stop();
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
    {
        mpME->SetNotifyWindow( 0, WM_GRAPHNOTIFY, 0);
        mpME->Release();
    }

    if( mpMC )
        mpMC->Release();

    if( mpEV )
        mpEV->Release();

    if( mpOMF )
        mpOMF->Release();

    if( mpGB )
        mpGB->Release();
}
// ------------------------------------------------------------------------------
bool Player::create( const OUString& rURL )
{
    HRESULT hR;
    bool    bRet = false;

    if( SUCCEEDED( hR = CoCreateInstance( CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**) &mpGB ) ) )
    {
        // Don't use the overlay mixer on Windows Vista
        // It disables the desktop composition as soon as RenderFile is called
        // also causes some other problems: video rendering is not reliable
        if( !isWindowsVistaOrHigher() && SUCCEEDED( CoCreateInstance( CLSID_OverlayMixer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**) &mpOMF ) ) )
        {
            mpGB->AddFilter( mpOMF, L"com_sun_star_media_OverlayMixerFilter" );

            if( !SUCCEEDED( mpOMF->QueryInterface( IID_IDDrawExclModeVideo, (void**) &mpEV ) ) )
                mpEV = NULL;
        }

        if( SUCCEEDED( hR = mpGB->RenderFile( reinterpret_cast<LPCWSTR>(rURL.getStr()), NULL ) ) &&
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

    if( bRet )
        maURL = rURL;
    else
        maURL = OUString();

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
    mbAddWindow = sal_False;
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
    long nCode;
    LONG_PTR nParam1, nParam2;

    while( mpME && SUCCEEDED( mpME->GetEvent( &nCode, &nParam1, &nParam2, 0 ) ) )
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
    ::osl::MutexGuard aGuard(m_aMutex);
    if( mpMC )
    {
        if ( mbAddWindow )
        {
            static WNDCLASS* mpWndClass = NULL;
            if ( !mpWndClass )
            {
                mpWndClass = new WNDCLASS;

                memset( mpWndClass, 0, sizeof( *mpWndClass ) );
                mpWndClass->hInstance = GetModuleHandle( NULL );
                mpWndClass->cbWndExtra = sizeof( DWORD );
                mpWndClass->lpfnWndProc = MediaPlayerWndProc_2;
                mpWndClass->lpszClassName = "com_sun_star_media_Sound_Player";
                mpWndClass->hbrBackground = (HBRUSH) ::GetStockObject( BLACK_BRUSH );
                mpWndClass->hCursor = ::LoadCursor( NULL, IDC_ARROW );

                ::RegisterClass( mpWndClass );
            }
            if ( !mnFrameWnd )
            {
                mnFrameWnd = (int) ::CreateWindow( mpWndClass->lpszClassName, NULL,
                                           0,
                                           0, 0, 0, 0,
                                           (HWND) NULL, NULL, mpWndClass->hInstance, 0 );
                if ( mnFrameWnd )
                {
                    ::ShowWindow((HWND) mnFrameWnd, SW_HIDE);
                    ::SetWindowLong( (HWND) mnFrameWnd, 0, (DWORD) this );
                    // mpVW->put_Owner( (OAHWND) mnFrameWnd );
                    setNotifyWnd( mnFrameWnd );
                }
            }
        }

        mpMC->Run();
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if( mpMC )
        mpMC->Stop();
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

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
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_Duration( &aRefTime );

    return aRefTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

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
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_CurrentPosition( &aRefTime );

    return aRefTime;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    double fRet( 0.0 );

    if( mpMP  )
        mpMP->get_Rate( &fRet );

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbLooping;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (mpBA && (mbMuted != static_cast<bool>(bSet)))
    {
        mbMuted = bSet;
        mpBA->put_Volume( mbMuted ? -10000 : mnUnmutedVolume );
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = static_cast< long >( nVolumeDB ) * 100;

    if( !mbMuted && mpBA )
        mpBA->put_Volume( mnUnmutedVolume );
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return( static_cast< sal_Int16 >( mnUnmutedVolume / 100 ) );
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);

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
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( mpVW && aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::win::Window* pWindow = new ::avmedia::win::Window( mxMgr, *this );

        xRet = pWindow;

        if( !pWindow->create( aArguments ) )
            xRet = uno::Reference< ::media::XPlayerWindow >();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
    throw (uno::RuntimeException)
{
    uno::Reference< media::XFrameGrabber > xRet;

    if( maURL.getLength() > 0 )
    {
        FrameGrabber* pGrabber = new FrameGrabber( mxMgr );

        xRet = pGrabber;

        if( !pGrabber->create( maURL ) )
            xRet.clear();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString( AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_WIN_PLAYER_SERVICENAME;
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_WIN_PLAYER_SERVICENAME ;

    return aRet;
}

} // namespace win
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
