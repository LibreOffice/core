// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOTestingTileParameter.h"
#import "MLOTestingTileParametersViewController.h"

@interface MLOTestingTileParameter ()
@property MLOTestingTileParametersViewController * params;
@property (nonatomic,strong) MLOTestingTileParameterExtractor extractor;
@property UILabel * label;
@property UITextField * data;
@property NSInteger defaultValue;
@end

@implementation MLOTestingTileParameter

-(MLOTestingTileParameter *)initWithParams:(MLOTestingTileParametersViewController *) params label:(NSString *)label extractor:(MLOTestingTileParameterExtractor) extractor defaultValue:(NSInteger) defaultValue{
    NSLog(@"Creating tile testing param %@ with default value %d",label,defaultValue);
    self = [self init];
    if(self){
        self.params = params;
        self.extractor = extractor;
        self.defaultValue = defaultValue;

        [self initLabel:label];
        [self initTextField];
    }
    return self;
}

-(NSString *)description{
    return [@"MLOTestingTileParameter: " stringByAppendingString:self.label.text];
}

-(void) initLabel:(NSString *) label{
    self.label =[[UILabel alloc] initWithFrame:CGRECT_ZERO];
    self.label.text =  label;
    self.label.textAlignment = NSTextAlignmentRight;
}

-(void) initTextField{
    self.data = [[UITextField alloc] initWithFrame:CGRECT_ZERO];
    [self.data setKeyboardType:UIKeyboardTypeNumberPad];
    self.data.textAlignment = NSTextAlignmentLeft;
    [self resetValue];
}

-(void)resetValue{

    self.data.text = [NSString stringWithFormat:@"%d",self.defaultValue];
}

-(void)setParamFrame:(CGRect)  paramFrame{
    NSLog(@"%@ setParamFrame",self);

    self.label.frame=CGRectMake(paramFrame.origin.x,
                                paramFrame.origin.y,
                                paramFrame.size.width/2.0f,
                                paramFrame.size.height);
    self.data.frame =CGRectMake(paramFrame.origin.x + paramFrame.size.width/2.0f,
                                paramFrame.origin.y,
                                paramFrame.size.width/2.0f,
                                paramFrame.size.height);
}

-(void)addToSuperview{
    NSLog(@"%@ addToSuperview",self);
    [self.params.view addSubview:self.label];
    [self.params.view addSubview:self.data];
}

-(void)extract{
    NSLog(@"%@ extract",self);

    if([[NSNumberFormatter new]numberFromString:self.data.text] == nil){

        NSLog(@"%@ got illegal value: %@, reseting to %d",self,self.data.text,self.defaultValue);

        [self resetValue];
    }
    self.extractor([self.data.text floatValue]);
}
@end
