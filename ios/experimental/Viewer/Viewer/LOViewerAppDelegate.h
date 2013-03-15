//
//  LOViewerAppDelegate.h
//  Viewer
//
//  Created by Tor Lillqvist on 2012-11-27.
//  Copyright (c) 2012 Tor Lillqvist. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LOViewerAppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

- (void) threadMainMethod: (id) argument;

@end
