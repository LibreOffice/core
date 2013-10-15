// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"
#import "MLOInvoker.h"

@class MLOMainViewController;
@interface MLOManager : UIResponder <UIApplicationDelegate,UITextViewDelegate>
@property (strong,nonatomic) MLOMainViewController * mainViewController;

+(MLOManager *) getInstance;

-(BOOL) textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text;
-(void)openInLibreOfficeFilePath:(NSString *) filePath fileNameWithExtension:(NSString *) fileName superView:(UIView *) superview window:(UIWindow *) window invoker:(NSObject<MLOInvoker> *) invoker;
-(void)openInLibreOfficeFilePath:(NSString *) filePath superView:(UIView *) superview window:(UIWindow *) window invoker:(NSObject<MLOInvoker> *) invoker;
-(void) hideLibreOffice;
-(NSString *)filenameWithExtension;
-(NSString *)extension;
-(CGRect)bounds;
-(void)start;

@end
