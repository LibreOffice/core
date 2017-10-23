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
    SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        rtl::OUString const & rMessage): m_aMessage(rMessage) {}

    SAL_EXCEPTION_DLLPRIVATE MalformedUriException(
        MalformedUriException const & other): m_aMessage(other.m_aMessage) {}

    SAL_EXCEPTION_DLLPRIVATE ~MalformedUriException() {}

    SAL_EXCEPTION_DLLPRIVATE MalformedUriException operator =(
        MalformedUriException const & rOther)
    { m_aMessage = rOther.m_aMessage; return *this; }

    /** Get the message.

        @return
        A reference to the message.  The reference is valid for the lifetime of
        this MalformedUriException.
     */
    SAL_EXCEPTION_DLLPRIVATE rtl::OUString const & getMessage() const
    { return m_aMessage; }

private:
    rtl::OUString m_aMessage;
};

}

#endif // INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
