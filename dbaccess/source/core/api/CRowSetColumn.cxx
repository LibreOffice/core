/*************************************************************************
 *
 *  $RCSfile: CRowSetColumn.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:38 $
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

#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _UTL_TYPES_HXX_
#include <unotools/types.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef DBACCESS_CORE_API_CROWSETCOLUMN_HXX
#include "CRowSetColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif

using namespace dbaccess;
//  using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//  using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
//  using namespace ::com::sun::star::util;
using namespace cppu;
using namespace osl;


// -------------------------------------------------------------------------
//ORowSetColumn::ORowSetColumn( const Reference < XResultSetMetaData >& _xMetaData,
//                              const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XRow >& _xRow,
//                              sal_Int32 _nPos,
//                              ORowSetMatrix::iterator& _rColumnValue,
//                              ORowSetMatrix::iterator& _rEnd)
//  : ORowSetDataColumn(_xMetaData,_xRow,NULL,_nPos,_rColumnValue,_rEnd)
//{
//
//}
//------------------------------------------------------------------------------
Any SAL_CALL ORowSetColumn::queryInterface( const Type & _rType ) throw (RuntimeException)
{
    Any aReturn = OResultColumn::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType,static_cast< XColumn* >(this));
    return aReturn;
}
// -------------------------------------------------------------------------
// utl::OPropertyArrayUsageHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ORowSetColumn::createArrayHelper( ) const
{
    BEGIN_PROPERTY_HELPER(27)
        DECL_PROP1(ALIGN,                   sal_Int32,          MAYBEVOID);
        DECL_PROP1(CATALOGNAME,             ::rtl::OUString,    READONLY);
        DECL_PROP0_IFACE(CONTROLMODEL,      XPropertySet                );
        DECL_PROP1(DISPLAYSIZE,             sal_Int32,          READONLY);
        DECL_PROP1(NUMBERFORMAT,            sal_Int32,          MAYBEVOID);
        DECL_PROP1_BOOL(ISAUTOINCREMENT,                        READONLY);
        DECL_PROP1_BOOL(ISCASESENSITIVE,                        READONLY);
        DECL_PROP1_BOOL(ISCURRENCY,                             READONLY);
        DECL_PROP1_BOOL(ISDEFINITELYWRITABLE,                   READONLY);
        DECL_PROP0_BOOL(HIDDEN                                          );
        DECL_PROP1(ISNULLABLE,              sal_Int32,          READONLY);
        DECL_PROP1_BOOL(ISREADONLY,                             READONLY);
        DECL_PROP1_BOOL(ISSEARCHABLE,                           READONLY);
        DECL_PROP1_BOOL(ISSIGNED,                               READONLY);
        DECL_PROP1_BOOL(ISWRITABLE,                             READONLY);
        DECL_PROP1(LABEL,                   ::rtl::OUString,    READONLY);
        DECL_PROP1(NAME,                    ::rtl::OUString,    READONLY);
        DECL_PROP1(PRECISION,               sal_Int32,          READONLY);
        DECL_PROP1(RELATIVEPOSITION,        sal_Int32,          MAYBEVOID);
        DECL_PROP1(SCALE,                   sal_Int32,          READONLY);
        DECL_PROP1(SCHEMANAME,              ::rtl::OUString,    READONLY);
        DECL_PROP1(SERVICENAME,             ::rtl::OUString,    READONLY);
        DECL_PROP1(TABLENAME,               ::rtl::OUString,    READONLY);
        DECL_PROP1(TYPE,                    sal_Int32,          READONLY);
        DECL_PROP1(TYPENAME,                ::rtl::OUString,    READONLY);
        DECL_PROP2(VALUE,                   Any,                READONLY,BOUND);
        DECL_PROP1(WIDTH,                   sal_Int32,          MAYBEVOID);
    END_PROPERTY_HELPER();
}

// cppu::OPropertySetHelper
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ORowSetColumn::getInfoHelper()
{
    return *static_cast< ::utl::OPropertyArrayUsageHelper< ORowSetColumn >* >(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            OColumnSettings::getFastPropertyValue( rValue, nHandle );
            break;
        case PROPERTY_ID_VALUE:
            if(m_rColumnValue != m_rEnd && (*m_rColumnValue).isValid())
                rValue = (*(*m_rColumnValue))[m_nPos].makeAny();
            break;
        default:
            ORowSetDataColumn::getFastPropertyValue(rValue,nHandle);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL ORowSetColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue )throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            OColumnSettings::setFastPropertyValue_NoBroadcast( nHandle, rValue );
            break;
        case PROPERTY_ID_VALUE:
            break;
        default:
            ORowSetDataColumn::setFastPropertyValue_NoBroadcast(nHandle,rValue );
    }
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ORowSetColumn::convertFastPropertyValue( Any & rConvertedValue,
                                                            Any & rOldValue,
                                                            sal_Int32 nHandle,
                                                            const Any& rValue ) throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch(nHandle)
    {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HIDDEN:
        case PROPERTY_ID_CONTROLMODEL:
            bModified = OColumnSettings::convertFastPropertyValue( rConvertedValue, rOldValue, nHandle, rValue );
            break;
        case PROPERTY_ID_VALUE:
            rConvertedValue = rValue;
            bModified = !::utl::compare(rConvertedValue, rOldValue);
            break;
        default:
            bModified = ORowSetDataColumn::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);

    }

    return bModified;
}
// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > ORowSetColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XColumn > *)0 ),OColumn::getTypes());
    return aTypes.getTypes();
}
//--------------------------------------------------------------------------
//Sequence< sal_Int8 > ORowSetColumn::getImplementationId() throw (RuntimeException)
//{
//  static OImplementationId * pId = 0;
//  if (! pId)
//  {
//      MutexGuard aGuard( Mutex::getGlobalMutex() );
//      if (! pId)
//      {
//          static OImplementationId aId;
//          pId = &aId;
//      }
//  }
//  return pId->getImplementationId();
//}
// -------------------------------------------------------------------------
void ORowSetColumn::fireValueChange(const ::com::sun::star::uno::Any& _rOldValue)
{
    sal_Int32 nHandle = PROPERTY_ID_VALUE;
    Any aVal;

    getFastPropertyValue(aVal,PROPERTY_ID_VALUE);
    if(!::utl::compare(aVal,_rOldValue))
    {
        m_aOldValue = _rOldValue;

        fire(&nHandle, &aVal, &_rOldValue, 1, sal_False );
    }
}
