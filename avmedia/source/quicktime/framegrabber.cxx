/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: framegrabber.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:27:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
