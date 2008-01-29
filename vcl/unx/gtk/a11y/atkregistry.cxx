/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: atkregistry.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 16:19:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

