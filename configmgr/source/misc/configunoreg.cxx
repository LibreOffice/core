/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configunoreg.cxx,v $
 * $Revision: 1.33 $
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
#include "precompiled_configmgr.hxx"
#include <stdio.h>

#include "confapifactory.hxx"
#include "serviceinfohelper.hxx"
#include <cppuhelper/factory.hxx>
#include <rtl/ustrbuf.hxx>

// ***************************************************************************************
//
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//

//---------------------------------------------------------------------------------------
void RegisterService(
        const configmgr::ServiceRegistrationInfo* pInfo,
        const com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > & xKey)
{
    if (pInfo == 0 || pInfo->registeredServiceNames==0 || pInfo->implementationName==0)
        return;

    rtl::OUStringBuffer aMainKeyName;
    aMainKeyName.appendAscii("/");
    aMainKeyName.appendAscii(pInfo->implementationName);
    aMainKeyName.appendAscii("/UNO/SERVICES");

    com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey >  xNewKey( xKey->createKey(aMainKeyName.makeStringAndClear()) );
    OSL_ENSURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    for(sal_Char const * const* p = pInfo->registeredServiceNames ; *p; ++p)
    {
        xNewKey->createKey(rtl::OUString::createFromAscii(*p));
    }
}

//---------------------------------------------------------------------------------------

void RegisterSingleton(
        const configmgr::SingletonRegistrationInfo* pInfo,
        const com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > & xKey)
{
    if (pInfo == 0 ||   pInfo->singletonName            ==0 ||
                        pInfo->implementationName       ==0 ||
                        pInfo->instantiatedServiceName  ==0 )
        return;

    rtl::OUStringBuffer aSingletonKeyName;
    aSingletonKeyName.appendAscii("/");
    aSingletonKeyName.appendAscii(pInfo->implementationName);
    aSingletonKeyName.appendAscii("/UNO/SINGLETONS/");
    aSingletonKeyName.appendAscii(pInfo->singletonName);

    com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey >  xNewKey( xKey->createKey(aSingletonKeyName.makeStringAndClear()) );
    OSL_ENSURE(xNewKey.is(), "CONFMGR::component_writeInfo : could not create a registry key !");

    xNewKey->setStringValue(rtl::OUString::createFromAscii(pInfo->instantiatedServiceName));

    if (pInfo->mappedImplementation != 0)
        RegisterService(pInfo->mappedImplementation,xKey);
}


//-----------------------------------------------------------------------------
struct ServiceImplementationRequest
{
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xRet;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > const m_xServiceManager;
    rtl::OUString const sImplementationName;

    //-------------------------------------------------------------------------
    ServiceImplementationRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    )
    : m_xServiceManager(reinterpret_cast<com::sun::star::lang::XMultiServiceFactory*>(pServiceManager))
    , sImplementationName(rtl::OUString::createFromAscii(pImplementationName))
    {
    }
    //-------------------------------------------------------------------------
    inline
    sal_Bool shouldCreate(const configmgr::ServiceRegistrationInfo* pInfo) const
    {
        OSL_ENSURE(!xRet.is(), "CreateProvider : invalid creation request: we already have a return value !");
        return !xRet.is()   &&
                pInfo != 0  &&
                0 == sImplementationName.compareToAscii(pInfo->implementationName);
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateProviderFactory(
                const configmgr::ServiceRegistrationInfo* pInfo,
                bool bAdmin
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            configmgr::ServiceRegistrationHelper aInfo(pInfo);

            const com::sun::star::uno::Sequence< rtl::OUString > Services=  aInfo.getRegisteredServiceNames();

            xRet = configmgr::createProviderFactory( aInfo.getImplementationName(), bAdmin);

            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(com::sun::star::uno::Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateServiceFactory(
                const configmgr::ServiceRegistrationInfo* pInfo,
                ::cppu::ComponentFactoryFunc Factory
            )
    {
        if (this->shouldCreate(pInfo))
        try
        {
            configmgr::ServiceRegistrationHelper aInfo(pInfo);

            const com::sun::star::uno::Sequence< rtl::OUString > Services=  aInfo.getRegisteredServiceNames();

            xRet = cppu::createSingleComponentFactory( Factory, aInfo.getImplementationName(), Services, 0);

            OSL_ENSURE(xRet.is(), "CreateProvider : WHERE IS THE return value !");
        }
        catch(com::sun::star::uno::Exception&)
        {
        }
        return xRet.is();
    }

    //-------------------------------------------------------------------------

    sal_Bool CreateSingletonMapperFactory(
                const configmgr::SingletonRegistrationInfo* pInfo,
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

extern "C" SAL_DLLPUBLIC_EXPORT
void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /* ppEnv */
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
                            void* /* pServiceManager */,
                            void* pRegistryKey
            )
{
    if (pRegistryKey)
    try
    {
        com::sun::star::uno::Reference< com::sun::star::registry::XRegistryKey > xKey(reinterpret_cast<com::sun::star::registry::XRegistryKey*>(pRegistryKey));

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

        // backends
        RegisterService(configmgr::backend::getDefaultBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::backend::getSingleBackendAdapterServiceInfo(), xKey) ;
        RegisterService(configmgr::backend::getMultiStratumBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalBackendServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalDataImportServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalHierarchyBrowserServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalSchemaSupplierServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalLegacyStratumServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalDataStratumServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalReadonlyStratumServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalResourceStratumServiceInfo(), xKey) ;
        RegisterService(configmgr::localbe::getLocalMultiStratumServiceInfo(), xKey) ;

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
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
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
        // backends
        aReq.CreateServiceFactory(
                configmgr::backend::getDefaultBackendServiceInfo(),
                configmgr::backend::instantiateDefaultBackend)
        ||
        aReq.CreateServiceFactory(
                configmgr::backend::getSingleBackendAdapterServiceInfo(),
                configmgr::backend::instantiateSingleBackendAdapter)
        ||
        aReq.CreateServiceFactory(
                configmgr::backend::getMultiStratumBackendServiceInfo(),
                configmgr::backend::instantiateMultiStratumBackend)
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
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalSchemaSupplierServiceInfo(),
                configmgr::localbe::instantiateLocalSchemaSupplier)
        ||
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalLegacyStratumServiceInfo(),
                configmgr::localbe::instantiateLocalLegacyStratum)
        ||
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalDataStratumServiceInfo(),
                configmgr::localbe::instantiateLocalDataStratum)
        ||
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalReadonlyStratumServiceInfo(),
                configmgr::localbe::instantiateLocalReadonlyStratum)
        ||
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalResourceStratumServiceInfo(),
                configmgr::localbe::instantiateLocalResourceStratum)
        ||
         aReq.CreateServiceFactory(
                configmgr::localbe::getLocalMultiStratumServiceInfo(),
                configmgr::localbe::instantiateLocalMultiStratum)
        ||
        // im/export
        aReq.CreateServiceFactory(
            configmgr::backend::getMergeImportServiceInfo(),
            &configmgr::backend::instantiateMergeImporter)
        ||
        aReq.CreateServiceFactory(
            configmgr::backend::getCopyImportServiceInfo(),
            &configmgr::backend::instantiateCopyImporter);

        pRet = aReq.getService();
    }

    return pRet;
}
//---------------------------------------------------------------------------------------

