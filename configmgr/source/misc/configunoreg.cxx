/*************************************************************************
 *
 *  $RCSfile: configunoreg.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:22 $
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

#include "confapifactory.hxx"

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::lang::XSingleComponentFactory;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::configmgr::ServiceRegistrationInfo;
using ::configmgr::SingletonRegistrationInfo;
using ::configmgr::AsciiServiceName;

// ***************************************************************************************
//
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//

//---------------------------------------------------------------------------------------
void RegisterService(
        const ServiceRegistrationInfo* pInfo,
        const Reference< XRegistryKey > & xKey)
{
    if (pInfo == 0 || pInfo->registeredServiceNames==0 || pInfo->implementationName==0)
        return;

    OUStringBuffer aMainKeyName;
    aMainKeyName.appendAscii("/");
    aMainKeyName.appendAscii(pInfo->implementationName);
    aMainKeyName.appendAscii("/UNO/SERVICES");

    Reference< XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName.makeStringAndClear()) );
    OSL_ENSURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    for(AsciiServiceName const* p = pInfo->registeredServiceNames ; *p; ++p)
    {
        xNewKey->createKey(OUString::createFromAscii(*p));
    }
}

//---------------------------------------------------------------------------------------

void RegisterSingleton(
        const SingletonRegistrationInfo* pInfo,
        const Reference< XRegistryKey > & xKey)
{
    if (pInfo == 0 ||   pInfo->singletonName            ==0 ||
                        pInfo->implementationName       ==0 ||
                        pInfo->instantiatedServiceName  ==0 )
        return;

    OUStringBuffer aSingletonKeyName;
    aSingletonKeyName.appendAscii("/");
    aSingletonKeyName.appendAscii(pInfo->implementationName);
    aSingletonKeyName.appendAscii("/UNO/SINGLETONS/");
    aSingletonKeyName.appendAscii(pInfo->singletonName);

    Reference< XRegistryKey >  xNewKey( xKey->createKey(aSingletonKeyName.makeStringAndClear()) );
    OSL_ENSURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    xNewKey->setStringValue(OUString::createFromAscii(pInfo->instantiatedServiceName));

    if (pInfo->mappedImplementation != 0)
        RegisterService(pInfo->mappedImplementation,xKey);
}


//-----------------------------------------------------------------------------
struct ServiceImplementationRequest
{
    Reference< XInterface > xRet;
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
    sal_Bool shouldCreate(const ServiceRegistrationInfo* pInfo) const
    {
        OSL_ENSURE(!xRet.is(), "CreateProvider : invalid creation request: we already have a return value !");
        return !xRet.is()   &&
                pInfo != 0  &&
                0 == sImplementationName.compareToAscii(pInfo->implementationName);
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateProviderFactory(
                const ServiceRegistrationInfo* pInfo,
                bool bAdmin
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            configmgr::ServiceRegistrationHelper aInfo(pInfo);

            const Sequence< OUString > Services=  aInfo.getRegisteredServiceNames();

            xRet = configmgr::createProviderFactory( aInfo.getImplementationName(), bAdmin);

            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateServiceFactory(
                const ServiceRegistrationInfo* pInfo,
                ::cppu::ComponentFactoryFunc Factory
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            configmgr::ServiceRegistrationHelper aInfo(pInfo);

            const Sequence< OUString > Services=  aInfo.getRegisteredServiceNames();

            xRet = cppu::createSingleComponentFactory( Factory, aInfo.getImplementationName(), Services, 0);

            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateSingletonMapperFactory(
                const SingletonRegistrationInfo* pInfo,
                ::cppu::ComponentFactoryFunc Mapper
            )
    {
        OSL_ENSURE(pInfo && pInfo->mappedImplementation, "CreateProvider : Cannot map unmapped singleton !");

        return pInfo && pInfo->mappedImplementation &&
                CreateServiceFactory(pInfo->mappedImplementation,Mapper);
    }

    //-------------------------------------------------------------------------
    void* getService() const
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

        // configuration access entry points: configuration provider
        RegisterSingleton(configmgr::getDefaultProviderSingletonInfo(), xKey) ;

        RegisterService(configmgr::getConfigurationProviderServiceInfo(), xKey);
        RegisterService(configmgr::getDefaultProviderServiceInfo(), xKey);
        RegisterService(configmgr::getAdminProviderServiceInfo(), xKey);

        // registry wrapper (deprecated)
        RegisterService(configmgr::getConfigurationRegistryServiceInfo(), xKey);

        // updating
        RegisterService(configmgr::backend::getUpdateMergerServiceInfo(), xKey);

        // xml handling
        RegisterService(configmgr::xml::getSchemaParserServiceInfo(), xKey);
        RegisterService(configmgr::xml::getLayerParserServiceInfo(), xKey);
        RegisterService(configmgr::xml::getLayerWriterServiceInfo(), xKey);

        // bootstrap handling
        RegisterSingleton(configmgr::getBootstrapContextSingletonInfo(), xKey) ;
        RegisterService(configmgr::getBootstrapContextServiceInfo(), xKey) ;

        // backend singletons
        RegisterSingleton(configmgr::backend::getDefaultBackendSingletonInfo(), xKey) ;
        RegisterSingleton(configmgr::backend::getDefaultSingleBackendSingletonInfo(), xKey) ;

        // backends
        RegisterService(configmgr::backend::getDefaultBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::backend::getDefaultSingleBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::backend::getSingleBackendAdapterServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalDataImportServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalHierarchyBrowserServiceInfo(), xKey) ;

        // im/export
        RegisterService(configmgr::backend::getMergeImportServiceInfo(), xKey);
        RegisterService(configmgr::backend::getCopyImportServiceInfo(), xKey);

        return sal_True;
    }
    catch (::com::sun::star::registry::InvalidRegistryException& )
    {
        OSL_ENSURE(sal_False, "configmgr: component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
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

        // configuration access entry points: configuration provider
        aReq.CreateProviderFactory(
            configmgr::getConfigurationProviderServiceInfo(),
            false)
        ||
        aReq.CreateProviderFactory(
            configmgr::getAdminProviderServiceInfo(),
            true)
        ||
        aReq.CreateServiceFactory(
            configmgr::getDefaultProviderServiceInfo(),
            &configmgr::instantiateDefaultProvider)
        ||
        // registry wrapper (deprecated)
        aReq.CreateServiceFactory(
            configmgr::getConfigurationRegistryServiceInfo(),
            &configmgr::instantiateConfigRegistry)
        ||
        // updating
        aReq.CreateServiceFactory(
            configmgr::backend::getUpdateMergerServiceInfo(),
            &configmgr::backend::instantiateUpdateMerger)
        ||
        // xml handling
        aReq.CreateServiceFactory(
            configmgr::xml::getSchemaParserServiceInfo(),
            &configmgr::xml::instantiateSchemaParser)
        ||
        aReq.CreateServiceFactory(
            configmgr::xml::getLayerParserServiceInfo(),
            &configmgr::xml::instantiateLayerParser)
        ||
        aReq.CreateServiceFactory(
            configmgr::xml::getLayerWriterServiceInfo(),
            &configmgr::xml::instantiateLayerWriter)
        ||
        // bootstrap handling
        aReq.CreateServiceFactory(
            configmgr::getBootstrapContextServiceInfo(),
            &configmgr::instantiateBootstrapContext)
        ||
        // backend singletons
        aReq.CreateSingletonMapperFactory(
                configmgr::backend::getDefaultBackendSingletonInfo(),
                configmgr::backend::getDefaultBackendSingleton)
        ||
        aReq.CreateSingletonMapperFactory(
                configmgr::backend::getDefaultSingleBackendSingletonInfo(),
                configmgr::backend::getDefaultSingleBackendSingleton)
        ||
        // backends
        aReq.CreateServiceFactory(
                configmgr::backend::getDefaultBackendServiceInfo(),
                configmgr::backend::instantiateDefaultBackend)
        ||
        aReq.CreateServiceFactory(
                configmgr::backend::getDefaultSingleBackendServiceInfo(),
                configmgr::backend::instantiateDefaultSingleBackend)
        ||
        aReq.CreateServiceFactory(
                configmgr::backend::getSingleBackendAdapterServiceInfo(),
                configmgr::backend::instantiateSingleBackendAdapter)
        ||
        aReq.CreateServiceFactory(
                configmgr::localbe::getLocalBackendServiceInfo(),
                configmgr::localbe::instantiateLocalBackend)
        ||
        aReq.CreateServiceFactory(
                configmgr::localbe::getLocalDataImportServiceInfo(),
                configmgr::localbe::instantiateLocalDataImporter)
        ||
        aReq.CreateServiceFactory(
                configmgr::localbe::getLocalHierarchyBrowserServiceInfo(),
                configmgr::localbe::instantiateLocalHierarchyBrowser)
        ||
        // im/export
        aReq.CreateServiceFactory(
            configmgr::backend::getMergeImportServiceInfo(),
            &configmgr::backend::instantiateMergeImporter)
        ||
        aReq.CreateServiceFactory(
            configmgr::backend::getCopyImportServiceInfo(),
            &configmgr::backend::instantiateCopyImporter)
        ||
        false;

        pRet = aReq.getService();
    }

    return pRet;
}
//---------------------------------------------------------------------------------------

