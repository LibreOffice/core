/*************************************************************************
 *
 *  $RCSfile: AKey.cxx,v $
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

#ifndef _CONNECTIVITY_ADO_KEY_HXX_
#include "ado/AKey.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;

// -------------------------------------------------------------------------
void WpADOKey::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    _ADOKey* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOKEY_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOKEY_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoKey::OAdoKey(sal_Bool _bCase, ADOKey* _pKey) : OKey_ADO(_bCase)
{
    construct();
    if(_pKey)
        m_aKey = WpADOKey(_pKey);
    else
        m_aKey.Create();

    refreshColumns();
}
// -------------------------------------------------------------------------
OAdoKey::OAdoKey(   const ::rtl::OUString& _Name,
            const ::rtl::OUString& _ReferencedTable,
            sal_Int32       _Type,
            sal_Int32       _UpdateRule,
            sal_Int32       _DeleteRule,
            sal_Bool _bCase
          ) : OKey_ADO(_Name,
                          _ReferencedTable,
                          _Type,
                          _UpdateRule,
                          _DeleteRule,_bCase)
{
    construct();
    m_aKey.Create();
    m_aKey.put_Name(_Name);
    m_aKey.put_UpdateRule(Map2Rule(_UpdateRule));
    m_aKey.put_DeleteRule(Map2Rule(_DeleteRule));
    m_aKey.put_RelatedTable(_ReferencedTable);
    m_aKey.put_Type((KeyTypeEnum)_Type);

    refreshColumns();
}
// -------------------------------------------------------------------------
void OAdoKey::refreshColumns()
{
    ::std::vector< ::rtl::OUString> aVector;

    ADOColumns* pColumns = m_aKey.get_Columns();
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
Any SAL_CALL OAdoKey::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;
    return OKey_ADO::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OAdoKey::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OKey_ADO::getTypes());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoKey::getUnoTunnelImplementationId()
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
sal_Int64 OAdoKey::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
// -------------------------------------------------------------------------
void OAdoKey::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aKey.IsValid())
    {

        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_Name(aVal);
                }
                break;
            case PROPERTY_ID_TYPE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_Type((KeyTypeEnum)nVal);
                }
                break;
            case PROPERTY_ID_REFERENCEDTABLE:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aKey.put_RelatedTable(aVal);
                }
                break;
            case PROPERTY_ID_UPDATERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_UpdateRule(Map2Rule(nVal));
                }
                break;
            case PROPERTY_ID_DELETERULE:
                {
                    sal_Int32 nVal=0;
                    rValue >>= nVal;
                    m_aKey.put_DeleteRule(Map2Rule(nVal));
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
void OAdoKey::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aKey.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aKey.get_Name();
                break;
            case PROPERTY_ID_TYPE:
                rValue <<= (sal_Int32)m_aKey.get_Type();
                break;
            case PROPERTY_ID_REFERENCEDTABLE:
                rValue <<= m_aKey.get_RelatedTable();
                break;
            case PROPERTY_ID_UPDATERULE:
                rValue <<= MapRule(m_aKey.get_UpdateRule());
                break;
            case PROPERTY_ID_DELETERULE:
                rValue <<= MapRule(m_aKey.get_DeleteRule());
                break;
        }
    }
}
// -------------------------------------------------------------------------
sal_Int32 OAdoKey::MapRule(const RuleEnum& _eNum) const
{
        sal_Int32 eNum = KeyRule::NO_ACTION;
    switch(_eNum)
    {
        case adRICascade:
                        eNum = KeyRule::CASCADE;
            break;
        case adRISetNull:
                        eNum = KeyRule::SET_NULL;
            break;
        case adRINone:
                        eNum = KeyRule::NO_ACTION;
            break;
        case adRISetDefault:
                        eNum = KeyRule::SET_DEFAULT;
            break;
    }
    return eNum;
}
// -------------------------------------------------------------------------
RuleEnum OAdoKey::Map2Rule(const sal_Int32& _eNum) const
{
    RuleEnum eNum = adRINone;
    switch(_eNum)
    {
                case KeyRule::CASCADE:
            eNum = adRICascade;
            break;
                case KeyRule::SET_NULL:
            eNum = adRISetNull;
            break;
                case KeyRule::NO_ACTION:
            eNum = adRINone;
            break;
                case KeyRule::SET_DEFAULT:
            eNum = adRISetDefault;
            break;
    }
    return eNum;
}


