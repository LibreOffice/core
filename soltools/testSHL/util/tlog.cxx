/*************************************************************************
 *
 *  $RCSfile: tlog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ganaya $ $Date: 2001-05-04 04:30:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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





