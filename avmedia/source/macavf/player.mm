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
#include <rtl/ref.hxx>

#include <cmath> // for log10()

using namespace ::com::sun::star;

@implementation MacAVObserverObject

- (void)observeValueForKeyPath:(NSString*)pKeyPath ofObject:(id)pObject change:(NSDictionary*)pChangeDict context:(void*)pContext
{
    (void) pObject;
    (void) pChangeDict;
    avmedia::macavf::MacAVObserverHandler* pHandler = static_cast<avmedia::macavf::MacAVObserverHandler*>(pContext);
    pHandler->handleObservation( pKeyPath );
}

- (void)onNotification:(NSNotification*)pNotification
{
    NSString* pNoteName = [pNotification name];
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


namespace avmedia::macavf {

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


Player::Player()
:   mpPlayer( nullptr )
,   mfUnmutedVolume( 0 )
,   mfStopTime( DBL_MAX )
,   mbMuted( false )
,   mbLooping( false )
{}


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


bool Player::handleObservation( NSString* pKeyPath )
{
    if( [pKeyPath isEqualToString:AVPlayerItemDidPlayToEndTimeNotification])
    {
        if( mbLooping )
            setMediaTime( 0.0);
    }
    return true;
}


bool Player::create( const OUString& rURL )
{
    // get the media asset
    NSString* aNSStr = [NSString stringWithCharacters:reinterpret_cast<unichar const *>(rURL.getStr()) length:rURL.getLength()];
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


void SAL_CALL Player::start()
{
    if( !mpPlayer )
        return;

    [mpPlayer play];
    // else // TODO: delay until it becomes ready
}


void SAL_CALL Player::stop()
{
    if( !mpPlayer )
        return;
    const bool bPlaying = isPlaying();
    if( bPlaying )
        [mpPlayer pause];
}


sal_Bool SAL_CALL Player::isPlaying()
{
    if( !mpPlayer )
        return false;
    const float fRate = [mpPlayer rate];
    return (fRate != 0.0);
}


double SAL_CALL Player::getDuration()
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


void SAL_CALL Player::setMediaTime( double fTime )
{
    if( mpPlayer )
        [mpPlayer seekToTime: CMTimeMakeWithSeconds(fTime,1000) ];
}


double SAL_CALL Player::getMediaTime()
{
    if( !mpPlayer )
        return 0.0;

    const double position = CMTimeGetSeconds( [mpPlayer currentTime] );
    if( position >= mfStopTime )
        if( isPlaying() )
            stop();

    return position;
}


void Player::setStopTime( double fTime )
{
    mfStopTime = fTime;
}


double Player::getStopTime()
{
    return mfStopTime;
}


void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
{
    mbLooping = bSet;
}


sal_Bool SAL_CALL Player::isPlaybackLoop()
{
    return mbLooping;
}


void SAL_CALL Player::setMute( sal_Bool bSet )
{
    if( !mpPlayer )
        return;

    mbMuted = bSet;
    [mpPlayer setMuted:mbMuted];
}


sal_Bool SAL_CALL Player::isMute()
{
    return mbMuted;
}


void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
{
    // -40dB <-> AVPlayer volume 0.0
    //   0dB <-> AVPlayer volume 1.0
    mfUnmutedVolume = (nVolumeDB <= -40) ? 0.0 : pow( 10.0, nVolumeDB / 20.0 );

    // change volume
    if( !mbMuted && mpPlayer )
        [mpPlayer setVolume:mfUnmutedVolume];
}


sal_Int16 SAL_CALL Player::getVolumeDB()
{
    if( !mpPlayer )
        return 0;

    // get the actual volume
    const float fVolume = [mpPlayer volume];

    // convert into Decibel value
    // -40dB <-> AVPlayer volume 0.0
    //   0dB <-> AVPlayer volume 1.0
    const int nVolumeDB = (fVolume <= 0) ? -40 : lrint( 20.0*log10(fVolume));

    return static_cast<sal_Int16>(nVolumeDB);
}


awt::Size SAL_CALL Player::getPreferredPlayerWindowSize()
{
    awt::Size aSize( 0, 0 ); // default size

    AVAsset* pMovie = [[mpPlayer currentItem] asset];
    NSArray* pVideoTracks = [pMovie tracksWithMediaType:AVMediaTypeVideo];
    if ([pVideoTracks count] > 0)
    {
        AVAssetTrack* pFirstVideoTrack = static_cast<AVAssetTrack*>([pVideoTracks objectAtIndex:0]);
        const CGSize aPrefSize = [pFirstVideoTrack naturalSize];
        aSize = awt::Size( aPrefSize.width, aPrefSize.height );
    }

    return aSize;
}


uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
{
    // get the preferred window size
    const awt::Size aSize( getPreferredPlayerWindowSize() );

    // get the parent view
    sal_IntPtr nNSViewPtr = 0;
    aArguments[0] >>= nNSViewPtr;
    NSView* pParentView = reinterpret_cast<NSView*>(nNSViewPtr);

    // check the window parameters
    if( (aSize.Width <= 0) || (aSize.Height <= 0) || (pParentView == nullptr) )
         return {};

    // create the window
    return new ::avmedia::macavf::Window( *this, pParentView );
}


uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber()
{
    rtl::Reference<FrameGrabber> pGrabber = new FrameGrabber();
    AVAsset* pMovie = [[mpPlayer currentItem] asset];
    if( !pGrabber->create( pMovie ) )
        return {};

    return pGrabber;
}


OUString SAL_CALL Player::getImplementationName(  )
{
    return AVMEDIA_MACAVF_PLAYER_IMPLEMENTATIONNAME;
}


sal_Bool SAL_CALL Player::supportsService( const OUString& ServiceName )
{
    return ServiceName == AVMEDIA_MACAVF_PLAYER_SERVICENAME;
}


uno::Sequence< OUString > SAL_CALL Player::getSupportedServiceNames(  )
{
    return { AVMEDIA_MACAVF_PLAYER_SERVICENAME };
}

} // namespace avmedia::macavf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
