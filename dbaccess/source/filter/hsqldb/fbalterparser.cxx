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

#include "fbalterparser.hxx"
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

namespace dbahsql
{
OUString FbAlterStmtParser::compose() const
{
    if (getActionType() == AlterAction::UNKNOWN)
    {
        SAL_WARN("dbaccess", "Unknown type of ALTER statement in FbAlterStmtParser");
        return OUString{};
    }
    else if (getActionType() == AlterAction::ADD_FOREIGN)
        return getStatement(); // do nothing with that
    OUStringBuffer sSql("ALTER TABLE ");
    sSql.append(getTableName());

    if (getActionType() == AlterAction::IDENTITY_RESTART)
    {
        sSql.append(" ALTER COLUMN ");
    }
    sSql.append(getColumnName());
    sSql.append(" RESTART WITH ");

    // Firebird: restart with 0 means the first number is 1, not 0.
    sSql.append(getIdentityParam() - 1);

    return sSql.makeStringAndClear();
}

} // dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
