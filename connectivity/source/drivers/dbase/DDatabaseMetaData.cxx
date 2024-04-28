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

#include <dbase/DDatabaseMetaData.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <FDatabaseMetaDataResultSet.hxx>
#include <dbase/DIndex.hxx>
#include <connectivity/FValue.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>
#include <ucbhelper/content.hxx>

using namespace ::comphelper;
using namespace connectivity::dbase;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;

ODbaseDatabaseMetaData::ODbaseDatabaseMetaData(::connectivity::file::OConnection* _pCon)    :ODatabaseMetaData(_pCon)
{
}

ODbaseDatabaseMetaData::~ODbaseDatabaseMetaData()
{
}

Reference< XResultSet > ODbaseDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);

    ODatabaseMetaDataResultSet::ORows aRows;
    aRows.reserve(10);
    ODatabaseMetaDataResultSet::ORow aRow
    {
        ODatabaseMetaDataResultSet::getEmptyValue(),
        new ORowSetValueDecorator(OUString("VARCHAR")),
        new ORowSetValueDecorator(DataType::VARCHAR),
        new ORowSetValueDecorator(sal_Int32(254)),
        ODatabaseMetaDataResultSet::getQuoteValue(),
        ODatabaseMetaDataResultSet::getQuoteValue(),
        new ORowSetValueDecorator(OUString("length")),
        new ORowSetValueDecorator(sal_Int32(ColumnValue::NULLABLE)),
        ODatabaseMetaDataResultSet::get1Value(),
        new ORowSetValueDecorator(sal_Int32(ColumnSearch::FULL)),
        ODatabaseMetaDataResultSet::get1Value(),
        ODatabaseMetaDataResultSet::get0Value(),
        ODatabaseMetaDataResultSet::get0Value(),
        new ORowSetValueDecorator(OUString("C")),
        ODatabaseMetaDataResultSet::get0Value(),
        ODatabaseMetaDataResultSet::get0Value(),
        ODatabaseMetaDataResultSet::getEmptyValue(),
        ODatabaseMetaDataResultSet::getEmptyValue(),
        new ORowSetValueDecorator(sal_Int32(10))
    };

    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("LONGVARCHAR"));
    aRow[2] = new ORowSetValueDecorator(DataType::LONGVARCHAR);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(2147483647));
    aRow[6] = new ORowSetValueDecorator();
    aRow[13] = new ORowSetValueDecorator(OUString("M"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
    aRow[2] = new ORowSetValueDecorator(DataType::DATE);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(10));
    aRow[13] = new ORowSetValueDecorator(OUString("D"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("BOOLEAN"));
    aRow[2] = new ORowSetValueDecorator(DataType::BIT);
    aRow[3] = ODatabaseMetaDataResultSet::get1Value();
    aRow[4] = ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
    aRow[6] = new ORowSetValueDecorator(OUString());
    aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
    aRow[13] = new ORowSetValueDecorator(OUString("L"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("DOUBLE"));
    aRow[2] = new ORowSetValueDecorator(DataType::DOUBLE);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(8));
    aRow[13] = new ORowSetValueDecorator(OUString("B"));
    aRows.push_back(aRow);

    aRow[11] = new ORowSetValueDecorator(ORowSetValue(true));
    aRow[13] = new ORowSetValueDecorator(OUString("Y"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
    aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
    aRow[11] = new ORowSetValueDecorator(ORowSetValue(false));
    aRow[13] = new ORowSetValueDecorator(OUString("T"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("INTEGER"));
    aRow[2] = new ORowSetValueDecorator(DataType::INTEGER);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(10));
    aRow[13] = new ORowSetValueDecorator(OUString("I"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("DECIMAL"));
    aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
    aRow[6] = new ORowSetValueDecorator(OUString("length,scale"));
    aRow[13] = new ORowSetValueDecorator(OUString("F"));
    aRows.push_back(aRow);

    aRow[1] = new ORowSetValueDecorator(OUString("NUMERIC"));
    aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
    aRow[3] = new ORowSetValueDecorator(sal_Int32(16));
    aRow[13] = new ORowSetValueDecorator(OUString("N"));
    aRow[15] = new ORowSetValueDecorator(sal_Int32(16));
    aRows.push_back(aRow);

    pResult->setRows(std::move(aRows));
    return pResult;
}

Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern,
        const OUString& columnNamePattern )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(19);

    try
    {
        aRow[10] = new ORowSetValueDecorator(sal_Int32(10));
        for (auto& tabName : xNames->getElementNames())
        {
            if (match(tableNamePattern, tabName, '\0'))
            {
                Reference<XColumnsSupplier> xTable(xNames->getByName(tabName), css::uno::UNO_QUERY);
                OSL_ENSURE(xTable.is(),"Table not found! Normally an exception had to be thrown here!");
                aRow[3] = new ORowSetValueDecorator(tabName);

                Reference< XNameAccess> xColumns = xTable->getColumns();
                if(!xColumns.is())
                    throw SQLException();

                Reference< XPropertySet> xColumn;
                sal_Int32 i = 0;
                for (auto& colName : xColumns->getElementNames())
                {
                    ++i;
                    if (match(columnNamePattern, colName, '\0'))
                    {
                        aRow[4] = new ORowSetValueDecorator(colName);

                        xColumn.set(xColumns->getByName(colName), css::uno::UNO_QUERY);
                        OSL_ENSURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                        aRow[5] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
                        aRow[6] = new ORowSetValueDecorator(getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))));
                        aRow[7] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
                        aRow[9] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                        aRow[11] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))));
                        aRow[13] = new ORowSetValueDecorator(getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))));
                        switch(aRow[5]->getValue().getInt32())
                        {
                        case DataType::CHAR:
                        case DataType::VARCHAR:
                            aRow[16] = new ORowSetValueDecorator(sal_Int32(254));
                            break;
                        case DataType::LONGVARCHAR:
                            aRow[16] = new ORowSetValueDecorator(sal_Int32(65535));
                            break;
                        default:
                            aRow[16] = new ORowSetValueDecorator(sal_Int32(0));
                        }
                        aRow[17] = new ORowSetValueDecorator(i);
                        switch(aRow[11]->getValue().getInt32())
                        {
                        case ColumnValue::NO_NULLS:
                            aRow[18] = new ORowSetValueDecorator(OUString("NO"));
                            break;
                        case ColumnValue::NULLABLE:
                            aRow[18] = new ORowSetValueDecorator(OUString("YES"));
                            break;
                        default:
                            aRow[18] = new ORowSetValueDecorator(OUString());
                        }
                        aRows.push_back(aRow);
                    }
                }
            }
        }
    }
    catch (const WrappedTargetException& e)
    {
        SQLException aSql;
        if (e.TargetException >>= aSql)
            throw aSql;
        throw WrappedTargetRuntimeException(e.Message, e.Context, e.TargetException);
    }
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    pResult->setRows(std::move(aRows));

    return pResult;
}

Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getIndexInfo(
    const Any& /*catalog*/, const OUString& /*schema*/, const OUString& table,
        sal_Bool unique, sal_Bool /*approximate*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
                throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
                throw SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(14);

    aRow[5]     = new ORowSetValueDecorator(OUString());
    aRow[10]    = new ORowSetValueDecorator(OUString("A"));

    Reference< XIndexesSupplier> xTable(
        xNames->getByName(table), css::uno::UNO_QUERY);
    aRow[3] = new ORowSetValueDecorator(table);
    aRow[7] = new ORowSetValueDecorator(sal_Int32(3));

    Reference< XNameAccess> xIndexes = xTable->getIndexes();
    if(!xIndexes.is())
        throw SQLException();

    Reference< XPropertySet> xIndex;
    for (auto& idxName : xIndexes->getElementNames())
    {
        xIndex.set(xIndexes->getByName(idxName), css::uno::UNO_QUERY);
        OSL_ENSURE(xIndex.is(),"Indexes contains a column who isn't a fastpropertyset!");

        if(unique && !getBOOL(xIndex->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            continue;
        aRow[4] = new ORowSetValueDecorator(ORowSetValue(getBOOL(xIndex->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE)))));
        aRow[6] = new ORowSetValueDecorator(idxName);

        auto pIndex = dynamic_cast<ODbaseIndex*>(xIndex.get());
        if(pIndex)
        {
            aRow[11] = new ORowSetValueDecorator(static_cast<sal_Int32>(pIndex->getHeader().db_maxkeys));
            aRow[12] = new ORowSetValueDecorator(static_cast<sal_Int32>(pIndex->getHeader().db_pagecount));
        }

        Reference<XColumnsSupplier> xColumnsSup(xIndex,UNO_QUERY);
        Reference< XNameAccess> xColumns = xColumnsSup->getColumns();

        sal_Int32 j = 0;
        for (auto& colName : xColumns->getElementNames())
        {
            aRow[8] = new ORowSetValueDecorator(++j);
            aRow[9] = new ORowSetValueDecorator(colName);
            aRows.push_back(aRow);
        }
    }

    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eIndexInfo);
    pResult->setRows(std::move(aRows));
    return pResult;
}

OUString SAL_CALL ODbaseDatabaseMetaData::getURL(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return "sdbc:dbase:" + m_pConnection->getURL();
}

sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxCharLiteralLength(  )
{
    return 254;
}

sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnNameLength(  )
{
    return 10;
}

sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 1;
}

sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInTable(  )
{
    return 128;
}

sal_Bool SAL_CALL ODbaseDatabaseMetaData::supportsAlterTableWithAddColumn(  )
{
    return true;
}

sal_Bool SAL_CALL ODbaseDatabaseMetaData::supportsAlterTableWithDropColumn(  )
{
    return false;
}

sal_Bool SAL_CALL ODbaseDatabaseMetaData::isReadOnly(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bReadOnly = false;
    ::ucbhelper::Content aFile(m_pConnection->getContent(),Reference< XCommandEnvironment >(), comphelper::getProcessComponentContext());
    aFile.getPropertyValue("IsReadOnly") >>= bReadOnly;

    return bReadOnly;
}

bool ODbaseDatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    return true;
}

bool ODbaseDatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
