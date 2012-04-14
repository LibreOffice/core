/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@suse.cz> (SUSE, Inc.)
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/svapp.hxx>

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;
using namespace com::sun::star;

class OoxmlModelTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp();
    virtual void tearDown();
    void testN751054();
    void testN751117();
    void testN751017();
    void testN750935();

    CPPUNIT_TEST_SUITE(OoxmlModelTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testN751054);
    CPPUNIT_TEST(testN751117);
    CPPUNIT_TEST(testN751017);
    CPPUNIT_TEST(testN750935);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load an OOXML file and make the document available via mxComponent.
    void load(const OUString& rURL);
    uno::Reference<lang::XComponent> mxComponent;
};

void OoxmlModelTest::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/ooxmltok/data/") + rFilename);
}

void OoxmlModelTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(getMultiServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxDesktop.is());
}

void OoxmlModelTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void OoxmlModelTest::testN751054()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("n751054.docx")));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    text::TextContentAnchorType eValue;
    xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorType"))) >>= eValue;
    CPPUNIT_ASSERT(eValue != text::TextContentAnchorType_AS_CHARACTER);
}

void OoxmlModelTest::testN751117()
{
    load("n751117.docx");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);

    // First shape: the end should be an arrow, should be rotated and should be flipped.
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    OUString aValue;
    xPropertySet->getPropertyValue("LineEndName") >>= aValue;
    CPPUNIT_ASSERT(aValue.indexOf("Arrow") != -1);

    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("RotateAngle") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(90 * 100), nValue);

    uno::Reference<drawing::XShape> xShape(xPropertySet, uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());
    CPPUNIT_ASSERT(aActualSize.Width < 0);

    // The second shape should be a line
    uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.drawing.LineShape"));
}

void OoxmlModelTest::testN751017()
{
    load("n751017.docx");
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xMasters(xTextFieldsSupplier->getTextFieldMasters());
    // Make sure we have a variable named foo.
    CPPUNIT_ASSERT(xMasters->hasByName("com.sun.star.text.FieldMaster.SetExpression.foo"));

    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    bool bFoundSet(false), bFoundGet(false);
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        sal_Int16 nValue = 0;
        OUString aValue;
        if (xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression"))
        {
            bFoundSet = true;
            uno::Reference<text::XDependentTextField> xDependentTextField(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xMasterProps(xDependentTextField->getTextFieldMaster());

            // First step: did we set foo to "bar"?
            xMasterProps->getPropertyValue("Name") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
        else if (xServiceInfo->supportsService("com.sun.star.text.TextField.GetExpression"))
        {
            // Second step: check the value of foo.
            bFoundGet = true;
            xPropertySet->getPropertyValue("Content") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("foo"), aValue);
            xPropertySet->getPropertyValue("SubType") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(text::SetVariableType::STRING, nValue);
            xPropertySet->getPropertyValue("CurrentPresentation") >>= aValue;
            CPPUNIT_ASSERT_EQUAL(OUString("bar"), aValue);
        }
    }
    CPPUNIT_ASSERT(bFoundSet);
    CPPUNIT_ASSERT(bFoundGet);
}

void OoxmlModelTest::testN750935()
{
    load("n750935.docx");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5), xCursor->getPage());
}

CPPUNIT_TEST_SUITE_REGISTRATION(OoxmlModelTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
