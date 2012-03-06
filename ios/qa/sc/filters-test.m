/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Tor Lillqvist <tml@iki.fi> (initial developer)
 * Copyright (C) 2011 SUSE Linux http://suse.com (initial developer's employer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#import <UIKit/UIKit.h>

#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/plugin/TestPlugIn.h"
//#include "cppunit/plugin/PlugInManager.h"

extern "C" {
    extern CppUnitTestPlugIn *cppunitTestPlugIn(void);
    extern int lo_main(int argc, const char **argv);
}

int 
main(int argc, char ** argv)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    int retVal = UIApplicationMain (argc, argv, @"UIApplication", @"loAppDelegate");
    [pool release];
    return retVal;
}

@interface loAppDelegate : NSObject <UIApplicationDelegate> {
}
@property (nonatomic, retain) UIWindow *window;
@end

@implementation loAppDelegate

@synthesize window=_window;

- (BOOL)application: (UIApplication *) application
didFinishLaunchingWithOptions: (NSDictionary *) launchOptions
{
  (void) application;
  (void) launchOptions;

  UIWindow *uiw = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  uiw.backgroundColor = [UIColor redColor];
  self.window = uiw;
  [uiw release];

  CppUnitTestPlugIn *iface = cppunitTestPlugIn();
  iface->initialize(&CppUnit::TestFactoryRegistry::getRegistry(), CppUnit::PlugInParameters());

  const char *argv[] = {
      "lo-qa-sc-filters-test",
      "dummy-testlib",
      "--headless",
      "--protector",
      "dummy-libunoexceptionprotector",
      "dummy-unoexceptionprotector"
  };

  lo_main(sizeof(argv)/sizeof(*argv), argv);

  [self.window makeKeyAndVisible];
  return YES;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

