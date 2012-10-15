/* -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>

#include <premac.h>
#import <UIKit/UIKit.h>
#include <postmac.h>

#include <cppuhelper/bootstrap.hxx>
#include <osl/detail/ios-bootstrap.h>
#include <osl/process.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

#ifdef __cplusplus
extern "C" {
#endif

extern void * analysis_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
extern void * date_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
extern void * sc_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
extern void * scfilt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
extern void * unoxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );

const lib_to_component_mapping *
lo_get_libmap(void)
{
    static lib_to_component_mapping map[] = {
        { "libanalysislo.a", analysis_component_getFactory },
        { "libdatelo.a", date_component_getFactory },
        { "libscfiltlo.a", scfilt_component_getFactory },
        { "libsclo.a", sc_component_getFactory },
        { "libunoxmllo.a", unoxml_component_getFactory },
        { NULL, NULL }
    };

    return map;
}

#ifdef __cplusplus
}
#endif

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
    int i;

    (void) application;
    (void) launchOptions;

    UIWindow *uiw = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    uiw.backgroundColor = [UIColor redColor];
    self.window = uiw;
    [uiw release];

    // See unotest/source/cpp/bootstrapfixturebase.cxx
    const char *app_root = [[[NSBundle mainBundle] bundlePath] UTF8String];
    setenv("SRC_ROOT", app_root, 1);
    setenv("OUTDIR_FOR_BUILD", app_root, 1);

    setenv("SAL_LOG", "yes", 1);

    const char *argv[] = {
        "lo-qa-sc-filters-test",
        "dummy-testlib",
        "--headless",
        "--protector",
        "dummy-libunoexceptionprotector",
        "unoexceptionprotector",
        "--protector",
        "dummy-libunobootstrapprotector",
        "unobootstrapprotector",
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        "placeholder-uno-types",
        "placeholder-uno-services"
    };

    const int argc = sizeof(argv)/sizeof(*argv);

    NSString *app_root_escaped = [[[NSBundle mainBundle] bundlePath] stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];

    NSString *uno_types = @"-env:UNO_TYPES=";

    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"udkapi.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" "];
    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"types.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" "];
    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"ure/types.rdb"]];

    assert(strcmp(argv[argc-2], "placeholder-uno-types") == 0);
    argv[argc-2] = [uno_types UTF8String];

    NSString *uno_services = @"-env:UNO_SERVICES=";

    const char *services[] = {
        "services.rdb",
        "ComponentTarget/basic/util/sb.component",
        "ComponentTarget/chart2/source/controller/chartcontroller.component",
        "ComponentTarget/chart2/source/chartcore.component",
        "ComponentTarget/comphelper/util/comphelp.component",
        "ComponentTarget/eventattacher/source/evtatt.component",
        "ComponentTarget/fileaccess/source/fileacc.component",
        "ComponentTarget/filter/source/config/cache/filterconfig1.component",
        "ComponentTarget/oox/util/oox.component",
        "ComponentTarget/package/source/xstor/xstor.component",
        "ComponentTarget/package/util/package2.component",
        "ComponentTarget/sax/source/expatwrap/expwrap.component",
        "ComponentTarget/sax/source/fastparser/fastsax.component",
        "ComponentTarget/sc/util/sc.component",
        "ComponentTarget/sc/util/scfilt.component",
        "ComponentTarget/scaddins/source/analysis/analysis.component",
        "ComponentTarget/scaddins/source/datefunc/date.component",
        "ComponentTarget/sot/util/sot.component",
        "ComponentTarget/svl/util/svl.component",
        "ComponentTarget/toolkit/util/tk.component",
        "ComponentTarget/ucb/source/ucp/tdoc/ucptdoc1.component",
        "ComponentTarget/unotools/util/utl.component",
        "ComponentTarget/unoxml/source/rdf/unordf.component",
        "ComponentTarget/framework/util/fwk.component",
        "ComponentTarget/i18npool/util/i18npool.component",
        "ComponentTarget/sfx2/util/sfx.component",
        "ComponentTarget/unoxml/source/service/unoxml.component",
        "ComponentTarget/configmgr/source/configmgr.component",
        "ComponentTarget/ucb/source/core/ucb1.component",
        "ComponentTarget/ucb/source/ucp/file/ucpfile1.component"
    };

    for (i = 0; i < sizeof(services)/sizeof(services[0]); i++) {
        uno_services = [uno_services stringByAppendingString: @"file://"];
        uno_services = [uno_services stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: [NSString stringWithUTF8String: services[i]]]];
        if (i < sizeof(services)/sizeof(services[0]) - 1)
            uno_services = [uno_services stringByAppendingString: @" "];
    }

    assert(strcmp(argv[argc-1], "placeholder-uno-services") == 0);
    argv[argc-1] = [uno_services UTF8String];

    osl_setCommandArgs(argc, (char **) argv);

    Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());

    Reference< XMultiComponentFactory > xMultiComponentFactoryClient(
        xComponentContext->getServiceManager() );

    Reference< XInterface > xInterface =
        xMultiComponentFactoryClient->createInstanceWithContext(
            OUString("com.sun.star.frame.Desktop"),
            xComponentContext );

    // Do something here

    [self.window makeKeyAndVisible];
    return YES;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
