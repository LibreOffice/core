/*************************************************************************
 *
 *  $RCSfile: APreparedStatement.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:57:43 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_APREPAREDSTATEMENT_HXX_
#include "ado/APreparedStatement.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_ARESULTSETMETADATA_HXX_
#include "ado/AResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ARESULTSET_HXX_
#include "ado/AResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ADRIVER_HXX_
#include "ado/ADriver.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif


#define CHECK_RETURN(x)                                                 \
    if(!x)                                                              \
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
using namespace connectivity::ado;
using namespace connectivity;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;


IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.APreparedStatement","com.sun.star.sdbc.PreparedStatement");

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const OTypeInfoMap& _TypeInfo,const ::rtl::OUString& sql)
    : OStatement_Base( _pConnection )
    ,m_aTypeInfo(_TypeInfo)
{
    osl_incrementInterlockedCount( &m_refCount );

    OSQLParser aParser(_pConnection->getDriver()->getORB());
    ::rtl::OUString sErrorMessage;
    ::rtl::OUString sNewSql;
    OSQLParseNode* pNode = aParser.parseTree(sErrorMessage,sql);
    if(pNode)
    {   // special handling for parameters
        // we recusive replace all occurences of ? in the statement and replace them with name like "æ¬å"
        sal_Int32 nParameterCount = 0;
        ::rtl::OUString sDefaultName = ::rtl::OUString::createFromAscii("parame");
        replaceParameterNodeName(pNode,sDefaultName,nParameterCount);
        pNode->parseNodeToStr(sNewSql,_pConnection->getMetaData());
        delete pNode;
    }
    else
        sNewSql = sql;
    CHECK_RETURN(m_Command.put_CommandText(sNewSql))
    CHECK_RETURN(m_Command.put_Prepared(VARIANT_TRUE))
    m_pParameters = m_Command.get_Parameters();
    m_pParameters->AddRef();
    m_pParameters->Refresh();

    osl_decrementInterlockedCount( &m_refCount );
}

// -------------------------------------------------------------------------
OPreparedStatement::~OPreparedStatement()
{
    if (m_pParameters)
    {
        OSL_ENSURE( sal_False, "OPreparedStatement::~OPreparedStatement: not disposed!" );
        m_pParameters->Release();
        m_pParameters = NULL;
    }
}

// -------------------------------------------------------------------------

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_Base::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface( rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XPreparedBatchExecution*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedBatchExecution > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_Base::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_RecordSet);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
void OPreparedStatement::disposing()
{
    m_xMetaData = NULL;
    if (m_pParameters)
    {
        m_pParameters->Release();
        m_pParameters = NULL;
    }
    OStatement_Base::disposing();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    SQLWarning  warning;

    // Reset warnings

    clearWarnings ();

    // Reset the statement handle, warning and saved Resultset

    //  reset();

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
// -------------------------------------------------------------------------

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

// -------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const DataTypeEnum& _eType,
                                      const sal_Int32& _nSize,const OLEVariant& _Val) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    sal_Int32 nCount = 0;
    m_pParameters->get_Count(&nCount);
    if(nCount < (parameterIndex-1))
    {
        ::rtl::OUString sDefaultName = ::rtl::OUString::createFromAscii("parame");
        sDefaultName += ::rtl::OUString::valueOf(parameterIndex);
        ADOParameter* pParam = m_Command.CreateParameter(sDefaultName,_eType,adParamInput,_nSize,_Val);
        if(pParam)
            m_pParameters->Append(pParam);
    }
    else
    {
        ADOParameter* pParam = NULL;
        m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
        WpADOParameter aParam(pParam);
        if(pParam)
        {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OUString sParam = aParam.GetName();

#endif // OSL_DEBUG_LEVEL

            DataTypeEnum eType = aParam.GetADOType();
            if ( _eType != eType )
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
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adLongVarWChar,x.getLength(),x);
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // first clear the old things
    m_xMetaData = NULL;
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
    Reference< XResultSet > pRs = pSet;
    m_xResultSet = WeakReference<XResultSet>(pRs);

    return m_xResultSet;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adBoolean,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adTinyInt,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBDate,sizeof(x),x);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBTime,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDBTimeStamp,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adDouble,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adSingle,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adInteger,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adBigInt,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    OLEVariant aVal;
    aVal.setNull();
    setParameter(parameterIndex,adEmpty,0,aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    //  setObject (parameterIndex, x, sqlType, 0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adSmallInt,sizeof(x),x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    setParameter(parameterIndex,adVarBinary,sizeof(sal_Int8)*x.getLength(),x);
}

// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(x.is())
    {
        Sequence< sal_Int8 > aData;
        x->readBytes(aData,length);
        setBytes(parameterIndex,aData);
    }
}
// -------------------------------------------------------------------------

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
                ::rtl::OUString sParam = aParam.GetName();
                CHECK_RETURN(aParam.PutValue(aVal));
            }
        }
            //  m_pParameters->Delete(OLEVariant(i));

    }
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
    //  clearParameters(  );
    //  m_aBatchList.erase();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::addBatch( ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 > ();
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_Base::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw(::com::sun::star::uno::RuntimeException)
{
    OStatement_Base::release();
}
// -----------------------------------------------------------------------------
void OPreparedStatement::replaceParameterNodeName(OSQLParseNode* _pNode,
                                                  const ::rtl::OUString& _sDefaultName,
                                                  sal_Int32& _rParameterCount)
{
    sal_Int32 nCount = _pNode->count();
    for(sal_Int32 i=0;i < nCount;++i)
    {
        OSQLParseNode* pChildNode = _pNode->getChild(i);
        if(SQL_ISRULE(pChildNode,parameter) && pChildNode->count() == 1)
        {
            OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString::createFromAscii(":") ,SQL_NODE_PUNCTUATION,0);
            delete pChildNode->replace(pChildNode->getChild(0),pNewNode);
            ::rtl::OUString sParameterName = _sDefaultName;
            sParameterName += ::rtl::OUString::valueOf(++_rParameterCount);
            pChildNode->append(new OSQLParseNode( sParameterName,SQL_NODE_NAME,0));
        }
        else
            replaceParameterNodeName(pChildNode,_sDefaultName,_rParameterCount);

    }
}
// -----------------------------------------------------------------------------



