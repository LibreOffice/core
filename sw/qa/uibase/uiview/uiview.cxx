/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/file.hxx>

#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <swdtflvr.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/uibase/uiview/data/";

/// Covers sw/source/uibase/uiview/ fixes.
class SwUibaseUiviewTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseUiviewTest, testUpdateAllObjectReplacements)
{
    // Make a temporary copy of the test document
    utl::TempFile tmp;
    tmp.EnableKillingFile();
    OUString sTempCopy = tmp.GetURL();
    CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                         osl::File::copy(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                             + "updateall-objectreplacements.odt",
                                         sTempCopy));

    /* BASIC code that exhibits the problem:

    desktop = CreateUnoService("com.sun.star.frame.Desktop")
    Dim props(0) as new com.sun.star.beans.PropertyValue
    props(0).Name = "Hidden"
    props(0).Value = true
    component = desktop.loadComponentFromURL("file://.../test.odt", "_default", 0, props)
    Wait 1000 ' workaround
    dispatcher = createUnoService("com.sun.star.frame.DispatchHelper")
    frame = component.CurrentController.Frame
    dispatcher.executeDispatch(frame, ".uno:UpdateAll", "", 0, Array())
    component.storeSelf(Array())
    component.dispose()
    */

    uno::Reference<lang::XMultiServiceFactory> xFactory(comphelper::getProcessServiceFactory());

    // Load the copy
    uno::Reference<uno::XInterface> xInterface
        = xFactory->createInstance("com.sun.star.frame.Desktop");
    uno::Reference<frame::XComponentLoader> xComponentLoader(xInterface, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aLoadArgs(1);
    aLoadArgs[0].Name = "Hidden";
    aLoadArgs[0].Value <<= true;
    mxComponent = xComponentLoader->loadComponentFromURL(sTempCopy, "_default", 0, aLoadArgs);

    // Perform the .uno:UpdateAll call and save
    xInterface = xFactory->createInstance("com.sun.star.frame.DispatchHelper");
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(xInterface, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<frame::XDispatchProvider> xDispatchProvider(
        xModel->getCurrentController()->getFrame(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aNoArgs;
    xDispatchHelper->executeDispatch(xDispatchProvider, ".uno:UpdateAll", OUString(), 0, aNoArgs);
    uno::Reference<frame::XStorable2> xStorable(mxComponent, uno::UNO_QUERY);
    xStorable->storeSelf(aNoArgs);

    // Check the contents of the updated copy and verify that ObjectReplacements are there
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess
        = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(xFactory),
                                                      sTempCopy);

    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Components"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("ObjectReplacements/Components_1"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
