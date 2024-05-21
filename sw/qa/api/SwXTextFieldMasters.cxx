/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xelementaccess.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
// #include <com/sun/star/container/XNameAccess.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXTextFieldMasters.
 */
class SwXTextFieldMasters final : public UnoApiTest,
                                  public apitest::XElementAccess,
                                  public apitest::XNameAccess
{
public:
    SwXTextFieldMasters()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<beans::XPropertySet>::get())
        , XNameAccess(u"com.sun.star.text.fieldmaster.SetExpression.Illustration"_ustr)
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
        Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);

        Reference<text::XTextFieldsSupplier> xTFS;

        try
        {
            xTFS = Reference<text::XTextFieldsSupplier>(xTextDocument, UNO_QUERY_THROW);
        }
        catch (Exception&)
        {
        }

        // Reference<container::XNameAccess> xNA = xTFS->getTextFieldMasters();
        // Sequence<OUString> aNames = xNA->getElementNames();
        // std::cout << aNames[0] << std::endl;

        return Reference<XInterface>(xTFS->getTextFieldMasters(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXTextFieldMasters);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXTextFieldMasters);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
