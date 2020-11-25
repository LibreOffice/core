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

#pragma once

#include <sal/config.h>

#include <WinDef.h>

#include "wincommon.hxx"

#include <com/sun/star/media/XPlayer.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

struct IGraphBuilder;
struct IBaseFilter;
struct IMediaControl;
struct IMediaEventEx;
struct IMediaSeeking;
struct IMediaPosition;
struct IBasicAudio;
struct IBasicVideo;
struct IVideoWindow;
struct IDDrawExclModeVideo;
struct IDirectDraw;
struct IDirectDrawSurface;

namespace avmedia::win {

typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer,
                                         css::lang::XServiceInfo > Player_BASE;


class Player : public cppu::BaseMutex,
               public Player_BASE
{
public:

    explicit Player();
    ~Player() override;

    bool                create( const OUString& rURL );

    void                setNotifyWnd( HWND nNotifyWnd );
    void                processEvent();

    const IVideoWindow* getVideoWindow() const;

    // XPlayer
    virtual void SAL_CALL start(  ) override;
    virtual void SAL_CALL stop(  ) override;
    virtual sal_Bool SAL_CALL isPlaying(  ) override;
    virtual double SAL_CALL getDuration(  ) override;
    virtual void SAL_CALL setMediaTime( double fTime ) override;
    virtual double SAL_CALL getMediaTime(  ) override;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) override;
    virtual void SAL_CALL setMute( sal_Bool bSet ) override;
    virtual sal_Bool SAL_CALL isMute(  ) override;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) override;
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) override;
    virtual css::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const css::uno::Sequence< css::uno::Any >& aArguments ) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing() override;

private:

    OUString                maURL;
    IGraphBuilder*          mpGB;
    IBaseFilter*            mpOMF;
    IMediaControl*          mpMC;
    IMediaEventEx*          mpME;
    IMediaSeeking*          mpMS;
    IMediaPosition*         mpMP;
    IBasicAudio*            mpBA;
    IBasicVideo*            mpBV;
    IVideoWindow*           mpVW;
    IDDrawExclModeVideo*    mpEV;
    long                    mnUnmutedVolume;
    HWND                    mnFrameWnd;
    bool                    mbMuted;
    bool                    mbLooping;
    bool                    mbAddWindow;

    void                    ImplLayoutVideoWindow();
};

} // namespace avmedia::win


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
