/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_connectivity.hxx"

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

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /*ppEnv*/
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

typedef void* (SAL_CALL * OMozillaBootstrap_CreateInstanceFunction)(const Reference< XMultiServiceFactory >& _rxFactory );
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL createMozillaBootstrap(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
        const ::rtl::OUString sModuleName(RTL_CONSTASCII_USTRINGPARAM(SVLIBRARY( "mozabdrv" )));

        // load the dbtools library
        oslModule s_hModule = osl_loadModuleRelative(
            reinterpret_cast< oslGenericFunction >(&createMozillaBootstrap),
            sModuleName.pData, 0);
        OSL_ENSURE(NULL != s_hModule, "MozabDriver::registerClient: could not load the dbtools library!");
        if (NULL != s_hModule)
        {

            // get the symbol for the method creating the factory
            const ::rtl::OUString sFactoryCreationFunc = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("OMozillaBootstrap_CreateInstance"));
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
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
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
        else if (aImplName.equals(  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.mozilla.MozillaBootstrap"))  ))
        {
            Sequence< ::rtl::OUString > aSNS( 1 );
            aSNS[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap"));
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
