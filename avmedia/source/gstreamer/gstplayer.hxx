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

#ifndef _PLAYER_HXX
#define _PLAYER_HXX

#include <osl/conditn.hxx>
#include "gstcommon.hxx"

#include "com/sun/star/media/XPlayer.hpp"
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

typedef struct _GstVideoOverlay GstVideoOverlay;

namespace avmedia { namespace gstreamer {

// ----------
// - Player -
// ----------

typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::media::XPlayer,
                                          ::com::sun::star::lang::XServiceInfo > GstPlayer_BASE;

class Player : public ::cppu::BaseMutex,
               public GstPlayer_BASE
{
public:

    Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
    ~Player();

    void preparePlaybin( const OUString& rURL, GstElement *pSink );
    bool create( const OUString& rURL );
    virtual void processMessage( GstMessage *message );
    virtual GstBusSyncReply processSyncMessage( GstMessage *message );

    // XPlayer
    virtual void SAL_CALL start(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaying(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getDuration(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMediaTime( double fTime ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getMediaTime(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getRate(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMute( sal_Bool bSet ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isMute(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getVolumeDB(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getPreferredPlayerWindowSize(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > SAL_CALL createPlayerWindow( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XFrameGrabber > SAL_CALL createFrameGrabber(  ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing(void);

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;

    OUString                maURL;

    // Add elements and pipeline here
    GstElement*             mpPlaybin;  // the playbin is also a pipeline
    sal_Bool                mbFakeVideo;

    gdouble                 mnUnmutedVolume;
    sal_Bool                mbPlayPending;
    sal_Bool                mbMuted;
    sal_Bool                mbLooping;
    sal_Bool                mbInitialized;

    long                    mnWindowID;
    GstVideoOverlay*        mpXOverlay;
    gint64                  mnDuration;
    int                     mnWidth;
    int                     mnHeight;

    osl::Condition          maSizeCondition;
};

} // namespace gstreamer
} // namespace avmedia

#endif // _PLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
