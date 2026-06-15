// -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "macos.h"

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>

const char *user_name = nullptr;

int coolwsd_server_socket_fd = -1;

COKit *lo_kit;

std::string getBundlePath() {
    static std::string bundlePath;
    if (bundlePath.empty()) {
        NSString *path = [[NSBundle mainBundle] bundlePath];
        bundlePath = std::string([path UTF8String]);
    }
    return bundlePath;
}

std::string getAppSupportURL() {
    @autoreleasepool {
        NSFileManager *fileManager = [NSFileManager defaultManager];

        // Get the URLs for the Application Support directory in the user domain
        NSArray<NSURL *> *urls = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
        NSURL *appSupportURL = [urls firstObject];
        if (!appSupportURL)
            return std::string();

        // Get the app's name from the bundle
        NSString *appName = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"];
        if (!appName) {
            // Fallback if CFBundleName is not set
            appName = @"CODA";
        }

        // Append your app's name to create a unique directory
        NSURL *appDirectoryURL = [appSupportURL URLByAppendingPathComponent:appName isDirectory:YES];

        // Create the directory if it doesn't exist
        if (![fileManager fileExistsAtPath:[appDirectoryURL path]]) {
            NSError *error = nil;
            BOOL success = [fileManager createDirectoryAtURL:appDirectoryURL withIntermediateDirectories:YES attributes:nil error:&error];
            if (!success) {
                NSLog(@"Error creating Application Support directory: %@", error.localizedDescription);
                return std::string();
            }
        }

        // Return the URL as string
        return [[appDirectoryURL absoluteString] UTF8String];
    }
}

std::string getResourceURL(const char *name, const char *ext) {
    NSURL *url = [[NSBundle mainBundle] URLForResource:[NSString stringWithUTF8String:name] withExtension:[NSString stringWithUTF8String:ext]];
    return std::string([[url absoluteString] UTF8String]);
}

std::string getResourcePath(const char *name, const char *ext) {
    NSString *path = [[NSBundle mainBundle] pathForResource:[NSString stringWithUTF8String:name] ofType:[NSString stringWithUTF8String:ext]];
    return std::string([path UTF8String]);
}

void reveal_in_file_manager(const char *uri) {
    if (uri == nullptr || *uri == '\0')
        return;
    @autoreleasepool {
        NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:uri]];
        if (!url)
            return;
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[url]];
    }
}

extern "C" void setUserName(void)
{
    static std::string storage;

    @autoreleasepool {

        // 1. Managed configuration
        NSDictionary *managed = [[NSUserDefaults standardUserDefaults]
            persistentDomainForName:@"com.apple.configuration.managed"];
        if (managed) {
            NSString *val = managed[@"userName"];
            if (val) {
                storage = [val UTF8String];
                user_name = storage.c_str();
                return;
            }
        }

        // 2. Standard user defaults
        NSString *udName = [[NSUserDefaults standardUserDefaults]
                            stringForKey:@"userName"];
        if (udName) {
            storage = [udName UTF8String];
            user_name = storage.c_str();
            return;
        }

        // 3. System full name
        NSString *full = NSFullUserName();
        if (full.length > 0) {
            storage = [full UTF8String];
            user_name = storage.c_str();
            return;
        }

        // 4. System username
        NSString *shortName = NSUserName();
        if (shortName.length > 0) {
            storage = [shortName UTF8String];
            user_name = storage.c_str();
            return;
        }
    }
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
