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

#import "DocumentBrowserViewController.h"
#import "DocumentViewController.h"
#import "SceneDelegate.h"

static UIViewController *bottomPresentedViewController(UIViewController *vc) {
    if ([vc presentedViewController] == nil)
        return vc;
    return bottomPresentedViewController([vc presentedViewController]);
}

static DocumentViewController *newDocumentViewControllerFor(NSURL *url, bool readOnly) {
    UIStoryboard *storyBoard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
    DocumentViewController *documentViewController = [storyBoard instantiateViewControllerWithIdentifier:@"DocumentViewController"];
    documentViewController.document = [[CODocument alloc] initWithFileURL:url];
    documentViewController.document->fakeClientFd = -1;
    documentViewController.document->readOnly = readOnly;
    documentViewController.document.viewController = documentViewController;

    return documentViewController;
 }

@implementation SceneDelegate

// Nothing needed so far, the window property in the .h file is enough.

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
    UIWindowScene *windowScene = (UIWindowScene *) scene;

    if (!windowScene)
        return;

    for (UIOpenURLContext* context in connectionOptions.URLContexts) {
        DocumentViewController *documentViewController = newDocumentViewControllerFor(context.URL, !context.options.openInPlace);
        [windowScene.windows[0].rootViewController presentViewController:documentViewController animated:NO completion:nil];
    }
    if ([connectionOptions.URLContexts count] > 0)
        [windowScene.windows[0] makeKeyAndVisible];
}


- (void)scene:(UIScene *)scene openURLContexts:(NSSet<UIOpenURLContext *> *)URLContexts {
    UIWindowScene *windowScene = (UIWindowScene *) scene;

    if (!windowScene)
        return;

    for (UIOpenURLContext* context in URLContexts) {
        DocumentViewController *documentViewController = newDocumentViewControllerFor(context.URL, !context.options.openInPlace);
        [bottomPresentedViewController(windowScene.windows[0].rootViewController) presentViewController:documentViewController animated:NO completion:nil];
    }
    if ([URLContexts count] > 0)
        [windowScene.windows[0] makeKeyAndVisible];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
