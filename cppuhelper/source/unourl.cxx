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


#include <cppuhelper/unourl.hxx>

#include <rtl/malformeduriexception.hxx>
#include <rtl/string.h>
#include <rtl/textenc.h>
#include <rtl/uri.h>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include <rtl/character.hxx>
#include <sal/types.h>

#include <map>
#include <memory>

using cppu::UnoUrl;
using cppu::UnoUrlDescriptor;

class UnoUrlDescriptor::Impl
{
public:
    typedef std::map< OUString, OUString > Parameters;

    OUString m_aDescriptor;
    OUString m_aName;
    Parameters m_aParameters;

    /** @exception rtl::MalformedUriException
     */
    explicit inline Impl(OUString const & m_aDescriptor);

    Impl * clone() const { return new Impl(*this); }
};

inline UnoUrlDescriptor::Impl::Impl(OUString const & rDescriptor)
{
    m_aDescriptor = rDescriptor;
    enum State { STATE_NAME0, STATE_NAME, STATE_KEY0, STATE_KEY, STATE_VALUE };
    State eState = STATE_NAME0;
    sal_Int32 nStart = 0;
    OUString aKey;
    for (sal_Int32 i = 0;; ++i)
    {
        bool bEnd = i == rDescriptor.getLength();
        sal_Unicode c = bEnd ? 0 : rDescriptor[i];
        switch (eState)
        {
        case STATE_NAME0:
            if (bEnd || !rtl::isAsciiAlphanumeric(c))
                throw rtl::MalformedUriException(
                    "UNO URL contains bad descriptor name");
            nStart = i;
            eState = STATE_NAME;
            break;

        case STATE_NAME:
            if (bEnd || c == 0x2C) // ','
            {
                m_aName
                    = rDescriptor.copy(nStart, i - nStart).toAsciiLowerCase();
                eState = STATE_KEY0;
            }
            else if (!rtl::isAsciiAlphanumeric(c))
                throw rtl::MalformedUriException(
                    "UNO URL contains bad descriptor name");
            break;

        case STATE_KEY0:
            if (bEnd || !rtl::isAsciiAlphanumeric(c))
                throw rtl::MalformedUriException(
                    "UNO URL contains bad parameter key");
            nStart = i;
            eState = STATE_KEY;
            break;

        case STATE_KEY:
            if (c == 0x3D) // '='
            {
                aKey = rDescriptor.copy(nStart, i - nStart).toAsciiLowerCase();
                nStart = i + 1;
                eState = STATE_VALUE;
            }
            else if (bEnd || !rtl::isAsciiAlphanumeric(c))
                throw rtl::MalformedUriException(
                    "UNO URL contains bad parameter key");
            break;

        case STATE_VALUE:
            if (bEnd || c == 0x2C) // ','
            {
                if (!m_aParameters.emplace(
                            aKey,
                            rtl::Uri::decode(rDescriptor.copy(nStart,
                                                              i - nStart),
                                             rtl_UriDecodeWithCharset,
                                             RTL_TEXTENCODING_UTF8)).second)
                    throw rtl::MalformedUriException(
                        "UNO URL contains duplicated parameter");
                eState = STATE_KEY0;
            }
            break;
        }
        if (bEnd)
            break;
    }
}

UnoUrlDescriptor::UnoUrlDescriptor(OUString const & rDescriptor):
    m_pImpl(new Impl(rDescriptor))
{}

UnoUrlDescriptor::UnoUrlDescriptor(UnoUrlDescriptor const & rOther):
    m_pImpl(rOther.m_pImpl->clone())
{}

UnoUrlDescriptor::~UnoUrlDescriptor()
{
    delete m_pImpl;
}

UnoUrlDescriptor & UnoUrlDescriptor::operator =(UnoUrlDescriptor const & rOther)
{
    if (this != &rOther)
    {
        std::unique_ptr<Impl> newImpl(rOther.m_pImpl->clone());
        delete m_pImpl;
        m_pImpl = newImpl.release();
    }
    return *this;
}

OUString const & UnoUrlDescriptor::getDescriptor() const
{
    return m_pImpl->m_aDescriptor;
}

OUString const & UnoUrlDescriptor::getName() const
{
    return m_pImpl->m_aName;
}

bool UnoUrlDescriptor::hasParameter(OUString const & rKey) const
{
    return m_pImpl->m_aParameters.find(rKey.toAsciiLowerCase())
        != m_pImpl->m_aParameters.end();
}

OUString UnoUrlDescriptor::getParameter(OUString const & rKey) const
{
    Impl::Parameters::const_iterator
        aIt(m_pImpl->m_aParameters.find(rKey.toAsciiLowerCase()));
    return aIt == m_pImpl->m_aParameters.end() ? OUString() : aIt->second;
}

class UnoUrl::Impl
{
public:
    UnoUrlDescriptor m_aConnection;
    UnoUrlDescriptor m_aProtocol;
    OUString m_aObjectName;

    Impl * clone() const { return new Impl(*this); }

    /** @exception rtl::MalformedUriException
     */
    static inline Impl * create(OUString const & rUrl);

private:
    Impl(OUString const & rConnectionDescriptor,
         OUString const & rProtocolDescriptor,
         OUString const & rObjectName):
        m_aConnection(rConnectionDescriptor),
        m_aProtocol(rProtocolDescriptor),
        m_aObjectName(rObjectName)
    {}
};

inline UnoUrl::Impl * UnoUrl::Impl::create(OUString const & rUrl)
{
    if (!rUrl.startsWithIgnoreAsciiCase("uno:"))
        throw rtl::MalformedUriException("UNO URL does not start with \"uno:\"");
    sal_Int32 i = RTL_CONSTASCII_LENGTH("uno:");
    sal_Int32 j = rUrl.indexOf(';', i);
    if (j < 0)
        throw rtl::MalformedUriException("UNO URL has too few semicolons");
    OUString aConnection(rUrl.copy(i, j - i));
    i = j + 1;
    j = rUrl.indexOf(0x3B, i); // ';'
    if (j < 0)
        throw rtl::MalformedUriException("UNO URL has too few semicolons");
    OUString aProtocol(rUrl.copy(i, j - i));
    i = j + 1;
    if (i == rUrl.getLength())
        throw rtl::MalformedUriException("UNO URL contains empty ObjectName");
    for (j = i; j < rUrl.getLength(); ++j)
    {
        sal_Unicode c = rUrl[j];
        if (!rtl::isAsciiAlphanumeric(c) && c != 0x21 && c != 0x24 // '!', '$'
            && c != 0x26 && c != 0x27 && c != 0x28 // '&', ''', '('
            && c != 0x29 && c != 0x2A && c != 0x2B // ')', '*', '+'
            && c != 0x2C && c != 0x2D && c != 0x2E // ',', '-', '.'
            && c != 0x2F && c != 0x3A && c != 0x3D // '/', ':', '='
            && c != 0x3F && c != 0x40 && c != 0x5F // '?', '@', '_'
            && c != 0x7E) // '~'
            throw rtl::MalformedUriException("UNO URL contains invalid ObjectName");
    }
    return new Impl(aConnection, aProtocol, rUrl.copy(i));
}

UnoUrl::UnoUrl(OUString const & rUrl): m_pImpl(Impl::create(rUrl))
{}

UnoUrl::UnoUrl(UnoUrl const & rOther): m_pImpl(rOther.m_pImpl->clone())
{}

UnoUrl::~UnoUrl()
{
    delete m_pImpl;
}

UnoUrl & UnoUrl::operator =(UnoUrl const & rOther)
{
    if (this != &rOther)
    {
        std::unique_ptr<Impl> newImpl(rOther.m_pImpl->clone());
        delete m_pImpl;
        m_pImpl = newImpl.release();
    }
    return *this;
}

UnoUrlDescriptor const & UnoUrl::getConnection() const
{
    return m_pImpl->m_aConnection;
}

UnoUrlDescriptor const & UnoUrl::getProtocol() const
{
    return m_pImpl->m_aProtocol;
}

OUString const & UnoUrl::getObjectName() const
{
    return m_pImpl->m_aObjectName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
