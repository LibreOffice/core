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

#include <file/FStringFunctions.hxx>

#include <comphelper/string.hxx>
#include <rtl/ustrbuf.hxx>

using namespace connectivity;
using namespace connectivity::file;

ORowSetValue OOp_Upper::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    return lhs.getString().toAsciiUpperCase();
}

ORowSetValue OOp_Lower::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    return lhs.getString().toAsciiLowerCase();
}

ORowSetValue OOp_Ascii::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;
    OString sStr(OUStringToOString(lhs.getString(), RTL_TEXTENCODING_ASCII_US));
    sal_Int32 nAscii = sStr.toChar();
    return nAscii;
}

ORowSetValue OOp_CharLength::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    return lhs.getString().getLength();
}

ORowSetValue OOp_Char::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (lhs.empty())
        return ORowSetValue();

    OUStringBuffer sRet(static_cast<sal_Int32>(lhs.size()));
    std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if (!aIter->isNull())
        {
            char c = static_cast<char>(aIter->getInt32());

            sRet.appendAscii(&c, 1);
        }
    }

    return sRet.makeStringAndClear();
}

ORowSetValue OOp_Concat::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (lhs.empty())
        return ORowSetValue();

    OUStringBuffer sRet;
    std::vector<ORowSetValue>::const_reverse_iterator aIter = lhs.rbegin();
    std::vector<ORowSetValue>::const_reverse_iterator aEnd = lhs.rend();
    for (; aIter != aEnd; ++aIter)
    {
        if (aIter->isNull())
            return ORowSetValue();

        sRet.append(aIter->getString());
    }

    return sRet.makeStringAndClear();
}

ORowSetValue OOp_Locate::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (std::any_of(lhs.begin(), lhs.end(),
                    [](const ORowSetValue& rValue) { return rValue.isNull(); }))
        return ORowSetValue();

    if (lhs.size() == 2)
        return OUString(OUString::number(lhs[0].getString().indexOf(lhs[1].getString()) + 1));

    else if (lhs.size() != 3)
        return ORowSetValue();

    return lhs[1].getString().indexOf(lhs[2].getString(), lhs[0].getInt32()) + 1;
}

ORowSetValue OOp_SubString::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (std::any_of(lhs.begin(), lhs.end(),
                    [](const ORowSetValue& rValue) { return rValue.isNull(); }))
        return ORowSetValue();

    if (lhs.size() == 2 && lhs[0].getInt32() >= sal_Int32(0))
        return lhs[1].getString().copy(lhs[0].getInt32() - 1);

    else if (lhs.size() != 3 || lhs[1].getInt32() < sal_Int32(0))
        return ORowSetValue();

    return lhs[2].getString().copy(lhs[1].getInt32() - 1, lhs[0].getInt32());
}

ORowSetValue OOp_LTrim::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    OUString sRet = lhs.getString();
    OUString sNew = sRet.trim();
    return sRet.copy(sRet.indexOf(sNew));
}

ORowSetValue OOp_RTrim::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    OUString sRet = lhs.getString();
    OUString sNew = sRet.trim();
    return sRet.copy(0, sRet.lastIndexOf(sNew[sNew.getLength() - 1]) + 1);
}

ORowSetValue OOp_Space::operate(const ORowSetValue& lhs) const
{
    if (lhs.isNull())
        return lhs;

    sal_Int32 nCount = std::max(lhs.getInt32(), sal_Int32(0));
    OUStringBuffer sRet(nCount);
    comphelper::string::padToLength(sRet, nCount, ' ');
    return sRet.makeStringAndClear();
}

ORowSetValue OOp_Replace::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (lhs.size() != 3)
        return ORowSetValue();

    OUString sStr = lhs[2].getString();
    OUString sFrom = lhs[1].getString();
    OUString sTo = lhs[0].getString();
    sal_Int32 nIndexOf = sStr.indexOf(sFrom);
    while (nIndexOf != -1)
    {
        sStr = sStr.replaceAt(nIndexOf, sFrom.getLength(), sTo);
        nIndexOf = sStr.indexOf(sFrom, nIndexOf + sTo.getLength());
    }

    return sStr;
}

ORowSetValue OOp_Repeat::operate(const ORowSetValue& lhs, const ORowSetValue& rhs) const
{
    if (lhs.isNull() || rhs.isNull())
        return lhs;

    const OUString s = lhs.getString();
    const sal_Int32 nCount = std::max(rhs.getInt32(), sal_Int32(0));
    OUStringBuffer sRet(s.getLength() * nCount);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        sRet.append(s);
    }
    return sRet.makeStringAndClear();
}

ORowSetValue OOp_Insert::operate(const std::vector<ORowSetValue>& lhs) const
{
    if (lhs.size() != 4)
        return ORowSetValue();

    OUString sStr = lhs[3].getString();

    sal_Int32 nStart = lhs[2].getInt32();
    if (nStart < 1)
        nStart = 1;
    return sStr.replaceAt(nStart - 1, lhs[1].getInt32(), lhs[0].getString());
}

ORowSetValue OOp_Left::operate(const ORowSetValue& lhs, const ORowSetValue& rhs) const
{
    if (lhs.isNull() || rhs.isNull())
        return lhs;

    OUString sRet = lhs.getString();
    sal_Int32 nCount = rhs.getInt32();
    if (nCount < 0)
        return ORowSetValue();
    return sRet.copy(0, nCount);
}

ORowSetValue OOp_Right::operate(const ORowSetValue& lhs, const ORowSetValue& rhs) const
{
    if (lhs.isNull() || rhs.isNull())
        return lhs;

    sal_Int32 nCount = rhs.getInt32();
    OUString sRet = lhs.getString();
    if (nCount < 0 || nCount >= sRet.getLength())
        return ORowSetValue();

    return sRet.copy(sRet.getLength() - nCount, nCount);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
