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


#include <connectivity/sdbcx/VColumn.hxx>
#include <osl/diagnose.h>
#include <file/FPreparedStatement.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <file/FResultSetMetaData.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/dbconversion.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/PColumn.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <strings.hrc>

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

OPreparedStatement::OPreparedStatement( OConnection* _pConnection)
    : OStatement_BASE2( _pConnection )
{
}


OPreparedStatement::~OPreparedStatement()
{
}


void OPreparedStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OStatement_BASE2::disposing();

    m_xParamColumns = nullptr;
    m_xMetaData.clear();
    if(m_aParameterRow.is())
    {
        m_aParameterRow->get().clear();
        m_aParameterRow = nullptr;
    }
}

void OPreparedStatement::construct(const OUString& sql)
{
    OStatement_Base::construct(sql);

    m_aParameterRow = new OValueRefVector();
    m_aParameterRow->get().push_back(new ORowSetValueDecorator(sal_Int32(0)) );

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);

    if ( m_aSQLIterator.getStatementType() == OSQLStatementType::Select )
        m_xParamColumns = m_aSQLIterator.getParameters();
    else
    {
        m_xParamColumns = new OSQLColumns();
        // describe all parameters need for the resultset
        describeParameter();
    }

    OValueRefRow aTemp;
    OResultSet::setBoundedColumns(m_aEvaluateRow,aTemp,m_xParamColumns,xNames,false,m_xDBMetaData,m_aColMapping);
}

rtl::Reference<OResultSet> OPreparedStatement::makeResultSet()
{
    closeResultSet();

    rtl::Reference<OResultSet> xResultSet(createResultSet());
    m_xResultSet = xResultSet.get();
    initializeResultSet(xResultSet.get());
    initResultSet(xResultSet.get());
    return xResultSet;
}

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType )
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
}

css::uno::Sequence< css::uno::Type > SAL_CALL OPreparedStatement::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XPreparedStatement>::get(),
                                    cppu::UnoType<XParameters>::get(),
                                    cppu::UnoType<XResultSetMetaDataSupplier>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}


Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aSQLIterator.getSelectColumns(),m_aSQLIterator.getTables().begin()->first,m_pTable.get());
    return m_xMetaData.get();
}


void SAL_CALL OPreparedStatement::close(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    closeResultSet();
}


sal_Bool SAL_CALL OPreparedStatement::execute(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    rtl::Reference<OResultSet> xRS(makeResultSet());
    // since we don't support the XMultipleResults interface, nobody will ever get that ResultSet...
    if(xRS.is())
        xRS->dispose();

    return m_aSQLIterator.getStatementType() == OSQLStatementType::Select;
}


sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    rtl::Reference<OResultSet> xRS(makeResultSet());
    if(xRS.is())
    {
        const sal_Int32 res(xRS->getRowCountResult());
        // nobody will ever get that ResultSet...
        xRS->dispose();
        return res;
    }
    else
        return 0;
}


void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x )
{
    setParameter(parameterIndex,x);
}


Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return Reference< XConnection >(m_pConnection.get());
}


Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return makeResultSet().get();
}


void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x )
{
    setParameter(parameterIndex,static_cast<bool>(x));
}

void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const util::Date& aData )
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aData));
}

void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const util::Time& aVal )
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}


void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const util::DateTime& aVal )
{
    setParameter(parameterIndex,DBTypeConversion::toDouble(aVal));
}


void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setLong( sal_Int32 /*parameterIndex*/, sal_Int64 /*aVal*/ )
{
    throwFeatureNotImplementedSQLException( "XParameters::setLong", *this );
}


void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if ( m_aAssignValues.is() )
        (m_aAssignValues->get())[m_aParameterIndexes[parameterIndex]]->setNull();
    else
        (m_aParameterRow->get())[parameterIndex]->setNull();
}


void SAL_CALL OPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ )
{
    throwFeatureNotImplementedSQLException( "XParameters::setClob", *this );
}


void SAL_CALL OPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ )
{
    throwFeatureNotImplementedSQLException( "XParameters::setBlob", *this );
}


void SAL_CALL OPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ )
{
    throwFeatureNotImplementedSQLException( "XParameters::setArray", *this );
}


void SAL_CALL OPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ )
{
    throwFeatureNotImplementedSQLException( "XParameters::setRef", *this );
}


void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale )
{
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


void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ )
{
    setNull(parameterIndex,sqlType);
}


void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x )
{
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


void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x )
{
    setParameter(parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    setBinaryStream(parameterIndex,x,length );
}


void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    setParameter(parameterIndex,aSeq);
}


void SAL_CALL OPreparedStatement::clearParameters(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_aParameterRow->get().clear();
    m_aParameterRow->get().push_back(new ORowSetValueDecorator(sal_Int32(0)) );
}

OResultSet* OPreparedStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}

void OPreparedStatement::initResultSet(OResultSet *pResultSet)
{
    // check if we got enough parameters
    if ( (m_aParameterRow.is() && ( m_aParameterRow->get().size() -1 ) < m_xParamColumns->get().size()) ||
         (m_xParamColumns.is() && !m_aParameterRow.is() && !m_aParameterRow->get().empty()) )
         m_pConnection->throwGenericSQLException(STR_INVALID_PARA_COUNT,*this);

    pResultSet->OpenImpl();
    pResultSet->setMetaData(getMetaData());
}

void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}

void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}

void OPreparedStatement::checkAndResizeParameters(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    if ( m_aAssignValues.is() && (parameterIndex < 1 || parameterIndex >= static_cast<sal_Int32>(m_aParameterIndexes.size())) )
        throwInvalidIndexException(*this);
    else if ( static_cast<sal_Int32>(m_aParameterRow->get().size()) <= parameterIndex )
    {
        sal_Int32 i = m_aParameterRow->get().size();
        m_aParameterRow->get().resize(parameterIndex+1);
        for ( ; i <= parameterIndex; ++i)
        {
            if ( !(m_aParameterRow->get())[i].is() )
                (m_aParameterRow->get())[i] = new ORowSetValueDecorator;
        }
    }
}

void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    if(m_aAssignValues.is())
        *(m_aAssignValues->get())[m_aParameterIndexes[parameterIndex]] = x;
    else
        *((m_aParameterRow->get())[parameterIndex]) = x;
}

sal_uInt32 OPreparedStatement::AddParameter(OSQLParseNode const * pParameter, const Reference<XPropertySet>& _xCol)
{
    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument is not a parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: Error in Parse Tree");

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
                                                    ,false
                                                    ,false
                                                    ,m_aSQLIterator.isCaseSensitive()
                                                    ,OUString()
                                                    ,OUString()
                                                    ,OUString());
    m_xParamColumns->get().push_back(xParaColumn);
    return m_xParamColumns->get().size();
}

void OPreparedStatement::describeColumn(OSQLParseNode const * _pParameter, OSQLParseNode const * _pNode,const OSQLTable& _xTable)
{
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

void OPreparedStatement::describeParameter()
{
    std::vector< OSQLParseNode*> aParseNodes;
    scanParameter(m_pParseTree,aParseNodes);
    if ( !aParseNodes.empty() )
    {
        //  m_xParamColumns = new OSQLColumns();
        const OSQLTables& rTabs = m_aSQLIterator.getTables();
        if( !rTabs.empty() )
        {
            OSQLTable xTable = rTabs.begin()->second;
            for (auto const& parseNode : aParseNodes)
            {
                describeColumn(parseNode,parseNode->getParent()->getChild(0),xTable);
            }
        }
    }
}
void OPreparedStatement::initializeResultSet(OResultSet* pRS)
{
    OStatement_Base::initializeResultSet(pRS);

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

void OPreparedStatement::parseParamterElem(const OUString& _sColumnName, OSQLParseNode* pRow_Value_Constructor_Elem)
{
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
    SetAssignValue(_sColumnName, OUString(), true, nParameter);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
