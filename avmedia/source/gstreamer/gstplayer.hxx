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

#include "com/sun/star/media/XPlayer.hpp"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

typedef struct _GstVideoOverlay GstVideoOverlay;

namespace avmedia { namespace gstreamer {


// - Player -


typedef ::cppu::WeakComponentImplHelper< css::media::XPlayer,
                                         css::lang::XServiceInfo > GstPlayer_BASE;

class Player : public ::cppu::BaseMutex,
               public GstPlayer_BASE
{
public:

    explicit Player( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxMgr );
    virtual ~Player();

    void preparePlaybin( const OUString& rURL, GstElement *pSink );
    bool create( const OUString& rURL );
    void processMessage( GstMessage *message );
    GstBusSyncReply processSyncMessage( GstMessage *message );

    // XPlayer
    virtual void SAL_CALL start(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL stop(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isPlaying(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDuration(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMediaTime( double fTime ) throw (css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getMediaTime(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMute( sal_Bool bSet ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isMute(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::media::XPlayerWindow > SAL_CALL createPlayerWindow( const css::uno::Sequence< css::uno::Any >& aArguments ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMgr;

    OUString                maURL;

    // Add elements and pipeline here
    GstElement*             mpPlaybin;  // the playbin is also a pipeline
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

    guint                   mnWatchID;
    bool                    mbWatchID;

    osl::Condition          maSizeCondition;
};

} // namespace gstreamer
} // namespace avmedia

#endif // INCLUDED_AVMEDIA_SOURCE_GSTREAMER_GSTPLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
