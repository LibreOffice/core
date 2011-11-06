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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"

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

