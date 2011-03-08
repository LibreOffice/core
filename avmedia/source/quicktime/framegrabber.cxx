/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "framegrabber.hxx"
#include "player.hxx"

#include <tools/stream.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <unotools/localfilehelper.hxx>

#define AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME "com.sun.star.comp.avmedia.FrameGrabber_Quicktime"
#define AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME "com.sun.star.media.FrameGrabber_Quicktime"

using namespace ::com::sun::star;

namespace avmedia { namespace quicktime {

// ----------------
// - FrameGrabber -
// ----------------

FrameGrabber::FrameGrabber( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr )
{
    OSErr result;

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    // check the version of QuickTime installed
    result = Gestalt(gestaltQuickTime,&mnVersion);
     if ((result == noErr) && (mnVersion >= QT701))
    {
      // we have version 7.01 or later, initialize
      mpMovie = [QTMovie movie];
      [mpMovie retain];
      mbInitialized = true;
    }
    [pool release];
}

// ------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------

bool FrameGrabber::create( const ::rtl::OUString& rURL )
{
    bool bRet = false;
    maURL = rURL;
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSString* aNSStr = [[[NSString alloc] initWithCharacters: rURL.getStr() length: rURL.getLength()]stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding] ;
    NSURL* aURL = [NSURL URLWithString:aNSStr ];

    // create the Movie

        mpMovie = [mpMovie initWithURL:aURL error:nil];
        if(mpMovie)
        {
            [mpMovie retain];
            bRet = true;
        }

    [pool release];

    return( bRet );
}

// ------------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > SAL_CALL FrameGrabber::grabFrame( double fMediaTime )
    throw (uno::RuntimeException)
{
    uno::Reference< graphic::XGraphic > xRet;

    NSImage* pImage = [mpMovie frameImageAtTime: QTMakeTimeWithTimeInterval(fMediaTime)];
    NSData *pBitmap = [pImage TIFFRepresentation];
    long nSize = [pBitmap length];
    const void* pBitmapData = [pBitmap bytes];
    SvMemoryStream  aMemStm( (char *)pBitmapData, nSize, STREAM_READ | STREAM_WRITE );
    Graphic aGraphic;
    if ( GraphicConverter::Import( aMemStm, aGraphic, CVT_TIF ) == ERRCODE_NONE )
    {
        xRet = aGraphic.GetXGraphic();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL FrameGrabber::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_FRAMEGRABBER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL FrameGrabber::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL FrameGrabber::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_FRAMEGRABBER_SERVICENAME ) );

    return aRet;
}

} // namespace quicktime
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
