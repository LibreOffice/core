/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <connectivity/sdbcx/VColumn.hxx>
#include "MPreparedStatement.hxx"
#include "MResultSetMetaData.hxx"
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <sal/log.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mork;
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.mork.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const OUString& sql)
    :OCommonStatement(_pConnection)
    ,m_sSqlStatement(sql)
    ,m_pResultSet()
{
}

OPreparedStatement::~OPreparedStatement()
{
}

void OPreparedStatement::lateInit()
{
    if ( eSelect != parseSql( m_sSqlStatement ) )
        throw SQLException();
}

void SAL_CALL OPreparedStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OCommonStatement::disposing();

    m_xMetaData.clear();
    if(m_aParameterRow.is())
    {
        m_aParameterRow->get().clear();
        m_aParameterRow = nullptr;
    }
    m_xParamColumns = nullptr;
}

OCommonStatement::StatementType OPreparedStatement::parseSql( const OUString& sql , bool bAdjusted )
{
    StatementType eStatementType = OCommonStatement::parseSql( sql, bAdjusted );
    if ( eStatementType != eSelect )
        return eStatementType;

    m_xParamColumns = new OSQLColumns();

    // describe all parameters need for the resultset
    describeParameter();

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
    OResultSet::setBoundedColumns( m_aRow, m_xParamColumns, xNames, false, m_xDBMetaData, m_aColMapping );

    return eStatementType;
}

void OPreparedStatement::initializeResultSet( OResultSet* _pResult )
{
    OCommonStatement::initializeResultSet( _pResult );
    _pResult->setParameterRow( m_aParameterRow );
}


void OPreparedStatement::clearCachedResultSet()
{
    OCommonStatement::clearCachedResultSet();
    m_pResultSet.clear();
    m_xMetaData.clear();
}

void OPreparedStatement::cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult )
{
    OCommonStatement::cacheResultSet( _pResult );
    OSL_PRECOND( m_pResultSet == nullptr, "OPreparedStatement::parseSql: you should call clearCachedResultSet before!" );
    m_pResultSet = _pResult;
}


void SAL_CALL OPreparedStatement::acquire() throw()
{
    OCommonStatement::acquire();
}

void SAL_CALL OPreparedStatement::release() throw()
{
    OCommonStatement::release();
}

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType )
{
    Any aRet = OCommonStatement::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}

css::uno::Sequence< css::uno::Type > SAL_CALL OPreparedStatement::getTypes(  )
{
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OCommonStatement::getTypes());
}


Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    bool bReadOnly = true;
    if ( m_pResultSet.is() )
        bReadOnly = m_pResultSet->determineReadOnly();
    // if we do not have a result set, then we have not been executed, yet. In this case, assuming readonly=true is
    // okay, /me thinks.

    if ( !m_xMetaData.is() )
        m_xMetaData = new OResultSetMetaData( m_pSQLIterator->getSelectColumns(), m_pSQLIterator->getTables().begin()->first ,m_pTable,bReadOnly );

    return m_xMetaData;
}


sal_Bool SAL_CALL OPreparedStatement::execute(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    Reference< XResultSet> xResult = executeQuery();
    return xResult.is();
}


sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XStatement::executeUpdate", *this );
    return 0;
}


void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    setParameter( parameterIndex, x );
}


Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    return Reference< XConnection >(m_pConnection.get());
}


Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    // our statement has already been parsed in lateInit, no need to do all this (potentially expensive)
    // stuff again. Just execute.
    return impl_executeCurrentQuery();
}


void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 /*parameterIndex*/, sal_Bool /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBoolean", *this );
}

void SAL_CALL OPreparedStatement::setByte( sal_Int32 /*parameterIndex*/, sal_Int8 /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setByte", *this );
}


void SAL_CALL OPreparedStatement::setDate( sal_Int32 /*parameterIndex*/, const Date& /*aData*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setDate", *this );
}


void SAL_CALL OPreparedStatement::setTime( sal_Int32 /*parameterIndex*/, const css::util::Time& /*aVal*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setTime", *this );
}


void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 /*parameterIndex*/, const DateTime& /*aVal*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setTimestamp", *this );
}


void SAL_CALL OPreparedStatement::setDouble( sal_Int32 /*parameterIndex*/, double /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setDouble", *this );
}


void SAL_CALL OPreparedStatement::setFloat( sal_Int32 /*parameterIndex*/, float /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setFloat", *this );
}


void SAL_CALL OPreparedStatement::setInt( sal_Int32 /*parameterIndex*/, sal_Int32 /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setInt", *this );
}


void SAL_CALL OPreparedStatement::setLong( sal_Int32 /*parameterIndex*/, sal_Int64 /*aVal*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setLong", *this );
}


void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    checkAndResizeParameters(parameterIndex);

    (m_aParameterRow->get())[parameterIndex].setNull();
}


void SAL_CALL OPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setClob", *this );
}


void SAL_CALL OPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBlob", *this );
}


void SAL_CALL OPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setArray", *this );
}


void SAL_CALL OPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setRef", *this );
}


void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 /*parameterIndex*/, const Any& /*x*/, sal_Int32 /*sqlType*/, sal_Int32 /*scale*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setObjectWithInfo", *this );
}


void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ )
{
    setNull(parameterIndex,sqlType);
}


void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x )
{
    ::dbtools::implSetObject(this,parameterIndex,x);
}


void SAL_CALL OPreparedStatement::setShort( sal_Int32 /*parameterIndex*/, sal_Int16 /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setShort", *this );
}


void SAL_CALL OPreparedStatement::setBytes( sal_Int32 /*parameterIndex*/, const Sequence< sal_Int8 >& /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBytes", *this );
}


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 /*parameterIndex*/, const Reference< css::io::XInputStream >& /*x*/, sal_Int32 /*length*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setCharacterStream", *this );
}


void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 /*parameterIndex*/, const Reference< css::io::XInputStream >& /*x*/, sal_Int32 /*length*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XParameters::setBinaryStream", *this );
}


void SAL_CALL OPreparedStatement::clearParameters(  )
{
}

void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OCommonStatement::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}


void OPreparedStatement::checkAndResizeParameters(sal_Int32 parameterIndex)
{
    ::connectivity::checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    if ( !m_aParameterRow.is() ) {
        m_aParameterRow = new OValueVector();
        m_aParameterRow->get().push_back(sal_Int32(0));
    }

    if (static_cast<sal_Int32>(m_aParameterRow->get().size()) <= parameterIndex)
        m_aParameterRow->get().resize(parameterIndex+1);
}

void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const
ORowSetValue& x)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkAndResizeParameters(parameterIndex);

    SAL_INFO(
        "connectivity.mork",
        "setParameter(" << parameterIndex << ", '" << x.getString() << "')");
    (m_aParameterRow->get())[parameterIndex] = x;
}


void OPreparedStatement::AddParameter(OSQLParseNode const * pParameter, const Reference<XPropertySet>& _xCol)
{
    OSL_ENSURE(SQL_ISRULE(pParameter,parameter),"OResultSet::AddParameter: Argument is not a Parameter");
    OSL_ENSURE(pParameter->count() > 0,"OResultSet: error in parse tree");

    OUString sParameterName;

    // set up Parameter-Column:
    sal_Int32 eType = DataType::VARCHAR;
    sal_uInt32 nPrecision = 255;
    sal_Int32 nScale = 0;
    sal_Int32 nNullable = ColumnValue::NULLABLE;

    if (_xCol.is())
    {
        // Type, Precision, Scale ... utilize the selected Columns,
        // then this Column will get the value assigned or with this
        // Column will the value be compared.
        eType = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
        nPrecision = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
        nScale = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
        nNullable = getINT32(_xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
        _xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= sParameterName;
    }

    Reference<XPropertySet> xParaColumn = new connectivity::sdbcx::OColumn(sParameterName
                                                    ,OUString()
                                                    ,OUString()
                                                    ,OUString()
                                                    ,nNullable
                                                    ,nPrecision
                                                    ,nScale
                                                    ,eType
                                                    ,false
                                                    ,false
                                                    ,false
                                                    ,m_pSQLIterator->isCaseSensitive()
                                                    ,OUString()
                                                    ,OUString()
                                                    ,OUString());
    m_xParamColumns->get().push_back(xParaColumn);
}

void OPreparedStatement::describeColumn(OSQLParseNode const * _pParameter,
                                        OSQLParseNode const * _pNode,
                                        const OSQLTable& _xTable)
{
    Reference<XPropertySet> xProp;
    if(SQL_ISRULE(_pNode,column_ref))
    {
        OUString sColumnName,sTableRange;
        m_pSQLIterator->getColumnRange(_pNode,sColumnName,sTableRange);
        if(!sColumnName.isEmpty())
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
    scanParameter(m_pParseTree.get(), aParseNodes);
    if(!aParseNodes.empty())
    {
        m_xParamColumns = new OSQLColumns();
        const OSQLTables& rTabs = m_pSQLIterator->getTables();
        if(!rTabs.empty())
        {
            OSQLTable xTable = rTabs.begin()->second;
            for (auto const& parseNode : aParseNodes)
            {
                describeColumn(parseNode,parseNode->getParent()->getChild(0),xTable);
            }
        }
    }
}


void OPreparedStatement::scanParameter(OSQLParseNode* pParseNode,std::vector< OSQLParseNode*>& _rParaNodes)
{
    OSL_ENSURE(pParseNode != nullptr,"OResultSet: internal error: invalid ParseNode");

    // Parameter Name-Row found?
    if (SQL_ISRULE(pParseNode,parameter))
    {
        OSL_ENSURE(pParseNode->count() >= 1,"OResultSet: Faulty Parse Tree");
        OSL_ENSURE(pParseNode->getChild(0)->getNodeType() == SQLNodeType::Punctuation,"OResultSet: Faulty Parse Tree");

        _rParaNodes.push_back(pParseNode);
        // further search isn't necessary
        return;
    }

    // Search on in Parse Tree
    for (size_t i = 0; i < pParseNode->count(); i++)
        scanParameter(pParseNode->getChild(i),_rParaNodes);
}

css::uno::Reference< css::sdbc::XResultSet > SAL_CALL OPreparedStatement::getResultSet(  )
{
    return nullptr;
}

sal_Int32 SAL_CALL OPreparedStatement::getUpdateCount(  )
{
    return 0;
}

sal_Bool SAL_CALL OPreparedStatement::getMoreResults(  )
{
    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
