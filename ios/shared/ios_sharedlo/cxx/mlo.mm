// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCommon.h"
//#include <stdlib.h>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#include <osl/detail/component-mapping.h>
#include <touch/touch.h>
#undef TimeValue
#include <osl/process.h>

#define MAP_LIB(LIB) { "lib" #LIB ".a", LIB##_component_getFactory }
#define MAP_LIB_LO(LIB) { "lib" #LIB "lo.a", LIB##_component_getFactory }
#define MAP_LIB_LO_1(LIB) { "lib" #LIB "lo.a", LIB##1_component_getFactory }

extern "C"
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {
        LO_CORE_FACTORY_MAP
        LO_WRITER_FACTORY_MAP
        MAP_LIB_LO(uui),
        MAP_LIB_LO(i18nsearch),
        { NULL, NULL }
    };

    return map;
}

extern "C"
const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {
        NON_APP_SPECIFIC_CONSTRUCTOR_MAP
        { NULL, NULL }
    };

    return map;
}

NSString * createPaths(NSString * base,NSString * appRootEscaped,NSArray * fileNames){
    NSString * prefix = @"file://";
    BOOL first = YES;
    
    for (NSString * fileName in fileNames) {
        
        base = [base stringByAppendingString: prefix];
        
        base = [base stringByAppendingString: [appRootEscaped stringByAppendingPathComponent: fileName]];
        
        if(first){
            prefix = [@" " stringByAppendingString:prefix];
            first = NO;
        }
    }
    
    return base;
}


extern "C"
void
mlo_initialize(void)
{
    NSString * bundlePath = [[NSBundle mainBundle] bundlePath];
    
    NSString * app_root_escaped = [bundlePath stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];
        
    NSString * uno_types = createPaths(@"-env:UNO_TYPES=", app_root_escaped, @[@"offapi.rdb", @"oovbaapi.rdb", @"types.rdb"]);
    
    NSString * uno_services = createPaths(@"-env:UNO_SERVICES=", app_root_escaped, @[@"ure/services.rdb", @"services.rdb"]);
    
        
    const char *argv[] = {
        [[[NSBundle mainBundle] executablePath] UTF8String],
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        [uno_types UTF8String],
        [uno_services UTF8String],"--writer"};
    
    const int argc = sizeof(argv)/sizeof(*argv);
    
    osl_setCommandArgs(argc, (char **) argv);
}

// Dummy implementation for now, probably this should be handled in a
// different way in each app that uses this "shared" library? Like
// most in touch.h, the dialog API is not set in stone yet.

MLODialogResult touch_ui_dialog_modal(MLODialogKind kind, const char *message)
{
    NSLog(@"===>  %s", message);
    return MLODialogOK;
}
