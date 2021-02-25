/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <test/lang/xcomponent.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextCursor.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;
using namespace css::beans;

namespace
{
/**
 * Initial tests for SwXTextField.
 */
struct SwXTextField final : public test::BootstrapFixture,
                            public unotest::MacrosTest,
                            public apitest::XComponent
{
    virtual void setUp() override;
    void tearDown() override;

    Reference<XInterface> init() override;
    void triggerDesktopTerminate() override;

    CPPUNIT_TEST_SUITE(SwXTextField);
    CPPUNIT_TEST(testAddEventListener);
    CPPUNIT_TEST(testRemoveEventListener);
    CPPUNIT_TEST_SUITE_END();

private:
    css::uno::Reference<css::lang::XComponent> component_;
};

void SwXTextField::setUp()
{
    test::BootstrapFixture::setUp();
    mxDesktop.set(
        frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void SwXTextField::tearDown()
{
    if (component_.is())
    {
        component_->dispose();
    }
}

void SwXTextField::triggerDesktopTerminate() { mxDesktop->terminate(); }

Reference<XInterface> SwXTextField::init()
{
    component_ = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);
    Reference<lang::XMultiServiceFactory> xMSF(component_, UNO_QUERY_THROW);

    Reference<XPropertySet> xFieldMaster(
        xMSF->createInstance("com.sun.star.text.FieldMaster.Database"), UNO_QUERY_THROW);
    xFieldMaster->setPropertyValue("DataBaseName", makeAny(OUString("Address Book File")));
    xFieldMaster->setPropertyValue("DataTableName", makeAny(OUString("address")));
    xFieldMaster->setPropertyValue("DataColumnName", makeAny(OUString("FIRSTNAME")));

    Reference<text::XDependentTextField> xField(
        xMSF->createInstance("com.sun.star.text.TextField.Database"), UNO_QUERY_THROW);
    xField->attachTextFieldMaster(xFieldMaster);
    Reference<text::XText> xText = xTextDocument->getText();
    Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    Reference<text::XTextContent> xFieldAsContent(xField, UNO_QUERY_THROW);
    xText->insertTextContent(xCursor, xFieldAsContent, false);
    return Reference<XInterface>(xField, UNO_QUERY_THROW);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextField);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
