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

#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <swdbtoolsclient.hxx>
#include <osl/diagnose.h>
#include <tools/solar.h>

using namespace ::connectivity::simple;
using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;

//====================================================================
//= SwDbtoolsClient
//====================================================================
namespace
{
    // -----------------------------------------------------------------------------
    // this namespace contains access to all static members of the class SwDbtoolsClient
    // to make the initialize of the dll a little bit faster
    // -----------------------------------------------------------------------------
    ::osl::Mutex& getDbtoolsClientMutex()
    {
        static  ::osl::Mutex aMutex;
        return aMutex;
    }
    // -----------------------------------------------------------------------------
    sal_Int32& getDbToolsClientClients()
    {
        static  sal_Int32 nClients = 0;
        return nClients;
    }
    // -----------------------------------------------------------------------------
    oslModule& getDbToolsClientModule()
    {
        static oslModule hDbtoolsModule = NULL;
        return hDbtoolsModule;
    }
    // -----------------------------------------------------------------------------
    createDataAccessToolsFactoryFunction& getDbToolsClientFactoryFunction()
    {
        static createDataAccessToolsFactoryFunction pFactoryCreationFunc = NULL;
        return pFactoryCreationFunc;
    }
    // -----------------------------------------------------------------------------
}

SwDbtoolsClient::SwDbtoolsClient()
{
}

SwDbtoolsClient::~SwDbtoolsClient()
{
    if(m_xDataAccessFactory.is())
    {
        // clear the factory _before_ revoking the client
        // (the revocation may unload the DBT lib)
        m_xDataAccessFactory = NULL;
        // revoke the client
        revokeClient();
    }
}

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" void * createDataAccessToolsFactory();

#endif

void SwDbtoolsClient::registerClient()
{
    ::osl::MutexGuard aGuard(getDbtoolsClientMutex());
    if (1 == ++getDbToolsClientClients())
    {
        OSL_ENSURE(NULL == getDbToolsClientModule(), "SwDbtoolsClient::registerClient: inconsistence: already have a module!");
        OSL_ENSURE(NULL == getDbToolsClientFactoryFunction(), "SwDbtoolsClient::registerClient: inconsistence: already have a factory function!");

#ifndef DISABLE_DYNLOADING
        const OUString sModuleName(RTL_CONSTASCII_USTRINGPARAM(SVLIBRARY("dbtools")));

        // load the dbtools library
        getDbToolsClientModule() = osl_loadModuleRelative(
            &thisModule, sModuleName.pData, 0);
        OSL_ENSURE(NULL != getDbToolsClientModule(), "SwDbtoolsClient::registerClient: could not load the dbtools library!");
        if (NULL != getDbToolsClientModule())
        {
            // get the symbol for the method creating the factory
            const OUString sFactoryCreationFunc("createDataAccessToolsFactory");
            //  reinterpret_cast<createDataAccessToolsFactoryFunction> removed for gcc permissive
            getDbToolsClientFactoryFunction() = reinterpret_cast< createDataAccessToolsFactoryFunction >(
                osl_getFunctionSymbol(getDbToolsClientModule(), sFactoryCreationFunc.pData));

            if (NULL == getDbToolsClientFactoryFunction())
            {   // did not find the symbol
                OSL_FAIL("SwDbtoolsClient::registerClient: could not find the symbol for creating the factory!");
                osl_unloadModule(getDbToolsClientModule());
                getDbToolsClientModule() = NULL;
            }
        }
#else
        getDbToolsClientFactoryFunction() = createDataAccessToolsFactory;
#endif
    }
}

void SwDbtoolsClient::revokeClient()
{
    ::osl::MutexGuard aGuard(getDbtoolsClientMutex());
    if (0 == --getDbToolsClientClients())
    {
#ifndef DISABLE_DYNLOADING
        getDbToolsClientFactoryFunction() = NULL;
        if (getDbToolsClientModule())
            osl_unloadModule(getDbToolsClientModule());
#endif
        getDbToolsClientModule() = NULL;
    }
}

void SwDbtoolsClient::getFactory()
{
    if(!m_xDataAccessFactory.is())
    {
        registerClient();
        if(getDbToolsClientFactoryFunction())
        {   // loading the lib succeeded
            void* pUntypedFactory = (*getDbToolsClientFactoryFunction())();
            IDataAccessToolsFactory* pDBTFactory = static_cast<IDataAccessToolsFactory*>(pUntypedFactory);
            OSL_ENSURE(pDBTFactory, "SwDbtoolsClient::SwDbtoolsClient: no factory returned!");
            if (pDBTFactory)
            {
                m_xDataAccessFactory = pDBTFactory;
                // by definition, the factory was aquired once
                m_xDataAccessFactory->release();
            }
        }
    }
}

::rtl::Reference< ::connectivity::simple::IDataAccessTools >
    SwDbtoolsClient::getDataAccessTools()
{
    if(!m_xDataAccessTools.is())
    {
        getFactory();
        if(m_xDataAccessFactory.is())
            m_xDataAccessTools = m_xDataAccessFactory->getDataAccessTools();
    }
    return m_xDataAccessTools;
}

::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >
    SwDbtoolsClient::getAccessTypeConversion()
{
    if(!m_xAccessTypeConversion.is())
    {
        getFactory();
        if(m_xDataAccessFactory.is())
            m_xAccessTypeConversion = m_xDataAccessFactory->getTypeConversionHelper();
    }
    return m_xAccessTypeConversion;
}

Reference< XDataSource > SwDbtoolsClient::getDataSource(
        const OUString& rRegisteredName,
        const Reference<XComponentContext>& rxContext
            )
{
    Reference< XDataSource > xRet;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    xAccess = getDataAccessTools();
    if(xAccess.is())
        xRet = xAccess->getDataSource(rRegisteredName, rxContext);
    return xRet;
}

sal_Int32 SwDbtoolsClient::getDefaultNumberFormat(
        const Reference< XPropertySet >& rxColumn,
        const Reference< XNumberFormatTypes >& rxTypes,
        const lang::Locale& rLocale
            )
{
    sal_Int32 nRet = -1;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    xAccess = getDataAccessTools();
    if(xAccess.is())
        nRet = xAccess->getDefaultNumberFormat( rxColumn, rxTypes, rLocale);
    return nRet;
}

OUString SwDbtoolsClient::getFormattedValue(
        const uno::Reference< beans::XPropertySet>& _rxColumn,
        const uno::Reference< util::XNumberFormatter>& _rxFormatter,
        const lang::Locale& _rLocale,
        const util::Date& _rNullDate
            )

{
    ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion > xConversion =
                    getAccessTypeConversion();
    OUString sRet;
    if(xConversion.is())
        sRet = xConversion->getFormattedValue(_rxColumn, _rxFormatter, _rLocale, _rNullDate);
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
