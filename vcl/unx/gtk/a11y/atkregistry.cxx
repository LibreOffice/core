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

#include "atkregistry.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

static GHashTable *uno_to_gobject = nullptr;

/*****************************************************************************/

AtkObject *
ooo_wrapper_registry_get(const Reference< XAccessible >& rxAccessible)
{
    if( uno_to_gobject )
    {
        gpointer cached =
            g_hash_table_lookup(uno_to_gobject, static_cast<gpointer>(rxAccessible.get()));

        if( cached )
            return ATK_OBJECT( cached );
    }

    return nullptr;
}

/*****************************************************************************/

void
ooo_wrapper_registry_add(const Reference< XAccessible >& rxAccessible, AtkObject *obj)
{
   if( !uno_to_gobject )
        uno_to_gobject = g_hash_table_new (nullptr, nullptr);

   g_hash_table_insert( uno_to_gobject, static_cast<gpointer>(rxAccessible.get()), obj );
}

/*****************************************************************************/

void
ooo_wrapper_registry_remove(
    css::uno::Reference<css::accessibility::XAccessible> const & pAccessible)
{
    if( uno_to_gobject )
        g_hash_table_remove(
            uno_to_gobject, static_cast<gpointer>(pAccessible.get()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
