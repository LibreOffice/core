// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <Foundation/Foundation.h>

@interface NSObject (MLOUtils)
-(CGRect) getFullFrameForRect:(CGRect) similarRect;
-(void) performBlock:(void (^)(void))block
          afterDelay:(NSTimeInterval)delay;
-(BOOL)returnSuccees:(BOOL) success forAction:(NSString *) action andAlertForError:(NSError *) error;
@end
