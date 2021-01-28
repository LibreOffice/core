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
void lcl_appendWithSpace(OUStringBuffer& sBuff, std::u16string_view sStr)
{
    sBuff.append(" ");
    sBuff.append(sStr);
}

OUString lcl_DataTypetoFbTypeName(sal_Int32 eType)
{
    switch (eType)
    {
        case DataType::CHAR:
        case DataType::BINARY:
            return "CHAR";
        case DataType::VARCHAR:
        case DataType::VARBINARY:
            return "VARCHAR";
        case DataType::TINYINT: // no such type in Firebird
        case DataType::SMALLINT:
            return "SMALLINT";
        case DataType::INTEGER:
            return "INTEGER";
        case DataType::BIGINT:
            return "BIGINT";
        case DataType::NUMERIC:
            return "NUMERIC";
        case DataType::DECIMAL:
            return "DECIMAL";
        case DataType::BOOLEAN:
            return "BOOLEAN";
        case DataType::LONGVARCHAR:
        case DataType::LONGVARBINARY:
        case DataType::CLOB:
        case DataType::BLOB:
        case DataType::OTHER:
            return "BLOB";
        case DataType::DATE:
            return "DATE";
        case DataType::TIME:
            return "TIME";
        case DataType::TIMESTAMP:
            return "TIMESTAMP";
        case DataType::DOUBLE:
        case DataType::REAL:
            return "DOUBLE PRECISION";
        case DataType::FLOAT:
            return "FLOAT";
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
            return "SUB_TYPE 1";
        case DataType::LONGVARBINARY:
            return "SUB_TYPE -9546";
        case DataType::BINARY:
        case DataType::VARBINARY:
            return "CHARACTER SET OCTETS";
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

    rSql.append(",");
    rSql.append("PRIMARY KEY(");
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
    sSql.append("CREATE TABLE ");
    sSql.append(getTableName());

    lcl_appendWithSpace(sSql, u"("); // column declaration
    auto& rColumns = getColumnDef();
    auto columnIter = rColumns.cbegin();
    while (columnIter != rColumns.end())
    {
        lcl_appendWithSpace(sSql, columnIter->getName());
        lcl_appendWithSpace(sSql, lcl_DataTypetoFbTypeName(columnIter->getDataType()));

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
                sSql.append(OUString::number(*it));
                ++it;
                if (it != params.end())
                    sSql.append(",");
            }
            sSql.append(")"); // end of param declaration
        }

        // special modifiers here, based on type (e.g. charset, subtype)
        OUString sModifier = lcl_getTypeModifier(columnIter->getDataType());
        if (!sModifier.isEmpty())
            lcl_appendWithSpace(sSql, sModifier);

        if (columnIter->isAutoIncremental())
        {
            lcl_appendWithSpace(sSql, u"GENERATED BY DEFAULT AS IDENTITY (START WITH ");

            // start with 0:
            // HSQLDB: first value will be 0.
            // Firebird: first value will be 1.
            sSql.append(columnIter->getStartValue() - 1);
            sSql.append(")");
        }
        else if (!columnIter->isNullable())
            lcl_appendWithSpace(sSql, u"NOT NULL");

        if (columnIter->isCaseInsensitive())
            lcl_appendWithSpace(sSql, u"COLLATE UNICODE_CI");

        const OUString& sDefaultVal = columnIter->getDefault();
        if (!sDefaultVal.isEmpty())
        {
            lcl_appendWithSpace(sSql, u"DEFAULT");
            if (sDefaultVal.equalsIgnoreAsciiCase("NOW"))
                lcl_appendWithSpace(sSql, u"\'NOW\'"); // Fb likes it single quoted
            else
                lcl_appendWithSpace(sSql, sDefaultVal);
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
