/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/container/xenumerationaccess.hxx>
#include <test/util/xrefreshable.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextFieldTypes.
 */
class SwXTextFieldTypes final : public UnoApiTest,
                                public apitest::XElementAccess,
                                public apitest::XEnumerationAccess,
                                public apitest::XRefreshable
{
public:
    SwXTextFieldTypes()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XDependentTextField>::get())
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
        mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    Reference<XInterface> init() override
    {
        component_ = loadFromDesktop(u"private:factory/swriter"_ustr,
                                     u"com.sun.star.text.TextDocument"_ustr);
        Reference<text::XTextDocument> xTextDocument(component_, UNO_QUERY_THROW);

        Reference<text::XTextFieldsSupplier> xTFS;

        try
        {
            xTFS = Reference<text::XTextFieldsSupplier>(xTextDocument, UNO_QUERY_THROW);
        }
        catch (Exception&)
        {
        }

        return Reference<XInterface>(xTFS->getTextFields(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXTextFieldTypes);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testCreateEnumeration);
    CPPUNIT_TEST(testRefreshListener);
    CPPUNIT_TEST_SUITE_END();

private:
    Reference<lang::XComponent> component_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextFieldTypes);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
