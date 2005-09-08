/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tstMgr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:32:25 $
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
#include "tstMgr.hxx"

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#ifndef _RTL_TRES_HXX_
#include <rtl/tres.hxx>
#endif

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
