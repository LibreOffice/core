/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xenumeration.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXFieldEnumeration.
 */
class SwXFieldEnumeration final : public UnoApiTest, public apitest::XEnumeration
{
public:
    SwXFieldEnumeration()
        : UnoApiTest(u""_ustr)
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
        Reference<lang::XMultiServiceFactory> xMSF(mxComponent, UNO_QUERY_THROW);

        Reference<beans::XPropertySet> xFieldMaster;
        Reference<text::XDependentTextField> xTF;

        try
        {
            xFieldMaster = Reference<beans::XPropertySet>(
                xMSF->createInstance(u"com.sun.star.text.FieldMaster.Database"_ustr),
                UNO_QUERY_THROW);
            xTF = Reference<text::XDependentTextField>(
                xMSF->createInstance(u"com.sun.star.text.TextField.Database"_ustr),
                UNO_QUERY_THROW);
        }
        catch (Exception&)
        {
        }

        try
        {
            xFieldMaster->setPropertyValue(u"DataBaseName"_ustr, Any(u"Bibliography"_ustr));
            xFieldMaster->setPropertyValue(u"DataTableName"_ustr, Any(u"biblio"_ustr));
            xFieldMaster->setPropertyValue(u"DataColumnName"_ustr, Any(u"Address"_ustr));
        }
        catch (lang::WrappedTargetException&)
        {
        }
        catch (lang::IllegalArgumentException&)
        {
        }
        catch (beans::UnknownPropertyException&)
        {
        }
        catch (beans::PropertyVetoException&)
        {
        }

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();
        Reference<text::XTextContent> xTextContent(xTF, UNO_QUERY_THROW);

        xTF->attachTextFieldMaster(xFieldMaster);
        xText->insertTextContent(xCursor, xTextContent, false);

        Reference<text::XTextFieldsSupplier> xTFS(xTextDocument, UNO_QUERY_THROW);
        Reference<container::XEnumerationAccess> xEnumerationAccess = xTFS->getTextFields();

        return Reference<XInterface>(xEnumerationAccess->createEnumeration(), UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXFieldEnumeration);
    CPPUNIT_TEST(testHasMoreElements);
    CPPUNIT_TEST(testNextElement);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXFieldEnumeration);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
