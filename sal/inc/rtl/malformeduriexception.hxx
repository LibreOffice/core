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

#ifndef INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
#define INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX

#include "rtl/ustring.hxx"

namespace rtl {

/** An exception indicating a malformed URI.

    <P>Used when parsing (part of) a URI fails for syntactical reasons.</P>
 */
class SAL_EXCEPTION_DLLPUBLIC_EXPORT MalformedUriException
{
public:
    /** Create a MalformedUriException.

        @param rMessage
        A message containing any details about the exception.
     */
    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        rtl::OUString const & rMessage): m_aMessage(rMessage) {}

    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        MalformedUriException const & other): m_aMessage(other.m_aMessage) {}

    inline SAL_EXCEPTION_DLLPRIVATE ~MalformedUriException() {}

    inline SAL_EXCEPTION_DLLPRIVATE MalformedUriException operator =(
        MalformedUriException const & rOther)
    { m_aMessage = rOther.m_aMessage; return *this; }

    /** Get the message.

        @return
        A reference to the message.  The reference is valid for the lifetime of
        this MalformedUriException.
     */
    inline SAL_EXCEPTION_DLLPRIVATE rtl::OUString const & getMessage() const
    { return m_aMessage; }

private:
    rtl::OUString m_aMessage;
};

}

#endif // INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
