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


#include "ParameterSubstitution.hxx"
#include "FDatabaseMetaDataResultSet.hxx"
#include "FDatabaseMetaDataResultSetMetaData.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <comphelper/property.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/common_res.hrc"
#include "TConnection.hxx"

using namespace connectivity;
using namespace dbtools;
using namespace cppu;

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet()
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(ODatabaseMetaDataResultSet_BASE::rBHelper)
    ,m_aStatement(nullptr)
    ,m_xMetaData(nullptr)
    ,m_nColPos(0)
    ,m_bBOF(true)
    ,m_bEOF(true)
{
    construct();
}


ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet( MetaDataResultSetType _eType )
    :ODatabaseMetaDataResultSet_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(ODatabaseMetaDataResultSet_BASE::rBHelper)
    ,m_aStatement(nullptr)
    ,m_xMetaData(nullptr)
    ,m_nColPos(0)
    ,m_bBOF(true)
    ,m_bEOF(true)
{
    construct();

    setType(_eType);
}


ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
}

void ODatabaseMetaDataResultSet::construct()
{
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),           PROPERTY_ID_FETCHSIZE,          0,&m_nFetchSize,        ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),        PROPERTY_ID_RESULTSETTYPE,          PropertyAttribute::READONLY,&m_nResultSetType,       ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),      PROPERTY_ID_FETCHDIRECTION,     0,  &m_nFetchDirection, ::cppu::UnoType<sal_Int32>::get());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY), PROPERTY_ID_RESULTSETCONCURRENCY,   PropertyAttribute::READONLY,&m_nResultSetConcurrency,                ::cppu::UnoType<sal_Int32>::get());
}

void ODatabaseMetaDataResultSet::setType(MetaDataResultSetType _eType)
{
    switch( _eType )
    {
        case eCatalogs:             setCatalogsMap(); break;
        case eSchemas:              setSchemasMap(); break;
        case eColumnPrivileges:     setColumnPrivilegesMap(); break;
        case eColumns:              setColumnsMap(); break;
        case eTables:               setTablesMap(); break;
        case eTableTypes:           setTableTypes(); break;
        case eProcedureColumns:     setProcedureColumnsMap(); break;
        case eProcedures:           setProceduresMap(); break;
        case eExportedKeys:         setExportedKeysMap(); break;
        case eImportedKeys:         setImportedKeysMap(); break;
        case ePrimaryKeys:          setPrimaryKeysMap(); break;
        case eIndexInfo:            setIndexInfoMap(); break;
        case eTablePrivileges:      setTablePrivilegesMap(); break;
        case eCrossReference:       setCrossReferenceMap(); break;
        case eTypeInfo:             setTypeInfoMap(); break;
        case eBestRowIdentifier:    setBestRowIdentifierMap(); break;
        case eVersionColumns:       setVersionColumnsMap(); break;
        case eUDTs:                 setUDTsMap(); break;
        default:
            OSL_FAIL("Wrong type!");
    }
}

void ODatabaseMetaDataResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_aStatement    = nullptr;
m_xMetaData.clear();
    m_aRowsIter = m_aRows.end();
    m_aRows.clear();
    m_aRowsIter = m_aRows.end();
}

void SAL_CALL ODatabaseMetaDataResultSet::acquire() throw()
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}

void SAL_CALL ODatabaseMetaDataResultSet::release() throw()
{
    ODatabaseMetaDataResultSet_BASE::release();
}

Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}

Sequence< Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  ) throw(RuntimeException, std::exception)
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<com::sun::star::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<com::sun::star::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<com::sun::star::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}

void ODatabaseMetaDataResultSet::setRows(const ORows& _rRows)
{
    m_aRows = _rRows;
    m_bBOF = true;
    m_bEOF = m_aRows.empty();
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
    {
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
            columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i))
            )
            return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

void ODatabaseMetaDataResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if(columnIndex >= (sal_Int32)(*m_aRowsIter).size() || columnIndex < 1)
        ::dbtools::throwInvalidIndexException(*this);
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}



sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException, std::exception)
{
    return 0;
}


sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData();

    return m_xMetaData;
}

Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}



Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return nullptr;
}


Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex).makeAny();
}


sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}




::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}



::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException, std::exception)
{
    return getValue(columnIndex);
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    return m_bEOF;
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

SAL_WNOUNREACHABLE_CODE_POP


void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
}

void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
}


void SAL_CALL ODatabaseMetaDataResultSet::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );

    }
    dispose();
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 /*row*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 /*row*/ ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

SAL_WNOUNREACHABLE_CODE_POP


Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException, std::exception)
{
    return m_aStatement.get();
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::dbtools::throwFunctionSequenceException(*this);
    return sal_False;
}

SAL_WNOUNREACHABLE_CODE_POP



sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException, std::exception)
{
    return m_bBOF;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );

    if ( m_bBOF )
    {
       m_aRowsIter = m_aRows.begin();
       m_bBOF = false;
    }
    else
    {
        if ( m_bEOF )
            throwFunctionSequenceException( *this );
        else
            if ( m_aRowsIter != m_aRows.end() )
                ++m_aRowsIter;
    }

    bool bSuccess = m_aRowsIter != m_aRows.end();
    if ( !bSuccess )
    {
        m_bEOF = true;
        m_bBOF = m_aRows.empty();
    }
    return bSuccess;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    if(m_aRowsIter == m_aRows.end() || !(*m_aRowsIter)[m_nColPos].is())
        return sal_True;

    return (*m_aRowsIter)[m_nColPos]->getValue().isNull();
}

void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException, std::exception)
{
}


void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException, std::exception)
{
}

void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
}

Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    return Any();
}

::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

void ODatabaseMetaDataResultSet::setProceduresMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setCatalogsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setSchemasMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setSchemasMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setColumnsMap()
{

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablesMap()
{

    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setProcedureColumnsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setPrimaryKeysMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setIndexInfoMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setCrossReferenceMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setVersionColumnsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setVersionColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setBestRowIdentifierMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setBestRowIdentifierMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTypeInfoMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setUDTsMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setUDTsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTableTypes()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTableTypes();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setExportedKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setExportedKeysMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setImportedKeysMap()
{
    ODatabaseMetaDataResultSetMetaData* pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setImportedKeysMap();
    m_xMetaData = pMetaData;
}

Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

ORowSetValueDecorator& ORowSetValueDecorator::operator=(const ORowSetValue& _aValue)
{
    m_aValue = _aValue;
    return *this;
}

const ORowSetValue& ODatabaseMetaDataResultSet::getValue(sal_Int32 columnIndex)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );

    if ( isBeforeFirst() || isAfterLast() )
        ::dbtools::throwFunctionSequenceException( *this );

    checkIndex(columnIndex );
    m_nColPos = columnIndex;

    if(m_aRowsIter != m_aRows.end() && (*m_aRowsIter)[columnIndex].is())
        return *(*m_aRowsIter)[columnIndex];
    return m_aEmptyValue;
}

/// return an empty ORowSetValueDecorator
ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getEmptyValue()
{
    static ORowSetValueDecoratorRef aEmptyValueRef = new ORowSetValueDecorator();
    return aEmptyValueRef;
}

/// return an ORowSetValueDecorator with 0 as value
ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::get0Value()
{
    static ORowSetValueDecoratorRef a0ValueRef = new ORowSetValueDecorator((sal_Int32)0);
    return a0ValueRef;
}

/// return an ORowSetValueDecorator with 1 as value
ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::get1Value()
{
    static ORowSetValueDecoratorRef a1ValueRef = new ORowSetValueDecorator((sal_Int32)1);
    return a1ValueRef;
}

/// return an ORowSetValueDecorator with ColumnSearch::BASIC as value
ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getBasicValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(ColumnSearch::BASIC);
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getSelectValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("SELECT"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getInsertValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("INSERT"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getDeleteValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("DELETE"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getUpdateValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("UPDATE"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getCreateValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("CREATE"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getReadValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("READ"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getAlterValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("ALTER"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getDropValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("DROP"));
    return aValueRef;
}

ORowSetValueDecoratorRef ODatabaseMetaDataResultSet::getQuoteValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("'"));
    return aValueRef;
}

void SAL_CALL ODatabaseMetaDataResultSet::initialize( const Sequence< Any >& _aArguments ) throw (Exception, RuntimeException, std::exception)
{
    if ( _aArguments.getLength() == 2 )
    {
        sal_Int32 nResultSetType = 0;
        if ( _aArguments[0] >>= nResultSetType)
        {
            setType(static_cast<MetaDataResultSetType>(nResultSetType));
            Sequence< Sequence<Any> > aRows;
            if ( _aArguments[1] >>= aRows )
            {
                ORows aRowsToSet;
                const Sequence<Any>* pRowsIter = aRows.getConstArray();
                const Sequence<Any>* pRowsEnd  = pRowsIter + aRows.getLength();
                for (; pRowsIter != pRowsEnd;++pRowsIter)
                {
                    ORow aRowToSet;
                    const Any* pRowIter = pRowsIter->getConstArray();
                    const Any* pRowEnd = pRowIter + pRowsIter->getLength();
                    for (; pRowIter != pRowEnd;++pRowIter)
                    {
                        ORowSetValueDecoratorRef aValue;
                        switch( pRowIter->getValueTypeClass() )
                        {
                            case TypeClass_BOOLEAN:
                                {
                                    bool bValue = false;
                                    *pRowIter >>= bValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(bValue));
                                }
                                break;
                            case TypeClass_BYTE:
                                {
                                    sal_Int8 nValue(0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_SHORT:
                            case TypeClass_UNSIGNED_SHORT:
                                {
                                    sal_Int16 nValue(0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_LONG:
                            case TypeClass_UNSIGNED_LONG:
                                {
                                    sal_Int32 nValue(0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_HYPER:
                            case TypeClass_UNSIGNED_HYPER:
                                {
                                    sal_Int64 nValue(0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_FLOAT:
                                {
                                    float nValue(0.0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_DOUBLE:
                                {
                                    double nValue(0.0);
                                    *pRowIter >>= nValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(nValue));
                                }
                                break;
                            case TypeClass_STRING:
                                {
                                    OUString sValue;
                                    *pRowIter >>= sValue;
                                    aValue = new ORowSetValueDecorator(ORowSetValue(sValue));
                                }
                                break;
                            default:
                                break;
                        }
                        aRowToSet.push_back(aValue);
                    }
                    aRowsToSet.push_back(aRowToSet);
                } // for (; pRowsIter != pRowsEnd;++pRowsIter
                setRows(aRowsToSet);
            }
        }
    }
}
// XServiceInfo


    OUString ODatabaseMetaDataResultSet::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.helper.DatabaseMetaDataResultSet");
    }

    Sequence< OUString > ODatabaseMetaDataResultSet::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence<OUString> aSNS { "com.sun.star.sdbc.ResultSet" };
        return aSNS;
    }

    OUString SAL_CALL ODatabaseMetaDataResultSet::getImplementationName(  ) throw(RuntimeException, std::exception)
    {
        return getImplementationName_Static();
    }

    sal_Bool SAL_CALL ODatabaseMetaDataResultSet::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL ODatabaseMetaDataResultSet::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_Static();
    }

    namespace connectivity
    {
        Reference< XInterface >  SAL_CALL ODatabaseMetaDataResultSet_CreateInstance(const Reference< XComponentContext >& ) throw( Exception )
        {
            return *(new ODatabaseMetaDataResultSet());
        }
    }


namespace
{
    cppu::ImplementationEntry entries[] = {
        { &ODatabaseMetaDataResultSet_CreateInstance, &ODatabaseMetaDataResultSet::getImplementationName_Static, &ODatabaseMetaDataResultSet::getSupportedServiceNames_Static,
            &cppu::createSingleComponentFactory, nullptr, 0 },
        { &ParameterSubstitution::create, &ParameterSubstitution::getImplementationName_Static, &ParameterSubstitution::getSupportedServiceNames_Static,
            &cppu::createSingleComponentFactory, nullptr, 0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
    };
}

extern "C"
{


SAL_DLLPUBLIC_EXPORT void* SAL_CALL dbtools_component_getFactory(const sal_Char* implName, void* serviceManager, void* registryKey)
{
    return cppu::component_getFactoryHelper(implName, serviceManager, registryKey, entries);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
