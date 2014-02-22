/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

#include "../dom/documentbuilder.hxx"
#include "../dom/saxbuilder.hxx"
#include "../xpath/xpathapi.hxx"
#include "../events/testlistener.hxx"

using namespace ::DOM;
using namespace ::DOM::events;
using namespace ::XPath;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL unoxml_component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void * /*pRegistryKey*/)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (CDocumentBuilder::_getImplementationName().equalsAscii( pImplementationName )  )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createOneInstanceFactory(
                    xServiceManager, CDocumentBuilder::_getImplementationName(),
                    CDocumentBuilder::_getInstance, CDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CSAXDocumentBuilder::_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CSAXDocumentBuilder::_getImplementationName(),
                    CSAXDocumentBuilder::_getInstance, CSAXDocumentBuilder::_getSupportedServiceNames()));
        }
        else if (CXPathAPI::_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CXPathAPI::_getImplementationName(),
                    CXPathAPI::_getInstance, CXPathAPI::_getSupportedServiceNames()));
        }
        else if (CTestListener::_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = Reference< XSingleServiceFactory >(
                cppu::createSingleFactory(
                    xServiceManager, CTestListener::_getImplementationName(),
                    CTestListener::_getInstance, CTestListener::_getSupportedServiceNames()));
        }

        
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    
    return pReturn ;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
