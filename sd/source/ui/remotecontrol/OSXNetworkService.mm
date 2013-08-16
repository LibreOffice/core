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
 #import <CoreFoundation/CoreFoundation.h>
 #import "OSXNetworkService.h"
#include <postmac.h>

@implementation OSXBonjourService

- (void) publishImpressRemoteServiceOnLocalNetworkWithName:(NSString *)sName
{
    netService = [[NSNetService alloc] initWithDomain:@"local" type:"_impressremote._tcp" name:sName port:1599];

  if (netService != nil)
  {
            [netService setDelegate:self];
            [netService scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            [netService publish];
  }
}

-(void)netService:(NSNetService *)aNetService
    didNotPublish:(NSDictionary *)dict {
    NSLog(@"Service %p did not publish: %@", aNetService, dict);
}

- (void)dealloc {
    [netService stop];
    [netService release];
    [super dealloc];
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
