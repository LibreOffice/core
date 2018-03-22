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

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/util/Date.hpp>

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/DataType.hpp>

#include <rtl/ustrbuf.hxx>

#include "hsqlimport.hxx"
#include "parseschema.hxx"
#include "rowinputbinary.hxx"

namespace
{
using namespace css::io;
using namespace css::uno;
using namespace css::sdbc;

using ColumnTypeVector = std::vector<sal_Int32>;
using RowVector = std::vector<Any>;
using IndexVector = std::vector<sal_Int32>;

void lcl_setParams(const RowVector& row, Reference<XParameters>& xParam,
                   const ColumnTypeVector& rColTypes)
{
    assert(row.size() == rColTypes.size());
    for (size_t i = 0; i < rColTypes.size(); ++i)
    {
        switch (rColTypes.at(i))
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            {
                OUString sVal;
                if (row.at(i) >>= sVal)
                {
                    xParam->setString(i + 1, sVal);
                }
            }
            break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            {
                sal_Int16 nVal;
                if (row.at(i) >>= nVal)
                {
                    xParam->setShort(i + 1, nVal);
                }
            }
            break;
            case DataType::INTEGER:
            {
                sal_Int32 nVal;
                if (row.at(i) >>= nVal)
                {
                    xParam->setInt(i + 1, nVal);
                }
            }
            break;
            case DataType::BIGINT:
            {
                sal_Int64 nVal;
                if (row.at(i) >>= nVal)
                {
                    xParam->setLong(i + 1, nVal);
                }
            }
            break;
            case DataType::REAL:
            case DataType::FLOAT:
            case DataType::DOUBLE:
            {
                double nVal;
                if (row.at(i) >>= nVal)
                {
                    xParam->setDouble(i + 1, nVal);
                }
            }
            break;
            case DataType::NUMERIC:
            case DataType::DECIMAL:
            {
                Sequence<Any> aNumeric;
                if (row.at(i) >>= aNumeric)
                {
                    sal_Int32 nScale = 0;
                    if (aNumeric[1] >>= nScale)
                        xParam->setObjectWithInfo(i + 1, aNumeric[0], rColTypes.at(i), nScale);
                }
            }
            break;
            case DataType::DATE:
            {
                css::util::Date date;
                if (row.at(i) >>= date)
                {
                    xParam->setDate(i + 1, date);
                }
            }
            break;
            case DataType::TIME:
            {
                css::util::Time time;
                if (row.at(i) >>= time)
                {
                    xParam->setTime(i + 1, time);
                }
            }
            break;
            case DataType::TIMESTAMP:
            {
                css::util::DateTime dateTime;
                if (row.at(i) >>= dateTime)
                {
                    xParam->setTimestamp(i + 1, dateTime);
                }
            }
            break;
            case DataType::BOOLEAN:
            {
                bool bVal = false;
                if (row.at(i) >>= bVal)
                    xParam->setBoolean(i + 1, bVal);
            }
            break;
            case DataType::OTHER:
                // TODO
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
            {
                Sequence<sal_Int8> nVal;
                if (row.at(i) >>= nVal)
                {
                    xParam->setBytes(i + 1, nVal);
                }
                break;
            }
            default:
                throw WrongFormatException();
        }
    }
}

OUString lcl_createInsertStatement(const OUString& sTableName, sal_Int32 nColumnCount)
{
    assert(nColumnCount > 0);
    OUStringBuffer sql("INSERT INTO ");
    sql.append(sTableName);
    sql.append(" VALUES (");
    for (int i = 0; i < nColumnCount - 1; ++i)
    {
        sql.append("?,");
    }
    sql.append("?)");
    return sql.makeStringAndClear();
}

} // unnamed namespace

namespace dbahsql
{
using namespace css::embed;

HsqlImporter::HsqlImporter(Reference<XConnection>& rConnection, const Reference<XStorage>& rStorage)
    : m_rConnection(rConnection)
    , m_xStorage(nullptr)
{
    m_xStorage.set(rStorage);
}

void HsqlImporter::insertRow(const RowVector& xRows, const OUString& sTableName,
                             const ColumnTypeVector& rColTypes)
{
    OUString sStatement = lcl_createInsertStatement(sTableName, xRows.size());
    Reference<XPreparedStatement> xStatement = m_rConnection->prepareStatement(sStatement);

    Reference<XParameters> xParameter(xStatement, UNO_QUERY);
    assert(xParameter.is());
    xParameter->clearParameters();

    lcl_setParams(xRows, xParameter, rColTypes);
    xStatement->executeQuery();
}

void HsqlImporter::processTree(HsqlBinaryNode& rNode, HsqlRowInputStream& rStream,
                               const ColumnTypeVector& rColTypes, const OUString& sTableName)
{
    rNode.readChildren(rStream);
    std::vector<Any> row = rNode.readRow(rStream, rColTypes);
    insertRow(row, sTableName, rColTypes);

    sal_Int32 nNext = rNode.getLeft();
    if (nNext > 0)
    {
        HsqlBinaryNode aLeft{ nNext };
        processTree(aLeft, rStream, rColTypes, sTableName);
    }
    nNext = rNode.getRight();
    if (nNext > 0)
    {
        HsqlBinaryNode aRight{ nNext };
        processTree(aRight, rStream, rColTypes, sTableName);
    }
}

/**
 * Format from the indexed file position is the following:
 * <Node x20><Row>
 * Where Node is a 20 byte data, representing the rows in a binary tree:
 * <Size x4><Balance x4><Left x4> <Right x4><Parent x4>
 *
 * Size is the size of <Row>;
 * Balance: ?
 * Left/Right/Parent: File position of the Left/Right/Parent child
 */
void HsqlImporter::parseTableRows(const IndexVector& rIndexes,
                                  const std::vector<sal_Int32>& rColTypes,
                                  const OUString& sTableName)
{
    constexpr char BINARY_FILENAME[] = "data";

    if (!m_xStorage->hasByName(BINARY_FILENAME))
    {
        SAL_WARN("dbaccess", "data file does not exist in storage during hsqldb import");
        assert(false); // TODO throw error
    }

    Reference<css::io::XStream> xStream(
        m_xStorage->openStreamElement(BINARY_FILENAME, ElementModes::READ));

    HsqlRowInputStream rowInput;
    Reference<XInputStream> xInput = xStream->getInputStream();
    rowInput.setInputStream(xInput);
    for (const auto& index : rIndexes)
    {
        if (index <= 0)
            break;

        HsqlBinaryNode aNode{ index };
        processTree(aNode, rowInput, rColTypes, sTableName);
    }
    xInput->closeInput();
}

void HsqlImporter::importHsqlDatabase()
{
    assert(m_xStorage);

    SchemaParser parser(m_xStorage);
    SqlStatementVector statements = parser.parseSchema();

    // schema
    for (auto& sSql : statements)
    {
        Reference<XStatement> statement = m_rConnection->createStatement();
        statement->executeQuery(sSql);
    }

    // data
    for (const auto& tableIndex : parser.getTableIndexes())
    {
        std::vector<sal_Int32> aColTypes = parser.getTableColumnTypes(tableIndex.first);
        parseTableRows(tableIndex.second, aColTypes, tableIndex.first);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
