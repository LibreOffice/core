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

#ifndef INCLUDED_RTL_URI_HXX
#define INCLUDED_RTL_URI_HXX

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
                                       rtl_TextEncoding eCharset);

    /** A wrapper around rtl_uriEncode() from <rtl/uri.h> (see there), using
        a predefined rtl_UriCharClass enumeration member.
     */
    static inline rtl::OUString encode(rtl::OUString const & rText,
                                       rtl_UriCharClass eCharClass,
                                       rtl_UriEncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset);

    /** A wrapper around rtl_uriDecode() from <rtl/uri.h> (see there).
     */
    static inline rtl::OUString decode(rtl::OUString const & rText,
                                       rtl_UriDecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset);

    /** A wrapper around rtl_uriConvertRelToAbs() from <rtl/uri.h> (see there).

        @exception MalformedUriException
        Thrown in case rtl_uriConvertRelToAbs() signals an exception due to a
        malformed base URI.
     */
    static inline rtl::OUString convertRelToAbs(
        rtl::OUString const & rBaseUriRef, rtl::OUString const & rRelUriRef);

private:
    Uri() SAL_DELETED_FUNCTION;

    Uri(Uri &) SAL_DELETED_FUNCTION;

    ~Uri() SAL_DELETED_FUNCTION;

    void operator =(Uri) SAL_DELETED_FUNCTION;
};

inline rtl::OUString Uri::encode(rtl::OUString const & rText,
                                 sal_Bool const * pCharClass,
                                 rtl_UriEncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
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

#endif // INCLUDED_RTL_URI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
