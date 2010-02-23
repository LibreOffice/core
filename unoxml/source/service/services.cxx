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

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>

#include "../dom/documentbuilder.hxx"
#include "../dom/saxbuilder.hxx"
#include "../xpath/xpathapi.hxx"
#include "../events/testlistener.hxx"

using namespace ::DOM;
using namespace ::DOM::events;
using namespace ::XPath;
using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

void SAL_CALL
component_getImplementationEnvironment(const sal_Char **ppEnvironmentTypeName, uno_Environment ** /*ppEnvironment */)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL
component_writeInfo(void * /*pServiceManager*/, void* pRegistryKey )
{
    Reference< XRegistryKey > xKey(reinterpret_cast< XRegistryKey* >(pRegistryKey));
    Reference< XRegistryKey > xNewKey;
    OUString aImpl;

    // register DOM service
    aImpl =  OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CDocumentBuilder::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CDocumentBuilder::_getSupportedServiceNames()[0]);

    // register DOM service
    aImpl =  OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CSAXDocumentBuilder::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CSAXDocumentBuilder::_getSupportedServiceNames()[0]);

    // register XPath service
    aImpl = OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CXPathAPI::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CXPathAPI::_getSupportedServiceNames()[0]);

    // register EventTest service
    aImpl = OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += CTestListener::_getImplementationName();
    aImpl += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey(aImpl);
    xNewKey->createKey(CTestListener::_getSupportedServiceNames()[0]);

    return sal_True;
}

void* SAL_CALL
component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void * /*pRegistryKey*/)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (CDocumentBuilder::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createOneInstanceFactory(
                    xServiceManager, CDocumentBuilder::_getImplementationName(),
                    CDocumentBuilder::_getInstance, CDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CSAXDocumentBuilder::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CSAXDocumentBuilder::_getImplementationName(),
                    CSAXDocumentBuilder::_getInstance, CSAXDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CXPathAPI::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CXPathAPI::_getImplementationName(),
                    CXPathAPI::_getInstance, CXPathAPI::_getSupportedServiceNames()));
        }
        else if (CTestListener::_getImplementationName().compareToAscii( pImplementationName ) == 0 )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CTestListener::_getImplementationName(),
                    CTestListener::_getInstance, CTestListener::_getSupportedServiceNames()));
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"
