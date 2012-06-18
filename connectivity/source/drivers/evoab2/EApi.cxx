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
    static struct {
    const char *sym_name;
    SymbolFunc *ref_value;
    } aApiMap[] = {
    SYM_MAP( e_contact_field_name ),
    SYM_MAP( e_contact_get ),
    SYM_MAP( e_contact_get_type ),
    SYM_MAP( e_contact_field_id ),
    SYM_MAP( e_source_peek_name ),
    SYM_MAP( e_source_get_property ),
    SYM_MAP( e_source_list_peek_groups ),
    SYM_MAP( e_source_group_peek_sources ),
    SYM_MAP( e_book_new ),
    SYM_MAP( e_book_open ),
    SYM_MAP( e_book_get_uri ),
    SYM_MAP( e_book_get_source ),
    SYM_MAP( e_book_get_addressbooks ),
    SYM_MAP( e_book_get_contacts ),
    SYM_MAP( e_book_authenticate_user ),
    SYM_MAP( e_book_query_field_test ),
    SYM_MAP( e_book_query_and ),
    SYM_MAP( e_book_query_or ),
    SYM_MAP( e_book_query_not ),
    SYM_MAP( e_book_query_ref ),
    SYM_MAP( e_book_query_unref ),
    SYM_MAP( e_book_query_from_string ),
    SYM_MAP( e_book_query_to_string ),
    SYM_MAP( e_book_query_field_exists ),
    SYM_MAP( e_source_group_peek_base_uri)
    };
#undef SYM_MAP

static bool
tryLink( oslModule &aModule, const char *pName )
{
    for( guint i = 0; i < G_N_ELEMENTS( aApiMap ); i++ )
    {
    SymbolFunc aMethod;
    aMethod = (SymbolFunc) osl_getFunctionSymbol
        ( aModule, rtl::OUString::createFromAscii ( aApiMap[ i ].sym_name ).pData );
    if( !aMethod )
    {
        fprintf( stderr, "Warning: missing symbol '%s' in '%s'",
             aApiMap[ i ].sym_name, pName );
        return false;
    }
    * aApiMap[ i ].ref_value = aMethod;
    }
    return true;
}

bool EApiInit()
{
    oslModule aModule;

    for( guint j = 0; j < G_N_ELEMENTS( eBookLibNames ); j++ )
    {
        aModule = osl_loadModule( rtl::OUString::createFromAscii
                                  ( eBookLibNames[ j ] ).pData,
                                  SAL_LOADMODULE_DEFAULT );
        if( aModule)
        {
            if ( tryLink( aModule, eBookLibNames[ j ] ) )
                return true;
            osl_unloadModule( aModule );
        }
    }
    fprintf( stderr, "Can find no compliant libebook client libraries\n" );
    return false;
}

#if 0
// hjs: SOLARDEF does no longer exist please lookup the required
// defines in a regular compile line
/*
 * Test code - enable &
 *
 * Compile with ( after source LinuxIntelEnv.Set.sh )
   gcc $SOLARDEF -I $SOLARVER/$UPD/$INPATH/inc \
     -I. `pkg-config --cflags --libs gobject-2.0` \
     -L $SOLARVER/$UPD/$INPATH/lib -luno_sal -lstdc++ EApi.cxx
 */

int main( int argc, char **argv)
{
    return EApiInit();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
