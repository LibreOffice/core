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


#include "TableFieldDescription.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <comphelper/namedvaluecollection.hxx>

#include <functional>

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
    : ::salhelper::SimpleReferenceObject()

{
    DBG_CTOR(OTableFieldDesc,NULL);
    *this = rRS;
}

//------------------------------------------------------------------------------
OTableFieldDesc::OTableFieldDesc(const OUString& rT, const OUString& rF )
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
OTableFieldDesc& OTableFieldDesc::operator=( const OTableFieldDesc& rRS )
{
    if (&rRS == this)
        return *this;

    m_aCriteria = rRS.GetCriteria();
    m_aTableName = rRS.GetTable();
    m_aAliasName = rRS.GetAlias();      // table range
    m_aFieldName = rRS.GetField();      // column
    m_aFieldAlias = rRS.GetFieldAlias();    // column alias
    m_aFunctionName = rRS.GetFunction();    // Funktionsname
    m_pTabWindow = rRS.GetTabWindow();
    m_eDataType = rRS.GetDataType();
    m_eFunctionType = rRS.GetFunctionType();
    m_eFieldType = rRS.GetFieldType();
    m_eOrderDir = rRS.GetOrderDir();
    m_nIndex = rRS.GetFieldIndex();
    m_nColWidth = rRS.GetColWidth();
    m_nColumnId = rRS.m_nColumnId;
    m_bGroupBy = rRS.IsGroupBy();
    m_bVisible = rRS.IsVisible();

    return *this;
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
                m_aCriteria != rDesc.GetCriteria()  ||
                m_bVisible != rDesc.IsVisible() );

}

//------------------------------------------------------------------------------
void OTableFieldDesc::SetCriteria( sal_uInt16 nIdx, const OUString& rCrit)
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    if (nIdx < m_aCriteria.size())
        m_aCriteria[nIdx] = rCrit;
    else
    {
        for(sal_Int32 i=m_aCriteria.size();i<nIdx;++i)
            m_aCriteria.push_back( OUString());
        m_aCriteria.push_back(rCrit);
    }
}

//------------------------------------------------------------------------------
OUString OTableFieldDesc::GetCriteria( sal_uInt16 nIdx ) const
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);
    OUString aRetStr;
    if( nIdx < m_aCriteria.size())
        aRetStr = m_aCriteria[nIdx];

    return aRetStr;
}

// -----------------------------------------------------------------------------
namespace
{
    struct SelectPropertyValueAsString : public ::std::unary_function< PropertyValue, OUString >
    {
        OUString operator()( const PropertyValue& i_rPropValue ) const
        {
            OUString sValue;
            OSL_VERIFY( i_rPropValue.Value >>= sValue );
            return sValue;
        }
    };
}

// -----------------------------------------------------------------------------
void OTableFieldDesc::Load( const ::com::sun::star::beans::PropertyValue& i_rSettings, const bool i_bIncludingCriteria )
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);

    ::comphelper::NamedValueCollection aFieldDesc( i_rSettings.Value );
    m_aAliasName = aFieldDesc.getOrDefault( "AliasName", m_aAliasName );
    m_aTableName = aFieldDesc.getOrDefault( "TableName", m_aTableName );
    m_aFieldName = aFieldDesc.getOrDefault( "FieldName", m_aFieldName );
    m_aFieldAlias = aFieldDesc.getOrDefault( "FieldAlias", m_aFieldAlias );
    m_aFunctionName = aFieldDesc.getOrDefault( "FunctionName", m_aFunctionName );
    m_eDataType = aFieldDesc.getOrDefault( "DataType", m_eDataType );
    m_eFunctionType = aFieldDesc.getOrDefault( "FunctionType", m_eFunctionType );
    m_nColWidth = aFieldDesc.getOrDefault( "ColWidth", m_nColWidth );
    m_bGroupBy = aFieldDesc.getOrDefault( "GroupBy", m_bGroupBy );
    m_bVisible = aFieldDesc.getOrDefault( "Visible", m_bVisible );

    m_eFieldType = static_cast< ETableFieldType >( aFieldDesc.getOrDefault( "FieldType", static_cast< sal_Int32 >( m_eFieldType ) ) );
    m_eOrderDir = static_cast< EOrderDir >( aFieldDesc.getOrDefault( "OrderDir", static_cast< sal_Int32 >( m_eOrderDir ) ) );

    if ( i_bIncludingCriteria )
    {
        const Sequence< PropertyValue > aCriteria( aFieldDesc.getOrDefault( "Criteria", Sequence< PropertyValue >() ) );
        m_aCriteria.resize( aCriteria.getLength() );
        ::std::transform(
            aCriteria.getConstArray(),
            aCriteria.getConstArray() + aCriteria.getLength(),
            m_aCriteria.begin(),
            SelectPropertyValueAsString()
        );
    }
}
//------------------------------------------------------------------------------
void OTableFieldDesc::Save( ::comphelper::NamedValueCollection& o_rSettings, const bool i_bIncludingCriteria )
{
    DBG_CHKTHIS(OTableFieldDesc,NULL);

    o_rSettings.put( "AliasName", m_aAliasName );
    o_rSettings.put( "TableName", m_aTableName );
    o_rSettings.put( "FieldName", m_aFieldName );
    o_rSettings.put( "FieldAlias", m_aFieldAlias );
    o_rSettings.put( "FunctionName", m_aFunctionName );
    o_rSettings.put( "DataType", m_eDataType );
    o_rSettings.put( "FunctionType", (sal_Int32)m_eFunctionType );
    o_rSettings.put( "FieldType", (sal_Int32)m_eFieldType );
    o_rSettings.put( "OrderDir", (sal_Int32)m_eOrderDir );
    o_rSettings.put( "ColWidth", m_nColWidth );
    o_rSettings.put( "GroupBy", m_bGroupBy );
    o_rSettings.put( "Visible", m_bVisible );

    if ( i_bIncludingCriteria )
    {
        if ( !m_aCriteria.empty() )
        {
            sal_Int32 c = 0;
            Sequence< PropertyValue > aCriteria( m_aCriteria.size() );
            for (   ::std::vector< OUString >::const_iterator crit = m_aCriteria.begin();
                    crit != m_aCriteria.end();
                    ++crit, ++c
                )
            {
                aCriteria[c].Name = OUString( "Criterion_" ) + OUString::number( c );
                aCriteria[c].Value <<= *crit;
            }

            o_rSettings.put( "Criteria", aCriteria );
        }
    }
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
