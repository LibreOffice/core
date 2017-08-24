/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

namespace
{

char const DATA_DIRECTORY[] = "/writerperfect/qa/unit/data/writer/epubexport/";

/// Tests the EPUB export filter.
class EPUBExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;
    void testOutlineLevel();

    CPPUNIT_TEST_SUITE(EPUBExportTest);
    CPPUNIT_TEST(testOutlineLevel);
    CPPUNIT_TEST_SUITE_END();
};

void EPUBExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void EPUBExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void EPUBExportTest::testOutlineLevel()
{
    // Import the bugdoc and export as EPUB.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "outline-level.fodt";
    mxComponent = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("EPUB");
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());

    // Make sure that the output is split into two.
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(mxComponentContext, aTempFile.GetURL());
    CPPUNIT_ASSERT(xNameAccess->hasByName("OEBPS/sections/section0001.xhtml"));
    // This failed, output was a single section.
    CPPUNIT_ASSERT(xNameAccess->hasByName("OEBPS/sections/section0002.xhtml"));
    CPPUNIT_ASSERT(!xNameAccess->hasByName("OEBPS/sections/section0003.xhtml"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(EPUBExportTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
