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
@property UITextField * step;
@property NSInteger defaultValue;
@property UIStepper * dataStepper;
@property UIStepper * stepStepper;
@end

static const CGFloat DEFAULT_STEP_VALUE = 1;

@implementation MLOTestingTileParameter

-(MLOTestingTileParameter *)initWithParams:(MLOTestingTileParametersViewController *) params label:(NSString *)label extractor:(MLOTestingTileParameterExtractor) extractor defaultValue:(NSInteger) defaultValue{
    NSLog(@"Creating tile testing param %@ with default value %d",label,defaultValue);
    self = [self init];
    if(self){
        self.params = params;
        self.extractor = extractor;
        self.defaultValue = defaultValue;
        [self initLabel:label];
        self.dataStepper = [self createStepper];
        self.stepStepper = [self createStepper];
        [self initDataTextField];
        [self initStepTextField];
    }
    return self;
}

-(UIStepper *) createStepper{
    UIStepper * stepper = [UIStepper new];
    stepper.maximumValue = MAXFLOAT;
    stepper.minimumValue = -MAXFLOAT;
    stepper.stepValue = DEFAULT_STEP_VALUE;
    stepper.autorepeat = YES;
    stepper.continuous = NO;
    [stepper addObserver:self forKeyPath:@"value"
                          options: NSKeyValueObservingOptionNew
                          context:0];
    return stepper;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{

    if (object == self.dataStepper) {
         NSNumber * floatNumber = change[NSKeyValueChangeNewKey];
        CGFloat value = [self currentDataValue] + [floatNumber floatValue];

        if(value == ((NSInteger) value)){
            self.data.text = [[NSNumber numberWithInteger:(NSInteger) value] stringValue];
        }else{
            self.data.text = [[NSNumber numberWithFloat:value] stringValue];
        }
        [self.params renderTile];
    }else if (object == self.stepStepper){

        NSNumber * floatNumber = change[NSKeyValueChangeNewKey];
        NSInteger value = [floatNumber integerValue];
        NSNumber * newValue = [NSNumber numberWithInteger:value];
        self.step.text = [newValue stringValue];
        self.dataStepper.stepValue = [newValue floatValue];
    }
}


-(void)initStepTextField{

    self.step =[[UITextField alloc] initWithFrame:CGRECT_ZERO];
    self.data.textAlignment = NSTextAlignmentLeft;
    self.step.text = [[NSNumber numberWithInteger:(NSInteger)DEFAULT_STEP_VALUE] stringValue];
}

-(NSString *)description{
    return [@"MLOTestingTileParameter: " stringByAppendingString:self.label.text];
}

-(void) initLabel:(NSString *) label{
    self.label =[[UILabel alloc] initWithFrame:CGRECT_ZERO];
    self.label.text =  label;
    self.label.textAlignment = NSTextAlignmentCenter;
}

-(void) initDataTextField{
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

    CGFloat x = paramFrame.origin.x;
    CGFloat y = paramFrame.origin.y;
    CGFloat w = paramFrame.size.width;
    CGFloat h = paramFrame.size.height;

    CGFloat labelW = w/3.0f;
    CGFloat otherW = w/6.0f;

    self.label.frame=CGRectMake(x,
                                y,
                                labelW,
                                h);
    self.data.frame =CGRectMake(x + labelW,
                                y,
                                otherW,
                                h);
    self.dataStepper.frame = CGRectMake(x + labelW + otherW,
                                        y,
                                        otherW,
                                        h);

    self.step.frame = CGRectMake(x + labelW + 2*otherW,
                                 y,
                                 otherW,
                                 h);
    self.stepStepper.frame = CGRectMake(x + labelW + 3*otherW,
                                        y,
                                        otherW,
                                        h);

}

-(void)addToSuperview{
    NSLog(@"%@ addToSuperview",self);
    [self.params.view addSubview:self.label];
    [self.params.view addSubview:self.data];
    [self.params.view addSubview:self.dataStepper];
    [self.params.view addSubview:self.step];
    [self.params.view addSubview:self.stepStepper];
}

-(BOOL)isNumber:(NSString *) string{

    return [[NSNumberFormatter new]numberFromString:string] == nil;
}

-(CGFloat) currentDataValue{
    if([self isNumber:self.data.text]){

        NSLog(@"%@ got illegal value: %@, reseting to %d",self,self.data.text,self.defaultValue);

        [self resetValue];
    }
    return [self.data.text floatValue];
}

-(void)extract{
    NSLog(@"%@ extract",self);

    self.extractor([self currentDataValue]);
}
@end
