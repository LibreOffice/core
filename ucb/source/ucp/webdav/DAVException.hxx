/*************************************************************************
 *
 *  $RCSfile: DAVException.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kso $ $Date: 2002-08-21 07:34:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DAVEXCEPTION_HXX_
#define _DAVEXCEPTION_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace webdav_ucp
{

class DAVException
{
    public:
        enum ExceptionCode {
            DAV_HTTP_ERROR = 0, // Generic error, mData = error message
            DAV_HTTP_LOOKUP,    // Name lookup failed, mData = server[:port]
            DAV_HTTP_AUTH,      // User authentication failed on server
            DAV_HTTP_AUTHPROXY, // User authentication failed on proxy
            DAV_HTTP_CONNECT,   // Could not connect to server, mData = server[:port]
            DAV_HTTP_TIMEOUT,   // Connection timed out
            DAV_HTTP_FAILED,    // The precondition failed
            DAV_HTTP_RETRY,     // Retry request
            DAV_HTTP_REDIRECT,  // See http_redirect.h, mData = new URL
            DAV_SESSION_CREATE, // session creation error, mData = server[:port]
            DAV_INVALID_ARG };  // mData = file URL

    private:
        ExceptionCode   mExceptionCode;
        rtl::OUString   mData;
        sal_uInt16      mStatusCode;

    public:
         DAVException( ExceptionCode inExceptionCode ) :
            mExceptionCode( inExceptionCode ), mStatusCode( 0 ) {};
         DAVException( ExceptionCode inExceptionCode,
                       const rtl::OUString & rData ) :
            mExceptionCode( inExceptionCode ), mData( rData ),
            mStatusCode( 0 ) {};
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

}; // namespace webdav_ucp

#endif // _DAVEXCEPTION_HXX_
