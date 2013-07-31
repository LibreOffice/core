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

#include "vlccommon.hxx"
#include <vlc/vlc.h>
#include <boost/shared_ptr.hpp>
#include <cppuhelper/compbase2.hxx>
#include <com/sun/star/media/XPlayer.hpp>
#include <cppuhelper/basemutex.hxx>


namespace avmedia {
namespace vlc {

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::media::XPlayer,
                                          ::com::sun::star::lang::XServiceInfo > VLC_Base;

class VLCPlayer : public ::cppu::BaseMutex,
                  public VLC_Base
{
    boost::shared_ptr<libvlc_instance_t> mInstance;
    boost::shared_ptr<libvlc_media_t> mMedia;
    boost::shared_ptr<libvlc_media_player_t> mPlayer;
    const rtl::OUString mUrl;
    bool mPlaybackLoop;
public:
    VLCPlayer( const rtl::OUString& url );

    const rtl::OUString& url() const;

    void SAL_CALL start();
    void SAL_CALL stop();
    ::sal_Bool SAL_CALL isPlaying();
    double SAL_CALL getDuration();
    void SAL_CALL setMediaTime( double fTime );
    double SAL_CALL getMediaTime();
    double SAL_CALL getRate();
    void SAL_CALL setPlaybackLoop( ::sal_Bool bSet );
    ::sal_Bool SAL_CALL isPlaybackLoop();
    void SAL_CALL setVolumeDB( ::sal_Int16 nDB );
    ::sal_Int16 SAL_CALL getVolumeDB();
    void SAL_CALL setMute( ::sal_Bool bSet );
    ::sal_Bool SAL_CALL isMute();
    css::awt::Size SAL_CALL getPreferredPlayerWindowSize();
    ::com::sun::star::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments );
    ::com::sun::star::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber();

    ::rtl::OUString SAL_CALL getImplementationName();
    ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& serviceName );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();
};

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */