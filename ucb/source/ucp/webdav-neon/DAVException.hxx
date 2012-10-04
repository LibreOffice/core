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

#ifndef _DAVEXCEPTION_HXX_
#define _DAVEXCEPTION_HXX_

#include <rtl/ustring.hxx>

namespace webdav_ucp
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
         DAVException( ExceptionCode inExceptionCode ) :
            mExceptionCode( inExceptionCode ), mStatusCode( SC_NONE ) {};
         DAVException( ExceptionCode inExceptionCode,
                       const rtl::OUString & rData ) :
            mExceptionCode( inExceptionCode ), mData( rData ),
            mStatusCode( SC_NONE ) {};
         DAVException( ExceptionCode inExceptionCode,
                       const rtl::OUString & rData,
                       sal_uInt16 nStatusCode ) :
            mExceptionCode( inExceptionCode ), mData( rData ),
            mStatusCode( nStatusCode ) {};
        ~DAVException( ) {};

    const ExceptionCode & getError() const { return mExceptionCode; }
    const rtl::OUString & getData() const  { return mData; }
    sal_uInt16 getStatus() const { return mStatusCode; }
};

} // namespace webdav_ucp

#endif // _DAVEXCEPTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
