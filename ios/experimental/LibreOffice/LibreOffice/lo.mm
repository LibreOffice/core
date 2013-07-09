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

#include <osl/detail/ios-bootstrap.h>
#include <osl/process.h>

extern "C" {
    extern void * analysis_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * animcore_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * avmedia_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * chartcore_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * cui_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * date_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * dba_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * dbaxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * embobj_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * emboleobj_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * evtatt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * expwrap_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fastsax_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fileacc_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * filterconfig1_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * frm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fsstorage_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwk_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwl_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * fwm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * hwp_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * hyphen_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * lng_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * lnth_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * oox_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * pricing_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sc_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * scd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * scfilt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sdd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sm_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * smd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sot_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * spell_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * spl_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svgfilter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svt_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svx_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * svxcore_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * sw_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * swd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * t602filter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * textfd_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * tk_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * ucppkg1_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unordf_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * unoxml_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * uui_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * wpftdraw_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * wpftwriter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
    extern void * writerfilter_component_getFactory( const char * pImplName, void * pServiceManager, void * pRegistryKey );
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
        { "libanalysislo.a", analysis_component_getFactory },
        { "libanimcorelo.a", animcore_component_getFactory },
        { "libavmedialo.a", avmedia_component_getFactory },
        { "libchartcorelo.a", chartcore_component_getFactory },
        { "libcuilo.a", cui_component_getFactory },
        { "libdatelo.a", date_component_getFactory },
        { "libdbalo.a", dba_component_getFactory },
        { "libdbaxmllo.a", dbaxml_component_getFactory },
        { "libembobj.a", embobj_component_getFactory },
        { "libemboleobj.a", emboleobj_component_getFactory },
        { "libevtattlo.a", evtatt_component_getFactory },
        { "libexpwraplo.a", expwrap_component_getFactory },
        { "libfastsaxlo.a", fastsax_component_getFactory },
        { "libfileacc.a", fileacc_component_getFactory },
        { "libfilterconfiglo.a", filterconfig1_component_getFactory },
        { "libfrmlo.a", frm_component_getFactory },
        { "libfsstoragelo.a", fsstorage_component_getFactory },
        { "libfwklo.a", fwk_component_getFactory },
        { "libfwllo.a", fwl_component_getFactory },
        { "libfwmlo.a", fwm_component_getFactory },
        { "libhwplo.a", hwp_component_getFactory },
        { "libhyphenlo.a", hyphen_component_getFactory },
        { "liblnglo.a", lng_component_getFactory },
        { "liblnthlo.a", lnth_component_getFactory },
        { "libooxlo.a", oox_component_getFactory },
        { "libpricinglo.a", pricing_component_getFactory },
        { "libscdlo.a", scd_component_getFactory },
        { "libscfiltlo.a", scfilt_component_getFactory },
        { "libsclo.a", sc_component_getFactory },
        { "libsddlo.a", sdd_component_getFactory },
        { "libsdlo.a", sd_component_getFactory },
        { "libsmdlo.a", smd_component_getFactory },
        { "libsmlo.a", sm_component_getFactory },
        { "libsotlo.a", sot_component_getFactory },
        { "libspelllo.a", spell_component_getFactory },
        { "libspllo.a", spl_component_getFactory },
        { "libsvgfilterlo.a", svgfilter_component_getFactory },
        { "libsvtlo.a", svt_component_getFactory },
        { "libsvxcorelo.a", svxcore_component_getFactory },
        { "libsvxlo.a", svx_component_getFactory },
        { "libswdlo.a", swd_component_getFactory },
        { "libswlo.a", sw_component_getFactory },
        { "libt602filterlo.a", t602filter_component_getFactory },
        { "libtextfdlo.a", textfd_component_getFactory },
        { "libtklo.a", tk_component_getFactory },
        { "libucppkg1.a", ucppkg1_component_getFactory },
        { "libunordflo.a", unordf_component_getFactory },
        { "libunoxmllo.a", unoxml_component_getFactory },
        { "libuuilo.a", uui_component_getFactory },
        { "libwpftdrawlo.a", wpftdraw_component_getFactory },
        { "libwpftwriterlo.a", wpftwriter_component_getFactory },
        { "libwriterfilterlo.a", writerfilter_component_getFactory },
        { "libxmlfdlo.a", xmlfd_component_getFactory },
        { "libxmlsecurity.a", xmlsecurity_component_getFactory },
        { "libxoflo.a", xof_component_getFactory },
        { "libxolo.a", xo_component_getFactory },
        { NULL, NULL }
    };

    return map;
}

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

    argv[0] = [[[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"Viewer"] UTF8String];

    NSString *app_root_escaped = [[[NSBundle mainBundle] bundlePath] stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];

    NSString *uno_types = @"-env:UNO_TYPES=";

    uno_types = [uno_types stringByAppendingString: @"file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"offapi.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"oovbaapi.rdb"]];

    uno_types = [uno_types stringByAppendingString: @" file://"];
    uno_types = [uno_types stringByAppendingString: [app_root_escaped stringByAppendingPathComponent: @"udkapi.rdb"]];

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
