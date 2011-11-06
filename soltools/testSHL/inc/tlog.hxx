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


#ifndef _SOLTOOLS_TESTSHL_TLOG_HXX__
#define _SOLTOOLS_TESTSHL_TLOG_HXX__

#include    <osl/file.hxx>
#include <rtl/tres.hxx>

#ifndef _SOLTOOLS_TESTSHL_TUTIL_HXX_
#include "tutil.hxx"
#endif

#include <iostream>

using namespace std;

// <namespace_tstutl>
namespace tstutl {

// <class_tLog>
class tLog {

    // <private_members>
    ::osl::File*    m_logfile;              // fileobject
    ::rtl::OUString m_logname;              // name of log
    // </private_members>

    // <private_methods>
    void initialize( const ::rtl::OString& name );
    // </private_methods>

public:

    // <public_ctors>
    tLog() : m_logfile( 0 ) {
    }

    tLog( const sal_Char* name ) {
        if( name ) {
            initialize( name );
        }
        else  {
            m_logfile = 0;
        }

    }
    // </public_ctors>

    // <dtor>
    virtual ~tLog() {
        if ( m_logfile ) {
            m_logfile->close();
            delete( m_logfile );
        }
    } // </dtor>

    // <public_methods>
    inline ::rtl::OUString& getName() { return m_logname; }
    inline ::osl::File* getFile() { return m_logfile; }

    // open logfile for overwrite (default) or append
    ::osl::FileBase::RC open( sal_Bool append = sal_False );
    ::osl::FileBase::RC close();

    ::osl::FileBase::RC writeRes( ::rtl::TestResult& oRes, sal_Bool v = sal_False ,
                                                  sal_Bool xml = sal_False );

    // write methods without (default) or with echo on display
    ::osl::FileBase::RC write( const sal_Char* buf, sal_Bool v = sal_False );
    // </public_methods>

}; // </class_tLog>

} // </namespace_tstutl>

#endif
