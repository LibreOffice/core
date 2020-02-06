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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>

#include <comphelper/processfactory.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

/// oox drawingml tests.
class OoxDrawingmlTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
    void loadAndReload(const OUString& rURL, const OUString& rFilterName);
};

void OoxDrawingmlTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void OoxDrawingmlTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void OoxDrawingmlTest::loadAndReload(const OUString& rURL, const OUString& rFilterName)
{
    mxComponent = loadFromDesktop(rURL);
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= rFilterName;
    utl::TempFile aTempFile;
    xStorable->storeToURL(aTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mxComponent->dispose();
    validate(aTempFile.GetFileName(), test::OOXML);
    mxComponent = loadFromDesktop(aTempFile.GetURL());
}

char const DATA_DIRECTORY[] = "/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(OoxDrawingmlTest, testTransparentText)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "transparent-text.pptx";
    loadAndReload(aURL, "Impress Office Open XML");

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraph(
        xShape->createEnumeration()->nextElement(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPortion(xParagraph->createEnumeration()->nextElement(),
                                                 uno::UNO_QUERY);

    sal_Int16 nTransparency = 0;
    xPortion->getPropertyValue("CharTransparence") >>= nTransparency;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 75
    // - Actual  : 0
    // i.e. the transparency of the character color was lost on import/export.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(75), nTransparency);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
