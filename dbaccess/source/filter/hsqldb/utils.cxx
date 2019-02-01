
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

#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <connectivity/dbexception.hxx>

#include <sal/log.hxx>

#include "utils.hxx"

using namespace dbahsql;

namespace
{
int getHexValue(sal_Unicode c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else
    {
        return -1;
    }
}

} // unnamed namespace

//Convert ascii escaped unicode to utf-8
OUString utils::convertToUTF8(const OString& original)
{
    OUString res = OStringToOUString(original, RTL_TEXTENCODING_UTF8);
    for (sal_Int32 i = 0;;)
    {
        i = res.indexOf("\\u", i);
        if (i == -1)
        {
            break;
        }
        i += 2;
        if (res.getLength() - i >= 4)
        {
            bool escape = true;
            sal_Unicode c = 0;
            for (sal_Int32 j = 0; j != 4; ++j)
            {
                auto const n = getHexValue(res[i + j]);
                if (n == -1)
                {
                    escape = false;
                    break;
                }
                c = (c << 4) | n;
            }
            if (escape)
            {
                i -= 2;
                res = res.replaceAt(i, 6, OUString(c));
                ++i;
            }
        }
    }
    return res;
}

OUString utils::getTableNameFromStmt(const OUString& sSql)
{
    auto stmtComponents = comphelper::string::split(sSql, sal_Unicode(u' '));
    assert(stmtComponents.size() > 2);
    auto wordIter = stmtComponents.begin();

    if (*wordIter == "CREATE" || *wordIter == "ALTER")
        ++wordIter;
    if (*wordIter == "CACHED")
        ++wordIter;
    if (*wordIter == "TABLE")
        ++wordIter;

    // it may contain spaces if it's put into apostrophes.
    if (wordIter->indexOf("\"") >= 0)
    {
        sal_Int32 nAposBegin = sSql.indexOf("\"");
        sal_Int32 nAposEnd = nAposBegin;
        bool bProperEndAposFound = false;
        while (!bProperEndAposFound)
        {
            nAposEnd = sSql.indexOf("\"", nAposEnd + 1);
            if (sSql[nAposEnd - 1] != u'\\')
                bProperEndAposFound = true;
        }
        OUString result = sSql.copy(nAposBegin, nAposEnd - nAposBegin + 1);
        return result;
    }

    // next word is the table's name
    // it might stuck together with the column definitions.
    sal_Int32 nParenPos = wordIter->indexOf("(");
    if (nParenPos > 0)
        return wordIter->copy(0, nParenPos);
    else
        return *wordIter;
}

void utils::ensureFirebirdTableLength(const OUString& sName)
{
    if (sName.getLength() > 30) // Firebird limitation
    {
        constexpr char NAME_TOO_LONG[] = "Firebird 3 doesn't currently support table names of more "
                                         "than 30 characters, please shorten your table names in "
                                         "the original file and try again.";
        dbtools::throwGenericSQLException(NAME_TOO_LONG,
                                          ::comphelper::getProcessComponentContext());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
