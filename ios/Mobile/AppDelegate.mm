// -*- Mode: objc; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*-
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "config.h"

#import <cassert>
#import <cstdlib>
#import <cstring>

#define LIBO_INTERNAL_ONLY
#import <COKit/COKit.hxx>

#include <comphelper/kit.hxx>
#include <i18nlangtag/languagetag.hxx>

#import "ios.h"
#import "AppDelegate.h"
#import "DocumentBrowserViewController.h"
#import "CODocument.h"
#import "DocumentViewController.h"

#import "FakeSocket.hpp"
#import "Kit.hpp"
#import "Log.hpp"
#import "ProcUtil.hpp"
#import "COOLWSD.hpp"
#import "SetupKitEnvironment.hpp"
#import "Util.hpp"

#import <common/LangUtil.hpp>

NSString *app_locale;
NSString *app_text_direction;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    auto trace = std::getenv("COOL_LOGLEVEL");
    if (!trace)
        trace = strdup("warning");

    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad)
        setupKitEnvironment("notebookbar");
    else
        setupKitEnvironment("");

    Log::initialize("Mobile", trace);
    ProcUtil::setThreadName("main");

    // Clear the cache directory if it is for another build of the app
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *userDirectory = [paths objectAtIndex:0];
    NSString *cacheDirectory = [userDirectory stringByAppendingPathComponent:@"cache"];

    NSString *coreVersionHashFile = [cacheDirectory stringByAppendingPathComponent:@"core_version_hash"];
    NSString *coolwsdVersionHashFile = [cacheDirectory stringByAppendingPathComponent:@"coolwsd_version_hash"];

    NSData *oldCoreVersionHash = [NSData dataWithContentsOfFile:coreVersionHashFile];
    NSData *oldCoolwsdVersionHash = [NSData dataWithContentsOfFile:coolwsdVersionHashFile];

    NSData *coreVersionHash = [NSData dataWithBytes:CORE_VERSION_HASH length:strlen(CORE_VERSION_HASH)];
    NSData *coolwsdVersionHash = [NSData dataWithBytes:COOLWSD_VERSION_HASH length:strlen(COOLWSD_VERSION_HASH)];

    if (oldCoreVersionHash == nil
        || ![oldCoreVersionHash isEqualToData:coreVersionHash]
        || oldCoolwsdVersionHash == nil
        || ![oldCoolwsdVersionHash isEqualToData:coolwsdVersionHash]) {

        [[NSFileManager defaultManager] removeItemAtPath:cacheDirectory error:nil];

        if (![[NSFileManager defaultManager] createDirectoryAtPath:cacheDirectory withIntermediateDirectories:NO attributes:nil error:nil])
            NSLog(@"Could not create %@", cacheDirectory);

        if (![[NSFileManager defaultManager] createFileAtPath:coreVersionHashFile contents:coreVersionHash attributes:nil])
            NSLog(@"Could not create %@", coreVersionHashFile);

        if (![[NSFileManager defaultManager] createFileAtPath:coolwsdVersionHashFile contents:coolwsdVersionHash attributes:nil])
            NSLog(@"Could not create %@", coolwsdVersionHashFile);
    }

    // Having LANG in the environment is expected to happen only when debugging from Xcode. When
    // testing some language one doesn't know it might be risky to simply set one's iPad to that
    // language, as it might be hard to find the way to set it back to a known language.

    char *lang = std::getenv("LANG");
    // Fix assert failure when running "My Mac (Designed for iPad)" in Xcode
    // LANG values such as en_US.UTF-8 trigger an assert in the LibreOffice
    // code so replace all "_" characters with "-" characters.
    if (lang != nullptr) {
        app_locale = [[NSString stringWithUTF8String:lang] stringByReplacingOccurrencesOfString:@"_" withString:@"-"];
        // Eliminate invalid language tag exceptions in JavaScript by
        // trimming any text encoding from LANG
        NSRange range = [app_locale rangeOfString:@"."];
        if (range.location != NSNotFound)
            app_locale = [app_locale substringToIndex:range.location];
    }
    if (!app_locale || ![app_locale length])
        app_locale = [[NSLocale preferredLanguages] firstObject];

    if (LangUtil::isRtlLanguage(std::string([app_locale UTF8String])))
        app_text_direction = @"rtl";
    else
        app_text_direction = @"";

    lo_kit = cok_init_2(nullptr, nullptr);

    comphelper::COKit::setLanguageTag(LanguageTag(OUString::fromUtf8(OString([app_locale UTF8String])), true));

    // This fires off a thread running the LOKit runLoop()
    runKitLoopInAThread();

    // Look for the setting indicating the URL for a file containing a list of URLs for template
    // documents to download. If set, start a task to download it, and then to download the listed
    // templates.

    // First check managed configuration, if present
    NSDictionary *managedConfig = [[NSUserDefaults standardUserDefaults] dictionaryForKey:@"com.apple.configuration.managed"];

    // Look for managed configuration setting of the user name.

    if (managedConfig != nil) {
        NSString *userName = managedConfig[@"userName"];
        if (userName != nil && [userName isKindOfClass:[NSString class]])
            user_name = [userName UTF8String];
    }

    if (user_name == nullptr)
        user_name = [[[NSUserDefaults standardUserDefaults] stringForKey:@"userName"] UTF8String];

    // Remove any leftover allegedly temporary folders with copies of documents left behind from
    // previous instances of the app that were killed while editing, various random files that for
    // instance NSS seems to love to create, etc, by removing the whole tmp folder.
    NSURL *tempFolderURL = [[NSFileManager defaultManager] temporaryDirectory];
    if (![[NSFileManager defaultManager] removeItemAtURL:tempFolderURL error:nil]) {
        NSLog(@"Could not remove tmp folder %@", tempFolderURL);
    }

    if (![[NSFileManager defaultManager] createDirectoryAtURL:tempFolderURL withIntermediateDirectories:YES attributes:nil error:nil]) {
        NSLog(@"Could not create tmp folder %@", tempFolderURL);
    }

    fakeSocketSetLoggingCallback([](const std::string& line)
                                 {
                                     LOG_INF_NOFILE(line);
                                 });

    dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                   ^{
                       char *argv[2];
                       argv[0] = strdup([[NSBundle mainBundle].executablePath UTF8String]);
                       argv[1] = nullptr;
                       ProcUtil::setThreadName("app");
                       auto coolwsd = new COOLWSD();
                       coolwsd->run(1, argv);

                       // Should never return
                       assert(false);
                       NSLog(@"lolwsd->run() unexpectedly returned");
                       std::abort();
                   });
    return YES;
}

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options API_AVAILABLE(ios(13.0)) {
    return [UISceneConfiguration configurationWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}

- (void)applicationWillResignActive:(UIApplication *)application {
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
}

- (void)applicationWillTerminate:(UIApplication *)application {
    // tdf#126974 We don't want any global object destructors to be called, the code
    // is not prepared for that.
    std::_Exit(1);
}

// This method is called when you use the "Share > Open in Collabora Office" functionality in the
// Files app. Possibly also in other use cases.
- (BOOL)application:(UIApplication *)app openURL:(NSURL *)inputURL options:(NSDictionary<UIApplicationOpenURLOptionsKey, id> *)options {
    // Ensure the URL is a file URL
    if (!inputURL.isFileURL) {
        return NO;
    }

    // Reveal / import the document at the URL
    DocumentBrowserViewController *documentBrowserViewController = (DocumentBrowserViewController *)self.window.rootViewController;
    [documentBrowserViewController revealDocumentAtURL:inputURL importIfNeeded:YES completion:^(NSURL * _Nullable revealedDocumentURL, NSError * _Nullable error) {
        if (error) {
            LOG_ERR("Failed to reveal the document at URL " << [[inputURL description] UTF8String] << " with error: " << [[error description] UTF8String]);
            return;
        }

        // Present the Document View Controller for the revealed URL
        [documentBrowserViewController presentDocumentAtURL:revealedDocumentURL];
    }];
    return YES;
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
