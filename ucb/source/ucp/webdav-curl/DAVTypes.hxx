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


#pragma once

#include <memory>
#include <list>
#include <map>
#include <osl/mutex.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>

namespace http_dav_ucp
{
/* Excerpt from RFC 4918
   <https://tools.ietf.org/html/rfc4918#section-18>

   18.1 Class 1

   A class 1 compliant resource MUST meet all "MUST" requirements in all
   sections of this document.

   Class 1 compliant resources MUST return, at minimum, the value "1" in
   the DAV header on all responses to the OPTIONS method.

   18.2 Class 2

   A class 2 compliant resource MUST meet all class 1 requirements and
   support the LOCK method, the DAV:supportedlock property, the DAV:
   lockdiscovery property, the Time-Out response header and the Lock-
   Token request header.  A class 2 compliant resource SHOULD also
   support the Timeout request header and the 'owner' XML element.

   Class 2 compliant resources MUST return, at minimum, the values "1"
   and "2" in the DAV header on all responses to the OPTIONS method.

   18.3.  Class 3

   A resource can explicitly advertise its support for the revisions to
   [RFC2518] made in this document.  Class 1 MUST be supported as well.
   Class 2 MAY be supported.  Advertising class 3 support in addition to
   class 1 and 2 means that the server supports all the requirements in
   this specification.  Advertising class 3 and class 1 support, but not
   class 2, means that the server supports all the requirements in this
   specification except possibly those that involve locking support.

*/

    class DAVOptions
    {
    private:
        bool    m_isClass1;
        bool    m_isClass2;
        bool    m_isClass3;
        /// for server that do not implement it
        bool    m_isHeadAllowed;
        /// Internally used to maintain the locked state of the resource, only if it's a Class 2 resource
        bool    m_isLocked;
        /// contains the methods allowed on this resource
        OUString    m_aAllowedMethods;

        /// target time when this capability becomes stale
        sal_uInt32 m_nStaleTime;
        OUString  m_sURL;
        OUString  m_sRedirectedURL;

        /// The cached HTT response status code. It's 0 if the code was dealt with and there is no need to cache it
        sal_uInt16 m_nHttpResponseStatusCode;
        /// The cached string with the server returned HTTP response status code string, corresponds to m_nHttpResponseStatusCode.
        OUString  m_sHttpResponseStatusText;

    public:
        DAVOptions();

        DAVOptions( const DAVOptions & rOther );

        virtual ~DAVOptions();

        bool isClass1() { return m_isClass1; };
        void setClass1( bool Class1 = true ) { m_isClass1 = Class1; };

        bool isClass2() { return m_isClass2; };
        void setClass2( bool Class2 = true ) { m_isClass2 = Class2; };

        bool isClass3() { return m_isClass3; };
        void setClass3( bool Class3 = true ) { m_isClass3 = Class3; };

        bool isHeadAllowed() { return m_isHeadAllowed; };
        void setHeadAllowed( bool HeadAllowed = true ) { m_isHeadAllowed = HeadAllowed; };

        sal_uInt32 getStaleTime() { return m_nStaleTime ; };
        void setStaleTime( const sal_uInt32 nStaleTime ) { m_nStaleTime = nStaleTime; };

        const OUString & getURL() { return m_sURL; };
        void setURL( const OUString & sURL ) { m_sURL = sURL; };

        const OUString & getRedirectedURL() { return m_sRedirectedURL; };
        void setRedirectedURL( const OUString & sRedirectedURL ) { m_sRedirectedURL = sRedirectedURL; };

        void  setAllowedMethods( const OUString & aAllowedMethods ) { m_aAllowedMethods = aAllowedMethods; } ;
        const OUString & getAllowedMethods() { return m_aAllowedMethods; } ;
        bool isLockAllowed() { return ( m_aAllowedMethods.indexOf( "LOCK" ) != -1 ); };

        void setLocked( bool locked = true ) { m_isLocked = locked; } ;
        bool isLocked() { return m_isLocked; };

        sal_uInt16 getHttpResponseStatusCode() { return m_nHttpResponseStatusCode; };
        void setHttpResponseStatusCode( const sal_uInt16 nHttpResponseStatusCode ) { m_nHttpResponseStatusCode = nHttpResponseStatusCode; };

        const OUString & getHttpResponseStatusText() { return m_sHttpResponseStatusText; };
        void setHttpResponseStatusText( const OUString & rHttpResponseStatusText ) { m_sHttpResponseStatusText = rHttpResponseStatusText; };

        void init() {
            m_isClass1 = false;
            m_isClass2 = false;
            m_isClass3 = false;
            m_isHeadAllowed = true;
            m_isLocked = false;
            m_aAllowedMethods.clear();
            m_nStaleTime = 0;
            m_sURL.clear();
            m_sRedirectedURL.clear();
            m_nHttpResponseStatusCode = 0;
            m_sHttpResponseStatusText.clear();
        };

        DAVOptions & operator=( const DAVOptions& rOpts );
        bool operator==( const DAVOptions& rOpts ) const;

    };

    // TODO: the OUString key element in std::map needs to be changed with a URI representation
    // along with a specific compare (std::less) implementation, as suggested in
    // <https://tools.ietf.org/html/rfc3986#section-6>, to find by URI and not by string comparison
    typedef std::map< OUString, DAVOptions,
                      std::less< OUString > > DAVOptionsMap;

    class DAVOptionsCache
    {
        DAVOptionsMap m_aTheCache;
        osl::Mutex         m_aMutex;
    public:
        explicit DAVOptionsCache();
        ~DAVOptionsCache();

        bool getDAVOptions( const OUString & rURL, DAVOptions & rDAVOptions );
        void removeDAVOptions( const OUString & rURL );
        void addDAVOptions( DAVOptions & rDAVOptions, const sal_uInt32 nLifeTime );

        void updateCachedOption( DAVOptions & rDAVOptions, const sal_uInt32 nLifeTime );

        /** return the cached value of HTTP response status code
            If the cached value is found stale, it is removed.

            @param (in) OUString
                   the resource URL

            @return int
                    the cached
                    are not present (meaning the resource should be checked for
                    presence anyway)
                    false if resource was not found
        */
        sal_uInt16 getHttpResponseStatusCode( const OUString & rURL, OUString & rHttpResponseStatusText );

        bool isHeadAllowed( const OUString & rURL );

        void setHeadAllowed( const OUString & rURL, bool HeadAllowed = true );

    private:

        /// remove the last '/' in aUrl, if it exists
        static void normalizeURLLastChar( OUString& aUrl ) {
            if ( aUrl.getLength() > 1 &&
                 ( ( aUrl.lastIndexOf( '/' ) + 1 ) == aUrl.getLength() ) )
                aUrl = aUrl.copy(0, aUrl.getLength() - 1 );
        };
    };

    enum Depth { DAVZERO = 0, DAVONE = 1, DAVINFINITY = -1 };

    enum ProppatchOperation { PROPSET = 0, PROPREMOVE = 1 };

    struct ProppatchValue
    {
        ProppatchOperation  operation;
        OUString            name;
        css::uno::Any       value;

        ProppatchValue( const ProppatchOperation o,
                        const OUString & n,
                        const css::uno::Any & v )
            : operation( o ), name( n ), value( v ) {}
    };
} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
