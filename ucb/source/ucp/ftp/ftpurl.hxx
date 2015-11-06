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

#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPURL_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPURL_HXX

#include "curl.hxx"
#include <curl/easy.h>
#include <com/sun/star/io/XOutputStream.hpp>

#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <osl/file.h>
#include <vector>

#include "ftpdirp.hxx"
#include "ftpcfunc.hxx"

namespace ftp {

    /** Forward declarations.
     */

    class FTPContentProvider;


    enum FTPErrors { FILE_EXIST_DURING_INSERT = CURL_LAST +1,
                     FOLDER_EXIST_DURING_INSERT,
                     FOLDER_MIGHT_EXIST_DURING_INSERT,
                     FILE_MIGHT_EXIST_DURING_INSERT };

    class malformed_exception : public std::exception { };

    class curl_exception : public std::exception
    {
    public:

        explicit curl_exception(sal_Int32 err)
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
            const OUString& aIdent,
            FTPContentProvider* pFCP = 0
        )
            throw(
                malformed_exception
            );

        FTPURL(const FTPURL& r);

        ~FTPURL();

        OUString host() const { return m_aHost; }

        OUString port() const { return m_aPort; }

        OUString username() const { return m_aUsername; }

        /** This returns the URL, but cleaned from
         *  unnessary ellipses.
         */

        OUString ident(bool withslash,bool internal) const;

        /** returns the parent url.
         */

        OUString parent(bool internal = false) const;

        /** sets the unencoded title */
        void child(const OUString& title);

        /** returns the unencoded title */
        OUString child() const;

        std::vector<FTPDirentry> list(sal_Int16 nMode) const
            throw(curl_exception);

        // returns a pointer to an open tempfile,
        // sought to the beginning of.
        oslFileHandle open() throw(curl_exception, std::exception);

        FTPDirentry direntry() const
            throw(curl_exception, malformed_exception, std::exception);

        void insert(bool ReplaceExisting,void* stream) const
            throw(curl_exception);

        void mkdir(bool ReplaceExisting) const
            throw(curl_exception, malformed_exception, std::exception);

        OUString ren(const OUString& NewTitle)
            throw(curl_exception, std::exception);

        void del() const
            throw(curl_exception, malformed_exception, std::exception);


    private:

        FTPContentProvider *m_pFCP;

        mutable OUString m_aUsername;
        bool m_bShowPassword;
        mutable OUString m_aHost;
        mutable OUString m_aPort;
        mutable OUString m_aType;

        /** Contains the encoded pathsegments of the url.
         */
        std::vector<OUString> m_aPathSegmentVec;

        void parse(const OUString& url)
            throw(
                malformed_exception
            );

        OUString net_title() const throw(curl_exception, std::exception);
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
