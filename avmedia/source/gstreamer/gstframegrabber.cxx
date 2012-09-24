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

#include "gstframegrabber.hxx"
#include "gstplayer.hxx"

#include <gst/gstbuffer.h>
#include <gst/video/video.h>
#include <gst/video/gstvideosink.h>

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>

#include <string>

#ifdef AVMEDIA_GST_0_10
#  define AVMEDIA_GST_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_GStreamer_0_10"
#  define AVMEDIA_GST_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_GStreamer_0_10"
#else
#  define AVMEDIA_GST_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_GStreamer"
#  define AVMEDIA_GST_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_GStreamer"
#endif

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {

void FrameGrabber::disposePipeline()
{
    if( mpPipeline != NULL )
    {
        gst_element_set_state( mpPipeline, GST_STATE_NULL );
        g_object_unref( G_OBJECT( mpPipeline ) );
        mpPipeline = NULL;
    }
}

FrameGrabber::FrameGrabber( const rtl::OUString &rURL ) :
    FrameGrabber_BASE()
{
    gchar *pPipelineStr;
    pPipelineStr = g_strdup_printf(
#ifdef AVMEDIA_GST_0_10
        "uridecodebin uri=%s ! ffmpegcolorspace ! videoscale ! appsink "
        "name=sink caps=\"video/x-raw-rgb,format=RGB,pixel-aspect-ratio=1/1,"
        "bpp=(int)24,depth=(int)24,endianness=(int)4321,"
        "red_mask=(int)0xff0000, green_mask=(int)0x00ff00, blue_mask=(int)0x0000ff\"",
#else
        "uridecodebin uri=%s ! videoconvert ! videoscale ! appsink "
        "name=sink caps=\"video/x-raw,format=RGB,pixel-aspect-ratio=1/1\"",
#endif
        rtl::OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 ).getStr() );

    GError *pError = NULL;
    mpPipeline = gst_parse_launch( pPipelineStr, &pError );
    if( pError != NULL) {
        g_warning( "Failed to construct frame-grabber pipeline '%s'\n", pError->message );
        g_error_free( pError );
        disposePipeline();
    }

    if( mpPipeline ) {
        // pre-roll
        switch( gst_element_set_state( mpPipeline, GST_STATE_PAUSED ) ) {
        case GST_STATE_CHANGE_FAILURE:
        case GST_STATE_CHANGE_NO_PREROLL:
            g_warning( "failure pre-rolling media" );
            disposePipeline();
            break;
        default:
            break;
        }
    }
    if( mpPipeline &&
        gst_element_get_state( mpPipeline, NULL, NULL, 5 * GST_SECOND ) == GST_STATE_CHANGE_FAILURE )
        disposePipeline();
}

FrameGrabber::~FrameGrabber()
{
    disposePipeline();
}

FrameGrabber* FrameGrabber::create( const rtl::OUString &rURL )
{
    return new FrameGrabber( rURL );
}

uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
    throw (uno::RuntimeException)
{
    uno::Reference< graphic::XGraphic > xRet;

    if( !mpPipeline )
        return xRet;

    gint64 gst_position = llround( fMediaTime * 1E9 );
    gst_element_seek_simple(
        mpPipeline, GST_FORMAT_TIME,
        (GstSeekFlags)(GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH),
        gst_position );

    GstElement *pSink = gst_bin_get_by_name( GST_BIN( mpPipeline ), "sink" );
    if( !pSink )
        return xRet;

    GstBuffer *pBuf = NULL;
    GstCaps *pCaps = NULL;

    // synchronously fetch the frame
#ifdef AVMEDIA_GST_0_10
    g_signal_emit_by_name( pSink, "pull-preroll", &pBuf, NULL );
    if( pBuf )
        pCaps = GST_BUFFER_CAPS( pBuf );
#else
    GstSample *pSample = NULL;
    g_signal_emit_by_name( pSink, "pull-preroll", &pSample, NULL );

    if( pSample )
    {
        pBuf = gst_sample_get_buffer( pSample );
        pCaps = gst_sample_get_caps( pSample );
    }
#endif

    // get geometry
    int nWidth = 0, nHeight = 0;
    if( !pCaps )
        g_warning( "could not get snapshot format\n" );
    else
    {
        GstStructure *pStruct = gst_caps_get_structure( pCaps, 0 );

        /* we need to get the final caps on the buffer to get the size */
        if( !gst_structure_get_int( pStruct, "width", &nWidth ) ||
            !gst_structure_get_int( pStruct, "height", &nHeight ) )
            nWidth = nHeight = 0;
    }

    if( pBuf && nWidth > 0 && nHeight > 0 &&
        // sanity check the size
#ifdef AVMEDIA_GST_0_10
        GST_BUFFER_SIZE( pBuf ) >= ( nWidth * nHeight * 3 )
#else
        gst_buffer_get_size( pBuf ) >= ( nWidth * nHeight * 3 )
#endif
        )
    {
        sal_uInt8 *pData = NULL;
#ifdef AVMEDIA_GST_0_10
        pData = GST_BUFFER_DATA( pBuf );
#else
        GstMapInfo aMapInfo;
        gst_buffer_map( pBuf, &aMapInfo, GST_MAP_READ );
        pData = aMapInfo.data;
#endif

        int nStride = GST_ROUND_UP_4( nWidth * 3 );
        Bitmap aBmp( Size( nWidth, nHeight ), 24 );

        BitmapWriteAccess *pWrite = aBmp.AcquireWriteAccess();
        if( pWrite )
        {
            // yet another cheesy pixel copying loop
            for( int y = 0; y < nHeight; ++y )
            {
                sal_uInt8 *p = pData + y * nStride;
                for( int x = 0; x < nWidth; ++x )
                {
                    BitmapColor col( p[0], p[1], p[2] );
                    pWrite->SetPixel( y, x, col );
                    p += 3;
                }
            }
        }
        aBmp.ReleaseAccess( pWrite );

#ifndef AVMEDIA_GST_0_10
        gst_buffer_unmap( pBuf, &aMapInfo );
#endif

        xRet = Graphic( aBmp ).GetXGraphic();
    }

    return xRet;
}

::rtl::OUString SAL_CALL FrameGrabber::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( AVMEDIA_GST_FRAMEGRABBER_IMPLEMENTATIONNAME );
}

sal_Bool SAL_CALL FrameGrabber::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_GST_FRAMEGRABBER_SERVICENAME;
}

uno::Sequence< ::rtl::OUString > SAL_CALL FrameGrabber::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = AVMEDIA_GST_FRAMEGRABBER_SERVICENAME;

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
