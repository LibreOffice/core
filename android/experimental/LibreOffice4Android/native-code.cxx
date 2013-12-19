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
        LO_CORE_FACTORY_MAP
        LO_WRITER_FACTORY_MAP
        { "libanimcorelo.a", animcore_component_getFactory },
        { "libavmedialo.a", avmedia_component_getFactory },
        { "libbasprov.uno.a", basprov_component_getFactory },
        { "libchartcorelo.a", chartcore_component_getFactory },
        { "libdlgprov.uno.a", dlgprov_component_getFactory },
        { "libdbalo.a", dba_component_getFactory },
        { "libdbaxmllo.a", dbaxml_component_getFactory },
        { "libfilterconfiglo.a", filterconfig1_component_getFactory },
        { "libfrmlo.a", frm_component_getFactory },
        { "libfwklo.a", fwk_component_getFactory },
        { "libfwmlo.a", fwm_component_getFactory },
        { "libhwplo.a", hwp_component_getFactory },
        { "libprotocolhandlerlo.a", protocolhandler_component_getFactory },
        { "libscdlo.a", scd_component_getFactory },
        { "libscfiltlo.a", scfilt_component_getFactory },
        { "libscriptframe.a", scriptframe_component_getFactory },
        { "libsblo.a", sb_component_getFactory },
        { "libsclo.a", sc_component_getFactory },
        { "libsddlo.a", sdd_component_getFactory },
        { "libsdlo.a", sd_component_getFactory },
        { "libsmdlo.a", smd_component_getFactory },
        { "libsmlo.a", sm_component_getFactory },
        { "libscriptframe.a", scriptframe_component_getFactory },
        { "libstringresource.uno.a", stringresource_component_getFactory },
        { "libsvgfilterlo.a", svgfilter_component_getFactory },
        { "libsvxcorelo.a", svxcore_component_getFactory },
        { "libt602filterlo.a", t602filter_component_getFactory },
        { "libtextfdlo.a", textfd_component_getFactory },
        { "libtklo.a", tk_component_getFactory },
        { "libucppkg1.a", ucppkg1_component_getFactory },
        { "libvbaswobj.uno.a", vbaswobj_component_getFactory },
        { "libvbaevents.uno.a", vbaevents_component_getFactory },
        { "libwpftdrawlo.a", wpftdraw_component_getFactory },
        { "libwpftwriterlo.a", wpftwriter_component_getFactory },
        { "libxmlfdlo.a", xmlfd_component_getFactory },
        { NULL, NULL }
    };

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
