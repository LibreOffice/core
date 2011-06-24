/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"

#include <stdio.h>

#include "inc/tlog.hxx"

#include "inc/tstMgr.hxx"

using namespace tstutl;

void usage();
void test_shl( vector< sal_Char* > cmdln, sal_Bool boom );

#if (defined UNX)
int main( int argc, char* argv[] )
#else
int _cdecl main( int argc, char* argv[] )
#endif
{
    if ( argc < 3 ) {
        usage();
    }
    sal_Bool boom = sal_False;
    vector< sal_Char* > cmdln;

    sal_Int32 i;
    for ( i = 1; i < argc; i++ ) {
        sal_Char* ptr = argv[i];
        if ( ptr[0] == '-' ) {
            boom = sal_True;
        }
        else  {
            cmdln.push_back( ptr );
        }
    }
    if ( cmdln.size() < 3 ) {
        cmdln.push_back( 0 );
    }
    if ( ! cmdln[0] || ! cmdln[1] ) {
        usage();
    }

    test_shl( cmdln, boom );

    return(0);
}

void test_shl( vector< sal_Char*> cmdln, sal_Bool boom ) {

    tstMgr tst;

    if ( tst.initialize( cmdln[0], boom )) {
        tst.test_EntriesFromFile( cmdln[1], cmdln[2] );
    }
    else {
        sal_Char* msg = "could not find module\n";
        fprintf( stdout, "%s\n", msg );
    }
}

void usage(){
    fprintf( stdout,
            "USAGE: testSHL shlname scename [logname] [-boom]\n" );
    exit(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
