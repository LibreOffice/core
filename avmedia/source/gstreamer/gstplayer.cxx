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

#include "gstplayer.hxx"
#include "gstwindow.hxx"
#include "gstframegrabber.hxx"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string>
#include <gst/gstelement.h>
#include <gst/interfaces/xoverlay.h>


// maximum timeout time in nanoseconds
#define GST_MAX_TIMEOUT (2500 * GST_MSECOND)

using namespace ::com::sun::star;

namespace avmedia
{
namespace gst
{
const double NANO_TIME_FACTOR = 1000000000.0;

const long      VIDEO_DEFAULT_WIDTH = 256;
const long      VIDEO_DEFAULT_HEIGHT = 192;

// ----------------
// - GstBusSource -
// ----------------

struct GstBusSource : public GSource
{
    GstBus* mpBus;

    GstBusSource() :
        mpBus( NULL )
    {}

    ~GstBusSource()
    {}
};


// -----------------------------------------------------------------------
extern "C"
{

static gpointer
lcl_implThreadFunc( gpointer pData )
{
    return( pData ? static_cast< Player* >( pData )->run() : NULL );
}

static gboolean
lcl_implBusCheck( GSource* pSource )
{
    GstBusSource* pBusSource = static_cast< GstBusSource* >( pSource );

    return( pBusSource &&
           GST_IS_BUS( pBusSource->mpBus ) &&
           gst_bus_have_pending( GST_BUS_CAST( pBusSource->mpBus ) ) );
}


static gboolean
lcl_implBusPrepare( GSource* pSource, gint* pTimeout )
{
    if (pTimeout)
    {
        *pTimeout = 0;
    }

    return lcl_implBusCheck(pSource);
}

static gboolean
lcl_implBusDispatch( GSource* pSource,
                     GSourceFunc /*aCallback*/,
                     gpointer pData )
{
    GstBusSource* pBusSource = static_cast< GstBusSource* >( pSource );
    gboolean bRet = false;

    if( pData && pBusSource && GST_IS_BUS( pBusSource->mpBus ) )
    {
        GstMessage* pMsg = gst_bus_pop( pBusSource->mpBus );

        if( pMsg )
        {
            bRet = static_cast< Player* >( pData )->busCallback(
                        pBusSource->mpBus, pMsg );
            gst_message_unref( pMsg );
        }
    }

    return( bRet );
}

static void
lcl_implBusFinalize( GSource* pSource )
{
    GstBusSource* pBusSource = static_cast< GstBusSource* >( pSource );

    if( pBusSource && pBusSource->mpBus )
    {
        gst_object_unref( pBusSource->mpBus );
        pBusSource->mpBus = NULL;
    }
}

static gboolean
lcl_implIdleFunc( gpointer pData )
{
    return( pData ? static_cast< Player* >( pData )->idle() : true );
}

static GstBusSyncReply
lcl_implHandleCreateWindowFunc( GstBus* pBus, GstMessage* pMsg, gpointer pData )
{
    return (pData)
        ? static_cast< Player* >( pData )->handleCreateWindow( pBus, pMsg )
        : GST_BUS_PASS;
}

} // extern "C"



// ---------------
// - Player -
// ---------------
Player::Player( GString* pURI ) :
    mpMutex( g_mutex_new() ),
    mpCond( g_cond_new() ),
    mpThread( NULL ),
    mpContext( NULL ),
    mpLoop( NULL ),
    mpPlayer( NULL ),
    mpURI( pURI ),
    mpPlayerWindow( NULL ),
    mnIsVideoSource( 0 ),
    mnVideoWidth( 0 ),
    mnVideoHeight( 0 ),
    mnInitialized( 0 ),
    mnVolumeDB( 0 ),
    mnLooping( 0 ),
    mnQuit( 0 ),
    mnVideoWindowSet( 0 ),
    mnInitFail( 0 )
{
    // initialize GStreamer framework only once
    static bool bGstInitialized = false;

    if( !bGstInitialized )
    {
        gst_init( NULL, NULL );
        bGstInitialized = true;
    }

    if( pURI )
    {
        OSL_TRACE( ">>> --------------------------------" );
        OSL_TRACE( ">>> Creating Player object with URL: %s", pURI->str );

        mpThread = g_thread_create( &lcl_implThreadFunc, this, true, NULL );
    }
}

// ------------------------------------------------------------------------------

Player::~Player()
{
    if( g_atomic_pointer_get( &mpPlayer ) )
    {
        implQuitThread();
    }

    // cleanup
    g_cond_free( mpCond );
    g_mutex_free( mpMutex );
    g_string_free( mpURI, false );
}

// ------------------------------------------------------------------------------
Player* Player::create( const ::rtl::OUString& rURL )
{
    Player* pPlayer = NULL;

    if( rURL.getLength() )
    {
        // safely initialize GLib threading framework
        try
        {
            if( !g_thread_supported() )
            {
                g_thread_init( NULL );
            }
        }
        catch( ... )
        {}

        if( g_thread_supported() )
        {
            const INetURLObject aURL( rURL );

            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            {
                GString* pURI = g_string_new( ::rtl::OUStringToOString(
                                                 aURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                 RTL_TEXTENCODING_UTF8 ).getStr() );

                if( pURI->len )
                {
                    pPlayer = new Player( pURI );

                    // wait until thread signals that it has finished initialization
                    if( pPlayer->mpThread )
                    {
                        g_mutex_lock( pPlayer->mpMutex );

                        while( !pPlayer->implIsInitialized() )
                        {
                            g_cond_wait( pPlayer->mpCond, pPlayer->mpMutex );
                        }

                        g_mutex_unlock( pPlayer->mpMutex );
                    }

                    // check if player pipeline could be initialized
                    if( !pPlayer->mpPlayer )
                    {
                        delete pPlayer;
                        pPlayer = NULL;
                    }
                }
                else
                {
                    g_string_free( pURI, false );
                }
            }
        }
    }

    return( pPlayer );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::start()
     throw( uno::RuntimeException )
{
    if( implInitPlayer() && !isPlaying() )
    {
        gst_element_set_state( mpPlayer, GST_STATE_PLAYING );
    }
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::stop()
     throw( uno::RuntimeException )
{
    if( implInitPlayer() && isPlaying() )
    {
        gst_element_set_state( mpPlayer, GST_STATE_PAUSED );
    }
}

// ------------------------------------------------------------------------------
sal_Bool SAL_CALL Player::isPlaying()
     throw( uno::RuntimeException )
{
    GstState aState = GST_STATE_NULL;

    if( mpPlayer )
    {
        gst_element_get_state( mpPlayer, &aState, NULL, GST_MAX_TIMEOUT );
    }

    return( GST_STATE_PLAYING == aState );
}

// ------------------------------------------------------------------------------
double SAL_CALL Player::getDuration()
     throw( uno::RuntimeException )
{
    gint64 nDuration = 0;

    if( implInitPlayer() )
    {
        GstFormat aFormat = GST_FORMAT_TIME;

        if( !gst_element_query_duration( mpPlayer, &aFormat, &nDuration ) ||
           ( GST_FORMAT_TIME != aFormat ) ||
           ( nDuration < 0 ) )
        {
            nDuration = 0;
        }
    }

    return( static_cast< double >( nDuration ) / NANO_TIME_FACTOR );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setMediaTime( double fTime )
     throw( uno::RuntimeException )
{
    if( implInitPlayer() )
    {
        fTime = ::std::min( ::std::max( fTime, 0.0 ), getDuration() );

        gst_element_seek_simple( mpPlayer, GST_FORMAT_TIME,
                                (GstSeekFlags) ( GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT ),
                                static_cast< gint64 >( fTime * NANO_TIME_FACTOR ) );
    }
}

// ------------------------------------------------------------------------------
double SAL_CALL Player::getMediaTime()
     throw( uno::RuntimeException )
{
    double fRet = 0.0;

    if( implInitPlayer() )
    {
        GstFormat aFormat = GST_FORMAT_TIME;
        gint64 nCurTime = 0;

        if( gst_element_query_position( mpPlayer, &aFormat, &nCurTime ) &&
           ( GST_FORMAT_TIME == aFormat ) &&
           ( nCurTime >= 0 ) )
        {
            fRet = static_cast< double >( nCurTime ) / NANO_TIME_FACTOR;
        }
    }

    return( fRet );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setStopTime( double /* fTime */ )
     throw( uno::RuntimeException )
{
    OSL_TRACE( "GStreamer method avmedia::gst::Player::setStopTime needs to be implemented" );

    /*  Currently no need for implementation since higher levels of code don't use this method at all
        !!! TODO: needs to be implemented if this functionality is needed at a later point of time
    if( implInitPlayer() )
    {
    }

     */
}

// ------------------------------------------------------------------------------
double SAL_CALL Player::getStopTime()
     throw( uno::RuntimeException )
{
    /*
        Currently no need for implementation since higher levels of code don't set a stop time ATM
        !!! TODO: needs to be fully implemented if this functionality is needed at a later point of time
    */

    return( getDuration() );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setRate( double /* fRate */ )
     throw( uno::RuntimeException )
{
    OSL_TRACE( "GStreamer method avmedia::gst::Player::setRate needs to be implemented" );

    /*  Currently no need for implementation since higher levels of code don't use this method at all
        !!! TODO: needs to be implemented if this functionality is needed at a later point of time
    */
}

// ------------------------------------------------------------------------------
double SAL_CALL Player::getRate()
     throw( uno::RuntimeException )
{
    /*
        Currently no need for implementation since higher levels of code don't set a different rate than 1 ATM
        !!! TODO: needs to be fully implemented if this functionality is needed at a later point of time
    */

    return( 1.0 );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
     throw( uno::RuntimeException )
{
    if( bSet && !isPlaybackLoop() )
        g_atomic_int_inc( &mnLooping );
    else if( !bSet && isPlaybackLoop() )
        g_atomic_int_dec_and_test( &mnLooping );
}

// ------------------------------------------------------------------------------
sal_Bool SAL_CALL Player::isPlaybackLoop()
     throw( uno::RuntimeException )
{
    return( g_atomic_int_get( &mnLooping ) > 0 );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setMute( sal_Bool bSet )
     throw( uno::RuntimeException )
{
    if( implInitPlayer() && ( bSet != isMute() ) )
    {
        if( bSet )
        {
            g_object_set( mpPlayer, "volume", 0.0, NULL );
        }
        else
        {
            setVolumeDB( mnVolumeDB );
        }
    }
}

// ------------------------------------------------------------------------------
sal_Bool SAL_CALL Player::isMute()
     throw( uno::RuntimeException )
{
    gdouble fGstVolume = 1.0;

    if( implInitPlayer() )
    {
        g_object_get( mpPlayer, "volume", &fGstVolume, NULL );
    }

    return( 0.0 == fGstVolume );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
     throw( uno::RuntimeException )
{
    if( implInitPlayer() )
    {
        g_mutex_lock( mpMutex );
        mnVolumeDB = nVolumeDB;
        g_mutex_unlock( mpMutex );

        // maximum gain for gstreamer volume is 10
        double fGstVolume = pow( 10.0, static_cast< double >( ::std::min(
                                                                  nVolumeDB, static_cast< sal_Int16 >( 20 ) ) / 20.0 ) );

        g_object_set( mpPlayer, "volume", fGstVolume, NULL );
    }
}

// ------------------------------------------------------------------------------
sal_Int16 SAL_CALL Player::getVolumeDB()
     throw( uno::RuntimeException )
{
    return( static_cast< sal_Int16 >( g_atomic_int_get( &mnVolumeDB ) ) );
}

// ------------------------------------------------------------------------------
awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
     throw( uno::RuntimeException )
{
    awt::Size aSize( 0, 0 );

    if( implInitPlayer() && ( g_atomic_int_get( &mnIsVideoSource ) > 0 ) )
    {
        aSize.Width = g_atomic_int_get( &mnVideoWidth );
        aSize.Height = g_atomic_int_get( &mnVideoHeight );

        // if we have a video source, but no size is given => use default size
        if( ( aSize.Width <= 0 ) || ( aSize.Height <= 0 ) )
        {
            aSize.Width = VIDEO_DEFAULT_WIDTH;
            aSize.Height = VIDEO_DEFAULT_HEIGHT;
        }
    }

    OSL_TRACE( ">>> Requested preferred video size is: %d x %d pixel", aSize.Width, aSize.Height );

    return( aSize );
}

// ------------------------------------------------------------------------------
uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow(
    const uno::Sequence< uno::Any >& rArguments )
     throw( uno::RuntimeException )
{
    uno::Reference< ::media::XPlayerWindow > xRet;
    awt::Size aSize( getPreferredPlayerWindowSize() );

    OSL_ENSURE( !g_atomic_pointer_get( &mpPlayerWindow ), "::avmedia::gst::Player already has a player window" );

    if( ( aSize.Width > 0 ) && ( aSize.Height > 0 ) )
    {
        Window* pPlayerWindow = new Window( *this );

        xRet = pPlayerWindow;

        if( !pPlayerWindow->create( rArguments ) )
        {
            xRet.clear();
        }
        else
        {
            // try to use gconf user configurable video sink first
            GstElement* pVideoSink = gst_element_factory_make( "gconfvideosink", NULL );

            if( ( NULL != pVideoSink ) ||
                ( NULL != ( pVideoSink = gst_element_factory_make( "autovideosink", NULL ) ) ) ||
                ( NULL != ( pVideoSink = gst_element_factory_make( "xvimagesink", NULL ) ) ) ||
                ( NULL != ( pVideoSink = gst_element_factory_make( "ximagesink", NULL ) ) ) )
            {
                GstState aOldState = GST_STATE_NULL;

                mpPlayerWindow = pPlayerWindow;
                gst_element_get_state( mpPlayer, &aOldState, NULL, GST_MAX_TIMEOUT );
                gst_element_set_state( mpPlayer, GST_STATE_READY );
                g_object_set( mpPlayer, "video-sink", pVideoSink, NULL );
                gst_element_set_state( mpPlayer, aOldState );
            }
        }
    }

    return( xRet );
}

// ------------------------------------------------------------------------------
uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
     throw( ::com::sun::star::uno::RuntimeException )
{
    FrameGrabber* pFrameGrabber = NULL;
    const awt::Size aPrefSize( getPreferredPlayerWindowSize() );

    if( ( aPrefSize.Width > 0 ) && ( aPrefSize.Height > 0 ) )
    {
        pFrameGrabber = FrameGrabber::create( mpURI );
    }

    return( pFrameGrabber );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::dispose()
     throw( uno::RuntimeException )
{
    if( mpPlayer )
    {
        stop();
        implQuitThread();
    }

    OSL_ASSERT( NULL == mpPlayer );
}

// ------------------------------------------------------------------------------
void SAL_CALL Player::addEventListener( const uno::Reference< lang::XEventListener >& /*rxListener*/ )
     throw( uno::RuntimeException )
{}

// ------------------------------------------------------------------------------
void SAL_CALL Player::removeEventListener( const uno::Reference< lang::XEventListener >& /*rxListener*/ )
     throw( uno::RuntimeException )
{}

// ------------------------------------------------------------------------------
::rtl::OUString SAL_CALL Player::getImplementationName()
     throw( uno::RuntimeException )
{
    return( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_PLAYER_IMPLEMENTATIONNAME ) ) );
}

// ------------------------------------------------------------------------------
sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
     throw( uno::RuntimeException )
{
    return( ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( AVMEDIA_GSTREAMER_PLAYER_SERVICENAME ) ) );
}

// ------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames()
     throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString > aRet( 1 );
    aRet[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_PLAYER_SERVICENAME ) );

    return( aRet );
}

// ------------------------------------------------------------------------------
void Player::implQuitThread()
{
    if( mpThread )
    {
        // set quit flag to 1 so that the main loop will be quit in idle
        // handler the next time it is called from the thread's main loop
        g_atomic_int_inc( &mnQuit );

        // wait until loop and as such the thread has quit
        g_thread_join( mpThread );
        mpThread = NULL;
    }
}

// ------------------------------------------------------------------------------
bool Player::implInitPlayer()
{
    bool bRet = false;

    if( mpPlayer && (mnInitFail < 3) )
    {
        GstState aState = GST_STATE_NULL;

        if( gst_element_get_state( mpPlayer, &aState, NULL, GST_MAX_TIMEOUT ) == GST_STATE_CHANGE_SUCCESS )
        {
            bRet = ( GST_STATE_PAUSED == aState ) || ( GST_STATE_PLAYING == aState );

            if( !bRet )
            {
                gst_element_set_state( mpPlayer, GST_STATE_PAUSED );
                bRet = ( gst_element_get_state( mpPlayer, &aState, NULL,
                                                GST_MAX_TIMEOUT ) == GST_STATE_CHANGE_SUCCESS ) &&
                    ( GST_STATE_PAUSED == aState );
            }
        }

        if( ! bRet )
            mnInitFail++;
    }

    return( bRet );
}

// ------------------------------------------------------------------------------
gboolean Player::busCallback( GstBus* /*pBus*/,
                              GstMessage* pMsg )
{
    if( pMsg && mpLoop )
    {
        switch( GST_MESSAGE_TYPE( pMsg ) )
        {
            case ( GST_MESSAGE_EOS ):
            {
                if( g_atomic_int_get( &mnLooping ) > 0 )
                {
                    setMediaTime( 0.0 );
                    start();
                }
                else
                {
                    stop();
                }
            }
            break;

            case ( GST_MESSAGE_ERROR ):
            {
                gchar* pDebug;
                GError* pErr;

                gst_message_parse_error( pMsg, &pErr, &pDebug );
                fprintf( stderr, "Error: %s\n", pErr->message );

                g_free( pDebug );
                g_error_free( pErr );
            }
            break;

            default:
            {
                break;
            }
        }
    }

    return( true );
}

// ------------------------------------------------------------------------------
void Player::implHandleNewElementFunc( GstBin* /* pBin */,
                                       GstElement* pElement,
                                       gpointer pData )
{
    if( pElement )
    {
#ifdef DEBUG
        gchar* pElementName = gst_element_get_name( pElement );

        if( pElementName )
        {
            OSL_TRACE( ">>> Bin has element: %s", pElementName );
            g_free( pElementName );
        }
#endif

        if( GST_IS_BIN( pElement ) )
        {
            // set this handler in case we have a GstBin element
            g_signal_connect( GST_BIN( pElement ), "element-added",
                              G_CALLBACK( Player::implHandleNewElementFunc ), pData );
        }

        // watch for all pads that are going to be added to this element;
        g_signal_connect( pElement, "pad-added",
                          G_CALLBACK( Player::implHandleNewPadFunc ), pData );
    }
}

// ------------------------------------------------------------------------------
void Player::implHandleNewPadFunc( GstElement* pElement,
                                   GstPad* pPad,
                                   gpointer pData )
{
    Player* pPlayer = static_cast< Player* >( pData );

    if( pPlayer && pElement && pPad )
    {
#ifdef DEBUG
        gchar* pElementName = gst_element_get_name( pElement );
        gchar* pPadName = gst_pad_get_name( pPad );

        OSL_TRACE( ">>> Element %s has pad: %s", pElementName, pPadName );

        g_free( pPadName );
        g_free( pElementName );
#endif

        GstCaps* pCaps = gst_pad_get_caps( pPad );

        // we are interested only in getting video properties
        // width and height or if we have a video source at all
        if( pCaps )
        {
            for( gint i = 0, nSize = gst_caps_get_size( pCaps ); i < nSize; ++i )
            {
                const GstStructure* pStruct = gst_caps_get_structure( pCaps, i );

                if( pStruct )
                {
                    const gchar* pStructName = gst_structure_get_name( pStruct );

#ifdef DEBUG
                    OSL_TRACE( "\t>>> Pad has structure: %s", pStructName );

                    for( gint n = 0, nFields = gst_structure_n_fields( pStruct ); n < nFields; ++n )
                    {
                        OSL_TRACE( "\t\t>>> Structure has field: %s", gst_structure_nth_field_name( pStruct, n ) );
                    }
#endif

                    // just look for structures having 'video' in their names
                    if( ::std::string( pStructName ).find( "video" ) != ::std::string::npos )
                    {
                        g_atomic_int_inc( &pPlayer->mnIsVideoSource );

                        for( gint n = 0, nFields = gst_structure_n_fields( pStruct ); n < nFields; ++n )
                        {
                            const gchar* pFieldName = gst_structure_nth_field_name( pStruct, n );
                            gint nValue;

                            if( ( ::std::string( pFieldName ).find( "width" ) != ::std::string::npos ) &&
                               gst_structure_get_int( pStruct, pFieldName, &nValue ) )
                            {
                                const gint nDiff = nValue - g_atomic_int_get( &pPlayer->mnVideoWidth );
                                g_atomic_int_add( &pPlayer->mnVideoWidth, ::std::max( nDiff, 0 ) );
                            }
                            else if( ( ::std::string( pFieldName ).find( "height" ) != ::std::string::npos ) &&
                                    gst_structure_get_int( pStruct, pFieldName, &nValue ) )
                            {
                                const gint nDiff = nValue - g_atomic_int_get( &pPlayer->mnVideoHeight );
                                g_atomic_int_add( &pPlayer->mnVideoHeight, ::std::max( nDiff, 0 ) );
                            }
                        }
                    }
                }
            }

            gst_caps_unref( pCaps );
        }
    }
}

// ------------------------------------------------------------------------------
gboolean Player::idle()
{
    // test if main loop should quit and set flag mnQuit to 1
    bool bQuit = g_atomic_int_compare_and_exchange( &mnQuit, 1, 1 );

    if( bQuit )
    {
        // set mnQuit back to 0 to avoid mutiple g_main_loop_quit calls
        // in case Player::idle() is called again later;
        // the flag should have been set only once within Ctor called from
        // the application thread
        g_atomic_int_dec_and_test( &mnQuit );
        g_main_loop_quit( mpLoop );
    }

    // don't eat up all cpu time
    usleep( 1000 );

    return( true );
}

// ------------------------------------------------------------------------------
gpointer Player::run()
{
    static GSourceFuncs aSourceFuncs =
    {
        &lcl_implBusPrepare,
        &lcl_implBusCheck,
        &lcl_implBusDispatch,
        &lcl_implBusFinalize,
        NULL,
        NULL
    };

    if( NULL != ( mpPlayer = gst_element_factory_make( "playbin", NULL ) ) )
    {
        // initialization
        // no mutex necessary since initialization
        // is synchronous until loop is started
        mpContext = g_main_context_new();
        mpLoop = g_main_loop_new( mpContext, false );

        // add idle callback
        GSource* pIdleSource = g_idle_source_new();
        g_source_set_callback( pIdleSource, &lcl_implIdleFunc, this, NULL );
        g_source_attach( pIdleSource, mpContext );

        // add bus callback
        GSource* pBusSource = g_source_new( &aSourceFuncs, sizeof( GstBusSource ) );
        static_cast< GstBusSource* >( pBusSource )->mpBus = gst_pipeline_get_bus( GST_PIPELINE( mpPlayer ) );
        g_source_set_callback( pBusSource, NULL, this, NULL );
        g_source_attach( pBusSource, mpContext );

        // add bus sync handler to intercept video window creation for setting our own window
        gst_bus_set_sync_handler( static_cast< GstBusSource* >( pBusSource )->mpBus,
                                  &lcl_implHandleCreateWindowFunc, this );

        // watch for all elements (and pads) that will be added to the playbin,
        // in order to retrieve properties like video width and height
        g_signal_connect( GST_BIN( mpPlayer ), "element-added",
                          G_CALLBACK( Player::implHandleNewElementFunc ), this );

        // set source URI for player
        g_object_set( mpPlayer, "uri", mpURI->str, NULL );

        // set video fake sink first, since we only create a player without window here
        // and don't want to have the gstreamer default window appearing
        g_object_set( mpPlayer, "video-sink", gst_element_factory_make( "fakesink", NULL ), NULL );

        // set state of player to READY or destroy object in case of FAILURE
        if( gst_element_set_state( mpPlayer, GST_STATE_READY ) == GST_STATE_CHANGE_FAILURE )
        {
            gst_object_unref( mpPlayer );
            mpPlayer = NULL;
        }

        g_atomic_int_add( &mnInitialized, 1 );
        g_cond_signal( mpCond );

        // run the main loop
        g_main_loop_run( mpLoop );

        // clenanup
        // no mutex necessary since other thread joined us (this thread)
        // after setting the quit flag
        if( mpPlayer )
        {
            gst_element_set_state( mpPlayer, GST_STATE_NULL );
            gst_object_unref( mpPlayer );
            mpPlayer = NULL;
        }

        g_main_loop_unref( mpLoop );
        mpLoop = NULL;

        g_source_destroy( pBusSource );
        g_source_unref( pBusSource );

        g_source_destroy( pIdleSource );
        g_source_unref( pIdleSource );

        g_main_context_unref( mpContext );
        mpContext = NULL;
    }
    else
    {
        g_atomic_int_add( &mnInitialized, 1 );
        g_cond_signal( mpCond );
    }

    return( NULL );
}

// ------------------------------------------------------------------------------
GstBusSyncReply Player::handleCreateWindow( GstBus* /* pBus */,
                                            GstMessage* pMsg )
{
    GstBusSyncReply eRet = GST_BUS_PASS;

    if( pMsg &&
       ( GST_MESSAGE_TYPE( pMsg ) == GST_MESSAGE_ELEMENT ) &&
       gst_structure_has_name( pMsg->structure, "prepare-xwindow-id" ) &&
       g_atomic_pointer_get( &mpPlayerWindow ) )
    {
        OSL_TRACE( ">>> Got Request to create XOverlay" );

        gst_x_overlay_set_xwindow_id( GST_X_OVERLAY( GST_MESSAGE_SRC( pMsg ) ),
                                     static_cast< Window* >( g_atomic_pointer_get(
                                                                &mpPlayerWindow ) )->getXWindowHandle() );

        gst_message_unref( pMsg );
        eRet = GST_BUS_DROP;
    }

    return( eRet );
}
}     // namespace gst
} // namespace avmedia
