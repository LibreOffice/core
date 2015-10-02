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

#include <math.h>

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"

using namespace ::com::sun::star;

SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9

namespace avmedia { namespace quicktime {

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr ) :
    mxMgr( rxMgr ),
    mpMovie( nil ),
    mnUnmutedVolume( 0 ),
    mnStopTime( DBL_MAX ),  //max double
    mbMuted( false ),
    mbInitialized( false )
{
    NSApplicationLoad();
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    mbInitialized = true;
    [pool release];
}


Player::~Player()
{
    if( mpMovie )
    {
        [mpMovie release];
        mpMovie = nil;
    }
}


QTMovie* Player::getMovie()
{
    OSL_ASSERT( mpMovie );
    return mpMovie;
}


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

        NSString* aNSStr = [[[NSString alloc] initWithCharacters: reinterpret_cast<unichar const *>(rURL.getStr()) length: rURL.getLength()]stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding] ;
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
            SAL_INFO ( "avmedia.quicktime",
                       "NSMovie create failed with error " << (long)[pErr code] <<
                       " (" << [[pErr localizedDescription] UTF8String] << ")"
                       );
        }
        [pool release];
    }

    return bRet;
}


void SAL_CALL Player::start(  )
{
  SAL_INFO ( "avmedia.quicktime", "Player::start" );

  if( mpMovie )
  {
      [mpMovie play];
  }
}


void SAL_CALL Player::stop(  )
{
    SAL_INFO ( "avmedia.quicktime", "Player::stop" );
    if( mpMovie )
    {
        [mpMovie stop];
    }
}


sal_Bool SAL_CALL Player::isPlaying()
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


double SAL_CALL Player::getDuration(  )
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


void SAL_CALL Player::setMediaTime( double fTime )
{
    SAL_INFO ( "avmedia.quicktime", "Player::setMediaTime" );

    if ( mpMovie )
    {
        [mpMovie setCurrentTime: QTMakeTimeWithTimeInterval(fTime)];
    }
}


double SAL_CALL Player::getMediaTime(  )
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

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
    SAL_INFO ( "avmedia.quicktime",
               "Player::setPlaybackLoop ? " << ( bSet?"True":"False" ) );

    if(bSet)
    {
        [mpMovie setAttribute:[NSNumber numberWithBool:YES] forKey: QTMovieLoopsAttribute]  ;
    }
    else
    {
         [mpMovie setAttribute:[NSNumber numberWithBool:NO] forKey: QTMovieLoopsAttribute]  ;
    }
}


sal_Bool SAL_CALL Player::isPlaybackLoop(  )
{
    bool bRet = [[mpMovie attributeForKey:QTMovieLoopsAttribute] boolValue];

    SAL_INFO ( "avmedia.quicktime",
               "Player::isPlaybackLoop ? " << ( bRet?"True":"False" ) );

    return bRet;
}


void SAL_CALL Player::setMute( sal_Bool bSet )
{
    SAL_INFO ( "avmedia.quicktime",
               "set mute: " << bSet <<
               " muted: " << mbMuted <<
               " unmuted volume: " << mnUnmutedVolume );

    // change the volume to 0 or the unmuted volume
    if(  mpMovie && mbMuted != bool(bSet) )
    {
        [mpMovie setMuted: bSet ];
        mbMuted = bSet;
    }

}


sal_Bool SAL_CALL Player::isMute(  )
{
    SAL_INFO ( "avmedia.quicktime", "Player::isMuted" );

    return mbMuted;
}


void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
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

    SAL_INFO ( "avmedia.quicktime",
               "set volume: " << nVolumeDB <<
               " gst volume: " << mnUnmutedVolume );

    // change volume
    if( !mbMuted && mpMovie )
    {
        [mpMovie setVolume: mnUnmutedVolume ];
    }
}


sal_Int16 SAL_CALL Player::getVolumeDB(  )
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


awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
{
    NSSize  nsSize = [[mpMovie attributeForKey:QTMovieNaturalSizeAttribute] sizeValue];
    awt::Size aSize( nsSize.width, nsSize.height );
    return aSize;
}


uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    SAL_INFO ( "avmedia.quicktime",
               "Player::createPlayerWindow " << aSize.Width << " x " << aSize.Height <<
               " length: " << aArguments.getLength() );

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


uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
{
  uno::Reference< media::XFrameGrabber > xRet;
  SAL_INFO ( "avmedia.quicktime", "Player::createFrameGrabber" );

  if( !maURL.isEmpty() )
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


::rtl::OUString SAL_CALL Player::getImplementationName(  )
{
    return ::rtl::OUString( AVMEDIA_QUICKTIME_PLAYER_IMPLEMENTATIONNAME );
}


sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
{
    return ( ServiceName == AVMEDIA_QUICKTIME_PLAYER_SERVICENAME );
}


uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
{
    return { AVMEDIA_QUICKTIME_PLAYER_SERVICENAME };
}

} // namespace quicktime
} // namespace avmedia

SAL_WNODEPRECATED_DECLARATIONS_POP

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
