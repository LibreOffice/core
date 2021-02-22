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

#include <component/CDatabaseMetaData.hxx>
#include <file/FConnection.hxx>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <FDatabaseMetaDataResultSet.hxx>
#include <comphelper/types.hxx>

using namespace connectivity::component;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

OComponentDatabaseMetaData::OComponentDatabaseMetaData(OConnection* _pCon)    :ODatabaseMetaData(_pCon)
{
}

OComponentDatabaseMetaData::~OComponentDatabaseMetaData()
{
}

Reference< XResultSet > OComponentDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);

    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;

        aRows.reserve(6);
        aRow.reserve(18);

        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR")));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(65535)));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value()); // ORowSetValue((sal_Int32)ColumnValue::NULLABLE)
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(ColumnSearch::CHAR)));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(sal_Int32(10)));


        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DECIMAL"));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("BOOL"));
        aRow[2] = new ORowSetValueDecorator(DataType::BIT);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = new ORowSetValueDecorator(sal_Int32(15));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIME"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIME);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);
    }

    pResult->setRows(aRows);
    return pResult;
}

Reference< XResultSet > SAL_CALL OComponentDatabaseMetaData::getColumns(
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
    ODatabaseMetaDataResultSet::ORow  aRow(19);

    aRow[10] = new ORowSetValueDecorator(sal_Int32(10));

    Sequence< OUString> aTabNames(xNames->getElementNames());
    const OUString* pTabIter = aTabNames.getConstArray();
    const OUString* pTabEnd      = pTabIter + aTabNames.getLength();
    for(;pTabIter != pTabEnd;++pTabIter)
    {
        if(match(tableNamePattern,*pTabIter,'\0'))
        {
            const Reference< XColumnsSupplier> xTable(xNames->getByName(*pTabIter),UNO_QUERY_THROW);
            aRow[3] = new ORowSetValueDecorator(*pTabIter);

            const Reference< XNameAccess> xColumns = xTable->getColumns();
            if(!xColumns.is())
                throw SQLException();

            const Sequence< OUString> aColNames(xColumns->getElementNames());

            const OUString* pColumnIter = aColNames.getConstArray();
            const OUString* pEnd = pColumnIter + aColNames.getLength();
            Reference< XPropertySet> xColumn;
            for(sal_Int32 i=1;pColumnIter != pEnd;++pColumnIter,++i)
            {
                if(match(columnNamePattern,*pColumnIter,'\0'))
                {
                    aRow[4]  = new ORowSetValueDecorator( *pColumnIter);

                    xColumns->getByName(*pColumnIter) >>= xColumn;
                    OSL_ENSURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
                    aRow[6] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))));
                    aRow[7] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
                    //  aRow[8] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[9] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                    aRow[11] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))));
                    //  aRow[12] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[13] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))));
                    //  aRow[14] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    //  aRow[15] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    switch(sal_Int32(aRow[5]->getValue()))
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
                    switch(sal_Int32(aRow[11]->getValue()))
                    {
                    case ColumnValue::NO_NULLS:
                        aRow[18]  = new ORowSetValueDecorator(OUString("NO"));
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18]  = new ORowSetValueDecorator(OUString("YES"));
                        break;
                    default:
                        aRow[18]  = new ORowSetValueDecorator(OUString());
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    pResult->setRows(aRows);

    return pResult;
}

sal_Int32 SAL_CALL OComponentDatabaseMetaData::getMaxBinaryLiteralLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL OComponentDatabaseMetaData::getMaxCharLiteralLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL OComponentDatabaseMetaData::getMaxColumnNameLength(  )
{
    return SAL_MAX_INT32;
}

sal_Int32 SAL_CALL OComponentDatabaseMetaData::getMaxColumnsInIndex(  )
{
    return 1;
}

sal_Int32 SAL_CALL OComponentDatabaseMetaData::getMaxColumnsInTable(  )
{
    return 256;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
