// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// This file is part of the LibreOffice project.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>

#import "View.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate, UITextViewDelegate>

@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) View *view;

- (void)threadMainMethod: (id) argument;
- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text;

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
