/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <osl/conditn.hxx>

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

    SAL_INFO( "sdremote.bluetooth", "ChannelDelegate::rfcommChannelClosed()\n");

    if ( pSocket )
    {
        pSocket->channelClosed();
    }
    pCommunicator = nullptr;
    pSocket = nullptr;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
