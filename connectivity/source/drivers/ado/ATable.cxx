/*************************************************************************
 *
 *  $RCSfile: ATable.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-04 09:08:24 $
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

#ifndef _CONNECTIVITY_ADO_TABLE_HXX_
#include "ado/ATable.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_INDEXES_HXX_
#include "ado/AIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_KEYS_HXX_
#include "ado/AKeys.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE ado
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;

// -------------------------------------------------------------------------
void WpADOTable::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    _ADOTable* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOTABLE_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOTABLE_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoTable::OAdoTable(sal_Bool _bCase,_ADOTable* _pTable) : OTable_TYPEDEF(_bCase)
{
    construct();
    if(_pTable)
        m_aTable = WpADOTable(_pTable);
    else
        m_aTable.Create();

    refreshColumns();
    refreshKeys();
    refreshIndexes();
}
// -------------------------------------------------------------------------
OAdoTable::OAdoTable(sal_Bool _bCase,       const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OTable_TYPEDEF(_bCase,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
{
    construct();
    m_aTable.Create();
    m_aTable.put_Name(_Name);
    {
        ADOProperties* pProps = m_aTable.get_Properties();
        pProps->AddRef();
        ADOProperty* pProp = NULL;
        pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Type")),&pProp);
        WpADOProperty aProp(pProp);
        if(pProp)
            aProp.PutValue(_Type);
        pProps->Release();
    }
    {
        ADOProperties* pProps = m_aTable.get_Properties();
        pProps->AddRef();
        ADOProperty* pProp = NULL;
        pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Description")),&pProp);
        WpADOProperty aProp(pProp);
        if(pProp)
            aProp.PutValue(_Description);
        pProps->Release();
    }


    refreshColumns();
    refreshKeys();
    refreshIndexes();
}
// -------------------------------------------------------------------------
void OAdoTable::refreshColumns()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOColumns* pColumns = m_aTable.get_Columns();
    if(pColumns)
    {
        pColumns->Refresh();

        sal_Int32 nCount = 0;
        pColumns->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOColumn* pColumn = NULL;
            pColumns->get_Item(OLEVariant(i),&pColumn);
            if(pColumn)
            {
                WpADOColumn aColumn(pColumn);
                aVector.push_back(aColumn.get_Name());
            }
        }
    }

    m_pColumns = new OColumns(*this,m_aMutex,aVector,pColumns,isCaseSensitive());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshKeys()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOKeys* pKeys = m_aTable.get_Keys();
    if(pKeys)
    {
        pKeys->Refresh();

        sal_Int32 nCount = 0;
        pKeys->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOKey* pKey = NULL;
            pKeys->get_Item(OLEVariant(i),&pKey);
            if(pKey)
            {
                WpADOKey aKey(pKey);
                aVector.push_back(aKey.get_Name());
            }
        }
    }

    m_pKeys = new OKeys(*this,m_aMutex,aVector,pKeys,isCaseSensitive());
}
// -------------------------------------------------------------------------
void OAdoTable::refreshIndexes()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOIndexes* pIndexes = m_aTable.get_Indexes();
    if(pIndexes)
    {
        pIndexes->Refresh();

        sal_Int32 nCount = 0;
        pIndexes->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOIndex* pIndex = NULL;
            pIndexes->get_Item(OLEVariant(i),&pIndex);
            if(pIndex)
            {
                WpADOIndex aIndex(pIndex);
                aVector.push_back(aIndex.get_Name());
            }
        }
    }

    m_pIndexes = new OIndexes(*this,m_aMutex,aVector,pIndexes,isCaseSensitive());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoTable::getUnoTunnelImplementationId()
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
sal_Int64 OAdoTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OAdoTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (rBHelper.bDisposed)
        throw DisposedException();

    m_aTable.put_Name(newName);
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OAdoTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (rBHelper.bDisposed)
        throw DisposedException();

        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        OAdoColumn* pColumn = (OAdoColumn*)xTunnel->getSomething(OAdoColumn:: getUnoTunnelImplementationId());
        m_aTable.get_Columns()->Delete(OLEVariant(colName));
        m_aTable.get_Columns()->Append(OLEVariant(pColumn->getColumnImpl()));
    }

    refreshColumns();

}
// -------------------------------------------------------------------------
void SAL_CALL OAdoTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (rBHelper.bDisposed)
        throw DisposedException();

    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        OAdoColumn* pColumn = (OAdoColumn*)xTunnel->getSomething(OAdoColumn:: getUnoTunnelImplementationId());
        m_aTable.get_Columns()->Delete(OLEVariant(index));
        m_aTable.get_Columns()->Append(OLEVariant(pColumn->getColumnImpl()));
    }
}
// -------------------------------------------------------------------------
void OAdoTable::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aTable.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                m_aTable.put_Name(getString(rValue));
                break;
            case PROPERTY_ID_TYPE:
                {
                    ADOProperties* pProps = m_aTable.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Type")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getString(rValue));
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_DESCRIPTION:
                {
                    ADOProperties* pProps = m_aTable.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Description")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getString(rValue));
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_SCHEMANAME:
                break;
            default:
                                throw Exception();
        }
    }
}
// -------------------------------------------------------------------------
void OAdoTable::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aTable.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aTable.get_Name();
                break;
            case PROPERTY_ID_TYPE:
                rValue <<= m_aTable.get_Type();
                break;
            case PROPERTY_ID_CATALOGNAME:
                {
                    WpADOCatalog aCat(m_aTable.get_ParentCatalog());
                    if(aCat.IsValid())
                        rValue <<= aCat.GetObjectOwner(m_aTable.get_Name(),adPermObjTable);
                }
                break;
            case PROPERTY_ID_SCHEMANAME:
                //  rValue <<= m_aTable.get_Type();
                break;
            case PROPERTY_ID_DESCRIPTION:
                {
                    ADOProperties* pProps = m_aTable.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Description")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        rValue <<= (::rtl::OUString)aProp.GetValue();
                    pProps->Release();
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
::rtl::OUString WpADOCatalog::GetObjectOwner(const ::rtl::OUString& _rName, ObjectTypeEnum _eNum)
{
    OLEVariant _rVar;
    _rVar.setNoArg();
    BSTR aBSTR;
    BSTR sStr1 = SysAllocString(_rName.getStr());
    pInterface->GetObjectOwner(sStr1,_eNum,_rVar,&aBSTR);
    SysFreeString(sStr1);
    rtl::OUString sRetStr((sal_Unicode*)aBSTR);
    SysFreeString(aBSTR);
    return sRetStr;
}
// -----------------------------------------------------------------------------

