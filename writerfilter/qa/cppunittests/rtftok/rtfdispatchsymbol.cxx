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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/rtftok/rtfdispatchsymbol.cxx.
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

constexpr OUStringLiteral DATA_DIRECTORY = u"/writerfilter/qa/cppunittests/rtftok/data/";

CPPUNIT_TEST_FIXTURE(Test, testPage)
{
    // Given a file with a \page and 2 \par tokens:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "page.rtf";

    // When loading that file:
    getComponent() = loadFromDesktop(aURL);

    // Then make sure we get exactly two paragraphs:
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xText->createEnumeration();
    xParagraphs->nextElement();
    xParagraphs->nextElement();
    // Without the accompanying fix in place, this test would have failed, the document had 3
    // paragraphs, not 2.
    CPPUNIT_ASSERT(!xParagraphs->hasMoreElements());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
