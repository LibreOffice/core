// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOTestingTileParametersViewController.h"
#import "MLOTestingTileParameter.h"
#import "MLOAppRoleTileTester.h"
#import "MLOTestingTileRendererViewController.h"

static const CGFloat RENDER_BUTTON_HEIGHT = 50.0f;

@interface MLOTestingTileParametersViewController ()
@property MLOAppRoleTileTester * tester;
@property NSArray * params;
@property UIButton * renderButton;
@end
@implementation MLOTestingTileParametersViewController

-(id)initWithTester:(MLOAppRoleTileTester *)tester{
    self = [self init];
    if(self){
        self.tester = tester;
        [self initParams];
        [self initRenderButton];

    }

    NSLog(@"%@ initWithTester",self);
    return self;
}

-(NSString *)description{
    return @"MLOTestingTileParametersViewController";
}

-(void)initParams{
    self.params = @[[self createParam:@"contextWidth"
                           extractor1:^(CGFloat value){self.contextWidth = value;}
                           extractor2:^(CGFloat value){self.contextWidth = self.contextHeight = value;}
                                value:CONTEXT_WIDTH_DEFAULT],

                    [self createConditionalParam:@"contextHeight"
                                       extractor:^(CGFloat value){self.contextHeight = value;}
                                           value:CONTEXT_HEIGHT_DEFAULT],

                    [self createNormalParam:@"tilePosX"
                                  extractor:^(CGFloat value){self.tilePosX = value;}
                                      value:TILE_POS_X_DEFAULT],

                    [self createNormalParam:@"tilePosY"
                                  extractor:^(CGFloat value){self.tilePosY = value;}
                                      value:TILE_POS_Y_DEFAULT],

                    [self createParam:@"tileWidth"
                           extractor1:^(CGFloat value){self.tileWidth = value;}
                           extractor2:^(CGFloat value){self.tileWidth = self.tileHeight = value;}
                                value:TILE_WIDTH_DEFAULT],

                    [self createConditionalParam:@"tileHeight"
                                       extractor:^(CGFloat value){self.tileHeight = value;}
                                           value:TILE_HEIGHT_DEFAULT]
                    ];
}

-(void)initRenderButton{ 
    UIButton * button =[UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button addTarget:self action:@selector(renderTile) forControlEvents:UIControlEventTouchDown];
    [button setTitle:@"Render Tile" forState:UIControlStateNormal];
    self.renderButton =button;
}


-(MLOTestingTileParameter *) createNormalParam:(NSString *)name extractor:(MLOTestingTileParameterExtractor) extractor value:(CGFloat)defaultValue{
      return [self createParam:name extractor1:extractor extractor2:extractor value:defaultValue];}

-(MLOTestingTileParameter *) createConditionalParam:(NSString *)name extractor:(MLOTestingTileParameterExtractor) extractor value:(CGFloat)defaultValue{
    return [self createParam:name extractor1:extractor extractor2:nil value:defaultValue];
}

-(MLOTestingTileParameter *) createParam:(NSString *)name extractor1:(MLOTestingTileParameterExtractor) extractor1 extractor2:(MLOTestingTileParameterExtractor) extractor2 value:(CGFloat)defaultValue{
    return [[MLOTestingTileParameter alloc] initWithParams:self label:name extractor1:extractor1 extractor2:extractor2 defaultValue:defaultValue];
}


-(void)resize{
    NSLog(@"%@ resize",self);
    CGFloat height = self.view.frame.size.height;
    CGFloat width = self.view.frame.size.width;
    if(width<height){
        height = height/2.0f;
    }
    CGFloat heightWithoutButton = height - RENDER_BUTTON_HEIGHT;
    CGFloat paramHeight = heightWithoutButton / [self.params count];
    CGFloat originY = 0;
    for (MLOTestingTileParameter * param in self.params) {
        [param setParamFrame:CGRectMake(0,
                                       originY,
                                       width,
                                       paramHeight)];
        originY+=paramHeight;
    }
    self.renderButton.frame = CGRectMake(0,
                                         originY,
                                         width,
                                         RENDER_BUTTON_HEIGHT);

}
-(void)addToSuperview{
    NSLog(@"%@ addToSuperview",self);
    [self.tester.view addSubview:self.view];
    for (MLOTestingTileParameter * param in self.params) {
        [param addToSuperview];
    }

    [self.view addSubview:self.renderButton];


}
-(void)renderTile{
    NSLog(@"%@ renderTile",self);
    for (MLOTestingTileParameter * param in self.params) {
        [param extract];
    }
    [self.tester.renderer render];
}
@end
