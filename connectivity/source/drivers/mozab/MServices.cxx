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


#include "MDriver.hxx"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include "bootstrap/MMozillaBootstrap.hxx"
#include <tools/solar.h>

using namespace connectivity::mozab;
using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::mozilla::XMozillaBootstrap;

typedef Reference< XSingleServiceFactory > (SAL_CALL *createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames,
            rtl_ModuleCount* _pTemp
        );

//---------------------------------------------------------------------------------------
struct ProviderRequest
{
    Reference< XSingleServiceFactory > xRet;
    Reference< XMultiServiceFactory > const xServiceManager;
    OUString const sImplementationName;

    ProviderRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    )
    : xServiceManager(reinterpret_cast<XMultiServiceFactory*>(pServiceManager))
    , sImplementationName(OUString::createFromAscii(pImplementationName))
    {
    }

    inline
    sal_Bool CREATE_PROVIDER(
                const OUString& Implname,
                const Sequence< OUString > & Services,
                ::cppu::ComponentInstantiation Factory,
                createFactoryFunc creator
            )
    {
        if (!xRet.is() && (Implname == sImplementationName))
        try
        {
            xRet = creator( xServiceManager, sImplementationName,Factory, Services,0);
        }
        catch(...)
        {
        }
        return xRet.is();
    }

    void* getProvider() const { return xRet.get(); }
};

//---------------------------------------------------------------------------------------
typedef void* (SAL_CALL * OMozillaBootstrap_CreateInstanceFunction)(const Reference< XMultiServiceFactory >& _rxFactory );
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL createMozillaBootstrap(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
        const ::rtl::OUString sModuleName(SVLIBRARY( "mozabdrv" ));

        // load the dbtools library
        oslModule s_hModule = osl_loadModuleRelative(
            reinterpret_cast< oslGenericFunction >(&createMozillaBootstrap),
            sModuleName.pData, 0);
        OSL_ENSURE(NULL != s_hModule, "MozabDriver::registerClient: could not load the dbtools library!");
        if (NULL != s_hModule)
        {

            // get the symbol for the method creating the factory
            const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString( "OMozillaBootstrap_CreateInstance");
            // reinterpret_cast<OMozabConnection_CreateInstanceFunction> removed GNU C
            OMozillaBootstrap_CreateInstanceFunction s_pCreationFunc = (OMozillaBootstrap_CreateInstanceFunction)osl_getFunctionSymbol(s_hModule, sFactoryCreationFunc.pData);

            if (NULL == s_pCreationFunc)
            {   // did not find the symbol
                OSL_FAIL("MozabDriver::registerClient: could not find the symbol for creating the factory!");
                osl_unloadModule(s_hModule);
                s_hModule = NULL;
            }
            MozillaBootstrap * pBootstrap = reinterpret_cast<MozillaBootstrap*>((*s_pCreationFunc)(_rxFactory));
            return *pBootstrap;
        }
        return NULL;
}
//---------------------------------------------------------------------------------------
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL mozab_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    void* pRet = 0;
    if (pServiceManager)
    {
        OUString aImplName( OUString::createFromAscii( pImplementationName ) );
        ProviderRequest aReq(pServiceManager,pImplementationName);
        if (aImplName.equals(  MozabDriver::getImplementationName_Static()  ))
        {
            aReq.CREATE_PROVIDER(
                MozabDriver::getImplementationName_Static(),
                MozabDriver::getSupportedServiceNames_Static(),
                MozabDriver_CreateInstance, ::cppu::createSingleFactory);
        }
        else if ( aImplName == "com.sun.star.comp.mozilla.MozillaBootstrap" )
        {
            Sequence< ::rtl::OUString > aSNS( 1 );
            aSNS[0] = ::rtl::OUString( "com.sun.star.mozilla.MozillaBootstrap");
            aReq.CREATE_PROVIDER(
                aImplName,
                aSNS,
                createMozillaBootstrap, ::cppu::createSingleFactory);
        }
        if(aReq.xRet.is())
            aReq.xRet->acquire();
        pRet = aReq.getProvider();
    }

    return pRet;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
