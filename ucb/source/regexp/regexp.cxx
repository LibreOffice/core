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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include <regexp.hxx>

#include <cstddef>

#include "osl/diagnose.h"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

namespace unnamed_ucb_regexp {} using namespace unnamed_ucb_regexp;
    // unnamed namespaces don't work well yet...

using namespace com::sun::star;
using namespace ucb_impl;

//============================================================================
//
//  Regexp
//
//============================================================================

inline Regexp::Regexp(Kind eTheKind, rtl::OUString const & rThePrefix,
                      bool bTheEmptyDomain, rtl::OUString const & rTheInfix,
                      bool bTheTranslation,
                      rtl::OUString const & rTheReversePrefix):
    m_eKind(eTheKind),
    m_aPrefix(rThePrefix),
    m_aInfix(rTheInfix),
    m_aReversePrefix(rTheReversePrefix),
    m_bEmptyDomain(bTheEmptyDomain),
    m_bTranslation(bTheTranslation)
{
    OSL_ASSERT(m_eKind == KIND_DOMAIN
               || (!m_bEmptyDomain && m_aInfix.getLength() == 0));
    OSL_ASSERT(m_bTranslation || m_aReversePrefix.getLength() == 0);
}

//============================================================================
namespace unnamed_ucb_regexp {

bool matchStringIgnoreCase(sal_Unicode const ** pBegin,
                           sal_Unicode const * pEnd,
                           rtl::OUString const & rString)
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

bool Regexp::matches(rtl::OUString const & rString,
                     rtl::OUString * pTranslation, bool * pTranslated) const
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
                    rtl::OUStringBuffer aBuffer(m_aReversePrefix);
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

//============================================================================
namespace unnamed_ucb_regexp {

inline bool isAlpha(sal_Unicode c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline bool isDigit(sal_Unicode c)
{
    return c >= '0' && c <= '9';
}

bool isScheme(rtl::OUString const & rString, bool bColon)
{
    // Return true if rString matches <scheme> (plus a trailing ":" if bColon
    // is true) from RFC 2396:
    sal_Unicode const * p = rString.getStr();
    sal_Unicode const * pEnd = p + rString.getLength();
    if (p != pEnd && isAlpha(*p))
        for (++p;;)
        {
            if (p == pEnd)
                return !bColon;
            sal_Unicode c = *p++;
            if (!(isAlpha(c) || isDigit(c)
                  || c == '+' || c == '-' || c == '.'))
                return bColon && c == ':' && p == pEnd;
        }
    return false;
}

void appendStringLiteral(rtl::OUStringBuffer * pBuffer,
                         rtl::OUString const & rString)
{
    OSL_ASSERT(pBuffer);

    pBuffer->append(sal_Unicode('"'));
    sal_Unicode const * p = rString.getStr();
    sal_Unicode const * pEnd = p + rString.getLength();
    while (p != pEnd)
    {
        sal_Unicode c = *p++;
        if (c == '"' || c == '\\')
            pBuffer->append(sal_Unicode('\\'));
        pBuffer->append(c);
    }
    pBuffer->append(sal_Unicode('"'));
}

}

rtl::OUString Regexp::getRegexp(bool bReverse) const
{
    if (m_bTranslation)
    {
        rtl::OUStringBuffer aBuffer;
        if (bReverse)
        {
            if (m_aReversePrefix.getLength() != 0)
                appendStringLiteral(&aBuffer, m_aReversePrefix);
        }
        else
        {
            if (m_aPrefix.getLength() != 0)
                appendStringLiteral(&aBuffer, m_aPrefix);
        }
        switch (m_eKind)
        {
            case KIND_PREFIX:
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("(.*)"));
                break;

            case KIND_AUTHORITY:
                aBuffer.
                    appendAscii(RTL_CONSTASCII_STRINGPARAM("(([/?#].*)?)"));
                break;

            case KIND_DOMAIN:
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("([^/?#]"));
                aBuffer.append(sal_Unicode(m_bEmptyDomain ? '*' : '+'));
                if (m_aInfix.getLength() != 0)
                    appendStringLiteral(&aBuffer, m_aInfix);
                aBuffer.
                    appendAscii(RTL_CONSTASCII_STRINGPARAM("([/?#].*)?)"));
                break;
        }
        aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("->"));
        if (bReverse)
        {
            if (m_aPrefix.getLength() != 0)
                appendStringLiteral(&aBuffer, m_aPrefix);
        }
        else
        {
            if (m_aReversePrefix.getLength() != 0)
                appendStringLiteral(&aBuffer, m_aReversePrefix);
        }
        aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\\1"));
        return aBuffer.makeStringAndClear();
    }
    else if (m_eKind == KIND_PREFIX && isScheme(m_aPrefix, true))
        return m_aPrefix.copy(0, m_aPrefix.getLength() - 1);
    else
    {
        rtl::OUStringBuffer aBuffer;
        if (m_aPrefix.getLength() != 0)
            appendStringLiteral(&aBuffer, m_aPrefix);
        switch (m_eKind)
        {
            case KIND_PREFIX:
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM(".*"));
                break;

            case KIND_AUTHORITY:
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("([/?#].*)?"));
                break;

            case KIND_DOMAIN:
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("[^/?#]"));
                aBuffer.append(sal_Unicode(m_bEmptyDomain ? '*' : '+'));
                if (m_aInfix.getLength() != 0)
                    appendStringLiteral(&aBuffer, m_aInfix);
                aBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("([/?#].*)?"));
                break;
        }
        return aBuffer.makeStringAndClear();
    }
}

//============================================================================
namespace unnamed_ucb_regexp {

bool matchString(sal_Unicode const ** pBegin, sal_Unicode const * pEnd,
                 sal_Char const * pString, size_t nStringLength)
{
    sal_Unicode const * p = *pBegin;

    sal_uChar const * q = reinterpret_cast< sal_uChar const * >(pString);
    sal_uChar const * qEnd = q + nStringLength;

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
                       rtl::OUString * pString)
{
    sal_Unicode const * p = *pBegin;

    if (p == pEnd || *p++ != '"')
        return false;

    rtl::OUStringBuffer aBuffer;
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

Regexp Regexp::parse(rtl::OUString const & rRegexp)
{
    // Detect an input of '<scheme>' as an abbreviation of '"<scheme>:".*'
    // where <scheme> is as defined in RFC 2396:
    if (isScheme(rRegexp, false))
        return Regexp(Regexp::KIND_PREFIX,
                      rRegexp
                          + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(":")),
                      false,
                      rtl::OUString(),
                      false,
                      rtl::OUString());

    sal_Unicode const * p = rRegexp.getStr();
    sal_Unicode const * pEnd = p + rRegexp.getLength();

    rtl::OUString aPrefix;
    scanStringLiteral(&p, pEnd, &aPrefix);

    if (p == pEnd)
        throw lang::IllegalArgumentException();

    if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM(".*")))
    {
        if (p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_PREFIX, aPrefix, false, rtl::OUString(),
                      false, rtl::OUString());
    }
    else if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("(.*)->")))
    {
        rtl::OUString aReversePrefix;
        scanStringLiteral(&p, pEnd, &aReversePrefix);

        if (!matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("\\1"))
            || p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_PREFIX, aPrefix, false, rtl::OUString(),
                      true, aReversePrefix);
    }
    else if (matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("([/?#].*)?")))
    {
        if (p != pEnd)
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_AUTHORITY, aPrefix, false, rtl::OUString(),
                      false, rtl::OUString());
    }
    else if (matchString(&p, pEnd,
                         RTL_CONSTASCII_STRINGPARAM("(([/?#].*)?)->")))
    {
        rtl::OUString aReversePrefix;
        if (!(scanStringLiteral(&p, pEnd, &aReversePrefix)
              && matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("\\1"))
              && p == pEnd))
            throw lang::IllegalArgumentException();

        return Regexp(Regexp::KIND_AUTHORITY, aPrefix, false, rtl::OUString(),
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

        rtl::OUString aInfix;
        scanStringLiteral(&p, pEnd, &aInfix);

        if (!matchString(&p, pEnd, RTL_CONSTASCII_STRINGPARAM("([/?#].*)?")))
            throw lang::IllegalArgumentException();

        rtl::OUString aReversePrefix;
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
