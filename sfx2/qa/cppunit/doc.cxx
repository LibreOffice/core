/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/propertyvalue.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <osl/file.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sfx2/source/doc/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(Test, testNoGrabBagShape)
{
    // Load a document and select the first shape.
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue("ReadOnly",
                                                                                       true) };
    getComponent() = loadFromDesktop("private:factory/simpress", "", aArgs);
    uno::Reference<frame::XModel> xModel(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage(
        xDrawPagesSupplier->getDrawPages()->getByIndex(0), uno::UNO_QUERY);
    uno::Any aShape = xDrawPage->getByIndex(0);
    uno::Reference<view::XSelectionSupplier> xController(xModel->getCurrentController(),
                                                         uno::UNO_QUERY);
    xController->select(aShape);

    // See if it has a signing certificate associated.
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(xModel.get());
    CPPUNIT_ASSERT(pBaseModel);
    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();

    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.beans.UnknownPropertyException
    // which was not caught later, resulting in a crash.
    pObjectShell->GetSignPDFCertificate();
}

CPPUNIT_TEST_FIXTURE(Test, testTempFilePath)
{
    // Create a test file in a directory that contains the URL-encoded "testÿ" string.
    getComponent() = loadFromDesktop("private:factory/swriter");
    auto pBaseModel = dynamic_cast<SfxBaseModel*>(getComponent().get());
    CPPUNIT_ASSERT(pBaseModel);
    OUString aTargetDir
        = m_directories.getURLFromWorkdir(u"CppunitTest/sfx2_doc.test.user/test%25C3%25Bf");
    osl::Directory::create(aTargetDir);
    OUString aTargetFile = aTargetDir + "/test.odt";
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        "FilterName", OUString("writer8")) };
    pBaseModel->storeAsURL(aTargetFile, aArgs);
    getComponent()->dispose();

    // Load it and export to PDF.
    getComponent() = loadFromDesktop(aTargetFile);
    pBaseModel = dynamic_cast<SfxBaseModel*>(getComponent().get());
    OUString aPdfTarget = aTargetDir + "/test.pdf";
    css::uno::Sequence<css::beans::PropertyValue> aPdfArgs{ comphelper::makePropertyValue(
        "FilterName", OUString("writer_pdf_Export")) };
    // Without the accompanying fix in place, this test would have failed on Windows with:
    // An uncaught exception of type com.sun.star.io.IOException
    // because we first tried to create a temp file next to test.odt in a directory named
    // "test%25C3%25Bf" instead of a directory named "test%C3%Bf".
    pBaseModel->storeToURL(aPdfTarget, aPdfArgs);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
