// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import <Foundation/Foundation.h>
#import "Client.h"
#import "Server.h"
#import "CommandInterpreter.h"

#define MSG_SLIDESHOW_STARTED @"SLIDESHOW_STARTED"
#define MSG_SLIDE_CHANGED @"SLIDE_CHANGED"
#define MSG_SLIDE_PREVIEW @"SLIDE_PREVIEW"
#define MSG_SLIDE_NOTES @"SLIDE_NOTES"

#define MSG_SERVERLIST_CHANGED @"SERVERLIST_CHANGED"
#define MSG_PAIRING_STARTED @"PAIRING_STARTED"
#define MSG_PAIRING_SUCCESSFUL @"PAIRING_SUCCESSFUL"

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

#define STATUS_PAIRING_PINVALIDATION @"STATUS_PAIRING_PINVALIDATION"

#define STATUS_CONNECTION_FAILED @"STATUS_CONNECTION_FAILED"


typedef enum ConnectionState : NSInteger ConnectionState;

enum ConnectionState : NSInteger {
    DISCONNECTED,
    SEARCHING,
    CONNECTING,
    CONNECTED
};

dispatch_queue_t backgroundQueue;

@interface CommunicationManager : NSObject

@property ConnectionState state;

@end
