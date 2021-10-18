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

#include <iostream>
#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace cppu;
using namespace rtl;
using namespace css::uno;
using namespace css::beans;
using namespace css::bridge;
using namespace css::frame;
using namespace css::lang;
using namespace css::text;

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    OUString sInputFileName("test.odt"), sOutputFileName("test.pdf");
    OUString sConnectionString("uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager");

    auto xComponentContext(defaultBootstrap_InitialComponentContext());
    auto xMultiComponentFactoryClient(xComponentContext->getServiceManager());
    auto xInterface = xMultiComponentFactoryClient->createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", xComponentContext);
    auto resolver = Reference<XUnoUrlResolver>(xInterface, UNO_QUERY);
    try
    {
        xInterface = Reference<XInterface>(resolver->resolve(sConnectionString), UNO_QUERY_THROW);
    }
    catch (Exception& e)
    {
        std::cout << "Error: cannot establish a connection using '" << sConnectionString << "'"
                  << std::endl
                  << e.Message << std::endl;
        std::exit(1);
    }

    auto xPropSet = Reference<XPropertySet>(xInterface, UNO_QUERY);
    xPropSet->getPropertyValue("DefaultContext") >>= xComponentContext;
    auto xMultiComponentFactoryServer(xComponentContext->getServiceManager());
    auto xComponentLoader = Desktop::create(xComponentContext);
    Sequence<PropertyValue> loadProperties(1);
    loadProperties[0].Name = "Hidden";
    loadProperties[0].Value <<= true;
    try
    {
        OUString sInputUrl, sOutputUrl, sWorkingDir;
        osl_getProcessWorkingDir(&sWorkingDir.pData);

        osl::FileBase::getAbsoluteFileURL(sWorkingDir, sInputUrl, sInputFileName);

        osl::FileBase::getAbsoluteFileURL(sWorkingDir, sOutputUrl, sOutputFileName);

        auto xComponent = xComponentLoader->loadComponentFromURL("file:///~/test.odt", "_blank", 0,
                                                                 loadProperties);
        auto xDocument = Reference<XTextDocument>(xComponent, UNO_QUERY_THROW);
        auto xStorable = Reference<XStorable>(xDocument, UNO_QUERY_THROW);
        auto storeProps = Sequence<PropertyValue>(3);
        storeProps[0].Name = "FilterName";
        storeProps[0].Value <<= OUString("writer_pdf_Export");
        storeProps[1].Name = "Overwrite";
        storeProps[1].Value <<= true;
        storeProps[2].Name = "SelectPdfVersion";
        storeProps[2].Value <<= sal_Int32(1);
        xStorable->storeToURL("file:///~/test.pdf", storeProps);
        Reference<XComponent>::query(xMultiComponentFactoryClient)->dispose();
        std::cout << "Output test.pdf generated." << std::endl;
    }
    catch (Exception& e)
    {
        std::cout << "Can not open the file test.odt" << std::endl;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
