// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
#if ENABLE_DEBUG == 1
        if (argc == 3 && strcmp(argv[1], "-copyTestFile") == 0) {
            // Used in our MobileUITests to copy test files to the right place. While it's unlikely that we'd run into it anywhere else, it doesn't really do harm...
            NSURL * documentDirectory = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
            NSURL * testFileDirectory = [documentDirectory URLByAppendingPathComponent:@"TestFiles" isDirectory:true];
            
            if (![[NSFileManager defaultManager] fileExistsAtPath:[testFileDirectory path]]) {
                [[NSFileManager defaultManager] createDirectoryAtPath:[testFileDirectory path] withIntermediateDirectories:true attributes:nil error:nil];
            }
            
            NSURL * newURL = [testFileDirectory URLByAppendingPathComponent:[NSString stringWithUTF8String:argv[2]]];
            
            if ([[NSFileManager defaultManager] fileExistsAtPath:[newURL path]]) {
                [[NSFileManager defaultManager] removeItemAtURL:newURL error:nil];
            }
            
            [[NSFileManager defaultManager] copyItemAtPath:[[[[[NSBundle bundleForClass:[AppDelegate class]] bundleURL] URLByAppendingPathComponent:@"data"] URLByAppendingPathComponent:[NSString stringWithUTF8String:argv[2]]] path] toPath:[newURL path] error:nil];
            
            // Fall through to just launching the app after we copy the file...
            // ...returning here causes a hang when trying to terminate the app in testing - since as we're never in a state where we "properly" launched
        }
#endif

        // To be able debug the app in Xcode when it is started from another app (like from clicking
        // on an .odt attachment in the Mail app), uncomment these lines. Then when you see the
        // SLEEPING line in Console (on your Mac), attach the process in Xcode.

        // NSLog(@"CollaboraOffice: SLEEPING");
        // sleep(20);

        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
