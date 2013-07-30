// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "SlideShow.h"
#import "Base64.h"
#import "slideShow_vc_iphone.h"
#import "UIImage+Resize.h"
#import <dispatch/dispatch.h>

@interface SlideShow()

@property (atomic, strong) NSMutableDictionary* imagesDictionary;
@property (atomic, strong) NSMutableDictionary* notesDictionary;

@property (atomic, strong) NSMutableDictionary* loadBuffer;
@property (nonatomic, strong) id slideShowImageReadyObserver;
@property (nonatomic, strong) id slideShowNoteReadyObserver;

@end

@implementation SlideShow

@synthesize size = _size;
@synthesize currentSlide = _currentSlide;
@synthesize delegate = _delegate;
@synthesize secondaryDelegate = _secondaryDelegate;

dispatch_queue_t backgroundQueue;
NSLock *dictLock;

- (void) setDelegate:(id<AsyncLoadHorizontalTableDelegate>)delegate
{
    NSLog(@"setting slideshow delegate to a %@", [delegate class]);
    _delegate = delegate;
}

- (SlideShow *) init{
    self = [super init];
    NSLog(@"SlideShow got init");
    self.imagesDictionary = [[NSMutableDictionary alloc] init];
    self.notesDictionary = [[NSMutableDictionary alloc] init];
    self.loadBuffer = [[NSMutableDictionary alloc] init];
    _size = 0;
    _currentSlide = 0;
    
    backgroundQueue = dispatch_queue_create("org.libreoffice.iosremote.bgqueue", DISPATCH_QUEUE_CONCURRENT);
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    /**
     This observer waits for storage updates like new image added or notes received. 
     It then checks in the loadBuffer to see if there is a view waiting for this update in loadBuffer, if yes, it loads it up and remove the waiting entry.
     loadBuffer stores key-value pair with viewTag as a key and slideIndex as value. 
     For the same view, we only keep the last requested slide index on the waiting list.
     It is thus indispensable to identify each view with an unique tag in its view controller. Here we use 0-20 to indentify central vc views and 21-N for swipe-in tableViewController which allows direct slide number change. 
     We handle lecturer's notes at the same time as an entry in the load buffer via an instrospection.
     */
    self.slideShowImageReadyObserver =[[NSNotificationCenter defaultCenter]
                                              addObserverForName:@"storage_update_ready"
                                                          object:nil
                                                           queue:mainQueue
                                                      usingBlock:^(NSNotification *note) {
                                                          dispatch_async(backgroundQueue, ^(void) {
                                                          if ([[self.loadBuffer allKeysForObject:[NSNumber numberWithInt:[[[note userInfo] objectForKey:@"index"] intValue]]] count]) {
                                                              NSArray * tagArray = [self.loadBuffer allKeysForObject:[NSNumber numberWithInt:[[[note userInfo] objectForKey:@"index"] intValue]]];
                                                              for (NSNumber *tag in tagArray) {
                                                                  UIView * view;
                                                                  if ([tag integerValue] > 20)
                                                                      view = [[self.secondaryDelegate view] viewWithTag: [tag integerValue]];
                                                                  else if ([tag integerValue] >= 0){
                                                                      NSLog(@"Received image, try to load for tag:%d", [tag integerValue]);
                                                                      view = [[self.delegate view] viewWithTag:[tag integerValue]];
                                                                      if (!view) {
                                                                          if (!self.delegate) {
                                                                              NSLog(@"Delegate nil");
                                                                          } else if (![self.delegate view])
                                                                              NSLog(@"view nil");
                                                                      }
                                                                      NSLog(@"Will load it into a %@", [view class]);
                                                                  }
                                                                  else
                                                                      view = [[self.delegate horizontalTableView] viewWithTag:[tag integerValue]];
                                                                  if ([view isKindOfClass:[UIImageView class]]){
                                                                      UIImage *image = [self.imagesDictionary objectForKey:[self.loadBuffer objectForKey:tag]];
                                                                      if (image) {
//                                                                          image = [image resizedImage:view.frame.size interpolationQuality:kCGInterpolationDefault];
                                                                          dispatch_async(dispatch_get_main_queue(), ^{
                                                                              NSLog(@"Setting image to tag: %ld", (long)[tag integerValue]);
                                                                             [(UIImageView *)view setImage:image];
                                                                          });
                                                                          [self.loadBuffer removeObjectForKey:tag];
                                                                      }
                                                                  }
                                                                  else if ([view isKindOfClass:[UIWebView class]]){
//                                                                      NSLog(@"Async notes");
                                                                      NSString *note = [self.notesDictionary objectForKey:[self.loadBuffer objectForKey:tag]];
                                                                      if (note) {
                                                                          dispatch_async(dispatch_get_main_queue(), ^{
                                                                              [(UIWebView *)view loadHTMLString:note baseURL:nil];
                                                                          });
                                                                          [self.loadBuffer removeObjectForKey:tag];
                                                                      }
                                                                  } else if ([view isKindOfClass:[UITableViewCell class]]){
                                                                      UIImage *image = [self.imagesDictionary objectForKey:[self.loadBuffer objectForKey:tag]];
                                                                      if (image){
                                                                          UIImageView *imageView = (UIImageView *)[view viewWithTag:1];
//                                                                          image = [image resizedImage:imageView.frame.size interpolationQuality:kCGInterpolationDefault];
                                                                          dispatch_async(dispatch_get_main_queue(), ^{
                                                                              [imageView setImage:image];
                                                                          });
                                                                          [self.loadBuffer removeObjectForKey:tag];
                                                                      }
                                                                  }
                                                              }
                                                          }
                                                          });
                                                      }];
    dictLock = [[NSLock alloc] init];
    return self;
}

- (void) putImage: (NSString *)img AtIndex: (uint) index{
        NSData* data = [NSData dataWithBase64String:img];
        UIImage* image = [UIImage imageWithData:data];
        [dictLock lock];
        [self.imagesDictionary setObject:image forKey:[NSNumber numberWithUnsignedInt:index]];
        [dictLock unlock];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"storage_update_ready"
                                                            object:nil
                                                          userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:index] forKey:@"index"]];

}

- (void) putNotes: (NSString *)notes AtIndex: (uint) index{
//    NSLog(@"Put note into %u", index);
    [self.notesDictionary setObject:notes forKey:[NSNumber numberWithUnsignedInt:index]];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"storage_update_ready"
                                                        object:nil
                                                      userInfo:[NSDictionary dictionaryWithObject:[NSNumber numberWithInt:index] forKey:@"index"]];
}

- (void) getContentAtIndex: (uint) index forView: (UIView*) view
{
    if (index >= self.size)
    {
        if ([view isKindOfClass:[UIImageView class]])
            [(UIImageView* )view setImage:[UIImage imageNamed:@"slide_finished.png"]];
        else if ([view isKindOfClass:[UITableViewCell class]])
        {
            UIImageView *image = (UIImageView *)[view viewWithTag:1];
            [image setImage:[UIImage imageNamed:@"slide_finished.png"]];
        }
        else if ([view isKindOfClass:[UIWebView class]])
            [(UIWebView* )view loadHTMLString: @"SlideShow finished" baseURL:nil];
        return;
    }
    if (![self.imagesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]])
    {
        NSLog(@"Didn't find %u, putting tag: %d into buffer", index, [view tag]);
        [self.loadBuffer setObject:[NSNumber numberWithInt:index ] forKey:[NSNumber numberWithInt:[view tag]]];
    }
    else{
        if ([view isKindOfClass:[UIImageView class]])
            [(UIImageView* )view setImage:[self.imagesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]]];
        else if ([view isKindOfClass:[UITableViewCell class]])
        {
            UIImageView *image = (UIImageView *)[view viewWithTag:1];
            [image setImage:[self.imagesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]]];
        }
        else if ([view isKindOfClass:[UIWebView class]])
            [(UIWebView* )view loadHTMLString: [self.notesDictionary objectForKey:[NSNumber numberWithUnsignedInt:index]] baseURL:nil];
    }
}

@end
