/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: log.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:13:49 $
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
#ifndef __QADEV_REGSCAN_LOG_HXX__
#define __QADEV_REGSCAN_LOG_HXX__

#include <time.h>

#ifndef _RTL_USTRING_HXX_
#include    <rtl/ustring.hxx>
#endif

#ifndef _RTL_STRBUF_HXX_
#include    <rtl/strbuf.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include    <sal/types.h>
#endif

#ifndef _OSL_THREAD_H_
#include    <osl/thread.h>
#endif

#ifndef _OSL_FILE_HXX_
#include    <osl/file.hxx>
#endif

#include <cppunit/nocopy.hxx>

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
class Log : NOCOPY {

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
    virtual ~Log()
        {
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
    //! ::osl::FileBase::RC write( rtl::OStringBuffer& buf,
    //!                                                 sal_Bool v = sal_False );
    ::osl::FileBase::RC write( const rtl::OUString& buf,
                            rtl_TextEncoding enc = RTL_TEXTENCODING_ASCII_US,
                                                 sal_Bool v = sal_False   );

}; ///:~ Log

#endif
