// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

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

extern "C" {
    extern void * animcore_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * avmedia_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * dba_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * dbaxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * evtatt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fileacc_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * frm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fsstorage_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwk_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwl_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * hwp_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * hyphen_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * lng_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * lnth_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * lotuswordpro_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * oox_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sc_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * scd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * scfilt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sdd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * smd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * spell_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svgfilter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sw_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * swd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * t602filter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * textfd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unoxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unordf_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xmlfd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xmlsecurity_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xo_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * xof_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );

}

extern "C"
const lib_to_component_mapping *
lo_get_libmap(void)
{
    static lib_to_component_mapping map[] = {
        { "libanimcorelo.a", animcore_component_getFactory },
        { "libavmedialo.a", avmedia_component_getFactory },
        { "libdbalo.a", dba_component_getFactory },
        { "libdbaxmllo.a", dbaxml_component_getFactory },
        { "libevtattlo.a", evtatt_component_getFactory },
        { "libfileacc.a", fileacc_component_getFactory },
        { "libfrmlo.a", frm_component_getFactory },
        { "libfsstorage.uno.a", fsstorage_component_getFactory },
        { "libfwklo.a", fwk_component_getFactory },
        { "libfwllo.a", fwl_component_getFactory },
        { "libfwmlo.a", fwm_component_getFactory },
        { "libhwplo.a", hwp_component_getFactory },
        { "libhyphenlo.a", hyphen_component_getFactory },
        { "liblnglo.a", lng_component_getFactory },
        { "liblnthlo.a", lnth_component_getFactory },
        { "liblwpftlo.a", lotuswordpro_component_getFactory },
        { "libooxlo.a", oox_component_getFactory },
        { "libscdlo.a", scd_component_getFactory },
        { "libscfiltlo.a", scfilt_component_getFactory },
        { "libsclo.a", sc_component_getFactory },
        { "libsddlo.a", sdd_component_getFactory },
        { "libsdlo.a", sd_component_getFactory },
        { "libsmdlo.a", smd_component_getFactory },
        { "libsmlo.a", sm_component_getFactory },
        { "libsvgfilterlo.a", svgfilter_component_getFactory },
        { "libswdlo.a", swd_component_getFactory },
        { "libswlo.a", sw_component_getFactory },
        { "libt602filterlo.a", t602filter_component_getFactory },
        { "libtextfdlo.a", textfd_component_getFactory },
        { "libunordflo.a", unordf_component_getFactory },
        { "libunoxmllo.a", unoxml_component_getFactory },
        { "libxmlfdlo.a", xmlfd_component_getFactory },
        { "libxmlsecurity.a", xmlsecurity_component_getFactory },
        { "libxoflo.a", xof_component_getFactory },
        { "libxolo.a", xo_component_getFactory },
        { NULL, NULL }
    };

    return map;
}

void
lo_initialize(void)
{
    // See unotest/source/cpp/bootstrapfixturebase.cxx
    const char *app_root = [[[NSBundle mainBundle] bundlePath] UTF8String];
    setenv("SRC_ROOT", app_root, 1);
    setenv("OUTDIR_FOR_BUILD", app_root, 1);

    setenv("SAL_LOG", "yes", 1);

    const char *argv[] = {
        "Viewer",
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

    for (unsigned i = 0; i < sizeof(services)/sizeof(services[0]); i++) {
        uno_services = [uno_services stringByAppendingString: @"file://"];
        uno_services = [uno_services stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: [NSString stringWithUTF8String: services[i]]]];
        if (i < sizeof(services)/sizeof(services[0]) - 1)
            uno_services = [uno_services stringByAppendingString: @" "];
    }

    assert(strcmp(argv[argc-1], "placeholder-uno-services") == 0);
    argv[argc-1] = [uno_services UTF8String];

    osl_setCommandArgs(argc, (char **) argv);

    try {

        Reference< XComponentContext > xComponentContext(::cppu::defaultBootstrap_InitialComponentContext());

        Reference< XMultiComponentFactory > xMultiComponentFactoryClient( xComponentContext->getServiceManager() );

        Reference< XInterface > xInterface =
            xMultiComponentFactoryClient->createInstanceWithContext( OUString("com.sun.star.frame.Desktop"),
                                                                     xComponentContext );
    }
    catch (Exception e) {
        SAL_WARN("Viewer", e.Message);
    }
}
