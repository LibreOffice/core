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

#include "ado/AColumn.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include "ado/ACatalog.hxx"

#include <o3tl/compat_functional.hxx>

using namespace ::comphelper;

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

void WpADOColumn::Create()
{
    HRESULT         hr = -1;

    _ADOColumn* pColumn = NULL;
    hr = CoCreateInstance(ADOS::CLSID_ADOCOLUMN_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOCOLUMN_25,
                          (void**)&pColumn );


    if( !FAILED( hr ) )
    {
        operator=( pColumn );
        pColumn->Release( );
    }
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn(sal_Bool _bCase,OConnection* _pConnection,_ADOColumn* _pColumn)
    : connectivity::sdbcx::OColumn(::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),0,0,0,0,sal_False,sal_False,sal_False,_bCase)
    ,m_pConnection(_pConnection)
{
    construct();
    OSL_ENSURE(_pColumn,"Column can not be null!");
    m_aColumn = WpADOColumn(_pColumn);
    //  m_aColumn.put_ParentCatalog(_pConnection->getAdoCatalog()->getCatalog());
    fillPropertyValues();
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn(sal_Bool _bCase,OConnection* _pConnection)
    : connectivity::sdbcx::OColumn(_bCase)
    ,m_pConnection(_pConnection)
{
    m_aColumn.Create();
    m_aColumn.put_ParentCatalog(_pConnection->getAdoCatalog()->getCatalog());
    construct();
    fillPropertyValues();
    m_Type = DataType::OTHER;
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoColumn::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OAdoColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OColumn_ADO::getSomething(rId);
}
// -------------------------------------------------------------------------
void OAdoColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISASCENDING),     PROPERTY_ID_ISASCENDING,    nAttrib,&m_IsAscending, ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(static_cast< ::rtl::OUString*>(0)));
}
// -----------------------------------------------------------------------------
void OAdoColumn::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aColumn.IsValid())
    {
        const sal_Char* pAdoPropertyName = NULL;

        switch(nHandle)
        {
            case PROPERTY_ID_ISASCENDING:
                m_aColumn.put_SortOrder(::cppu::any2bool(rValue) ? adSortAscending : adSortDescending);
                break;
            case PROPERTY_ID_RELATEDCOLUMN:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aColumn.put_RelatedColumn(aVal);
                }
                break;
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aColumn.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_TYPE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aColumn.put_Type(ADOS::MapJdbc2ADOType(nVal,m_pConnection->getEngineType()));
                }
                break;
            case PROPERTY_ID_TYPENAME:
                //  rValue <<= m_pTable->getCatalog()->getConnection()->getTypeInfo()->find();
                break;
            case PROPERTY_ID_PRECISION:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aColumn.put_Precision(nVal);
                }
                break;
            case PROPERTY_ID_SCALE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    if ( !m_IsCurrency )
                        m_aColumn.put_NumericScale((sal_Int8)nVal);
                }
                break;
            case PROPERTY_ID_ISNULLABLE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    if ( nVal == ColumnValue::NULLABLE )
                        m_aColumn.put_Attributes( adColNullable );
                }
                break;
            case PROPERTY_ID_ISROWVERSION:
                break;

            case PROPERTY_ID_ISAUTOINCREMENT:
                OTools::putValue( m_aColumn.get_Properties(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Autoincrement" )), getBOOL( rValue ) );
                break;

            case PROPERTY_ID_IM001:
            case PROPERTY_ID_DESCRIPTION:
                pAdoPropertyName = "Description";
                break;

            case PROPERTY_ID_DEFAULTVALUE:
                pAdoPropertyName = "Default";
                break;
        }

        if ( pAdoPropertyName )
            OTools::putValue( m_aColumn.get_Properties(), ::rtl::OUString::createFromAscii( pAdoPropertyName ), getString( rValue ) );
    }
    OColumn_ADO::setFastPropertyValue_NoBroadcast(nHandle,rValue);
}
// -----------------------------------------------------------------------------
void OAdoColumn::fillPropertyValues()
{
    if(m_aColumn.IsValid())
    {
        m_IsAscending       = m_aColumn.get_SortOrder() == adSortAscending;
        m_ReferencedColumn  = m_aColumn.get_RelatedColumn();
        m_Name              = m_aColumn.get_Name();
        m_Precision         = m_aColumn.get_Precision();
        m_Scale             = m_aColumn.get_NumericScale();
        m_IsNullable        = ((m_aColumn.get_Attributes() & adColNullable) == adColNullable) ? ColumnValue::NULLABLE : ColumnValue::NO_NULLS;

        DataTypeEnum eType  = m_aColumn.get_Type();
        m_IsCurrency        = (eType == adCurrency);
        if ( m_IsCurrency && !m_Scale)
            m_Scale = 4;
        m_Type              = ADOS::MapADOType2Jdbc(eType);

        sal_Bool bForceTo = sal_True;
        const OTypeInfoMap* pTypeInfoMap = m_pConnection->getTypeInfo();
        const OExtendedTypeInfo* pTypeInfo = OConnection::getTypeInfoFromType(*m_pConnection->getTypeInfo(),eType,::rtl::OUString(),m_Precision,m_Scale,bForceTo);
        if ( pTypeInfo )
            m_TypeName = pTypeInfo->aSimpleType.aTypeName;
        else if ( eType == adVarBinary && ADOS::isJetEngine(m_pConnection->getEngineType()) )
        {
            ::comphelper::TStringMixEqualFunctor aCase(sal_False);
            OTypeInfoMap::const_iterator aFind = ::std::find_if(pTypeInfoMap->begin(),
                                                            pTypeInfoMap->end(),
                                                            ::o3tl::compose1(
                                                            ::std::bind2nd(aCase, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VarBinary"))),
                                                                ::o3tl::compose1(
                                                                    ::std::mem_fun(&OExtendedTypeInfo::getDBName),
                                                                    ::o3tl::select2nd<OTypeInfoMap::value_type>())
                                                                )

                                                    );

            if ( aFind != pTypeInfoMap->end() ) // change column type if necessary
            {
                eType = aFind->first;
                pTypeInfo = aFind->second;
            }

            if ( !pTypeInfo )
            {
                pTypeInfo = OConnection::getTypeInfoFromType(*m_pConnection->getTypeInfo(),adBinary,::rtl::OUString(),m_Precision,m_Scale,bForceTo);
                eType = adBinary;
            }

            if ( pTypeInfo )
            {
                m_TypeName = pTypeInfo->aSimpleType.aTypeName;
                m_Type  = ADOS::MapADOType2Jdbc(eType);
            }
        }


        // fill some specific props
        {
            WpADOProperties aProps( m_aColumn.get_Properties() );

            if ( aProps.IsValid() )
            {
                m_IsAutoIncrement = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Autoincrement")) );

                m_Description = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Description")) );

                m_DefaultValue = OTools::getValue( aProps, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Default")) );

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
    }
}
// -----------------------------------------------------------------------------
WpADOColumn OAdoColumn::getColumnImpl() const
{
    return m_aColumn;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OAdoColumn::acquire() throw()
{
    OColumn_ADO::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OAdoColumn::release() throw()
{
    OColumn_ADO::release();
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
