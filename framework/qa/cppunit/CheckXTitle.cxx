/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/scheduler.hxx>

using namespace ::com::sun::star;

namespace
{
// These were JUnit tests from framework/qa/complex/XTitle/CheckXTitle.java
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"/framework/qa/cppunit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, checkDefaultTitle)
{
    // Cycles through default view and print preview
    // and asserts that the title doesn't change.

    // Load document
    uno::Reference<lang::XMultiServiceFactory> xFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface
        = xFactory->createInstance("com.sun.star.frame.Desktop");
    uno::Reference<frame::XComponentLoader> xComponentLoader(xInterface, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue("Hidden", false) };
    mxComponent = xComponentLoader->loadComponentFromURL("private:factory/swriter", "_default", 0,
                                                         aLoadArgs);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);

    uno::Reference<frame::XTitle> xTitle(xFrame, css::uno::UNO_QUERY);
    OUString defaultTitle = xTitle->getTitle();

    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(mxComponentContext));

    // Open print preview window and get title.
    util::URL printPreviewURL;
    printPreviewURL.Complete = u".uno::PrintPreview"_ustr;
    xParser->parseStrict(printPreviewURL);
    uno::Reference<frame::XDispatchProvider> xDispatchProvider(xModel->getCurrentController(),
                                                               uno::UNO_QUERY);
    xDispatchProvider->queryDispatch(printPreviewURL, "", 0);
    uno::Reference<frame::XTitle> xTitle2(xFrame, css::uno::UNO_QUERY);
    Scheduler::ProcessEventsToIdle();

    // Default window title and print preview window title should be the same.
    CPPUNIT_ASSERT_EQUAL(defaultTitle, xTitle2->getTitle());

    // Close print preview window and check to see if default window title has changed
    util::URL closePreviewURL;
    closePreviewURL.Complete = u".uno::ClosePreview"_ustr;
    xParser->parseStrict(closePreviewURL);
    uno::Reference<frame::XDispatchProvider> xDispatchProvider2(xModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xDispatchProvider2->queryDispatch(closePreviewURL, "", 0);
    uno::Reference<frame::XTitle> xTitle3(xFrame, css::uno::UNO_QUERY);
    Scheduler::ProcessEventsToIdle();

    // Default window title should be the same after closing print preview window.
    CPPUNIT_ASSERT_EQUAL(defaultTitle, xTitle3->getTitle());
}

CPPUNIT_TEST_FIXTURE(Test, checkTitleSuggestedFileName)
{
    uno::Sequence<beans::PropertyValue> aArguments
        = { comphelper::makePropertyValue(u"SuggestedSaveAsName"_ustr, u"suggestedname.odt"_ustr),
            comphelper::makePropertyValue(u"Hidden"_ustr, false) };
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr, "com.sun.star.text.TextDocument",
                                  aArguments);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);

    uno::Reference<frame::XTitle> xTitle(xFrame, css::uno::UNO_QUERY);
    OUString windowTitle = xTitle->getTitle();

    CPPUNIT_ASSERT(windowTitle.startsWith("suggestedname.odt"));
}

CPPUNIT_TEST_FIXTURE(Test, setTitleAndCheck)
{
    // Set the frame title, then cycle through default and print preview. Close the window
    // and check for infinite recursion.

    uno::Reference<lang::XMultiServiceFactory> xFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface
        = xFactory->createInstance("com.sun.star.frame.Desktop");
    uno::Reference<frame::XComponentLoader> xComponentLoader(xInterface, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue("Hidden", false) };
    mxComponent = xComponentLoader->loadComponentFromURL("private:factory/swriter", "_default", 0,
                                                         aLoadArgs);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    uno::Reference<frame::XFrame2> xFrame(xController->getFrame(), uno::UNO_QUERY);

    xFrame->setTitle(u"documentTitle"_ustr);

    uno::Reference<frame::XDispatchProvider> xDispatchProvider(xModel->getCurrentController(),
                                                               uno::UNO_QUERY);
    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(mxComponentContext));

    util::URL printPreviewURL;
    printPreviewURL.Complete = u".uno::PrintPreview"_ustr;
    xParser->parseStrict(printPreviewURL);
    xDispatchProvider->queryDispatch(printPreviewURL, "", 0);
    Scheduler::ProcessEventsToIdle();

    util::URL closePreviewURL;
    closePreviewURL.Complete = u".uno::ClosePreview"_ustr;
    xParser->parseStrict(closePreviewURL);
    uno::Reference<frame::XDispatchProvider> xDispatchProvider2(xModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    xDispatchProvider2->queryDispatch(closePreviewURL, "", 0);
    Scheduler::ProcessEventsToIdle();

    util::URL closeDocURL;
    closeDocURL.Complete = u".uno::CloseWin"_ustr;
    xParser->parseStrict(closeDocURL);
    uno::Reference<frame::XDispatchProvider> xDispatchProvider3(xModel->getCurrentController(),
                                                                uno::UNO_QUERY);
    uno::Reference<frame::XTitle> xTitle(xFrame, css::uno::UNO_QUERY);
    OUString windowTitle = xTitle->getTitle();

    CPPUNIT_ASSERT_EQUAL(windowTitle, u"documentTitle"_ustr);

    xDispatchProvider3->queryDispatch(closeDocURL, "", 0);
    Scheduler::ProcessEventsToIdle();
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
