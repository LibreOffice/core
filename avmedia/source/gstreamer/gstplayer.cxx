/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
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

#include <math.h>

#include <rtl/string.hxx>

#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "gstplayer.hxx"
#include "gstframegrabber.hxx"
#include "gstwindow.hxx"

#ifndef AVMEDIA_GST_0_10
#  include <gst/video/videooverlay.h>
#endif

#define AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.Player_GStreamer"
#define AVMEDIA_GST_PLAYER_SERVICENAME "com.sun.star.media.Player_GStreamer"

#ifdef AVMEDIA_GST_0_10
#  define AVVERSION "gst 0.10: "
#else
#  define AVVERSION "gst 1.0: "
#endif

#if 1 //OSL_DEBUG_LEVEL > 2
#define DBG(...) do { fprintf (stderr, "%s", AVVERSION); fprintf (stderr, __VA_ARGS__); fprintf (stderr, "\n"); } while (0);
#else
#define DBG(...)
#endif

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {

// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr ),
    mpPlaybin( NULL ),
    mbFakeVideo (sal_False ),
    mnUnmutedVolume( 0 ),
    mbPlayPending ( false ),
    mbMuted( false ),
    mbLooping( false ),
    mbInitialized( false ),
    mnWindowID( 0 ),
    mpXOverlay( NULL ),
    mnDuration( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 )
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

// ------------------------------------------------------------------------------

Player::~Player()
{
    DBG( "%p Player::~Player", this );

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
}

// ------------------------------------------------------------------------------

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

#if 1 // OSL_DEBUG_LEVEL > 0
    if ( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR )
    {
        GError* error;
        gchar* error_debug;

        gst_message_parse_error( message, &error, &error_debug );
        fprintf(stderr, "gstreamer error: '%s' debug: '%s'", error->message, error_debug);
    }
#endif

#ifdef AVMEDIA_GST_0_10
    if (message->structure &&
        !strcmp( gst_structure_get_name( message->structure ), "prepare-xwindow-id" ) && mnWindowID != 0 )
#else
    if (gst_message_has_name (message, "prepare-window-handle") && mnWindowID != 0 )
#endif
    {
        DBG( "%p processSyncMessage has handle: %s", this, GST_MESSAGE_TYPE_NAME( message ) );
        if( mpXOverlay )
            g_object_unref( G_OBJECT ( mpXOverlay ) );
        mpXOverlay = GST_VIDEO_OVERLAY( GST_MESSAGE_SRC( message ) );
        g_object_ref( G_OBJECT ( mpXOverlay ) );
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

                if( gst_structure_get( gst_caps_get_structure (caps, 0),
                                       "width", G_TYPE_INT, &w,
                                       "height", G_TYPE_INT, &h,
                                       NULL ) ) {
                    mnWidth = w;
                    mnHeight = h;

                    fprintf (stderr, "queried size: %d x %d", mnWidth, mnHeight );

                    maSizeCondition.set();
                }
                gst_caps_unref( caps );
            }
        }
#endif
    } else if( GST_MESSAGE_TYPE( message ) == GST_MESSAGE_ERROR ) {
        fprintf (stderr, "Error !\n");
        if( mnWidth == 0 ) {
            // an error occurred, set condition so that OOo thread doesn't wait for us
            maSizeCondition.set();
        }
    }

    return GST_BUS_PASS;
}

void Player::preparePlaybin( const ::rtl::OUString& rURL, bool bFakeVideo )
{
        GstBus *pBus;

        if( mpPlaybin != NULL ) {
            gst_element_set_state( mpPlaybin, GST_STATE_NULL );
            mbPlayPending = false;
            g_object_unref( mpPlaybin );
        }

        mpPlaybin = gst_element_factory_make( "playbin", NULL );

        if( bFakeVideo )
            g_object_set( G_OBJECT( mpPlaybin ), "video-sink", gst_element_factory_make( "fakesink", NULL ), NULL );

        mbFakeVideo = bFakeVideo;

        rtl::OString ascURL = OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 );
        g_object_set( G_OBJECT( mpPlaybin ), "uri", ascURL.getStr() , NULL );

        pBus = gst_element_get_bus( mpPlaybin );
        gst_bus_add_watch( pBus, pipeline_bus_callback, this );
        DBG( "%p set sync handler", this );
        gst_bus_set_sync_handler( pBus, pipeline_bus_sync_handler, this );
        g_object_unref( pBus );
}

bool Player::create( const ::rtl::OUString& rURL )
{
    bool    bRet = false;

    // create all the elements and link them

    DBG("create player, URL: %s", OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 ).getStr());

    if( mbInitialized )
    {
        preparePlaybin( rURL, true );

        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
        mbPlayPending = false;

        bRet = true;
    }

    if( bRet )
        maURL = rURL;
    else
        maURL = ::rtl::OUString();

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start()
    throw (uno::RuntimeException)
{
    // set the pipeline state to READY and run the loop
    if( mbInitialized && NULL != mpPlaybin )
    {
        gst_element_set_state( mpPlaybin, GST_STATE_PLAYING );
        mbPlayPending = true;
    }
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop()
    throw (uno::RuntimeException)
{
    // set the pipeline in PAUSED STATE
    if( mpPlaybin )
        gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );

    mbPlayPending = false;
    DBG( "stop %p", mpPlaybin );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    bool            bRet = mbPlayPending;

    // return whether the pipeline is in PLAYING STATE or not
    if( !mbPlayPending && mbInitialized && mpPlaybin )
    {
        bRet = GST_STATE_PLAYING == GST_STATE( mpPlaybin );
    }

    DBG( "isPlaying %d", bRet );

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration()
    throw (uno::RuntimeException)
{
    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.01;

    if( mpPlaybin && mnDuration > 0 ) {
        duration = mnDuration / 1E9;
    }

    return duration;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    if( mpPlaybin ) {
        gint64 gst_position = llround (fTime * 1E9);

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

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime()
    throw (uno::RuntimeException)
{
    double position = 0.0;

    if( mpPlaybin ) {
        // get current position in the stream
        gint64 gst_position;
        if( wrap_element_query_position( mpPlaybin, GST_FORMAT_TIME, &gst_position ) )
            position = gst_position / 1E9;
    }

    return position;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate()
    throw (uno::RuntimeException)
{
    double rate = 0.0;

    // TODO get the window rate
    if( mbInitialized )
    {

    }

    return rate;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    // TODO check how to do with GST
    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop()
    throw (uno::RuntimeException)
{
    // TODO check how to do with GST
    return mbLooping;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    DBG( "set mute: %d muted: %d unmuted volume: %lf", bSet, mbMuted, mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if(  mpPlaybin && mbMuted != bSet )
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

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute()
    throw (uno::RuntimeException)
{
    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
    mnUnmutedVolume = pow( 10.0, nVolumeDB / 20.0 );

    DBG( "set volume: %d gst volume: %lf", nVolumeDB, mnUnmutedVolume );

    // change volume
     if( !mbMuted && mpPlaybin )
     {
         g_object_set( G_OBJECT( mpPlaybin ), "volume", (gdouble) mnUnmutedVolume, NULL );
     }
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB()
    throw (uno::RuntimeException)
{
    sal_Int16 nVolumeDB(0);

    if( mpPlaybin ) {
        double nGstVolume = 0.0;

        g_object_get( G_OBJECT( mpPlaybin ), "volume", &nGstVolume, NULL );

        nVolumeDB = (sal_Int16) ( 20.0*log10 ( nGstVolume ) );
    }

    return nVolumeDB;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
    throw (uno::RuntimeException)
{
    awt::Size aSize( 0, 0 );

    DBG( "%p Player::getPreferredPlayerWindowSize, member %d x %d", this, mnWidth, mnHeight );

    TimeValue aTimeout = { 10, 0 };
#if 1 // OSL_DEBUG_LEVEL > 2
    osl::Condition::Result aResult =
#endif
                                 maSizeCondition.wait( &aTimeout );

    if( mbFakeVideo ) {
        mbFakeVideo = sal_False;

         g_object_set( G_OBJECT( mpPlaybin ), "video-sink", NULL, NULL );
         gst_element_set_state( mpPlaybin, GST_STATE_READY );
         gst_element_set_state( mpPlaybin, GST_STATE_PAUSED );
    }

    DBG( "%p Player::getPreferredPlayerWindowSize after waitCondition %d, member %d x %d", this, aResult, mnWidth, mnHeight );

    if( mnWidth != 0 && mnHeight != 0 ) {
        aSize.Width = mnWidth;
        aSize.Height = mnHeight;
    }

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& rArguments )
    throw (uno::RuntimeException)
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

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
                mnWindowID = pEnvData->aWindow;
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
    throw (uno::RuntimeException)
{
    uno::Reference< media::XFrameGrabber > xRet;

    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName()
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GST_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_GST_PLAYER_SERVICENAME;
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_GST_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
