/*************************************************************************
 *
 *  $RCSfile: ftpurl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2002-08-28 07:23:17 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _FTP_FTPURL_HXX_
#define _FTP_FTPURL_HXX_

#include <curl/curl.h>
#include <curl/easy.h>
#include <com/sun/star/io/XOutputStream.hpp>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <vector>

#include "ftpdirp.hxx"
#include "ftpcfunc.hxx"

namespace ftp {

    /** Forward declarations.
     */

    class FTPContentProvider;



    enum FTPErrorCode {
        FTPCouldNotDetermineSystem = CURL_LAST + 1
    };


    class malformed_exception { };


    class no_such_directory_exception
    {
    public:

        no_such_directory_exception(sal_Int32 err)
            : n_err(err) { }

        sal_Int32 code() const { return n_err; }


    private:

        sal_Int32 n_err;
    };


    class FTPURL
    {
    public:

        FTPURL(
            const rtl::OUString& aIdent,
            FTPContentProvider* pFCP = 0
        )
            throw(
                malformed_exception
            );


        ~FTPURL();

        rtl::OUString username() const { return m_aUsername; }

        rtl::OUString password() const { return m_aPassword; }

        rtl::OUString host() const { return m_aHost; }

        sal_Int32 port() const { return m_nPort; }

        /** This returns the URL, but cleaned from
         *  unnessary ellipses.
         */

        rtl::OUString ident() const;


        std::vector<FTPDirentry> list(
            sal_Int16 nMode
        ) const
            throw(
                no_such_directory_exception
            );

        FTPDirentry direntry(
            const rtl::OUString& passwd = rtl::OUString()
        ) const
            throw(
                no_such_directory_exception
            );


    private:

        osl::Mutex m_mutex;

        FTPContentProvider *m_pFCP;

        rtl::OUString m_aIdent;


        rtl::OUString m_aUsername;
        rtl::OUString m_aPassword;
        rtl::OUString m_aHost;
        sal_Int32    m_nPort;

        /** Contains the decoded pathsegments of the url.
         */
        std::vector<rtl::OUString> m_aPathSegmentVec;

        void parse()
            throw(
                malformed_exception
            );

    };

}


#endif
