/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

