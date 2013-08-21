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


#include <stdio.h>
#include "connectivity/sdbcx/VColumn.hxx"
#include <osl/diagnose.h>
#include "file/FPreparedStatement.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "file/FResultSetMetaData.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/dbconversion.hxx"
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include "connectivity/PColumn.hxx"
#include "diagnose_ex.h"
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <tools/debug.hxx>
#include "resource/file_res.hrc"

using namespace connectivity;
using namespace comphelper;
using namespace ::dbtools;
using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbc.driver.file.PreparedStatement","com.sun.star.sdbc.PreparedStatement");

DBG_NAME( file_OPreparedStatement )
// -------------------------------------------------------------------------
OPreparedStatement::OPreparedStatement( OConnection* _pConnection)
    : OStatement_BASE2( _pConnection )
    ,m_pResultSet(NULL)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::OPreparedStatement" );
    DBG_CTOR( file_OPreparedStatement, NULL );
}

// -------------------------------------------------------------------------
OPreparedStatement::~OPreparedStatement()
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::~OPreparedStatement" );
    DBG_DTOR( file_OPreparedStatement, NULL );
}

// -------------------------------------------------------------------------
void OPreparedStatement::disposing()
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::disposing" );
    ::osl::MutexGuard aGuard(m_aMutex);

    clearMyResultSet();
    OStatement_BASE2::disposing();

    if(m_pResultSet)
    {
        m_pResultSet->release();
        m_pResultSet = NULL;
    }

    m_xParamColumns = NULL;
    m_xMetaData.clear();
    if(m_aParameterRow.is())
    {
        m_aParameterRow->get().clear();
        m_aParameterRow = NULL;
    }


}
// -------------------------------------------------------------------------
void OPreparedStatement::construct(const OUString& sql)  throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::construct" );
    OStatement_Base::construct(sql);

    m_aParameterRow = new OValueRefVector();
    m_aParameterRow->get().push_back(new ORowSetValueDecorator(sal_Int32(0)) );

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);

    if ( m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT )
        m_xParamColumns = m_aSQLIterator.getParameters();
    else
    {
        m_xParamColumns = new OSQLColumns();
        // describe all parameters need for the resultset
        describeParameter();
    }

    OValueRefRow aTemp;
    OResultSet::setBoundedColumns(m_aEvaluateRow,aTemp,m_xParamColumns,xNames,sal_False,m_xDBMetaData,m_aColMapping);

    m_pResultSet = createResultSet();
    m_pResultSet->acquire();
    m_xResultSet = Reference<XResultSet>(m_pResultSet);
    initializeResultSet(m_pResultSet);
}
// -------------------------------------------------------------------------

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
        ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::getMetaData" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aSQLIterator.getSelectColumns(),m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::close" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearMyResultSet();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::execute" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    initResultSet();

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::executeUpdate" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    initResultSet();

    return m_pResultSet ? m_pResultSet->getRowCountResult() : sal_Int32(0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setString" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::getConnection" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::executeQuery" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return initResultSet();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setBoolean" );
    setParameter(parameterIndex,static_cast<bool>(x));
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setByte" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const util::Date& aData ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setDate" );
    setParameter(parameterIndex,DBTypeConversion::toDouble(aData));
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const util::Time& aVal ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setTime" );
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const util::DateTime& aVal ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setTimestamp" );
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setDouble" );
    setParameter(parameterIndex,x);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setFloat" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setInt" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 /*parameterIndex*/, sal_Int64 /*aVal*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setLong" );
    throwFeatureNotImplementedException( "XParameters::setLong", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setNull" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if ( m_aAssignValues.is() )
        (m_aAssignValues->get())[m_aParameterIndexes[parameterIndex]]->setNull();
    else
        (m_aParameterRow->get())[parameterIndex]->setNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setClob" );
    throwFeatureNotImplementedException( "XParameters::setClob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setBlob" );
    throwFeatureNotImplementedException( "XParameters::setBlob", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setArray" );
    throwFeatureNotImplementedException( "XParameters::setArray", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setRef" );
    throwFeatureNotImplementedException( "XParameters::setRef", *this );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setObjectWithInfo" );
    switch(sqlType)
    {
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            setString(parameterIndex,::comphelper::getString(x));
            break;
        default:
            ::dbtools::setObjectWithInfo(this,parameterIndex,x,sqlType,scale);
            break;
    }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setObjectNull" );
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setObject" );
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const OUString sError( m_pConnection->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", OUString::number(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
    //  setObject (parameterIndex, x, sqlType, 0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setShort" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setBytes" );
    setParameter(parameterIndex,x);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setCharacterStream" );
    setBinaryStream(parameterIndex,x,length );
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setBinaryStream" );
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    setParameter(parameterIndex,aSeq);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::clearParameters" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_aParameterRow->get().clear();
    m_aParameterRow->get().push_back(new ORowSetValueDecorator(sal_Int32(0)) );
}
// -------------------------------------------------------------------------
OResultSet* OPreparedStatement::createResultSet()
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::createResultSet" );
    return new OResultSet(this,m_aSQLIterator);
}
// -----------------------------------------------------------------------------
Reference<XResultSet> OPreparedStatement::initResultSet()
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::initResultSet" );
    m_pResultSet->clear();
    Reference<XResultSet> xRs(m_pResultSet);

    // check if we got enough parameters
    if ( (m_aParameterRow.is() && ( m_aParameterRow->get().size() -1 ) < m_xParamColumns->get().size()) ||
         (m_xParamColumns.is() && !m_aParameterRow.is() && !m_aParameterRow->get().empty()) )
         m_pConnection->throwGenericSQLException(STR_INVALID_PARA_COUNT,*this);

    m_pResultSet->OpenImpl();
    m_pResultSet->setMetaData(getMetaData());

    return xRs;
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkAndResizeParameters(sal_Int32 parameterIndex)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::checkAndResizeParameters" );
    ::connectivity::checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    if ( m_aAssignValues.is() && (parameterIndex < 1 || parameterIndex >= static_cast<sal_Int32>(m_aParameterIndexes.size())) )
        throwInvalidIndexException(*this);
    else if ( static_cast<sal_Int32>((m_aParameterRow->get()).size()) <= parameterIndex )
    {
        sal_Int32 i = m_aParameterRow->get().size();
        (m_aParameterRow->get()).resize(parameterIndex+1);
        for ( ;i <= parameterIndex+1; ++i )
        {
            if ( !(m_aParameterRow->get())[i].is() )
                (m_aParameterRow->get())[i] = new ORowSetValueDecorator;
        }
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const ORowSetValue& x)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::setParameter" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if(m_aAssignValues.is())
        *(m_aAssignValues->get())[m_aParameterIndexes[parameterIndex]] = x;
    else
        *((m_aParameterRow->get())[parameterIndex]) = x;
}
// -----------------------------------------------------------------------------
sal_uInt32 OPreparedStatement::AddParameter(OSQLParseNode * pParameter, const Reference<XPropertySet>& _xCol)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::AddParameter" );
    OSL_UNUSED( pParameter );
    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument ist kein Parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: Fehler im Parse Tree");
#if OSL_DEBUG_LEVEL > 0
    OSQLParseNode * pMark = pParameter->getChild(0);
    OSL_UNUSED( pMark );
#endif

    OUString sParameterName;
    // set up Parameter-Column:
    sal_Int32 eType = DataType::VARCHAR;
    sal_uInt32 nPrecision = 255;
    sal_Int32 nScale = 0;
    sal_Int32 nNullable = ColumnValue::NULLABLE;

    if (_xCol.is())
    {
    // Use type, precision, scale ... from the given column,
    // because this Column will get a value assigned or
    // with this Column the value will be compared.
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))         >>= eType;
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))    >>= nPrecision;
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))        >>= nScale;
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))   >>= nNullable;
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))         >>= sParameterName;
    }

    Reference<XPropertySet> xParaColumn = new connectivity::parse::OParseColumn(sParameterName
                                                    ,OUString()
                                                    ,OUString()
                                                    ,OUString()
                                                    ,nNullable
                                                    ,nPrecision
                                                    ,nScale
                                                    ,eType
                                                    ,sal_False
                                                    ,sal_False
                                                    ,m_aSQLIterator.isCaseSensitive()
                                                    ,OUString()
                                                    ,OUString()
                                                    ,OUString());
    m_xParamColumns->get().push_back(xParaColumn);
    return m_xParamColumns->get().size();
}
// -----------------------------------------------------------------------------
void OPreparedStatement::describeColumn(OSQLParseNode* _pParameter,OSQLParseNode* _pNode,const OSQLTable& _xTable)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::describeColumn" );
    Reference<XPropertySet> xProp;
    if(SQL_ISRULE(_pNode,column_ref))
    {
        OUString sColumnName,sTableRange;
        m_aSQLIterator.getColumnRange(_pNode,sColumnName,sTableRange);
        if ( !sColumnName.isEmpty() )
        {
            Reference<XNameAccess> xNameAccess = _xTable->getColumns();
            if(xNameAccess->hasByName(sColumnName))
                xNameAccess->getByName(sColumnName) >>= xProp;
            AddParameter(_pParameter,xProp);
        }
    }
    //  else
        //  AddParameter(_pParameter,xProp);
}
// -------------------------------------------------------------------------
void OPreparedStatement::describeParameter()
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::describeParameter" );
    ::std::vector< OSQLParseNode*> aParseNodes;
    scanParameter(m_pParseTree,aParseNodes);
    if ( !aParseNodes.empty() )
    {
        //  m_xParamColumns = new OSQLColumns();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if( !xTabs.empty() )
        {
            OSQLTable xTable = xTabs.begin()->second;
            ::std::vector< OSQLParseNode*>::const_iterator aIter = aParseNodes.begin();
            for (;aIter != aParseNodes.end();++aIter )
            {
                describeColumn(*aIter,(*aIter)->getParent()->getChild(0),xTable);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::initializeResultSet(OResultSet* _pResult)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::initializeResultSet" );
    OStatement_Base::initializeResultSet(_pResult);

    m_pResultSet->setParameterColumns(m_xParamColumns);
    m_pResultSet->setParameterRow(m_aParameterRow);

    // Substitute parameter (AssignValues and criteria):
    if (!m_xParamColumns->get().empty())
    {
        // begin with AssignValues
        sal_uInt16 nParaCount=0; // gives the current number of previously set Parameters

        // search for parameters to be substituted:
        size_t nCount = m_aAssignValues.is() ? m_aAssignValues->get().size() : 1; // 1 is important for the Criteria
        for (size_t j = 1; j < nCount; j++)
        {
            sal_uInt32 nParameter = (*m_aAssignValues).getParameterIndex(j);
            if (nParameter == SQL_NO_PARAMETER)
                continue;   // this AssignValue is no Parameter

            ++nParaCount; // now the Parameter is valid
        }

        if (m_aParameterRow.is() &&  (m_xParamColumns->get().size()+1) != m_aParameterRow->get().size() )
        {
            sal_Int32 i = m_aParameterRow->get().size();
            sal_Int32 nParamColumns = m_xParamColumns->get().size()+1;
            m_aParameterRow->get().resize(nParamColumns);
            for ( ;i < nParamColumns; ++i )
            {
                if ( !(m_aParameterRow->get())[i].is() )
                    (m_aParameterRow->get())[i] = new ORowSetValueDecorator;
            }
        }
        if (m_aParameterRow.is() && nParaCount < m_aParameterRow->get().size() )
            m_pSQLAnalyzer->bindParameterRow(m_aParameterRow);
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::parseParamterElem(const OUString& _sColumnName, OSQLParseNode* pRow_Value_Constructor_Elem)
{
    SAL_INFO( "connectivity.drivers", "file Ocke.Janssen@sun.com OPreparedStatement::parseParamterElem" );
    Reference<XPropertySet> xCol;
    m_xColNames->getByName(_sColumnName) >>= xCol;
    sal_Int32 nParameter = -1;
    if(m_xParamColumns.is())
    {
        OSQLColumns::Vector::const_iterator aIter = find(m_xParamColumns->get().begin(),m_xParamColumns->get().end(),_sColumnName,::comphelper::UStringMixEqual(m_pTable->isCaseSensitive()));
        if(aIter != m_xParamColumns->get().end())
            nParameter = m_xParamColumns->get().size() - (m_xParamColumns->get().end() - aIter) + 1;// +1 because the rows start at 1
    }
    if(nParameter == -1)
        nParameter = AddParameter(pRow_Value_Constructor_Elem,xCol);
    // Save number of parameter in the variable:
    SetAssignValue(_sColumnName, OUString(), sal_True, nParameter);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
