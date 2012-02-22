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
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <rtl/oustringostreaminserter.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

#define TWIP_TO_MM100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;
using namespace com::sun::star;

class RtfModelTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    virtual void setUp();
    virtual void tearDown();
    void testFdo45553();
    void testN192129();
    void testFdo45543();
    void testN695479();

    CPPUNIT_TEST_SUITE(RtfModelTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testFdo45553);
    CPPUNIT_TEST(testN192129);
    CPPUNIT_TEST(testFdo45543);
    CPPUNIT_TEST(testN695479);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    void load(const OUString& rURL);
    uno::Reference<lang::XComponent> mxComponent;
};

void RtfModelTest::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/rtftok/data/") + rFilename);
}

void RtfModelTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(getMultiServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), uno::UNO_QUERY);
    CPPUNIT_ASSERT(mxDesktop.is());
}

void RtfModelTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void RtfModelTest::testFdo45553()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo45553.rtf")));

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            OUString aStr = xRange->getString();
            if (aStr.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("space-before")))
            {
                sal_Int32 nMargin = 0;
                uno::Reference<beans::XPropertySet> xPropertySet(xRange, uno::UNO_QUERY);
                uno::Any aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaTopMargin")));
                aValue >>= nMargin;
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(120)), nMargin);
            }
            else if (aStr.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("space-after")))
            {
                sal_Int32 nMargin = 0;
                uno::Reference<beans::XPropertySet> xPropertySet(xRange, uno::UNO_QUERY);
                uno::Any aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBottomMargin")));
                aValue >>= nMargin;
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(240)), nMargin);
            }
        }
    }
}

void RtfModelTest::testN192129()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("n192129.rtf")));

    // We expect that the result will be 16x16px.
    Size aExpectedSize(16, 16);
    MapMode aMap(MAP_100TH_MM);
    aExpectedSize = Application::GetDefaultDevice()->PixelToLogic( aExpectedSize, aMap );

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());

    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
}

void RtfModelTest::testFdo45543()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo45543.rtf")));

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    OUStringBuffer aBuf;
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            aBuf.append(xRange->getString());
        }
    }
    CPPUNIT_ASSERT_EQUAL((sal_Int32)5, aBuf.getLength());
}

void RtfModelTest::testN695479()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("n695479.rtf")));

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    sal_Int16 nSizeType = 0;
    uno::Any aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SizeType")));
    aValue >>= nSizeType;
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, nSizeType);
    sal_Int32 nHeight = 0;
    aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height")));
    aValue >>= nHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(300)), nHeight);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(i), uno::UNO_QUERY);
        if (xServiceInfo->supportsService(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFrame"))))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
            uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(OUString(RTL_CONSTASCII_USTRINGPARAM("plain")), xText->getString());

            if (i == 0)
            {
                // Additonally, the frist frame should have double border at the bottom.
                aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BottomBorder")));
                table::BorderLine2 aBorder;
                aValue >>= aBorder;
                CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::DOUBLE, aBorder.LineStyle);
            }
        }
        else if (xServiceInfo->supportsService(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.LineShape"))))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            sal_Int16 nHori = 0;
            aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HoriOrientRelation")));
            aValue >>= nHori;
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, nHori);
            sal_Int16 nVert = 0;
            aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VertOrientRelation")));
            aValue >>= nVert;
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nVert);
        }
    }
    CPPUNIT_ASSERT(bFrameFound);
    CPPUNIT_ASSERT(bDrawFound);
}

CPPUNIT_TEST_SUITE_REGISTRATION(RtfModelTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
