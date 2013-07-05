// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "SlideShow.h"
#import "Base64.h"
#import "slideShowPreview_vc.h"
#import <dispatch/dispatch.h>

@interface SlideShow()

@property (nonatomic, strong) NSMutableDictionary* imagesDictionary;
@property (nonatomic, strong) NSMutableDictionary* notesDictionary;

@property int lastRequestedImage, lastRequestedNote;
@property (nonatomic, strong) id slideShowImageReadyObserver;
@property (nonatomic, strong) id slideShowNoteReadyObserver;

@end

@implementation SlideShow

@synthesize size = _size;
@synthesize currentSlide = _currentSlide;
@synthesize delegate = _delegate;

dispatch_queue_t backgroundQueue;

- (SlideShow *) init{
    self = [super init];
    self.imagesDictionary = [[NSMutableDictionary alloc] init];
    self.notesDictionary = [[NSMutableDictionary alloc] init];
    _size = 0;
    _currentSlide = 0;
    
    backgroundQueue = dispatch_queue_create("org.libreoffice.iosremote.bgqueue", NULL);
//    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
//    self.slideShowImageReadyObserver =[[NSNotificationCenter defaultCenter]
//                                              addObserverForName:@"storage_update_image_ready"
//                                                          object:nil
//                                                           queue:mainQueue
//                                                      usingBlock:^(NSNotification *note) {
//                                                          if ([[[note userInfo] objectForKey:@"index"] intValue] == self.lastRequestedImage) {
//                                                              [self.delegate.image setImage:[self getImageAtIndex:self.lastRequestedImage]];
//                                                              self.lastRequestedImage = -1;
//                                                          }
//                                                      }];
    
//    self.slideShowNoteReadyObserver = [[NSNotificationCenter defaultCenter] addObserverForName:@"storage_update_note_ready"
//                                                          object:nil
//                                                           queue:mainQueue
//                                                      usingBlock:^(NSNotification *note) {
//                                                          if ([[[note userInfo] objectForKey:@"index"] intValue] == self.lastRequestedNote) {
//                                                              [self.delegate.lecturer_notes loadHTMLString:[self getNotesAtIndex:self.lastRequestedNote] baseURL:nil];
//                                                              self.lastRequestedNote = -1;
//                                                          }
//                                                      }];
    
    return self;
}

- (void) putImage: (NSString *)img AtIndex: (uint) index{
//    NSLog(@"Put Image into %u", index);
    dispatch_async(backgroundQueue, ^(void) {
        NSData* data = [NSData dataWithBase64String:img];
        UIImage* image = [UIImage imageWithData:data];
        [self.imagesDictionary setObject:image forKey:[NSNumber numberWithUnsignedInt:index]];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"storage_update_image_ready"
                                                            object:nil
                                                          userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:index] forKey:@"index"]];
    });
}

- (void) putNotes: (NSString *)notes AtIndex: (uint) index{
//    NSLog(@"Put note into %u", index);
    [self.notesDictionary setObject:notes forKey:[NSNumber numberWithUnsignedInt:index]];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"storage_update_note_ready"
                                                        object:nil
                                                      userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:index] forKey:@"index"]];
}

- (UIImage *) getImageAtIndex: (uint) index
{
    if (![self.imagesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]]) {
        self.lastRequestedImage = index;
        return nil;
    }
    else{
        self.lastRequestedImage = -1;
        return [self.imagesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]];
    }
}

- (NSString *) getNotesAtIndex: (uint) index
{
    if (![self.notesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]]) {
        self.lastRequestedNote = index;
        return nil;
    }
    else {
        self.lastRequestedNote = -1;
        return [self.notesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]];
    }
}



@end
