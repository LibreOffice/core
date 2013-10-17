// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOManager.h"
#import "MLOMainViewController.h"
#import "MLOInvoker.h"
#include "touch/touch.h"

#import "mlo.h"
#import "mlo_uno.h"

#define APP_STUB_DEF (UIApplication *) X { (void) X;}

static MLOManager * instance = nil;

static const NSTimeInterval FADE_IN_DURATION = 0.3;

@interface MLOManager ()

@property NSObject<MLOInvoker> * invoker;
@property NSString * openedFileNameWithExtension;
@property NSString * openedFilePath;

@end

@implementation MLOManager

-(id) init{
    self = [super init];
    
    if (self) {
        self.mainViewController = nil;
        self.invoker = nil;
        [self resetOpenedFile];
    }
    return self;
}

-(BOOL)isInit{
    return self.mainViewController != nil;
}

- (void)applicationDidEnterBackground: APP_STUB_DEF
- (void)applicationWillEnterForeground: APP_STUB_DEF
- (void)applicationDidBecomeActive: APP_STUB_DEF
- (void)applicationWillTerminate:  APP_STUB_DEF


- (void)applicationWillResignActive:(UIApplication *) application{
    if (_mainViewController) {
        [_mainViewController hideLibreOffice];
    }
}

-(BOOL)application:(UIApplication *) application didFinishLaunchingWithOptions:(NSDictionary *) launchOptions{

    return YES;
}

- (void)application:(UIApplication *)application didChangeStatusBarFrame:(CGRect)oldStatusBarFrame
{
    IGNORE_ARG(application);
    
    IGNORE_ARG(oldStatusBarFrame);
    
    [self.mainViewController rotate];
}

-(void)start{
    
    NSLog(@"L O : START LIBRE OFFICE");
    
    if (![self isInit]) {
        
        NSLog(@"L O : BEGINNING INITIALIZATION");
        
        [self initLo];
        
        NSLog(@"L O : INITIALIZATION COMPLETED!!!");
    }else{
        NSLog(@"L O : SKIPPED. ALREADY INITIALIZED.");
    }
}

+(MLOManager *) getInstance{
    if(instance ==nil){
        instance = [self new];
    }
    return instance;
}

-(void) initLo{
    
    self.mainViewController = [MLOMainViewController new];
    
    [[[NSThread alloc] initWithTarget:self selector:@selector(threadMainMethod:) object:nil] start];
    
}

-(void)addLibreOfficeAsSubview:(UIView * ) superview{
    
    self.mainViewController.view.alpha = 0;
    
    [superview addSubview: self.mainViewController.view];
    
    [UIView animateWithDuration:FADE_IN_DURATION animations:^(){
        self.mainViewController.view.alpha = 1;
    }];
}

-(void)showLibreOfficeAfterAddingToSuperView:(UIWindow *) window{
    window.backgroundColor = [UIColor whiteColor];
    
    [_mainViewController showLibreOffice:window];
}

- (void)threadMainMethod:(id)argument
{
    IGNORE_ARG(argument);
    
    @autoreleasepool {
        NSLog(@"CALLING mlo_initialize");
        
        mlo_initialize();
        
        NSLog(@"touch_lo_runMain RETURNED\r\n\r\nCALLING lo_runMain");
        
        touch_lo_runMain();
        
        NSLog(@"lo_runMain RETURNED");
    }
}

-(void) hideLibreOffice{
    
    [self.invoker willHideLibreOffice];
    
    mlo_close();
    
    [self resetOpenedFile];
    
    [self.mainViewController.view removeFromSuperview];
    
    [self.invoker didHideLibreOffice];
    
}

-(CGRect) bounds{
    return [self.invoker bounds];
}

-(void)resetOpenedFile{
    self.openedFileNameWithExtension = nil;
    self.openedFilePath = nil;
}

-(void)openInLibreOfficeFilePath:(NSString *) filePath fileNameWithExtension:(NSString *) fileNameWithExtension superView:(UIView *) superview window:(UIWindow *) window invoker:
(NSObject<MLOInvoker> *) invoker{
    self.invoker = invoker;
    self.openedFileNameWithExtension = fileNameWithExtension;
    self.openedFilePath = filePath;
    if(mlo_open_file(filePath)){
        [invoker willShowLibreOffice];
        [self addLibreOfficeAsSubview:superview];
        [self showLibreOfficeAfterAddingToSuperView:window];
        [invoker didShowLibreOffice];
    }else{
        [self hideLibreOffice];
        
    }
}

-(void)openInLibreOfficeFilePath:(NSString *) filePath superView:(UIView *) superview window:(UIWindow *) window invoker:(NSObject<MLOInvoker> *) invoker{
    [self openInLibreOfficeFilePath:filePath
              fileNameWithExtension:[filePath lastPathComponent]
                          superView:superview
                             window:window
                            invoker:invoker];
}


-(NSString *)extension{
    
    NSString * extension= [self.openedFilePath pathExtension];
    NSLog(@"File extension is %@",extension);
    return extension;
}
-(NSString *)filenameWithExtension{
    NSLog(@"Filename with extension is %@",self.openedFileNameWithExtension);
    return self.openedFileNameWithExtension;
}

@end
