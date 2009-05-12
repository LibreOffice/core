/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: atkregistry.cxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "atkregistry.hxx"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

static GHashTable *uno_to_gobject = NULL;

/*****************************************************************************/

AtkObject *
ooo_wrapper_registry_get(const Reference< XAccessible >& rxAccessible)
{
    if( uno_to_gobject )
    {
        gpointer cached =
            g_hash_table_lookup(uno_to_gobject, (gpointer) rxAccessible.get());

        if( cached )
            return ATK_OBJECT( cached );
    }

    return NULL;
}

/*****************************************************************************/

void
ooo_wrapper_registry_add(const Reference< XAccessible >& rxAccessible, AtkObject *obj)
{
   if( !uno_to_gobject )
        uno_to_gobject = g_hash_table_new (NULL, NULL);

   g_hash_table_insert( uno_to_gobject, (gpointer) rxAccessible.get(), obj );
}

/*****************************************************************************/

void
ooo_wrapper_registry_remove(XAccessible *pAccessible)
{
    if( uno_to_gobject )
        g_hash_table_remove( uno_to_gobject, (gpointer) pAccessible );
}

