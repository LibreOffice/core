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


    enum FTPErrors { FOLDER_MIGHT_EXIST_DURING_INSERT = CURL_LAST,
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
        /// @throws malformed_exception
        FTPURL(
            const OUString& aIdent,
            FTPContentProvider* pFCP
        );

        FTPURL(const FTPURL& r);

        ~FTPURL();

        const OUString& host() const { return m_aHost; }

        const OUString& port() const { return m_aPort; }

        const OUString& username() const { return m_aUsername; }

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

        /// @throws curl_exception
        std::vector<FTPDirentry> list(sal_Int16 nMode) const;

        // returns a pointer to an open tempfile,
        // sought to the beginning of.
        /// @throws curl_exception
        oslFileHandle open();

        /// @throws curl_exception
        /// @throws malformed_exception
        FTPDirentry direntry() const;

        /// @throws curl_exception
        void insert(bool ReplaceExisting,void* stream) const;

        /// @throws curl_exception
        /// @throws malformed_exception
        void mkdir(bool ReplaceExisting) const;

        /// @throws curl_exception
        OUString ren(const OUString& NewTitle);

        /// @throws curl_exception
        /// @throws malformed_exception
        void del() const;


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

        /// @throws malformed_exception
        void parse(const OUString& url);

        /// @throws curl_exception
        OUString net_title() const;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
