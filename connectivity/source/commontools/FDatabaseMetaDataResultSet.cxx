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


#include <ParameterSubstitution.hxx>
#include <FDatabaseMetaDataResultSet.hxx>
#include <FDatabaseMetaDataResultSetMetaData.hxx>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbexception.hxx>
#include <o3tl/unreachable.hxx>
#include <TConnection.hxx>

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

void SAL_CALL ODatabaseMetaDataResultSet::acquire() noexcept
{
    ODatabaseMetaDataResultSet_BASE::acquire();
}

void SAL_CALL ODatabaseMetaDataResultSet::release() noexcept
{
    ODatabaseMetaDataResultSet_BASE::release();
}

Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    return aRet.hasValue() ? aRet : ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
}

Sequence< Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                    cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                    cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}

void ODatabaseMetaDataResultSet::setRows(const ORows& _rRows)
{
    m_aRows = _rRows;
    m_bBOF = true;
    m_bEOF = m_aRows.empty();
}

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const OUString& columnName )
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
    O3TL_UNREACHABLE;
}

void ODatabaseMetaDataResultSet::checkIndex(sal_Int32 columnIndex )
{
    if(columnIndex >= static_cast<sal_Int32>((*m_aRowsIter).size()) || columnIndex < 1)
        ::dbtools::throwInvalidIndexException(*this);
}

Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}

Reference< css::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex )
{
    return bool(getValue(columnIndex));
}


sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


css::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  )
{
    return 0;
}


sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    if(!m_xMetaData.is())
        m_xMetaData = new ODatabaseMetaDataResultSetMetaData();

    return m_xMetaData;
}

Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}

Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    return getValue(columnIndex).makeAny();
}


sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


css::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


css::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex )
{
    return getValue(columnIndex);
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  )
{
    return m_bEOF;
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

SAL_WNOUNREACHABLE_CODE_POP


void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
}

void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
}


void SAL_CALL ODatabaseMetaDataResultSet::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );

    }
    dispose();
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 /*row*/ )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 /*row*/ )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

SAL_WNOUNREACHABLE_CODE_POP


Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  )
{
    return m_aStatement.get();
}


SAL_WNOUNREACHABLE_CODE_PUSH

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  )
{
    ::dbtools::throwFunctionSequenceException(*this);
    return false;
}

SAL_WNOUNREACHABLE_CODE_POP


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  )
{
    return m_bBOF;
}


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  )
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


sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed );


    if(m_aRowsIter == m_aRows.end() || !(*m_aRowsIter)[m_nColPos].is())
        return true;

    return (*m_aRowsIter)[m_nColPos]->getValue().isNull();
}

void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  )
{
}


void SAL_CALL ODatabaseMetaDataResultSet::cancel(  )
{
}

void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  )
{
}

Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  )
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
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setProceduresMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setCatalogsMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setCatalogsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setSchemasMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setSchemasMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setColumnPrivilegesMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setColumnPrivilegesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setColumnsMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablesMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTablesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setProcedureColumnsMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setProcedureColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setPrimaryKeysMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setPrimaryKeysMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setIndexInfoMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setIndexInfoMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTablePrivilegesMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTablePrivilegesMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setCrossReferenceMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setCrossReferenceMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setVersionColumnsMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setVersionColumnsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setBestRowIdentifierMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setBestRowIdentifierMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTypeInfoMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTypeInfoMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setUDTsMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setUDTsMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setTableTypes()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setTableTypes();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setExportedKeysMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setExportedKeysMap();
    m_xMetaData = pMetaData;
}

void ODatabaseMetaDataResultSet::setImportedKeysMap()
{
    rtl::Reference<ODatabaseMetaDataResultSetMetaData> pMetaData = new ODatabaseMetaDataResultSetMetaData();
    pMetaData->setImportedKeysMap();
    m_xMetaData = pMetaData;
}

Reference< css::beans::XPropertySetInfo > SAL_CALL ODatabaseMetaDataResultSet::getPropertySetInfo(  )
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
ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getEmptyValue()
{
    static ORowSetValueDecoratorRef aEmptyValueRef = new ORowSetValueDecorator();
    return aEmptyValueRef;
}

/// return an ORowSetValueDecorator with 0 as value
ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::get0Value()
{
    static ORowSetValueDecoratorRef a0ValueRef = new ORowSetValueDecorator(sal_Int32(0));
    return a0ValueRef;
}

/// return an ORowSetValueDecorator with 1 as value
ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::get1Value()
{
    static ORowSetValueDecoratorRef a1ValueRef = new ORowSetValueDecorator(sal_Int32(1));
    return a1ValueRef;
}

/// return an ORowSetValueDecorator with ColumnSearch::BASIC as value
ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getBasicValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(ColumnSearch::BASIC);
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getSelectValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("SELECT"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getInsertValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("INSERT"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getDeleteValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("DELETE"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getUpdateValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("UPDATE"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getCreateValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("CREATE"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getReadValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("READ"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getAlterValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("ALTER"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getDropValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("DROP"));
    return aValueRef;
}

ORowSetValueDecoratorRef const & ODatabaseMetaDataResultSet::getQuoteValue()
{
    static ORowSetValueDecoratorRef aValueRef = new ORowSetValueDecorator(OUString("'"));
    return aValueRef;
}

void SAL_CALL ODatabaseMetaDataResultSet::initialize( const Sequence< Any >& _aArguments )
{
    if ( _aArguments.getLength() != 2 )
        return;

    sal_Int32 nResultSetType = 0;
    if ( !(_aArguments[0] >>= nResultSetType))
        return;

    setType(static_cast<MetaDataResultSetType>(nResultSetType));
    Sequence< Sequence<Any> > aRows;
    if ( !(_aArguments[1] >>= aRows) )
        return;

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
// XServiceInfo


    OUString SAL_CALL ODatabaseMetaDataResultSet::getImplementationName(  )
    {
        return "org.openoffice.comp.helper.DatabaseMetaDataResultSet";
    }

    sal_Bool SAL_CALL ODatabaseMetaDataResultSet::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    Sequence< OUString > SAL_CALL ODatabaseMetaDataResultSet::getSupportedServiceNames(  )
    {
        return Sequence<OUString>{ "com.sun.star.sdbc.ResultSet" };
    }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_dbtools_ODatabaseMetaDataResultSet_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ODatabaseMetaDataResultSet());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
