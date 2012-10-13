/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _FTP_FTPURL_HXX_
#define _FTP_FTPURL_HXX_

#include "curl.hxx"
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

    protected:
        ~CurlInput() {}
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

        /** sets the unencoded title */
        void child(const rtl::OUString& title);

        /** returns the unencoded title */
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

        /** Contains the encoded pathsegments of the url.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
