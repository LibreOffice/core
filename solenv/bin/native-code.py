#!/usr/bin/python
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import print_function
from optparse import OptionParser

single_component_map = {
        'basprov' : '{ "libbasprovlo.a", basprov_component_getFactory },',
        'cui' : '{ "libcuilo.a", cui_component_getFactory },',
        'dlgprov' : '{ "libdlgprovlo.a", dlgprov_component_getFactory },',
        'protocolhandler' : '{ "libprotocolhandlerlo.a", protocolhandler_component_getFactory },',
        'scriptframe' : '{ "libscriptframe.a", scriptframe_component_getFactory },',
        'sb' : '{ "libsblo.a", sb_component_getFactory },',
        'spl' : '{ "libspllo.a", spl_component_getFactory },',
        'stringresource' :'{ "libstringresourcelo.a", stringresource_component_getFactory },',
        'uui' : '{ "libuuilo.a", uui_component_getFactory },',
        'vbaswobj' : '{ "libvbaswobjlo.a", vbaswobj_component_getFactory },',
        'vbaevents' :'{ "libvbaeventslo.a", vbaevents_component_getFactory },'}

opts = OptionParser()
opts.add_option("-j", "--java-guard", action="store_true", help="include external java functions", dest="java", default=False)
opts.add_option("-f", "--factory", action="append", help="list of factory groups to get into lib_to_factory_mapping", dest="factories")
opts.add_option("-s", "--single-component", action="append", help="list of single getFactories to get into lib_to_factory_mapping", dest="components")
opts.add_option("-c", "--constructor", action="append", help="list of constructor groups to get into lib_to_constructor_mapping", dest="constructors")

(options, args) = opts.parse_args()

print ("""#include "osl/detail/android-bootstrap.h"

extern "C"
__attribute__ ((visibility("default")))
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {""")

if options.factories:
    for f in options.factories:
        print ('        LO_' + f + '_FACTORY_MAP')

if options.components:
    for c in options.components:
        print ('        ' + single_component_map[c])

print ("""
        { NULL, NULL }
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

extern "C"
__attribute__ ((visibility("default")))
const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {
        NON_APP_SPECIFIC_CONSTRUCTOR_MAP""")

if options.constructors:
    for c in options.constructors:
        print (c)

print ("""
        { NULL, NULL }
    };

    return map;
}""")
