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

#ifndef INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTPLAYER_HXX
#define INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTPLAYER_HXX

#include <osl/conditn.hxx>
#include "gstcommon.hxx"

#include <com/sun/star/media/XPlayer.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#if defined(ENABLE_GTKSINK)
#    include <gtk/gtk.h>
#endif

typedef struct _GstVideoOverlay GstVideoOverlay;

namespace avmedia { namespace gstreamer {


typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer,
                                         css::lang::XServiceInfo > GstPlayer_BASE;

class Player final : public ::cppu::BaseMutex,
               public GstPlayer_BASE
{
public:

    explicit Player();
    virtual ~Player() override;

    void preparePlaybin( const OUString& rURL, GstElement *pSink );
    bool create( const OUString& rURL );
    void processMessage( GstMessage *message );
    GstBusSyncReply processSyncMessage( GstMessage *message );

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
    virtual void SAL_CALL disposing() final override;

private:
    OUString                maURL;

    // Add elements and pipeline here
    GstElement*             mpPlaybin;  // the playbin is also a pipeline
    GstElement*             mpVolumeControl;  // the playbin is also a pipeline
#if defined(ENABLE_GTKSINK)
    GtkWidget*              mpGtkWidget;
#endif
    bool                    mbUseGtkSink;
    bool                    mbFakeVideo;

    gdouble                 mnUnmutedVolume;
    bool                    mbPlayPending;
    bool                    mbMuted;
    bool                    mbLooping;
    bool                    mbInitialized;

    long                    mnWindowID;
    GstVideoOverlay*        mpXOverlay;
    gint64                  mnDuration;
    int                     mnWidth;
    int                     mnHeight;

    css::awt::Rectangle     maArea;     // Area of the player window.

    guint                   mnWatchID;
    bool                    mbWatchID;

    osl::Condition          maSizeCondition;
};

} // namespace gstreamer
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTPLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
