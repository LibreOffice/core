/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextGraphicObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/rtfimport/data/", "Rich Text Format")
    {
    }

    virtual void preTest(const char* filename) SAL_OVERRIDE
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename) == "fdo48023.rtf")
        {
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ru"));
            Application::SetSettings(aSettings);
        }
        else if (OString(filename) == "fdo44211.rtf")
        {
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("lt"));
            Application::SetSettings(aSettings);
        }
    }

    virtual void postTest(const char* filename) SAL_OVERRIDE
    {
        if (OString(filename) == "fdo48023.rtf" || OString(filename) == "fdo44211.rtf")
            Application::SetSettings(m_aSavedSettings);
    }

protected:
    /// Copy&paste helper.
    void paste(OUString aFilename, uno::Reference<text::XTextRange> xTextRange = uno::Reference<text::XTextRange>())
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        uno::Sequence<beans::PropertyValue> aDescriptor(xTextRange.is() ? 3 : 2);
        aDescriptor[0].Name = "InputStream";
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(getURLFromSrc("/sw/qa/extras/rtfimport/data/") + aFilename, STREAM_WRITE);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
        aDescriptor[0].Value <<= xStream;
        aDescriptor[1].Name = "InsertMode";
        aDescriptor[1].Value <<= sal_True;
        if (xTextRange.is())
        {
            aDescriptor[2].Name = "TextInsertModeRange";
            aDescriptor[2].Value <<= xTextRange;
        }
        xFilter->filter(aDescriptor);
    }

    AllSettings m_aSavedSettings;
};

#define DECLARE_RTFIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, Test)

#if !defined(MACOSX) && !defined(WNT)

DECLARE_RTFIMPORT_TEST(testFdo45553, "fdo45553.rtf")
{
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
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(120)), getProperty<sal_Int32>(xRange, "ParaTopMargin"));
            else if ( aStr == "space-after" )
                CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(240)), getProperty<sal_Int32>(xRange, "ParaBottomMargin"));
        }
    }
}

DECLARE_RTFIMPORT_TEST(testN192129, "n192129.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo45543, "fdo45543.rtf")
{
    CPPUNIT_ASSERT_EQUAL(5, getLength());
}

DECLARE_RTFIMPORT_TEST(testN695479, "n695479.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xIndexAccess->getByIndex(0), uno::UNO_QUERY);

    // Negative ABSH should mean fixed size.
    CPPUNIT_ASSERT_EQUAL(text::SizeType::FIX, getProperty<sal_Int16>(xPropertySet, "SizeType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(300)), getProperty<sal_Int32>(xPropertySet, "Height"));

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
                // Additionally, the frist frame should have double border at the bottom.
                CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::DOUBLE, getProperty<table::BorderLine2>(xPropertySet, "BottomBorder").LineStyle);
        }
        else if (xServiceInfo->supportsService("com.sun.star.drawing.LineShape"))
        {
            // The older "drawing objects" syntax should be recognized.
            bDrawFound = true;
            xPropertySet.set(xServiceInfo, uno::UNO_QUERY);
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xPropertySet, "HoriOrientRelation"));
            CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(xPropertySet, "VertOrientRelation"));
        }
    }
    CPPUNIT_ASSERT(bFrameFound);
    CPPUNIT_ASSERT(bDrawFound);
}

DECLARE_RTFIMPORT_TEST(testFdo42465, "fdo42465.rtf")
{
    CPPUNIT_ASSERT_EQUAL(3, getLength());
}

DECLARE_RTFIMPORT_TEST(testFdo45187, "fdo45187.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be two shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // They should be anchored to different paragraphs.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor0 = uno::Reference<text::XTextContent>(xDraws->getByIndex(0), uno::UNO_QUERY)->getAnchor();
    uno::Reference<text::XTextRange> xAnchor1 = uno::Reference<text::XTextContent>(xDraws->getByIndex(1), uno::UNO_QUERY)->getAnchor();
    // Was 0 ("starts at the same position"), should be 1 ("starts before")
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xTextRangeCompare->compareRegionStarts(xAnchor0, xAnchor1));
}

DECLARE_RTFIMPORT_TEST(testFdo46662, "fdo46662.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(1) >>= aProps; // 2nd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if ( rProp.Name == "ParentNumbering" )
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), rProp.Value.get<sal_Int16>());
        else if ( rProp.Name == "Suffix" )
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

DECLARE_RTFIMPORT_TEST(testN750757, "n750757.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    CPPUNIT_ASSERT_EQUAL(sal_Bool(false), getProperty<sal_Bool>(xParaEnum->nextElement(), "ParaContextMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Bool(true), getProperty<sal_Bool>(xParaEnum->nextElement(), "ParaContextMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo45563, "fdo45563.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo43965, "fdo43965.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // First paragraph: the parameter of \up was ignored
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(36), getProperty<sal_Int32>(xPropertySet, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(xPropertySet, "CharEscapementHeight"));

    // Second paragraph: Word vs Writer border default problem
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), getProperty<table::BorderLine2>(xParaEnum->nextElement(), "TopBorder").LineWidth);

    // Finally, make sure that we have two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testN751020, "n751020.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(200)), getProperty<sal_Int32>(xParaEnum->nextElement(), "ParaBottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo47326, "fdo47326.rtf")
{
    // This was 15 only, as \super buffered text, then the contents of it got lost.
    CPPUNIT_ASSERT_EQUAL(19, getLength());
}

DECLARE_RTFIMPORT_TEST(testFdo47036, "fdo47036.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAtCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType") == text::TextContentAnchorType_AT_CHARACTER)
            nAtCharacter++;
    }
    // The image at the document start was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAtCharacter);

    // There should be 2 textboxes, not 4
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo46955, "fdo46955.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
            CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE, getProperty<sal_Int16>(xRangeEnum->nextElement(), "CharCaseMap"));
    }
}

DECLARE_RTFIMPORT_TEST(testFdo45394, "fdo45394.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected("\xd0\x9f\xd0\x9a \xd0\xa0\xd0\x98\xd0\x9a", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo48104, "fdo48104.rtf")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo47107, "fdo47107.rtf")
{
    uno::Reference<container::XNameAccess> xNumberingStyles(getStyles("NumberingStyles"));
    // Make sure numbered and bullet legacy syntax is recognized, this used to throw a NoSuchElementException
    xNumberingStyles->getByName("WWNum1");
    xNumberingStyles->getByName("WWNum2");
}

DECLARE_RTFIMPORT_TEST(testFdo45182, "fdo45182.rtf")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // Encoding in the footnote was wrong.
    OUString aExpected("\xc5\xbeivnost\xc3\xad\n", 11, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo44176, "fdo44176.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles(getStyles("PageStyles"));
    uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    sal_Int32 nFirstTop = 0, nDefaultTop = 0, nDefaultHeader = 0;
    xFirstPage->getPropertyValue("TopMargin") >>= nFirstTop;
    xDefault->getPropertyValue("TopMargin") >>= nDefaultTop;
    xDefault->getPropertyValue("HeaderHeight") >>= nDefaultHeader;
    CPPUNIT_ASSERT_EQUAL(nFirstTop, nDefaultTop + nDefaultHeader);
}

DECLARE_RTFIMPORT_TEST(testFdo39053, "fdo39053.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    int nAsCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType") == text::TextContentAnchorType_AS_CHARACTER)
            nAsCharacter++;
    // The image in binary format was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAsCharacter);
}

DECLARE_RTFIMPORT_TEST(testFdo48356, "fdo48356.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    int i = 0;
    while (xParaEnum->hasMoreElements())
    {
        xParaEnum->nextElement();
        i++;
    }
    // The document used to be imported as two paragraphs.
    CPPUNIT_ASSERT_EQUAL(1, i);
}

DECLARE_RTFIMPORT_TEST(testFdo48023, "fdo48023.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    // Implicit encoding detection based on locale was missing
    OUString aExpected("\xd0\x9f\xd1\x80\xd0\xbe\xd0\xb3\xd1\x80\xd0\xb0\xd0\xbc\xd0\xbc\xd0\xb8\xd1\x81\xd1\x82", 22, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo48876, "fdo48876.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::MINIMUM, getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing").Mode);
}

DECLARE_RTFIMPORT_TEST(testFdo48193, "fdo48193.rtf")
{
    CPPUNIT_ASSERT_EQUAL(7, getLength());
}

DECLARE_RTFIMPORT_TEST(testFdo44211, "fdo44211.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected("\xc4\x85\xc4\x8d\xc4\x99", 6, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo48037, "fdo48037.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo47764, "fdo47764.rtf")
{
    // \cbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getParagraph(1), "ParaBackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo38786, "fdo38786.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \chpgn was ignored, so exception was thrown
    xFields->nextElement();
}

DECLARE_RTFIMPORT_TEST(testN757651, "n757651.rtf")
{
    // The bug was that due to buggy layout the text expanded to two pages.
    if (Application::GetDefaultDevice()->IsFontAvailable(OUString("Times New Roman")))
        CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo49501, "fdo49501.rtf")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(sal_True, getProperty<sal_Bool>(xStyle, "IsLandscape"));
    sal_Int32 nExpected(TWIP_TO_MM100(567));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo49271, "fdo49271.rtf")
{
    CPPUNIT_ASSERT_EQUAL(25.f, getProperty<float>(getParagraph(2), "CharHeight"));
}

DECLARE_RTFIMPORT_TEST(testFdo49692, "fdo49692.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

DECLARE_RTFIMPORT_TEST(testFdo45190, "fdo45190.rtf")
{
    // inherited \fi should be reset
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(-100)), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
}

DECLARE_RTFIMPORT_TEST(testFdo50539, "fdo50539.rtf")
{
    // \chcbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharBackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo50665, "fdo50665.rtf")
{
    // Access the second run, which is a textfield
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
    // This used to be the default, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("Book Antiqua"), getProperty<OUString>(xRun, "CharFontName"));
}

DECLARE_RTFIMPORT_TEST(testFdo49659, "fdo49659.rtf")
{
    // Both tables were ignored: 1) was in the header, 2) was ignored due to missing empty par at the end of the doc
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    // The graphic was also empty
    uno::Reference<beans::XPropertySet> xGraphic(getProperty< uno::Reference<beans::XPropertySet> >(getShape(1), "Graphic"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, getProperty<sal_Int8>(xGraphic, "GraphicType"));
}

DECLARE_RTFIMPORT_TEST(testFdo46966, "fdo46966.rtf")
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(720)), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo52066, "fdo52066.rtf")
{
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(19)), xShape->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testFdo48033, "fdo48033.rtf")
{
    /*
     * The problem was that the picture (48033) or OLE object (53594) was in the first cell,
     * instead of the second one.
     *
     * oTable = ThisComponent.TextTables(0)
     * oParas = oTable.getCellByName("B1").Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.TextPortionType ' Frame, was Text
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testFdo53594, "fdo53594.rtf")
{
    /*
     * The problem was that the picture (48033) or OLE object (53594) was in the first cell,
     * instead of the second one.
     *
     * oTable = ThisComponent.TextTables(0)
     * oParas = oTable.getCellByName("B1").Text.createEnumeration
     * oPara = oParas.nextElement
     * oRuns = oPara.createEnumeration
     * oRun = oRuns.nextElement
     * xray oRun.TextPortionType ' Frame, was Text
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testFdo36089, "fdo36089.rtf")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(-50), getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEscapement"));
}

DECLARE_RTFIMPORT_TEST(testFdo49892, "fdo49892.rtf")
{
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

DECLARE_RTFIMPORT_TEST(testFdo48446, "fdo48446.rtf")
{
    OUString aExpected("\xd0\x98\xd0\xbc\xd1\x8f", 6, RTL_TEXTENCODING_UTF8);
    getParagraph(1, aExpected);
}

DECLARE_RTFIMPORT_TEST(testFdo47495, "fdo47495.rtf")
{
    // Used to have 4 paragraphs, as a result the original bugdoc had 2 pages instead of 1.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

DECLARE_RTFIMPORT_TEST(testAllGapsWord, "all_gaps_word.rtf")
{
    BorderTest borderTest;
    borderTest.testTheBorders(mxComponent);
}

DECLARE_RTFIMPORT_TEST(testFdo52052, "fdo52052.rtf")
{
    // Make sure the textframe containing the text "third" appears on the 3rd page.
    CPPUNIT_ASSERT_EQUAL(OUString("third"), parseDump("/root/page[3]/body/txt/anchored/fly/txt/text()"));
}

DECLARE_RTFIMPORT_TEST(testInk, "ink.rtf")
{
    /*
     * The problem was that the second segment had wrong command count and wrap type.
     *
     * oShape = ThisComponent.DrawPage(0)
     * oPathPropVec = oShape.CustomShapeGeometry(1).Value
     * oSegments = oPathPropVec(1).Value
     * msgbox oSegments(1).Count ' was 0x2000 | 10, should be 10
     * msgbox oShape.Surround ' was 2, should be 1
     */
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            rProp.Value >>= aPathProps;
    }
    uno::Sequence<drawing::EnhancedCustomShapeSegment> aSegments;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Segments")
            rProp.Value >>= aSegments;
    }
    CPPUNIT_ASSERT_EQUAL(sal_Int16(10), aSegments[1].Count);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
}

DECLARE_RTFIMPORT_TEST(testFdo52389, "fdo52389.rtf")
{
    // The last '!' character at the end of the document was lost
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

DECLARE_RTFIMPORT_TEST(testFdo49655, "fdo49655.rtf")
{
    /*
     * 49655 :
     * The problem was that the table was not imported due to the '  ' string in the middle of the table definition.
     *
     * xray ThisComponent.TextTables.Count 'was 0
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo62805, "fdo62805.rtf")
{
    /*
     * 62805 :
     * The problem was that the table was not imported due to the absence of \pard after \row.
     * The table was instead in a group (the '}' replace the \pard).
     */
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo52475, "fdo52475.rtf")
{
    // The problem was that \chcbpat0 resulted in no color, instead of COL_AUTO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 3), "CharBackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo55493, "fdo55493.rtf")
{
    // The problem was that the width of the PNG was detected as 15,24cm, instead of 3.97cm
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3969), xShape->getSize().Width);
}

DECLARE_RTFIMPORT_TEST(testCopyPastePageStyle, "copypaste-pagestyle.rtf")
{
    // The problem was that RTF import during copy&paste did not ignore page styles.
    // Once we have more copy&paste tests, makes sense to refactor this to some helper method.
    paste("copypaste-pagestyle-paste.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), getProperty<sal_Int32>(xPropertySet, "Width")); // Was letter, i.e. 21590
}

DECLARE_RTFIMPORT_TEST(testCopyPasteFootnote, "copypaste-footnote.rtf")
{
    // The RTF import did not handle the case when the position wasn't the main document XText, but something different, e.g. a footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    paste("copypaste-footnote-paste.rtf", xTextRange);

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo61193, "hello.rtf")
{
    // Pasting content that contained a footnote caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo61193.rtf", xEnd);
}

DECLARE_RTFIMPORT_TEST(testShptxtPard, "shptxt-pard.rtf")
{
    // The problem was that \pard inside \shptxt caused loss of shape text
    uno::Reference<text::XText> xText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("shape text"), xText->getString());
}

DECLARE_RTFIMPORT_TEST(testDoDhgt, "do-dhgt.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        sal_Int32 nFillColor = getProperty<sal_Int32>(xDraws->getByIndex(i), "FillColor");
        if (nFillColor == 0xc0504d) // red
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x9bbb59) // green
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x4f81bd) // blue
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
    }
}

DECLARE_RTFIMPORT_TEST(testDplinehollow, "dplinehollow.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(drawing::LineStyle_NONE, getProperty<drawing::LineStyle>(xPropertySet, "LineStyle"));
}

DECLARE_RTFIMPORT_TEST(testLeftmarginDefault, "leftmargin-default.rtf")
{
    // The default left/right margin was incorrect when the top margin was set to zero.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), getProperty<sal_Int32>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "LeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testDppolyline, "dppolyline.rtf")
{
    // This was completely ignored, for now, just make sure we have all 4 lines.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo56512, "fdo56512.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    OUString aExpected("\xd7\xa2\xd7\x95\xd7\xa1\xd7\xa7 \xd7\x9e\xd7\x95\xd7\xa8\xd7\xa9\xd7\x94 ", 20, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo52989, "fdo52989.rtf")
{
    // Same as n#192129, but for JPEG files.
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(423), xShape->getSize().Width);
}

DECLARE_RTFIMPORT_TEST(testFdo48442, "fdo48442.rtf")
{
    // The problem was that \pvmrg is the default in RTF, but not in Writer.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_PRINT_AREA, getProperty<sal_Int16>(xShape, "VertOrientRelation")); // was FRAME
}

DECLARE_RTFIMPORT_TEST(testFdo55525, "fdo55525.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Negative left margin was ~missing, -191
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1877), getProperty<sal_Int32>(xTable, "LeftMargin"));
    // Cell width of A1 was 3332 (e.g. not set, 30% percent of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(896), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFIMPORT_TEST(testFdo57708, "fdo57708.rtf")
{
    // There were two issues: the doc was of 2 pages and the picture was missing.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Two objects: a picture and a textframe.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo54473, "fdo54473.rtf")
{
    // The problem was that character styles were not imported due to a typo.
    CPPUNIT_ASSERT_EQUAL(OUString("Anot"), getProperty<OUString>(getRun(getParagraph(1), 1, "Text "), "CharStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("ForeignTxt"), getProperty<OUString>(getRun(getParagraph(1), 3, "character "), "CharStyleName"));
}

DECLARE_RTFIMPORT_TEST(testFdo49934, "fdo49934.rtf")
{
    // Column break without columns defined should be a page break, but it was just ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo57886, "fdo57886.rtf")
{
    // Was 'int from <?> to <?> <?>'.
    CPPUNIT_ASSERT_EQUAL(OUString("int from {firstlower} to {firstupper} {firstbody}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_RTFIMPORT_TEST(testFdo58076, "fdo58076.rtf")
{
    // An additional section was created, so the default page style didn't have the custom margins.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2251), getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1752), getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo57678, "fdo57678.rtf")
{
    // Paragraphs of the two tables were not converted to tables.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo45183, "fdo45183.rtf")
{
    // Was text::WrapTextMode_PARALLEL, i.e. shpfblwtxt didn't send the shape below text.
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Was 247, resulting in a table having width almost zero and height of 10+ pages.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16237), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_RTFIMPORT_TEST(testFdo54612, "fdo54612.rtf")
{
    // \dpptx without a \dppolycount caused a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo58933, "fdo58933.rtf")
{
    // The problem was that the table had an additional cell in its first line.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 4.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getCellNames().getLength());
}

DECLARE_RTFIMPORT_TEST(testFdo44053, "fdo44053.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The with of the table's A1 and A2 cell should equal.
    CPPUNIT_ASSERT_EQUAL(getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position,
            getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFIMPORT_TEST(testFdo48440, "fdo48440.rtf")
{
    // Page break was ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo58646line, "fdo58646line.rtf")
{
    // \line symbol was ignored
    getParagraph(1, "foo\nbar");
}

DECLARE_RTFIMPORT_TEST(testFdo58646, "fdo58646.rtf")
{
    // Page break was ignored inside a continuous section, on title page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo59419, "fdo59419.rtf")
{
    // Junk to be ignored broke import of the table.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo58076_2, "fdo58076-2.rtf")
{
    // Position of the picture wasn't correct.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(8345)), getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
}

DECLARE_RTFIMPORT_TEST(testFdo59953, "fdo59953.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A1 was 4998 (e.g. not set / not wide enough, ~50% of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(7649), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFIMPORT_TEST(testFdo59638, "fdo59638.rtf")
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletChar")
        {
            // Was '*', should be 'o'.
            CPPUNIT_ASSERT_EQUAL(OUString("\xEF\x82\xB7", 3, RTL_TEXTENCODING_UTF8), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_RTFIMPORT_TEST(testFdo60722, "fdo60722.rtf")
{
    // The problem was that the larger shape was over the smaller one, and not the other way around.
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("larger"), getProperty<OUString>(xShape, "Description"));

    xShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("smaller"), getProperty<OUString>(xShape, "Description"));

    // Color of the line was blue, and it had zero width.
    xShape.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(26), getProperty<sal_uInt32>(xShape, "LineWidth"));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), getProperty<sal_uInt32>(xShape, "LineColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo61909, "fdo61909.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
    // Was the Writer default font.
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"), getProperty<OUString>(xTextRange, "CharFontName"));
    // Was 0x008000.
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(xTextRange, "CharBackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo62288, "fdo62288.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Margins were inherited from the previous cell, even there was a \pard there.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo37716, "fdo37716.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    // \nowrap got ignored, so Surround was text::WrapTextMode_PARALLEL
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(xFrames->getByIndex(0), "Surround"));
}

DECLARE_RTFIMPORT_TEST(testFdo51916, "fdo51916.rtf")
{
    // Complex nested table caused a crash.
}

DECLARE_RTFIMPORT_TEST(testFdo63023, "fdo63023.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "HeaderText");
    // Back color was black (0) in the header, due to missing color table in the substream.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF99), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xHeaderText), 1), "CharBackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo42109, "fdo42109.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // Make sure the page number is imported as a field in the B1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testFdo62977, "fdo62977.rtf")
{
    // The middle character was imported as '?' instead of the proper unicode value.
    getRun(getParagraph(1), 1, OUString("\xE5\xB9\xB4\xEF\xBC\x94\xE6\x9C\x88", 9, RTL_TEXTENCODING_UTF8));
}

DECLARE_RTFIMPORT_TEST(testN818997, "n818997.rtf")
{
    // \page was ignored between two \shp tokens.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo64671, "fdo64671.rtf")
{
    // Additional '}' was inserted before the special character.
    getRun(getParagraph(1), 1, OUString("\xC5\xBD", 2, RTL_TEXTENCODING_UTF8));
}

DECLARE_RTFIMPORT_TEST(testPageBackground, "page-background.rtf")
{
    // The problem was that \background was ignored.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName(DEFAULT_STYLE), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x92D050), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo62044, "fdo62044.rtf")
{
    // The problem was that RTF import during copy&paste did not ignore existing paragraph styles.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo62044-paste.rtf", xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(xPropertySet, "CharHeight")); // Was 18, i.e. reset back to original value.
}

DECLARE_RTFIMPORT_TEST(testPoshPosv, "posh-posv.rtf")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(getShape(1), "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "FrameIsAutomaticHeight"));
}

DECLARE_RTFIMPORT_TEST(testN825305, "n825305.rtf")
{
    // The problem was that the textbox wasn't transparent, due to unimplemented fFilled == 0.
    uno::Reference<beans::XPropertyState> xPropertyState(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getShape(2), "BackColorTransparency"));
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("BackColorTransparency");
    // Was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
}

DECLARE_RTFIMPORT_TEST(testParaBottomMargin, "para-bottom-margin.rtf")
{
    // This was 353, i.e. bottom margin of the paragraph was 0.35cm instead of 0.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testN823655, "n823655.rtf")
{
    uno::Sequence<beans::PropertyValue> aProps = getProperty< uno::Sequence<beans::PropertyValue> >(getShape(1), "CustomShapeGeometry");
    uno::Sequence<beans::PropertyValue> aPathProps;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];
        if (rProp.Name == "Path")
            aPathProps = rProp.Value.get< uno::Sequence<beans::PropertyValue> >();
    }
    uno::Sequence<drawing::EnhancedCustomShapeParameterPair> aCoordinates;
    for (int i = 0; i < aPathProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aPathProps[i];
        if (rProp.Name == "Coordinates")
            aCoordinates = rProp.Value.get< uno::Sequence<drawing::EnhancedCustomShapeParameterPair> >();
    }
    // The first coordinate pair of this freeform shape was 286,0 instead of 0,286.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(286), aCoordinates[0].Second.Value.get<sal_Int32>());
}

DECLARE_RTFIMPORT_TEST(testFdo66040, "fdo66040.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // This was 0 (no shapes were imported), we want two textframes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // The second paragraph of the first shape should be actually a table, with "A" in its A1 cell.
    uno::Reference<text::XTextRange> xTextRange(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A"), uno::Reference<text::XTextRange>(xTable->getCellByName("A1"), uno::UNO_QUERY)->getString());

    // Make sure the second shape has the correct position and size.
    uno::Reference<drawing::XShape> xShape(xDraws->getByIndex(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14420), getProperty<sal_Int32>(xShape, "HoriOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1032), getProperty<sal_Int32>(xShape, "VertOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(14000), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), xShape->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testN823675, "n823675.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    awt::FontDescriptor aFont;

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "BulletFont")
            aFont = rProp.Value.get<awt::FontDescriptor>();
    }
    // This was empty, i.e. no font name was set for the bullet numbering.
    CPPUNIT_ASSERT_EQUAL(OUString("Symbol"), aFont.Name);
}

DECLARE_RTFIMPORT_TEST(testFdo47802, "fdo47802.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Shape inside table was ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo39001, "fdo39001.rtf")
{
    // Document was of 4 pages, \sect at the end of the doc wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_RTFIMPORT_TEST(testGroupshape, "groupshape.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
    uno::Reference<drawing::XShapes> xGroupshape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

DECLARE_RTFIMPORT_TEST(testGroupshape_notext, "groupshape-notext.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // There should be a single groupshape with 2 children.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
    uno::Reference<drawing::XShapes> xGroupshape(xDraws->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xGroupshape->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo66565, "fdo66565.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of A2 was 554, should be 453/14846*10000
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(304), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFIMPORT_TEST(testFdo54900, "fdo54900.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

DECLARE_RTFIMPORT_TEST(testFdo64637, "fdo64637.rtf")
{
    // The problem was that the custom "Company" property was added twice, the second invocation resulted in an exception.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDocumentPropertiesSupplier->getDocumentProperties()->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getProperty<OUString>(xPropertySet, "Company"));
}

DECLARE_RTFIMPORT_TEST(testN820504, "n820504.rtf")
{
    // The shape was anchored at-page instead of at-character (that's incorrect as Word only supports at-character and as-character).
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFIMPORT_TEST(testFdo67365, "fdo67365.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xRows = xTable->getRows();
    // The table only had 3 rows.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xRows->getCount());
    // This was 4999, i.e. the two cells of the row had equal widths instead of a larger and a smaller cell.
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5290), getProperty< uno::Sequence<text::TableColumnSeparator> >(xRows->getByIndex(2), "TableColumnSeparators")[0].Position);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Paragraph was aligned to center, should be left.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

DECLARE_RTFIMPORT_TEST(testFdo67498, "fdo67498.rtf")
{
    // Left margin of the default page style wasn't set (was 2000).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(TWIP_TO_MM100(5954)), getProperty<sal_Int32>(getStyles("PageStyles")->getByName(DEFAULT_STYLE), "LeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo47440, "fdo47440.rtf")
{
    // Vertical and horizontal orientation of the picture wasn't imported (was text::RelOrientation::FRAME).
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "VertOrientRelation"));
}

DECLARE_RTFIMPORT_TEST(testFdo53556, "fdo53556.rtf")
{
    // This was drawing::FillStyle_SOLID, which resulted in being non-transparent, hiding text which would be visible.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getShape(3), "FillStyle"));

    // This was a com.sun.star.drawing.CustomShape, which resulted in lack of word wrapping in the bugdoc.
    uno::Reference<beans::XPropertySet> xShapeProperties(getShape(1), uno::UNO_QUERY);
    uno::Reference<drawing::XShapeDescriptor> xShapeDescriptor(xShapeProperties, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("FrameShape"), xShapeDescriptor->getShapeType());
}

DECLARE_RTFIMPORT_TEST(testFdo63428, "hello.rtf")
{
    // Pasting content that contained an annotation caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo63428.rtf", xEnd);

    // Additionally, commented range was imported as a normal comment.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"), getProperty<OUString>(getRun(getParagraph(1), 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"), getProperty<OUString>(getRun(getParagraph(1), 4), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testGroupshapeRotation, "groupshape-rotation.rtf")
{
    // Rotation on groupshapes wasn't handled correctly, RotateAngle was 4500.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(315 * 100), getProperty<sal_Int32>(getShape(1), "RotateAngle"));
}

DECLARE_RTFIMPORT_TEST(testFdo44715, "fdo44715.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Style information wasn't reset, which caused character height to be 16.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraphOfText(2, xCell->getText()), "CharHeight"));
}

DECLARE_RTFIMPORT_TEST(testFdo68076, "fdo68076.rtf")
{
    // Encoding of the last char was wrong (more 'o' than 'y').
    OUString aExpected("\xD0\x9E\xD0\xB1\xD1\x8A\xD0\xB5\xD0\xBA\xD1\x82 \xE2\x80\x93 \xD1\x83", 19, RTL_TEXTENCODING_UTF8);
    getParagraph(1, aExpected);
}

DECLARE_RTFIMPORT_TEST(testFdo68291, "fdo68291.odt")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo68291-paste.rtf", xEnd);

    // This was "Standard", causing an unwanted page break on next paste.
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "PageDescName"));
}

DECLARE_RTFIMPORT_TEST(testFdo69384, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo69384-paste.rtf", xEnd);

    // Import got interrupted in the middle of style sheet table import,
    // resuling in missing styles and text.
    getStyles("ParagraphStyles")->getByName("Text body justified");
}

DECLARE_RTFIMPORT_TEST(testFdo70221, "fdo70221.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testCp1000018, "cp1000018.rtf")
{
    // The problem was that the empty paragraph at the end of the footnote got
    // lost during import.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString aExpected("Footnote first line.\n");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

#endif

DECLARE_RTFIMPORT_TEST(testNestedTable, "rhbz1065629.rtf")
{
    // nested table in second cell was missing
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Responsable Commercial:"), xPara->getString());
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Nom: John Doe"), xPara->getString());

    // outer table: background color, borders for B1/B2 cell
    xTable.set(xTables->getByIndex(2), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 fullPtSolid(
            1, 0, 35, 0, table::BorderLineStyle::SOLID, 35);
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
        getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
        getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
        getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
        getProperty<table::BorderLine2>(xCell, "BottomBorder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xCC0000), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 halfPtSolid(
            /*0*/1, 0, 18, 0, table::BorderLineStyle::SOLID, 18);
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
        getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffffff),
            getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("B2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
        getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
        getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffffff),
            getProperty<sal_Int32>(xCell, "BackColor"));

    // \sect at the end resulted in spurious page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFIMPORT_TEST(testContSectionPageBreak, "cont-section-pagebreak.rtf")
{
    uno::Reference<text::XTextRange> xParaSecond = getParagraph(2);
    CPPUNIT_ASSERT_EQUAL(OUString("SECOND"), xParaSecond->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
            getProperty<style::BreakType>(xParaSecond, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(OUString(""),
            getProperty<OUString>(xParaSecond, "PageDescName"));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(""), xParaNext->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Converted1"),
            getProperty<OUString>(xParaNext, "PageDescName"));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("THIRD"), xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
            getProperty<style::BreakType>(xParaThird, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(OUString(""),
            getProperty<OUString>(xParaThird, "PageDescName"));

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFooterPara, "footer-para.rtf")
{
    // check that paragraph properties in footer are imported
    uno::Reference<text::XText> xFooterText =
        getProperty< uno::Reference<text::XText> >(
            getStyles("PageStyles")->getByName("First Page"), "FooterText");
    uno::Reference<text::XTextContent> xParagraph =
        getParagraphOrTable(1, xFooterText);
    CPPUNIT_ASSERT_EQUAL(OUString("All Rights Reserved."),
        uno::Reference<text::XTextRange>(xParagraph, uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL((sal_Int16)style::ParagraphAdjust_CENTER,
        getProperty</*style::ParagraphAdjust*/sal_Int16>(xParagraph, "ParaAdjust"));
}

DECLARE_RTFIMPORT_TEST(testCp1000016, "hello.rtf")
{
    // The single-line document had a second fake empty para on Windows.
    bool bFound = true;
    try
    {
        getParagraph(2);
    }
    catch (const container::NoSuchElementException&)
    {
        bFound = false;
    }
    CPPUNIT_ASSERT_EQUAL(false, bFound);
}

DECLARE_RTFIMPORT_TEST(testFdo65090, "fdo65090.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables( ), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had 3 cells, instead of a horizontally merged one and a normal one (2 -> 1 separator).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFIMPORT_TEST(testShpzDhgt, "shpz-dhgt.rtf")
{
    // Test that shpz has priority over dhght and not the other way around.
    // Drawpage is sorted by ZOrder, so first should be red (back).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(getShape(1), "FillColor"));
    // Second (front) should be green.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00), getProperty<sal_Int32>(getShape(2), "FillColor"));
}

DECLARE_RTFIMPORT_TEST(testBackground, "background.rtf")
{
    // The first shape wasn't in the foreground.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<sal_Bool>(getShape(1), "Opaque")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<sal_Bool>(getShape(2), "Opaque")));
}

DECLARE_RTFIMPORT_TEST(testLevelfollow, "levelfollow.rtf")
{
    uno::Reference<container::XIndexAccess> xNum1Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::LISTTAB), comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, tab

    uno::Reference<container::XIndexAccess> xNum2Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum2"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::SPACE), comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, space

    uno::Reference<container::XIndexAccess> xNum3Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum3"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::NOTHING), comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, nothing
}

DECLARE_RTFIMPORT_TEST(testCharColor, "char-color.rtf")
{
    // This was -1: character color wasn't set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x365F91), getProperty<sal_Int32>(getParagraph(1), "CharColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo69289, "fdo69289.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // There were only 2 cells (1 separators) in the table, should be 3 (2 separators).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFIMPORT_TEST(testDptxbxRelation, "dptxbx-relation.rtf")
{
    // This was FRAME, not PAGE_FRAME, even if dobxpage is in the document.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
