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
#include <sal/log.hxx>
#include "alterparser.hxx"
#include "utils.hxx"

namespace dbahsql
{
void AlterStmtParser::parse(const OUString& sSql)
{
    m_sStmt = sSql;
    if (!sSql.startsWith("ALTER"))
    {
        SAL_WARN("dbaccess", "Not an ALTER statement");
        return;
    }

    m_sTableName = utils::getTableNameFromStmt(sSql);
    auto words = comphelper::string::split(sSql, sal_Unicode(u' '));

    if (words[3] == "ALTER" && words[4] == "COLUMN")
    {
        m_sColumnName = words[5];
        if (words[6] == "RESTART" && words[7] == "WITH")
        {
            m_eAction = AlterAction::IDENTITY_RESTART;
            m_nIdentityParam = words[8].toInt32();
        }
    }
    else if (words[3] == "ADD" && words[4] == "CONSTRAINT")
    {
        m_eAction = AlterAction::ADD_FOREIGN;
    }
}

} // namespace dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
