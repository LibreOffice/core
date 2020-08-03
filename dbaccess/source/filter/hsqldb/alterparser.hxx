/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>

namespace dbahsql
{
enum class AlterAction
{
    UNKNOWN,
    ADD_FOREIGN,
    IDENTITY_RESTART
};

class AlterStmtParser
{
private:
    OUString m_sStmt;
    OUString m_sTableName;
    OUString m_sColumnName;
    AlterAction m_eAction = AlterAction::UNKNOWN;
    sal_Int32 m_nIdentityParam = 0;

protected:
    AlterAction getActionType() const { return m_eAction; }
    OUString const& getColumnName() const { return m_sColumnName; }
    sal_Int32 getIdentityParam() const { return m_nIdentityParam; }
    OUString const& getStatement() const { return m_sStmt; }

public:
    virtual ~AlterStmtParser() = default;

    /**
     * @return name of the table which is to be created.
     */
    OUString const& getTableName() const { return m_sTableName; }

    void parse(const OUString& sSql);

    virtual OUString compose() const = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
