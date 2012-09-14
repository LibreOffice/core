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

#include <objbase.h>
#include <strmif.h>
#include <Amvideo.h>
#include <Qedit.h>
#include <uuids.h>

#include "framegrabber.hxx"
#include "player.hxx"

#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <unotools/localfilehelper.hxx>

#define AVMEDIA_GST_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_GStreamer"
#define AVMEDIA_GST_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_GStreamer"

using namespace ::com::sun::star;

namespace avmedia { namespace gstreamer {

// ----------------
// - FrameGrabber -
// ----------------

FrameGrabber::FrameGrabber( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
    ::CoInitialize( NULL );
}

// ------------------------------------------------------------------------------

FrameGrabber::~FrameGrabber()
{
    ::CoUninitialize();
}

// ------------------------------------------------------------------------------

IMediaDet* FrameGrabber::implCreateMediaDet( const OUString& rURL ) const
{
    IMediaDet* pDet = NULL;

    if( SUCCEEDED( CoCreateInstance( CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, (void**) &pDet ) ) )
    {
        String aLocalStr;

        if( ::utl::LocalFileHelper::ConvertURLToPhysicalName( rURL, aLocalStr ) && aLocalStr.Len() )
        {
            if( !SUCCEEDED( pDet->put_Filename( ::SysAllocString( aLocalStr.GetBuffer() ) ) ) )
            {
                pDet->Release();
                pDet = NULL;
            }
        }
    }

    return pDet;
}

// ------------------------------------------------------------------------------

bool FrameGrabber::create( const OUString& rURL )
{
    // just check if a MediaDet interface can be created with the given URL
    IMediaDet*  pDet = implCreateMediaDet( rURL );

    if( pDet )
    {
        maURL = rURL;
        pDet->Release();
        pDet = NULL;
    }
    else
        maURL = OUString();

    return( maURL.getLength() > 0 );
}

// ------------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
    throw (uno::RuntimeException)
{
    uno::Reference< graphic::XGraphic > xRet;
    IMediaDet*                          pDet = implCreateMediaDet( maURL );

    if( pDet )
    {
        double  fLength;
        long    nStreamCount;
        bool    bFound = false;

        if( SUCCEEDED( pDet->get_OutputStreams( &nStreamCount ) ) )
        {
            for( long n = 0; ( n < nStreamCount ) && !bFound; ++n )
            {
                GUID aMajorType;

                if( SUCCEEDED( pDet->put_CurrentStream( n ) )  &&
                    SUCCEEDED( pDet->get_StreamType( &aMajorType ) ) &&
                    ( aMajorType == MEDIATYPE_Video ) )
                {
                    bFound = true;
                }
            }
        }

        if( bFound &&
            ( S_OK == pDet->get_StreamLength( &fLength ) ) &&
            ( fLength > 0.0 ) && ( fMediaTime >= 0.0 ) && ( fMediaTime <= fLength ) )
        {
            AM_MEDIA_TYPE   aMediaType;
            long            nWidth = 0, nHeight = 0, nSize = 0;

            if( SUCCEEDED( pDet->get_StreamMediaType( &aMediaType ) ) )
            {
                if( ( aMediaType.formattype == FORMAT_VideoInfo ) &&
                    ( aMediaType.cbFormat >= sizeof( VIDEOINFOHEADER ) ) )
                {
                    VIDEOINFOHEADER* pVih = reinterpret_cast< VIDEOINFOHEADER* >( aMediaType.pbFormat );

                    nWidth = pVih->bmiHeader.biWidth;
                    nHeight = pVih->bmiHeader.biHeight;

                    if( nHeight < 0 )
                        nHeight *= -1;
                }

                if( aMediaType.cbFormat != 0 )
                {
                    ::CoTaskMemFree( (PVOID) aMediaType.pbFormat );
                    aMediaType.cbFormat = 0;
                    aMediaType.pbFormat = NULL;
                }

                if( aMediaType.pUnk != NULL )
                {
                    aMediaType.pUnk->Release();
                    aMediaType.pUnk = NULL;
                }
            }

            if( ( nWidth > 0 ) && ( nHeight > 0 ) &&
                SUCCEEDED( pDet->GetBitmapBits( 0, &nSize, NULL, nWidth, nHeight ) ) &&
                ( nSize > 0  ) )
            {
                char* pBuffer = new char[ nSize ];

                try
                {
                    if( SUCCEEDED( pDet->GetBitmapBits( fMediaTime, NULL, pBuffer, nWidth, nHeight ) ) )
                    {
                        SvMemoryStream  aMemStm( pBuffer, nSize, STREAM_READ | STREAM_WRITE );
                        Bitmap          aBmp;

                        if( aBmp.Read( aMemStm, false ) && !aBmp.IsEmpty() )
                        {
                            const Graphic aGraphic( aBmp );
                            xRet = aGraphic.GetXGraphic();
                        }
                    }
                }
                catch( ... )
                {
                }

                delete [] pBuffer;
            }
        }

        pDet->Release();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

OUString SAL_CALL FrameGrabber::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return OUString( AVMEDIA_GST_FRAMEGRABBER_IMPLEMENTATIONNAME );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL FrameGrabber::supportsService( const OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName == AVMEDIA_GST_FRAMEGRABBER_SERVICENAME;
}

// ------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aRet(1);
    aRet[0] = AVMEDIA_GST_FRAMEGRABBER_SERVICENAME ;

    return aRet;
}

} // namespace gstreamer
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
