// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOTestingTile.h"
#import "MLOTestingTileParametersViewController.h"
#import "MLOTestingTileRendererViewController.h"
#import "MLOAppRoleTileTester.h"
#include <touch/touch.h>

@interface MLOTestingTile ()
@property MLOAppRoleTileTester * tester;
@end

@implementation MLOTestingTile


-(id)initWithTester:(MLOAppRoleTileTester *)tester{
    
    self = [self initWithFrame:[MLOTestingTile resized:tester]];
    if(self){
        self.tester =tester;
        self.backgroundColor =[UIColor greenColor];
    }
    return self;

}

+(CGRect)resized:(MLOAppRoleTileTester *)tester{
    return CGRectMake(  10,
                        10,
                        tester.params.contextWidth,
                        tester.params.contextHeight);
}

-(void)resize{
    self.frame = [MLOTestingTile resized:self.tester];
}

- (void)drawRect:(CGRect)rect
{
    NSTimeInterval startTime = CACurrentMediaTime();
    CGContextRef context = UIGraphicsGetCurrentContext();

    MLODpxPoint tilePosition =MLODpxPointByDpxes(self.tester.params.tilePosX,self.tester.params.tilePosY);
    MLODpxSize tileSize = MLODpxSizeByDpxes(self.tester.params.tileWidth,self.tester.params.tileHeight);
    NSInteger contextWidth = self.tester.params.contextWidth;
    NSInteger contextHeight = self.tester.params.contextHeight;

    NSLog(@"touch_lo_draw_tile(contextWidth=%d, contextHeight=%d, tilePosition=%@, tileSize=%@",
          contextWidth,
          contextHeight,
          NSStringFromCGPoint(tilePosition),
          NSStringFromCGSize(tileSize));

    touch_lo_draw_tile(context,
                       contextWidth,
                       contextHeight,
                       tilePosition,
                       tileSize);

    NSLog(@"tile rendering took %f seconds",CACurrentMediaTime() - startTime);
    MLODpxSize size = touch_lo_get_content_size();
    NSLog(@"touch_lo_get_content_size: width=%f, height=%f",size.width, size.height);
}

@end
