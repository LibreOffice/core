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
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

char const DATA_DIRECTORY[] = "/xmloff/qa/unit/data/";

/// Covers xmloff/source/text/ fixes.
class XmloffStyleTest : public test::BootstrapFixture,
                        public unotest::MacrosTest,
                        public XmlTestTools
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    void registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx) override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void XmloffStyleTest::registerNamespaces(xmlXPathContextPtr& pXmlXpathCtx)
{
    XmlTestTools::registerODFNamespaces(pXmlXpathCtx);
}

void XmloffStyleTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XmloffStyleTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testMailMergeInEditeng)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "mail-merge-editeng.odt";
    getComponent() = loadFromDesktop(aURL);
    // Without the accompanying fix in place, this test would have failed, as unexpected
    // <text:database-display> in editeng text abored the whole import process.
    CPPUNIT_ASSERT(getComponent().is());
}

CPPUNIT_TEST_FIXTURE(XmloffStyleTest, testListId)
{
    // Given a document with a simple list (no continue-list="..." attribute):
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "list-id.fodt";
    getComponent() = loadFromDesktop(aURL);

    // When storing that document as ODF:
    uno::Reference<frame::XStorable> xStorable(getComponent(), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aStoreProps = comphelper::InitPropertySequence({
        { "FilterName", uno::makeAny(OUString("writer8")) },
    });
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    xStorable->storeToURL(aTempFile.GetURL(), aStoreProps);

    // Then make sure that unreferenced xml:id="..." attributes are not written:
    std::unique_ptr<SvStream> pStream = parseExportStream(aTempFile, "content.xml");
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    // Without the accompanying fix in place, this failed with:
    // - XPath '//text:list' unexpected 'id' attribute
    // i.e. xml:id="..." was written unconditionally, even when no other list needed it.
    assertXPathNoAttribute(pXmlDoc, "//text:list", "id");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
