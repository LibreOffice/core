/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: uri.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _RTL_URI_HXX_
#define _RTL_URI_HXX_

#include "rtl/malformeduriexception.hxx"
#include "rtl/uri.h"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

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
