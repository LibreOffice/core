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

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/detail/ios-bootstrap.h>
#include <osl/process.h>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkitExperimental.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/view/XRenderable.hpp>

#include <vcl/svapp.hxx>

using namespace com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringToOString;

#define SMALLSIZE 100

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
    extern void * svt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svx_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sw_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * swd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * t602filter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * textfd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unoxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unordf_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * uui_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
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
        { "fsstorage.uno.a", fsstorage_component_getFactory },
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
        { "libspelllo.a", spell_component_getFactory },
        { "libsvgfilterlo.a", svgfilter_component_getFactory },
        { "libsvtlo.a", svt_component_getFactory },
        { "libsvxlo.a", svx_component_getFactory },
        { "libswdlo.a", swd_component_getFactory },
        { "libswlo.a", sw_component_getFactory },
        { "libt602filterlo.a", t602filter_component_getFactory },
        { "libtextfdlo.a", textfd_component_getFactory },
        { "libunordflo.a", unordf_component_getFactory },
        { "libunoxmllo.a", unoxml_component_getFactory },
        { "libuuilo.a", uui_component_getFactory },
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
    setenv("SAL_LOG", "+WARN+INFO", 1);

    const char *argv[] = {
        "placeholder-exe",
        "-env:URE_INTERNAL_LIB_DIR=file:///",
        "placeholder-uno-types",
        "placeholder-uno-services",
        "placeholder-document"
    };

    const int argc = sizeof(argv)/sizeof(*argv);

    argv[0] = [[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"Viewer"] UTF8String];

    NSString *app_root_escaped = [[[NSBundle mainBundle] bundlePath] stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];

    NSString *uno_types = @"-env:UNO_TYPES=";

    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"ure/types.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"types.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"offapi.rdb"]];

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
    argv[4] = [[app_root_escaped stringByAppendingPathComponent: @"test1.odt"] UTF8String];

    osl_setCommandArgs(argc, (char **) argv);
}
