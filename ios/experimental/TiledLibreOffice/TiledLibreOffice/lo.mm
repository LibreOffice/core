// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdlib.h>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#include <osl/detail/component-mapping.h>
#include <osl/process.h>
#include <touch/touch.h>

extern "C"
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {
        LO_EXTENDED_CORE_FACTORY_MAP
        LO_WRITER_FACTORY_MAP
        { "libcuilo.a", cui_component_getFactory },
        { "libspllo.a", spl_component_getFactory },
        { "libuuilo.a", uui_component_getFactory },
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

static NSString *createPaths(NSString *base, NSString *appRootEscaped, NSArray *fileNames)
{
    NSString *result;
    NSString *prefix = @"file://";
    BOOL first = YES;

    result = base;
    for (NSString *fileName in fileNames) {
        result = [result stringByAppendingString: prefix];
        result = [result stringByAppendingString: [appRootEscaped stringByAppendingPathComponent: fileName]];

        if (first) {
            prefix = [@" " stringByAppendingString:prefix];
            first = NO;
        }
    }

    return result;
}

extern "C" void lo_initialize(void)
{
    NSString * bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString * app_root_escaped = [bundlePath stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];
    NSString * uno_types = createPaths(@"-env:UNO_TYPES=", app_root_escaped, @[@"offapi.rdb", @"oovbaapi.rdb", @"types.rdb"]);
    NSString * uno_services = createPaths(@"-env:UNO_SERVICES=", app_root_escaped, @[@"ure/services.rdb", @"services.rdb"]);

    const char *argv[] = {
        [[[NSBundle mainBundle] executablePath] UTF8String],
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        [uno_types UTF8String],
        [uno_services UTF8String],
        [[@"file://" stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"test1.odt"]] UTF8String]};

    const int argc = sizeof(argv)/sizeof(*argv);

    osl_setCommandArgs(argc, (char **) argv);
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
