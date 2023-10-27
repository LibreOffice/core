/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <sal/main.h>
#include <cppuhelper/bootstrap.hxx>
#include <rtl/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/process.h>

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
    OUString sInputFileName, sOutputFileName;
    OUString sConnectionString("uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager");

    sal_Int32 nCount = rtl_getAppCommandArgCount();

    if (nCount < 1)
    {
        std::cout
            << "using: Convertor -env:URE_MORE_TYPES=<office_types_rdb_url> <file_url> "
               "[<uno_connection_url>]"
            << std::endl
            << std::endl
            << "example: Convertor -env:URE_MORE_TYPES=\"file:///.../program/offapi.rdb\" test.odt"
               "\"uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager\""
            << std::endl;
        exit(1);
    }

    Reference<XComponentContext> xComponentContext(defaultBootstrap_InitialComponentContext());
    Reference<XMultiComponentFactory> xMultiComponentFactoryClient(
        xComponentContext->getServiceManager());
    Reference<XInterface> xInterface = xMultiComponentFactoryClient->createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", xComponentContext);
    Reference<XUnoUrlResolver> resolver(xInterface, UNO_QUERY);
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

    Reference<XPropertySet> xPropSet(xInterface, UNO_QUERY);
    xPropSet->getPropertyValue("DefaultContext") >>= xComponentContext;
    Reference<XMultiComponentFactory> xMultiComponentFactoryServer(
        xComponentContext->getServiceManager());
    Reference<XDesktop2> xComponentLoader = Desktop::create(xComponentContext);
    Sequence<PropertyValue> loadProperties(1);
    loadProperties[0].Name = "Hidden";
    loadProperties[0].Value <<= true;
    try
    {
        OUString sInputUrl, sAbsoluteInputUrl, sOutputUrl, sAbsoluteOutputUrl, sWorkingDir;
        osl_getProcessWorkingDir(&sWorkingDir.pData);
        rtl_getAppCommandArg(0, &sInputFileName.pData);

        osl::FileBase::getFileURLFromSystemPath(sInputFileName, sInputUrl);
        osl::FileBase::getAbsoluteFileURL(sWorkingDir, sInputUrl, sAbsoluteInputUrl);
        std::cout << sAbsoluteInputUrl << std::endl;

        osl::FileBase::getFileURLFromSystemPath("output.pdf", sOutputUrl);
        osl::FileBase::getAbsoluteFileURL(sWorkingDir, sOutputUrl, sAbsoluteOutputUrl);
        std::cout << sAbsoluteOutputUrl << std::endl;

        Reference<XComponent> xComponent = xComponentLoader->loadComponentFromURL(
            sAbsoluteInputUrl, "_blank", 0, loadProperties);
        Reference<XTextDocument> xDocument(xComponent, UNO_QUERY_THROW);
        Reference<XStorable> xStorable(xDocument, UNO_QUERY_THROW);
        Sequence<PropertyValue> storeProps(3);
        storeProps[0].Name = "FilterName";
        storeProps[0].Value <<= OUString("writer_pdf_Export");
        storeProps[1].Name = "Overwrite";
        storeProps[1].Value <<= true;
        storeProps[2].Name = "SelectPdfVersion";
        storeProps[2].Value <<= sal_Int32(1);
        xStorable->storeToURL(sAbsoluteOutputUrl, storeProps);
        Reference<XComponent>::query(xMultiComponentFactoryClient)->dispose();
        std::cout << "Output output.pdf generated." << std::endl;
    }
    catch (Exception& e)
    {
        std::cout << "Can not open the input file." << std::endl << e.Message << std::endl;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
