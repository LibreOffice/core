/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tlog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:31:07 $
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
#ifndef _SOLTOOLS_TESTSHL_TLOG_HXX__
#define _SOLTOOLS_TESTSHL_TLOG_HXX__

#ifndef _OSL_FILE_HXX_
#include    <osl/file.hxx>
#endif

#ifndef _RTL_TRES_HXX_
#include <rtl/tres.hxx>
#endif

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
