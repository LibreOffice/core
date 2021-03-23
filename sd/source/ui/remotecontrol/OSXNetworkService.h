/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

#include <premac.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/NSNetServices.h>
#import <Foundation/NSRunLoop.h>
#include <postmac.h>

@interface OSXBonjourService : NSObject <NSNetServiceDelegate>
{
    NSNetService* netService;
}

- (void)publishImpressRemoteServiceOnLocalNetworkWithName:(NSString*)sName;

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
