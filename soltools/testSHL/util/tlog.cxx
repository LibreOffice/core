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

#include    "tlog.hxx"

using namespace std;

// <namespace_tstutl>
namespace tstutl {

// <method_initialize>
void tLog::initialize( const ::rtl::OString& name ) {
    m_logname = cnvrtPth( name );
    m_logfile = new ::osl::File( m_logname );
} // </method_initialize>

// <method_open>
::osl::FileBase::RC tLog::open( sal_Bool append ) {

    if ( m_logfile ) {
        ::osl::FileBase::RC ret;

        if ( ! append ) {
            ret = ::osl::File::remove( m_logname );
        }

    if( m_logfile->open( OpenFlag_Write ) == ::osl::FileBase::E_NOENT ) {
            ret = m_logfile->open( OpenFlag_Write | OpenFlag_Create );
        }
        else  {
            ret = m_logfile->setPos( Pos_End, 0 );
        }
        return ret;
    }
    return ( ::osl::FileBase::E_INVAL );
} // </method_open>

// <method_close>
::osl::FileBase::RC tLog::close() {
    if ( m_logfile ) {
        return m_logfile->close();
    }
    return ( ::osl::FileBase::E_INVAL );
} // </method_close>

// <method_writeRes>
::osl::FileBase::RC tLog::writeRes( ::rtl::TestResult& oRes, sal_Bool v, sal_Bool xml ) {
    ::osl::FileBase::RC ret;

    sal_Char* ptr = oRes.getName();
    ptr = cat( ptr, ";" );
    ptr = cat( ptr, oRes.getResult() );
    ret = write( cat( ptr, "\n" ), v );
    delete [] ptr;

    return( ret );
} // </method_writeRes>

// <method_write>
::osl::FileBase::RC tLog::write( const sal_Char* buf, sal_Bool v ) {

    if ( ! m_logfile ) {
        fprintf( stderr, "%s", buf );
        return ( ::osl::FileBase::E_NOENT );
    }
    sal_uInt64 uBytes=0;
    sal_uInt32 len = ln( buf );
    const sal_Char* ptr = buf;

    if ( v ) {
        fprintf( stderr, "%s", buf );
    }
    return m_logfile->write( buf, len , uBytes );
} // </method_write>

} // </namespace_tstutl>





