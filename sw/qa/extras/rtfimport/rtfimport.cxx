/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XDocumentIndexMark.hpp>
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
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/XFormField.hpp>

#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include "comphelper/configuration.hxx"

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/rtfimport/data/", "Rich Text Format")
    {
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename) == "fdo48023.rtf" || OString(filename) == "fdo72031.rtf")
        {
            std::unique_ptr<Resetter> pResetter(new Resetter([this]()
            {
                Application::SetSettings(this->m_aSavedSettings);
            }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ru"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        else if (OString(filename) == "fdo44211.rtf")
        {
            std::unique_ptr<Resetter> pResetter(new Resetter([this]()
            {
                Application::SetSettings(this->m_aSavedSettings);
            }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("lt"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    /// Copy&paste helper.
    void paste(const OUString& aFilename, uno::Reference<text::XTextRange> const& xTextRange = uno::Reference<text::XTextRange>())
    {
        uno::Reference<document::XFilter> xFilter(m_xSFactory->createInstance("com.sun.star.comp.Writer.RtfFilter"), uno::UNO_QUERY_THROW);
        uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
        xImporter->setTargetDocument(mxComponent);
        uno::Sequence<beans::PropertyValue> aDescriptor(xTextRange.is() ? 3 : 2);
        aDescriptor[0].Name = "InputStream";
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc("/sw/qa/extras/rtfimport/data/") + aFilename, StreamMode::WRITE);
        uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(*pStream));
        aDescriptor[0].Value <<= xStream;
        aDescriptor[1].Name = "InsertMode";
        aDescriptor[1].Value <<= true;
        if (xTextRange.is())
        {
            aDescriptor[2].Name = "TextInsertModeRange";
            aDescriptor[2].Value <<= xTextRange;
        }
        xFilter->filter(aDescriptor);
    }
    AllSettings m_aSavedSettings;
};

#if !defined(_WIN32)

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
            if (aStr == "space-before")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(120)), getProperty<sal_Int32>(xRange, "ParaTopMargin"));
            else if (aStr == "space-after")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(240)), getProperty<sal_Int32>(xRange, "ParaBottomMargin"));
        }
    }
}

DECLARE_RTFIMPORT_TEST(testN192129, "n192129.rtf")
{
    // We expect that the result will be 16x16px.
    Size aExpectedSize(16, 16);
    MapMode aMap(MapUnit::Map100thMM);
    aExpectedSize = Application::GetDefaultDevice()->PixelToLogic(aExpectedSize, aMap);

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    awt::Size aActualSize(xShape->getSize());
    if ((aExpectedSize.Width() - aActualSize.Width) / 2 != 0)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Width()), aActualSize.Width);
    }
    if ((aExpectedSize.Height() - aActualSize.Height) / 2 != 0)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(aExpectedSize.Height()), aActualSize.Height);
    }
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(300)), getProperty<sal_Int32>(xPropertySet, "Height"));

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

        if (rProp.Name == "ParentNumbering")
            CPPUNIT_ASSERT_EQUAL(sal_Int16(2), rProp.Value.get<sal_Int16>());
        else if (rProp.Name == "Suffix")
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0), rProp.Value.get<OUString>().getLength());
    }
}

DECLARE_RTFIMPORT_TEST(testN750757, "n750757.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin")));
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(200)), getProperty<sal_Int32>(xParaEnum->nextElement(), "ParaBottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo79384, "fdo79384.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    CPPUNIT_ASSERT_EQUAL(OUString("Маркеры спискамЫ", 31, RTL_TEXTENCODING_UTF8), xTextRange->getString());
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

DECLARE_RTFIMPORT_TEST(testFdo81892, "fdo81892.rtf")
{
    // table was not centered
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xTable, "HoriOrient"));

    // fdo#81893: paragraph with \page was not centered
    uno::Reference<text::XTextRange> xPara(getParagraph(2, "Performance"));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFIMPORT_TEST(testFdo45394, "fdo45394.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
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
    uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName("Standard"), uno::UNO_QUERY);
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

DECLARE_RTFIMPORT_TEST(testFdo85812, "fdo85812.rtf")
{
    lang::Locale locale(getProperty<lang::Locale>(getRun(getParagraph(1), 1, "This "), "CharLocale"));
    // the \lang inside the group was applied to CJK not Western
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    // further testing indicates that Word is doing really weird stuff
    // \loch \hich \dbch is reset by opening a group
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("ru"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("RU"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(2), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(3), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("ru"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("RU"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 1, "CharGroup"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(4), 2, "AfterChar"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(5), 2, "AfterBookmark"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    // \ltrch \rtlch works differently - it is sticky across groups
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 1, "CharGroup"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 1, "CharGroup"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("DZ"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 2, "AfterChar"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(6), 2, "AfterChar"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("EG"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, "AfterBookmark"), "CharLocale");
    CPPUNIT_ASSERT_EQUAL(OUString("en"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("US"), locale.Country);
    locale = getProperty<lang::Locale>(getRun(getParagraph(7), 2, "AfterBookmark"), "CharLocaleComplex");
    CPPUNIT_ASSERT_EQUAL(OUString("ar"), locale.Language);
    CPPUNIT_ASSERT_EQUAL(OUString("EG"), locale.Country);
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
    if (Application::GetDefaultDevice()->IsFontAvailable("Times New Roman"))
        CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo49501, "fdo49501.rtf")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
    sal_Int32 nExpected(convertTwipToMm100(567));
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
        {
            OUString aExpected(static_cast<sal_Unicode>(0x200B));
            CPPUNIT_ASSERT_EQUAL(aExpected, rProp.Value.get<OUString>());
        }
    }
}

DECLARE_RTFIMPORT_TEST(testFdo45190, "fdo45190.rtf")
{
    // inherited \fi should be reset
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));

    // but direct one not
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-100)), getProperty<sal_Int32>(getParagraph(2), "ParaFirstLineIndent"));
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

DECLARE_OOXMLIMPORT_TEST(testTdf59699, "tdf59699.rtf")
{
    // This resulted in a lang.IndexOutOfBoundsException: the referenced graphic data wasn't imported.
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_RTFIMPORT_TEST(testFdo46966, "fdo46966.rtf")
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(720)), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo52066, "fdo52066.rtf")
{
    /*
     * The problem was that the height of the shape was too big.
     *
     * xray ThisComponent.DrawPage(0).Size.Height
     */
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(19)), xShape->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testFdo76633, "fdo76633.rtf")
{
    // check that there is only a graphic object, not an additional rectangle
    uno::Reference<lang::XServiceInfo> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT(xShape->supportsService("com.sun.star.text.TextGraphicObject"));
    try
    {
        uno::Reference<drawing::XShape> xShape2(getShape(2), uno::UNO_QUERY);
        CPPUNIT_FAIL("exception expected");
    }
    catch (lang::IndexOutOfBoundsException const&)
    {
        /* expected */
    }
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
    BorderTest::testTheBorders(mxComponent, false);
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

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
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
    table::BorderLine2 line(getProperty<table::BorderLine2>(xPropertySet, "TopBorder"));
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, line.LineStyle);
}

DECLARE_RTFIMPORT_TEST(testLeftmarginDefault, "leftmargin-default.rtf")
{
    // The default left/right margin was incorrect when the top margin was set to zero.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testDppolyline, "dppolyline.rtf")
{
    // This was completely ignored, for now, just make sure we have all 4 lines.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo79319, "fdo79319.rtf")
{
    // the thin horizontal rule was imported as a big fat rectangle
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(100), getProperty<sal_Int16>(xShape, "RelativeWidth"));
    // FIXME the width/height numbers here are bogus; they should be 15238 / 53
    // (as they are when opening the file in a full soffice)
#if 0
    CPPUNIT_ASSERT_EQUAL(sal_Int32(15238), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(53), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(xShape, "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(xShape, "HoriOrient"));
#endif
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
    OString aMessage("xShape->getSize().Width() = ");
    aMessage += OString::number(xShape->getSize().Width);

    // This was 2, should be 423 (or 369?).
    CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), xShape->getSize().Width >= 273);
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
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
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
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
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

DECLARE_RTFIMPORT_TEST(testFdo78502, "fdo78502.rtf")
{
    // ";" separators were inserted as text
    getParagraph(1, "foo");
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(8345)), getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
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

DECLARE_RTFIMPORT_TEST(testDoDhgtOld, "do-dhgt-old.rtf")
{
    // The file contains 3 shapes which have the same dhgt (z-order).
    // Test that the order is 1) a 2) black rectangle 3) b, and not something else
    uno::Reference<text::XText> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("a"), xShape->getString());

    xShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, getProperty<sal_uInt32>(xShape, "FillColor"));

    xShape.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xShape->getString());
}

DECLARE_RTFIMPORT_TEST(testTdf84684, "tdf84684.rtf")
{
    // The ZOrder of the two children of the group shape were swapped.
    uno::Reference<drawing::XShapes> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<container::XNamed> xChild1(xGroup->getByIndex(0), uno::UNO_QUERY);
    // This was Pie 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Rectangle 1"), xChild1->getName());
    uno::Reference<container::XNamed> xChild2(xGroup->getByIndex(1), uno::UNO_QUERY);
    // This was Rectangle 1.
    CPPUNIT_ASSERT_EQUAL(OUString("Pie 2"), xChild2->getName());
}

DECLARE_RTFIMPORT_TEST(testFdo61909, "fdo61909.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
    // Was the Writer default font.
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"), getProperty<OUString>(xTextRange, "CharFontName"));
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
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
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

DECLARE_RTFIMPORT_TEST(testFdo81944, "fdo81944.rtf")
{
    // font properties in style were not imported
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Segoe UI"), getProperty<OUString>(xStyle, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(9.0f, getProperty<float>(xStyle, "CharHeight"));
    // not sure if this should be set on Asian or Complex or both?
    CPPUNIT_ASSERT_EQUAL(OUString("Times New Roman"), getProperty<OUString>(xStyle, "CharFontNameComplex"));
    CPPUNIT_ASSERT_EQUAL(11.0f, getProperty<float>(xStyle, "CharHeightComplex"));
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

DECLARE_RTFIMPORT_TEST(testFdo70578, "fdo70578.rtf")
{
    // Style without explicit \s0 was not imported as the default style
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Subtitle"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), xStyle->getParentStyle());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testPoshPosv, "posh-posv.rtf")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(getShape(1), "HoriOrient"));
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::CENTER, getProperty<sal_Int16>(getShape(1), "VertOrient"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getShape(1), "FrameIsAutomaticHeight"));
}

DECLARE_RTFIMPORT_TEST(testTdf96326, "tdf96326.rtf")
{
    // Make sure this is not checked.
    auto xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(1), 2), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters = xFormField->getParameters();
    // This was true, ffres=25 was interpreted as checked.
    CPPUNIT_ASSERT_EQUAL(false, bool(xParameters->hasElements()));

    // And this is checked.
    xFormField = getProperty< uno::Reference<text::XFormField> >(getRun(getParagraph(2), 2), "Bookmark");
    xParameters = xFormField->getParameters();
    CPPUNIT_ASSERT_EQUAL(true, xParameters->getByName("Checkbox_Checked").get<bool>());
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
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));

    // This was 353, i.e. bottom margin of the paragraph was 0.35cm instead of 0.
    // The reason why this is 0 despite the default style containing \sa200
    // is that Word will actually interpret \sN (or \pard which apparently
    // implies \s0) as "set style N and for every attribute of that style,
    // set an attribute with default value on the paragraph"
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
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

DECLARE_RTFIMPORT_TEST(testFdo77996, "fdo77996.rtf")
{
    // all styles were imported as name "0"
    uno::Reference<container::XNameAccess> xChars(getStyles("CharacterStyles"));
    CPPUNIT_ASSERT(!xChars->hasByName("0"));
    CPPUNIT_ASSERT(xChars->hasByName("strong"));
    CPPUNIT_ASSERT(xChars->hasByName("author"));
    uno::Reference<container::XNameAccess> xParas(getStyles("ParagraphStyles"));
    CPPUNIT_ASSERT(!xParas->hasByName("0"));
    CPPUNIT_ASSERT(xParas->hasByName("extract2"));
    // some document properties were lost
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Aln Lin (Bei Jing)"), xProps->getAuthor());
    OUString aTitle("\xe5\x8e\xa6\xe9\x97\xa8\xe9\x92\xa8\xe4\xb8\x9a\xe8\x82\xa1\xe4\xbb\xbd\xe6\x9c\x89\xe9\x99\x90\xe5\x85\xac\xe5\x8f\xb8", 30, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aTitle, xProps->getTitle());
    uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("jay"), getProperty<OUString>(xUDProps, "Operator"));

    // fdo#80486 also check that the ftnsep doesn't insert paragraph breaks
    getParagraph(1, aTitle);
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

DECLARE_RTFIMPORT_TEST(testFdo81033, "fdo81033.rtf")
{
    // Number of tabstops in the paragraph should be 2, was 3.
    uno::Sequence<style::TabStop> tabs(
        getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), tabs.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5808), tabs[0].Position);
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_LEFT, tabs[0].Alignment);
    CPPUNIT_ASSERT_EQUAL(sal_Unicode(' '), tabs[0].FillChar);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16002), tabs[1].Position);
    CPPUNIT_ASSERT_EQUAL(style::TabAlign_LEFT, tabs[1].Alignment);
    CPPUNIT_ASSERT_EQUAL(sal_Unicode('_'), tabs[1].FillChar);
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(5954)), getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
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
    CPPUNIT_ASSERT_EQUAL(uno::Any(), uno::Reference<beans::XPropertySet>(getParagraph(1), uno::UNO_QUERY)->getPropertyValue("PageDescName"));
}

DECLARE_RTFIMPORT_TEST(testFdo69384, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo69384-paste.rtf", xEnd);

    // Import got interrupted in the middle of style sheet table import,
    // resulting in missing styles and text.
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

class testTdf105511 : public Test
{
protected:
    virtual OUString getTestName() override
    {
        return OUString("testTdf105511");
    }
public:
    CPPUNIT_TEST_SUITE(testTdf105511);
    CPPUNIT_TEST(Import);
    CPPUNIT_TEST_SUITE_END();

    void Import()
    {
        struct DefaultLocale : public comphelper::ConfigurationProperty<DefaultLocale, rtl::OUString>
        {
            static OUString path()
            {
                return OUString("/org.openoffice.Office.Linguistic/General/DefaultLocale");
            }
            ~DefaultLocale() = delete;
        };
        auto batch = comphelper::ConfigurationChanges::create();
        DefaultLocale::set("ru-RU", batch);
        batch->commit();
        executeImportTest("tdf105511.rtf", nullptr);
    }
    virtual void verify() override
    {
        OUString aExpected("\xd0\x98\xd0\xbc\xd1\x8f", 6, RTL_TEXTENCODING_UTF8);
        getParagraph(1, aExpected);
    }
};
CPPUNIT_TEST_SUITE_REGISTRATION(testTdf105511);

#endif
DECLARE_RTFIMPORT_TEST(testFdo94835, "fdo94835.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), xDraws->getCount());
}
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
    table::BorderLine2 fullPtSolid(1, 0, 35, 0, table::BorderLineStyle::SOLID, 35);
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
    table::BorderLine2 halfPtSolid(/*0*/1, 0, 18, 0, table::BorderLineStyle::SOLID, 18);
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
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaSecond, uno::UNO_QUERY)->getPropertyValue("PageDescName"));
    // actually not sure how many paragraph there should be between
    // SECOND and THIRD - important is that the page break is on there
    uno::Reference<text::XTextRange> xParaNext = getParagraph(3);
    CPPUNIT_ASSERT_EQUAL(OUString(""), xParaNext->getString());
    //If PageDescName is not empty, a page break / switch to page style is defined
    CPPUNIT_ASSERT(uno::Any() != getProperty<OUString>(xParaNext, "PageDescName"));
    uno::Reference<text::XTextRange> xParaThird = getParagraph(4);
    CPPUNIT_ASSERT_EQUAL(OUString("THIRD"), xParaThird->getString());
    CPPUNIT_ASSERT_EQUAL(style::BreakType_NONE,
                         getProperty<style::BreakType>(xParaThird, "BreakType"));
    CPPUNIT_ASSERT_EQUAL(uno::Any(),
                         uno::Reference<beans::XPropertySet>(xParaThird, uno::UNO_QUERY)->getPropertyValue("PageDescName"));

    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testFooterPara, "footer-para.rtf")
{
    // check that paragraph properties in footer are imported
    uno::Reference<text::XText> xFooterText =
        getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("First Page"), "FooterText");
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
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had 3 cells, instead of a horizontally merged one and a normal one (2 -> 1 separator).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFIMPORT_TEST(testTableBorderDefaults, "fdo68779.rtf")
{
    // table borders without \brdrw were not imported
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 solid(1, 0, 26, 0, table::BorderLineStyle::SOLID, 26);
    CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(1), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 dotted(1, 0, 26, 0, table::BorderLineStyle::DOTTED, 26);
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(2), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 doubled(1, 26, 26, 26, table::BorderLineStyle::DOUBLE, 79);
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(3), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 thinThickMG(1, 14, 26, 14, table::BorderLineStyle::THINTHICK_MEDIUMGAP, 53);
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                getProperty<table::BorderLine2>(xCell, "BottomBorder"));
}

DECLARE_RTFIMPORT_TEST(testShpzDhgt, "shpz-dhgt.rtf")
{
    // Test that shpz has priority over dhgt and not the other way around.
    // Drawpage is sorted by ZOrder, so first should be red (back).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(getShape(1), "FillColor"));
    // Second (front) should be green.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00), getProperty<sal_Int32>(getShape(2), "FillColor"));
}

DECLARE_RTFIMPORT_TEST(testBackground, "background.rtf")
{
    // The first shape wasn't in the foreground.
    CPPUNIT_ASSERT_EQUAL(true, bool(getProperty<bool>(getShape(1), "Opaque")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getProperty<bool>(getShape(2), "Opaque")));
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

DECLARE_RTFIMPORT_TEST(testDprectAnchor, "dprect-anchor.rtf")
{
    // This was at-page, which is not something Word supports, so clearly an import error.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFIMPORT_TEST(testFdo76628, "fdo76628.rtf")
{
    OUString aExpected("\xd0\x9e\xd0\x91\xd0\xa0\xd0\x90\xd0\x97\xd0\x95\xd0\xa6", 14, RTL_TEXTENCODING_UTF8);
    // Should be 'SAMPLE' in Russian, was garbage.
    getParagraph(1, aExpected);

    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    OUString aExpectedHeader("\xd0\x9f\xd0\xbe\xd0\xb4\xd0\xb3\xd0\xbe\xd1\x82\xd0\xbe\xd0\xb2\xd0\xbb\xd0\xb5\xd0\xbd\xd0\xbe", 24, RTL_TEXTENCODING_UTF8);
    // Should be 'prepared' in Russian, was garbage.
    getParagraphOfText(1, xHeaderText, aExpectedHeader);
}

DECLARE_RTFIMPORT_TEST(testFdo74823, "fdo74823.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // Cell width of C2 was too large / column separator being 3749 too small (e.g. not set, around 3/7 of total width)
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(5391), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[2].Position);
}

DECLARE_RTFIMPORT_TEST(testFdo74599, "fdo74599.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Heading 3"), uno::UNO_QUERY);
    // Writer default styles weren't disabled, so the color was gray, not default (black).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(xPropertySet, "CharColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo77267, "fdo77267.rtf")
{
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFIMPORT_TEST(testFdo75735, "fdo75735.rtf")
{
    // Number of tabstops in the second paragraph should be 3, was 6.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty< uno::Sequence<style::TabStop> >(getParagraph(2), "ParaTabStops").getLength());
}

DECLARE_RTFIMPORT_TEST(testFontOverride, "font-override.rtf")
{
    // This was "Times New Roman".
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

DECLARE_RTFIMPORT_TEST(testColumnBreak, "column-break.rtf")
{
    // Column break at the very start of the document was ignored.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_RTFIMPORT_TEST(testFdo73241, "fdo73241.rtf")
{
    // This was 2, page break in table wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFIMPORT_TEST(testFdo80905, "fdo80905.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xFields->nextElement();
    // The problem was that there was only one field in the document, but there should be true.
    CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(xFields->hasMoreElements()));
}

DECLARE_RTFIMPORT_TEST(testUnbalancedColumnsCompat, "unbalanced-columns-compat.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, we ignored the relevant compat setting to make this non-last section unbalanced.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_RTFIMPORT_TEST(testOleInline, "ole-inline.rtf")
{
    // Problem was that inline shape had at-page anchor.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFIMPORT_TEST(testFdo80742, "fdo80742.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Heading 2"), uno::UNO_QUERY);
    // This was 0, outline level was body text.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xPropertySet, "OutlineLevel"));
}

DECLARE_RTFIMPORT_TEST(testFdo82106, "fdo82106.rtf")
{
    // Tab was missing after footnote not containing a tab.
    getParagraph(2, "before\tafter");
}

DECLARE_RTFIMPORT_TEST(testBehindDoc, "behind-doc.rtf")
{
    // The problem was that "behind doc" didn't result in the shape being in the background, only in being wrapped as "through".
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(xShape, "Surround"));
    // This was true.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xShape, "Opaque"));
}

DECLARE_RTFIMPORT_TEST(testFdo74229, "fdo74229.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, due to ignoring RTF_TRGAPH.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(67)), getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

DECLARE_RTFIMPORT_TEST(testFdo79959, "fdo79959.rtf")
{
    // This was false, as the style was imported as " Test", i.e. no whitespace stripping.
    CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(getStyles("ParagraphStyles")->hasByName("Test")));
}

DECLARE_RTFIMPORT_TEST(testFdo82078, "fdo82078.rtf")
{
    // This was awt::FontWeight::BOLD, i.e. the second run was bold, when it should be normal.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(getRun(getParagraph(1), 2), "CharWeight"));
}

DECLARE_RTFIMPORT_TEST(testCsBold, "cs-bold.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run was bold, when it should be bold (applied character style without direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFIMPORT_TEST(testFdo82114, "fdo82114.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Converted1"), "HeaderText");
    OUString aActual = xHeaderText->getString();
    OUString aExpected("First page header, section 2");
    // This was 'Right page header, section 1'.
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFIMPORT_TEST(testFdo44984, "fdo44984.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was Text, i.e. the checkbox field portion was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStartEnd"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testFdo84679, "fdo84679.rtf")
{
    // The problem was that the paragraph in A1 had some bottom margin, but it should not.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 282.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_RTFIMPORT_TEST(testFdo82071, "fdo82071.rtf")
{
    // The problem was that in TOC, chapter names were underlined, but they should not be.
    uno::Reference<text::XTextRange> xRun = getRun(getParagraph(2), 1);
    // Make sure we test the right text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 1"), xRun->getString());
    // This was awt::FontUnderline::SINGLE.
    CPPUNIT_ASSERT_EQUAL(awt::FontUnderline::NONE, getProperty<sal_Int16>(xRun, "CharUnderline"));
}

DECLARE_RTFIMPORT_TEST(testFdo83464, "fdo83464.rtf")
{
    // Problem was that the text in the textfrme had wrong font.
    uno::Reference<text::XTextRange> xFrameText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFrameText->getString());
    // This was Times New Roman.
    CPPUNIT_ASSERT_EQUAL(OUString("Verdana"), getProperty<OUString>(getRun(getParagraphOfText(1, xFrameText->getText()), 1), "CharFontName"));
}

DECLARE_RTFIMPORT_TEST(testFdo85179, "fdo85179.rtf")
{
    // This was 0, border around the picture was ignored on import.
    // 360: EMU -> MM100
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(50800/360), getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_RTFIMPORT_TEST(testFdo86761, "fdo86761.rtf")
{
    // This was 26, even if the picture should have no border, due to fLine=0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_RTFIMPORT_TEST(testFdo82859, "fdo82859.rtf")
{
    // This was 0: "0xffffff" was converted to 0, i.e. the background was black instead of the default.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(getShape(1), "BackColor"));
}

DECLARE_RTFIMPORT_TEST(testFdo82076, "fdo82076.rtf")
{
    // Footnote position was wrong: should be at the end of the B1 cell.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // This resulted in container::NoSuchElementException: the footnote was at the start of the A1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 2), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testFdo82512, "fdo82512.rtf")
{
    // This was style::BreakType_NONE, column break was before the 3rd paragraph, not before the 2nd one.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
}

DECLARE_RTFIMPORT_TEST(testUnbalancedColumns, "unbalanced-columns.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_RTFIMPORT_TEST(testFdo84685, "fdo84685.rtf")
{
    // index mark was not imported
    uno::Reference<text::XDocumentIndexMark> xMark(
        getProperty<uno::Reference<text::XDocumentIndexMark>>(
            getRun(getParagraph(1), 1),
            "DocumentIndexMark"));
    CPPUNIT_ASSERT(xMark.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Key the 1st"), getProperty<OUString>(xMark, "PrimaryKey"));
    // let's test toc entry too
    uno::Reference<text::XDocumentIndexMark> xTOCMark(
        getProperty<uno::Reference<text::XDocumentIndexMark>>(
            getRun(getParagraph(2), 1),
            "DocumentIndexMark"));
    CPPUNIT_ASSERT(xTOCMark.is());
    uno::Reference<lang::XServiceInfo> xTOCSI(xTOCMark, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTOCSI->supportsService("com.sun.star.text.ContentIndexMark"));
}

DECLARE_RTFIMPORT_TEST(testFdo83204, "fdo83204.rtf")
{
    // This was Standard, \sN was ignored after \bkmkstart and \pard.
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 ).
DECLARE_RTFIMPORT_TEST(testCp950listleveltext1, "cp950listleveltext1.rtf")
{
    // suffix with Chinese only ( most common case generated by MSO2010 TC)
    const sal_Unicode aExpectedSuffix[1] = { 0x3001 };  // This is a dot that is generally used as suffix of Chinese list number
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum3"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 ).
DECLARE_RTFIMPORT_TEST(testCp950listleveltext2, "cp950listleveltext2.rtf")
{
    // Prefix and suffix with Chinese only ( tweaked from default in MSO2010 TC)
    const sal_Unicode aExpectedPrefix[2] = { 0x524d, 0x7f6e };
    const sal_Unicode aExpectedSuffix[3] = { 0x3001, 0x5f8c, 0x7f6e };

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix,aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix,SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}


// This testcase illustrate leveltext with multibyte strings coded in cp950 ( BIG5 )
DECLARE_RTFIMPORT_TEST(testCp950listleveltext3, "cp950listleveltext3.rtf")
{
    // Prefix and suffix that mix Chinese and English ( tweaked from default in MSO2010 TC)
    const sal_Unicode aExpectedPrefix[4] = { 0x524d, 0x0061, 0x7f6e, 0x0062 };
    const sal_Unicode aExpectedSuffix[6] = { 0x3001, 0x0063, 0x5f8c, 0x0064, 0x7f6e, 0x0065 };

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix,aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    // Suffix was '\0' instead of ' '.
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix,SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_RTFIMPORT_TEST(testChtOutlineNumberingRtf, "chtoutline.rtf")
{
    const sal_Unicode aExpectedPrefix[2] = { 0x7b2c, 0x0020 };
    const sal_Unicode aExpectedSuffix[2] = { 0x0020, 0x7ae0 };
    uno::Reference< text::XChapterNumberingSupplier > xChapterNumberingSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference< container::XIndexAccess> xLevels(xChapterNumberingSupplier->getChapterNumberingRules());
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString aSuffix,aPrefix;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            aSuffix = rProp.Value.get<OUString>();
        if (rProp.Name == "Prefix")
            aPrefix = rProp.Value.get<OUString>();
    }
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedPrefix,SAL_N_ELEMENTS(aExpectedPrefix)), aPrefix);
    CPPUNIT_ASSERT_EQUAL(OUString(aExpectedSuffix,SAL_N_ELEMENTS(aExpectedSuffix)), aSuffix);
}

DECLARE_RTFIMPORT_TEST(testFdo85889pc, "fdo85889-pc.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected("\xc2\xb1\xe2\x89\xa5\xe2\x89\xa4", 8, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo85889pca, "fdo85889-pca.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected("\xc2\xb1\xe2\x80\x97\xc2\xbe", 7, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testTdf90046, "tdf90046.rtf")
{
    // this was crashing on importing the footnote
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Ma"), xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo85889mac, "fdo85889-mac.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected("\xc3\x92\xc3\x9a\xc3\x9b", 6, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo72031, "fdo72031.rtf")
{
    OUString aExpected("\xc3\x85", 2, RTL_TEXTENCODING_UTF8);
    CPPUNIT_ASSERT_EQUAL(aExpected, getRun(getParagraph(1), 1)->getString());
}

DECLARE_RTFIMPORT_TEST(testFdo86750, "fdo86750.rtf")
{
    // This was 'HYPERLINK#anchor', the URL of the hyperlink had the field type as a prefix, leading to broken links.
    CPPUNIT_ASSERT_EQUAL(OUString("#anchor"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

DECLARE_RTFIMPORT_TEST(testTdf88811, "tdf88811.rtf")
{
    // The problem was that shapes anchored to the paragraph that is moved into a textframe were lost, so this was 2.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xDrawPage->getCount());
}

DECLARE_RTFIMPORT_TEST(testFdo49893, "fdo49893.rtf")
{
    // Image from shape was not loaded, invalid size of image after load
    uno::Reference<drawing::XShape> xShape(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(432)), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(1296)), xShape->getSize().Width);
}

DECLARE_RTFIMPORT_TEST(testFdo49893_2, "fdo49893-2.rtf")
{
    // Ensure that header text exists on each page (especially on second page)
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_RTFIMPORT_TEST(testFdo49893_3, "fdo49893-3.rtf")
{
    // No artifacts (black lines in left top corner) as shape #3 are expected
    try
    {
        uno::Reference<drawing::XShape> xShape2(getShape(3), uno::UNO_QUERY);
        CPPUNIT_FAIL("exception expected: no shape #3 in document");
    }
    catch (lang::IndexOutOfBoundsException const&)
    {
        /* expected */
    }

    // Correct wrapping for shape
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGHT, getProperty<text::WrapTextMode>(getShape(2), "Surround"));
}

DECLARE_RTFIMPORT_TEST(testFdo89496, "fdo89496.rtf")
{
    // Just ensure that document is loaded and shape exists
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
}

DECLARE_RTFIMPORT_TEST(testFdo75614, "tdf75614.rtf")
{
    // Text after the footnote was missing, so this resulted in a css::container::NoSuchElementException.
    CPPUNIT_ASSERT_EQUAL(OUString("after."), getRun(getParagraph(1), 3)->getString());
}

DECLARE_RTFIMPORT_TEST(mathtype, "mathtype.rtf")
{
    OUString aFormula = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT(!aFormula.isEmpty());
}

DECLARE_RTFIMPORT_TEST(testTdf86182, "tdf86182.rtf")
{
    // Writing mode was the default, i.e. text::WritingMode2::CONTEXT.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(getParagraph(1), "WritingMode"));
}

DECLARE_RTFIMPORT_TEST(testWrapDistance, "wrap-distance.rtf")
{
    // Custom shape, handled directly in RTFSdrImport.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.drawing.CustomShape"), xShape->getShapeType());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), getProperty<sal_Int32>(xShape, "TopMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), getProperty<sal_Int32>(xShape, "BottomMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), getProperty<sal_Int32>(xShape, "LeftMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), getProperty<sal_Int32>(xShape, "RightMargin") / 1000);

    // Picture, handled in GraphicImport, shared with DOCX.
    xShape = getShape(2);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), getProperty<sal_Int32>(xShape, "TopMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), getProperty<sal_Int32>(xShape, "BottomMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), getProperty<sal_Int32>(xShape, "LeftMargin") / 1000);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), getProperty<sal_Int32>(xShape, "RightMargin") / 1000);
}

DECLARE_RTFIMPORT_TEST(testTdf91074, "tdf91074.rtf")
{
    // The file failed to load, as the border color was imported using the LineColor UNO property.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_LIGHTRED), getProperty<table::BorderLine2>(xShape, "TopBorder").Color);
}

DECLARE_RTFIMPORT_TEST(testTdf90260Nopar, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("tdf90260-nopar.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
}

DECLARE_RTFIMPORT_TEST(testTdf90260Par, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("tdf90260-par.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

DECLARE_RTFIMPORT_TEST(testTdf86814, "tdf86814.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold (applied paragraph style with direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFIMPORT_TEST(testTdf90315, "tdf90315.rtf")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xTextSection(xTextSections->getByIndex(0), uno::UNO_QUERY);
    // This was 0, but default should be 720 twips.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), getProperty<sal_Int32>(xTextSection->getPropertyValue("TextColumns"), "AutomaticDistance"));
}

DECLARE_RTFIMPORT_TEST(testBinSkipping, "bin-skipping.rtf")
{
    // before, it was importing '/nMUST NOT IMPORT'
    CPPUNIT_ASSERT_EQUAL(OUString("text"), getRun(getParagraph(1), 1)->getString());
}

DECLARE_RTFIMPORT_TEST(testTdf92061, "tdf92061.rtf")
{
    // This was "C", i.e. part of the footnote ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("body-after"), getRun(getParagraph(1), 3)->getString());
}

DECLARE_RTFIMPORT_TEST(testTdf92481, "tdf92481.rtf")
{
    // This was 0, RTF_WIDOWCTRL was not imported.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(2), getProperty<sal_Int8>(getParagraph(1), "ParaWidows"));
}

DECLARE_RTFIMPORT_TEST(testTdf94456, "tdf94456.rtf")
{
    // Paragraph left margin and first line indent wasn't imported correctly.

    // This was 1270.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(762), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    // This was -635.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-762), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_RTFIMPORT_TEST(testTdf94435, "tdf94435.rtf")
{
    // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFIMPORT_TEST(testTdf59454, "tdf59454.rtf")
{
    // This was 1, section break was ignored right before a table.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFIMPORT_TEST(testTdf54584, "tdf54584.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \PAGE was ignored, so no fields were in document -> exception was thrown
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("No fields in document found: field \"\\PAGE\" was not properly read",
                                    xFields->nextElement());
}

DECLARE_RTFIMPORT_TEST(testTdf96308Deftab, "tdf96308-deftab.rtf")
{
    uno::Reference<lang::XMultiServiceFactory> xTextFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefaults(xTextFactory->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    // This was 1270 as \deftab was ignored on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(284)), getProperty<sal_Int32>(xDefaults, "TabStopDistance"));
}

DECLARE_RTFIMPORT_TEST(testTdf96308Tabpos, "tdf96308-tabpos.rtf")
{
    // Get the tab stops of the second para in the B1 cell of the first table in the document.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    xParaEnum->nextElement();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    auto aTabStops = getProperty< uno::Sequence<style::TabStop> >(xPara, "ParaTabStops");
    // This failed: tab stops were not deleted as direct formatting on the paragraph.
    CPPUNIT_ASSERT(!aTabStops.hasElements());
}

DECLARE_RTFIMPORT_TEST(testLndscpsxn, "lndscpsxn.rtf")
{
    // Check landscape flag.
    CPPUNIT_ASSERT_EQUAL(4, getPages());

    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // check that the first page has landscape flag
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xStylePage(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the second page has no landscape flag
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the third page has landscape flag
    xCursor->jumpToPage(3);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the last page has no landscape flag
    xCursor->jumpToLastPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xStylePage, "IsLandscape"));
}

DECLARE_RTFIMPORT_TEST(testLandscape, "landscape.rtf")
{
    // Check landscape flag.
    CPPUNIT_ASSERT_EQUAL(3, getPages());

    // All pages should have flag orientiation
    uno::Reference<container::XNameAccess> pageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // check that the first page has landscape flag
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xStylePage(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the second page has landscape flag
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));

    // check that the last page has landscape flag
    xCursor->jumpToLastPage();
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xStylePage.set(pageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStylePage, "IsLandscape"));
}

DECLARE_RTFIMPORT_TEST(testTdf97035, "tdf97035.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // First cell width of the second row should be 2299
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2299), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFIMPORT_TEST(testTdf99498, "tdf99498.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);

    // Table width was a tiny sub one char wide 145twips, it should now be a table wide
    // enough to see all the text in the first column without breaking into multiple lines
    CPPUNIT_ASSERT_EQUAL(sal_Int32(7056), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}


DECLARE_RTFIMPORT_TEST(testTdf87034, "tdf87034.rtf")
{
    // This was A1BC34D, i.e. the first "super" text portion was mis-imported,
    // and was inserted instead right before the second "super" text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("A1B3C4D"), getParagraph(1)->getString());
}

DECLARE_RTFIMPORT_TEST(testClassificatonPaste, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();

    // Not classified source, not classified destination: OK.
    paste("classification-no.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(OUString("classification-no"), getParagraph(2)->getString());

    // Classified source, not classified destination: nothing should happen.
    OUString aOld = xText->getString();
    paste("classification-yes.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

DECLARE_RTFIMPORT_TEST(testClassificatonPasteLevels, "classification-confidential.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();

    // Classified source and classified destination, but internal only has a
    // higher level than confidential: nothing should happen.
    OUString aOld = xText->getString();
    paste("classification-yes.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

#if !defined(MACOSX) && !defined(WNT)
DECLARE_RTFIMPORT_TEST(testTdf90097, "tdf90097.rtf")
{
    // Get the second child of the group shape.
    uno::Reference<container::XIndexAccess> xGroup(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xGroup->getByIndex(0), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<awt::Point> > aPolyPolySequence;
    xShape->getPropertyValue("PolyPolygon") >>= aPolyPolySequence;
    uno::Sequence<awt::Point>& rPolygon = aPolyPolySequence[0];
    // Vertical flip for the line shape was ignored, so Y coordinates were swapped.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2819), rPolygon[0].X);
    // This was 1619.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1963), rPolygon[0].Y);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3181), rPolygon[1].X);
    // This was 1962.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1620), rPolygon[1].Y);
}
#endif

DECLARE_RTFIMPORT_TEST(testTdf95707, "tdf95707.rtf")
{
    // Graphic was replaced with a "Read-Error" placeholder.
    CPPUNIT_ASSERT(getProperty<OUString>(getShape(1), "GraphicURL") != "vnd.sun.star.GraphicObject:0000000000000000000000000000000000000000");
}

DECLARE_RTFIMPORT_TEST(testTdf96275, "tdf96275.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(3, xCell->getText());
    // This was text: the shape's frame was part of the 1st paragraph instead of the 3rd one.
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xParagraph, 1), "TextPortionType"));
}

DECLARE_RTFIMPORT_TEST(testTdf82073, "tdf82073.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was -1: the background color was automatic, not black.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_RTFIMPORT_TEST(testTdf74795, "tdf74795.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, \trpaddl was ignored on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));

    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Make sure that the scope of the default is only one row.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFIMPORT_TEST(testTdf77349, "tdf77349.rtf")
{
    uno::Reference<container::XNamed> xImage(getShape(1), uno::UNO_QUERY);
    // This was empty: imported image wasn't named automatically.
    CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xImage->getName());
}

DECLARE_RTFIMPORT_TEST(testTdf50821, "tdf50821.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, \trpaddfl was mishandled on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(191), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFIMPORT_TEST(testTdf91684, "tdf91684.rtf")
{
    // Scaling of the group shape children were incorrect, this was 3203.
    // (Unit was assumed to be twips, but it was relative coordinates.)
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1337), getShape(1)->getSize().Height);
}

DECLARE_RTFIMPORT_TEST(testFlip, "flip.rtf")
{
    comphelper::SequenceAsHashMap aMap = getProperty< uno::Sequence<beans::PropertyValue> >(getShapeByName("h-and-v"), "CustomShapeGeometry");
    // This resulted in a uno::RuntimeException, as MirroredX wasn't set at all, so could not extract void to boolean.
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty< uno::Sequence<beans::PropertyValue> >(getShapeByName("h-only"), "CustomShapeGeometry");
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredX"].get<bool>());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());

    aMap = getProperty< uno::Sequence<beans::PropertyValue> >(getShapeByName("v-only"), "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT_EQUAL(true, aMap["MirroredY"].get<bool>());

    aMap = getProperty< uno::Sequence<beans::PropertyValue> >(getShapeByName("neither-h-nor-v"), "CustomShapeGeometry");
    CPPUNIT_ASSERT(!aMap["MirroredX"].hasValue());
    CPPUNIT_ASSERT(!aMap["MirroredY"].hasValue());
}

DECLARE_RTFIMPORT_TEST(testTdf78506, "tdf78506.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1sd level

    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "Suffix")
            // This was '0', invalid \levelnumbers wasn't ignored.
            CPPUNIT_ASSERT_EQUAL(CHAR_ZWSP, rProp.Value.get<OUString>().toChar());
    }
}

DECLARE_RTFIMPORT_TEST(testTdf100507, "tdf100507.rtf")
{
    // This was 0: left margin of the first paragraph was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6618), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testTdf44986, "tdf44986.rtf")
{
    // Check that the table at the second paragraph.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // Check the first row of the table, it should have two cells (one separator).
    // This was 0: the first row had no separators, so it had only one cell, which was too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFIMPORT_TEST(testTdf90697, "tdf90697.rtf")
{
    // We want section breaks to be seen as section breaks, not as page breaks,
    // so this document should have only one page, not three.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFIMPORT_TEST(testTdf104317, "tdf104317.rtf")
{
    // This failed to load, we tried to set CustomShapeGeometry on a line shape.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

DECLARE_RTFIMPORT_TEST(testTdf104744, "tdf104744.rtf")
{
    // This was 0, as an unexpected "left margin is 0" token was created during
    // import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFIMPORT_TEST(testTdf105852, "tdf105852.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // All rows but last were merged -> there were only 2 rows
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTableRows->getCount());
    // The first row must have 4 cells.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
    // The third row must have 1 merged cell.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(2), "TableColumnSeparators").getLength());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
