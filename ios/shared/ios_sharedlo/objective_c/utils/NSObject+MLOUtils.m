// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "NSObject+MLOUtils.h"
static CGFloat smaller = -1,bigger = -1;
static CGRect portrait,landscape;

@implementation NSObject (MLOUtils)

-(void)fetchStatics{
    if(smaller<0){
        smaller = [UIScreen mainScreen].bounds.size.height;
        bigger = [UIScreen mainScreen].bounds.size.width;
        if(smaller> bigger){
            NSInteger temp = bigger;
            bigger= smaller;
            smaller= temp;
        }
        portrait = CGRectMake(0, 0, smaller, bigger);
        landscape  = CGRectMake(0, 0, bigger, smaller);
    }
}

-(CGRect) getFullFrameForRect:(CGRect) similarRect{
    [self fetchStatics];
    if(similarRect.size.width >smaller){
        
        return landscape;
    }
    return portrait;
}
- (void)performBlock:(void (^)(void))block
          afterDelay:(NSTimeInterval)delay
{
    [self performSelector:@selector(fireBlockAfterDelay:)
               withObject:block
               afterDelay:delay];
}

- (void)fireBlockAfterDelay:(void (^)(void))block {
    block();
}

-(BOOL)returnSuccees:(BOOL) success forAction:(NSString *) action andAlertForError:(NSError *) error  {
    if(!success){
        
        UIAlertView *failedDeleteAlert =
        [[UIAlertView alloc]initWithTitle:@"Meh:"
                                  message:[NSString stringWithFormat:@"Failed to perform: %@",action]
                                 delegate:self
                        cancelButtonTitle:@"Close"
                        otherButtonTitles:nil];
        
        [failedDeleteAlert show];
        
        NSLog(@"Could not %@ :%@ ",action,[error localizedDescription]);
    }
    return success;
}

@end
