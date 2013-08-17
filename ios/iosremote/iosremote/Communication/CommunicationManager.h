// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import <Foundation/Foundation.h>

@class Client;
@class Server;
@class CommandInterpreter;
@class CommandTransmitter;
@class SlideShow;

#define MSG_SLIDESHOW_STARTED @"SLIDESHOW_STARTED"
#define MSG_SLIDE_CHANGED @"SLIDE_CHANGED"
#define MSG_SLIDE_PREVIEW @"SLIDE_PREVIEW"
#define MSG_SLIDE_NOTES @"SLIDE_NOTES"

#define MSG_SERVERLIST_CHANGED @"SERVERLIST_CHANGED"
#define MSG_PAIRING_STARTED @"PAIRING_STARTED"
#define MSG_PAIRING_SUCCESSFUL @"PAIRING_SUCCESSFUL"

#define SLIDESHOW_INFO_RECEIVED @"SLIDE_INFO_RECEIVED"

/**
 * Notify the UI that the service has connected to a server AND a slideshow
 * is running.
 * In this case the PresentationActivity should be started.
 */
#define STATUS_CONNECTED_SLIDESHOW_RUNNING @"STATUS_CONNECTED_SLIDESHOW_RUNNING"
/**
 * Notify the UI that the service has connected to a server AND no slideshow
 * is running.
 * In this case the StartPresentationActivity should be started.
 */
#define STATUS_CONNECTED_NOSLIDESHOW @"STATUS_CONNECTED_NOSLIDESHOW"

/**
 * Asking for a pairing interface
 */
#define STATUS_PAIRING_PINVALIDATION @"LO_SERVER_VALIDATING_PIN"


/*
 * Paired, should bring users to the preview page
 */
#define STATUS_PAIRING_PAIRED @"LO_SERVER_SERVER_PAIRED"

#define STATUS_CONNECTION_FAILED @"STATUS_CONNECTION_FAILED"


typedef enum ConnectionState : NSInteger ConnectionState;
typedef enum SearchState : NSInteger SearchState;

enum ConnectionState : NSInteger {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

enum SearchState : NSInteger {
    WAITING,
    SEARCHING
};

typedef enum {
    NotReachable = 0,
    ReachableViaWiFi,
    ReachableViaWWAN
} NetworkStatus;

@interface CommunicationManager : NSObject

- (id) initWithExistingServers;
- (NSNumber *) getPairingPin;
- (void) connectToServer:(Server*)server;
- (void) addServersWithName:(NSString*)name
                  AtAddress:(NSString*)addr;
- (void) removeServerAtIndex:(NSUInteger)index;

- (NetworkStatus) currentReachabilityStatus;
+ (id)fetchSSIDInfo;

@property ConnectionState state;
@property SearchState searchState;
@property (nonatomic, strong) id delegate;
@property (atomic, strong) NSMutableArray* servers;
@property (atomic, strong) NSMutableArray* autoDiscoveryServers;
@property (nonatomic, strong) Client* client;
@property (nonatomic, strong) CommandInterpreter* interpreter;
@property (nonatomic, strong) CommandTransmitter* transmitter;

+ (CommunicationManager *)sharedComManager;

@end
