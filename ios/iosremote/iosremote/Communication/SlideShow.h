// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import <Foundation/Foundation.h>

@protocol AsyncLoadHorizontalTableDelegate <NSObject>

@optional @property (nonatomic, weak) UITableView * horizontalTableView;
@property (nonatomic, strong) UIView * view;

@end

@class slideShowPreview_vc;

@interface SlideShow : NSObject

@property uint size;
@property uint currentSlide;
//@property (nonatomic, strong) NSString * title;
@property (nonatomic, strong) id <AsyncLoadHorizontalTableDelegate> delegate;
@property (nonatomic, strong) id secondaryDelegate;

- (void) putImage: (NSString *)img AtIndex: (uint) index;
- (void) putNotes: (NSString *)notes AtIndex: (uint) index;

- (void) getContentAtIndex: (uint) index forView: (UIView*) view;

- (NSString *) title;
- (void) setTitle:(NSString *)title;

@end
