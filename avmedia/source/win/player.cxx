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

#include <objbase.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/file.hxx>
#include <rtl/ref.hxx>

#define AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_DirectX"
#define AVMEDIA_WIN_PLAYER_SERVICENAME "com.sun.star.media.Player_DirectX"

using namespace ::com::sun::star;

namespace avmedia::win {

static LRESULT CALLBACK MediaPlayerWndProc_2( HWND hWnd,UINT nMsg, WPARAM nPar1, LPARAM nPar2 )
{
    Player* pPlayer = reinterpret_cast<Player*>(::GetWindowLongPtrW( hWnd, 0 ));
    bool    bProcessed = true;

    if( pPlayer )
    {
        switch( nMsg )
        {
            case WM_GRAPHNOTIFY:
                pPlayer->processEvent();
            break;
            default:
                bProcessed = false;
            break;
        }
    }
    else
        bProcessed = false;

    return( bProcessed ? 0 : DefWindowProcW( hWnd, nMsg, nPar1, nPar2 ) );
}


Player::Player() :
    Player_BASE(m_aMutex),
    mpGB( nullptr ),
    mpOMF( nullptr ),
    mpMC( nullptr ),
    mpME( nullptr ),
    mpMS( nullptr ),
    mpMP( nullptr ),
    mpBA( nullptr ),
    mpBV( nullptr ),
    mpVW( nullptr ),
    mpEV( nullptr ),
    mnUnmutedVolume( 0 ),
    mnFrameWnd( nullptr ),
    mbMuted( false ),
    mbLooping( false ),
    mbAddWindow( true )
{
    ::CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED );
}


Player::~Player()
{
    if( mnFrameWnd )
        ::DestroyWindow( mnFrameWnd );

    ::CoUninitialize();
}


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


bool Player::create( const OUString& rURL )
{
    HRESULT hR;
    bool    bRet = false;

    if( SUCCEEDED( hR = CoCreateInstance( CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, reinterpret_cast<void**>(&mpGB) ) ) )
    {
        // Don't use the overlay mixer on Windows Vista
        // It disables the desktop composition as soon as RenderFile is called
        // also causes some other problems: video rendering is not reliable

        // tdf#128057: IGraphBuilder::RenderFile seems to fail to handle file URIs properly when
        // they contain encoded characters like "%23"; so pass system path in that case instead.
        OUString aFile(rURL);
        if (aFile.startsWithIgnoreAsciiCase("file:"))
            osl::FileBase::getSystemPathFromFileURL(rURL, aFile);

        if( SUCCEEDED( hR = mpGB->RenderFile( o3tl::toW(aFile.getStr()), nullptr ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaControl, reinterpret_cast<void**>(&mpMC) ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaEventEx, reinterpret_cast<void**>(&mpME) ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaSeeking, reinterpret_cast<void**>(&mpMS) ) ) &&
            SUCCEEDED( hR = mpGB->QueryInterface( IID_IMediaPosition, reinterpret_cast<void**>(&mpMP) ) ) )
        {
            // Video interfaces
            mpGB->QueryInterface( IID_IVideoWindow, reinterpret_cast<void**>(&mpVW) );
            mpGB->QueryInterface( IID_IBasicVideo, reinterpret_cast<void**>(&mpBV) );

            // Audio interface
            mpGB->QueryInterface( IID_IBasicAudio, reinterpret_cast<void**>(&mpBA) );

            if( mpBA )
                mpBA->put_Volume( mnUnmutedVolume );

            bRet = true;
        }
    }

    if( bRet )
        maURL = rURL;
    else
        maURL.clear();

    return bRet;
}


const IVideoWindow* Player::getVideoWindow() const
{
    return mpVW;
}


void Player::setNotifyWnd( HWND nNotifyWnd )
{
    mbAddWindow = false;
    if( mpME )
        mpME->SetNotifyWindow( reinterpret_cast<OAHWND>(nNotifyWnd), WM_GRAPHNOTIFY, reinterpret_cast< LONG_PTR>( this ) );
}


void Player::processEvent()
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
}


void SAL_CALL Player::start(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if( mpMC )
    {
        if ( mbAddWindow )
        {
            static WNDCLASSW* mpWndClass = nullptr;
            if ( !mpWndClass )
            {
                mpWndClass = new WNDCLASSW;

                memset( mpWndClass, 0, sizeof( *mpWndClass ) );
                mpWndClass->hInstance = GetModuleHandleW( nullptr );
                mpWndClass->cbWndExtra = sizeof( DWORD );
                mpWndClass->lpfnWndProc = MediaPlayerWndProc_2;
                mpWndClass->lpszClassName = L"com_sun_star_media_Sound_Player";
                mpWndClass->hbrBackground = static_cast<HBRUSH>(::GetStockObject( BLACK_BRUSH ));
                mpWndClass->hCursor = ::LoadCursor( nullptr, IDC_ARROW );

                RegisterClassW( mpWndClass );
            }
            if ( !mnFrameWnd )
            {
                mnFrameWnd = CreateWindowW( mpWndClass->lpszClassName, nullptr,
                                            0,
                                            0, 0, 0, 0,
                                            nullptr, nullptr, mpWndClass->hInstance, nullptr );
                if ( mnFrameWnd )
                {
                    ::ShowWindow(mnFrameWnd, SW_HIDE);
                    SetWindowLongPtrW( mnFrameWnd, 0, reinterpret_cast<LONG_PTR>(this) );
                    // mpVW->put_Owner( (OAHWND) mnFrameWnd );
                    setNotifyWnd( mnFrameWnd );
                }
            }
        }

        mpMC->Run();
    }
}


void SAL_CALL Player::stop(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if( mpMC )
        mpMC->Stop();
}


sal_Bool SAL_CALL Player::isPlaying()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OAFilterState   eFilterState;
    bool            bRet = false;

    if( mpMC && SUCCEEDED( mpMC->GetState( 10, &eFilterState ) ) )
        bRet = ( State_Running == eFilterState );

    return bRet;
}


double SAL_CALL Player::getDuration(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_Duration( &aRefTime );

    return aRefTime;
}


void SAL_CALL Player::setMediaTime( double fTime )
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


double SAL_CALL Player::getMediaTime(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    REFTIME aRefTime( 0.0 );

    if( mpMP  )
        mpMP->get_CurrentPosition( &aRefTime );

    return aRefTime;
}


void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mbLooping = bSet;
}


sal_Bool SAL_CALL Player::isPlaybackLoop(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbLooping;
}


void SAL_CALL Player::setMute( sal_Bool bSet )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (mpBA && (mbMuted != static_cast<bool>(bSet)))
    {
        mbMuted = bSet;
        mpBA->put_Volume( mbMuted ? -10000 : mnUnmutedVolume );
    }
}


sal_Bool SAL_CALL Player::isMute(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbMuted;
}


void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = static_cast< long >( nVolumeDB ) * 100;

    if( !mbMuted && mpBA )
        mpBA->put_Volume( mnUnmutedVolume );
}


sal_Int16 SAL_CALL Player::getVolumeDB(  )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return static_cast< sal_Int16 >( mnUnmutedVolume / 100 );
}


awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
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


uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( mpVW && aSize.Width > 0 && aSize.Height > 0 )
    {
        rtl::Reference<::avmedia::win::Window> pWindow = new ::avmedia::win::Window( *this );

        xRet = pWindow;

        if( !pWindow->create( aArguments ) )
            xRet.clear();
    }

    return xRet;
}


uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
{
    uno::Reference< media::XFrameGrabber > xRet;

    if( !maURL.isEmpty() )
    {
        rtl::Reference<FrameGrabber> pGrabber = new FrameGrabber();

        xRet = pGrabber;

        if( !pGrabber->create( maURL ) )
            xRet.clear();
    }

    return xRet;
}


OUString SAL_CALL Player::getImplementationName(  )
{
    return AVMEDIA_WIN_PLAYER_IMPLEMENTATIONNAME;
}


sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames(  )
{
    return { AVMEDIA_WIN_PLAYER_SERVICENAME };
}

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
