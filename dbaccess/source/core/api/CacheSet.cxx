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
#include "precompiled_dbaccess.hxx"
#include "CacheSet.hxx"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include "dbastrings.hrc"
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>

#include <limits>

#include <connectivity/dbtools.hxx>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <comphelper/extract.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

using namespace comphelper;

using namespace dbaccess;
using namespace dbtools;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::osl;

DBG_NAME(OCacheSet)

OCacheSet::OCacheSet(sal_Int32 i_nMaxRows)
            :m_nMaxRows(i_nMaxRows)
            ,m_bInserted(sal_False)
            ,m_bUpdated(sal_False)
            ,m_bDeleted(sal_False)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::OCacheSet" );
    DBG_CTOR(OCacheSet,NULL);

}

::rtl::OUString OCacheSet::getIdentifierQuoteString() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getIdentifierQuoteString" );
    ::rtl::OUString sQuote;
    Reference<XDatabaseMetaData> xMeta;
    if ( m_xConnection.is() && (xMeta = m_xConnection->getMetaData()).is() )
        sQuote = xMeta->getIdentifierQuoteString();
    return sQuote;
}

void OCacheSet::construct(  const Reference< XResultSet>& _xDriverSet,const ::rtl::OUString& /*i_sRowSetFilter*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::construct" );
    OSL_ENSURE(_xDriverSet.is(),"Invalid resultSet");
    if(_xDriverSet.is())
    {
        m_xDriverSet = _xDriverSet;
        m_xDriverRow.set(_xDriverSet,UNO_QUERY);
        m_xSetMetaData = Reference<XResultSetMetaDataSupplier>(_xDriverSet,UNO_QUERY)->getMetaData();
        if ( m_xSetMetaData.is() )
        {
            const sal_Int32 nCount = m_xSetMetaData->getColumnCount();
            m_aNullable.realloc(nCount);
            m_aSignedFlags.realloc(nCount);
            m_aColumnTypes.realloc(nCount);
            sal_Bool* pNullableIter = m_aNullable.getArray();
            sal_Bool* pSignedIter = m_aSignedFlags.getArray();
            sal_Int32* pColumnIter = m_aColumnTypes.getArray();
            for (sal_Int32 i=1; i <= nCount; ++i,++pSignedIter,++pColumnIter,++pNullableIter)
            {
                *pNullableIter = m_xSetMetaData->isNullable(i) != ColumnValue::NO_NULLS;
                *pSignedIter = m_xSetMetaData->isSigned(i);
                *pColumnIter = m_xSetMetaData->getColumnType(i);
            }
        }
        Reference< XStatement> xStmt(m_xDriverSet->getStatement(),UNO_QUERY);
        if(xStmt.is())
            m_xConnection = xStmt->getConnection();
        else
        {
            Reference< XPreparedStatement> xPrepStmt(m_xDriverSet->getStatement(),UNO_QUERY);
            if ( xPrepStmt.is() )
                m_xConnection = xPrepStmt->getConnection();
        }
    }
}

OCacheSet::~OCacheSet()
{
    try
    {
        m_xDriverSet = NULL;
        m_xDriverRow = NULL;
        m_xSetMetaData = NULL;
        m_xConnection = NULL;
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception occurred");
    }
    catch(...)
    {
        OSL_ENSURE(0,"Unknown Exception occurred");
    }

    DBG_DTOR(OCacheSet,NULL);
}

void OCacheSet::fillTableName(const Reference<XPropertySet>& _xTable)  throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::fillTableName" );
    OSL_ENSURE(_xTable.is(),"OCacheSet::fillTableName: PropertySet is empty!");
    if(!m_aComposedTableName.getLength() && _xTable.is() )
    {
        Reference<XDatabaseMetaData> xMeta(m_xConnection->getMetaData());
        m_aComposedTableName = composeTableName(xMeta
                        ,comphelper::getString(_xTable->getPropertyValue(PROPERTY_CATALOGNAME))
                        ,comphelper::getString(_xTable->getPropertyValue(PROPERTY_SCHEMANAME))
                        ,comphelper::getString(_xTable->getPropertyValue(PROPERTY_NAME))
                        ,sal_True
                        ,::dbtools::eInDataManipulation);
    }
}

void SAL_CALL OCacheSet::insertRow( const ORowSetRow& _rInsertRow,const connectivity::OSQLTable& _xTable ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::insertRow" );
    ::rtl::OUStringBuffer aSql(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INSERT INTO ")));
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ( ")));
    // set values and column names
    ::rtl::OUStringBuffer aValues = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" VALUES ( "));
    static ::rtl::OUString aPara(RTL_CONSTASCII_USTRINGPARAM("?,"));
    ::rtl::OUString aQuote = getIdentifierQuoteString();
    static ::rtl::OUString aComma(RTL_CONSTASCII_USTRINGPARAM(","));
    sal_Int32 i = 1;
    ORowVector< ORowSetValue >::Vector::const_iterator aIter = _rInsertRow->get().begin()+1;
    connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
    for(; aIter != aEnd;++aIter)
    {
        aSql.append(::dbtools::quoteName( aQuote,m_xSetMetaData->getColumnName(i++)));
        aSql.append(aComma);
        aValues.append(aPara);
    }

    aSql.setCharAt(aSql.getLength()-1,')');
    aValues.setCharAt(aValues.getLength()-1,')');

    aSql.append(aValues.makeStringAndClear());
    // now create end execute the prepared statement
    {
        Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
        Reference< XParameters > xParameter(xPrep,UNO_QUERY);
        i = 1;
        for(aIter = _rInsertRow->get().begin()+1; aIter != aEnd;++aIter,++i)
        {
            if(aIter->isNull())
                xParameter->setNull(i,aIter->getTypeKind());
            else
                setParameter(i,xParameter,*aIter,m_xSetMetaData->getColumnType(i),m_xSetMetaData->getScale(i));
        }

        m_bInserted = xPrep->executeUpdate() > 0;
    }

    // TODO set the bookmark in the insert row
}

void OCacheSet::fillParameters( const ORowSetRow& _rRow
                                        ,const connectivity::OSQLTable& _xTable
                                        ,::rtl::OUStringBuffer& _sCondition
                                        ,::rtl::OUStringBuffer& _sParameter
                                        ,::std::list< sal_Int32>& _rOrgValues)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::fillParameters" );
    // use keys and indexes for exact positioning
    // first the keys
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(xSet);
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if(xIndexSup.is())
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    Reference<XPropertySet> xIndexColsSup;
    Reference<XNameAccess> xIndexColumns;
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    if(xIndexes.is())
    {
        for(sal_Int32 j=0;j<xIndexes->getCount();++j)
        {
            xIndexColsSup.set(xIndexes->getByIndex(j),UNO_QUERY);
            if( xIndexColsSup.is()
                && comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                && !comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
              )
                aAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
        }
    }

    ::rtl::OUString aColumnName;

    static ::rtl::OUString aPara(RTL_CONSTASCII_USTRINGPARAM("?,"));
    static ::rtl::OUString aAnd(RTL_CONSTASCII_USTRINGPARAM(" AND "));

    ::rtl::OUString aQuote  = getIdentifierQuoteString();

    sal_Int32 nCheckCount = 1; // index for the orginal values
    sal_Int32 i = 1;

    ::rtl::OUString sIsNull(RTL_CONSTASCII_USTRINGPARAM(" IS NULL"));
    ::rtl::OUString sParam(RTL_CONSTASCII_USTRINGPARAM(" = ?"));
    ORowVector< ORowSetValue >::Vector::const_iterator aIter = _rRow->get().begin()+1;
    ORowVector< ORowSetValue >::Vector::const_iterator aEnd = _rRow->get().end()+1;
    for(; aIter != aEnd;++aIter,++nCheckCount,++i)
    {
        aColumnName = m_xSetMetaData->getColumnName(i);
        if(xPrimaryKeyColumns.is() && xPrimaryKeyColumns->hasByName(aColumnName))
        {
            _sCondition.append(::dbtools::quoteName( aQuote,aColumnName));
            if(aIter->isNull())
                _sCondition.append(sIsNull);
            else
                _sCondition.append(sParam);
            _sCondition.append(aAnd);
            _rOrgValues.push_back(nCheckCount);

        }
        ::std::vector< Reference<XNameAccess> >::const_iterator aIndexEnd = aAllIndexColumns.end();
        for( ::std::vector< Reference<XNameAccess> >::const_iterator aIndexIter = aAllIndexColumns.begin();
                aIndexIter != aIndexEnd;++aIndexIter)
        {
            if((*aIndexIter)->hasByName(aColumnName))
            {
                _sCondition.append(::dbtools::quoteName( aQuote,aColumnName));
                if(aIter->isNull())
                    _sCondition.append(sIsNull);
                else
                    _sCondition.append(sParam);
                _sCondition.append(aAnd);
                _rOrgValues.push_back(nCheckCount);
                break;
            }
        }
        if(aIter->isModified())
        {
            _sParameter.append(::dbtools::quoteName( aQuote,aColumnName));
            _sParameter.append(aPara);
        }
    }
}

void SAL_CALL OCacheSet::updateRow(const ORowSetRow& _rInsertRow ,const ORowSetRow& _rOrginalRow,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::updateRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UPDATE "));
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" SET ")));
    // list all cloumns that should be set

    ::rtl::OUStringBuffer aCondition;
    ::std::list< sal_Int32> aOrgValues;
    fillParameters(_rInsertRow,_xTable,aCondition,aSql,aOrgValues);
    aSql.setCharAt(aSql.getLength()-1,' ');
    if ( aCondition.getLength() )
    {
        aCondition.setLength(aCondition.getLength()-5);

        aSql.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE ")));
        aSql.append(aCondition.makeStringAndClear());
    }
    else
        ::dbtools::throwSQLException(
            DBACORE_RESSTRING( RID_STR_NO_UPDATE_MISSING_CONDITION ), SQL_GENERAL_ERROR, *this );

    // now create end execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
    sal_Int32 i = 1;
    connectivity::ORowVector< ORowSetValue > ::Vector::iterator aEnd = _rInsertRow->get().end();
    for(ORowVector< ORowSetValue >::Vector::const_iterator aIter = _rInsertRow->get().begin()+1; aIter != aEnd;++aIter)
    {
        if(aIter->isModified())
        {
            setParameter(i,xParameter,*aIter,m_xSetMetaData->getColumnType(i),m_xSetMetaData->getScale(i));
            ++i;
        }
    }
    ::std::list< sal_Int32>::const_iterator aOrgValueEnd = aOrgValues.end();
    for(::std::list< sal_Int32>::const_iterator aOrgValue = aOrgValues.begin(); aOrgValue != aOrgValueEnd;++aOrgValue,++i)
    {
        setParameter(i,xParameter,(_rOrginalRow->get())[*aOrgValue],m_xSetMetaData->getColumnType(i),m_xSetMetaData->getScale(i));
    }

     m_bUpdated = xPrep->executeUpdate() > 0;
}

void SAL_CALL OCacheSet::deleteRow(const ORowSetRow& _rDeleteRow ,const connectivity::OSQLTable& _xTable  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::deleteRow" );
    Reference<XPropertySet> xSet(_xTable,UNO_QUERY);
    fillTableName(xSet);

    ::rtl::OUStringBuffer aSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DELETE FROM "));
    aSql.append(m_aComposedTableName);
    aSql.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE ")));

    // list all cloumns that should be set
    ::rtl::OUString aQuote  = getIdentifierQuoteString();
    static ::rtl::OUString aAnd(RTL_CONSTASCII_USTRINGPARAM(" AND "));

    // use keys and indexes for exact positioning
    // first the keys
    const Reference<XNameAccess> xPrimaryKeyColumns = getPrimaryKeyColumns_throw(xSet);
    // second the indexes
    Reference<XIndexesSupplier> xIndexSup(_xTable,UNO_QUERY);
    Reference<XIndexAccess> xIndexes;
    if(xIndexSup.is())
        xIndexes.set(xIndexSup->getIndexes(),UNO_QUERY);

    //  Reference<XColumnsSupplier>
    Reference<XPropertySet> xIndexColsSup;
    Reference<XNameAccess> xIndexColumns;
    ::std::vector< Reference<XNameAccess> > aAllIndexColumns;
    if(xIndexes.is())
    {
        for(sal_Int32 j=0;j<xIndexes->getCount();++j)
        {
            xIndexColsSup.set(xIndexes->getByIndex(j),UNO_QUERY);
            if( xIndexColsSup.is()
                && comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISUNIQUE))
                && !comphelper::getBOOL(xIndexColsSup->getPropertyValue(PROPERTY_ISPRIMARYKEYINDEX))
              )
                aAllIndexColumns.push_back(Reference<XColumnsSupplier>(xIndexColsSup,UNO_QUERY)->getColumns());
        }
    }

    ::rtl::OUStringBuffer aColumnName;
    ::std::list< sal_Int32> aOrgValues;
    fillParameters(_rDeleteRow,_xTable,aSql,aColumnName,aOrgValues);

    aSql.setLength(aSql.getLength()-5);

    // now create and execute the prepared statement
    Reference< XPreparedStatement > xPrep(m_xConnection->prepareStatement(aSql.makeStringAndClear()));
    Reference< XParameters > xParameter(xPrep,UNO_QUERY);
    sal_Int32 i = 1;
    ::std::list< sal_Int32>::const_iterator aOrgValueEnd = aOrgValues.end();
    for(::std::list< sal_Int32>::const_iterator j = aOrgValues.begin(); j != aOrgValueEnd;++j,++i)
    {
        setParameter(i,xParameter,(_rDeleteRow->get())[*j],m_xSetMetaData->getColumnType(i),m_xSetMetaData->getScale(i));
    }

    m_bDeleted = xPrep->executeUpdate() > 0;
}

void OCacheSet::setParameter(sal_Int32 nPos
                             ,const Reference< XParameters >& _xParameter
                             ,const ORowSetValue& _rValue
                             ,sal_Int32 _nType
                             ,sal_Int32 _nScale) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::setParameter" );
    sal_Int32 nType = ( _nType != DataType::OTHER ) ? _nType : _rValue.getTypeKind();
    ::dbtools::setObjectWithInfo(_xParameter,nPos,_rValue,nType,_nScale);
}

void OCacheSet::fillValueRow(ORowSetRow& _rRow,sal_Int32 _nPosition)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::fillValueRow" );
    Any aBookmark = getBookmark();
    if(!aBookmark.hasValue())
        aBookmark = makeAny(_nPosition);

    connectivity::ORowVector< ORowSetValue >::Vector::iterator aIter = _rRow->get().begin();
    connectivity::ORowVector< ORowSetValue >::Vector::iterator aEnd = _rRow->get().end();
    (*aIter) = aBookmark;
    ++aIter;
    for(sal_Int32 i=1;aIter != aEnd;++aIter,++i)
    {
        aIter->setSigned(m_aSignedFlags[i-1]);
        aIter->fill(i,m_aColumnTypes[i-1],m_aNullable[i-1],this);
    }
}

sal_Bool SAL_CALL OCacheSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::wasNull" );
    return m_xDriverRow->wasNull();
}

::rtl::OUString SAL_CALL OCacheSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getString" );
    return m_xDriverRow->getString(columnIndex);
}

sal_Bool SAL_CALL OCacheSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getBoolean" );
    return m_xDriverRow->getBoolean(columnIndex);
}

sal_Int8 SAL_CALL OCacheSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getByte" );
    return m_xDriverRow->getByte(columnIndex);
}

sal_Int16 SAL_CALL OCacheSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getShort" );
    return m_xDriverRow->getShort(columnIndex);
}

sal_Int32 SAL_CALL OCacheSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getInt" );
    return m_xDriverRow->getInt(columnIndex);
}

sal_Int64 SAL_CALL OCacheSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getLong" );
    return m_xDriverRow->getLong(columnIndex);
}

float SAL_CALL OCacheSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getFloat" );
    return m_xDriverRow->getFloat(columnIndex);
}

double SAL_CALL OCacheSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getDouble" );
    return m_xDriverRow->getDouble(columnIndex);
}

Sequence< sal_Int8 > SAL_CALL OCacheSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getBytes" );
    return m_xDriverRow->getBytes(columnIndex);
}

::com::sun::star::util::Date SAL_CALL OCacheSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getDate" );
    return m_xDriverRow->getDate(columnIndex);
}

::com::sun::star::util::Time SAL_CALL OCacheSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getTime" );
    return m_xDriverRow->getTime(columnIndex);
}

::com::sun::star::util::DateTime SAL_CALL OCacheSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getTimestamp" );
    return m_xDriverRow->getTimestamp(columnIndex);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCacheSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getBinaryStream" );
    return m_xDriverRow->getBinaryStream(columnIndex);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCacheSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getCharacterStream" );
    return m_xDriverRow->getCharacterStream(columnIndex);
}

Any SAL_CALL OCacheSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getObject" );
    return m_xDriverRow->getObject(columnIndex,typeMap);
}

Reference< XRef > SAL_CALL OCacheSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getRef" );
    return m_xDriverRow->getRef(columnIndex);
}

Reference< XBlob > SAL_CALL OCacheSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getBlob" );
    return m_xDriverRow->getBlob(columnIndex);
}

Reference< XClob > SAL_CALL OCacheSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getClob" );
    return m_xDriverRow->getClob(columnIndex);
}

Reference< XArray > SAL_CALL OCacheSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getArray" );
    return m_xDriverRow->getArray(columnIndex);
}

// XResultSet
sal_Bool SAL_CALL OCacheSet::next(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::next" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->next();
}

sal_Bool SAL_CALL OCacheSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::isBeforeFirst" );
    return m_xDriverSet->isBeforeFirst();
}

sal_Bool SAL_CALL OCacheSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::isAfterLast" );
    return m_xDriverSet->isAfterLast();
}

sal_Bool SAL_CALL OCacheSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::isFirst" );
    return m_xDriverSet->isFirst();
}

sal_Bool SAL_CALL OCacheSet::isLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::isLast" );
    return m_xDriverSet->isLast();
}

void SAL_CALL OCacheSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::beforeFirst" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_xDriverSet->beforeFirst();
}

void SAL_CALL OCacheSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::afterLast" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    m_xDriverSet->afterLast();
}

sal_Bool SAL_CALL OCacheSet::first(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::first" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->first();
}

sal_Bool SAL_CALL OCacheSet::last(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::last" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->last();
}

sal_Int32 SAL_CALL OCacheSet::getRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getRow" );
    return m_xDriverSet->getRow();
}

sal_Bool SAL_CALL OCacheSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::absolute" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->absolute(row);
}

sal_Bool SAL_CALL OCacheSet::relative( sal_Int32 rows ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::relative" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->relative(rows);
}

sal_Bool SAL_CALL OCacheSet::previous(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::previous" );
    m_bInserted = m_bUpdated = m_bDeleted = sal_False;
    return m_xDriverSet->previous();
}

void SAL_CALL OCacheSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::refreshRow" );
    m_xDriverSet->refreshRow();
}

sal_Bool SAL_CALL OCacheSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::rowUpdated" );
    return m_xDriverSet->rowUpdated();
}

sal_Bool SAL_CALL OCacheSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::rowInserted" );
    return m_xDriverSet->rowInserted();
}

sal_Bool SAL_CALL OCacheSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::rowDeleted" );
    return m_xDriverSet->rowDeleted();
}

Reference< XInterface > SAL_CALL OCacheSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OCacheSet::getStatement" );
    return m_xDriverSet->getStatement();
}

bool OCacheSet::isResultSetChanged() const
{
    return false;
}

void OCacheSet::reset(const Reference< XResultSet>& /*_xDriverSet*/)
{
    OSL_ENSURE(0,"Illegal call!");
}

void OCacheSet::mergeColumnValues(sal_Int32 i_nColumnIndex,ORowSetValueVector::Vector& /*io_aInsertRow*/,ORowSetValueVector::Vector& /*io_aRow*/,::std::vector<sal_Int32>& o_aChangedColumns)
{
    o_aChangedColumns.push_back(i_nColumnIndex);
}

bool OCacheSet::columnValuesUpdated(ORowSetValueVector::Vector& /*io_aCachedRow*/,const ORowSetValueVector::Vector& /*io_aRow*/)
{
    return false;
}

bool OCacheSet::updateColumnValues(const ORowSetValueVector::Vector& /*io_aCachedRow*/,ORowSetValueVector::Vector& /*io_aRow*/,const ::std::vector<sal_Int32>& /*i_aChangedColumns*/)
{
    return true;
}

void OCacheSet::fillMissingValues(ORowSetValueVector::Vector& /*io_aRow*/) const
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
