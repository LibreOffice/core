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
#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONURI_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONURI_HXX

#include <config_lgpl.h>
#include <ne_uri.h>
#include <rtl/ustring.hxx>
#include "DAVException.hxx"

namespace webdav_ucp
{

#define DEFAULT_HTTP_PORT       80
#define DEFAULT_HTTPS_PORT      443
#define DEFAULT_FTP_PORT        21

// A URI implementation for use with the neon/expat library
class NeonUri
{
    private:
        OUString mURI;
        OUString mScheme;
        OUString mUserInfo;
        OUString mHostName;
        sal_Int32       mPort;
        OUString mPath;

        void init( const OString & rUri, const ne_uri * pUri );
        void calculateURI ();

    public:
        /// @throws DAVException
        explicit NeonUri( const OUString & inUri );
        /// @throws DAVException
        explicit NeonUri( const ne_uri * inUri );

        bool operator== ( const NeonUri & rOther ) const;
        bool operator!= ( const NeonUri & rOther ) const
        { return !operator==( rOther ); }

        const OUString & GetURI() const
                                            { return mURI; };
        const OUString & GetScheme() const
                                            { return mScheme; };
        const OUString & GetUserInfo() const
                                            { return mUserInfo; };
        const OUString & GetHost() const
                                            { return mHostName; };
        sal_Int32       GetPort()     const
                                            { return mPort; };
        const OUString &     GetPath() const
                                            { return mPath; };

        OUString GetPathBaseName() const;

        OUString GetPathBaseNameUnescaped() const;

        void SetScheme (const OUString& scheme)
            { mScheme = scheme; calculateURI (); };

        void AppendPath (const OUString& rPath);

        static OUString escapeSegment( const OUString& segment );
        static OUString unescape( const OUString& string );

        // "host:port", omit ":port" for port 80 and 443
        static OUString makeConnectionEndPointString(
                                        const OUString & rHostName,
                                        int nPort );
};

} // namespace webdav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_NEONURI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
