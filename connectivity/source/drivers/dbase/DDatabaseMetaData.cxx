/*************************************************************************
 *
 *  $RCSfile: DDatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:17:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _CONNECTIVITY_DBASE_ODbaseDatabaseMetaData_HXX_
#include "dbase/DDatabaseMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPropertySet_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_ADATABASEMETADATARESULTSET_HXX_
#include "file/FDatabaseMetaDataResultSet.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbase
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;



ODbaseDatabaseMetaData::ODbaseDatabaseMetaData(OConnection* _pCon)  :ODatabaseMetaData(_pCon)
{
}
// -------------------------------------------------------------------------
ODbaseDatabaseMetaData::~ODbaseDatabaseMetaData()
{
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setTypeInfoMap();
    ORows aRows;
    ORow aRow;

    aRow.push_back(Any());
    aRow.push_back(makeAny(::rtl::OUString::createFromAscii("CHAR")));
    aRow.push_back(makeAny(DataType::CHAR));
    aRow.push_back(makeAny((sal_Int32)254));
    aRow.push_back(makeAny(::rtl::OUString::createFromAscii("'")));
    aRow.push_back(makeAny(::rtl::OUString::createFromAscii("'")));
    aRow.push_back(Any());
    aRow.push_back(makeAny((sal_Int32)ColumnValue::NULLABLE));
    aRow.push_back(makeAny((sal_Int32)1));
    aRow.push_back(makeAny((sal_Int32)ColumnSearch::CHAR));
    aRow.push_back(makeAny((sal_Int32)1));
    aRow.push_back(makeAny((sal_Int32)0));
    aRow.push_back(makeAny((sal_Int32)0));
    aRow.push_back(Any());
    aRow.push_back(makeAny((sal_Int32)0));
    aRow.push_back(makeAny((sal_Int32)0));
    aRow.push_back(Any());
    aRow.push_back(Any());
    aRow.push_back(makeAny((sal_Int32)10));
    aRows.push_back(aRow);

    aRow[1] = makeAny(::rtl::OUString::createFromAscii("VARCHAR"));
    aRow[2] = makeAny(DataType::VARCHAR);
    aRow[4] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRow[5] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRows.push_back(aRow);


    aRow[1] = makeAny(::rtl::OUString::createFromAscii("LONGVARCHAR"));
    aRow[2] = makeAny(DataType::LONGVARCHAR);
    aRow[3] = makeAny((sal_Int32)65535);
    aRow[4] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRow[5] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRows.push_back(aRow);

    aRow[1] = makeAny(::rtl::OUString::createFromAscii("BOOL"));
    aRow[2] = makeAny(DataType::BIT);
    aRow[3] = makeAny((sal_Int32)1);
    aRow[9] = makeAny((sal_Int32)ColumnSearch::BASIC);
    aRows.push_back(aRow);

    aRow[1] = makeAny(::rtl::OUString::createFromAscii("DATE"));
    aRow[2] = makeAny(DataType::DATE);
    aRow[3] = makeAny((sal_Int32)10);
    aRow[4] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRow[5] = makeAny(::rtl::OUString::createFromAscii("'"));
    aRows.push_back(aRow);

    aRow[1] = makeAny(::rtl::OUString::createFromAscii("DECIMAL"));
    aRow[2] = makeAny(DataType::DECIMAL);
    aRow[3] = makeAny((sal_Int32)20);
    aRow[15] = makeAny((sal_Int32)15);
    aRows.push_back(aRow);

    aRow[1] = makeAny(::rtl::OUString::createFromAscii("NUMERIC"));
    aRow[2] = makeAny(DataType::NUMERIC);
    aRow[3] = makeAny((sal_Int32)20);
    aRow[15] = makeAny((sal_Int32)20);
    aRows.push_back(aRow);

    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setColumnPrivilegesMap();
    return xRef;
}

// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );


    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ORows aRows;
    ORow aRow(19);
    aRow[10] <<= (sal_Int32)10;
    Sequence< ::rtl::OUString> aTabNames(xNames->getElementNames());
    const ::rtl::OUString* pTabBegin    = aTabNames.getConstArray();
    const ::rtl::OUString* pTabEnd      = pTabBegin + aTabNames.getLength();
    for(;pTabBegin != pTabEnd;++pTabBegin)
    {
        if(match(tableNamePattern,*pTabBegin,'\0'))
        {
            Reference< XColumnsSupplier> xTable;
            xNames->getByName(*pTabBegin) >>= xTable;
            aRow[3] <<= *pTabBegin;

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
                    aRow[4] <<= *pBegin;

                    xColumns->getByName(*pBegin) >>= xColumn;
                    OSL_ENSHURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = xColumn->getPropertyValue(PROPERTY_TYPE);
                    aRow[6] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    aRow[7] = xColumn->getPropertyValue(PROPERTY_PRECISION);
                    //  aRow[8] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    aRow[9] = xColumn->getPropertyValue(PROPERTY_SCALE);
                    aRow[11] = xColumn->getPropertyValue(PROPERTY_ISNULLABLE);
                    //  aRow[12] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    aRow[13] = xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE);
                    //  aRow[14] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    //  aRow[15] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    switch(getINT32(aRow[5]))
                    {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aRow[16] <<= (sal_Int32)254;
                        break;
                    case DataType::LONGVARCHAR:
                        aRow[16] <<= (sal_Int32)65535;
                        break;
                    default:
                        aRow[16] <<= (sal_Int32)0;
                    }
                    aRow[17] <<= i;
                    switch(getINT32(aRow[11]))
                    {
                    case ColumnValue::NO_NULLS:
                        aRow[18] <<= ::rtl::OUString::createFromAscii("NO");
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18] <<= ::rtl::OUString::createFromAscii("YES");
                        break;
                    default:
                        aRow[18] <<= ::rtl::OUString();
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setColumnsMap();
    pResult->setRows(aRows);

    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getVersionColumns(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setVersionColumnsMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getExportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setExportedKeysMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getImportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setImportedKeysMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getPrimaryKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setPrimaryKeysMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
        sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
                throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
                throw SQLException();

    ORows aRows;
    ORow aRow(14);
    aRow[5] <<= ::rtl::OUString();
    aRow[10] <<= ::rtl::OUString::createFromAscii("A");

    Reference< XIndexesSupplier> xTable;
    xNames->getByName(table) >>= xTable;
    aRow[3] <<= table;
    aRow[7] <<= (sal_Int32)3;

    Reference< XNameAccess> xIndexes = xTable->getIndexes();
    if(!xIndexes.is())
                throw SQLException();

    Sequence< ::rtl::OUString> aIdxNames(xIndexes->getElementNames());

    const ::rtl::OUString* pBegin = aIdxNames.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aIdxNames.getLength();
    Reference< XPropertySet> xIndex;
    for(;pBegin != pEnd;++pBegin)
    {
        xIndexes->getByName(*pBegin) >>= xIndex;
        OSL_ENSHURE(xIndex.is(),"Indexes contains a column who isn't a fastpropertyset!");

        if(unique && !getBOOL(xIndex->getPropertyValue(PROPERTY_ISUNIQUE)))
            continue;
        aRow[4] = xIndex->getPropertyValue(PROPERTY_ISUNIQUE);
        aRow[6] <<= *pBegin;

        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
        if(xTunnel.is())
        {
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
            if(pIndex)
            {
                aRow[11] <<= pIndex->getHeader().db_maxkeys;
                aRow[12] <<= pIndex->getHeader().db_pagecount;
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
            xColumns->getByName(*pColBegin) >>= xColumn;
            aRow[8] <<= j;
            aRow[9] <<= *pColBegin;
            aRows.push_back(aRow);
        }
    }

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setIndexInfoMap();
    pResult->setRows(aRows);
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
        sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setBestRowIdentifierMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getTablePrivileges(
        const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setTablePrivilegesMap();
    return xRef;
}
// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL ODbaseDatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
        const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setCrossReferenceMap();
    return xRef;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL ODbaseDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return ::rtl::OUString::createFromAscii("sdbc:dbase:");
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 254;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 10;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL ODbaseDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 128;
}


