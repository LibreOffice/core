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
@property UIButton * modeButton;
@property MLOTestingTileParametersMode mode;
@end
@implementation MLOTestingTileParametersViewController

-(id)initWithTester:(MLOAppRoleTileTester *)tester{
    self = [self init];
    if(self){
        self.tester = tester;
        [self initParams];
        [self initModeButton];
        [self initRenderButton];

        self.mode = WIDTH_IS_NOT_HEIGHT;
        [self changeMode];

    }

    NSLog(@"%@ initWithTester",self);
    return self;
}

-(NSString *)description{
    return @"MLOTestingTileParametersViewController";
}

-(void)initParams{
    self.params = @[[self createParam:@"contextWidth"
            widthIsNotHeightExtractor:^(CGFloat value){self.contextWidth = value;}
               widthIsHeightExtractor:^(CGFloat value){self.contextWidth = self.contextHeight = value;}
                                value:CONTEXT_WIDTH_DEFAULT],

                    [self createParam:@"contextHeight"
            widthIsNotHeightExtractor:^(CGFloat value){self.contextHeight = value;}
                                value:CONTEXT_HEIGHT_DEFAULT],

                    [self createParam:@"tilePosX"
                         anyExtractor:^(CGFloat value){self.tilePosX = value;}
                                value:TILE_POS_X_DEFAULT],

                    [self createParam:@"tilePosY"
                         anyExtractor:^(CGFloat value){self.tilePosY = value;}
                                value:TILE_POS_Y_DEFAULT],

                    [self createParam:@"tileWidth"
            widthIsNotHeightExtractor:^(CGFloat value){self.tileWidth = value;}
               widthIsHeightExtractor:^(CGFloat value){self.tileWidth = self.tileHeight = value;}
                                value:TILE_WIDTH_DEFAULT],

                    [self createParam:@"tileHeight"
            widthIsNotHeightExtractor:^(CGFloat value){self.tileHeight = value;}
                                value:TILE_HEIGHT_DEFAULT]
                    ];
}

-(void)initRenderButton{ 
    UIButton * button =[UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button addTarget:self action:@selector(renderTile) forControlEvents:UIControlEventTouchDown];
    [button setTitle:@"Render Tile" forState:UIControlStateNormal];
    self.renderButton =button;
}

-(void)initModeButton{
    UIButton * button =[UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button addTarget:self action:@selector(changeMode) forControlEvents:UIControlEventTouchDown];
    [button setTitle:MLOTestingTileParametersModeString(self.mode) forState:UIControlStateNormal];
    self.modeButton =button;
}

-(void)changeMode{

    switch (self.mode) {
        case WIDTH_IS_HEIGHT:
            self.mode = WIDTH_IS_NOT_HEIGHT;
            break;
        case WIDTH_IS_NOT_HEIGHT:
            self.mode = WIDTH_IS_HEIGHT;
            break;
    }

    [self.modeButton setTitle:MLOTestingTileParametersModeString(self.mode) forState:UIControlStateNormal];
    for(MLOTestingTileParameter * param in self.params){
        [param enterMode:self.mode];
    }
}



-(MLOTestingTileParameter *) createParam:(NSString *)name anyExtractor:(MLOTestingTileParameterExtractor) anyExtractor value:(CGFloat)defaultValue{
      return [self createParam:name widthIsNotHeightExtractor:anyExtractor widthIsHeightExtractor:anyExtractor value:defaultValue];
}

-(MLOTestingTileParameter *) createParam:(NSString *)name widthIsNotHeightExtractor:(MLOTestingTileParameterExtractor) widthIsNotHeightExtractor value:(CGFloat)defaultValue{
    return [self createParam:name widthIsNotHeightExtractor:widthIsNotHeightExtractor widthIsHeightExtractor:nil value:defaultValue];
}

-(MLOTestingTileParameter *) createParam:(NSString *)name widthIsNotHeightExtractor:(MLOTestingTileParameterExtractor) extractor1 widthIsHeightExtractor:(MLOTestingTileParameterExtractor) extractor2 value:(CGFloat)defaultValue{
    return [[MLOTestingTileParameter alloc] initWithParams:self label:name widthIsNotHeightExtractor:extractor1 widthIsHeightExtractor:extractor2 defaultValue:defaultValue];
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
    CGFloat halfWidth = width/2.0f;
    self.modeButton.frame = CGRectMake(0,
                                       originY,
                                       halfWidth,
                                       RENDER_BUTTON_HEIGHT);
    self.renderButton.frame = CGRectMake(halfWidth,
                                         originY,
                                         halfWidth,
                                         RENDER_BUTTON_HEIGHT);

}
-(void)addToSuperview{
    NSLog(@"%@ addToSuperview",self);
    [self.tester.view addSubview:self.view];
    for (MLOTestingTileParameter * param in self.params) {
        [param addToSuperview];
    }

    [self.view addSubview:self.renderButton];
    [self.view addSubview:self.modeButton];


}
-(void)renderTile{
    NSLog(@"%@ renderTile",self);
    for (MLOTestingTileParameter * param in self.params) {
        [param extractMode:self.mode];
    }
    [self.tester.renderer render];
}
@end
