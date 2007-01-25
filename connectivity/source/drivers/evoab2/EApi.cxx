 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EApi.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 13:46:54 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include <rtl/ustring.hxx>
#include <osl/module.h>
#include <stdio.h>
#define  DECLARE_FN_POINTERS 1
#ifndef _CONNECTIVITY_EVOAB_EVOLUTION_API_HXX_
#include "EApi.h"
#endif
static char *eBookLibNames[] = {
    "libebook-1.2.so.9", // evolution-2.8
    "libebook-1.2.so.5", // evolution-2.4 and 2.6+
    "libebook-1.2.so.3", // evolution-2.2
    "libebook.so.8"      // evolution-2.0
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
    aMethod = (SymbolFunc) osl_getSymbol
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

