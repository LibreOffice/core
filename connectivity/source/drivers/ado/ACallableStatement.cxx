/*************************************************************************
 *
 *  $RCSfile: ACallableStatement.cxx,v $
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

#ifndef _CONNECTIVITY_ADO_ACALLABLESTATEMENT_HXX_
#include "ado/ACallableStatement.hxx"
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
    m_pParameters->get_Item(OLEVariant(columnIndex-1),&pParam);     \
    if(pParam)                                                      \
        pParam->get_Value(&m_aValue);
//**************************************************************
//************ Class: java.sql.CallableStatement
//**************************************************************
OCallableStatement::OCallableStatement( OConnection* _pConnection,const ::std::vector<connectivity::OTypeInfo>& _TypeInfo,const ::rtl::OUString& sql )
                : OPreparedStatement( _pConnection, _TypeInfo, sql )
{
    m_Command.put_CommandType(adCmdStoredProc);
}
// -------------------------------------------------------------------------

Any SAL_CALL OCallableStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPreparedStatement::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(rType,static_cast< XRow*>(this));
    return aRet;
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
    return (sal_Int64)m_aValue.getCurrency();
}
// -------------------------------------------------------------------------

Any SAL_CALL OCallableStatement::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
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

void SAL_CALL OCallableStatement::registerOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(parameterIndex-1),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType));
        pParam->put_Direction(adParamOutput);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OCallableStatement::registerNumericOutParameter( sal_Int32 parameterIndex, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ADOParameter* pParam = NULL;
    m_pParameters->get_Item(OLEVariant(parameterIndex-1),&pParam);
    if(pParam)
    {
        pParam->put_Type(ADOS::MapJdbc2ADOType(sqlType));
        pParam->put_Direction(adParamOutput);
        pParam->put_NumericScale(scale);
    }
}
// -------------------------------------------------------------------------


Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL OCallableStatement::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XArray > SAL_CALL OCallableStatement::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OCallableStatement::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OCallableStatement::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OCallableStatement::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    m_aValue.setNull();
    return NULL;
}
// -------------------------------------------------------------------------


