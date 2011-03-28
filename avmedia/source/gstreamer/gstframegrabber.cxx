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
    Player( pURI  ),
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

    if( pMsg && pMsg->structure )
    {
        GstStructure* pStruct = pMsg->structure;
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
