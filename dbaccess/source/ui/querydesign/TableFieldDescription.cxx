/*************************************************************************
 *
 *  $RCSfile: TableFieldDescription.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:14:37 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

DBG_NAME(OTableFieldDesc);
//==============================================================================
OTableFieldDesc::OTableFieldDesc()
             :m_bVisible(sal_False)
             ,m_pTabWindow(0)
             ,m_eOrderDir( ORDER_NONE )
             ,m_eFunctionType( FKT_NONE )
             ,m_bGroupBy(sal_False)
             ,m_nColWidth(0)
             ,m_eFieldType(TAB_NORMAL_FIELD)
             ,m_nIndex(0)
             ,m_eDataType(1000)
             ,m_nColumnId(-1)
{
    DBG_CTOR(OTableFieldDesc,NULL);
}
//------------------------------------------------------------------------------
OTableFieldDesc::OTableFieldDesc(const OTableFieldDesc& rRS)
        :m_aTableName(rRS.GetTable()),
        m_aAliasName(rRS.GetAlias()),       // table range
        m_aFieldName(rRS.GetField()),       // column
        m_aFieldAlias(rRS.GetFieldAlias()), // column alias
        m_aDatabaseName(rRS.GetDatabase()), // qualifier or catalog
        m_aFunctionName(rRS.GetFunction()), // Funktionsname
        m_bVisible(rRS.IsVisible()),
        m_pTabWindow(rRS.GetTabWindow()),
        m_eDataType(rRS.GetDataType()),
        m_eOrderDir(rRS.GetOrderDir()),
        m_nColWidth(rRS.GetColWidth()),
        m_eFieldType(rRS.GetFieldType()),
        m_nIndex(rRS.GetFieldIndex()),
        m_eFunctionType( rRS.GetFunctionType() ),
        m_bGroupBy(rRS.IsGroupBy()),
        m_vecCriteria( rRS.m_vecCriteria),
        m_nColumnId( rRS.m_nColumnId)

{
    DBG_CTOR(OTableFieldDesc,NULL);
}

//------------------------------------------------------------------------------
OTableFieldDesc::OTableFieldDesc(const ::rtl::OUString& rT, const ::rtl::OUString& rF )
             :m_bVisible(sal_False)
             ,m_pTabWindow(0)
             ,m_eOrderDir( ORDER_NONE )
             ,m_eFunctionType( FKT_NONE )
             ,m_bGroupBy(sal_False)
             ,m_nColumnId(-1)
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
                m_aDatabaseName != rDesc.GetDatabase()  ||
                m_aFunctionName != rDesc.GetFunction()  ||
                m_aFieldName != rDesc.GetField()        ||
                m_aTableName != rDesc.GetTable()        ||
                m_bGroupBy != rDesc.IsGroupBy()         ||
                m_vecCriteria != rDesc.GetCriteria()    ||
                m_bVisible != rDesc.IsVisible() );

}
// -----------------------------------------------------------------------------
void OTableFieldDesc::clear()
{
    m_vecCriteria.clear();
    ::std::vector< ::rtl::OUString>().swap( m_vecCriteria );
    m_aTableName    = m_aAliasName = m_aFieldName = m_aFieldAlias = m_aDatabaseName = m_aFunctionName = ::rtl::OUString();
    m_pTabWindow    = NULL;
    m_eDataType     = 1000;
    m_bVisible      = sal_False;
    m_eOrderDir     =  ORDER_NONE;
    m_eFunctionType = FKT_NONE;
    m_bGroupBy      = sal_False;
}

//------------------------------------------------------------------------------
void OTableFieldDesc::NextOrderDir()
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    int nTmp = ((int)m_eOrderDir);
    nTmp++;
    nTmp %= (ORDER_DESC+1);
    m_eOrderDir = EOrderDir(nTmp);
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

//------------------------------------------------------------------------------
sal_Bool OTableFieldDesc::IsNumericDataType() const
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    sal_Bool bErg(sal_False);
    switch (m_eDataType)
    {
        case DataType::BIT:
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::NUMERIC:
            bErg = sal_True;
    }
    return bErg;
}
// -----------------------------------------------------------------------------
void OTableFieldDesc::Load(const ::com::sun::star::beans::PropertyValue& _rProperty)
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    Sequence<PropertyValue> aFieldDesc;
    _rProperty.Value >>= aFieldDesc;
    if ( aFieldDesc.getLength() == 13 )
    {
        sal_Int32 nCount = aFieldDesc.getLength();
        for (sal_Int32 nPos = 0; nPos < nCount; ++nPos)
        {
            if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AliasName")) )
                aFieldDesc[nPos].Value >>= m_aAliasName;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TableName")) )
                aFieldDesc[nPos].Value >>= m_aTableName;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldName")) )
                aFieldDesc[nPos].Value >>= m_aFieldName;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldAlias")) )
                aFieldDesc[nPos].Value >>= m_aFieldAlias;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseName")) )
                aFieldDesc[nPos].Value >>= m_aDatabaseName;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionName")) )
                aFieldDesc[nPos].Value >>= m_aFunctionName;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataType")) )
                aFieldDesc[nPos].Value >>= m_eDataType;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FunctionType")) )
                aFieldDesc[nPos].Value >>= m_eFunctionType;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldType")) )
            {
                sal_Int32 nTemp = 0;
                aFieldDesc[nPos].Value >>= nTemp;
                m_eFieldType = static_cast<ETableFieldType>(nTemp);
            }
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OrderDir")) )
            {
                sal_Int32 nTemp = 0;
                aFieldDesc[nPos].Value >>= nTemp;
                m_eOrderDir = static_cast<EOrderDir>(nTemp);
            }
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ColWidth")) )
                aFieldDesc[nPos].Value >>= m_nColWidth;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GroupBy")) )
                aFieldDesc[nPos].Value >>= m_bGroupBy;
            else if ( aFieldDesc[nPos].Name == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Visible")) )
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
    aFieldDesc[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseName"));
    aFieldDesc[nPos++].Value <<= m_aDatabaseName;
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



