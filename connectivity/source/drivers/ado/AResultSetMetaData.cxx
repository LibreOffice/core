/*************************************************************************
 *
 *  $RCSfile: AResultSetMetaData.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:36:29 $
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

#ifndef _CONNECTIVITY_ADO_ARESULTSETMETADATA_HXX_
#include "ado/AResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif

#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
OResultSetMetaData::~OResultSetMetaData()
{
    m_pRecordSet->Release();
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getColumnDisplaySize( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid() && aField.GetActualSize() != -1)
        return aField.GetActualSize();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnType( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    return ADOS::MapADOType2Jdbc(aField.GetADOType());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::getColumnCount(  ) throw(SQLException, RuntimeException)
{
    if(m_nColCount != -1)
        return m_nColCount;

    ADOFields* pFields  = NULL;
    m_pRecordSet->get_Fields(&pFields);
    WpOLEAppendCollection<ADOFields, ADOField, WpADOField>  aFields(pFields);
    m_nColCount = aFields.GetItemCount();
    return m_nColCount;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCaseSensitive( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            bRet = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ISCASESENSITIVE")) );
    }
    return bRet;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getSchemaName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OResultSetMetaData::getColumnName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetName();

    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getTableName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    ::rtl::OUString sTableName;

    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
            sTableName = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BASETABLENAME")) );
    }
    return sTableName;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getCatalogName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnTypeName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnLabel( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return getColumnName(column);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OResultSetMetaData::getColumnServiceName( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isCurrency( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return ((aField.GetAttributes() & adFldFixed) == adFldFixed) && (aField.GetADOType() == adCurrency);
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isAutoIncrement( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = sal_False;
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if ( aField.IsValid() )
    {
        WpADOProperties aProps( aField.get_Properties() );
        if ( aProps.IsValid() )
        {
            bRet = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ISAUTOINCREMENT")) );
#if OSL_DEBUG_LEVEL > 0
            sal_Int32 nCount = aProps.GetItemCount();
            for (sal_Int32 i = 0; i<nCount; ++i)
            {
                WpADOProperty aProp = aProps.GetItem(i);
                ::rtl::OUString sName = aProp.GetName();
                ::rtl::OUString sVal = aProp.GetValue();
            }
#endif
        }
    }
    return bRet;
}
// -------------------------------------------------------------------------


sal_Bool SAL_CALL OResultSetMetaData::isSigned( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldNegativeScale) == adFldNegativeScale;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getPrecision( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetPrecision();
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSetMetaData::getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
        return aField.GetNumericScale();
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSetMetaData::isNullable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldIsNullable) == adFldIsNullable;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isSearchable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return sal_True;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isReadOnly( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        //  return (aField.GetStatus() & adFieldReadOnly) == adFieldReadOnly;
    }
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSetMetaData::isDefinitelyWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    WpADOField aField = ADOS::getField(m_pRecordSet,column);
    if(aField.IsValid())
    {
        return (aField.GetAttributes() & adFldUpdatable) == adFldUpdatable;
    }
    return sal_False;
;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSetMetaData::isWritable( sal_Int32 column ) throw(SQLException, RuntimeException)
{
    return isDefinitelyWritable(column);
}
// -------------------------------------------------------------------------

