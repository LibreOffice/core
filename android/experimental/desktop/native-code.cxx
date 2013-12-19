/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "osl/detail/android-bootstrap.h"

extern "C"
__attribute__ ((visibility("default")))
const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {
        LO_EXTENDED_CORE_FACTORY_MAP
        LO_BASE_CORE_FACTORY_MAP
        LO_CALC_CORE_FACTORY_MAP
        LO_DRAW_CORE_FACTORY_MAP
        LO_MATH_FACTORY_MAP
        LO_WRITER_FACTORY_MAP
        { "libbasprovlo.a", basprov_component_getFactory },
        { "libdlgprovlo.a", dlgprov_component_getFactory },
        { "libcuilo.a", cui_component_getFactory },
        { "libprotocolhandlerlo.a", protocolhandler_component_getFactory },
        { "libscriptframe.a", scriptframe_component_getFactory },
        { "libsblo.a", sb_component_getFactory },
        { "libspllo.a", spl_component_getFactory },
        { "libscriptframe.a", scriptframe_component_getFactory },
        { "libstringresourcelo.a", stringresource_component_getFactory },
        { "libuuilo.a", uui_component_getFactory },
        { "libvbaswobjlo.a", vbaswobj_component_getFactory },
        { "libvbaeventslo.a", vbaevents_component_getFactory },
        { NULL, NULL }
    };

    // Guard against possible function-level link-time pruning of
    // "unused" code. We need to pull these in, too, as they aren't in
    // any of the libs we link with -Wl,--whole-archive. Is this necessary?
    extern void Java_org_libreoffice_android_AppSupport_runMain();
    volatile void *p = (void *) Java_org_libreoffice_android_AppSupport_runMain;

    extern void Java_org_libreoffice_android_AppSupport_renderVCL();
    p = (void *) Java_org_libreoffice_android_AppSupport_renderVCL;

    return map;
}

extern "C"
__attribute__ ((visibility("default")))
const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {
        NON_APP_SPECIFIC_CONSTRUCTOR_MAP
        { NULL, NULL }
    };

    return map;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
