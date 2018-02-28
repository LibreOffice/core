/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_PARSECHEMA_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_PARSECHEMA_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <vector>
#include <map>

namespace dbahsql
{
typedef std::vector<OUString> SqlStatementVector;

class SchemaParser
{
private:
    css::uno::Reference<css::embed::XStorage>& m_rStorage;

    // column type for each table. It is filled after parsing schema.
    std::map<OUString, std::vector<sal_Int32>> m_ColumnTypes;

public:
    explicit SchemaParser(css::uno::Reference<css::embed::XStorage>& rStorage);

    SqlStatementVector parseSchema();

    std::vector<sal_Int32> getTableColumnTypes(const OUString& sTableName) const;
};
}

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_PARSESCHEMA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
