/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ACallableStatement.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:12:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADO_ACALLABLESTATEMENT_HXX_
#include "ado/ACallableStatement.hxx"
#endif

#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OCallableStatement,"com.sun.star.sdbcx.ACallableStatement","com.sun.star.sdbc.CallableStatement");

#define GET_PARAM()                                                 \
    ADOParameter* pParam = NULL;                                    \
    m_pParameters->get_Item(OLEVariant(sal_Int32(columnIndex-1)),&pParam);      \
    if(pParam)                                                      \
        pParam->get_Value(&m_aValue);
//**************************************************************
//************ Class: java.sql.CallableStatement
//**************************************************************
OCallableStatement::OCallableStatement( OConnection* _pConnection,const OTypeInfoMap& _TypeInfo,const ::rtl::OUString& sql )
                : OPreparedStatement( _pConnection, _TypeInfo, sql )
{
    m_Command.put_CommandType(adCmdStoredProc);
}
// -------------------------------------------------------------------------

Any SAL_CALL OCallableStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPreparedStatement::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< XRow*>(this));
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OCallableStatement::wasNull(  ) throw(SQLException, RuntimeException)
{
    return m_aValue.isNull();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OCallableStatement::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------
sal_Int8 SAL_CALL OCallableStatement::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OCallableStatement::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------
::com::sun::star::util::Date SAL_CALL OCallableStatement::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------
double SAL_CALL OCallableStatement::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

float SAL_CALL OCallableStatement::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OCallableStatement::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OCallableStatement::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return (sal_Int64)m_aValue.getCurrency().int64;
}
// -------------------------------------------------------------------------

Any SAL_CALL OCallableStatement::getObject( sal_Int32 /*columnIndex*/, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getObject", *this );
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OCallableStatement::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OCallableStatement::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

 ::com::sun::star::util::Time SAL_CALL OCallableStatement::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

 ::com::sun::star::util::DateTime SAL_CALL OCallableStatement::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    GET_PARAM()
    return m_aValue;
}
// -------------------------------------------------------------------------

void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& /*typeName*/ ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType,m_pConnection->getEngineType()));
        pParam->put_Direction(adParamOutput);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(sal_Int32(parameterIndex-1)),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType,m_pConnection->getEngineType()));
        pParam->put_Direction(adParamOutput);
        pParam->put_NumericScale((sal_Int8)scale);
    }
}
// -------------------------------------------------------------------------


Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBinaryStream", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getCharacterStream", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getArray", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getClob", *this );
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getBlob", *this );
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 /*columnIndex*/) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRow::getRef", *this );
    return NULL;
}
// -------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OCallableStatement::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OPreparedStatement::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCallableStatement::release() throw(::com::sun::star::uno::RuntimeException)
{
    OPreparedStatement::release();
}
// -----------------------------------------------------------------------------


