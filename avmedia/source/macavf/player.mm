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

#include "player.hxx"
#include "framegrabber.hxx"
#include "window.hxx"

#include <cmath> // for log10()

using namespace ::com::sun::star;

@implementation MacAVObserverObject

- (void)observeValueForKeyPath:(NSString*)pKeyPath ofObject:(id)pObject change:(NSDictionary*)pChangeDict context:(void*)pContext
{
    (void) pObject;
    NSString* pDictStr = [NSString stringWithFormat:@"%@", pChangeDict];
    OSL_TRACE( "MacAVObserver::onKeyChange k=\"%s\" c=%s", [pKeyPath UTF8String], [pDictStr UTF8String]);
    avmedia::macavf::MacAVObserverHandler* pHandler = static_cast<avmedia::macavf::MacAVObserverHandler*>(pContext);
    pHandler->handleObservation( pKeyPath );
}

- (void)onNotification:(NSNotification*)pNotification
{
    NSString* pNoteName = (NSString*)[pNotification name];
    OSL_TRACE( "MacAVObserver::onNotification key=\"%s\"", [pNoteName UTF8String]);
    HandlersForObject::iterator it = maHandlersForObject.find( [pNotification object]);
    if( it != maHandlersForObject.end() )
        (*it).second->handleObservation( pNoteName );
}

- (void)setHandlerForObject:(NSObject*)pObject handler:(avmedia::macavf::MacAVObserverHandler*)pHandler
{
    maHandlersForObject[ pObject] = pHandler;
}

- (void)removeHandlerForObject:(NSObject*)pObject
{
    maHandlersForObject.erase( pObject);
}

@end


namespace avmedia { namespace macavf {

MacAVObserverObject* MacAVObserverHandler::mpMacAVObserverObject = nullptr;

MacAVObserverObject* MacAVObserverHandler::getObserver()
{
    if( !mpMacAVObserverObject)
    {
        mpMacAVObserverObject = [MacAVObserverObject alloc];
        [mpMacAVObserverObject retain];
    }
    return mpMacAVObserverObject;
}


// ----------------
// - Player -
// ----------------

Player::Player( const uno::Reference< lang::XMultiServiceFactory >& rxMgr )
:   mxMgr( rxMgr )
,   mpPlayer( nullptr )
,   mfUnmutedVolume( 0 )
,   mfStopTime( DBL_MAX )
,   mbMuted( false )
,   mbLooping( false )
{}

// ------------------------------------------------------------------------------

Player::~Player()
{
    if( !mpPlayer )
        return;
    // remove the observers
    [mpPlayer removeObserver:getObserver() forKeyPath:@"currentItem.status"];
    AVPlayerItem* pOldPlayerItem = [mpPlayer currentItem];
    [[NSNotificationCenter defaultCenter] removeObserver:getObserver()
        name:AVPlayerItemDidPlayToEndTimeNotification
        object:pOldPlayerItem];
    [getObserver() removeHandlerForObject:pOldPlayerItem];
    // release the AVPlayer
    CFRelease( mpPlayer );
}

// ------------------------------------------------------------------------------

bool Player::handleObservation( NSString* pKeyPath )
{
    OSL_TRACE( "AVPlayer::handleObservation key=\"%s\"", [pKeyPath UTF8String]);
    if( [pKeyPath isEqualToString:AVPlayerItemDidPlayToEndTimeNotification])
    {
        OSL_TRACE( "AVPlayer replay=%d", mbLooping);
        if( mbLooping )
            setMediaTime( 0.0);
    }
    return true;
}

// ------------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
    // get the media asset
    NSString* aNSStr = [NSString stringWithCharacters:rURL.getStr() length:rURL.getLength()];
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
        //TODO: 10.11 stringByAddingPercentEscapesUsingEncoding
    NSURL* aNSURL = [NSURL URLWithString: [aNSStr stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    SAL_WNODEPRECATED_DECLARATIONS_POP
    // get the matching AVPlayerItem
    AVPlayerItem* pPlayerItem = [AVPlayerItem playerItemWithURL:aNSURL];

    // create or update the AVPlayer with the new AVPlayerItem
    if( !mpPlayer )
    {
        mpPlayer = [AVPlayer playerWithPlayerItem:pPlayerItem];
        CFRetain( mpPlayer );
        [mpPlayer setActionAtItemEnd:AVPlayerActionAtItemEndNone];
    }
    else
    {
        // remove the obsoleted observers
        AVPlayerItem* pOldPlayerItem = [mpPlayer currentItem];
        [mpPlayer removeObserver:getObserver() forKeyPath:@"currentItem.status"];
        [getObserver() removeHandlerForObject:pOldPlayerItem];
        [[NSNotificationCenter defaultCenter] removeObserver:getObserver()
            name:AVPlayerItemDidPlayToEndTimeNotification
            object:pOldPlayerItem];
        // replace the playeritem
        [mpPlayer replaceCurrentItemWithPlayerItem:pPlayerItem];
    }

    // observe the status of the current player item
    [mpPlayer addObserver:getObserver() forKeyPath:@"currentItem.status" options:0 context:this];

    // observe playback-end needed for playback looping
    [[NSNotificationCenter defaultCenter] addObserver:getObserver()
        selector:@selector(onNotification:)
        name:AVPlayerItemDidPlayToEndTimeNotification
        object:pPlayerItem];
    [getObserver() setHandlerForObject:pPlayerItem handler:this];

    return true;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start()
    throw (uno::RuntimeException)
{
    if( !mpPlayer )
        return;
#if 0
    const AVPlayerStatus eStatus = [mpPlayer status];
    OSL_TRACE ("Player::start status=%d", (int)eStatus);
    if( eStatus == AVPlayerStatusReadyToPlay)
#endif
    {
        [mpPlayer play];
    }
    // else // TODO: delay until it becomes ready
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop()
    throw (uno::RuntimeException)
{
    if( !mpPlayer )
        return;
    const bool bPlaying = isPlaying();
    OSL_TRACE ("Player::stop() playing=%d", bPlaying);
    if( bPlaying )
        [mpPlayer pause];
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    if( !mpPlayer )
        return false;
    const float fRate = [mpPlayer rate];
    return (fRate != 0.0);
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration()
    throw (uno::RuntimeException)
{
    // slideshow checks for non-zero duration, so cheat here
    double duration = 0.01;

    if( mpPlayer )
    {
        AVPlayerItem* pItem = [mpPlayer currentItem];
        if( [pItem status] == AVPlayerItemStatusReadyToPlay )
            duration = CMTimeGetSeconds( [pItem duration] );
        else // fall back to AVAsset's best guess
            duration = CMTimeGetSeconds( [[pItem asset] duration] );
    }

    return duration;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::setMediaTime( %.3fsec)", fTime);
    if( mpPlayer )
        [mpPlayer seekToTime: CMTimeMakeWithSeconds(fTime,1000) ];
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime()
    throw (uno::RuntimeException)
{
    if( !mpPlayer )
        return 0.0;

    const double position = CMTimeGetSeconds( [mpPlayer currentTime] );
    OSL_TRACE( "Player::getMediaTime() = %.3fsec", position);
    if( position >= mfStopTime )
        if( isPlaying() )
            stop();

    return position;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::setStopTime( %.3fsec)", fTime);
    mfStopTime = fTime;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getStopTime()
    throw (uno::RuntimeException)
{
    return mfStopTime;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::setPlaybackLoop( %d)", bSet );
    mbLooping = bSet;
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop()
    throw (uno::RuntimeException)
{
    const bool bRet = mbLooping;
    OSL_TRACE ("Player::isPlaybackLoop() = %d", bRet );
    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
    OSL_TRACE( "Player::setMute(%d), was-muted: %d unmuted-volume: %.3f", bSet, mbMuted, mfUnmutedVolume );

    if( !mpPlayer )
        return;

    mbMuted = bSet;
    [mpPlayer setMuted:mbMuted];
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute()
    throw (uno::RuntimeException)
{
    OSL_TRACE ("Player::isMuted() = %d", mbMuted);
    return mbMuted;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
    // -40dB <-> AVPlayer volume 0.0
    //   0dB <-> AVPlayer volume 1.0
    mfUnmutedVolume = (nVolumeDB <= -40) ? 0.0 : pow( 10.0, nVolumeDB / 20.0 );
    OSL_TRACE( "Player::setVolume(%ddB), muted=%d, unmuted-volume: %.3f", nVolumeDB, mbMuted, mfUnmutedVolume );

    // change volume
    if( !mbMuted && mpPlayer )
        [mpPlayer setVolume:mfUnmutedVolume];
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB()
    throw (uno::RuntimeException)
{
    if( !mpPlayer )
        return 0;

    // get the actual volume
    const float fVolume = [mpPlayer volume];

    // convert into Dezibel value
    // -40dB <-> AVPlayer volume 0.0
    //   0dB <-> AVPlayer volume 1.0
    const int nVolumeDB = (fVolume <= 0) ? -40 : lrint( 20.0*log10(fVolume));

    return (sal_Int16)nVolumeDB;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
    throw (uno::RuntimeException)
{
    awt::Size aSize( 0, 0 ); // default size

    AVAsset* pMovie = [[mpPlayer currentItem] asset];
    NSArray* pVideoTracks = [pMovie tracksWithMediaType:AVMediaTypeVideo];
    if ([pVideoTracks count] > 0)
    {
        AVAssetTrack* pFirstVideoTrack = (AVAssetTrack*) [pVideoTracks objectAtIndex:0];
        const CGSize aPrefSize = [pFirstVideoTrack naturalSize];
        aSize = awt::Size( aPrefSize.width, aPrefSize.height );
    }

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::RuntimeException)
{
    // get the preferred window size
    const awt::Size aSize( getPreferredPlayerWindowSize() );
    OSL_TRACE( "Player::createPlayerWindow %dx%d argsLength: %d", aSize.Width, aSize.Height, aArguments.getLength() );

    // get the parent view
    sal_IntPtr nNSViewPtr = 0;
    aArguments[0] >>= nNSViewPtr;
    NSView* pParentView = reinterpret_cast<NSView*>(nNSViewPtr);

    // check the window parameters
    uno::Reference< ::media::XPlayerWindow > xRet;
    if( (aSize.Width <= 0) || (aSize.Height <= 0) || (pParentView == nullptr) )
         return xRet;

    // create the window
    ::avmedia::macavf::Window* pWindow = new ::avmedia::macavf::Window( mxMgr, *this, pParentView );
    xRet = pWindow;
    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
    throw (uno::RuntimeException)
{
    uno::Reference< media::XFrameGrabber > xRet;
    OSL_TRACE ("Player::createFrameGrabber");

    FrameGrabber* pGrabber = new FrameGrabber( mxMgr );
    AVAsset* pMovie = [[mpPlayer currentItem] asset];
    if( pGrabber->create( pMovie ) )
        xRet = pGrabber;

    return xRet;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_MACAVF_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_MACAVF_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet { AVMEDIA_MACAVF_PLAYER_SERVICENAME };

    return aRet;
}

} // namespace macavf
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
