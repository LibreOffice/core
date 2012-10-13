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



#ifndef _DAVEXCEPTION_HXX_
#define _DAVEXCEPTION_HXX_

#include <rtl/ustring.hxx>

namespace http_dav_ucp
{

/////////////////////////////////////////////////////////////////////////////
// HTTP/WebDAV status codes
/////////////////////////////////////////////////////////////////////////////

const sal_uInt16 SC_NONE                             = 0;

// 1xx (Informational - no errors)
const sal_uInt16 SC_CONTINUE                         = 100;
const sal_uInt16 SC_SWITCHING_PROTOCOLS              = 101;
// DAV extensions
const sal_uInt16 SC_PROCESSING                       = 102;

//2xx (Successful - no errors)
const sal_uInt16 SC_OK                               = 200;
const sal_uInt16 SC_CREATED                          = 201;
const sal_uInt16 SC_ACCEPTED                         = 202;
const sal_uInt16 SC_NON_AUTHORITATIVE_INFORMATION    = 203;
const sal_uInt16 SC_NO_CONTENT                       = 204;
const sal_uInt16 SC_RESET_CONTENT                    = 205;
const sal_uInt16 SC_PARTIAL_CONTENT                  = 206;
// DAV extensions
const sal_uInt16 SC_MULTISTATUS                      = 207;

//3xx (Redirection)
const sal_uInt16 SC_MULTIPLE_CHOICES                 = 300;
const sal_uInt16 SC_MOVED_PERMANENTLY                = 301;
const sal_uInt16 SC_MOVED_TEMPORARILY                = 302;
const sal_uInt16 SC_SEE_OTHER                        = 303;
const sal_uInt16 SC_NOT_MODIFIED                     = 304;
const sal_uInt16 SC_USE_PROXY                        = 305;
const sal_uInt16 SC_TEMPORARY_REDIRECT               = 307;

//4xx (Client error)
const sal_uInt16 SC_BAD_REQUEST                      = 400;
const sal_uInt16 SC_UNAUTHORIZED                     = 401;
const sal_uInt16 SC_PAYMENT_REQUIRED                 = 402;
const sal_uInt16 SC_FORBIDDEN                        = 403;
const sal_uInt16 SC_NOT_FOUND                        = 404;
const sal_uInt16 SC_METHOD_NOT_ALLOWED               = 405;
const sal_uInt16 SC_NOT_ACCEPTABLE                   = 406;
const sal_uInt16 SC_PROXY_AUTHENTICATION_REQUIRED    = 407;
const sal_uInt16 SC_REQUEST_TIMEOUT                  = 408;
const sal_uInt16 SC_CONFLICT                         = 409;
const sal_uInt16 SC_GONE                             = 410;
const sal_uInt16 SC_LENGTH_REQUIRED                  = 411;
const sal_uInt16 SC_PRECONDITION_FAILED              = 412;
const sal_uInt16 SC_REQUEST_ENTITY_TOO_LARGE         = 413;
const sal_uInt16 SC_REQUEST_URI_TOO_LONG             = 414;
const sal_uInt16 SC_UNSUPPORTED_MEDIA_TYPE           = 415;
const sal_uInt16 SC_REQUESTED_RANGE_NOT_SATISFIABLE  = 416;
const sal_uInt16 SC_EXPECTATION_FAILED               = 417;
// DAV extensions
const sal_uInt16 SC_UNPROCESSABLE_ENTITY             = 422;
const sal_uInt16 SC_LOCKED                           = 423;
const sal_uInt16 SC_FAILED_DEPENDENCY                = 424;

//5xx (Server error)
const sal_uInt16 SC_INTERNAL_SERVER_ERROR            = 500;
const sal_uInt16 SC_NOT_IMPLEMENTED                  = 501;
const sal_uInt16 SC_BAD_GATEWAY                      = 502;
const sal_uInt16 SC_SERVICE_UNAVAILABLE              = 503;
const sal_uInt16 SC_GATEWAY_TIMEOUT                  = 504;
const sal_uInt16 SC_HTTP_VERSION_NOT_SUPPORTED       = 505;
// DAV extensions
const sal_uInt16 SC_INSUFFICIENT_STORAGE             = 507;

/////////////////////////////////////////////////////////////////////////////

class DAVException
{
    public:
        enum ExceptionCode {
            DAV_HTTP_ERROR = 0, // Generic error,
                                // mData = server error message,
                                // mStatusCode = HTTP status code
            DAV_HTTP_LOOKUP,    // Name lookup failed,
                                // mData = server[:port]
            DAV_HTTP_NOAUTH,    // No User authentication data provided - e.g., user aborts corresponding dialog
                                // mData = server[:port]
            DAV_HTTP_AUTH,      // User authentication failed on server,
                                // mData = server[:port]
            DAV_HTTP_AUTHPROXY, // User authentication failed on proxy,
                                // mData = proxy server[:port]
            DAV_HTTP_CONNECT,   // Could not connect to server,
                                // mData = server[:port]
            DAV_HTTP_TIMEOUT,   // Connection timed out
                                // mData = server[:port]
            DAV_HTTP_FAILED,    // The precondition failed
                                // mData = server[:port]
            DAV_HTTP_RETRY,     // Retry request
                                // mData = server[:port]
            DAV_HTTP_REDIRECT,  // Request was redirected,
                                // mData = new URL
            DAV_SESSION_CREATE, // session creation error,
                                // mData = server[:port]
            DAV_INVALID_ARG,    // invalid argument

            DAV_LOCK_EXPIRED,   // DAV lock expired

            DAV_NOT_LOCKED,     // not locked

            DAV_LOCKED_SELF,    // locked by this OOo session

            DAV_LOCKED          // locked by third party
        };

    private:
        ExceptionCode   mExceptionCode;
        rtl::OUString   mData;
        sal_uInt16      mStatusCode;

    public:
         DAVException( ExceptionCode inExceptionCode )
             : mExceptionCode( inExceptionCode )
             , mData()
             , mStatusCode( SC_NONE )
         {};
         DAVException( ExceptionCode inExceptionCode,
                       const rtl::OUString & rData )
             : mExceptionCode( inExceptionCode )
             , mData( rData )
             , mStatusCode( SC_NONE )
         {};
         DAVException( ExceptionCode inExceptionCode,
                       const rtl::OUString & rData,
                       sal_uInt16 nStatusCode )
            : mExceptionCode( inExceptionCode )
            , mData( rData )
            , mStatusCode( nStatusCode )
         {};
        ~DAVException( ) {};

    const ExceptionCode & getError() const { return mExceptionCode; }
    const rtl::OUString & getData() const  { return mData; }
    sal_uInt16 getStatus() const { return mStatusCode; }
};

} // namespace http_dav_ucp

#endif // _DAVEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
