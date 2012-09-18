/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _PLAYER_HXX
#define _PLAYER_HXX

#include "wincommon.hxx"

#include "com/sun/star/media/XPlayer.hpp"

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

namespace avmedia { namespace win {

// ----------
// - Player -
// ----------

class Player : public ::cppu::WeakImplHelper2< ::com::sun::star::media::XPlayer,
                                               ::com::sun::star::lang::XServiceInfo >
{
public:

                        Player( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxMgr );
                        ~Player();

    bool                create( const OUString& rURL );

    void                setNotifyWnd( int nNotifyWnd );
    void                setDDrawParams( IDirectDraw* pDDraw, IDirectDrawSurface* pDDrawSurface );
    long                processEvent();

    const IVideoWindow* getVideoWindow() const;

    // XPlayer
    virtual void SAL_CALL start(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL stop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaying(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getDuration(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMediaTime( double fTime ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getMediaTime(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getRate(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPlaybackLoop( bool bSet ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isPlaybackLoop(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setMute( bool bSet ) throw (::com::sun::star::uno::RuntimeException);
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

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxMgr;

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
    bool                    mbMuted;
    bool                    mbLooping;

    void                    ImplLayoutVideoWindow();
};

} // namespace win
} // namespace avmedia

#endif // _PLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
