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



#include "gstframegrabber.hxx"
#include "gstplayer.hxx"

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>

#include <string>


using namespace ::com::sun::star;

namespace avmedia { namespace gst {

const gulong GRAB_TIMEOUT = 10000000;

// ----------------
// - FrameGrabber -
// ----------------

FrameGrabber::FrameGrabber( GString* pURI ) :
    FrameGrabber_BASE(pURI),
    mpFrameMutex( g_mutex_new() ),
    mpFrameCond( g_cond_new() ),
    mpLastPixbuf( NULL ),
    mbIsInGrabMode( false )
{
}

// ------------------------------------------------------------------------------

FrameGrabber::~FrameGrabber()
{
    if( g_atomic_pointer_get( &mpPlayer ) )
    {
        implQuitThread();
    }

    // thread has ended, so that no more synchronization is necessary
    if( mpLastPixbuf )
    {
        g_object_unref( mpLastPixbuf );
        mpLastPixbuf = NULL;
    }

    g_cond_free( mpFrameCond );
    g_mutex_free( mpFrameMutex );
}

// ------------------------------------------------------------------------------

FrameGrabber* FrameGrabber::create( const GString* pURI )
{
    FrameGrabber* pFrameGrabber = NULL;

    if( pURI && pURI->len )
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
            pFrameGrabber = new FrameGrabber( g_string_new( pURI->str ) );

            // wait until thread signals that it has finished initialization
            if( pFrameGrabber->mpThread )
            {
                g_mutex_lock( pFrameGrabber->mpMutex );

                while( !pFrameGrabber->implIsInitialized() )
                {
                    g_cond_wait( pFrameGrabber->mpCond, pFrameGrabber->mpMutex );
                }

                g_mutex_unlock( pFrameGrabber->mpMutex );
            }

            GstElement* pPixbufSink = gst_element_factory_make( "gdkpixbufsink", NULL );

            // check if player pipeline and GdkPixbufSink could be initialized
            if( !pFrameGrabber->mpPlayer || !pPixbufSink )
            {
                delete pFrameGrabber;
                pFrameGrabber = NULL;
            }
            else
            {
                g_object_set( pFrameGrabber->mpPlayer, "audio-sink", gst_element_factory_make( "fakesink", NULL ), NULL );
                g_object_set( pFrameGrabber->mpPlayer, "video-sink", pPixbufSink, NULL );
            }
        }
    }

    return( pFrameGrabber );
}

// ------------------------------------------------------------------------------

gboolean FrameGrabber::busCallback( GstBus* pBus, GstMessage* pMsg )
{
    bool bDone = false;

    if( pMsg && gst_message_get_structure( pMsg ) )
    {
        const GstStructure* pStruct = gst_message_get_structure( pMsg );
        const gchar* pStructName = gst_structure_get_name( pStruct );

        if( ( ::std::string( pStructName ).find( "pixbuf" ) != ::std::string::npos ) &&
            gst_structure_has_field ( pStruct, "pixbuf") )
        {
            bool bFrameGrabbed = false;

            g_mutex_lock( mpFrameMutex );

            if( mbIsInGrabMode && ( getMediaTime() >= mfGrabTime ) )
            {
                OSL_TRACE( "Grabbing frame at %fs", getMediaTime() );

                if( mpLastPixbuf )
                {
                    g_object_unref( mpLastPixbuf );
                    mpLastPixbuf = NULL;
                }

                mpLastPixbuf = GDK_PIXBUF( g_value_dup_object( gst_structure_get_value( pStruct, "pixbuf" ) ) );
                bFrameGrabbed = true;
            }

            g_mutex_unlock( mpFrameMutex );

            if( bFrameGrabbed )
            {
                g_cond_signal( mpFrameCond );
            }

            bDone = true;
        }
    }

    return( bDone || Player::busCallback( pBus, pMsg ) );
}

// ------------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
    throw (uno::RuntimeException)
{
    uno::Reference< graphic::XGraphic > xRet;

    if( implInitPlayer() )
    {
        OSL_TRACE( "Trying to grab frame at %fs", fMediaTime );

        GTimeVal aTimeoutTime;

        g_get_current_time( &aTimeoutTime );
        g_time_val_add( &aTimeoutTime, GRAB_TIMEOUT );
        setMediaTime( fMediaTime );
        start();

        if( isPlaying() )
        {
            g_mutex_lock( mpFrameMutex );

            mbIsInGrabMode = true;
            mfGrabTime = fMediaTime;
            g_cond_timed_wait( mpFrameCond, mpFrameMutex, &aTimeoutTime );
            mbIsInGrabMode = false;

            g_mutex_unlock( mpFrameMutex );

            stop();
        }

        OSL_ENSURE( g_atomic_pointer_get( &mpLastPixbuf ), "FrameGrabber timed out without receiving a Pixbuf" );

        if( g_atomic_pointer_get( &mpLastPixbuf ) )
        {
            OSL_TRACE( "FrameGrabber received a GdkPixbuf");

            g_mutex_lock( mpFrameMutex );

            const int nWidth = gdk_pixbuf_get_width( mpLastPixbuf );
            const int nHeight = gdk_pixbuf_get_height( mpLastPixbuf );
            const int nChannels = gdk_pixbuf_get_n_channels( mpLastPixbuf );
            const guchar* pBuffer = gdk_pixbuf_get_pixels( mpLastPixbuf );

            if( pBuffer && ( nWidth > 0 ) && ( nHeight > 0 ) )
            {
                Bitmap aFrame( Size( nWidth, nHeight), 24 );
                bool bInit = false;

                if( ( gdk_pixbuf_get_colorspace( mpLastPixbuf ) == GDK_COLORSPACE_RGB ) &&
                    ( nChannels >= 3 ) && ( nChannels <= 4 ) &&
                    ( gdk_pixbuf_get_bits_per_sample( mpLastPixbuf ) == 8 ) )
                {
                    BitmapWriteAccess* pAcc = aFrame.AcquireWriteAccess();

                    if( pAcc )
                    {
                        BitmapColor aPixel( 0, 0, 0 );
                        const int nRowStride = gdk_pixbuf_get_rowstride( mpLastPixbuf );
                        const bool bAlpha = ( nChannels == 4  );

                        for( int nRow = 0; nRow < nHeight; ++nRow )
                        {
                            guchar* pCur = const_cast< guchar* >( pBuffer + nRow * nRowStride );

                            for( int nCol = 0; nCol < nWidth; ++nCol )
                            {
                                aPixel.SetRed( *pCur++ );
                                aPixel.SetGreen( *pCur++ );
                                aPixel.SetBlue( *pCur++ );

                                // ignore alpha channel
                                if( bAlpha )
                                {
                                    ++pCur;
                                }

                                pAcc->SetPixel( nRow, nCol, aPixel );
                            }
                        }

                        aFrame.ReleaseAccess( pAcc );
                        bInit = true;
                    }
                }

                if( !bInit )
                {
                    aFrame.Erase( Color( COL_BLACK ) );
                }

                xRet = Graphic( aFrame ).GetXGraphic();
            }

            g_object_unref( mpLastPixbuf );
            mpLastPixbuf = NULL;

            g_mutex_unlock( mpFrameMutex );
        }
    }

    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL FrameGrabber::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_GSTREAMER_FRAMEGRABBER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL FrameGrabber::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_GSTREAMER_FRAMEGRABBER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_GSTREAMER_FRAMEGRABBER_SERVICENAME ) );

    return aRet;
}

} // namespace win
} // namespace avmedia
