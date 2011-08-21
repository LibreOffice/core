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

    if( m_logfile->open( osl_File_OpenFlag_Write ) == ::osl::FileBase::E_NOENT ) {
            ret = m_logfile->open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
        }
        else  {
            ret = m_logfile->setPos( osl_Pos_End, 0 );
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

    if ( v ) {
        fprintf( stderr, "%s", buf );
    }
    return m_logfile->write( buf, len , uBytes );
} // </method_write>

} // </namespace_tstutl>





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
