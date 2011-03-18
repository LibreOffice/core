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
#ifndef _NEONURI_HXX_
#define _NEONURI_HXX_

#include <ne_uri.h>
#include <rtl/ustring.hxx>
#include <DAVException.hxx>

namespace webdav_ucp
{

#define DEFAULT_HTTP_PORT       80
#define DEFAULT_HTTPS_PORT      443
#define DEFAULT_FTP_PORT        21

// -------------------------------------------------------------------
// NeonUri
// A URI implementation for use with the neon/expat library
// -------------------------------------------------------------------
class NeonUri
{
    private:
        ::rtl::OUString mURI;
        ::rtl::OUString mScheme;
        ::rtl::OUString mUserInfo;
        ::rtl::OUString mHostName;
        sal_Int32       mPort;
        ::rtl::OUString mPath;

        void init( const rtl::OString & rUri, const ne_uri * pUri );
        void calculateURI ();

    public:
        NeonUri( const ::rtl::OUString & inUri ) throw ( DAVException );
        NeonUri( const ne_uri * inUri ) throw ( DAVException );
        ~NeonUri( );

        bool operator== ( const NeonUri & rOther ) const;
        bool operator!= ( const NeonUri & rOther ) const
        { return !operator==( rOther ); }

        const ::rtl::OUString & GetURI( void ) const
                                            { return mURI; };
        const ::rtl::OUString & GetScheme( void ) const
                                            { return mScheme; };
        const ::rtl::OUString & GetUserInfo( void ) const
                                            { return mUserInfo; };
        const ::rtl::OUString & GetHost( void ) const
                                            { return mHostName; };
        sal_Int32       GetPort( void )     const
                                            { return mPort; };
        const ::rtl::OUString &     GetPath( void ) const
                                            { return mPath; };

        ::rtl::OUString GetPathBaseName ( void ) const;

        ::rtl::OUString GetPathBaseNameUnescaped ( void ) const;

        void SetScheme (const ::rtl::OUString& scheme)
            { mScheme = scheme; calculateURI (); };

        void AppendPath (const ::rtl::OUString& rPath);

        static ::rtl::OUString escapeSegment( const ::rtl::OUString& segment );
        static ::rtl::OUString unescape( const ::rtl::OUString& string );

        // "host:port", omit ":port" for port 80 and 443
        static rtl::OUString makeConnectionEndPointString(
                                        const rtl::OUString & rHostName,
                                        int nPort );
        rtl::OUString makeConnectionEndPointString() const
        { return makeConnectionEndPointString( GetHost(), GetPort() ); }
};

} // namespace webdav_ucp

#endif // _NEONURI_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
