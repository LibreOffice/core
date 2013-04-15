// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#import "ViewController.h"

#include <osl/detail/ios-bootstrap.h>

#import "lo.h"

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    CGRect rect = [self.view bounds];
    NSLog(@"viewDidLoad: bounds=%dx%d@(%d,%d)", (int)rect.size.width, (int)rect.size.height, (int)rect.origin.x, (int)rect.origin.y);
    rect = [self.view frame];
    NSLog(@"  frame=%dx%d@(%d,%d)", (int)rect.size.width, (int)rect.size.height, (int)rect.origin.x, (int)rect.origin.y);
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
