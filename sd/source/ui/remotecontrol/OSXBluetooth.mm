/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/conditn.hxx> // Include this early to avoid error as check() gets defined by some SDK header to empty

#include <premac.h>
#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>
#include <postmac.h>

#include "OSXBluetooth.h"

@implementation ChannelDelegate

- (id) initWithCommunicatorAndSocket:(sd::Communicator*)communicator socket:(sd::OSXBluetoothWrapper*)socket
{
    pCommunicator = communicator;
    pSocket = socket;
    return self;
}

- (void) rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength
{
    (void) rfcommChannel;

    if ( pSocket )
    {
        pSocket->appendData(dataPointer, dataLength);
    }
}

- (void) rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel 
{
    (void) rfcommChannel;

    // TODO: broadcast premature closing of data channel
    pCommunicator = NULL;
    pSocket = NULL;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
