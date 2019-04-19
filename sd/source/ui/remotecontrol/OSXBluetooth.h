/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#import <IOBluetooth/objc/IOBluetoothRFCOMMChannel.h>

#include "IBluetoothSocket.hxx"
#include "Communicator.hxx"
#include "OSXBluetoothWrapper.hxx"

@interface ChannelDelegate : NSObject<IOBluetoothRFCOMMChannelDelegate>
{
    sd::Communicator* pCommunicator;
    sd::OSXBluetoothWrapper* pSocket;
}

- (id) initWithCommunicatorAndSocket:(sd::Communicator*)communicator socket:(sd::OSXBluetoothWrapper*)socket;
- (void) rfcommChannelData:(IOBluetoothRFCOMMChannel*)rfcommChannel data:(void *)dataPointer length:(size_t)dataLength;
- (void) rfcommChannelClosed:(IOBluetoothRFCOMMChannel*)rfcommChannel;

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
