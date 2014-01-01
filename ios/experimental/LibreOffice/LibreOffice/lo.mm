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

#include <osl/process.h>
#include <touch/touch.h>

extern "C"
void
lo_initialize(void)
{
    const char *argv[] = {
        "placeholder-exe",
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        "placeholder-uno-types",
        "placeholder-uno-services",
        "placeholder-document"
    };

    const int argc = sizeof(argv)/sizeof(*argv);

    argv[0] = [[[NSBundle mainBundle] executablePath] UTF8String];

    NSString *app_root_escaped = [[[NSBundle mainBundle] bundlePath] stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];

    NSString *uno_types = @"-env:UNO_TYPES=";

    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"offapi.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"oovbaapi.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"types.rdb"]];

    assert(strcmp(argv[2], "placeholder-uno-types") == 0);
    argv[2] = [uno_types UTF8String];

    NSString *uno_services = @"-env:UNO_SERVICES=";

    uno_services = [uno_services stringByAppendingString: @"file://"];
    uno_services = [uno_services stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"ure/services.rdb"]];

    uno_services = [uno_services stringByAppendingString: @" file://"];
    uno_services = [uno_services stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"services.rdb"]];

    assert(strcmp(argv[3], "placeholder-uno-services") == 0);
    argv[3] = [uno_services UTF8String];

    assert(strcmp(argv[4], "placeholder-document") == 0);
    NSString *file = @"file://";
    file = [file stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"test1.odt"]];
    argv[4] = [file UTF8String];

    osl_setCommandArgs(argc, (char **) argv);
}
