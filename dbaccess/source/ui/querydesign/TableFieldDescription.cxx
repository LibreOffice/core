/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableFieldDescription.cxx,v $
 * $Revision: 1.12 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace comphelper;
using namespace dbaui;

DBG_NAME(OTableFieldDesc)
//==============================================================================
OTableFieldDesc::OTableFieldDesc()
    :m_pTabWindow(0)
    ,m_eDataType(1000)
    ,m_eFunctionType( FKT_NONE )
    ,m_eFieldType(TAB_NORMAL_FIELD)
    ,m_eOrderDir( ORDER_NONE )
    ,m_nIndex(0)
    ,m_nColWidth(0)
    ,m_nColumnId((sal_uInt16)-1)
    ,m_bGroupBy(sal_False)
    ,m_bVisible(sal_False)
{
    DBG_CTOR(OTableFieldDesc,NULL);
}
//------------------------------------------------------------------------------
OTableFieldDesc::OTableFieldDesc(const OTableFieldDesc& rRS)
    : ::vos::OReference()
    ,m_vecCriteria( rRS.m_vecCriteria)
    ,m_aTableName(rRS.GetTable())
    ,m_aAliasName(rRS.GetAlias())       // table range
    ,m_aFieldName(rRS.GetField())       // column
    ,m_aFieldAlias(rRS.GetFieldAlias()) // column alias
    ,m_aFunctionName(rRS.GetFunction()) // Funktionsname
    ,m_pTabWindow(rRS.GetTabWindow())
    ,m_eDataType(rRS.GetDataType())
    ,m_eFunctionType( rRS.GetFunctionType() )
    ,m_eFieldType(rRS.GetFieldType())
    ,m_eOrderDir(rRS.GetOrderDir())
    ,m_nIndex(rRS.GetFieldIndex())
    ,m_nColWidth(rRS.GetColWidth())
    ,m_nColumnId( rRS.m_nColumnId)
    ,m_bGroupBy(rRS.IsGroupBy())
    ,m_bVisible(rRS.IsVisible())
{
    DBG_CTOR(OTableFieldDesc,NULL);
}

//------------------------------------------------------------------------------
OTableFieldDesc::OTableFieldDesc(const ::rtl::OUString& rT, const ::rtl::OUString& rF )
    :m_pTabWindow(0)
    ,m_eFunctionType( FKT_NONE )
    ,m_eOrderDir( ORDER_NONE )
    ,m_nColumnId((sal_uInt16)-1)
    ,m_bGroupBy(sal_False)
    ,m_bVisible(sal_False)
{
    DBG_CTOR(OTableFieldDesc,NULL);
    SetField( rF ); SetTable( rT );
}

//------------------------------------------------------------------------------
OTableFieldDesc::~OTableFieldDesc()
{
    DBG_DTOR(OTableFieldDesc,NULL);
}

//------------------------------------------------------------------------------
sal_Bool OTableFieldDesc::operator==( const OTableFieldDesc& rDesc )
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);

    return (    m_eOrderDir != rDesc.GetOrderDir()      ||
                m_eDataType != rDesc.GetDataType()      ||
                m_aAliasName != rDesc.GetAlias()        ||
                m_aFunctionName != rDesc.GetFunction()  ||
                m_aFieldName != rDesc.GetField()        ||
                m_aTableName != rDesc.GetTable()        ||
                m_bGroupBy != rDesc.IsGroupBy()         ||
                m_vecCriteria != rDesc.GetCriteria()    ||
                m_bVisible != rDesc.IsVisible() );

}

//------------------------------------------------------------------------------
void OTableFieldDesc::SetCriteria( sal_uInt16 nIdx, const ::rtl::OUString& rCrit)
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    if (nIdx < m_vecCriteria.size())
        m_vecCriteria[nIdx] = rCrit;
    else
    {
        for(sal_Int32 i=m_vecCriteria.size();i<nIdx;++i)
            m_vecCriteria.push_back( ::rtl::OUString());
        m_vecCriteria.push_back(rCrit);
    }
}

//------------------------------------------------------------------------------
::rtl::OUString OTableFieldDesc::GetCriteria( sal_uInt16 nIdx ) const
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    ::rtl::OUString aRetStr;
    if( nIdx < m_vecCriteria.size())
        aRetStr = m_vecCriteria[nIdx];

    return aRetStr;
}

// -----------------------------------------------------------------------------
void OTableFieldDesc::Load(const ::com::sun::star::beans::PropertyValue& _rProperty)
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    Sequence<PropertyValue> aFieldDesc;
    _rProperty.Value >>= aFieldDesc;
    //if ( aFieldDesc.getLength() == 12 )
    {
        sal_Int32 nCount = aFieldDesc.getLength();
        for (sal_Int32 nPos = 0; nPos < nCount; ++nPos)
        {
            if ( aFieldDesc[nPos].Name.equalsAscii("AliasName") )
                aFieldDesc[nPos].Value >>= m_aAliasName;
            else if ( aFieldDesc[nPos].Name.equalsAscii("TableName") )
                aFieldDesc[nPos].Value >>= m_aTableName;
            else if ( aFieldDesc[nPos].Name.equalsAscii("FieldName") )
                aFieldDesc[nPos].Value >>= m_aFieldName;
            else if ( aFieldDesc[nPos].Name.equalsAscii("FieldAlias") )
                aFieldDesc[nPos].Value >>= m_aFieldAlias;
            else if ( aFieldDesc[nPos].Name.equalsAscii("FunctionName") )
                aFieldDesc[nPos].Value >>= m_aFunctionName;
            else if ( aFieldDesc[nPos].Name.equalsAscii("DataType") )
                aFieldDesc[nPos].Value >>= m_eDataType;
            else if ( aFieldDesc[nPos].Name.equalsAscii("FunctionType") )
                aFieldDesc[nPos].Value >>= m_eFunctionType;
            else if ( aFieldDesc[nPos].Name.equalsAscii("FieldType") )
            {
                sal_Int32 nTemp = 0;
                aFieldDesc[nPos].Value >>= nTemp;
                m_eFieldType = static_cast<ETableFieldType>(nTemp);
            }
            else if ( aFieldDesc[nPos].Name.equalsAscii("OrderDir") )
            {
                sal_Int32 nTemp = 0;
                aFieldDesc[nPos].Value >>= nTemp;
                m_eOrderDir = static_cast<EOrderDir>(nTemp);
            }
            else if ( aFieldDesc[nPos].Name.equalsAscii("ColWidth") )
                aFieldDesc[nPos].Value >>= m_nColWidth;
            else if ( aFieldDesc[nPos].Name.equalsAscii("GroupBy") )
                aFieldDesc[nPos].Value >>= m_bGroupBy;
            else if ( aFieldDesc[nPos].Name.equalsAscii("Visible") )
                aFieldDesc[nPos].Value >>= m_bVisible;
        }
    }
}
//------------------------------------------------------------------------------
void OTableFieldDesc::Save(::com::sun::star::beans::PropertyValue& _rProperty)
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);


    Sequence<PropertyValue> aFieldDesc(13);
    sal_Int32 nPos = 0;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AliasName"));
    aFieldDesc[nPos++].Value <<= m_aAliasName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableName"));
    aFieldDesc[nPos++].Value <<= m_aTableName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldName"));
    aFieldDesc[nPos++].Value <<= m_aFieldName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldAlias"));
    aFieldDesc[nPos++].Value <<= m_aFieldAlias;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionName"));
    aFieldDesc[nPos++].Value <<= m_aFunctionName;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataType"));
    aFieldDesc[nPos++].Value <<= m_eDataType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionType"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)m_eFunctionType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldType"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)m_eFieldType;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OrderDir"));
    aFieldDesc[nPos++].Value <<= (sal_Int32)m_eOrderDir;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ColWidth"));
    aFieldDesc[nPos++].Value <<= m_nColWidth;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GroupBy"));
    aFieldDesc[nPos++].Value <<= m_bGroupBy;
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Visible"));
    aFieldDesc[nPos++].Value <<= m_bVisible;

    _rProperty.Value <<= aFieldDesc;
}
// -----------------------------------------------------------------------------



