/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"


#include "flat/EDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <tools/urlobj.hxx>
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <rtl/logfile.hxx>

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatDatabaseMetaData::OFlatDatabaseMetaData" );
}
// -------------------------------------------------------------------------
OFlatDatabaseMetaData::~OFlatDatabaseMetaData()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatDatabaseMetaData::~OFlatDatabaseMetaData" );
}
// -------------------------------------------------------------------------
Reference< XResultSet > OFlatDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatDatabaseMetaData::impl_getTypeInfo_throw" );
    ::osl::MutexGuard aGuard( m_aMutex );

    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;

        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CHAR"))));
        aRow.push_back(new ORowSetValueDecorator(DataType::CHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)254));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnSearch::CHAR));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));

        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR")));
        aRow[2] = new ORowSetValueDecorator(DataType::VARCHAR);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRows.push_back(aRow);


        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LONGVARCHAR")));
        aRow[2] = new ORowSetValueDecorator(DataType::LONGVARCHAR);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)65535);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE")));
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)10);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIME")));
        aRow[2] = new ORowSetValueDecorator(DataType::TIME);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)8);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIMESTAMP")));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)19);
        aRow[4] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRow[5] = ODatabaseMetaDataResultSet::getQuoteValue();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BOOL")));
        aRow[2] = new ORowSetValueDecorator(DataType::BIT);
        aRow[3] = ODatabaseMetaDataResultSet::get1Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DECIMAL")));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[15] = new ORowSetValueDecorator((sal_Int32)15);
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOUBLE")));
        aRow[2] = new ORowSetValueDecorator(DataType::DOUBLE);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUMERIC")));
        aRow[2] = new ORowSetValueDecorator(DataType::NUMERIC);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[15] = new ORowSetValueDecorator((sal_Int32)20);
        aRows.push_back(aRow);
    }

    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OFlatDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& tableNamePattern,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatDatabaseMetaData::getColumns" );
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(19);
    aRow[10] = new ORowSetValueDecorator((sal_Int32)10);
    Sequence< ::rtl::OUString> aTabNames(xNames->getElementNames());
    const ::rtl::OUString* pTabBegin    = aTabNames.getConstArray();
    const ::rtl::OUString* pTabEnd      = pTabBegin + aTabNames.getLength();
    for(;pTabBegin != pTabEnd;++pTabBegin)
    {
        if(match(tableNamePattern,*pTabBegin,'\0'))
        {
            Reference< XColumnsSupplier> xTable;
            ::cppu::extractInterface(xTable,xNames->getByName(*pTabBegin));
            aRow[3] = new ORowSetValueDecorator(*pTabBegin);

            Reference< XNameAccess> xColumns = xTable->getColumns();
            if(!xColumns.is())
                throw SQLException();

            Sequence< ::rtl::OUString> aColNames(xColumns->getElementNames());

            const ::rtl::OUString* pBegin = aColNames.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + aColNames.getLength();
            Reference< XPropertySet> xColumn;
            for(sal_Int32 i=1;pBegin != pEnd;++pBegin,++i)
            {
                if(match(columnNamePattern,*pBegin,'\0'))
                {
                    aRow[4] = new ORowSetValueDecorator(*pBegin);

                    ::cppu::extractInterface(xColumn,xColumns->getByName(*pBegin));
                    OSL_ENSURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
                    aRow[6] = new ORowSetValueDecorator(getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))));
                    aRow[7] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
                    aRow[9] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                    aRow[11] = new ORowSetValueDecorator(getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))));
                    aRow[13] = new ORowSetValueDecorator(getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))));

                    switch((sal_Int32)aRow[5]->getValue())
                    {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)254);
                        break;
                    case DataType::LONGVARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)65535);
                        break;
                    default:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)0);
                    }
                    aRow[17] = new ORowSetValueDecorator(i);
                    switch(sal_Int32(aRow[11]->getValue()))
                    {
                    case ColumnValue::NO_NULLS:
                        aRow[18]  = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NO")));
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18]  = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("YES")));
                        break;
                    default:
                        aRow[18]  = new ORowSetValueDecorator(::rtl::OUString());
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xRef = pResult;
    pResult->setRows(aRows);

    return xRef;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFlatDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatDatabaseMetaData::getURL" );
    ::osl::MutexGuard aGuard( m_aMutex );
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:flat:")) + m_pConnection->getURL();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
