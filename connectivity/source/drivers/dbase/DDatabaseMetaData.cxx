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


#include "dbase/DDatabaseMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <tools/urlobj.hxx>
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include "dbase/DIndex.hxx"
#include "connectivity/FValue.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <ucbhelper/content.hxx>
#include <rtl/logfile.hxx>

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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::ODbaseDatabaseMetaData" );
}
// -------------------------------------------------------------------------
ODbaseDatabaseMetaData::~ODbaseDatabaseMetaData()
{
}
// -------------------------------------------------------------------------
Reference< XResultSet > ODbaseDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::impl_getTypeInfo_throw" );
    ::osl::MutexGuard aGuard( m_aMutex );

    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;
        aRow.reserve(18);

        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR"))));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)254));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("length"))));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnValue::NULLABLE));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnSearch::FULL));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("C"))));
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));

        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LONGVARCHAR")));
        aRow[2] = new ORowSetValueDecorator(DataType::LONGVARCHAR);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)2147483647);
        aRow[6] = new ORowSetValueDecorator();
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("M")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE")));
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)10);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("D")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BOOLEAN")));
        aRow[2] = new ORowSetValueDecorator(DataType::BIT);
        aRow[3] = ODatabaseMetaDataResultSet::get1Value();
        aRow[4] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[5] = ODatabaseMetaDataResultSet::getEmptyValue();
        aRow[6] = new ORowSetValueDecorator(::rtl::OUString());
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("L")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOUBLE")));
        aRow[2] = new ORowSetValueDecorator(DataType::DOUBLE);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)8);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("B")));
        aRows.push_back(aRow);

        aRow[11] = new ORowSetValueDecorator(sal_True);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Y")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIMESTAMP")));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[11] = new ORowSetValueDecorator(sal_False);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("T")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INTEGER")));
        aRow[2] = new ORowSetValueDecorator(DataType::INTEGER);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)10);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("I")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DECIMAL")));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[6] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("length,scale")));
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("F")));
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUMERIC")));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)16);
        aRow[13] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("N")));
        aRow[15] = new ORowSetValueDecorator((sal_Int32)16);
        aRows.push_back(aRow);
    }

    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const ::rtl::OUString& /*schemaPattern*/, const ::rtl::OUString& tableNamePattern,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getColumns" );
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
            OSL_ENSURE(xTable.is(),"Table not found! Normallya exception had to be thrown here!");
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
                        aRow[18] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NO")));
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18] = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("YES")));
                        break;
                    default:
                        aRow[18] = new ORowSetValueDecorator(::rtl::OUString());
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
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getIndexInfo(
    const Any& /*catalog*/, const ::rtl::OUString& /*schema*/, const ::rtl::OUString& table,
        sal_Bool unique, sal_Bool /*approximate*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getIndexInfo" );
    ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
                throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
                throw SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow aRow(14);

    aRow[5]     = new ORowSetValueDecorator(::rtl::OUString());
    aRow[10]    = new ORowSetValueDecorator(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("A")));

    Reference< XIndexesSupplier> xTable;
    ::cppu::extractInterface(xTable,xNames->getByName(table));
    aRow[3] = new ORowSetValueDecorator(table);
    aRow[7] = new ORowSetValueDecorator((sal_Int32)3);

    Reference< XNameAccess> xIndexes = xTable->getIndexes();
    if(!xIndexes.is())
        throw SQLException();

    Sequence< ::rtl::OUString> aIdxNames(xIndexes->getElementNames());

    const ::rtl::OUString* pBegin = aIdxNames.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aIdxNames.getLength();
    Reference< XPropertySet> xIndex;
    for(;pBegin != pEnd;++pBegin)
    {
        ::cppu::extractInterface(xIndex,xIndexes->getByName(*pBegin));
        OSL_ENSURE(xIndex.is(),"Indexes contains a column who isn't a fastpropertyset!");

        if(unique && !getBOOL(xIndex->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            continue;
        aRow[4] = new ORowSetValueDecorator(getBOOL(xIndex->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))));
        aRow[6] = new ORowSetValueDecorator(*pBegin);

        Reference< XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
        if(xTunnel.is())
        {
            ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
            if(pIndex)
            {
                aRow[11] = new ORowSetValueDecorator((sal_Int32)pIndex->getHeader().db_maxkeys);
                aRow[12] = new ORowSetValueDecorator((sal_Int32)pIndex->getHeader().db_pagecount);
            }
        }

        Reference<XColumnsSupplier> xColumnsSup(xIndex,UNO_QUERY);
        Reference< XNameAccess> xColumns = xColumnsSup->getColumns();
        Sequence< ::rtl::OUString> aColNames(xColumns->getElementNames());

        const ::rtl::OUString* pColBegin = aColNames.getConstArray();
        const ::rtl::OUString* pColEnd = pColBegin + aColNames.getLength();
        Reference< XPropertySet> xColumn;
        for(sal_Int32 j=1;pColBegin != pColEnd;++pColBegin,++j)
        {
            aRow[8] = new ORowSetValueDecorator(j);
            aRow[9] = new ORowSetValueDecorator(*pColBegin);
            aRows.push_back(aRow);
        }
    }

    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eIndexInfo);
    Reference< XResultSet > xRef = pResult;
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODbaseDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getURL" );
    ::osl::MutexGuard aGuard( m_aMutex );
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:dbase:")) + m_pConnection->getURL();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getMaxBinaryLiteralLength" );
    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getMaxCharLiteralLength" );
    return 254;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getMaxColumnNameLength" );
    return 10;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getMaxColumnsInIndex" );
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::getMaxColumnsInTable" );
    return 128;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseDatabaseMetaData::supportsAlterTableWithAddColumn(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::supportsAlterTableWithAddColumn" );
    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseDatabaseMetaData::supportsAlterTableWithDropColumn(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::supportsAlterTableWithDropColumn" );
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODbaseDatabaseMetaData::isReadOnly(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::isReadOnly" );
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Bool bReadOnly = sal_False;
    static ::rtl::OUString sReadOnly( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ));
    ::ucbhelper::Content aFile(m_pConnection->getContent(),Reference< XCommandEnvironment >());
    aFile.getPropertyValue(sReadOnly) >>= bReadOnly;

    return bReadOnly;
}
// -----------------------------------------------------------------------------
sal_Bool ODbaseDatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw(  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::impl_storesMixedCaseQuotedIdentifiers_throw" );
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool ODbaseDatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw(  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbase", "Ocke.Janssen@sun.com", "ODbaseDatabaseMetaData::impl_supportsMixedCaseQuotedIdentifiers_throw" );
    return sal_True;
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
