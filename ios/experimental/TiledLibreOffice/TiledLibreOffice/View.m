// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <LibreOfficeKit/LibreOfficeKit.h>

#import "View.h"
#import "TiledView.h"

@interface View ()

@property UIView *subView;

@end

@implementation View

static LibreOfficeKit* loKit;
static LibreOfficeKitDocument* loDocument;

- (id)initWithFrame:(CGRect)frame kit:(LibreOfficeKit*)kit document:(LibreOfficeKitDocument*)document
{
    loKit = kit;
    loDocument = document;
    self = [super initWithFrame:frame];
    if (self) {
        const int MAXZOOM = 8;

        [self setMaximumZoomScale:MAXZOOM];
        [self setDelegate:self];

        long docWidth, docHeight;
        document->pClass->getDocumentSize(document, &docWidth, &docHeight);

        double widthScale = frame.size.width / docWidth;
        double docAspectRatio = docHeight / docWidth;

        // NSLog(@"View frame=%.0fx%.0f docSize=%.0fx%.0f scale=%.3f aspectRatio=%.3f", frame.size.width, frame.size.height, docWidth, docHeight, widthScale, docAspectRatio);

        self.subView = [[TiledView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.width*docAspectRatio) scale:widthScale maxZoom:MAXZOOM document:loDocument];
        [self addSubview:self.subView];

        UILabel *tpsLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, 20, 300, 40)];
        [tpsLabel setFont:[UIFont systemFontOfSize:38]];
        [tpsLabel setBackgroundColor: [UIColor colorWithRed:0 green:0 blue:0 alpha:0.3]];
        [tpsLabel setTextColor: [UIColor colorWithRed:1 green:1 blue:0 alpha:1]];
        [tpsLabel setTextAlignment: NSTextAlignmentRight];
        [self addSubview:tpsLabel];
        self.tpsLabel = tpsLabel;
    }
    return self;
}

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return self.subView;
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    CGRect frame = ((View*) scrollView).tpsLabel.frame;

    frame.origin.x = 20 + scrollView.contentOffset.x;
    frame.origin.y = 20 + scrollView.contentOffset.y;
    ((View *) scrollView).tpsLabel.frame = frame;
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
