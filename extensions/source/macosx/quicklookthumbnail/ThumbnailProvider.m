/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import <Quartz/Quartz.h>

#import "ThumbnailProvider.h"
#import "OOoSpotlightAndQuickLookImporter.h"

@implementation ThumbnailProvider

- (void)provideThumbnailForFileRequest:(QLFileThumbnailRequest *)request completionHandler:(void (^)(QLThumbnailReply * _Nullable, NSError * _Nullable))handler {
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSImage *image = nil;
    NSRect drawRect = NSMakeRect(0, 0, floorf(request.maximumSize.width), floorf(request.maximumSize.height));

    if (request.fileURL && [request.fileURL isFileURL] && drawRect.size.width > 0 && drawRect.size.height > 0) {
        OOoSpotlightAndQuickLookImporter *importer = [OOoSpotlightAndQuickLookImporter new];

        @try {
            image = [importer importDocumentThumbnail:[request.fileURL path]];
        }
        @catch (NSException *exception) {
            NSLog(@"provideThumbnailForFileRequest: Caught %@: %@", [exception name], [exception  reason]);
        }

        [importer release];

        if (image) {
            // The handler() function appears to run the drawing block asynchronously
            // so retain the image and release it in the drawing block.
            [image retain];
            NSSize imageSize = [image size];
            CGFloat widthRatio = imageSize.width / request.maximumSize.width;
            CGFloat heightRatio = imageSize.height / request.maximumSize.height;

            // Eliminate an undrawn white edge in thumbnail by rounding down any
            // scaling to the nearest integer.
            if (widthRatio > heightRatio)
                drawRect.size.height = floorf(imageSize.height / widthRatio);
            else
                drawRect.size.width = floorf(imageSize.width / heightRatio);
        }
    }

    handler([QLThumbnailReply replyWithContextSize:drawRect.size currentContextDrawingBlock:^BOOL {
        if (!image)
            return NO;

        [image drawInRect:drawRect];
        [image release];

        return YES;
    }], nil);

    [pool release];
}

@end
