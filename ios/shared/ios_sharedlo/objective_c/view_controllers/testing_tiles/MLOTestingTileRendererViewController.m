// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOTestingTileRendererViewController.h"
#import "MLOAppRoleTileTester.h"
#import "MLOTestingTile.h"

@interface MLOTestingTileRendererViewController ()
@property MLOAppRoleTileTester * tester;
@property UIScrollView * scrollView;
@property MLOTestingTile * tile;
@end

@implementation MLOTestingTileRendererViewController

-(id)initWithTester:(MLOAppRoleTileTester *)tester{
    self = [self init];
    if(self){
        self.tester = tester;
        [self initScrollView];
    }
    NSLog(@"%@ initWithTester",self);
    return self;
}

-(void)initScrollView{
    self.scrollView = [UIScrollView new];
    self.scrollView.backgroundColor = [UIColor grayColor];
    self.view = self.scrollView;
}
-(void)resize{
    NSLog(@"%@ resize",self);
    [self.tile resize];
}
-(NSString *)description{
    return @"MLOAppRoleTileTester";
}
-(void)addToSuperview{
    NSLog(@"%@ addToSuperview",self);

    [self.tester.view addSubview:self.view];
}
-(void)render{

    [self.tile removeFromSuperview];
    self.tile = [[MLOTestingTile alloc] initWithTester:self.tester];
    [self.scrollView addSubview:self.tile];
    self.scrollView.contentSize = self.tile.frame.size;
    
}
@end
