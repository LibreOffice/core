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

#include <prewin.h>
#include <postwin.h>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/propertyvalue.hxx>
#include <osl/file.hxx>
#include <unotools/tempfile.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svx/svdview.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/filter/pdfdocument.hxx>

using namespace ::com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/vcl/qa/cppunit/filter/ipdf/data/";
}

/// Covers vcl/source/filter/ipdf/ fixes.
class VclFilterIpdfTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;
    uno::Reference<xml::crypto::XSEInitializer> mxSEInitializer;
    uno::Reference<xml::crypto::XXMLSecurityContext> mxSecurityContext;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    uno::Reference<xml::crypto::XXMLSecurityContext>& getSecurityContext()
    {
        return mxSecurityContext;
    }
};

void VclFilterIpdfTest::setUp()
{
    test::BootstrapFixture::setUp();

    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());
    mxDesktop.set(frame::Desktop::create(xComponentContext));
    mxSEInitializer = xml::crypto::SEInitializer::create(xComponentContext);
    mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());
}

void VclFilterIpdfTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(VclFilterIpdfTest, testDictArrayDict)
{
    // Load a file that has markup like this:
    // 3 0 obj <<
    //   /Key[<</InnerKey 42>>]
    // >>
    OUString aSourceURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "dict-array-dict.pdf";
    SvFileStream aFile(aSourceURL, StreamMode::READ);
    vcl::filter::PDFDocument aDocument;
    CPPUNIT_ASSERT(aDocument.Read(aFile));
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT(!aPages.empty());
    vcl::filter::PDFObjectElement* pPage = aPages[0];
    auto pKey = dynamic_cast<vcl::filter::PDFArrayElement*>(pPage->Lookup("Key"));

    // Without the accompanying fix in place, this test would have failed, because the value of Key
    // was a dictionary element, not an array element.
    CPPUNIT_ASSERT(pKey);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
