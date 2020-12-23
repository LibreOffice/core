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

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>

using namespace ::com::sun::star;

/// oox mathml tests.
class OoxMathmlTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void OoxMathmlTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void OoxMathmlTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

OUStringLiteral const DATA_DIRECTORY = u"/oox/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(OoxMathmlTest, testImportCharacters)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "import-characters.pptx";
    // Without the accompanying fix in place, this failed with an assertion failure on import.
    getComponent() = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(getComponent().is());
}

CPPUNIT_TEST_FIXTURE(OoxMathmlTest, testImportMce)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "import-mce.pptx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);

    // Without the accompanying fix in place, this failed with:
    // - Expected: 1
    // - Actual  : 2
    // i.e. both the math object and its replacement image was imported, as two separate objects.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
