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
        { "libprotocolhandlerlo.a", protocolhandler_component_getFactory },
        { "libsblo.a", sb_component_getFactory },
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
