/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <Quartz/Quartz.h>

#import "PreviewViewController.h"
#import "OOoSpotlightAndQuickLookImporter.h"

@implementation PreviewViewController

- (void)preparePreviewOfFileAtURL:(NSURL *)url completionHandler:(void (^)(NSError * _Nullable))handler {
    if (!url || ![url isFileURL]) {
        handler([[NSError alloc] initWithDomain:NSURLErrorDomain code:NSURLErrorUnsupportedURL userInfo:nil]);
        return;
    }

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    OOoSpotlightAndQuickLookImporter *importer = [OOoSpotlightAndQuickLookImporter new];

    NSImage *image = nil;
    @try {
        image = [importer importDocumentThumbnail:[url path]];
    }
    @catch (NSException *exception) {
        NSLog(@"preparePreviewOfFileAtURL: Caught %@: %@", [exception name], [exception  reason]);
    }

    [importer release];

    if (!image) {
        handler([[NSError alloc] initWithDomain:NSURLErrorDomain code:NSURLErrorUnsupportedURL userInfo:nil]);
        [pool release];
        return;
    }

    // Make image automatic scale to the size of the view while preserving the
    // image's aspect ratio by using a CALayer to handle image drawing:
    // https://stackoverflow.com/questions/23002653/nsimageview-image-aspect-fill
    self.view.layer = [CALayer layer];
    self.view.layer.contentsGravity = kCAGravityResizeAspect;
    self.view.layer.contents = image;
    self.view.wantsLayer = YES;

    handler(nil);

    [pool release];
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
