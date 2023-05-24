/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "mysqlc_tables.hxx"

#include <connectivity/TTableHelper.hxx>

namespace connectivity::mysqlc
{
class Table : public OTableHelper
{
private:
    ::osl::Mutex& m_rMutex;
    sal_Int32 m_nPrivileges;

protected:
    void construct() override;

public:
    Table(Tables* pTables, ::osl::Mutex& rMutex,
          const css::uno::Reference<css::sdbc::XConnection>& _xConnection);
    Table(Tables* pTables, ::osl::Mutex& rMutex,
          const css::uno::Reference<css::sdbc::XConnection>& _xConnection, const OUString& rCatalog,
          const OUString& rSchema, const OUString& rName, const OUString& rType,
          const OUString& rDescription);

    // OTableHelper
    virtual ::connectivity::sdbcx::OCollection*
    createColumns(const ::std::vector<OUString>& rNames) override;
    virtual ::connectivity::sdbcx::OCollection*
    createKeys(const ::std::vector<OUString>& rNames) override;
    virtual ::connectivity::sdbcx::OCollection*
    createIndexes(const ::std::vector<OUString>& rNames) override;

    /** Returns always "RENAME TABLE " even for views.
    *
    * \return The start of the rename statement.
    * @see http://dev.mysql.com/doc/refman/5.1/de/rename-table.html
    */
    virtual OUString getRenameStart() const override;

    // XAlterTable
    /**
    * See css::sdbcx::ColumnDescriptor for details of
    * rDescriptor.
    */
    virtual void SAL_CALL
    alterColumnByName(const OUString& rColName,
                      const css::uno::Reference<css::beans::XPropertySet>& rDescriptor) override;

    virtual void SAL_CALL alterColumnByIndex(
        sal_Int32 index, const css::uno::Reference<css::beans::XPropertySet>& descriptor) override;

    /** returns the ALTER TABLE XXX statement
    */
    OUString getAlterTableColumnPart() const;
};

} // namespace connectivity::mysqlc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
