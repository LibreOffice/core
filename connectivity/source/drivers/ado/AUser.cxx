/*************************************************************************
 *
 *  $RCSfile: AUser.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:21 $
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

#ifndef _CONNECTIVITY_ADO_USER_HXX_
#include "ado/AUser.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_GROUPS_HXX_
#include "ado/AGroups.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_BCONNECTION_HXX_
#include "ado/AConnection.hxx"
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
void WpADOUser::Create()
{
    IClassFactory2* pIUnknown   = NULL;
    IUnknown        *pOuter     = NULL;
    HRESULT         hr = -1;
    _ADOUser* pCommand;
    hr = CoCreateInstance(ADOS::CLSID_ADOUSER_25,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          ADOS::IID_ADOUSER_25,
                          (void**)&pCommand );


    if( !FAILED( hr ) )
    {
        pInterface = pCommand;
        pInterface->AddRef();
    }
}
// -------------------------------------------------------------------------
OAdoUser::OAdoUser(sal_Bool _bCase, ADOUser* _pUser) : OUser_TYPEDEF(_bCase)
{
    construct();

    if(_pUser)
        m_aUser = WpADOUser(_pUser);
    else
        m_aUser.Create();
    refreshGroups();
}
// -------------------------------------------------------------------------
OAdoUser::OAdoUser(sal_Bool _bCase,   const ::rtl::OUString& _Name) : OUser_TYPEDEF(_Name,_bCase)
{
    construct();
    m_aUser.Create();
    m_aUser.put_Name(_Name);
    refreshGroups();
}
// -------------------------------------------------------------------------
void OAdoUser::refreshGroups()
{

    ::std::vector< ::rtl::OUString> aVector;

    ADOGroups* pGroups = m_aUser.get_Groups();
    if(pGroups)
    {
        pGroups->Refresh();

        sal_Int32 nCount = 0;
        pGroups->get_Count(&nCount);
        for(sal_Int32 i=0;i< nCount;++i)
        {
            ADOGroup* pGroup = NULL;
            pGroups->get_Item(OLEVariant(i),&pGroup);
            if(pGroup)
            {
                WpADOGroup aGroup(pGroup);
                aVector.push_back(aGroup.get_Name());
            }
        }
    }

    if(m_pGroups)
        delete m_pGroups;
    m_pGroups = new OGroups(*this,m_aMutex,aVector,pGroups,isCaseSensitive());
}
// -------------------------------------------------------------------------
Any SAL_CALL OAdoUser::queryInterface( const Type & rType ) throw(RuntimeException)
{
        Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;
    return OUser_TYPEDEF::queryInterface(rType);
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OAdoUser::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel > *)0 ));

    return ::utl::concatSequences(aTypes.getTypes(),OUser_TYPEDEF::getTypes());
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OAdoUser::getUnoTunnelImplementationId()
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
sal_Int64 OAdoUser::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}

// -------------------------------------------------------------------------
void OAdoUser::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue)throw (Exception)
{
    if(m_aUser.IsValid())
    {

        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                {
                    ::rtl::OUString aVal;
                    rValue >>= aVal;
                    m_aUser.put_Name(aVal);
                }
                break;
        }
    }
}
// -------------------------------------------------------------------------
void OAdoUser::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    if(m_aUser.IsValid())
    {
        switch(nHandle)
        {
            case PROPERTY_ID_NAME:
                rValue <<= m_aUser.get_Name();
                break;
        }
    }
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(sal_Bool _bCase,   ADOUser* _pUser) : OAdoUser(_bCase,_pUser)
{
}
// -------------------------------------------------------------------------
OUserExtend::OUserExtend(sal_Bool _bCase, const ::rtl::OUString& _Name) : OAdoUser(_bCase,_Name)
{
}
// -------------------------------------------------------------------------
void OUserExtend::construct()
{
    OUser_TYPEDEF::construct();
    registerProperty(PROPERTY_PASSWORD, PROPERTY_ID_PASSWORD,0,&m_Password,::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}


