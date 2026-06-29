/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <osl/conditn.h>
#include "macavfcommon.hxx"
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/media/XPlayer.hpp>

namespace avmedia::macavf {

class Player
:   public MacAVObserverHandler
,   public ::cppu::WeakImplHelper< css::media::XPlayer,
                                   css::lang::XServiceInfo >
{
public:
    explicit Player();
    virtual  ~Player() override;

    bool create( const OUString& rURL );
    bool create( AVAsset* );

    // XPlayer
    virtual void start() override;
    virtual void stop() override;
    virtual bool isPlaying() override;
    virtual double getDuration() override;
    virtual void setMediaTime( double fTime ) override;
    virtual double getMediaTime() override;
    /// @throws css::uno::RuntimeException
    virtual void setStopTime( double fTime );
    /// @throws css::uno::RuntimeException
    virtual double getStopTime();
    virtual void setPlaybackLoop( bool bSet ) override;
    virtual bool isPlaybackLoop() override;
    virtual void setMute( bool bSet ) override;
    virtual bool isMute() override;
    virtual void setVolumeDB( sal_Int16 nVolumeDB ) override;
    virtual sal_Int16 getVolumeDB() override;
    virtual css::awt::Size getPreferredPlayerWindowSize(  ) override;
    virtual css::uno::Reference< css::media::XPlayerWindow > createPlayerWindow( const css::uno::Sequence< cpo::uno::Any >& aArguments ) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > createFrameGrabber(  ) override;
    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > getSupportedServiceNames(  ) override;

    AVPlayer* getAVPlayer() const { return mpPlayer; }
    virtual bool handleObservation( NSString* pKeyPath ) override;

private:

    AVPlayer*           mpPlayer;

    float               mfUnmutedVolume;
    double              mfStopTime;

    bool                mbMuted;
    bool                mbLooping;
};

} // namespace avmedia::macavf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
