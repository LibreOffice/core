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
#ifndef _SOLTOOLS_TESTSHL_TLOG_HXX__
#define _SOLTOOLS_TESTSHL_TLOG_HXX__

#include    <osl/file.hxx>
#include <rtl/tres.hxx>

#include "tutil.hxx"

#include <iostream>

using namespace std;

namespace tstutl {

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
