/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*****************************************************************************
 * RemoteControlContainer.m
 * RemoteControlWrapper
 *
 * Created by Martin Kahr on 11.03.06 under a MIT-style license.
 * Copyright (c) 2006 martinkahr.com. All rights reserved.
 *
 * Code modified and adapted to OpenOffice.org
 * by Eric Bachard on 11.08.2008 under the same License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *****************************************************************************/

#import "RemoteControlContainer.h"

@implementation RemoteControlContainer

- (id) initWithDelegate: (id) _remoteControlDelegate {
    if ( (self = [super initWithDelegate:_remoteControlDelegate]) ) {
        remoteControls = [[NSMutableArray alloc] init];
#if OSL_DEBUG_LEVEL >= 2
        NSLog( @"Apple Remote: ControlContainer initWithDelegate ok");
    }
    else {
        NSLog( @"Apple Remote: RemoteControlContainer initWithDelegate failed");
#endif
    }

    return self;
}

- (void) dealloc {
    [self stopListening: self];
    [remoteControls release];
    [super dealloc];
}

- (BOOL) instantiateAndAddRemoteControlDeviceWithClass: (Class) clazz {
    BOOL toReturn = NO;
    RemoteControl* remoteControl = [[clazz alloc] initWithDelegate: delegate];
    if (remoteControl) {
        [remoteControls addObject: remoteControl];
        [remoteControl addObserver: self forKeyPath:@"listeningToRemote" options:NSKeyValueObservingOptionNew context:nil];
        toReturn = YES;
    }
#if OSL_DEBUG_LEVEL >= 2
    else {
        NSLog( @"Apple Remote: ControlContainer instantiateAndAddRemoteControlDeviceWithClass failed");
        toReturn = NO;
    }
#endif
    return toReturn;
}

- (unsigned int) count {
    return [remoteControls count];
}

- (void) reset {
    [self willChangeValueForKey:@"listeningToRemote"];
    [self didChangeValueForKey:@"listeningToRemote"];
#if OSL_DEBUG_LEVEL >= 2
    // debug purpose
    NSLog( @"Apple Remote: reset... (after listening)");
#endif
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
    [self reset];
    (void)keyPath;
    (void)object;
    (void)change;
    (void)context;
}

- (void) setListeningToRemote: (BOOL) value {
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        [[remoteControls objectAtIndex: i] setListeningToRemote: value];
    }
    if (value && value != [self isListeningToRemote]) [self performSelector:@selector(reset) withObject:nil afterDelay:0.01];
}
- (BOOL) isListeningToRemote {
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        if ([[remoteControls objectAtIndex: i] isListeningToRemote]) {
            return YES;
        }
    }
    return NO;
}

- (void) startListening: (id) sender {
#if OSL_DEBUG_LEVEL >= 2
    NSLog(@"Apple Remote: start listening to events... ");
#endif
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        [[remoteControls objectAtIndex: i] startListening: sender];
    }
}
- (void) stopListening: (id) sender {
#if OSL_DEBUG_LEVEL >= 2
    NSLog(@"Apple Remote: stopListening to events... ");
#endif
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        [[remoteControls objectAtIndex: i] stopListening: sender];
    }
}

- (BOOL) isOpenInExclusiveMode {
    BOOL mode = YES;
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        mode = mode && ([[remoteControls objectAtIndex: i] isOpenInExclusiveMode]);
    }
    return mode;
}
- (void) setOpenInExclusiveMode: (BOOL) value {
    for(NSUInteger i=0; i < [remoteControls count]; i++) {
        [[remoteControls objectAtIndex: i] setOpenInExclusiveMode:value];
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
