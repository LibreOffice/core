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
const lib_to_component_mapping *
lo_get_library_map(void)
{
    static lib_to_component_mapping map[] = {
        NON_APP_SPECIFIC_COMPONENT_MAP

        //from IOS
        
        //MAP_LIB_LO(analysis), //MAP_LIB_LO(animcore), //MAP_LIB_LO(sm),
        //MAP_LIB_LO(avmedia),  //MAP_LIB_LO(dba),      //MAP_LIB_LO(date),
        //MAP_LIB_LO(fwk),      //MAP_LIB_LO(fwm),      //MAP_LIB_LO(frm),
        //MAP_LIB_LO(pricing),  //MAP_LIB_LO(scd),      //MAP_LIB_LO(scfilt),
        //MAP_LIB_LO(hwp),
        MAP_LIB_LO(hyphen),
        //MAP_LIB_LO(dbaxml), //MAP_LIB_LO(sdd),      //MAP_LIB_LO(smd),
        MAP_LIB_LO(spell),
        //MAP_LIB_LO(svgfilter),//MAP_LIB_LO(t602filter),//MAP_LIB_LO(textfd),
        MAP_LIB_LO(uui),
        //MAP_LIB_LO(xmlfd),
        MAP_LIB(xmlsecurity), //MAP_LIB_LO(sc),       //MAP_LIB_LO(sd),
        MAP_LIB_LO(svx),        MAP_LIB_LO(fwl),        MAP_LIB_LO(oox),
        //MAP_LIB(fileacc),
        MAP_LIB_LO(svt),        MAP_LIB_LO(xof),
        MAP_LIB_LO(evtatt),     MAP_LIB_LO(fsstorage),  MAP_LIB_LO(lng),
        MAP_LIB_LO(swd),        MAP_LIB_LO(sw),         MAP_LIB_LO(lnth),
        MAP_LIB_LO(unordf),     MAP_LIB_LO(unoxml),     MAP_LIB_LO(xo),
        MAP_LIB_LO(stocservices), MAP_LIB_LO(reflection),

        // from ANDROID
        
        //MAP_LIB_LO(cui),      //MAP_LIB_LO(basprov),  //MAP_LIB_LO(chartcore),
        //MAP_LIB_LO(dlgprov),  //MAP_LIB_LO(vclcanvas),//MAP_LIB_LO(sb),
        //MAP_LIB(scriptframe), //MAP_LIB_LO(vbaevents),
        //MAP_LIB_LO(spl),      //MAP_LIB_LO(tk),       //MAP_LIB_LO(stringresource),
        //MAP_LIB(ucppkg1),     //MAP_LIB_LO(vbaswobj), //MAP_LIB_LO(wpftwriter),
        //MAP_LIB_LO(wpftdraw), //MAP_LIB_LO(svxcore),  //MAP_LIB_LO(protocolhandler),
        //MAP_LIB_LO_1(filterconfig), //MAP_LIB_LO(canvasfactory),    //MAP_LIB_LO(mtfrenderer),
        MAP_LIB_LO(writerfilter),
        MAP_LIB(embobj),        MAP_LIB(emboleobj),     MAP_LIB_LO(sot),
        
        // ADDED for search
        
        MAP_LIB_LO(i18nsearch),
        
        { NULL, NULL }
    };

    return map;
}

extern "C"
const lib_to_component_mapping *
lo_get_implementation_map(void)
{
    static lib_to_component_mapping map[] = {
        NON_APP_SPECIFIC_DIRECT_COMPONENT_MAP
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
