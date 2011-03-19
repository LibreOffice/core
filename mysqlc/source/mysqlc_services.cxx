/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2008 by Sun Microsystems, Inc.
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
************************************************************************/

#include "mysqlc_driver.hxx"

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;
using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::registry::XRegistryKey;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;

typedef Reference< XSingleServiceFactory > (SAL_CALL *createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames,
            rtl_ModuleCount* _pTemp
        );

//***************************************************************************************
//
// Die vorgeschriebene C-API muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
//

//---------------------------------------------------------------------------------------
void REGISTER_PROVIDER(
        const OUString& aServiceImplName,
        const Sequence< OUString>& Services,
        const Reference< XRegistryKey > & xKey)
{
    ::rtl::OUStringBuffer aMainKeyName;
    aMainKeyName.append( sal_Unicode( '/' ) );
    aMainKeyName.append( aServiceImplName );
    aMainKeyName.appendAscii( "/UNO/SERVICES" );

    Reference< XRegistryKey > xNewKey( xKey->createKey( aMainKeyName.makeStringAndClear() ) );
    OSL_ENSURE(xNewKey.is(), "SKELETON::component_writeInfo : could not create a registry key !");

    for (sal_Int32 i = 0; i < Services.getLength(); ++i) {
        xNewKey->createKey(Services[i]);
    }
}


//---------------------------------------------------------------------------------------
struct ProviderRequest
{
    Reference< XSingleServiceFactory > xRet;
    Reference< XMultiServiceFactory > const xServiceManager;
    OUString const sImplementationName;

    ProviderRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    ) : xServiceManager(reinterpret_cast<XMultiServiceFactory*>(pServiceManager))
      , sImplementationName(OUString::createFromAscii(pImplementationName))
    {
    }

    /* {{{ CREATE_PROVIDER -I- */
    inline sal_Bool CREATE_PROVIDER(
                const OUString& Implname,
                const Sequence< OUString > & Services,
                ::cppu::ComponentInstantiation Factory,
                createFactoryFunc creator
            )
    {
        if (!xRet.is() && (Implname == sImplementationName)) {
            try {
                xRet = creator( xServiceManager, sImplementationName,Factory, Services,0);
            } catch (...) {
            }
        }
        return xRet.is();
    }

    void* getProvider() const { return xRet.get(); }
};
/* }}} */


/* {{{ component_getImplementationEnvironment -I- */
extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /* ppEnv */
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
/* }}} */


/* {{{ component_writeInfo -I- */
extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void * /* pServiceManager */, void * pRegistryKey)
{
    if (pRegistryKey) {
        try {
            Reference< XRegistryKey > xKey(reinterpret_cast< XRegistryKey*>(pRegistryKey));

            REGISTER_PROVIDER(
                MysqlCDriver::getImplementationName_Static(),
                MysqlCDriver::getSupportedServiceNames_Static(), xKey);

            return sal_True;
        } catch (::com::sun::star::registry::InvalidRegistryException& ) {
            OSL_FAIL("SKELETON::component_writeInfo : could not create a registry key ! ## InvalidRegistryException !");
        }
    }
    return sal_False;
}
/* }}} */


/* {{{ component_getFactory -I- */
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
                    const sal_Char * pImplementationName,
                    void * pServiceManager,
                    void * /* pRegistryKey */)
{
    void* pRet = 0;
    if (pServiceManager) {
        ProviderRequest aReq(pServiceManager,pImplementationName);

        aReq.CREATE_PROVIDER(
            MysqlCDriver::getImplementationName_Static(),
            MysqlCDriver::getSupportedServiceNames_Static(),
            MysqlCDriver_CreateInstance, ::cppu::createSingleFactory)
        ;

        if(aReq.xRet.is()) {
            aReq.xRet->acquire();
        }

        pRet = aReq.getProvider();
    }

    return pRet;
};
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
