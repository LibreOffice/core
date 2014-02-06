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


#ifndef INCLUDED_SERFURI_HXX
#define INCLUDED_SERFURI_HXX

#include <apr_uri.h>
#include <rtl/ustring.hxx>
#include "DAVException.hxx"

namespace http_dav_ucp
{

#define DEFAULT_HTTP_PORT       80
#define DEFAULT_HTTPS_PORT      443

// -------------------------------------------------------------------
// SerfUri
// A URI implementation for use with the neon/expat library
// -------------------------------------------------------------------
class SerfUri
{
    private:
        apr_uri_t mAprUri;
        OUString mURI;
        OUString mScheme;
        OUString mUserInfo;
        OUString mHostName;
        sal_Int32       mPort;
        OUString mPath;

        void init( const apr_uri_t * pUri );
        void calculateURI ();

    public:
        SerfUri( const OUString & inUri ) throw ( DAVException );
        SerfUri( const apr_uri_t * inUri ) throw ( DAVException );
        ~SerfUri( );

        bool operator== ( const SerfUri & rOther ) const;
        bool operator!= ( const SerfUri & rOther ) const
        { return !operator==( rOther ); }

        apr_uri_t* getAprUri()
        {
            return &mAprUri;
        }
        const OUString & GetURI( void ) const
                                            { return mURI; };
        const OUString & GetScheme( void ) const
                                            { return mScheme; };
        const OUString & GetUserInfo( void ) const
                                            { return mUserInfo; };
        const OUString & GetHost( void ) const
                                            { return mHostName; };
        sal_Int32       GetPort( void )     const
                                            { return mPort; };
        const OUString &     GetPath( void ) const
                                            { return mPath; };

        OUString GetPathBaseName ( void ) const;

        OUString GetPathBaseNameUnescaped ( void ) const;

        void SetScheme (const OUString& scheme)
            { mScheme = scheme; calculateURI (); };

        void AppendPath (const OUString& rPath);

        static OUString escapeSegment( const OUString& segment );
        static OUString unescape( const OUString& string );

        // "host:port", omit ":port" for port 80 and 443
        static OUString makeConnectionEndPointString(
                                        const OUString & rHostName,
                                        int nPort );
        OUString makeConnectionEndPointString() const
        { return makeConnectionEndPointString( GetHost(), GetPort() ); }
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFURI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
