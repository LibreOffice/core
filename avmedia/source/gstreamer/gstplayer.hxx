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

#ifndef _GSTPLAYER_HXX
#define _GSTPLAYER_HXX

#include "gstcommon.hxx"
#include <glib.h>
#include <glib/gatomic.h>

// necessary for mixed environments with GStreamer-0.10 and GLib versions < 2.8
#ifndef G_GNUC_NULL_TERMINATED
#if __GNUC__ >= 4
#define G_GNUC_NULL_TERMINATED __attribute__((__sentinel__))
#else
#define G_GNUC_NULL_TERMINATED
#endif
#endif

struct _GOptionGroup;
typedef struct _GOptionGroup GOptionGroup;

#include <gst/gst.h>
#include "com/sun/star/media/XPlayer.hdl"

namespace avmedia
{
namespace gst
{
class Window;

// ---------------
// - Player_Impl -
// ---------------

class Player : public ::cppu::WeakImplHelper3< ::com::sun::star::media::XPlayer,
                                               ::com::sun::star::lang::XComponent,
                                               ::com::sun::star::lang::XServiceInfo >
{
public:

    // static create method instead of public Ctor
    static Player* create( const ::rtl::OUString& rURL );

    ~Player();

    // XPlayer
    virtual void SAL_CALL start()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL stop()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isPlaying()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL getDuration()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setMediaTime( double fTime )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL getMediaTime()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setStopTime( double fTime )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL getStopTime()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setRate( double fRate )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual double SAL_CALL getRate()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setPlaybackLoop( sal_Bool bSet )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isPlaybackLoop()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setMute( sal_Bool bSet )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isMute()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setVolumeDB( sal_Int16 nVolumeDB )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL getVolumeDB()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Size SAL_CALL getPreferredPlayerWindowSize()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > SAL_CALL createPlayerWindow(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::media::XFrameGrabber > SAL_CALL createFrameGrabber()
     throw( ::com::sun::star::uno::RuntimeException );

    // XComponent
    virtual void SAL_CALL dispose()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
     throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
     throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
     throw( ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
     throw( ::com::sun::star::uno::RuntimeException );

// these are public because the C callbacks call them
    virtual gboolean busCallback( GstBus* pBus,
                                  GstMessage* pMsg );

    virtual gboolean idle();

    virtual gpointer run();

    virtual GstBusSyncReply handleCreateWindow( GstBus* pBus,
                                                GstMessage* pMsg );

protected:

    Player( GString* pURI = NULL );

    void implQuitThread();

    bool implInitPlayer();

    bool implIsInitialized() const
    {
        return( g_atomic_int_get( &mnInitialized ) > 0 );
    }


private:

    Player( const Player& );

    Player& operator=( const Player& );

    static void implHandleNewElementFunc( GstBin* pBin,
                                          GstElement* pElement,
                                          gpointer pData );

    static void implHandleNewPadFunc( GstElement* pElem,
                                      GstPad* pPad,
                                      gpointer pData );

protected:

    GMutex* mpMutex;
    GCond* mpCond;
    GThread* mpThread;
    GMainContext* mpContext;
    GMainLoop* mpLoop;
    GstElement* mpPlayer;
    GString* mpURI;

private:

    ::avmedia::gst::Window* mpPlayerWindow;
    gint mnIsVideoSource;
    gint mnVideoWidth;
    gint mnVideoHeight;
    gint mnInitialized;
    gint mnVolumeDB;
    gint mnLooping;
    gint mnQuit;
    gint mnVideoWindowSet;
    gint mnInitFail;
};
}     // namespace gst
} // namespace avmedia

#endif // _GSTPLAYER_HXX
