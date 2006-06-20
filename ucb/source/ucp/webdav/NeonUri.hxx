/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NeonUri.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:37:41 $
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
#ifndef _NEONURI_HXX_
#define _NEONURI_HXX_

#ifndef NE_URI_H
#include <ne_uri.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _DAVEXCEPTION_HXX_
#include <DAVException.hxx>
#endif

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
        static ne_uri   sUriDefaultsHTTP;
        static ne_uri   sUriDefaultsHTTPS;
        static ne_uri   sUriDefaultsFTP;

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
        ::rtl::OUString GetPathDirName ( void ) const;

        ::rtl::OUString GetPathBaseNameUnescaped ( void ) const;

        void SetScheme (const ::rtl::OUString& scheme)
            { mScheme = scheme; calculateURI (); };

        void AppendPath (const ::rtl::OUString& path);

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
