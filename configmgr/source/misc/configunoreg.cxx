/*************************************************************************
 *
 *  $RCSfile: configunoreg.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-03 16:33:58 $
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
#include <stdio.h>

#ifndef _CONFIGMGR_PROVIDER_FACTORY_HXX_
#include "providerfactory.hxx"
#endif
#ifndef CONFIGMGR_API_SVCCOMPONENT_HXX_
#include "confsvccomponent.hxx"
#endif
#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::configmgr::ServiceInfo;
using ::configmgr::AsciiServiceName;

typedef Reference< XSingleServiceFactory > (SAL_CALL * createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames
        );

typedef Reference< XSingleServiceFactory > (SAL_CALL * createProviderFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::configmgr::ProviderInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames
        );

// ***************************************************************************************
//
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//

//---------------------------------------------------------------------------------------
void RegisterService(
        const ServiceInfo* pInfo,
        const Reference< XRegistryKey > & xKey)
{
    if (pInfo == 0 || pInfo->serviceNames==0 || pInfo->implementationName==0)
        return;

    OUString aMainKeyName(OUString(RTL_CONSTASCII_USTRINGPARAM("/")));
    aMainKeyName += OUString::createFromAscii(pInfo->implementationName);
    aMainKeyName += OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

    Reference< XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName) );
    OSL_ENSURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    AsciiServiceName const* p = pInfo->serviceNames;
    if (p != 0)
        for( ; *p; ++p)
        {
            xNewKey->createKey(OUString::createFromAscii(*p));
        }
}

//-----------------------------------------------------------------------------
struct ServiceImplementationRequest
{
    Reference< XSingleServiceFactory > xRet;
    Reference< XMultiServiceFactory > const m_xServiceManager;
    OUString const sImplementationName;

    //-------------------------------------------------------------------------
    ServiceImplementationRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    )
    : m_xServiceManager(reinterpret_cast<XMultiServiceFactory*>(pServiceManager))
    , sImplementationName(OUString::createFromAscii(pImplementationName))
    {
    }
    //-------------------------------------------------------------------------
    inline
    sal_Bool shouldCreate(const ServiceInfo* pInfo) const
    {
        OSL_ENSURE(!xRet.is(), "CreateProvider : invalid creation request: we already have a return value !");
        return !xRet.is()   &&
                pInfo != 0  &&
                0 == sImplementationName.compareToAscii(pInfo->implementationName);
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateService(
                const ServiceInfo* pInfo,
                ::cppu::ComponentInstantiation Factory,
                createFactoryFunc creator
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            const Sequence< OUString > Services=  configmgr::ServiceComponentImpl::getServiceNames(pInfo);

            xRet = creator( m_xServiceManager, OUString::createFromAscii(pInfo->implementationName),Factory, Services);
            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateProvider(
                const ServiceInfo* pInfo,
                ::configmgr::ProviderInstantiation Factory,
                createProviderFactoryFunc creator
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            const Sequence< OUString > Services=  configmgr::ServiceComponentImpl::getServiceNames(pInfo);

            xRet = creator( m_xServiceManager, OUString::createFromAscii(pInfo->implementationName),Factory, Services);
            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------
    void* getProvider() const
    {
        // we want to transport the interface pointer as flat C void pointer, so this prevents deletion
        if (xRet.is())
            xRet->acquire();

        return xRet.get();
    }
};

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **ppEnv
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" sal_Bool SAL_CALL component_writeInfo(
                void* pServiceManager,
                void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        Reference< XRegistryKey > xKey(reinterpret_cast<XRegistryKey*>(pRegistryKey));

        RegisterService(configmgr::getConfigurationProviderServices(), xKey);
        RegisterService(configmgr::getAdminProviderServices(), xKey);

        RegisterService(configmgr::getConfigurationRegistryServiceInfo(), xKey);

        // im/export
        RegisterService(configmgr::getDataExportServiceInfo(), xKey);
        RegisterService(configmgr::getDataImportServiceInfo(), xKey);

        return sal_True;
    }
    catch (::com::sun::star::registry::InvalidRegistryException& )
    {
        OSL_ENSURE(sal_False, "SBA::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
    }

    return sal_False;
}

//---------------------------------------------------------------------------------------
extern "C" void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    void* pRet = 0;
    if (pServiceManager)
    {
        ServiceImplementationRequest aReq(pServiceManager,pImplementationName);

        aReq.CreateProvider(
            configmgr::getConfigurationProviderServices(),
            &configmgr::instantiateConfigProvider,
            ::configmgr::createProviderFactory)
        ||
        aReq.CreateProvider(
            configmgr::getAdminProviderServices(),
            &configmgr::instantiateAdminProvider,
            ::configmgr::createProviderFactory)
        ||
        aReq.CreateProvider(
            configmgr::getUserAdminProviderServices(),
            &configmgr::instantiateUserAdminProvider,
            ::configmgr::createProviderFactory)
        ||
        aReq.CreateService(
            configmgr::getConfigurationRegistryServiceInfo(),
            &configmgr::instantiateConfigRegistry,
            ::cppu::createSingleFactory)
        ||
            /* Export */
        aReq.CreateService(
            configmgr::getDataExportServiceInfo(),
            &configmgr::instantiateDataExport,
            ::cppu::createSingleFactory)
        ||
            /* Import */
        aReq.CreateService(
            configmgr::getDataImportServiceInfo(),
            &configmgr::instantiateDataImport,
            ::cppu::createSingleFactory)
        ||
        false;

        pRet = aReq.getProvider();
    }

    return pRet;
};
//---------------------------------------------------------------------------------------

