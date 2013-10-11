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

#include "mysqlc_driver.hxx"

#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>

using namespace connectivity::mysqlc;
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
            rtl_ModuleCount*
        );

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
