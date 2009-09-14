/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 *************************************************************************

      Source Code Control System - Header

      $Header$

      Source Code Control System - Update

*************************************************************************/
#ifndef __QADEV_REGSCAN_LOG_HXX__
#define __QADEV_REGSCAN_LOG_HXX__

#include <time.h>
#include    <rtl/ustring.hxx>
#include    <rtl/strbuf.hxx>
#include    <sal/types.h>
#include    <osl/thread.h>
#include    <osl/file.hxx>

//!io #include <iostream>
#include <vector>

// using namespace std;


/**
 * Log derives the interface of the ::osl::File class ( is-a relation ).
 * Its members (has-a relation) are the (full qualified)name of the log
 * and an OStringBuffer which represents the content of the logfile.
 * It provides the functionality of easy-to-use open and write logs
 */
//: Log
class Log {

    ::osl::File*            m_logfile;          // fileobject
    rtl::OUString         m_logurl;           // url of log
    rtl::OStringBuffer  m_buf;              // content of log

    Log();

public:

    //> c'tor
    /**
     * constructors argument is a full qualified UNC path
     * @param OUString logfile ( full qualified UNC path )
     */
    Log( const rtl::OUString& logURL )
            : m_logfile( new ::osl::File( logURL ))
            , m_logurl(logURL)
    {} ///< c'tor

    //> d'tor
    virtual ~Log() {
        m_logfile->close();
        delete( m_logfile );
    } ///< d'tor

    //> inline methods
    // returns a reference to name instance
    inline rtl::OUString getLogURL() { return m_logurl; }
    inline rtl::OString getName() { return rtl::OUStringToOString(
                                    m_logurl, RTL_TEXTENCODING_ASCII_US  ); }
    ///< inline methods

    // open logfile for overwrite (default) or append
    ::osl::FileBase::RC open( sal_Bool append = sal_False );
    ::osl::FileBase::RC close() { return m_logfile->close(); }


    // write methods without (default) or with echo on display
    ::osl::FileBase::RC write( const sal_Char* buf, sal_Bool v = sal_False );
    ::osl::FileBase::RC write( const rtl::OString& buf,
                                   sal_Bool v = sal_False );
    // ::osl::FileBase::RC write( rtl::OStringBuffer const& buf,
    //                                sal_Bool v = sal_False );
    ::osl::FileBase::RC write( const rtl::OUString& buf,
                                   rtl_TextEncoding enc = RTL_TEXTENCODING_ASCII_US,
                                   sal_Bool v = sal_False   );

}; ///:~ Log

Log &operator <<( Log &_aLog, const sal_Char * _sValue );
Log &operator <<( Log &_aLog, rtl::OString const& _sValue );
Log &operator <<( Log &_aLog, rtl::OUString const& _sValue );
// Log &operator <<( Log &_aLog, rtl::OStringBuffer const& _sValue );

#endif
