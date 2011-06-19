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

#if !defined INCLUDED_CPPUHELPER_UNOURL_HXX
#define INCLUDED_CPPUHELPER_UNOURL_HXX

#include <memory>

namespace rtl { class OUString; }

namespace cppu {

/** A descriptor as part of a UNO URL (connection descriptor or protocol
    descriptor).

    Such a descriptor can also be useful outside the context of a full UNO URL.
    For example, some functions take a string representing a connection or
    protocol descriptor as input, and can use this class to parse the string.
 */
class UnoUrlDescriptor
{
public:
    /** @internal
     */
    class Impl;

    /** Construct a descriptor from a string representation.

        @param rDescriptor
        The string representation of a descriptor.

        @exception rtl::MalformedUriException
        Thrown when the given string representation is invalid.
     */
    explicit UnoUrlDescriptor(rtl::OUString const & rDescriptor);

    /** @internal
     */
    explicit UnoUrlDescriptor(std::auto_ptr< Impl > & rImpl);

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
        A (case insensitive) key.

        @return
        True if the parameters contain a matching key/value pair.
     */
    bool hasParameter(rtl::OUString const & rKey) const;

    /** Return the parameter value for a key.

        @param
        A (case insensitive) key.

        @return
        The (case sensitive) value associated with the given key, or an empty
        string if there is no matching key/value pair.
     */
    rtl::OUString getParameter(rtl::OUString const & rKey) const;

private:
    std::auto_ptr< Impl > m_xImpl;
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
class UnoUrl
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

    std::auto_ptr< Impl > m_xImpl;
};

}

#endif // INCLUDED_RTL_UNOURL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
