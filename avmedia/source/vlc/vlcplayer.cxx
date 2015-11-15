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

#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <cppuhelper/supportsservice.hxx>

#include "vlcplayer.hxx"
#include "vlcwindow.hxx"
#include "vlcframegrabber.hxx"
#include "wrapper/Instance.hxx"

using namespace ::com::sun::star;

namespace avmedia {
namespace vlc {

namespace
{
    const ::rtl::OUString AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME = "com.sun.star.comp.avmedia.Player_VLC";
    const ::rtl::OUString AVMEDIA_VLC_PLAYER_SERVICENAME = "com.sun.star.media.Player_VLC";

    const int MS_IN_SEC = 1000; // Millisec in sec
}

VLCPlayer::VLCPlayer( const rtl::OUString& url,
                      wrapper::Instance& instance,
                      wrapper::EventHandler& eh )
    : VLC_Base( m_aMutex )
    , mInstance( instance )
    , mEventHandler( eh )
    , mMedia( url, mInstance )
    , mPlayer( mMedia )
    , mEventManager( mPlayer, mEventHandler )
    , mUrl( url )
    , mPlaybackLoop( false )
    , mPrevWinID( 0 )
{
    mPlayer.setMouseHandling( false );
}

unsigned VLCPlayer::getWidth() const
{
    return mPlayer.getWidth();
}

unsigned VLCPlayer::getHeight() const
{
    return mPlayer.getHeight();
}

void SAL_CALL VLCPlayer::start() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (!mPlayer.play())
    {
        // TODO: Error
    }
}

void SAL_CALL VLCPlayer::stop() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.pause();
}

sal_Bool SAL_CALL VLCPlayer::isPlaying() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.isPlaying();
}

double SAL_CALL VLCPlayer::getDuration() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mMedia.getDuration() ) / MS_IN_SEC;
}

void SAL_CALL VLCPlayer::setScale( float factor )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setScale( factor );
}

void SAL_CALL VLCPlayer::setMediaTime( double fTime ) throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( fTime < 0.00000001 && !mPlayer.isPlaying() )
    {
        mPlayer.stop();
    }

    mPlayer.setTime( fTime * MS_IN_SEC );
}

double SAL_CALL VLCPlayer::getMediaTime() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<double>( mPlayer.getTime() ) / MS_IN_SEC;
}

void VLCPlayer::replay()
{
    setPlaybackLoop( false );
    stop();
    setMediaTime( 0 );
    start();
}

void SAL_CALL VLCPlayer::setPlaybackLoop( sal_Bool bSet ) throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlaybackLoop = bSet;

    if ( bSet )
        mEventManager.onEndReached([this](){ this->replay(); });
    else
        mEventManager.onEndReached();
}

sal_Bool SAL_CALL VLCPlayer::isPlaybackLoop() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlaybackLoop;
}

void SAL_CALL VLCPlayer::setVolumeDB( ::sal_Int16 nDB ) throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setVolume( static_cast<sal_Int16>( ( nDB + 40 ) * 10.0  / 4 ) );
}

::sal_Int16 SAL_CALL VLCPlayer::getVolumeDB() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return static_cast<sal_Int16>( mPlayer.getVolume() / 10.0 * 4 - 40 );
}

void SAL_CALL VLCPlayer::setMute( sal_Bool bSet ) throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    mPlayer.setMute( bSet );
}

sal_Bool SAL_CALL VLCPlayer::isMute() throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return mPlayer.getMute();
}

css::awt::Size SAL_CALL VLCPlayer::getPreferredPlayerWindowSize() throw ( css::uno::RuntimeException, std::exception )
{
    return css::awt::Size( 480, 360 );
}

namespace
{
    // TODO: Move this function to the common space for avoiding duplication with
    // gstreamer/gstwindow::createPlayerWindow functionality
    intptr_t GetWindowID( const uno::Sequence< uno::Any >& arguments )
    {
        if (arguments.getLength() <= 2)
            return -1;

        sal_IntPtr pIntPtr = 0;

        arguments[ 2 ] >>= pIntPtr;

        SystemChildWindow *pParentWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );

        const SystemEnvData* pEnvData = pParentWindow ? pParentWindow->GetSystemData() : nullptr;

        if (pEnvData == nullptr)
            return -1;

#if defined MACOSX
        const intptr_t id = reinterpret_cast<intptr_t>( pEnvData->mpNSView );
#elif defined WNT
        const intptr_t id = reinterpret_cast<intptr_t>( pEnvData->hWnd );
#else
        const intptr_t id = static_cast<intptr_t>( pEnvData->aWindow );
#endif

        return id;
    }
}

void SAL_CALL VLCPlayer::setWindowID( const intptr_t windowID )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    mPlayer.stop();
    mPlayer.setWindow( windowID );
}

void VLCPlayer::setVideoSize( unsigned width, unsigned height )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    mPlayer.setVideoSize( width, height );
}

uno::Reference< css::media::XPlayerWindow > SAL_CALL VLCPlayer::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
     throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    const intptr_t winID = GetWindowID( aArguments );
    VLCWindow * window;
    if ( mPrevWinID == 0 )
    {
        mPrevWinID = winID;
        window = new VLCWindow( *this, 0 );
    }
    else
        window = new VLCWindow( *this, mPrevWinID );

    if ( winID != -1 )
    {
        setWindowID( winID );
    }

    return css::uno::Reference< css::media::XPlayerWindow >( window );
}

uno::Reference< css::media::XFrameGrabber > SAL_CALL VLCPlayer::createFrameGrabber()
     throw ( css::uno::RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( !mrFrameGrabber.is() )
    {
        VLCFrameGrabber *frameGrabber = new VLCFrameGrabber( mEventHandler, mUrl );
        mrFrameGrabber.set( frameGrabber );
    }

    return mrFrameGrabber;
}

::rtl::OUString SAL_CALL VLCPlayer::getImplementationName()
     throw ( css::uno::RuntimeException, std::exception )
{
    return AVMEDIA_VLC_PLAYER_IMPLEMENTATIONNAME;
}

sal_Bool SAL_CALL VLCPlayer::supportsService( const ::rtl::OUString& serviceName )
     throw ( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

::uno::Sequence< ::rtl::OUString > SAL_CALL VLCPlayer::getSupportedServiceNames()
     throw ( css::uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet { AVMEDIA_VLC_PLAYER_SERVICENAME };
    return aRet;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
