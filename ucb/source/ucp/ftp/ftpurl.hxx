/*************************************************************************
 *
 *  $RCSfile: ftpurl.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:26:48 $
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

#include <stdio.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <vector>

#include "ftpdirp.hxx"
#include "ftpcfunc.hxx"

namespace ftp {

    /** Forward declarations.
     */

    class FTPHandleProvider;


    enum FTPErrors { FILE_EXIST_DURING_INSERT = CURL_LAST +1,
                     FOLDER_EXIST_DURING_INSERT,
                     FOLDER_MIGHT_EXIST_DURING_INSERT,
                     FILE_MIGHT_EXIST_DURING_INSERT };


    class malformed_exception { };


    class curl_exception
    {
    public:

        curl_exception(sal_Int32 err)
            : n_err(err) { }

        sal_Int32 code() const { return n_err; }


    private:

        sal_Int32 n_err;
    };

    class CurlInput {

    public:

        // returns the number of bytes actually read
        virtual sal_Int32 read(sal_Int8 *dest,sal_Int32 nBytesRequested) = 0;
    };


    class FTPURL
    {
    public:

        FTPURL(
            const rtl::OUString& aIdent,
            FTPHandleProvider* pFCP = 0
        )
            throw(
                malformed_exception
            );

        FTPURL(const FTPURL& r);

        ~FTPURL();

        rtl::OUString host() const { return m_aHost; }

        rtl::OUString port() const { return m_aPort; }

        rtl::OUString username() const { return m_aUsername; }

        /** This returns the URL, but cleaned from
         *  unnessary ellipses.
         */

        rtl::OUString ident(bool withslash,bool internal) const;

        /** returns the parent url.
         */

        rtl::OUString parent(bool internal = false) const;

        void child(const rtl::OUString& title);

        rtl::OUString child(void) const;

        std::vector<FTPDirentry> list(sal_Int16 nMode) const
            throw(curl_exception);

        // returns a pointer to an open tempfile,
        // seeked to the beginning of.
        FILE* open() throw(curl_exception);

        FTPDirentry direntry() const throw(curl_exception);

        void insert(bool ReplaceExisting,void* stream) const
            throw(curl_exception);

        void mkdir(bool ReplaceExisting) const
            throw(curl_exception);

        rtl::OUString ren(const rtl::OUString& NewTitle)
            throw(curl_exception);

        void del() const
            throw(curl_exception);


    private:

        osl::Mutex m_mutex;

        FTPHandleProvider *m_pFCP;

        mutable rtl::OUString m_aUsername;
        bool m_bShowPassword;
        mutable rtl::OUString m_aHost;
        mutable rtl::OUString m_aPort;
        mutable rtl::OUString m_aType;

        /** Contains the decoded pathsegments of the url.
         */
        std::vector<rtl::OUString> m_aPathSegmentVec;

        void parse(const rtl::OUString& url)
            throw(
                malformed_exception
            );

        rtl::OUString net_title() const throw(curl_exception);
    };

}


#endif
