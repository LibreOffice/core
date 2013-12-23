#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser

# foo_component_getFactory functions are split into groups, so that you could
# choose e.g. 'extended_core' and 'writer' functionality and through factory_map,
# relevant function sections will be referenced in lo_get_factory_map().
# That prevents garbage collector to ignore them as unused.

# We need the same groups for new constructor functions, started in
# ae3a0c8da50b36db395984637f5ad74d3b4887bc
# For now, there are only constructor functions for implementations in 'core'
# group, so no need for other groups, core_constructor_list is enough.
# (These functions are referenced in lo_get_constructor_map().)

core_factory_list = [
    ("libembobj.a", "embobj_component_getFactory"),
    ("libemboleobj.a", "emboleobj_component_getFactory"),
    ("libintrospectionlo.a", "introspection_component_getFactory"),
    ("libreflectionlo.a", "reflection_component_getFactory"),
    ("libstocserviceslo.a", "stocservices_component_getFactory"),
    ("libcomphelper.a", "comphelp_component_getFactory"),
    ("libconfigmgrlo.a", "configmgr_component_getFactory"),
    ("libdeployment.a", "deployment_component_getFactory"),
    ("libevtattlo.a", "evtatt_component_getFactory"),
    ("libfilterconfiglo.a", "filterconfig1_component_getFactory"),
    ("libfsstoragelo.a", "fsstorage_component_getFactory"),
    ("libfwklo.a", "fwk_component_getFactory"),
    ("libfwllo.a", "fwl_component_getFactory"),
    ("libhyphenlo.a", "hyphen_component_getFactory"),
    ("libi18npoollo.a", "i18npool_component_getFactory"),
    ("liblnglo.a", "lng_component_getFactory"),
    ("liblnthlo.a", "lnth_component_getFactory"),
    ("liblocalebe1lo.a", "localebe1_component_getFactory"),
    ("libooxlo.a", "oox_component_getFactory"),
    ("libpackage2.a", "package2_component_getFactory"),
    ("libsfxlo.a", "sfx_component_getFactory"),
    ("libsotlo.a", "sot_component_getFactory"),
    ("libspelllo.a", "spell_component_getFactory"),
    ("libsvllo.a", "svl_component_getFactory"),
    ("libsvtlo.a", "svt_component_getFactory"),
    ("libsvxlo.a", "svx_component_getFactory"),
    ("libtklo.a", "tk_component_getFactory"),
    ("libucb1.a", "ucb_component_getFactory"),
    ("libucpexpand1lo.a", "ucpexpand1_component_getFactory"),
    ("libucpfile1.a", "ucpfile_component_getFactory"),
    ("libunordflo.a", "unordf_component_getFactory"),
    ("libunoxmllo.a", "unoxml_component_getFactory"),
    ("libutllo.a", "utl_component_getFactory"),
    ("libvcllo.a", "vcl_component_getFactory"),
    ("libxmlsecurity.a", "xmlsecurity_component_getFactory"),
    ("libxolo.a", "xo_component_getFactory"),
    ("libxoflo.a", "xof_component_getFactory"),
    ("libxstor.a", "xstor_component_getFactory"),
    ]

extended_core_factory_list = core_factory_list + [
    ("libanimcorelo.a", "animcore_component_getFactory"),
    ("libavmedialo.a", "avmedia_component_getFactory"),
    ("libchartcorelo.a", "chartcore_component_getFactory"),
    ("libfilterconfiglo.a", "filterconfig1_component_getFactory"),
    ("libfrmlo.a", "frm_component_getFactory"),
    ("libfwklo.a", "fwk_component_getFactory"),
    ("libfwmlo.a", "fwm_component_getFactory"),
    ("libsvxcorelo.a", "svxcore_component_getFactory"),
    ("libtextfdlo.a", "textfd_component_getFactory"),
    ("libtklo.a", "tk_component_getFactory"),
    ("libucppkg1.a", "ucppkg1_component_getFactory"),
    ("libxmlfdlo.a", "xmlfd_component_getFactory"),
    ]

base_core_factory_list = [
    ("libdbalo.a", "dba_component_getFactory"),
    ("libdbaxmllo.a", "dbaxml_component_getFactory"),
    ]

calc_core_factory_list = [
    ("libscdlo.a", "scd_component_getFactory"),
    ("libscfiltlo.a", "scfilt_component_getFactory"),
    ("libsclo.a", "sc_component_getFactory"),
    ]

calc_factory_list = calc_core_factory_list + [
    ("libanalysislo.a", "analysis_component_getFactory"),
    ("libdatelo.a", "date_component_getFactory"),
    ("libpricinglo.a", "pricing_component_getFactory"),
    ]

draw_core_factory_list = [
    ("libsddlo.a", "sdd_component_getFactory"),
    ("libsdlo.a", "sd_component_getFactory"),
    ("libsvgfilterlo.a", "svgfilter_component_getFactory"),
    ("libwpftdrawlo.a", "wpftdraw_component_getFactory"),
    ]

math_factory_list = [
    ("libsmdlo.a", "smd_component_getFactory"),
    ("libsmlo.a", "sm_component_getFactory"),
    ]

writer_core_factory_list = [
    ("libswdlo.a", "swd_component_getFactory"),
    ("libswlo.a", "sw_component_getFactory"),
    ("libwriterfilterlo.a", "writerfilter_component_getFactory"),
    ]

writer_factory_list = writer_core_factory_list + [
    ("libhwplo.a", "hwp_component_getFactory"),
    ("libt602filterlo.a", "t602filter_component_getFactory"),
    ("libwpftwriterlo.a", "wpftwriter_component_getFactory"),
    ]

factory_map = {
    'core' : core_factory_list,
    'extended_core' : extended_core_factory_list,
    'base_core' : base_core_factory_list,
    'calc_core' : calc_core_factory_list,
    'calc' : calc_factory_list,
    'draw_core' : draw_core_factory_list,
    'math' : math_factory_list,
    'writer_core' : writer_core_factory_list,
    'writer' : writer_factory_list,
    }

core_constructor_list = [
# sax/source/expatwrap/expwrap.component
    "com_sun_star_comp_extensions_xml_sax_ParserExpat",
    "com_sun_star_comp_extensions_xml_sax_FastParser",
    "com_sun_star_extensions_xml_sax_Writer",
# stoc/util/bootstrap.component
    "com_sun_star_comp_stoc_DLLComponentLoader",
    "com_sun_star_comp_stoc_ImplementationRegistration",
    "com_sun_star_comp_stoc_NestedRegistry",
    "com_sun_star_comp_stoc_ORegistryServiceManager",
    "com_sun_star_comp_stoc_OServiceManager",
    "com_sun_star_comp_stoc_OServiceManagerWrapper",
    "com_sun_star_comp_stoc_SimpleRegistry",
    "com_sun_star_security_comp_stoc_AccessController",
    "com_sun_star_security_comp_stoc_FilePolicy",
    ]

# Components which are not in any group yet:
single_component_map = {
    'basprov' : ("libbasprovlo.a", "basprov_component_getFactory"),
    'cui' : ("libcuilo.a", "cui_component_getFactory"),
    'dlgprov' : ("libdlgprovlo.a", "dlgprov_component_getFactory"),
    'protocolhandler' : ("libprotocolhandlerlo.a", "protocolhandler_component_getFactory"),
    'scriptframe' : ("libscriptframe.a", "scriptframe_component_getFactory"),
    'sb' : ("libsblo.a", "sb_component_getFactory"),
    'spl' : ("libspllo.a", "spl_component_getFactory"),
    'stringresource' :("libstringresourcelo.a", "stringresource_component_getFactory"),
    'uui' : ("libuuilo.a", "uui_component_getFactory"),
    'vbaswobj' : ("libvbaswobjlo.a", "vbaswobj_component_getFactory"),
    'vbaevents' : ("libvbaeventslo.a", "vbaevents_component_getFactory"),
    }

opts = OptionParser()
opts.add_option("-j", "--java-guard", action="store_true", help="include external java functions", dest="java", default=False)
opts.add_option("-g", "--group", action="append", help="group of implementations to make available in application", dest="groups")
# TODO: components from single_component_map should be put into
# one of the groups too and --single-component should die.
opts.add_option("-s", "--single-component", action="append", help="list of single getFactories to get into lib_to_factory_mapping", dest="components")

(options, args) = opts.parse_args()

print ("""
#include <osl/detail/component-mapping.h>

extern "C" {
""")

if options.groups:
    for factory_group in options.groups:
        for (factory_name,factory_function) in factory_map[factory_group]:
            print ('void * '+factory_function+'( const char* , void* , void* );')

if options.components:
    for c in options.components:
        (c_name, c_function) = single_component_map[c]
        print ('void * '+c_function+'( const char* , void* , void* );')

print ('')
for constructor in core_constructor_list:
    print ('void * '+constructor+'( void *, void * );')

print ("""
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {""")

if options.groups:
    for factory_group in options.groups:
        for (factory_name,factory_function) in factory_map[factory_group]:
            print ('        { "'+factory_name+'", '+factory_function+' },')

if options.components:
    for c in options.components:
        (c_name, c_function) = single_component_map[c]
        print ('        { "'+c_name+'", '+c_function+' },')

print ("""
        { 0, 0 }
    };""")

if options.java:
    print ("""
    // Guard against possible function-level link-time pruning of
    // "unused" code. We need to pull these in, too, as they aren't in
    // any of the libs we link with -Wl,--whole-archive. Is this necessary?
    extern void Java_org_libreoffice_android_AppSupport_runMain();
    volatile void *p = (void *) Java_org_libreoffice_android_AppSupport_runMain;

    extern void Java_org_libreoffice_android_AppSupport_renderVCL();
    p = (void *) Java_org_libreoffice_android_AppSupport_renderVCL;""")

print ("""
    return map;
}

const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {""")
for constructor in core_constructor_list:
    print ('        { "' +constructor+ '", ' +constructor+ ' },')

print ("""
        { 0, 0 }
    };

    return map;
}

}""")
