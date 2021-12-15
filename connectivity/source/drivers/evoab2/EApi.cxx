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
#include <osl/module.hxx>
#define  DECLARE_FN_POINTERS 1
#include "EApi.h"
static const char *eBookLibNames[] = {
    "libebook-1.2.so.20", // evolution-data-server 3.33.2+
    "libebook-1.2.so.19", // evolution-data-server 3.24+
    "libebook-1.2.so.16"
};

typedef void (*SymbolFunc) ();

namespace {

struct ApiMap
{
    const char *sym_name;
    SymbolFunc *ref_value;
};

}

const ApiMap aCommonApiMap[] =
{
    { "eds_check_version", reinterpret_cast<SymbolFunc *>(&eds_check_version) },
    { "e_contact_field_name", reinterpret_cast<SymbolFunc *>(&e_contact_field_name) },
    { "e_contact_get", reinterpret_cast<SymbolFunc *>(&e_contact_get) },
    { "e_contact_get_type", reinterpret_cast<SymbolFunc *>(&e_contact_get_type) },
    { "e_contact_field_id", reinterpret_cast<SymbolFunc *>(&e_contact_field_id) },
    { "e_book_new", reinterpret_cast<SymbolFunc *>(&e_book_new) },
    { "e_book_open", reinterpret_cast<SymbolFunc *>(&e_book_open) },
    { "e_book_get_source", reinterpret_cast<SymbolFunc *>(&e_book_get_source) },
    { "e_book_get_contacts", reinterpret_cast<SymbolFunc *>(&e_book_get_contacts) },
    { "e_book_query_field_test", reinterpret_cast<SymbolFunc *>(&e_book_query_field_test) },
    { "e_book_query_and", reinterpret_cast<SymbolFunc *>(&e_book_query_and) },
    { "e_book_query_or", reinterpret_cast<SymbolFunc *>(&e_book_query_or) },
    { "e_book_query_not", reinterpret_cast<SymbolFunc *>(&e_book_query_not) },
    { "e_book_query_ref", reinterpret_cast<SymbolFunc *>(&e_book_query_ref) },
    { "e_book_query_unref", reinterpret_cast<SymbolFunc *>(&e_book_query_unref) },
    { "e_book_query_from_string", reinterpret_cast<SymbolFunc *>(&e_book_query_from_string) },
    { "e_book_query_to_string", reinterpret_cast<SymbolFunc *>(&e_book_query_to_string) },
    { "e_book_query_field_exists", reinterpret_cast<SymbolFunc *>(&e_book_query_field_exists) }
};

const ApiMap aNewApiMap[] =
{
    { "e_source_registry_list_sources", reinterpret_cast<SymbolFunc *>(&e_source_registry_list_sources) },
    { "e_source_registry_new_sync", reinterpret_cast<SymbolFunc *>(&e_source_registry_new_sync) },
    { "e_source_has_extension", reinterpret_cast<SymbolFunc *>(&e_source_has_extension) },
    { "e_source_get_extension", reinterpret_cast<SymbolFunc *>(&e_source_get_extension) },
    { "e_source_backend_get_backend_name", reinterpret_cast<SymbolFunc *>(&e_source_backend_get_backend_name) },
    { "e_source_get_display_name", reinterpret_cast<SymbolFunc *>(&e_source_get_display_name) },
    { "e_source_get_uid", reinterpret_cast<SymbolFunc *>(&e_source_get_uid) },
    { "e_source_registry_ref_source", reinterpret_cast<SymbolFunc *>(&e_source_registry_ref_source) },
    { "e_client_open_sync", reinterpret_cast<SymbolFunc *>(&e_client_open_sync) },
    { "e_client_get_source", reinterpret_cast<SymbolFunc *>(&e_client_get_source) },
    { "e_book_client_get_contacts_sync", reinterpret_cast<SymbolFunc *>(&e_book_client_get_contacts_sync) },
    { "e_client_util_free_object_slist", reinterpret_cast<SymbolFunc *>(&e_client_util_free_object_slist) }
};

//>= direct read access API (>= 3.8)
const ApiMap aClientApiMap38[] =
{
    { "e_book_client_connect_direct_sync", reinterpret_cast<SymbolFunc *>(&e_book_client_connect_direct_sync) }
};

template<size_t N> static bool
tryLink( osl::Module &rModule, const char *pName, const ApiMap (&pMap)[N])
{
    for (size_t i = 0; i < N; ++i)
    {
        SymbolFunc aMethod = reinterpret_cast<SymbolFunc>(
            rModule.getFunctionSymbol(OUString::createFromAscii(pMap[i].sym_name)));
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
    for( guint j = 0; j < G_N_ELEMENTS( eBookLibNames ); j++ )
    {
        osl::Module aModule(OUString::createFromAscii(eBookLibNames[j]), SAL_LOADMODULE_DEFAULT);

        if (!aModule.is())
            continue;

        if (tryLink( aModule, eBookLibNames[ j ], aCommonApiMap))
        {
            if (tryLink( aModule, eBookLibNames[ j ], aNewApiMap))
            {
                if (tryLink( aModule, eBookLibNames[ j ], aClientApiMap38))
                {
                    aModule.release();
                    return true;
                }
            }
        }
    }
    fprintf( stderr, "Can find no compliant libebook client libraries\n" );
    return false;
}

ESourceRegistry *get_e_source_registry()
{
    static ESourceRegistry *theInstance = e_source_registry_new_sync(nullptr, nullptr);
    return theInstance;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
