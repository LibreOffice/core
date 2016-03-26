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

#include "framegrabber.hxx"
#include "player.hxx"

#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <unotools/localfilehelper.hxx>

#define AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_Quicktime"
#define AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_Quicktime"

using namespace ::com::sun::star;

SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9

namespace avmedia { namespace quicktime {


FrameGrabber::FrameGrabber( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    mpMovie = [QTMovie movie];
    [mpMovie retain];
    mbInitialized = true;
    [pool release];
}


FrameGrabber::~FrameGrabber()
{
    if( mbInitialized )
    {
        if( mpMovie )
        {
            [mpMovie release];
            mpMovie = nil;
        }
    }
}


bool FrameGrabber::create( const ::rtl::OUString& rURL )
{
    bool bRet = false;
    maURL = rURL;
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSString* aNSStr = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rURL.getStr()) length: rURL.getLength()]stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding] ;
    NSURL* aURL = [NSURL URLWithString:aNSStr ];

    // create the Movie

        mpMovie = [mpMovie initWithURL:aURL error:reinterpret_cast<NSError **>(nil)];
        if(mpMovie)
        {
            [mpMovie retain];
            bRet = true;
        }

    [pool release];

    return bRet;
}


uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
{
    uno::Reference< graphic::XGraphic > xRet;

    NSImage* pImage = [mpMovie frameImageAtTime: QTMakeTimeWithTimeInterval(fMediaTime)];
    NSData *pBitmap = [pImage TIFFRepresentation];
    long nSize = [pBitmap length];
    const void* pBitmapData = [pBitmap bytes];
    SvMemoryStream  aMemStm( const_cast<void *>(pBitmapData), nSize, StreamMode::READ | StreamMode::WRITE );
    Graphic aGraphic;
    if ( GraphicConverter::Import( aMemStm, aGraphic, ConvertDataFormat::TIFF ) == ERRCODE_NONE )
    {
        xRet = aGraphic.GetXGraphic();
    }

    return xRet;
}


::rtl::OUString SAL_CALL FrameGrabber::getImplementationName(  )
{
    return ::rtl::OUString( AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME );
}


sal_Bool SAL_CALL FrameGrabber::supportsService( const ::rtl::OUString& ServiceName )
{
    return ( ServiceName == AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME );
}


uno::Sequence< ::rtl::OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
{
    return { AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME };
}

} // namespace quicktime
} // namespace avmedia

SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
