
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
