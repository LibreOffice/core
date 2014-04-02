/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unx/gtk/atkbridge.hxx>
#include <unx/gtk/gtkframe.hxx>

#include "atkfactory.hxx"
#include "atkutil.hxx"
#include "atkwindow.hxx"
#include <stdio.h>

bool InitAtkBridge(void)
{
#if !GTK_CHECK_VERSION(3,0,0)
    const char* pVersion = atk_get_toolkit_version();
    if( ! pVersion )
        return false;

    unsigned int major, minor, micro;

    /* check gail minimum version requirements */
    if( sscanf( pVersion, "%u.%u.%u", &major, &minor, &micro) < 3 )
    {
        // g_warning( "unable to parse gail version number" );
        return false;
    }

    if( ( (major << 16) | (minor << 8) | micro ) < ( (1 << 16) | 8 << 8 | 6 ) )
    {
        g_warning( "libgail >= 1.8.6 required for accessibility support" );
        return false;
    }

    /* Initialize the AtkUtilityWrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_ATK_UTIL ) );

    /* Initialize the GailWindow wrapper class */
    g_type_class_unref( g_type_class_ref( OOO_TYPE_WINDOW_WRAPPER ) );

    /* Register AtkObject wrapper factory */
    AtkRegistry * registry = atk_get_default_registry();
    if( registry )
        atk_registry_set_factory_type( registry, OOO_TYPE_FIXED, OOO_TYPE_WRAPPER_FACTORY );
#endif

    return true;
}

void DeInitAtkBridge()
{
#if !GTK_CHECK_VERSION(3,0,0)
    restore_gail_window_vtable();
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
