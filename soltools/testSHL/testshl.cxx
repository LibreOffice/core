/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testshl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:30:51 $
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

#include <stdio.h>

#ifndef _SOLTOOLS_TESTSHL_TLOG_HXX_
#include "inc/tlog.hxx"
#endif

#ifndef _SOLTOOLS_TESTSHL_TSTMGR_HXX_
#include "inc/tstMgr.hxx"
#endif

using namespace tstutl;

void usage();
void test_shl( vector< sal_Char* > cmdln, sal_Bool boom );

#if (defined UNX) || (defined OS2)
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

