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

#include "connectivity/sqlparse.hxx"
#include "ado/APreparedStatement.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "ado/AResultSetMetaData.hxx"
#include "ado/AResultSet.hxx"
#include "ado/ADriver.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include "connectivity/dbexception.hxx"
#include "connectivity/dbtools.hxx"
#include "resource/ado_res.hrc"

#include <limits>

#define CHECK_RETURN(x)                                                 \
    if(!x)                                                              \
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);

#ifdef max
#   undef max
#endif

using namespace connectivity::ado;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.APreparedStatement","com.sun.star.sdbc.PreparedStatement");

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const OTypeInfoMap& _TypeInfo,const OUString& sql)
    : OStatement_Base( _pConnection )
    ,m_aTypeInfo(_TypeInfo)
{
    osl_atomic_increment( &m_refCount );

    OSQLParser aParser(comphelper::getComponentContext(_pConnection->getDriver()->getORB()));
    OUString sErrorMessage;
    OUString sNewSql;
    OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,sql);
    if(pNode)
    {   // special handling for parameters
        //  we recusive replace all occurrences of ? in the statement and
        //  replace them with name like "parame" */
        sal_Int32 nParameterCount = 0;
        OUString sDefaultName( "parame" );
        replaceParameterNodeName(pNode,sDefaultName,nParameterCount);
        pNode->parseNodeToStr( sNewSql, _pConnection );
        delete pNode;
    }
    else
        sNewSql = sql;
    CHECK_RETURN(m_Command.put_CommandText(sNewSql))
    CHECK_RETURN(m_Command.put_Prepared(VARIANT_TRUE))
    m_pParameters = m_Command.get_Parameters();
    m_pParameters->AddRef();
    m_pParameters->Refresh();

    osl_atomic_decrement( &m_refCount );
}

OPreparedStatement::~OPreparedStatement()
{
    if (m_pParameters)
    {
        OSL_FAIL( "OPreparedStatement::~OPreparedStatement: not disposed!" );
        m_pParameters->Release();
        m_pParameters = NULL;
    }
}

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_Base::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XPreparedBatchExecution*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedBatchExecution > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_Base::getTypes());
}

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    if(!m_xMetaData.is() && m_RecordSet.IsValid())
        m_xMetaData = new OResultSetMetaData(m_RecordSet);
    return m_xMetaData;
}

void OPreparedStatement::disposing()
{
m_xMetaData.clear();
    if (m_pParameters)
    {
        m_pParameters->Release();
        m_pParameters = NULL;
    }
    OStatement_Base::disposing();
}

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();

}

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    SQLWarning  warning;
    clearWarnings ();

    // Call SQLExecute
    try {
        ADORecordset* pSet=NULL;
        CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdUnknown,&pSet))
        m_RecordSet = WpADORecordset(pSet);
    }
    catch (SQLWarning& ex)
    {
        // Save pointer to warning and save with ResultSet
        // object once it is created.

        warning = ex;
    }
    return m_RecordSet.IsValid();
}

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    ADORecordset* pSet=NULL;
    CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdUnknown,&pSet))
    if ( VT_ERROR == m_RecordsAffected.getType() )
    {
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);
        // to be sure that we get the error really thrown
        throw SQLException();
    }
    m_RecordSet = WpADORecordset(pSet);
    return  static_cast<sal_Int32>(m_RecordsAffected);
}

void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const DataTypeEnum& _eType,
                                      const sal_Int32& _nSize,const OLEVariant& _Val) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 nCount = 0;
    m_pParameters->get_Count(&nCount);
    if(nCount < (parameterIndex-1))
    {
        OUString sDefaultName( "parame" );
        sDefaultName += OUString::number(parameterIndex);
        ADOParameter* pParam = m_Command.CreateParameter(sDefaultName,_eType,adParamInput,_nSize,_Val);
        if(pParam)
        {
            m_pParameters->Append(pParam);
#if OSL_DEBUG_LEVEL > 0
            ADOParameter* pParam = NULL;
            m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
            WpADOParameter aParam(pParam);
            if(pParam)
            {
                DataTypeEnum eType = aParam.GetADOType();
                (void)eType;
            }
#endif
        }
    }
    else
    {
        ADOParameter* pParam = NULL;
        m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
        WpADOParameter aParam(pParam);
        if(pParam)
        {
#if OSL_DEBUG_LEVEL > 0
            OUString sParam = aParam.GetName();

#endif // OSL_DEBUG_LEVEL

            DataTypeEnum eType = aParam.GetADOType();
            if ( _eType != eType && _eType != adDBTimeStamp )
            {
                aParam.put_Type(_eType);
                eType = _eType;
                aParam.put_Size(_nSize);
            }

            if ( adVarBinary == eType && aParam.GetAttributes() == adParamLong )
            {
                aParam.AppendChunk(_Val);
            }
            else
                CHECK_RETURN(aParam.PutValue(_Val));
        }
    }
    ADOS::ThrowException(*m_pConnection->getConnection(),*this);
}

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const OUString& x ) throw(SQLException, RuntimeException)
{
    setParameter( parameterIndex, adLongVarWChar, ::std::numeric_limits< sal_Int32 >::max(), x );
}

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // first clear the old things
m_xMetaData.clear();
    disposeResultSet();
    if(m_RecordSet.IsValid())
        m_RecordSet.Close();
    m_RecordSet.clear();

    // the create the new onces
    m_RecordSet.Create();
    OLEVariant aCmd;
    aCmd.setIDispatch(m_Command);
    OLEVariant aCon;
    aCon.setNoArg();
    CHECK_RETURN(m_RecordSet.put_CacheSize(m_nFetchSize))
    CHECK_RETURN(m_RecordSet.put_MaxRecords(m_nMaxRows))
    CHECK_RETURN(m_RecordSet.Open(aCmd,aCon,m_eCursorType,m_eLockType,adOpenUnspecified))
    CHECK_RETURN(m_RecordSet.get_CacheSize(m_nFetchSize))
    CHECK_RETURN(m_RecordSet.get_MaxRecords(m_nMaxRows))
    CHECK_RETURN(m_RecordSet.get_CursorType(m_eCursorType))
    CHECK_RETURN(m_RecordSet.get_LockType(m_eLockType))

    OResultSet* pSet = new OResultSet(m_RecordSet,this);
    Reference< XResultSet > xRs = pSet;
    pSet->construct();
    pSet->setMetaData(getMetaData());
    m_xResultSet = WeakReference<XResultSet>(xRs);

    return xRs;
}

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adBoolean,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adTinyInt,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBDate,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBTime,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBTimeStamp,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDouble,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adSingle,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adInteger,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adBigInt,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 /*sqlType*/ ) throw(SQLException, RuntimeException)
{
    OLEVariant aVal;
    aVal.setNull();
    setParameter(parameterIndex,adEmpty,0,aVal);
}

void SAL_CALL OPreparedStatement::setClob( sal_Int32 /*parameterIndex*/, const Reference< XClob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::setClob", *this );
}

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 /*parameterIndex*/, const Reference< XBlob >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::setBlob", *this );
}

void SAL_CALL OPreparedStatement::setArray( sal_Int32 /*parameterIndex*/, const Reference< XArray >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::setArray", *this );
}

void SAL_CALL OPreparedStatement::setRef( sal_Int32 /*parameterIndex*/, const Reference< XRef >& /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::setRef", *this );
}

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
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

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    setNull(parameterIndex,sqlType);
}

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if(!::dbtools::implSetObject(this,parameterIndex,x))
    {
        const OUString sError( m_pConnection->getResources().getResourceStringWithSubstitution(
                STR_UNKNOWN_PARA_TYPE,
                "$position$", OUString::number(parameterIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adSmallInt,sizeof(x),x);
}

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adVarBinary,sizeof(sal_Int8)*x.getLength(),x);
}

void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 /*parameterIndex*/, const Reference< ::com::sun::star::io::XInputStream >& /*x*/, sal_Int32 /*length*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XParameters::setCharacterStream", *this );
}

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(x.is())
    {
        Sequence< sal_Int8 > aData;
        x->readBytes(aData,length);
        setBytes(parameterIndex,aData);
    }
}

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    if(m_pParameters)
    {
        sal_Int32 nCount = 0;
        m_pParameters->get_Count(&nCount);
        OLEVariant aVal;
        aVal.setEmpty();
        for(sal_Int32 i=0;i<nCount;++i)
        {
            ADOParameter* pParam = NULL;
            m_pParameters->get_Item(OLEVariant(i),&pParam);
            WpADOParameter aParam(pParam);
            if(pParam)
            {
                OUString sParam = aParam.GetName();
                CHECK_RETURN(aParam.PutValue(aVal));
            }
        }
    }
}

void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
}

void SAL_CALL OPreparedStatement::addBatch( ) throw(SQLException, RuntimeException)
{
}

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 > ();
}

void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_Base::acquire();
}

void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_Base::release();
}

void OPreparedStatement::replaceParameterNodeName(OSQLParseNode* _pNode,
                                                  const OUString& _sDefaultName,
                                                  sal_Int32& _rParameterCount)
{
    sal_Int32 nCount = _pNode->count();
    for(sal_Int32 i=0;i < nCount;++i)
    {
        OSQLParseNode* pChildNode = _pNode->getChild(i);
        if(SQL_ISRULE(pChildNode,parameter) && pChildNode->count() == 1)
        {
            OSQLParseNode* pNewNode = new OSQLParseNode(OUString(":") ,SQL_NODE_PUNCTUATION,0);
            delete pChildNode->replace(pChildNode->getChild(0),pNewNode);
            OUString sParameterName = _sDefaultName;
            sParameterName += OUString::number(++_rParameterCount);
            pChildNode->append(new OSQLParseNode( sParameterName,SQL_NODE_NAME,0));
        }
        else
            replaceParameterNodeName(pChildNode,_sDefaultName,_rParameterCount);

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
