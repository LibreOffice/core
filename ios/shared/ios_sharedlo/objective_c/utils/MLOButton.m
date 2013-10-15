// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOButton.h"
#import "MLOResourceImage.h"

@implementation MLOButton

+(id)buttonWithImage:(MLOResourceImage *) image{
    id button = [MLOButton buttonWithType:UIButtonTypeCustom];
    if(button){
        [button setDefaultImage:image.image];
    }
    return button;
}


-(void)addTarget:(id)target action:(SEL)action{
    [self addTarget:target action:action forControlEvents:UIControlEventTouchUpInside];
}
-(void)addAction:(SEL)action{
    [self addTarget:self action:action];
}

-(void)setDefaultImage:(UIImage *)image{
    [self setImage:image forState:UIControlStateNormal];
    self.contentMode = UIViewContentModeScaleAspectFit;
}

@end
