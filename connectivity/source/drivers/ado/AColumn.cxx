/*************************************************************************
 *
 *  $RCSfile: AColumn.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 11:23:39 $
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

#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

void WpADOColumn::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    _ADOColumn* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOCOLUMN_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOCOLUMN_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn(sal_Bool _bCase,_ADOColumn* _pColumn) : connectivity::sdbcx::OColumn(_bCase)
{
    if(_pColumn)
        m_aColumn = WpADOColumn(_pColumn);
    else
        m_aColumn.Create();
}
// -------------------------------------------------------------------------
OAdoColumn::OAdoColumn( const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase
                ) : connectivity::sdbcx::OColumn(_Name,
                                  _TypeName,
                                  _DefaultValue,
                                  _IsNullable,
                                  _Precision,
                                  _Scale,
                                  _Type,
                                  _IsAutoIncrement,
                                  sal_False,_IsCurrency,_bCase)
{
    m_aColumn.Create();
    m_aColumn.put_Name(_Name);
    m_aColumn.put_Type(ADOS::MapJdbc2ADOType(_Type));
    m_aColumn.put_Precision(_Precision);
    if(_IsNullable)
        m_aColumn.put_Attributes(adColNullable);
    {
        ADOProperties* pProps = m_aColumn.get_Properties();
        pProps->AddRef();
        ADOProperty* pProp = NULL;
        pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("AutoIncrement")),&pProp);
        WpADOProperty aProp(pProp);
        if(pProp)
            aProp.PutValue(_IsAutoIncrement);
        pProps->Release();
    }
    {
        ADOProperties* pProps = m_aColumn.get_Properties();
        pProps->AddRef();
        ADOProperty* pProp = NULL;
        pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Default")),&pProp);
        WpADOProperty aProp(pProp);
        if(pProp)
            aProp.PutValue(_DefaultValue);
        pProps->Release();
    }
}
// -------------------------------------------------------------------------
Any SAL_CALL OAdoColumn::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;
    return OColumn_ADO::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OAdoColumn::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OColumn_ADO::getTypes());
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
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}


typedef connectivity::sdbcx::ODescriptor    ODescriptor_TYPEDEF;
// -------------------------------------------------------------------------
//void OAdoColumn::construct()
//{
//  ODescriptor_TYPEDEF::construct();
//      sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
//
//  registerProperty(PROPERTY_TYPENAME,         PROPERTY_ID_TYPENAME,           nAttrib,&m_TypeName,        ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
//  registerProperty(PROPERTY_DEFAULTVALUE,     PROPERTY_ID_DEFAULTVALUE,       nAttrib,&m_DefaultValue,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
//  registerProperty(PROPERTY_PRECISION,        PROPERTY_ID_PRECISION,          nAttrib,&m_Precision,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
//  registerProperty(PROPERTY_TYPE,             PROPERTY_ID_TYPE,               nAttrib,&m_Type,            ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
//  registerProperty(PROPERTY_SCALE,            PROPERTY_ID_SCALE,              nAttrib,&m_Scale,           ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
//  registerProperty(PROPERTY_ISNULLABLE,       PROPERTY_ID_ISNULLABLE,         nAttrib,&m_IsNullable,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
//  registerProperty(PROPERTY_ISAUTOINCREMENT,  PROPERTY_ID_ISAUTOINCREMENT,    nAttrib,&m_IsAutoIncrement, ::getBooleanCppuType());
//}
//
void OAdoColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                                                const Any& rValue
                                                 )
                                                                                                 throw (Exception)
{
    if(m_aColumn.IsValid())
    {

        switch(nHandle)
        {
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
                    m_aColumn.put_Type(ADOS::MapJdbc2ADOType(nVal));
                }
                break;
            case PROPERTY_ID_TYPENAME:
                //  rValue <<= getResultSetType();
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
                    m_aColumn.put_NumericScale(nVal);
                }
                break;
            case PROPERTY_ID_ISNULLABLE:
                {
                    sal_Bool _b;
                    rValue >>= _b;
                    if(_b)
                        m_aColumn.put_Attributes(adColNullable);
                }
                break;
            case PROPERTY_ID_ISASCENDING:
                {
                    sal_Bool _b;
                    rValue >>= _b;
                    m_aColumn.put_SortOrder( _b ? adSortAscending : adSortDescending);
                }
                break;
            case PROPERTY_ID_ISROWVERSION:
                break;
            case PROPERTY_ID_ISAUTOINCREMENT:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("AutoIncrement")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getString(rValue));
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_DESCRIPTION:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Default")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getString(rValue));
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_DEFAULTVALUE:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Description")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getString(rValue));
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_ISCURRENCY:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Fixed Length")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        aProp.PutValue(getBOOL(rValue));
                    pProps->Release();
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
void OAdoColumn::getFastPropertyValue(
                                                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    if(m_aColumn.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aColumn.get_Name();
                break;
            case PROPERTY_ID_TYPE:
                rValue <<= ADOS::MapADOType2Jdbc(m_aColumn.get_Type());
                break;
            case PROPERTY_ID_TYPENAME:
                //  rValue <<= getResultSetType();
                break;
            case PROPERTY_ID_PRECISION:
                rValue <<= m_aColumn.get_Precision();
                break;
            case PROPERTY_ID_SCALE:
                rValue <<= m_aColumn.get_NumericScale();
                break;
            case PROPERTY_ID_ISNULLABLE:
                {
                    sal_Bool _b = m_aColumn.get_Attributes() == adColNullable;
                                        rValue <<= Any(&_b, ::getBooleanCppuType());
                }
                break;
            case PROPERTY_ID_ISASCENDING:
                {
                    sal_Bool _b = m_aColumn.get_SortOrder() == adSortAscending;
                                        rValue <<= Any(&_b, ::getBooleanCppuType());
                }
            case PROPERTY_ID_ISAUTOINCREMENT:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("AutoIncrement")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                    {
                        sal_Bool b = aProp.GetValue();
                                                rValue <<= Any(&b, ::getBooleanCppuType());
                    }
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_ISROWVERSION:
                //  rValue <<= getResultSetType();
                break;
            case PROPERTY_ID_DESCRIPTION:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Description")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        rValue <<= (::rtl::OUString)aProp.GetValue();
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_DEFAULTVALUE:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Default")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                        rValue <<= (::rtl::OUString)aProp.GetValue();
                    pProps->Release();
                }
                break;
            case PROPERTY_ID_ISCURRENCY:
                {
                    ADOProperties* pProps = m_aColumn.get_Properties();
                    pProps->AddRef();
                    ADOProperty* pProp = NULL;
                    pProps->get_Item(OLEVariant(::rtl::OUString::createFromAscii("Fixed Length")),&pProp);
                    WpADOProperty aProp(pProp);
                    if(pProp)
                    {
                        sal_Bool bVal = aProp.GetValue();
                                                rValue <<= Any(&bVal,getBooleanCppuType());
                    }
                    pProps->Release();
                }
                break;
        }
    }
}



