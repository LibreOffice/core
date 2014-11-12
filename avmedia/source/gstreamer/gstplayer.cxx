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

#include <math.h>

#include <cppuhelper/supportsservice.hxx>

#include <rtl/string.hxx>

#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "gstplayer.hxx"
#include "gstframegrabber.hxx"
#include "gstwindow.hxx"

#ifdef AVMEDIA_GST_0_10
#  define AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer_0_10"
#  define AVMEDIA_GST_PLAYER_SERVICENAME        "com.sun.star.media.Player_GStreamer_0_10"
#else
#  include <gst/video/videooverlay.h>
#  define AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer"
#  define AVMEDIA_GST_PLAYER_SERVICENAME        "com.sun.star.media.Player_GStreamer"
#endif

#if !defined DBG
# if OSL_DEBUG_LEVEL > 2
#ifdef AVMEDIA_GST_0_10
#  define AVVERSION "gst 0.10: "
#else
#  define AVVERSION "gst 1.0: "
#endif
#define DBG(...) do { fprintf (stderr, "%s", AVVERSION); fprintf (stderr, __VA_ARGS__); fprintf (stderr, "\n"); } while (0);
# else
#define DBG(...)
# endif
#endif

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {


// - Player -


Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    GstPlayer_BASE( m_aMutex ),
    mxMgr( rxMgr ),
    mpPlaybin( NULL ),
    mbFakeVideo (false ),
    mnUnmutedVolume( 0 ),
    mbPlayPending ( false ),
    mbMuted( false ),
    mbLooping( false ),
    mbInitialized( false ),
    mnWindowID( 0 ),
    mpXOverlay( NULL ),
    mnDuration( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnWatchID( 0 ),
    mbWatchID( false )
{
    // Initialize GStreamer library
    int argc = 1;
    char name[] = "libreoffice";
    char *arguments[] = { name };
    char** argv = arguments;
    GError* pError = NULL;

    mbInitialized = gst_init_check( &argc, &argv, &pError );

    DBG( "%p Player::Player", this );

    if (pError != NULL)
    {
        // TODO: thow an exception?
        DBG( "%p Player::Player error '%s'", this, pError->message );
        g_error_free (pError);
    }
}



Player::~Player()
{
    DBG( "%p Player::~Player", this );
    if( mbInitialized )
        disposing();
}

void SAL_CALL Player::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    stop();

    DBG( "%p Player::disposing", this );

    // Release the elements and pipeline
    if( mbInitialized )
    {
        if( mpPlaybin )
        {
            gst_element_set_state( mpPlaybin, GST_STATE_NULL );
            g_object_unref( G_OBJECT( mpPlaybin ) );

            mpPlaybin = NULL;
        }

        if( mpXOverlay ) {
            g_object_unref( G_OBJECT ( mpXOverlay ) );
            mpXOverlay = NULL;
        }

    }
    if (mbWatchID)
    {
        g_source_remove(mnWatchID);
        mbWatchID = false;
    }
}

static gboolean pipeline_bus_callback( GstBus *, GstMessage *message, gpointer data )
{
    Player* pPlayer = static_cast<Player*>(data);

    pPlayer->processMessage( message );

    return TRUE;
}

static GstBusSyncReply pipeline_bus_sync_handler( GstBus *, GstMessage * message, gpointer data )
{
    Player* pPlayer = static_cast<Player*>(data);

    return pPlayer->processSyncMessage( message );
}

void Player::processMessage( GstMessage *message )
{
    switch( GST_MESSAGE_TYPE( message ) ) {
    case GST_MESSAGE_EOS:
        gst_element_set_state( mpPlaybin, GST_STATE_READY );
        mbPlayPending = false;
        if (mbLooping)
            start();
        break;
    case GST_MESSAGE_STATE_CHANGED:
        if( message->src == GST_OBJECT( mpPlaybin ) ) {
            GstState newstate, pendingstate;

            gst_message_parse_state_changed (message, NULL, &newstate, &pendingstate);

            if( newstate == GST_STATE_PAUSED &&
                pendingstate == GST_STATE_VOID_PENDING &&
                mpXOverlay )
                gst_video_overlay_expose( mpXOverlay );

        if (mbPlayPending)
            mbPlayPending = ((newstate == GST_STATE_READY) || (newstate == GST_STATE_PAUSED));
        }
    default:
        break;
    }
}

static gboolean wrap_element_query_position (GstElement *element, GstFormat format, gint64 *cur)
{
#ifdef AVMEDIA_GST_0_10
    GstFormat my_format = format;
    return gst_element_query_position( element, &my_format, cur) && my_format == format && *cur > 0L;
#else
    return gst_element_query_position( element, format, cur );
#endif
}

static gboolean wrap_element_query_duration (GstElement *element, GstFormat format, gint64 *duration)
{
#ifdef AVMEDIA_GST_0_10
    GstFormat my_format = format;
    return gst_element_query_duration( element, &my_format, duration) && my_format == format && *duration > 0L;
#else
    return gst_element_query_duration( element, format, duration );
#endif
}

GstBusSyncReply Player::processSyncMessage( GstMessage *message )
{
//    DBG( "%p processSyncMessage has handle: %s", this, GST_MESSAGE_TYPE_NAME( message ) );

#if OSL_DEBUG_LEVEL > 0
    if ( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR )
    {
        GError* error;
        gchar* error_debug;

        gst_message_parse_error( message, &error, &error_debug );
        SAL_WARN(
            "avmedia",
            "gstreamer error: '" << error->message << "' debug: '"
                << error_debug << "'");
    }
#endif

#ifdef AVMEDIA_GST_0_10
    if (message->structure &&
        !strcmp( gst_structure_get_name( message->structure ), "prepare-xwindow-id" ) )
#else
    if (gst_is_video_overlay_prepare_window_handle_message (message) )
#endif
    {
        DBG( "%p processSyncMessage prepare window id: %s %d", this,
             GST_MESSAGE_TYPE_NAME( message ), (int)mnWindowID );
        if( mpXOverlay )
            g_object_unref( G_OBJECT ( mpXOverlay ) );
        g_object_set( GST_MESSAGE_SRC( message ), "force-aspect-ratio", FALSE, NULL );
        mpXOverlay = GST_VIDEO_OVERLAY( GST_MESSAGE_SRC( message ) );
        g_object_ref( G_OBJECT ( mpXOverlay ) );
        if ( mnWindowID != 0 )
            gst_video_overlay_set_window_handle( mpXOverlay, mnWindowID );
        return GST_BUS_DROP;
    }

#ifdef AVMEDIA_GST_0_10
    if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_STATE_CHANGED ) {
        if( message->src == GST_OBJECT( mpPlaybin ) ) {
            GstState newstate, pendingstate;

            gst_message_parse_state_changed (message, NULL, &newstate, &pendingstate);

            DBG( "%p state change received, new state %d pending %d", this,
                 (int)newstate, (int)pendingstate );
            if( newstate == GST_STATE_PAUSED &&
                pendingstate == GST_STATE_VOID_PENDING ) {

                DBG( "%p change to paused received", this );

                if( mnDuration == 0) {
                    gint64 gst_duration = 0L;
                    if( wrap_element_query_duration( mpPlaybin, GST_FORMAT_TIME, &gst_duration) )
                        mnDuration = gst_duration;
                }

                if( mnWidth == 0 ) {
                    GList *pStreamInfo = NULL;

                    g_object_get( G_OBJECT( mpPlaybin ), "stream-info", &pStreamInfo, NULL );

                    for ( ; pStreamInfo != NULL; pStreamInfo = pStreamInfo->next) {
                        GObject *pInfo = G_OBJECT( pStreamInfo->data );

                        if( !pInfo )
                            continue;

                        int nType;
                        g_object_get( pInfo, "type", &nType, NULL );
                        GEnumValue *pValue = g_enum_get_value( G_PARAM_SPEC_ENUM( g_object_class_find_property( G_OBJECT_GET_CLASS( pInfo ), "type" ) )->enum_class,
                                                               nType );

                        if( !g_ascii_strcasecmp( pValue->value_nick, "video" ) ) {
                            GstStructure *pStructure;
                            GstPad *pPad;

                            g_object_get( pInfo, "object", &pPad, NULL );
                            pStructure = gst_caps_get_structure( GST_PAD_CAPS( pPad ), 0 );
                            if( pStructure ) {
                                gst_structure_get_int( pStructure, "width", &mnWidth );
                                gst_structure_get_int( pStructure, "height", &mnHeight );
                                DBG( "queried size: %d x %d", mnWidth, mnHeight );
                            }
                            g_object_unref (pPad);
                        }
                    }

                    maSizeCondition.set();
                }
            }
        }
#else
    // We get to use the exciting new playbin2 ! (now known as playbin)
    if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ASYNC_DONE ) {
        if( mnDuration == 0) {
            gint64 gst_duration = 0L;
            if( wrap_element_query_duration( mpPlaybin, GST_FORMAT_TIME, &gst_duration) )
                mnDuration = gst_duration;
        }
        if( mnWidth == 0 ) {
            GstPad *pad = NULL;
            GstCaps *caps;

            g_signal_emit_by_name( mpPlaybin, "get-video-pad", 0, &pad );

            if( pad ) {
                int w = 0, h = 0;

                caps = gst_pad_get_current_caps( pad );

                if( gst_structure_get( gst_caps_get_structure( caps, 0 ),
                                       "width", G_TYPE_INT, &w,
                                       "height", G_TYPE_INT, &h,
                                       NULL ) ) {
                    mnWidth = w;
                    mnHeight = h;

                    DBG( "queried size: %d x %d", mnWidth, mnHeight );

                    maSizeCondition.set();
                }
                gst_caps_unref( caps );
                g_object_unref( pad );
            }
        }
#endif
    } else if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR ) {
        DBG( "Error !\n" );
        if( mnWidth == 0 ) {
            // an error occurred, set condition so that OOo thread doesn't wait for us
            maSizeCondition.set();
        }
    }

    return GST_BUS_PASS;
}

void Player::preparePlaybin( const OUString& rURL, GstElement *pSink )
{
        GstBus *pBus;

        if( mpPlaybin != NULL ) {
            gst_element_set_state( mpPlaybin, GST_STATE_NULL );
            mbPlayPending = false;
            g_object_unref( mpPlaybin );
        }

        mpPlaybin = gst_element_factory_make( "playbin", NULL );
        if( pSink != NULL ) // used for getting preferred size etc.
        {
            g_object_set( G_OBJECT( mpPlaybin ), "video-sink", pSink, NULL );
            mbFakeVideo = true;
        }
        else
            mbFakeVideo = false;

        OString ascURL = OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 );
        g_object_set( G_OBJECT( mpPlaybin ), "uri", ascURL.getStr() , NULL );

        pBus = gst_element_get_bus( mpPlaybin );
        if (mbWatchID)
        {
            g_source_remove(mnWatchID);
            mbWatchID = false;
        }
        mnWatchID = gst_bus_add_watch( pBus, pipeline_bus_callback, this );
        mbWatchID = true;
        DBG( "%p set sync handler", this );
#ifdef AVMEDIA_GST_0_10
        gst_bus_set_sync_handler( pBus, pipeline_bus_sync_handler, this );
#else
        gst_bus_set_sync_handler( pBus, pipeline_bus_sync_handler, this, NULL );
#endif
        g_object_unref( pBus );
}

bool Player::create( const OUString& rURL )
{
    bool    bRet = false;

    // create all the elements and link them

    DBG("create player, URL: %s", OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 ).getStr());

    if( mbInitialized && !rURL.isEmpty() )
    {
        // fakesink for pre-roll & sizing ...
        preparePlaybin( rURL, gst_element_factory_make( "fakesink", NULL ) );

        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
        mbPlayPending = false;

        bRet = true;
    }

    if( bRet )
        maURL = rURL;
    else
        maURL.clear();

    return bRet;
}



void SAL_CALL Player::start()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // set the pipeline state to READY and run the loop
    if( mbInitialized && NULL != mpPlaybin )
    {
        gst_element_set_state( mpPlaybin, GST_STATE_PLAYING );
        mbPlayPending = true;
    }
}



void SAL_CALL Player::stop()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // set the pipeline in PAUSED STATE
    if( mpPlaybin )
        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );

    mbPlayPending = false;
    DBG( "stop %p", mpPlaybin );
}



sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    bool            bRet = mbPlayPending;

    // return whether the pipeline is in PLAYING STATE or not
    if( !mbPlayPending && mbInitialized && mpPlaybin )
    {
        bRet = GST_STATE_PLAYING == GST_STATE( mpPlaybin );
    }

    DBG( "isPlaying %d", bRet );

    return bRet;
}



double SAL_CALL Player::getDuration()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.01;

    if( mpPlaybin && mnDuration > 0 ) {
        duration = mnDuration / GST_SECOND;
    }

    return duration;
}



void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if( mpPlaybin ) {
        gint64 gst_position = llround (fTime * GST_SECOND);

        gst_element_seek( mpPlaybin, 1.0,
                          GST_FORMAT_TIME,
                          GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET, gst_position,
                          GST_SEEK_TYPE_NONE, 0 );
        if( !isPlaying() )
            gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );

        DBG( "seek to: %" SAL_PRIdINT64 " ns original: %lf s", gst_position, fTime );
    }
}



double SAL_CALL Player::getMediaTime()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    double position = 0.0;

    if( mpPlaybin ) {
        // get current position in the stream
        gint64 gst_position;
        if( wrap_element_query_position( mpPlaybin, GST_FORMAT_TIME, &gst_position ) )
            position = gst_position / GST_SECOND;
    }

    return position;
}

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // TODO check how to do with GST
    mbLooping = bSet;
}



sal_Bool SAL_CALL Player::isPlaybackLoop()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // TODO check how to do with GST
    return mbLooping;
}



void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    DBG( "set mute: %d muted: %d unmuted volume: %lf", bSet, mbMuted, mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if(  mpPlaybin && (mbMuted ? 1 : 0) != bSet )
    {
        double nVolume = mnUnmutedVolume;
        if( bSet )
        {
            nVolume = 0.0;
        }

        g_object_set( G_OBJECT( mpPlaybin ), "volume", nVolume, NULL );

        mbMuted = bSet;
    }
}



sal_Bool SAL_CALL Player::isMute()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    return mbMuted;
}



void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    mnUnmutedVolume = pow( 10.0, nVolumeDB / 20.0 );

    DBG( "set volume: %d gst volume: %lf", nVolumeDB, mnUnmutedVolume );

    // change volume
     if( !mbMuted && mpPlaybin )
     {
         g_object_set( G_OBJECT( mpPlaybin ), "volume", (gdouble) mnUnmutedVolume, NULL );
     }
}



sal_Int16 SAL_CALL Player::getVolumeDB()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    sal_Int16 nVolumeDB(0);

    if( mpPlaybin ) {
        double nGstVolume = 0.0;

        g_object_get( G_OBJECT( mpPlaybin ), "volume", &nGstVolume, NULL );

        nVolumeDB = (sal_Int16) ( 20.0*log10 ( nGstVolume ) );
    }

    return nVolumeDB;
}



awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    awt::Size aSize( 0, 0 );

    if( maURL.isEmpty() )
    {
        DBG( "%p Player::getPreferredPlayerWindowSize - empty URL => 0x0", this );
        return aSize;
    }

    DBG( "%p pre-Player::getPreferredPlayerWindowSize, member %d x %d", this, mnWidth, mnHeight );

    TimeValue aTimeout = { 10, 0 };
#if OSL_DEBUG_LEVEL > 2
    osl::Condition::Result aResult =
#endif
                                 maSizeCondition.wait( &aTimeout );

    DBG( "%p Player::getPreferredPlayerWindowSize after waitCondition %d, member %d x %d", this, aResult, mnWidth, mnHeight );

    if( mnWidth != 0 && mnHeight != 0 ) {
        aSize.Width = mnWidth;
        aSize.Height = mnHeight;
    }

    return aSize;
}



uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( mbFakeVideo )
        preparePlaybin( maURL, NULL );

    DBG( "Player::createPlayerWindow %d %d length: %d", aSize.Width, aSize.Height, rArguments.getLength() );

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::gstreamer::Window* pWindow = new ::avmedia::gstreamer::Window( mxMgr, *this );

        xRet = pWindow;

        if( rArguments.getLength() > 2 )
        {
            sal_IntPtr pIntPtr = 0;
            rArguments[ 2 ] >>= pIntPtr;
            SystemChildWindow *pParentWindow = reinterpret_cast< SystemChildWindow* >( pIntPtr );
            const SystemEnvData* pEnvData = pParentWindow ? pParentWindow->GetSystemData() : NULL;
            OSL_ASSERT(pEnvData);
            if (pEnvData)
            {
                mnWindowID = pEnvData->aWindow;
                DBG( "set window id to %d XOverlay %p\n", (int)mnWindowID, mpXOverlay);
                gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
                if ( mpXOverlay != NULL )
                    gst_video_overlay_set_window_handle( mpXOverlay, mnWindowID );
            }
        }
    }

    return xRet;
}



uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    FrameGrabber* pFrameGrabber = NULL;
    const awt::Size aPrefSize( getPreferredPlayerWindowSize() );

    if( ( aPrefSize.Width > 0 ) && ( aPrefSize.Height > 0 ) )
        pFrameGrabber = FrameGrabber::create( maURL );
    DBG( "created FrameGrabber %p", pFrameGrabber );

    return pFrameGrabber;
}



OUString SAL_CALL Player::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString( AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME );
}



sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}



uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_GST_PLAYER_SERVICENAME ;

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
