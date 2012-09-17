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

#include "../swmodeltestbase.hxx"
#include "bordertest.hxx"

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

#define TWIP_TO_MM100(TWIP) ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;

class Test : public SwModelTestBase
{
public:
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
    void testFdo47107();
    void testFdo45182();
    void testFdo44176();
    void testFdo39053();
    void testFdo48356();
    void testFdo48023();
    void testFdo48876();
    void testFdo48193();
    void testFdo44211();
    void testFdo48037();
    void testFdo47764();
    void testFdo38786();
    void testN757651();
    void testFdo49501();
    void testFdo49271();
    void testFdo49692();
    void testFdo45190();
    void testFdo50539();
    void testFdo50665();
    void testFdo49659();
    void testFdo46966();
    void testFdo52066();
    void testFdo48033();
    void testFdo36089();
    void testFdo49892();
    void testFdo48446();
    void testFdo47495();
    void testAllGapsWord();
    void testFdo52052();

    CPPUNIT_TEST_SUITE(Test);
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
    CPPUNIT_TEST(testFdo47107);
    CPPUNIT_TEST(testFdo45182);
    CPPUNIT_TEST(testFdo44176);
    CPPUNIT_TEST(testFdo39053);
    CPPUNIT_TEST(testFdo48356);
    CPPUNIT_TEST(testFdo48023);
    CPPUNIT_TEST(testFdo48876);
    CPPUNIT_TEST(testFdo48193);
    CPPUNIT_TEST(testFdo44211);
    CPPUNIT_TEST(testFdo48037);
    CPPUNIT_TEST(testFdo47764);
    CPPUNIT_TEST(testFdo38786);
    CPPUNIT_TEST(testN757651);
    CPPUNIT_TEST(testFdo49501);
    CPPUNIT_TEST(testFdo49271);
    CPPUNIT_TEST(testFdo49692);
    CPPUNIT_TEST(testFdo45190);
    CPPUNIT_TEST(testFdo50539);
    CPPUNIT_TEST(testFdo50665);
    CPPUNIT_TEST(testFdo49659);
    CPPUNIT_TEST(testFdo46966);
    CPPUNIT_TEST(testFdo52066);
    CPPUNIT_TEST(testFdo48033);
    CPPUNIT_TEST(testFdo36089);
    CPPUNIT_TEST(testFdo49892);
    CPPUNIT_TEST(testFdo48446);
    CPPUNIT_TEST(testFdo47495);
    CPPUNIT_TEST(testAllGapsWord);
    CPPUNIT_TEST(testFdo52052);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
    /// Load an RTF file and make the document available via mxComponent.
    void load(const OUString& rURL);
    /// Get page count.
    int getPages();
};

void Test::load(const OUString& rFilename)
{
    mxComponent = loadFromDesktop(getURLFromSrc("/sw/qa/extras/rtfimport/data/") + rFilename);
}

int Test::getPages()
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    return xCursor->getPage();
}

void Test::testFdo45553()
{
    load("fdo45553.rtf");

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
            if ( aStr == "space-before" )
            {
                sal_Int32 nMargin = 0;
                uno::Reference<beans::XPropertySet> xPropertySet(xRange, uno::UNO_QUERY);
                xPropertySet->getPropertyValue("ParaTopMargin") >>= nMargin;
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(120)), nMargin);
            }
            else if ( aStr == "space-after" )
            {
                sal_Int32 nMargin = 0;
                uno::Reference<beans::XPropertySet> xPropertySet(xRange, uno::UNO_QUERY);
                xPropertySet->getPropertyValue("ParaBottomMargin") >>= nMargin;
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(240)), nMargin);
            }
        }
    }
}

void Test::testN192129()
{
    load("n192129.rtf");

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

void Test::testFdo45543()
{
    load("fdo45543.rtf");
    CPPUNIT_ASSERT_EQUAL(5, getLength());
}

void Test::testN695479()
{
    load("n695479.rtf");

    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    sal_Int16 nSizeType = 0;
    xPropertySet->getPropertyValue("SizeType") >>= nSizeType;
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, nSizeType);
    sal_Int32 nHeight = 0;
    xPropertySet->getPropertyValue("Height") >>= nHeight;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(300)), nHeight);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    bool bFrameFound = false, bDrawFound = false;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xDraws->getByIndex(i), uno::UNO_QUERY);
        if (xServiceInfo->supportsService("com.sun.star.text.TextFrame"))
        {
            // Both frames should be anchored to the first paragraph.
            bFrameFound = true;
            uno::Reference<text::XTextContent> xTextContent(xServiceInfo, uno::UNO_QUERY);
            uno::Reference<text::XTextRange> xRange(xTextContent->getAnchor(), uno::UNO_QUERY);
            uno::Reference<text::XText> xText(xRange->getText(), uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(OUString("plain"), xText->getString());

            if (i == 0)
            {
                // Additonally, the frist frame should have double border at the bottom.
                table::BorderLine2 aBorder;
                xPropertySet->getPropertyValue("BottomBorder") >>= aBorder;
                CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::DOUBLE, aBorder.LineStyle);
            }
        }
        else if (xServiceInfo->supportsService("com.sun.star.drawing.LineShape"))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            sal_Int16 nHori = 0;
            xPropertySet->getPropertyValue("HoriOrientRelation") >>= nHori;
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, nHori);
            sal_Int16 nVert = 0;
            xPropertySet->getPropertyValue("VertOrientRelation") >>= nVert;
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, nVert);
        }
    }
    CPPUNIT_ASSERT(bFrameFound);
    CPPUNIT_ASSERT(bDrawFound);
}

void Test::testFdo42465()
{
    load("fdo42465.rtf");
    CPPUNIT_ASSERT_EQUAL(3, getLength());
}

void Test::testFdo45187()
{
    load("fdo45187.rtf");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be two shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
    // They should be anchored to different paragraphs.
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    awt::Point aFirstPoint;
    xPropertySet->getPropertyValue("AnchorPosition") >>= aFirstPoint;
    xPropertySet.set(xDraws->getByIndex(1), uno::UNO_QUERY);
    awt::Point aSecondPoint;
    xPropertySet->getPropertyValue("AnchorPosition") >>= aSecondPoint;
    CPPUNIT_ASSERT(aFirstPoint.Y != aSecondPoint.Y);
}

void Test::testFdo46662()
{
    load("fdo46662.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(1) >>= aProps; // 2nd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if ( rProp.Name == "ParentNumbering" )
        {
            sal_Int16 nValue;
            rProp.Value >>= nValue;
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), nValue);
        }
        else if ( rProp.Name == "Suffix" )
        {
            rtl::OUString sValue;
            rProp.Value >>= sValue;
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), sValue.getLength());
        }
    }
}

void Test::testN750757()
{
    load("n750757.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Bool bValue;
    xPropertySet->getPropertyValue("ParaContextMargin") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(false), bValue);

    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("ParaContextMargin") >>= bValue;
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), bValue);
}

void Test::testFdo45563()
{
    load("fdo45563.rtf");
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

void Test::testFdo43965()
{
    load("fdo43965.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // First paragraph: the parameter of \up was ignored
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue;
    xPropertySet->getPropertyValue("CharEscapement") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(36), nValue);
    xPropertySet->getPropertyValue("CharEscapementHeight") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), nValue);

    // Second paragraph: Word vs Writer border default problem
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    table::BorderLine2 aBorder;
    xPropertySet->getPropertyValue("TopBorder") >>= aBorder;
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), aBorder.LineWidth);

    // Finally, make sure that we have two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testN751020()
{
    load("n751020.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("ParaBottomMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(200)), nValue);
}

void Test::testFdo47326()
{
    load("fdo47326.rtf");
    // This was 15 only, as \super buffered text, then the contents of it got lost.
    CPPUNIT_ASSERT_EQUAL(19, getLength());
}

void Test::testFdo47036()
{
    load("fdo47036.rtf");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAtCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(i), uno::UNO_QUERY);
        text::TextContentAnchorType eValue;
        xPropertySet->getPropertyValue("AnchorType") >>= eValue;
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

void Test::testFdo46955()
{
    load("fdo46955.rtf");

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
            xPropertySet->getPropertyValue("CharCaseMap") >>= nValue;
            CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, nValue);
        }
    }
}

void Test::testFdo45394()
{
    load("fdo45394.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);
    uno::Reference<text::XText> xHeaderText(xPropertySet->getPropertyValue("HeaderText"), uno::UNO_QUERY);
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected("ПК РИК", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

void Test::testFdo48104()
{
    load("fdo48104.rtf");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

void Test::testFdo47107()
{
    load("fdo47107.rtf");

    uno::Reference<container::XNameAccess> xNumberingStyles(getStyles("NumberingStyles"));
    // Make sure numbered and bullet legacy syntax is recognized, this used to throw a NoSuchElementException
    xNumberingStyles->getByName("WWNum1");
    xNumberingStyles->getByName("WWNum2");
}

void Test::testFdo45182()
{
    load("fdo45182.rtf");

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // Encoding in the footnote was wrong.
    OUString aExpected("živností", 10, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo44176()
{
    load("fdo44176.rtf");

    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName("Default"), uno::UNO_QUERY);
    sal_Int32 nFirstTop = 0, nDefaultTop = 0, nDefaultHeader = 0;
    xFirstPage->getPropertyValue("TopMargin") >>= nFirstTop;
    xDefault->getPropertyValue("TopMargin") >>= nDefaultTop;
    xDefault->getPropertyValue("HeaderHeight") >>= nDefaultHeader;
    CPPUNIT_ASSERT_EQUAL(nFirstTop, nDefaultTop + nDefaultHeader);
}

void Test::testFdo39053()
{
    load("fdo39053.rtf");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAsCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(i), uno::UNO_QUERY);
        text::TextContentAnchorType eValue;
        xPropertySet->getPropertyValue("AnchorType") >>= eValue;
        if (eValue == text::TextContentAnchorType_AS_CHARACTER)
            nAsCharacter++;
    }
    // The image in binary format was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAsCharacter);
}

void Test::testFdo48356()
{
    load("fdo48356.rtf");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    OUStringBuffer aBuf;
    int i = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        i++;
    }
    // The document used to be imported as two paragraphs.
    CPPUNIT_ASSERT_EQUAL(1, i);
}

void Test::testFdo48023()
{
    lang::Locale aLocale;
    aLocale.Language = "ru";
    AllSettings aSettings(Application::GetSettings());
    AllSettings aSavedSettings(aSettings);
    aSettings.SetLocale(aLocale);
    Application::SetSettings(aSettings);
    load("fdo48023.rtf");
    Application::SetSettings(aSavedSettings);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xTextRange(xRangeEnum->nextElement(), uno::UNO_QUERY);

    // Implicit encoding detection based on locale was missing
    OUString aExpected("Программист", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo48876()
{
    load("fdo48876.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    xPropertySet->getPropertyValue("ParaLineSpacing") >>= aSpacing;
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::MINIMUM, aSpacing.Mode);
}

void Test::testFdo48193()
{
    load("fdo48193.rtf");
    CPPUNIT_ASSERT_EQUAL(7, getLength());
}

void Test::testFdo44211()
{
    lang::Locale aLocale;
    aLocale.Language = "lt";
    AllSettings aSettings(Application::GetSettings());
    AllSettings aSavedSettings(aSettings);
    aSettings.SetLocale(aLocale);
    Application::SetSettings(aSettings);
    load("fdo44211.rtf");
    Application::SetSettings(aSavedSettings);

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xTextRange(xRangeEnum->nextElement(), uno::UNO_QUERY);

    OUString aExpected("ąčę", 6, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

void Test::testFdo48037()
{
    load("fdo48037.rtf");

    uno::Reference<util::XNumberFormatsSupplier> xNumberSupplier(mxComponent, uno::UNO_QUERY_THROW);
    lang::Locale aUSLocale, aFRLocale;
    aUSLocale.Language = "en";
    aFRLocale.Language = "fr";
    sal_Int32 nExpected = xNumberSupplier->getNumberFormats()->addNewConverted("d MMMM yyyy", aUSLocale, aFRLocale);

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
    sal_Int32 nActual = 0;
    xPropertySet->getPropertyValue("NumberFormat") >>= nActual;

    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
}

void Test::testFdo47764()
{
    load("fdo47764.rtf");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    // \cbpat with zero argument should mean the auto (-1) color, not a default color (black)
    xPropertySet->getPropertyValue("ParaBackColor") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), nValue);
}

void Test::testFdo38786()
{
    load("fdo38786.rtf");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \chpgn was ignored, so exception was thrown
    xFields->nextElement();
}

void Test::testN757651()
{
    load("n757651.rtf");

    // The bug was that due to buggy layout the text expanded to two pages.
    if (Application::GetDefaultDevice()->IsFontAvailable(OUString("Times New Roman")))
        CPPUNIT_ASSERT_EQUAL(1, getPages());
}

void Test::testFdo49501()
{
    load("fdo49501.rtf");

    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);

    sal_Bool bIsLandscape = sal_False;
    xStyle->getPropertyValue("IsLandscape") >>= bIsLandscape;
    CPPUNIT_ASSERT_EQUAL(sal_True, bIsLandscape);
    sal_Int32 nExpected(TWIP_TO_MM100(567));
    sal_Int32 nValue = 0;
    xStyle->getPropertyValue("LeftMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(nExpected, nValue);
    xStyle->getPropertyValue("RightMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(nExpected, nValue);
    xStyle->getPropertyValue("TopMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(nExpected, nValue);
    xStyle->getPropertyValue("BottomMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(nExpected, nValue);
}

void Test::testFdo49271()
{
    load("fdo49271.rtf");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    float fValue = 0;
    xPropertySet->getPropertyValue("CharHeight") >>= fValue;

    CPPUNIT_ASSERT_EQUAL(25.f, fValue);
}

void Test::testFdo49692()
{
    load("fdo49692.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
        {
            rtl::OUString sValue;
            rProp.Value >>= sValue;

            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), sValue.getLength());
        }
    }
}

void Test::testFdo45190()
{
    load("fdo45190.rtf");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // inherited \fi should be reset
    uno::Reference<beans::XPropertySet> xPropertySet(xParaEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nValue);

    // but direct one not
    xPropertySet.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPropertySet->getPropertyValue("ParaFirstLineIndent") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(-100)), nValue);
}

void Test::testFdo50539()
{
    load("fdo50539.rtf");

    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    // \chcbpat with zero argument should mean the auto (-1) color, not a default color (black)
    xPropertySet->getPropertyValue("CharBackColor") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), nValue);
}

void Test::testFdo50665()
{
    load("fdo50665.rtf");
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum(xParaEnumAccess->createEnumeration());
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum(xRunEnumAccess->createEnumeration());

    // Access the second run, which is a textfield
    xRunEnum->nextElement();
    uno::Reference<beans::XPropertySet> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    OUString aValue;
    xRun->getPropertyValue("CharFontName") >>= aValue;
    // This used to be the default, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("Book Antiqua"), aValue);
}

void Test::testFdo49659()
{
    load("fdo49659.rtf");

    // Both tables were ignored: 1) was in the header, 2) was ignored due to missing empty par at the end of the doc
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    // The graphic was also empty
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xGraphic;
    xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
    sal_Int8 nValue = 0;
    xGraphic->getPropertyValue("GraphicType") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, nValue);
}

void Test::testFdo46966()
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    load("fdo46966.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Default"), uno::UNO_QUERY);
    sal_Int32 nValue = 0;
    xPropertySet->getPropertyValue("TopMargin") >>= nValue;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(720)), nValue);
}

void Test::testFdo52066()
{
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    load("fdo52066.rtf");

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(19)), xShape->getSize().Height);
}

void Test::testFdo48033()
{
    /*
     * The problem was that the picture was in the first cell, instead of the second one.
     *
     * oTable = ThisComponent.TextTables(0)
     * oParas = oTable.getCellByName("B1").Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.TextPortionType ' Frame, was Text
     */
    load("fdo48033.rtf");
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
}

void Test::testFdo36089()
{
    load("fdo36089.rtf");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(-50), getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEscapement"));
}

void Test::testFdo49892()
{
    load("fdo49892.rtf");
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        OUString aDescription = getProperty<OUString>(xDraws->getByIndex(i), "Description");
        if (aDescription == "red")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "green")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "blue")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (aDescription == "rect")
        {
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(i), "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xDraws->getByIndex(i), "VertOrientRelation"));
        }
    }
}

void Test::testFdo48446()
{
    load("fdo48446.rtf");

    OUString aExpected("Имя", 6, RTL_TEXTENCODING_UTF8);
    getParagraph(1, aExpected);
}

void Test::testFdo47495()
{
    load("fdo47495.rtf");
    // Used to have 4 paragraphs, as a result the original bugdoc had 2 pages instead of 1.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

void Test::testAllGapsWord()
{
    load("all_gaps_word.rtf");
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}

void Test::testFdo52052()
{
    load("fdo52052.rtf");
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    CPPUNIT_ASSERT_EQUAL(OUString("third"), parseDump("/root/page[3]/body/txt/anchored/fly/txt/text()"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
