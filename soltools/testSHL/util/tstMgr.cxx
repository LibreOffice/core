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
#include "tstMgr.hxx"
#include <osl/module.hxx>
#include <rtl/tres.hxx>

#ifndef _SOLTOOLS_TESTSHL_TLOG_HXX_
#include "tlog.hxx"
#endif

#ifndef _SOLTOOLS_TESTSHL_TUTIL_HXX_
#include "tutil.hxx"
#endif

using namespace rtl;

// <namespace_tstutl>
namespace tstutl {

typedef void* ( tstFunc )( TestResult* );
void test_Entry_Impl( ::osl::Module& oMod, TestResult* oRes );

// <private_members>
struct tstMgr::tstMgr_Impl {
    ::osl::Module m_tstmodule;
    sal_Bool m_boom;
};
// </private_members>

// <method_initialize>
sal_Bool tstMgr::initialize( sal_Char* modName, sal_Bool boom ) {

    ::rtl::OUString tstMod( ::rtl::OUString::createFromAscii( modName ) );
    pImpl = new tstMgr_Impl;
    pImpl->m_boom = boom;
    return ( pImpl->m_tstmodule.load( tstMod ) );
} // <method_initialize>

// <method_test_Entries>
sal_Bool tstMgr::test_Entries( vector< sal_Char* > entries,
                                                        sal_Char* logName ) {

    sal_Bool bOK = sal_False;
    if ( ! entries.empty() ) {

        bOK = sal_True;
        vector< sal_Char* >::iterator iter = entries.begin();

        tLog log( logName );
        // open testLog
        log.open();
            while ( iter != entries.end() ) {
                if ( *iter[0] != '#' ) {
                    ::rtl::TestResult oRes( *iter, pImpl->m_boom );
                    test_Entry_Impl( pImpl->m_tstmodule, &oRes );
                    bOK &= oRes.getState();
                    log.writeRes( oRes );
                }
                iter++;
            }
        log.close();
    }
    return ( bOK );
} // </method_test_Entries>

// <method_test_Entry>
sal_Bool tstMgr::test_Entry( sal_Char* entry, sal_Char* logName ) {

    tLog log( logName );
    // open testLog
    log.open();
    ::rtl::TestResult oRes( entry, pImpl->m_boom );
    test_Entry_Impl( pImpl->m_tstmodule, &oRes );
    log.writeRes( oRes, sal_True );
    log.close();
    return ( oRes.getState() );
} // </method_test_Entry>

// <method_test_EntriesFromFile>
sal_Bool tstMgr::test_EntriesFromFile( sal_Char* fName, sal_Char* logName ) {

    sal_Bool bOK = sal_False;
    vector< sal_Char* > entries;

    if ( getEntriesFromFile( fName, entries ) ) {
        sal_Bool bOK = test_Entries( entries, logName );

        vector< sal_Char* >::iterator iter =  entries.begin();
        while ( iter != entries.end() ) {
            if ( *iter ) {
                delete [] *iter;
            }
            iter++;
        }
    }
    else  {
        bOK = test_Entry( fName );
    }
    return ( bOK );

} // </method_test_EntriesFromFile>

// <method_cleanup>
void tstMgr::cleanup() {
    if ( pImpl ) {
        delete pImpl;
    }
} // </method_cleanup>


// <function_test_Entry_Impl>
void test_Entry_Impl( ::osl::Module& oMod, ::rtl::TestResult* oRes ) {

    tstFunc* pFunc;                             // entry pointer
    ::rtl::OString entryName( "test_" );        // entryname prefix

    // prefix entryname
    entryName += oRes->getName();

    // get entry pointer
    pFunc = (tstFunc*) oMod.getSymbol(
                    ::rtl::OUString::createFromAscii( entryName.getStr() ) );

    if ( pFunc ) {
        // call entry
        pFunc( oRes );
        oRes->end();
    }
    else  {
        oRes->end("symbol not found");
    }
    // return
    return;

} // </function_test_Entry_Impl>

} // </namespace_tstutl>
