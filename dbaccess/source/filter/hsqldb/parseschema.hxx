/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/embed/XStorage.hpp>
#include <vector>
#include <map>

#include "columndef.hxx"

namespace dbahsql
{
using SqlStatementVector = std::vector<OUString>;

class SchemaParser
{
private:
    css::uno::Reference<css::embed::XStorage>& m_rStorage;

    // column type for each table. It is filled after parsing schema.
    std::map<OUString, std::vector<ColumnDefinition>> m_ColumnTypes;

    // root element's position of data for each table
    std::map<OUString, std::vector<sal_Int32>> m_Indexes;

    // primary keys of each table
    std::map<OUString, std::vector<OUString>> m_PrimaryKeys;

    SqlStatementVector m_sCreateStatements;
    SqlStatementVector m_sAlterStatements;

public:
    explicit SchemaParser(css::uno::Reference<css::embed::XStorage>& rStorage);

    /**
     * Parses table definitions contained by a file called "script" in storage.
     */
    void parseSchema();

    /**
     * @return A vector of schema definition SQL strings in Firebird dialect.
     */
    const SqlStatementVector& getCreateStatements() const { return m_sCreateStatements; }

    /**
     * @return A vector of alter SQL strings in Firebird dialect.
     */
    const SqlStatementVector& getAlterStatements() const { return m_sAlterStatements; }

    /**
     * Returns the column types of a table. It should not be called before
     * calling parseSchema().
     *
     * @param sTableName name of the table.
     *
     * @return A vector of column descriptors.
     */
    std::vector<ColumnDefinition> getTableColumnTypes(const OUString& sTableName) const;

    /**
     * Returns a vector of indexes for each table. These indexes are used for
     * locating the data related to the actual table in the binary file.
     *
     * The indexes point to root elements of AVL trees. Each node of the tree
     * contains one row.
     */
    const std::map<OUString, std::vector<sal_Int32>>& getTableIndexes() const;

    /**
     * Returns a vector of column names for each table. These columns are the
     * primary keys of the table.
     */
    const std::map<OUString, std::vector<OUString>>& getPrimaryKeys() const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
