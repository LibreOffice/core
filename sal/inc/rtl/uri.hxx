/*************************************************************************
 *
 *  $RCSfile: uri.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sb $ $Date: 2002-09-24 10:15:18 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_URI_HXX_
#define _RTL_URI_HXX_

#ifndef INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
#include "rtl/malformeduriexception.hxx"
#endif
#ifndef _RTL_URI_H_
#include "rtl/uri.h"
#endif
#ifndef _RTL_TEXTENC_H
#include "rtl/textenc.h"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

namespace rtl {

/** A wrapper around the C functions from <rtl/uri.h>.
 */
class Uri
{
public:
    /** A wrapper around rtl_uriEncode() from <rtl/uri.h> (see there), using
        an array of 128 booleans as char class.
     */
    static inline rtl::OUString encode(rtl::OUString const & rText,
                                       sal_Bool const * pCharClass,
                                       rtl_UriEncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
        SAL_THROW(());

    /** A wrapper around rtl_uriEncode() from <rtl/uri.h> (see there), using
        a predefined rtl_UriCharClass enumeration member.
     */
    static inline rtl::OUString encode(rtl::OUString const & rText,
                                       rtl_UriCharClass eCharClass,
                                       rtl_UriEncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
        SAL_THROW(());

    /** A wrapper around rtl_uriDecode() from <rtl/uri.h> (see there).
     */
    static inline rtl::OUString decode(rtl::OUString const & rText,
                                       rtl_UriDecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
        SAL_THROW(());

    /** A wrapper around rtl_uriConvertRelToAbs() from <rtl/uri.h> (see there).

        @exception MalformedUriException
        Thrown in case rtl_uriConvertRelToAbs() signals an exception due to a
        malformed base URI.
     */
    static inline rtl::OUString convertRelToAbs(
        rtl::OUString const & rBaseUriRef, rtl::OUString const & rRelUriRef);

private:
    /** not implemented
        @internal */
    Uri();

    /** not implemented
        @internal */
    Uri(Uri &);

    /** not implemented
        @internal */
    ~Uri();

    /** not implemented
        @internal */
    void operator =(Uri);
};

inline rtl::OUString Uri::encode(rtl::OUString const & rText,
                                 sal_Bool const * pCharClass,
                                 rtl_UriEncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
    SAL_THROW(())
{
    rtl::OUString aResult;
    rtl_uriEncode(const_cast< rtl::OUString & >(rText).pData,
                  pCharClass,
                  eMechanism,
                  eCharset,
                  &aResult.pData);
    return aResult;
}

inline rtl::OUString Uri::encode(rtl::OUString const & rText,
                                 rtl_UriCharClass eCharClass,
                                 rtl_UriEncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
    SAL_THROW(())
{
    rtl::OUString aResult;
    rtl_uriEncode(const_cast< rtl::OUString & >(rText).pData,
                  rtl_getUriCharClass(eCharClass),
                  eMechanism,
                  eCharset,
                  &aResult.pData);
    return aResult;
}

inline rtl::OUString Uri::decode(rtl::OUString const & rText,
                                 rtl_UriDecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
    SAL_THROW(())
{
    rtl::OUString aResult;
    rtl_uriDecode(const_cast< rtl::OUString & >(rText).pData,
                  eMechanism,
                  eCharset,
                  &aResult.pData);
    return aResult;
}

inline rtl::OUString Uri::convertRelToAbs(rtl::OUString const & rBaseUriRef,
                                          rtl::OUString const & rRelUriRef)
{
    rtl::OUString aResult;
    rtl::OUString aException;
    if (!rtl_uriConvertRelToAbs(
            const_cast< rtl::OUString & >(rBaseUriRef).pData,
            const_cast< rtl::OUString & >(rRelUriRef).pData, &aResult.pData,
            &aException.pData))
        throw MalformedUriException(aException);
    return aResult;
}

}

#endif // _RTL_URI_HXX_
