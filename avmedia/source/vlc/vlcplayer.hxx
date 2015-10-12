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

#ifndef INCLUDED_AVMEDIA_SOURCE_VLC_VLCPLAYER_HXX
#define INCLUDED_AVMEDIA_SOURCE_VLC_VLCPLAYER_HXX

#include "vlccommon.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/media/XPlayer.hpp>
#include <cppuhelper/basemutex.hxx>

#include "wrapper/Instance.hxx"
#include "wrapper/Media.hxx"
#include "wrapper/Player.hxx"
#include "wrapper/EventManager.hxx"

namespace avmedia {
namespace vlc {

typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer,
                                         css::lang::XServiceInfo > VLC_Base;

class VLCPlayer : public ::cppu::BaseMutex,
                  public VLC_Base
{
    wrapper::Instance&     mInstance;
    wrapper::EventHandler& mEventHandler;

    wrapper::Media         mMedia;
    wrapper::Player        mPlayer;
    wrapper::EventManager  mEventManager;
    const rtl::OUString    mUrl;
    bool                   mPlaybackLoop;
    css::uno::Reference< css::media::XFrameGrabber > mrFrameGrabber;
    intptr_t               mPrevWinID;
public:
    VLCPlayer( const rtl::OUString& url,
               wrapper::Instance& instance,
               wrapper::EventHandler& eh );

    void setVideoSize( unsigned width, unsigned height );
    unsigned getWidth() const;
    unsigned getHeight() const;

    void SAL_CALL setScale( float factor );
    void SAL_CALL setWindowID( const intptr_t windowID );

    void SAL_CALL start() throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL stop() throw ( css::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL isPlaying() throw ( css::uno::RuntimeException, std::exception ) override;
    double SAL_CALL getDuration() throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL setMediaTime( double fTime ) throw ( css::uno::RuntimeException, std::exception ) override;
    double SAL_CALL getMediaTime() throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw ( css::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL isPlaybackLoop() throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL setVolumeDB( ::sal_Int16 nDB ) throw ( css::uno::RuntimeException, std::exception ) override;
    ::sal_Int16 SAL_CALL getVolumeDB() throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL setMute( sal_Bool bSet ) throw ( css::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL isMute() throw ( css::uno::RuntimeException, std::exception ) override;
    css::awt::Size SAL_CALL getPreferredPlayerWindowSize() throw ( css::uno::RuntimeException, std::exception ) override;
    css::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const css::uno::Sequence< css::uno::Any >& aArguments )
            throw ( css::uno::RuntimeException, std::exception ) override;
    css::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber()
            throw ( css::uno::RuntimeException, std::exception ) override;

    ::rtl::OUString SAL_CALL getImplementationName()
            throw ( css::uno::RuntimeException, std::exception ) override;
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& serviceName )
            throw ( css::uno::RuntimeException, std::exception ) override;;
    css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw ( css::uno::RuntimeException, std::exception ) override;;

private:
    void replay();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
