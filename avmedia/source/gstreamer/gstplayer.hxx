/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>

namespace avmedia
{
namespace gst
{
class Window;

// ---------------
// - Player_Impl -
// ---------------
typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::media::XPlayer,
                                          ::com::sun::star::lang::XServiceInfo > Player_BASE;

class Player :  public cppu::BaseMutex,
                public Player_BASE
{
public:

    // static create method instead of public Ctor
    static Player* create( const ::rtl::OUString& rURL );

    ~Player();
//protected:
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

    // ::cppu::OComponentHelper
    virtual void SAL_CALL disposing(void);

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
