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

#include <flat/EDatabaseMetaData.hxx>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <FDatabaseMetaDataResultSet.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::flat;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;


OFlatDatabaseMetaData::OFlatDatabaseMetaData(::connectivity::file::OConnection* _pCon)  :ODatabaseMetaData(_pCon)
{
}

OFlatDatabaseMetaData::~OFlatDatabaseMetaData()
{
}

Reference< XResultSet > OFlatDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);

    static ODatabaseMetaDataResultSet::ORows aRows = []()
    {
        ODatabaseMetaDataResultSet::ORows tmp;
        ODatabaseMetaDataResultSet::ORow aRow
        {
             ODatabaseMetaDataResultSet::getEmptyValue() ,
             new ORowSetValueDecorator(u"CHAR"_ustr) ,
             new ORowSetValueDecorator(DataType::CHAR) ,
             new ORowSetValueDecorator(sal_Int32(254)) ,
             ODatabaseMetaDataResultSet::getQuoteValue() ,
             ODatabaseMetaDataResultSet::getQuoteValue() ,
             ODatabaseMetaDataResultSet::getEmptyValue() ,
             new ORowSetValueDecorator(sal_Int32(ColumnValue::NULLABLE)) ,
             ODatabaseMetaDataResultSet::get1Value() ,
             new ORowSetValueDecorator(sal_Int32(ColumnSearch::CHAR)) ,
             ODatabaseMetaDataResultSet::get1Value() ,
             ODatabaseMetaDataResultSet::get0Value() ,
             ODatabaseMetaDataResultSet::get0Value() ,
             ODatabaseMetaDataResultSet::getEmptyValue() ,
             ODatabaseMetaDataResultSet::get0Value() ,
             ODatabaseMetaDataResultSet::get0Value() ,
             ODatabaseMetaDataResultSet::getEmptyValue() ,
             ODatabaseMetaDataResultSet::getEmptyValue() ,
             new ORowSetValueDecorator(sal_Int32(10))
        };

        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"VARCHAR"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::VARCHAR);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);


        aRow[1] = new ORowSetValueDecorator(u"LONGVARCHAR"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::LONGVARCHAR);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(65535));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"DATE"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(10));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"TIME"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::TIME);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(8));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"TIMESTAMP"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(19));
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"BOOL"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::BIT);
        aRow[3] = ODatabaseMetaDataResultSet::get1Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"DECIMAL"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[15] = new ORowSetValueDecorator(sal_Int32(15));
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"DOUBLE"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::DOUBLE);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        tmp.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(u"NUMERIC"_ustr);
        aRow[2] = new ORowSetValueDecorator(DataType::NUMERIC);
        aRow[3] = new ORowSetValueDecorator(sal_Int32(20));
        aRow[15] = new ORowSetValueDecorator(sal_Int32(20));
        tmp.push_back(aRow);

        return tmp;
    }();

    pResult->setRows(std::move(aRows));
    return pResult;
}

Reference< XResultSet > SAL_CALL OFlatDatabaseMetaData::getColumns(
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
    aRow[10] = new ORowSetValueDecorator(sal_Int32(10));
    Sequence< OUString> aTabNames(xNames->getElementNames());
    const OUString* pTabBegin    = aTabNames.getConstArray();
    const OUString* pTabEnd      = pTabBegin + aTabNames.getLength();
    for(;pTabBegin != pTabEnd;++pTabBegin)
    {
        if(match(tableNamePattern,*pTabBegin,'\0'))
        {
            Reference< XColumnsSupplier> xTable(
                xNames->getByName(*pTabBegin), css::uno::UNO_QUERY);
            aRow[3] = new ORowSetValueDecorator(*pTabBegin);

            Reference< XNameAccess> xColumns = xTable->getColumns();
            if(!xColumns.is())
                throw SQLException();

            Sequence< OUString> aColNames(xColumns->getElementNames());

            const OUString* pBegin = aColNames.getConstArray();
            const OUString* pEnd = pBegin + aColNames.getLength();
            Reference< XPropertySet> xColumn;
            for(sal_Int32 i=1;pBegin != pEnd;++pBegin,++i)
            {
                if(match(columnNamePattern,*pBegin,'\0'))
                {
                    aRow[4] = new ORowSetValueDecorator(*pBegin);

                    xColumn.set(
                        xColumns->getByName(*pBegin), css::uno::UNO_QUERY);
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
                        aRow[18]  = new ORowSetValueDecorator(u"NO"_ustr);
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18]  = new ORowSetValueDecorator(u"YES"_ustr);
                        break;
                    default:
                        aRow[18]  = new ORowSetValueDecorator(OUString());
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    rtl::Reference<::connectivity::ODatabaseMetaDataResultSet> pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    pResult->setRows(std::move(aRows));

    return pResult;
}

OUString SAL_CALL OFlatDatabaseMetaData::getURL(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return "sdbc:flat:" + m_pConnection->getURL();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
