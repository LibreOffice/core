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

#include "fbcreateparser.hxx"
#include "columndef.hxx"
#include "utils.hxx"

#include <com/sun/star/sdbc/DataType.hpp>

#include <rtl/ustrbuf.hxx>

using namespace css::sdbc;

namespace
{
OUString lcl_DataTypetoFbTypeName(sal_Int32 eType)
{
    switch (eType)
    {
        case DataType::CHAR:
        case DataType::BINARY:
            return u"CHAR"_ustr;
        case DataType::VARCHAR:
        case DataType::VARBINARY:
            return u"VARCHAR"_ustr;
        case DataType::TINYINT: // no such type in Firebird
        case DataType::SMALLINT:
            return u"SMALLINT"_ustr;
        case DataType::INTEGER:
            return u"INTEGER"_ustr;
        case DataType::BIGINT:
            return u"BIGINT"_ustr;
        case DataType::NUMERIC:
            return u"NUMERIC"_ustr;
        case DataType::DECIMAL:
            return u"DECIMAL"_ustr;
        case DataType::BOOLEAN:
            return u"BOOLEAN"_ustr;
        case DataType::LONGVARCHAR:
        case DataType::LONGVARBINARY:
        case DataType::CLOB:
        case DataType::BLOB:
        case DataType::OTHER:
            return u"BLOB"_ustr;
        case DataType::DATE:
            return u"DATE"_ustr;
        case DataType::TIME:
            return u"TIME"_ustr;
        case DataType::TIMESTAMP:
            return u"TIMESTAMP"_ustr;
        case DataType::DOUBLE:
        case DataType::REAL:
            return u"DOUBLE PRECISION"_ustr;
        case DataType::FLOAT:
            return u"FLOAT"_ustr;
        default:
            assert(false);
            return OUString();
    }
}

OUString lcl_getTypeModifier(sal_Int32 eType)
{
    // TODO bind -9546 magic number to a common definition. It also appears
    // in the connectivity module.
    switch (eType)
    {
        case DataType::CLOB:
        case DataType::LONGVARCHAR:
            return u"SUB_TYPE 1"_ustr;
        case DataType::LONGVARBINARY:
            return u"SUB_TYPE -9546"_ustr;
        case DataType::BINARY:
        case DataType::VARBINARY:
            return u"CHARACTER SET OCTETS"_ustr;
        default:
            return OUString();
    }
}

} // unnamed namespace

namespace dbahsql
{
void FbCreateStmtParser::appendPrimaryKeyPart(OUStringBuffer& rSql) const
{
    const std::vector<OUString>& sPrimaryKeys = getPrimaryKeys();
    if (sPrimaryKeys.empty())
        return; // no primary key specified

    rSql.append(",PRIMARY KEY(");
    auto it = sPrimaryKeys.cbegin();
    while (it != sPrimaryKeys.end())
    {
        rSql.append(*it);
        ++it;
        if (it != sPrimaryKeys.end())
            rSql.append(",");
    }

    rSql.append(")"); // end of primary key declaration
}

void FbCreateStmtParser::ensureProperTableLengths() const
{
    const std::vector<ColumnDefinition>& rColumns = getColumnDef();
    for (const auto& col : rColumns)
        utils::ensureFirebirdTableLength(col.getName());
}

OUString FbCreateStmtParser::compose() const
{
    ensureProperTableLengths();
    OUStringBuffer sSql(128);
    sSql.append("CREATE TABLE " + getTableName() + " ("); // column declaration

    auto& rColumns = getColumnDef();
    auto columnIter = rColumns.cbegin();
    while (columnIter != rColumns.end())
    {
        sSql.append(" " + columnIter->getName() + " "
                    + lcl_DataTypetoFbTypeName(columnIter->getDataType()));

        std::vector<sal_Int32> params{ columnIter->getParams() };

        if (columnIter->getDataType() == DataType::NUMERIC
            || columnIter->getDataType() == DataType::DECIMAL)
        {
            // max precision is 18 here
            if (params.at(0) > 18)
                params[0] = 18;
        }

        // Firebird SQL dialect does not like parameters for TIMESTAMP
        if (!params.empty() && columnIter->getDataType() != DataType::TIMESTAMP)
        {
            sSql.append("(");
            auto it = params.cbegin();
            while (it != params.end())
            {
                sSql.append(*it);
                ++it;
                if (it != params.end())
                    sSql.append(",");
            }
            sSql.append(")"); // end of param declaration
        }

        // special modifiers here, based on type (e.g. charset, subtype)
        OUString sModifier = lcl_getTypeModifier(columnIter->getDataType());
        if (!sModifier.isEmpty())
            sSql.append(" " + sModifier);

        if (columnIter->isAutoIncremental())
        {
            // start with 0:
            // HSQLDB: first value will be 0.
            // Firebird: first value will be 1.
            sSql.append(" GENERATED BY DEFAULT AS IDENTITY (START WITH "
                        + OUString::number(columnIter->getStartValue() - 1) + ")");
        }
        else if (!columnIter->isNullable())
            sSql.append(" NOT NULL");

        if (columnIter->isCaseInsensitive())
            sSql.append(" COLLATE UNICODE_CI");

        const OUString& sDefaultVal = columnIter->getDefault();
        if (!sDefaultVal.isEmpty())
        {
            sSql.append(" DEFAULT ");
            if (sDefaultVal.equalsIgnoreAsciiCase("NOW"))
                sSql.append("'NOW'"); // Fb likes it single quoted
            else
                sSql.append(sDefaultVal);
        }

        ++columnIter;
        if (columnIter != rColumns.end())
            sSql.append(",");
    }

    appendPrimaryKeyPart(sSql);

    sSql.append(")"); // end of column declaration
    return sSql.makeStringAndClear();
}

} // dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
