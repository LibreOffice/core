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
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

class LotusWordProTest: public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp() override;

    virtual void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }

private:
    uno::Reference<uno::XComponentContext> mxComponentContext;
    uno::Reference<lang::XComponent> mxComponent;
};

void LotusWordProTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxComponentContext.set(comphelper::getComponentContext(getMultiServiceFactory()));
    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void LotusWordProTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

char const DATA_DIRECTORY[] = "/lotuswordpro/qa/cppunit/data/";

CPPUNIT_TEST_FIXTURE(LotusWordProTest, testTdf129993)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf129993.lwp";
    getComponent() = loadFromDesktop(aURL);
    CPPUNIT_ASSERT(getComponent().is());

    uno::Reference<text::XTextDocument> textDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(textDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    for(int i = 0; i < 15; ++i)
        xParaEnum->nextElement();
    uno::Reference<lang::XServiceInfo> xServiceInfo(xParaEnum->nextElement(),
            uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_True, xServiceInfo->supportsService("com.sun.star.text.TextTable"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
