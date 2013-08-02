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
#include <rtl/ustring.hxx>
#include <osl/module.h>
#include <stdio.h>
#define  DECLARE_FN_POINTERS 1
#include "EApi.h"
static const char *eBookLibNames[] = {
    "libebook-1.2.so.14", // bumped again (evolution-3.6)
    "libebook-1.2.so.13", // bumped again (evolution-3.4)
    "libebook-1.2.so.12", // bumped again
    "libebook-1.2.so.10", // bumped again
    "libebook-1.2.so.9",  // evolution-2.8
    "libebook-1.2.so.5",  // evolution-2.4 and 2.6+
    "libebook-1.2.so.3",  // evolution-2.2
    "libebook.so.8"       // evolution-2.0
};

typedef void (*SymbolFunc) (void);

#define SYM_MAP(a) { #a, (SymbolFunc *)&a }
struct ApiMap
{
    const char *sym_name;
    SymbolFunc *ref_value;
};

static const ApiMap aCommonApiMap[] =
{
    SYM_MAP( eds_check_version ),
    SYM_MAP( e_contact_field_name ),
    SYM_MAP( e_contact_get ),
    SYM_MAP( e_contact_get_type ),
    SYM_MAP( e_contact_field_id ),
    SYM_MAP( e_book_new ),
    SYM_MAP( e_book_open ),
    SYM_MAP( e_book_get_source ),
    SYM_MAP( e_book_get_contacts ),
    SYM_MAP( e_book_query_field_test ),
    SYM_MAP( e_book_query_and ),
    SYM_MAP( e_book_query_or ),
    SYM_MAP( e_book_query_not ),
    SYM_MAP( e_book_query_ref ),
    SYM_MAP( e_book_query_unref ),
    SYM_MAP( e_book_query_from_string ),
    SYM_MAP( e_book_query_to_string ),
    SYM_MAP( e_book_query_field_exists )
};

//< 3.6 api
static const ApiMap aOldApiMap[] =
{
    SYM_MAP( e_book_get_addressbooks ),
    SYM_MAP( e_book_get_uri ),
    SYM_MAP( e_book_authenticate_user ),
    SYM_MAP( e_source_group_peek_base_uri),
    SYM_MAP( e_source_peek_name ),
    SYM_MAP( e_source_get_property ),
    SYM_MAP( e_source_list_peek_groups ),
    SYM_MAP( e_source_group_peek_sources )
};

//>= 3.6 api
static const ApiMap aNewApiMap[] =
{
    SYM_MAP( e_source_registry_list_sources ),
    SYM_MAP( e_source_registry_new_sync ),
    SYM_MAP( e_source_has_extension ),
    SYM_MAP( e_source_get_extension ),
    SYM_MAP( e_source_backend_get_backend_name ),
    SYM_MAP( e_source_get_display_name ),
    SYM_MAP( e_source_get_uid ),
    SYM_MAP( e_source_registry_ref_source),
    SYM_MAP( e_client_open_sync ),
    SYM_MAP( e_client_get_source ),
    SYM_MAP( e_book_client_get_contacts_sync ),
    SYM_MAP( e_client_util_free_object_slist )
};

//== indirect read access (3.6 only)
static const ApiMap aClientApiMap36[] =
{
    SYM_MAP( e_book_client_new )
};

//>= direct read access API (>= 3.8)
static const ApiMap aClientApiMap38[] =
{
    SYM_MAP( e_book_client_connect_direct_sync )
};

#undef SYM_MAP

template<size_t N> static bool
tryLink( oslModule &aModule, const char *pName, const ApiMap (&pMap)[N])
{
    for (guint i = 0; i < N; ++i)
    {
        SymbolFunc aMethod = (SymbolFunc)osl_getFunctionSymbol
            (aModule, OUString::createFromAscii ( pMap[ i ].sym_name ).pData);
        if( !aMethod )
        {
            fprintf( stderr, "Warning: missing symbol '%s' in '%s'\n",
                 pMap[ i ].sym_name, pName );
            return false;
        }
        *pMap[ i ].ref_value = aMethod;
    }
    return true;
}

bool EApiInit()
{
    oslModule aModule;

    for( guint j = 0; j < G_N_ELEMENTS( eBookLibNames ); j++ )
    {
        aModule = osl_loadModule( OUString::createFromAscii
                                  ( eBookLibNames[ j ] ).pData,
                                  SAL_LOADMODULE_DEFAULT );

        if( aModule == NULL)
            continue;

        if (tryLink( aModule, eBookLibNames[ j ], aCommonApiMap))
        {
            if (eds_check_version( 3, 6, 0 ) != NULL)
            {
                if (tryLink( aModule, eBookLibNames[ j ], aOldApiMap))
                    return true;
            }
            else if (tryLink( aModule, eBookLibNames[ j ], aNewApiMap))
            {
                if (eds_check_version( 3, 7, 6 ) != NULL)
                {
                    if (tryLink( aModule, eBookLibNames[ j ], aClientApiMap36))
                        return true;
                }
                else
                {
                    if (tryLink( aModule, eBookLibNames[ j ], aClientApiMap38))
                        return true;
                }
            }
        }

        osl_unloadModule( aModule );
    }
    fprintf( stderr, "Can find no compliant libebook client libraries\n" );
    return false;
}

ESourceRegistry *get_e_source_registry()
{
    static ESourceRegistry *theInstance;
    if (!theInstance)
        theInstance = e_source_registry_new_sync(NULL, NULL);
    return theInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
