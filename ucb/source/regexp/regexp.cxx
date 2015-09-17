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

#include <regexp.hxx>

#include <cstddef>

#include "osl/diagnose.h"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

using namespace com::sun::star;
using namespace ucb_impl;



//  Regexp



inline Regexp::Regexp(Kind eTheKind, OUString const & rThePrefix,
                      bool bTheEmptyDomain, OUString const & rTheInfix,
                      bool bTheTranslation,
                      OUString const & rTheReversePrefix):
    m_eKind(eTheKind),
    m_aPrefix(rThePrefix),
    m_aInfix(rTheInfix),
    m_aReversePrefix(rTheReversePrefix),
    m_bEmptyDomain(bTheEmptyDomain),
    m_bTranslation(bTheTranslation)
{
    OSL_ASSERT(m_eKind == KIND_DOMAIN
               || (!m_bEmptyDomain && m_aInfix.isEmpty()));
    OSL_ASSERT(m_bTranslation || m_aReversePrefix.isEmpty());
}


namespace {

bool matchStringIgnoreCase(sal_Unicode const ** pBegin,
                           sal_Unicode const * pEnd,
                           OUString const & rString)
{
    sal_Unicode const * p = *pBegin;

    sal_Unicode const * q = rString.getStr();
    sal_Unicode const * qEnd = q + rString.getLength();

    if (pEnd - p < qEnd - q)
        return false;

    while (q != qEnd)
    {
        sal_Unicode c1 = *p++;
        sal_Unicode c2 = *q++;
        if (c1 >= 'a' && c1 <= 'z')
            c1 -= 'a' - 'A';
        if (c2 >= 'a' && c2 <= 'z')
            c2 -= 'a' - 'A';
        if (c1 != c2)
            return false;
    }

    *pBegin = p;
    return true;
}

}

bool Regexp::matches(OUString const & rString,
                     OUString * pTranslation, bool * pTranslated) const
{
    sal_Unicode const * pBegin = rString.getStr();
    sal_Unicode const * pEnd = pBegin + rString.getLength();

    bool bMatches = false;

    sal_Unicode const * p = pBegin;
    if (matchStringIgnoreCase(&p, pEnd, m_aPrefix))
    {
        sal_Unicode const * pBlock1Begin = p;
        sal_Unicode const * pBlock1End = pEnd;

        sal_Unicode const * pBlock2Begin = 0;
        sal_Unicode const * pBlock2End = 0;

        switch (m_eKind)
        {
            case KIND_PREFIX:
                bMatches = true;
                break;

            case KIND_AUTHORITY:
                bMatches = p == pEnd || *p == '/' || *p == '?' || *p == '#';
                break;

            case KIND_DOMAIN:
                if (!m_bEmptyDomain)
                {
                    if (p == pEnd || *p == '/' || *p == '?' || *p == '#')
                        break;
                    ++p;
                }
                for (;;)
                {
                    sal_Unicode const * q = p;
                    if (matchStringIgnoreCase(&q, pEnd, m_aInfix)
                        && (q == pEnd || *q == '/' || *q == '?' || *q == '#'))
                    {
                        bMatches = true;
                        pBlock1End = p;
                        pBlock2Begin = q;
                        pBlock2End = pEnd;
                        break;
                    }

                    if (p == pEnd)
                        break;

                    sal_Unicode c = *p++;
                    if (c == '/' || c == '?' || c == '#')
                        break;
                }
                break;
        }

        if (bMatches)
        {
            if (m_bTranslation)
            {
                if (pTranslation)
                {
                    OUStringBuffer aBuffer(m_aReversePrefix);
                    aBuffer.append(pBlock1Begin, pBlock1End - pBlock1Begin);
                    aBuffer.append(m_aInfix);
                    aBuffer.append(pBlock2Begin, pBlock2End - pBlock2Begin);
                    *pTranslation = aBuffer.makeStringAndClear();
                }
                if (pTranslated)
                    *pTranslated = true;
            }
            else
            {
                if (pTranslation)
                    *pTranslation = rString;
                if (pTranslated)
                    *pTranslated = false;
            }
        }
    }

    return bMatches;
}


namespace {

bool isScheme(OUString const & rString, bool bColon)
{
    // Return true if rString matches <scheme> (plus a trailing ":" if bColon
    // is true) from RFC 2396:
    sal_Unicode const * p = rString.getStr();
    sal_Unicode const * pEnd = p + rString.getLength();
    if (p != pEnd && rtl::isAsciiAlpha(*p))
        for (++p;;)
        {
            if (p == pEnd)
                return !bColon;
            sal_Unicode c = *p++;
            if (!(rtl::isAsciiAlpha(c) || rtl::isAsciiDigit(c)
                  || c == '+' || c == '-' || c == '.'))
                return bColon && c == ':' && p == pEnd;
        }
    return false;
}

void appendStringLiteral(OUStringBuffer * pBuffer,
                         OUString const & rString)
{
    OSL_ASSERT(pBuffer);

    pBuffer->append('"');
    sal_Unicode const * p = rString.getStr();
    sal_Unicode const * pEnd = p + rString.getLength();
    while (p != pEnd)
    {
        sal_Unicode c = *p++;
        if (c == '"' || c == '\\')
            pBuffer->append('\\');
        pBuffer->append(c);
    }
    pBuffer->append('"');
}

}

OUString Regexp::getRegexp(bool bReverse) const
{
    if (m_bTranslation)
    {
        OUStringBuffer aBuffer;
        if (bReverse)
        {
            if (!m_aReversePrefix.isEmpty())
                appendStringLiteral(&aBuffer, m_aReversePrefix);
        }
        else
        {
            if (!m_aPrefix.isEmpty())
                appendStringLiteral(&aBuffer, m_aPrefix);
        }
        switch (m_eKind)
        {
            case KIND_PREFIX:
                aBuffer.append("(.*)");
                break;

            case KIND_AUTHORITY:
                aBuffer.append("(([/?#].*)?)");
                break;

            case KIND_DOMAIN:
                aBuffer.append("([^/?#]");
                aBuffer.append(sal_Unicode(m_bEmptyDomain ? '*' : '+'));
                if (!m_aInfix.isEmpty())
                    appendStringLiteral(&aBuffer, m_aInfix);
                aBuffer.append("([/?#].*)?)");
                break;
        }
        aBuffer.append("->");
        if (bReverse)
        {
            if (!m_aPrefix.isEmpty())
                appendStringLiteral(&aBuffer, m_aPrefix);
        }
        else
        {
            if (!m_aReversePrefix.isEmpty())
                appendStringLiteral(&aBuffer, m_aReversePrefix);
        }
        aBuffer.append("\\1");
        return aBuffer.makeStringAndClear();
    }
    else if (m_eKind == KIND_PREFIX && isScheme(m_aPrefix, true))
        return m_aPrefix.copy(0, m_aPrefix.getLength() - 1);
    else
    {
        OUStringBuffer aBuffer;
        if (!m_aPrefix.isEmpty())
            appendStringLiteral(&aBuffer, m_aPrefix);
        switch (m_eKind)
        {
            case KIND_PREFIX:
                aBuffer.append(".*");
                break;

            case KIND_AUTHORITY:
                aBuffer.append("([/?#].*)?");
                break;

            case KIND_DOMAIN:
                aBuffer.append("[^/?#]");
                aBuffer.append( m_bEmptyDomain ? '*' : '+' );
                if (!m_aInfix.isEmpty())
                    appendStringLiteral(&aBuffer, m_aInfix);
                aBuffer.append("([/?#].*)?");
                break;
        }
        return aBuffer.makeStringAndClear();
    }
}


namespace {

bool matchString(sal_Unicode const ** pBegin, sal_Unicode const * pEnd,
                 sal_Char const * pString, size_t nStringLength)
{
    sal_Unicode const * p = *pBegin;

    unsigned char const * q = reinterpret_cast< unsigned char const * >(pString);
    unsigned char const * qEnd = q + nStringLength;

    if (pEnd - p < qEnd - q)
        return false;

    while (q != qEnd)
    {
        sal_Unicode c1 = *p++;
        sal_Unicode c2 = *q++;
        if (c1 != c2)
            return false;
    }

    *pBegin = p;
    return true;
}

bool scanStringLiteral(sal_Unicode const ** pBegin, sal_Unicode const * pEnd,
                       OUString * pString)
{
    sal_Unicode const * p = *pBegin;

    if (p == pEnd || *p++ != '"')
        return false;

    OUStringBuffer aBuffer;
    for (;;)
    {
        if (p == pEnd)
            return false;
        sal_Unicode c = *p++;
        if (c == '"')
            break;
        if (c == '\\')
        {
            if (p == pEnd)
                return false;
            c = *p++;
            if (c != '"' && c != '\\')
                return false;
        }
        aBuffer.append(c);
    }

    *pBegin = p;
    *pString = aBuffer.makeStringAndClear();
    return true;
}

}

Regexp Regexp::parse(OUString const & rRegexp)
{
    // Detect an input of '<scheme>' as an abbreviation of '"<scheme>:".*'
    // where <scheme> is as defined in RFC 2396:
    if (isScheme(rRegexp, false))
        return Regexp(Regexp::KIND_PREFIX,
                      rRegexp + ":",
                      false,
                      OUString(),
                      false,
                      OUString());

    sal_Unicode const * p = rRegexp.getStr();
    sal_Unicode const * pEnd = p + rRegexp.getLength();

    OUString aPrefix;
    scanStringLiteral(&p, pEnd, &aPrefix);

    if (p == pEnd)
        throw lang::IllegalArgumentException();

    // This and the matchString() calls below are some of the few places where
    // RTL_CONSTASCII_STRINGPARAM() should NOT be removed.
    // (c.f. https://gerrit.libreoffice.org/3117)
    if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM(".*")))
    {
        if (p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_PREFIX, aPrefix, false, OUString(),
                      false, OUString());
    }
    else if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("(.*)->")))
    {
        OUString aReversePrefix;
        scanStringLiteral(&p, pEnd, &aReversePrefix);

        if (!matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("\\1"))
            || p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_PREFIX, aPrefix, false, OUString(),
                      true, aReversePrefix);
    }
    else if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("([/?#].*)?")))
    {
        if (p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_AUTHORITY, aPrefix, false, OUString(),
                      false, OUString());
    }
    else if (matchString(&p, pEnd,
                         RTL_CONSTASCII_STRINGPARAM("(([/?#].*)?)->")))
    {
        OUString aReversePrefix;
        if (!(scanStringLiteral(&p, pEnd, &aReversePrefix)
              && matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("\\1"))
              && p == pEnd))
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_AUTHORITY, aPrefix, false, OUString(),
                      true, aReversePrefix);
    }
    else
    {
        bool bOpen = false;
        if (p != pEnd && *p == '(')
        {
            ++p;
            bOpen = true;
        }

        if (!matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("[^/?#]")))
            throw lang::IllegalArgumentException();

        if (p == pEnd || (*p != '*' && *p != '+'))
            throw lang::IllegalArgumentException();
        bool bEmptyDomain = *p++ == '*';

        OUString aInfix;
        scanStringLiteral(&p, pEnd, &aInfix);

        if (!matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("([/?#].*)?")))
            throw lang::IllegalArgumentException();

        OUString aReversePrefix;
        if (bOpen
            && !(matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM(")->"))
                 && scanStringLiteral(&p, pEnd, &aReversePrefix)
                 && matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("\\1"))))
            throw lang::IllegalArgumentException();

        if (p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_DOMAIN, aPrefix, bEmptyDomain, aInfix,
                      bOpen, aReversePrefix);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
