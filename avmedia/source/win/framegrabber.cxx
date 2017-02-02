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

#include <sal/config.h>

#include <memory>

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif
#include <prewin.h>
#include <postwin.h>
#include <objbase.h>
#include <strmif.h>
#include <Amvideo.h>
#include "interface.hxx"
#include <uuids.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "framegrabber.hxx"
#include "player.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/dibtools.hxx>

#define AVMEDIA_WIN_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_DirectX"
#define AVMEDIA_WIN_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_DirectX"

using namespace ::com::sun::star;

namespace avmedia { namespace win {


FrameGrabber::FrameGrabber( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
    ::CoInitialize( nullptr );
}


FrameGrabber::~FrameGrabber()
{
    ::CoUninitialize();
}

namespace {

IMediaDet* implCreateMediaDet( const OUString& rURL )
{
    IMediaDet* pDet = nullptr;

    if( SUCCEEDED( CoCreateInstance( CLSID_MediaDet, nullptr, CLSCTX_INPROC_SERVER, IID_IMediaDet, reinterpret_cast<void**>(&pDet) ) ) )
    {
        OUString aLocalStr;

        if( osl::FileBase::getSystemPathFromFileURL( rURL, aLocalStr )
            == osl::FileBase::E_None )
        {
            if( !SUCCEEDED( pDet->put_Filename( ::SysAllocString( reinterpret_cast<LPCOLESTR>(aLocalStr.getStr()) ) ) ) )
            {
                pDet->Release();
                pDet = nullptr;
            }
        }
    }

    return pDet;
}

}

bool FrameGrabber::create( const OUString& rURL )
{
    // just check if a MediaDet interface can be created with the given URL
    IMediaDet*  pDet = implCreateMediaDet( rURL );

    if( pDet )
    {
        maURL = rURL;
        pDet->Release();
        pDet = nullptr;
    }
    else
        maURL.clear();

    return !maURL.isEmpty();
}


uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
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
                    ::CoTaskMemFree( aMediaType.pbFormat );
                    aMediaType.cbFormat = 0;
                    aMediaType.pbFormat = nullptr;
                }

                if( aMediaType.pUnk != nullptr )
                {
                    aMediaType.pUnk->Release();
                    aMediaType.pUnk = nullptr;
                }
            }

            if( ( nWidth > 0 ) && ( nHeight > 0 ) &&
                SUCCEEDED( pDet->GetBitmapBits( 0, &nSize, nullptr, nWidth, nHeight ) ) &&
                ( nSize > 0  ) )
            {
                auto pBuffer = std::unique_ptr<char[]>(new char[ nSize ]);

                try
                {
                    if( SUCCEEDED( pDet->GetBitmapBits( fMediaTime, nullptr, pBuffer.get(), nWidth, nHeight ) ) )
                    {
                        SvMemoryStream  aMemStm( pBuffer.get(), nSize, StreamMode::READ | StreamMode::WRITE );
                        Bitmap          aBmp;

                        if( ReadDIB(aBmp, aMemStm, false ) && !aBmp.IsEmpty() )
                        {
                            const Graphic aGraphic( aBmp );
                            xRet = aGraphic.GetXGraphic();
                        }
                    }
                }
                catch( ... )
                {
                }
            }
        }

        pDet->Release();
    }

    return xRet;
}


OUString SAL_CALL FrameGrabber::getImplementationName(  )
{
    return OUString( AVMEDIA_WIN_FRAMEGRABBER_IMPLEMENTATIONNAME );
}


sal_Bool SAL_CALL FrameGrabber::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
{
    return { AVMEDIA_WIN_FRAMEGRABBER_SERVICENAME };
}

} // namespace win
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
