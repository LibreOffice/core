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

#ifndef _VLCPLAYER_HXX
#define _VLCPLAYER_HXX
#if defined UNX
#include <unistd.h>
#endif
#include "vlccommon.hxx"
#include <boost/shared_ptr.hpp>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/media/XPlayer.hpp>
#include <cppuhelper/basemutex.hxx>

#include "wrapper/Instance.hxx"
#include "wrapper/Media.hxx"
#include "wrapper/Player.hxx"
#include "wrapper/EventManager.hxx"

namespace avmedia {
namespace vlc {

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::media::XPlayer,
                                          ::com::sun::star::lang::XServiceInfo > VLC_Base;

class VLCPlayer : public ::cppu::BaseMutex,
                  public VLC_Base
{
    wrapper::Instance& mInstance;
    wrapper::EventHandler& mEventHandler;

    wrapper::Media mMedia;
    wrapper::Player mPlayer;
    wrapper::EventManager mEventManager;
    const rtl::OUString mUrl;
    bool mPlaybackLoop;
    ::com::sun::star::uno::Reference< css::media::XFrameGrabber > mrFrameGrabber;
    intptr_t mPrevWinID;
public:
    VLCPlayer( const rtl::OUString& url,
               wrapper::Instance& instance,
               wrapper::EventHandler& eh );

    unsigned getWidth() const;
    unsigned getHeight() const;

    void SAL_CALL setScale( float factor );
    void SAL_CALL setWindowID( const intptr_t windowID );

    void SAL_CALL start() throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL stop() throw ( ::com::sun::star::uno::RuntimeException );
    ::sal_Bool SAL_CALL isPlaying() throw ( ::com::sun::star::uno::RuntimeException );
    double SAL_CALL getDuration() throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setMediaTime( double fTime ) throw ( ::com::sun::star::uno::RuntimeException );
    double SAL_CALL getMediaTime() throw ( ::com::sun::star::uno::RuntimeException );
    double SAL_CALL getRate() throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setPlaybackLoop( ::sal_Bool bSet ) throw ( ::com::sun::star::uno::RuntimeException );
    ::sal_Bool SAL_CALL isPlaybackLoop() throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setVolumeDB( ::sal_Int16 nDB ) throw ( ::com::sun::star::uno::RuntimeException );
    ::sal_Int16 SAL_CALL getVolumeDB() throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setMute( ::sal_Bool bSet ) throw ( ::com::sun::star::uno::RuntimeException );
    ::sal_Bool SAL_CALL isMute() throw ( ::com::sun::star::uno::RuntimeException );
    css::awt::Size SAL_CALL getPreferredPlayerWindowSize() throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
            throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber()
            throw ( ::com::sun::star::uno::RuntimeException );

    ::rtl::OUString SAL_CALL getImplementationName()
            throw ( ::com::sun::star::uno::RuntimeException );
    ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& serviceName )
            throw ( ::com::sun::star::uno::RuntimeException );;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw ( ::com::sun::star::uno::RuntimeException );;

private:
    void replay();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
