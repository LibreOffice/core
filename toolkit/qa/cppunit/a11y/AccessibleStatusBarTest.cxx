/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <string>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XCloseable.hpp>

#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/scheduler.hxx>

#include <test/a11y/AccessibilityTools.hxx>
#include "XAccessibleComponentTester.hxx"
#include "XAccessibleContextTester.hxx"
#include "XAccessibleExtendedComponentTester.hxx"
#include "XAccessibleEventBroadcasterTester.hxx"

using namespace css;

namespace
{
class AccessibleStatusBarTest : public test::BootstrapFixture
{
private:
    uno::Reference<frame::XDesktop2> mxDesktop;

    uno::Reference<accessibility::XAccessibleContext>
    getTestObject(const uno::Reference<awt::XWindow>& xWindow);
    void runAllTests(const uno::Reference<awt::XWindow>& xWindow);
    uno::Reference<lang::XComponent> openDocument(std::string_view sKind);
    void testDocument(std::string_view sKind);

    void testWriterDoc() { testDocument("swriter"); }
    void testMathDoc() { testDocument("smath"); }
    void testDrawDoc() { testDocument("sdraw"); }
    void testImpressDoc() { testDocument("simpress"); }
    void testCalcDoc() { testDocument("scalc"); }

public:
    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(AccessibleStatusBarTest);
    CPPUNIT_TEST(testWriterDoc);
    CPPUNIT_TEST(testMathDoc);
    CPPUNIT_TEST(testDrawDoc);
    CPPUNIT_TEST(testImpressDoc);
    CPPUNIT_TEST(testCalcDoc);
    CPPUNIT_TEST_SUITE_END();
};

void AccessibleStatusBarTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop = frame::Desktop::create(mxComponentContext);
}

uno::Reference<accessibility::XAccessibleContext>
AccessibleStatusBarTest::getTestObject(const uno::Reference<awt::XWindow>& xWindow)
{
    uno::Reference<accessibility::XAccessible> xAccessible(xWindow, uno::UNO_QUERY_THROW);
    std::cout << "got accessible: " << xAccessible << std::endl;
    std::cout << "accessible name: " << AccessibilityTools::debugString(xAccessible) << std::endl;

    auto xContext = AccessibilityTools::getAccessibleObjectForRole(
        xAccessible, accessibility::AccessibleRole::STATUS_BAR);
    std::cout << "got context: " << xContext << std::endl;
    std::cout << "context name: " << AccessibilityTools::debugString(xContext) << std::endl;

    Scheduler::ProcessEventsToIdle(); // not sure why?

    uno::Reference<lang::XServiceInfo> xSI(xContext, uno::UNO_QUERY_THROW);
    std::cout << "implementation name: " << xSI->getImplementationName() << std::endl;
    auto serviceNames = xSI->getSupportedServiceNames();
    std::cout << "has " << serviceNames.size() << " services:" << std::endl;
    for (auto& service : serviceNames)
        std::cout << " * service: " << service << std::endl;

    return xContext;
}

void AccessibleStatusBarTest::runAllTests(const uno::Reference<awt::XWindow>& xWindow)
{
    auto xContext = getTestObject(xWindow);

    uno::Reference<accessibility::XAccessibleComponent> xAccessibleComponent(xContext,
                                                                             uno::UNO_QUERY_THROW);
    XAccessibleComponentTester componentTester(xAccessibleComponent);
    componentTester.testAll();

    XAccessibleContextTester contextTester(xContext);
    contextTester.testAll();

    uno::Reference<accessibility::XAccessibleExtendedComponent> xAccessibleExtendedComponent(
        xContext, uno::UNO_QUERY_THROW);
    XAccessibleExtendedComponentTester extendedComponentTester(xAccessibleExtendedComponent);
    extendedComponentTester.testAll();

    uno::Reference<accessibility::XAccessibleEventBroadcaster> xAccessibleEventBroadcaster(
        xContext, uno::UNO_QUERY_THROW);
    XAccessibleEventBroadcasterTester eventBroadcasterTester(xAccessibleEventBroadcaster, xWindow);
    eventBroadcasterTester.testAll();
}

uno::Reference<lang::XComponent> AccessibleStatusBarTest::openDocument(std::string_view sKind)
{
    /* not sure what's that about, but from SOfficeFactory.java:openDoc() */
    // that noargs thing for load attributes
    std::vector<beans::PropertyValue> aArgs;
    if (sKind == "simpress")
    {
        beans::PropertyValue aValue;
        aValue.Name = "OpenFlags";
        aValue.Handle = -1;
        aValue.Value <<= OUString("S");
        aValue.State = beans::PropertyState_DIRECT_VALUE;
        aArgs.push_back(aValue);
    }

    rtl::OUStringBuffer sURL("private:factory/");
    sURL.appendAscii(sKind.data(), sKind.length());

    return mxDesktop->loadComponentFromURL(sURL.makeStringAndClear(), "_blank", 40,
                                           comphelper::containerToSequence(aArgs));
}

void AccessibleStatusBarTest::testDocument(std::string_view sKind)
{
    auto xDoc = openDocument(sKind);
    std::cout << "got document: " << xDoc << std::endl;
    CPPUNIT_ASSERT(xDoc.is());
    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY_THROW);
    std::cout << "got model: " << xModel << std::endl;
    uno::Reference<awt::XWindow> xWindow(
        xModel->getCurrentController()->getFrame()->getContainerWindow());
    std::cout << "got window: " << xWindow << std::endl;
    uno::Reference<awt::XTopWindow> xTopWindow(xWindow, uno::UNO_QUERY_THROW);
    std::cout << "got top window: " << xTopWindow << std::endl;
    xTopWindow->toFront();

    Scheduler::ProcessEventsToIdle();

    runAllTests(xWindow);

    // close document
    uno::Reference<css::util::XCloseable> xCloseable(xDoc, uno::UNO_QUERY_THROW);
    xCloseable->close(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(AccessibleStatusBarTest);
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
