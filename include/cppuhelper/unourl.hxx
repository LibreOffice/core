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

#ifndef INCLUDED_CPPUHELPER_UNOURL_HXX
#define INCLUDED_CPPUHELPER_UNOURL_HXX

#include "sal/config.h"

#include "cppuhelper/cppuhelperdllapi.h"

namespace rtl { class OUString; }

namespace cppu {

/** A descriptor as part of a UNO URL (connection descriptor or protocol
    descriptor).

    Such a descriptor can also be useful outside the context of a full UNO URL.
    For example, some functions take a string representing a connection or
    protocol descriptor as input, and can use this class to parse the string.
 */
class SAL_WARN_UNUSED CPPUHELPER_DLLPUBLIC UnoUrlDescriptor
{
public:
    class Impl;

    /** Construct a descriptor from a string representation.

        @param rDescriptor
        The string representation of a descriptor.

        @exception rtl::MalformedUriException
        Thrown when the given string representation is invalid.
     */
    explicit UnoUrlDescriptor(rtl::OUString const & rDescriptor);

    UnoUrlDescriptor(UnoUrlDescriptor const & rOther);

    ~UnoUrlDescriptor();

    UnoUrlDescriptor & operator =(UnoUrlDescriptor const & rOther);

    /** Return the string representation of the descriptor.

        @return
        A reference to the string representation used to construct this
        descriptor, without any modifications.  The reference is valid for the
        lifetime of this URL object.
     */
    rtl::OUString const & getDescriptor() const;

    /** Return the name component of the descriptor.

        @return
        A reference to the (case insensitive) name, in lower case form.  The
        reference is valid for the lifetime of this URL object.
     */
    rtl::OUString const & getName() const;

    /** Test whether the parameters contain a key.

        @param
        rKey A (case insensitive) key.

        @return
        True if the parameters contain a matching key/value pair.
     */
    bool hasParameter(rtl::OUString const & rKey) const;

    /** Return the parameter value for a key.

        @param
        rKey A (case insensitive) key.

        @return
        The (case sensitive) value associated with the given key, or an empty
        string if there is no matching key/value pair.
     */
    rtl::OUString getParameter(rtl::OUString const & rKey) const;

private:
    Impl * m_pImpl;
};

/** Parse UNO URLs into their components.

    The ABNF for UNO URLs is as follows (see RFCs 2234, 2396, also see
    <http://udk.openoffice.org/common/man/spec/uno-url.html>):

    uno-url = "UNO:" connection ";" protocol ";" object-name
    connection = descriptor
    protocol = descriptor
    descriptor = name *("," parameter)
    name = 1*alphanum
    parameter = key "=" value
    key = 1*alphanum
    value = *vchar
    valchar = unreserved / escaped / "$" / "&" / "+" / "/" / ":" / "?" / "@"
    object-name = 1*ochar
    ochar = unreserved / "$" / "&" / "+" / "," / "/" / ":" / "=" / "?" / "@"

    Within a descriptor, the name and the keys are case insensitive, and within
    the parameter list all keys must be distinct.

    Parameter values are encoded using UTF-8.  Note that parsing of parameter
    values as done by UnoUrl and UnoUrlDescriptor is not strict:  Invalid UTF-16
    entities in the input, as well as broken escape sequences ("%" not followed
    by two hex digits) are copied verbatim to the output, invalid bytes in the
    converted UTF-8 data are considered individual Unicode characters, and
    invalid UTF-16 entities in the resulting output (e.g., a high surrogate not
    followed by a low surrogate) are not detected.
 */
class SAL_WARN_UNUSED CPPUHELPER_DLLPUBLIC UnoUrl
{
public:
    /** Construct a UNO URL from a string representation.

        @param rUrl
        The string representation of a UNO URL.

        @exception rtl::MalformedUriException
        Thrown when the given string representation is invalid.
     */
    explicit UnoUrl(rtl::OUString const & rUrl);

    UnoUrl(UnoUrl const & rOther);

    ~UnoUrl();

    UnoUrl & operator =(UnoUrl const & rOther);

    /** Return the connection descriptor component of the URL.

        @return
        A reference to the connection descriptor.  The reference is valid for
        the lifetime of this URL object.
     */
    UnoUrlDescriptor const & getConnection() const;

    /** Return the protocol descriptor component of the URL.

        @return
        A reference to the protocol descriptor.  The reference is valid for the
        lifetime of this URL object.
     */
    UnoUrlDescriptor const & getProtocol() const;

    /** Return the object-name component of the URL.

        @return
        A reference to the (case sensitive) object-name.  The reference is valid
        for the lifetime of this URL object.
     */
    rtl::OUString const & getObjectName() const;

private:
    class Impl;

    Impl * m_pImpl;
};

}

#endif // INCLUDED_RTL_UNOURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
