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

// Container for slideshow contents like images/notes
// Support asyncloadDelegate methods.
@interface SlideShow : NSObject

@property uint size;
@property uint currentSlide;
//@property (nonatomic, strong) NSString * title;
@property (nonatomic, strong) id <AsyncLoadHorizontalTableDelegate> delegate;
// Used for loading preview for next slide in the smaller UIImageView
@property (nonatomic, strong) id secondaryDelegate;

// When an image is received, we put it into the correct index
- (void) putImage: (NSString *)img AtIndex: (uint) index;
// When an note is received, we put it into the correct index
- (void) putNotes: (NSString *)notes AtIndex: (uint) index;

// Register an entry in the loadBuffer which loads the content (image or notes) directly into the desired UI element or wait until received the desired content from the server
- (void) getContentAtIndex: (uint) index forView: (UIView*) view;
// Store the filename of the slideshow
- (NSString *) title;
- (void) setTitle:(NSString *)title;

@end
