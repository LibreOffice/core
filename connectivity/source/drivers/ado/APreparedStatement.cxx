/*************************************************************************
 *
 *  $RCSfile: APreparedStatement.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:20 $
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
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
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

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const ::std::vector<OTypeInfo>& _TypeInfo,const ::rtl::OUString& sql)
    : OStatement_Base( _pConnection ),m_aTypeInfo(_TypeInfo)
{
    osl_incrementInterlockedCount( &m_refCount );

    CHECK_RETURN(m_Command.put_CommandText(sql))
    CHECK_RETURN(m_Command.put_Prepared(VARIANT_TRUE))
    m_pParameters = m_Command.get_Parameters();
    m_pParameters->AddRef();
    m_pParameters->Refresh();

    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_Base::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(  rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XPreparedBatchExecution*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedBatchExecution > *)0 ));

    return ::utl::concatSequences(aTypes.getTypes(),OStatement_Base::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    Reference< XResultSetMetaData > xRef = new OResultSetMetaData(m_RecordSet);
    return xRef;
}
// -------------------------------------------------------------------------
void OPreparedStatement::disposing()
{
    if(m_RecordSet.IsValid())
        m_RecordSet.Close();
    if(m_pParameters)
        m_pParameters->Release();
    OStatement_Base::disposing();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (OStatement_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    SQLWarning  warning;

    // Reset warnings

    clearWarnings ();

    // Reset the statement handle, warning and saved Resultset

    reset();

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
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ADORecordset* pSet=NULL;
    CHECK_RETURN(m_Command.Execute(m_RecordsAffected,m_Parameters,adCmdUnknown,&pSet))
    m_RecordSet = WpADORecordset(pSet);
    return m_RecordsAffected;
}

// -------------------------------------------------------------------------
void OPreparedStatement::setParameter(sal_Int32 parameterIndex, const DataTypeEnum& _eType,
                                      const sal_Int32& _nSize,const OLEVariant& _Val) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 nCount = 0;
    m_pParameters->get_Count(&nCount);
    if(!nCount)
    {

        ADOParameter* pParam = m_Command.CreateParameter(::rtl::OUString(),_eType,adParamInput,_nSize,_Val);
        if(pParam)
        {
            pParam->AddRef();
            m_pParameters->Append(pParam);
        }
    }
    else
    {
        ADOParameter* pParam = NULL;
        m_pParameters->get_Item(OLEVariant(parameterIndex-1),&pParam);
        if(pParam)
            pParam->put_Value(_Val);
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
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    WpADORecordset aSet;
    aSet.Create();
    OLEVariant aCmd;
    aCmd.setIDispatch(m_Command);
    OLEVariant aCon;
    aCon.setNoArg();
    CHECK_RETURN(aSet.put_CacheSize(m_nFetchSize))
    CHECK_RETURN(aSet.put_MaxRecords(m_nMaxRows))
    CHECK_RETURN(aSet.Open(aCmd,aCon,m_eCursorType,m_eLockType,adOpenUnspecified))

    CHECK_RETURN(aSet.get_CacheSize(m_nFetchSize))
    CHECK_RETURN(aSet.get_MaxRecords(m_nMaxRows))
    CHECK_RETURN(aSet.get_CursorType(m_eCursorType))
    CHECK_RETURN(aSet.get_LockType(m_eLockType))

    OResultSet* pSet = new OResultSet(aSet,this);
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
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(m_pParameters)
    {
        sal_Int32 nCount = 0;
        m_pParameters->get_Count(&nCount);
        for(sal_Int32 i=nCount-1;i>=0;--i)
            m_pParameters->Delete(OLEVariant(i));

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


