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

#include <math.h>

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"

using namespace ::com::sun::star;

namespace avmedia { namespace quicktime {

// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr ),
    mpMovie( nil ),
    /* GST
    mbFakeVideo (sal_False ),
    */
    mnUnmutedVolume( 0 ),
    mnStopTime( DBL_MAX ),  //max double
    mbMuted( false ),
    mbLooping( false ),
    mbInitialized( false ),
    mnWindowID( 0 ),
    mnDuration( 0 ),
    mnWidth( 0 ),
    mnHeight( 0 ),
    mnVersion( 0 ),
    maSizeCondition( osl_createCondition() )
{
    OSErr result;

    NSApplicationLoad();
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    // check the version of QuickTime installed
    result = Gestalt(gestaltQuickTime,&mnVersion);
    if ((result == noErr) && (mnVersion >= QT701))
    {
      // we have version 7.01 or later, initialize
      mbInitialized = true;
    }
    [pool release];
}

// ------------------------------------------------------------------------------

Player::~Player()
{
    if( mpMovie )
    {
        [mpMovie release];
        mpMovie = nil;
    }
}
// ------------------------------------------------------------------------------

QTMovie* Player::getMovie()
{
    OSL_ASSERT( mpMovie );
    return mpMovie;
}

// ------------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
    bool    bRet = false;
    // create the Movie
    if( mbInitialized )
    {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        if( mpMovie )
        {
            [mpMovie release];
            mpMovie = nil;
        }

        NSString* aNSStr = [[[NSString alloc] initWithCharacters: rURL.getStr() length: rURL.getLength()]stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding] ;
        NSURL* aURL = [NSURL URLWithString:aNSStr ];

        NSError* pErr = nil;
        mpMovie = [QTMovie movieWithURL:aURL error:&pErr];
        if(mpMovie)
        {
            [mpMovie retain];
            maURL = rURL;
            bRet = true;
        }
        if( pErr )
        {
            OSL_TRACE( "NSMovie create failed with error %ld (%s)",
                       (long)[pErr code],
                       [[pErr localizedDescription] UTF8String]
                       );
        }
        [pool release];
    }

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start(  )
    throw (uno::RuntimeException)
{
  OSL_TRACE ("Player::start");

  if( mpMovie )
  {
      [mpMovie play];
  }
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::stop");
    if( mpMovie )
    {
        [mpMovie stop];
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    bool bRet = false;

    if ( mpMovie )
    {
        if ([mpMovie rate] != 0)
        {
            bRet = true;
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration(  )
    throw (uno::RuntimeException)
{
    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.01;

    if ( mpMovie ) // && mnDuration > 0 ) {
    {
        QTTime structDuration =  [mpMovie duration] ;
        duration = (double)structDuration.timeValue / (double)structDuration.timeScale;
    }

    return duration;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::setMediaTime");

    if ( mpMovie )
    {
        [mpMovie setCurrentTime: QTMakeTimeWithTimeInterval(fTime)];
    }
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime(  )
    throw (uno::RuntimeException)
{
  double position = 0.0;

  if ( mpMovie )
  {
      QTTime structDuration =  [mpMovie currentTime] ;
      position = (double)structDuration.timeValue / (double)structDuration.timeScale;
  }

  if(isPlaying() && position>mnStopTime)
  {
      stop();
  }

  return position;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
    // Quicktime: 0 = stop, 1 = normal speed, 2 = double speed, -1 = normal speed backwards
    double rate = 1.0;

    OSL_TRACE ("Player::getRate");

    if ( mpMovie )
    {
        rate = (double) [mpMovie rate];
    }

    return rate;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::setPlaybackLoop? %s", bSet?"True":"False" );

    if(bSet)
    {
        [mpMovie setAttribute:[NSNumber numberWithBool:YES] forKey: QTMovieLoopsAttribute]  ;
    }
    else
    {
         [mpMovie setAttribute:[NSNumber numberWithBool:NO] forKey: QTMovieLoopsAttribute]  ;
    }
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
    bool bRet = [[mpMovie attributeForKey:QTMovieLoopsAttribute] boolValue];

    OSL_TRACE ("Player::isPlaybackLoop ? %s", bRet?"True":"False" );

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    OSL_TRACE( "set mute: %d muted: %d unmuted volume: %lf", bSet, mbMuted, mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if(  mpMovie && mbMuted != bSet )
    {
        [mpMovie setMuted: bSet ];
        mbMuted = bSet;
    }

}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::isMuted");

    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
    // OOo db volume -40 = QTVolume 0
    // OOo db volume 0   = QTvolume 1
    if(nVolumeDB==-40)
    {
        mnUnmutedVolume = 0;
    }
    else
    {
        mnUnmutedVolume = pow( 10.0, nVolumeDB / 20.0 );
    }

    OSL_TRACE( "set volume: %d gst volume: %f", nVolumeDB, mnUnmutedVolume );

    // change volume
    if( !mbMuted && mpMovie )
    {
        [mpMovie setVolume: mnUnmutedVolume ];
    }
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB(  )
    throw (uno::RuntimeException)
{
    sal_Int16 nVolumeDB = 0.0;

    if( mpMovie )
      {
          float volume = 0.0;

          volume = [mpMovie volume];
          if(volume>0)            //protect from log10(0)
          {
              nVolumeDB = (sal_Int16) ( 20.0*log10 ( volume ) );
          }
          else
          {
              nVolumeDB = -40 ;  // QT zero volume is no volume, -40db
          }
      }

    return nVolumeDB;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
    NSSize  nsSize = [[mpMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
    awt::Size aSize( nsSize.width, nsSize.height );
    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::RuntimeException)
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    OSL_TRACE( "Player::createPlayerWindow %d %d length: %d", aSize.Width, aSize.Height, aArguments.getLength() );

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        sal_IntPtr nPtr = 0;
        aArguments[0] >>= nPtr;
        NSView* pParentView = reinterpret_cast< NSView * >(nPtr);

        ::avmedia::quicktime::Window* pWindow = new ::avmedia::quicktime::Window( mxMgr, *this, pParentView );
        xRet = pWindow;
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
    throw (::com::sun::star::uno::RuntimeException)
{
  uno::Reference< media::XFrameGrabber > xRet;
  OSL_TRACE ("Player::createFrameGrabber");

  if( maURL.getLength() > 0 )
  {
      FrameGrabber* pGrabber = new FrameGrabber( mxMgr );

      xRet = pGrabber;

      if( !pGrabber->create( maURL ) )
      {
          xRet.clear();
      }
  }

  return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_QUICKTIME_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_QUICKTIME_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace quicktime
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
