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
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>

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
    void testFdo42465();
    void testFdo45187();
    void testFdo46662();
    void testN750757();
    void testFdo45563();
    void testFdo43965();
    void testN751020();
    void testFdo47326();
    void testFdo47036();
    void testFdo46955();
    void testFdo45394();
    void testFdo48104();

    CPPUNIT_TEST_SUITE(RtfModelTest);
#if !defined(MACOSX) && !defined(WNT)
    CPPUNIT_TEST(testFdo45553);
    CPPUNIT_TEST(testN192129);
    CPPUNIT_TEST(testFdo45543);
    CPPUNIT_TEST(testN695479);
    CPPUNIT_TEST(testFdo42465);
    CPPUNIT_TEST(testFdo45187);
    CPPUNIT_TEST(testFdo46662);
    CPPUNIT_TEST(testN750757);
    CPPUNIT_TEST(testFdo45563);
    CPPUNIT_TEST(testFdo43965);
    CPPUNIT_TEST(testN751020);
    CPPUNIT_TEST(testFdo47326);
    CPPUNIT_TEST(testFdo47036);
    CPPUNIT_TEST(testFdo46955);
    CPPUNIT_TEST(testFdo45394);
    CPPUNIT_TEST(testFdo48104);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load an RTF file and make the document available via mxComponent.
    void load(const OUString& rURL);
    /// Get the length of the whole document.
    int getLength();
    /// Get page count.
    int getPages();
    uno::Reference<lang::XComponent> mxComponent;
};

void RtfModelTest::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/rtftok/data/") + rFilename);
}

int RtfModelTest::getLength()
{
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
    return aBuf.getLength();
}

int RtfModelTest::getPages()
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    return xCursor->getPage();
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
    CPPUNIT_ASSERT_EQUAL(5, getLength());
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

void RtfModelTest::testFdo42465()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo42465.rtf")));
    CPPUNIT_ASSERT_EQUAL(3, getLength());
}

void RtfModelTest::testFdo45187()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo45187.rtf")));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be two shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    // They should be anchored to different paragraphs.
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Any aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorPosition")));
    awt::Point aFirstPoint;
    aValue >>= aFirstPoint;
    xPropertySet.set(xDraws->getByIndex(1), uno::UNO_QUERY);
    aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorPosition")));
    awt::Point aSecondPoint;
    aValue >>= aSecondPoint;
    CPPUNIT_ASSERT(aFirstPoint.Y != aSecondPoint.Y);
}

void RtfModelTest::testFdo46662()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo46662.rtf")));

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyles(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xNumberingStyles(xStyles->getByName(OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingStyles"))), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xNumberingStyles->getByName(OUString(RTL_CONSTASCII_USTRINGPARAM("WWNum3"))), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingRules"))), uno::UNO_QUERY);
    uno::Any aValue = xLevels->getByIndex(1); // 2nd level
    uno::Sequence<beans::PropertyValue> aProps;
    aValue >>= aProps;

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ParentNumbering")))
        {
            sal_Int16 nValue;
            rProp.Value >>= nValue;
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nValue);
        }
        else if (rProp.Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Suffix")))
        {
            rtl::OUString sValue;
            rProp.Value >>= sValue;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), sValue.getLength());
        }
    }
}

void RtfModelTest::testN750757()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("n750757.rtf")));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Bool bValue;
    uno::Any aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaContextMargin")));
    aValue >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(false), bValue);

    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    aValue = xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaContextMargin")));
    aValue >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), bValue);
}

void RtfModelTest::testFdo45563()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo45563.rtf")));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    int i = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        i++;
    }
    CPPUNIT_ASSERT_EQUAL(4, i);
}

void RtfModelTest::testFdo43965()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo43965.rtf")));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // First paragraph: the parameter of \up was ignored
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue;
    xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapement"))) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58), nValue);
    xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharEscapementHeight"))) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);

    // Second paragraph: Word vs Writer border default problem
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    table::BorderLine2 aBorder;
    xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TopBorder"))) >>= aBorder;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), aBorder.LineWidth);

    // Finally, make sure that we have two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void RtfModelTest::testN751020()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("n751020.rtf")));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParaBottomMargin"))) >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(200)), nValue);
}

void RtfModelTest::testFdo47326()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo47326.rtf")));
    // This was 15 only, as \super buffered text, then the contents of it got lost.
    CPPUNIT_ASSERT_EQUAL(19, getLength());
}

void RtfModelTest::testFdo47036()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo47036.rtf")));

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAtCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(i), uno::UNO_QUERY);
        text::TextContentAnchorType eValue;
        xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AnchorType"))) >>= eValue;
        if (eValue == text::TextContentAnchorType_AT_CHARACTER)
            nAtCharacter++;
    }
    // The image at the document start was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAtCharacter);

    // There should be 2 textboxes, not 4
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

void RtfModelTest::testFdo46955()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo46955.rtf")));

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
            sal_Int16 nValue;
            xPropertySet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharCaseMap"))) >>= nValue;
            CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, nValue);
        }
    }
}

void RtfModelTest::testFdo45394()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo45394.rtf")));

    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyles(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xPageStyles(xStyles->getByName("PageStyles"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xPageStyles->getByName("Default"), uno::UNO_QUERY);
    uno::Reference<text::XText> xHeaderText(xPropertySet->getPropertyValue("HeaderText"), uno::UNO_QUERY);
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected("ПК РИК", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

void RtfModelTest::testFdo48104()
{
    load(OUString(RTL_CONSTASCII_USTRINGPARAM("fdo48104.rtf")));
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_SUITE_REGISTRATION(RtfModelTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
