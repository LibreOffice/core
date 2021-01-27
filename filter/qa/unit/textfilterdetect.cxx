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

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/propertyvalue.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

namespace com::sun::star::io
{
class XInputStream;
}

using namespace com::sun::star;

namespace
{
/// Test class for PlainTextFilterDetect.
class TextFilterDetectTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void TextFilterDetectTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void TextFilterDetectTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/filter/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testTdf114428)
{
    uno::Reference<document::XExtendedFilterDetection> xDetect(
        getMultiServiceFactory()->createInstance("com.sun.star.comp.filters.PlainTextFilterDetect"),
        uno::UNO_QUERY);
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf114428.xhtml";
    SvFileStream aStream(aURL, StreamMode::READ);
    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor
        = { comphelper::makePropertyValue("DocumentService",
                                          OUString("com.sun.star.text.TextDocument")),
            comphelper::makePropertyValue("InputStream", xStream),
            comphelper::makePropertyValue("TypeName", OUString("generic_HTML")) };
    xDetect->detect(aDescriptor);
    utl::MediaDescriptor aMediaDesc(aDescriptor);
    OUString aFilterName = aMediaDesc.getUnpackedValueOrDefault("FilterName", OUString());
    // This was empty, XML declaration caused HTML detect to not handle XHTML.
    CPPUNIT_ASSERT_EQUAL(OUString("HTML (StarWriter)"), aFilterName);
}

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testEmptyFile)
{
    // Given an empty file, with a pptx extension
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "empty.pptx";

    // When loading the file
    getComponent() = loadFromDesktop(aURL);

    // Then make sure it is opened in Impress.
    uno::Reference<lang::XServiceInfo> xServiceInfo(getComponent(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo.is());

    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument"));

    getComponent()->dispose();

    // Now also test ODT
    aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "empty.odt";
    getComponent() = loadFromDesktop(aURL);
    xServiceInfo.set(getComponent(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo.is());
    // Make sure it opens in Writer.
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextDocument"));
    getComponent()->dispose();

    // ... and ODP
    aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "empty.odp";
    getComponent() = loadFromDesktop(aURL);
    xServiceInfo.set(getComponent(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo.is());
    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.presentation.PresentationDocument"));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
