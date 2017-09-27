/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
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
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

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
    Test() : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
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
        SvStream* pStream = utl::UcbStreamHelper::CreateStream(m_directories.getURLFromSrc("/sw/qa/extras/rtfexport/data/") + aFilename, StreamMode::WRITE);
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

DECLARE_RTFEXPORT_TEST(testFdo45553, "fdo45553.rtf")
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

DECLARE_RTFEXPORT_TEST(testN192129, "n192129.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo45543, "fdo45543.rtf")
{
    CPPUNIT_ASSERT_EQUAL(5, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo42465, "fdo42465.rtf")
{
    CPPUNIT_ASSERT_EQUAL(3, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo45187, "fdo45187.rtf")
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

DECLARE_RTFEXPORT_TEST(testN750757, "n750757.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo45563, "fdo45563.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo43965, "fdo43965.rtf")
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

DECLARE_RTFEXPORT_TEST(testN751020, "n751020.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(200)), getProperty<sal_Int32>(xParaEnum->nextElement(), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo79384, "fdo79384.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    CPPUNIT_ASSERT_EQUAL(OUString(u"Маркеры спискамЫ"), xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo47326, "fdo47326.rtf")
{
    // This was 15 only, as \super buffered text, then the contents of it got lost.
    CPPUNIT_ASSERT_EQUAL(19, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo46955, "fdo46955.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo81892, "fdo81892.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo45394, "fdo45394.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected(u"\u041F\u041A \u0420\u0418\u041A");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo48104, "fdo48104.rtf")
{
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo47107, "fdo47107.rtf")
{
    uno::Reference<container::XNameAccess> xNumberingStyles(getStyles("NumberingStyles"));
    // Make sure numbered and bullet legacy syntax is recognized, this used to throw a NoSuchElementException
    xNumberingStyles->getByName("WWNum1");
    xNumberingStyles->getByName("WWNum2");
}

DECLARE_RTFEXPORT_TEST(testFdo44176, "fdo44176.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo39053, "fdo39053.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo48356, "fdo48356.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo48023, "fdo48023.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    // Implicit encoding detection based on locale was missing
    OUString aExpected(u"\u041F\u0440\u043E\u0433\u0440\u0430\u043C\u043C\u0438\u0441\u0442");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo48876, "fdo48876.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::MINIMUM, getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing").Mode);
}

DECLARE_RTFEXPORT_TEST(testFdo48193, "fdo48193.rtf")
{
    CPPUNIT_ASSERT_EQUAL(7, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo44211, "fdo44211.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected(u"\u0105\u010D\u0119");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo48037, "fdo48037.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo47764, "fdo47764.rtf")
{
    // \cbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getParagraph(1), "ParaBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo38786, "fdo38786.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \chpgn was ignored, so exception was thrown
    xFields->nextElement();
}

DECLARE_RTFEXPORT_TEST(testN757651, "n757651.rtf")
{
    // The bug was that due to buggy layout the text expanded to two pages.
    if (Application::GetDefaultDevice()->IsFontAvailable("Times New Roman"))
        CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo49501, "fdo49501.rtf")
{
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xStyle, "IsLandscape"));
    sal_Int32 nExpected(convertTwipToMm100(567));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(nExpected, getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo49271, "fdo49271.rtf")
{
    CPPUNIT_ASSERT_EQUAL(25.f, getProperty<float>(getParagraph(2), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo50539, "fdo50539.rtf")
{
    // \chcbpat with zero argument should mean the auto (-1) color, not a default color (black)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo50665, "fdo50665.rtf")
{
    // Access the second run, which is a textfield
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
    // This used to be the default, as character properties were ignored.
    CPPUNIT_ASSERT_EQUAL(OUString("Book Antiqua"), getProperty<OUString>(xRun, "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo49659, "fdo49659.rtf")
{
    // Both tables were ignored: 1) was in the header, 2) was ignored due to missing empty par at the end of the doc
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    // The graphic was also empty
    uno::Reference<beans::XPropertySet> xGraphic(getProperty< uno::Reference<beans::XPropertySet> >(getShape(1), "Graphic"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL, getProperty<sal_Int8>(xGraphic, "GraphicType"));
}

DECLARE_RTFEXPORT_TEST(testFdo46966, "fdo46966.rtf")
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(720)), getProperty<sal_Int32>(xPropertySet, "TopMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo76633, "fdo76633.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo48033, "fdo48033.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo53594, "fdo53594.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo36089, "fdo36089.rtf")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(-50), getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEscapement"));
}

DECLARE_RTFEXPORT_TEST(testFdo48446, "fdo48446.rtf")
{
    OUString aExpected(u"\u0418\u043C\u044F");
    getParagraph(1, aExpected);
}

DECLARE_RTFEXPORT_TEST(testFdo47495, "fdo47495.rtf")
{
    // Used to have 4 paragraphs, as a result the original bugdoc had 2 pages instead of 1.
    CPPUNIT_ASSERT_EQUAL(2, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testAllGapsWord, "all_gaps_word.rtf")
{
    BorderTest borderTest;
    BorderTest::testTheBorders(mxComponent, false);
}

DECLARE_RTFEXPORT_TEST(testFdo52389, "fdo52389.rtf")
{
    // The last '!' character at the end of the document was lost
    CPPUNIT_ASSERT_EQUAL(6, getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo49655, "fdo49655.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo62805, "fdo62805.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo52475, "fdo52475.rtf")
{
    // The problem was that \chcbpat0 resulted in no color, instead of COL_AUTO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), getProperty<sal_Int32>(getRun(getParagraph(1), 3), "CharBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo55493, "fdo55493.rtf")
{
    // The problem was that the width of the PNG was detected as 15,24cm, instead of 3.97cm
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3969), xShape->getSize().Width);
}

DECLARE_RTFEXPORT_TEST(testCopyPastePageStyle, "copypaste-pagestyle.rtf")
{
    // The problem was that RTF import during copy&paste did not ignore page styles.
    // Once we have more copy&paste tests, makes sense to refactor this to some helper method.
    paste("copypaste-pagestyle-paste.rtf");

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), getProperty<sal_Int32>(xPropertySet, "Width")); // Was letter, i.e. 21590
}

DECLARE_RTFEXPORT_TEST(testCopyPasteFootnote, "copypaste-footnote.rtf")
{
    // The RTF import did not handle the case when the position wasn't the main document XText, but something different, e.g. a footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    paste("copypaste-footnote-paste.rtf", xTextRange);

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo61193, "hello.rtf")
{
    // Pasting content that contained a footnote caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo61193.rtf", xEnd);
}

DECLARE_RTFEXPORT_TEST(testTdf108123, "hello.rtf")
{
    // This crashed, the shape push/pop and table manager stack went out of
    // sync -> we tried to de-reference an empty stack.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("tdf108123.rtf", xEnd);
}

DECLARE_RTFEXPORT_TEST(testShptxtPard, "shptxt-pard.rtf")
{
    // The problem was that \pard inside \shptxt caused loss of shape text
    uno::Reference<text::XText> xText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("shape text"), xText->getString());
}

DECLARE_RTFEXPORT_TEST(testDoDhgt, "do-dhgt.rtf")
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

DECLARE_RTFEXPORT_TEST(testDplinehollow, "dplinehollow.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    table::BorderLine2 line(getProperty<table::BorderLine2>(xPropertySet, "TopBorder"));
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE, line.LineStyle);
}

DECLARE_RTFEXPORT_TEST(testLeftmarginDefault, "leftmargin-default.rtf")
{
    // The default left/right margin was incorrect when the top margin was set to zero.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2540), getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testDppolyline, "dppolyline.rtf")
{
    // This was completely ignored, for now, just make sure we have all 4 lines.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo56512, "fdo56512.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    OUString aExpected(u"\u05E2\u05D5\u05E1\u05E7 \u05DE\u05D5\u05E8\u05E9\u05D4 ");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo52989, "fdo52989.rtf")
{
    // Same as n#192129, but for JPEG files.
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    OString aMessage("xShape->getSize().Width() = ");
    aMessage += OString::number(xShape->getSize().Width);

    // This was 2, should be 423 (or 369?).
    CPPUNIT_ASSERT_MESSAGE(aMessage.getStr(), xShape->getSize().Width >= 273);
}

DECLARE_RTFEXPORT_TEST(testFdo54473, "fdo54473.rtf")
{
    // The problem was that character styles were not imported due to a typo.
    CPPUNIT_ASSERT_EQUAL(OUString("Anot"), getProperty<OUString>(getRun(getParagraph(1), 1, "Text "), "CharStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("ForeignTxt"), getProperty<OUString>(getRun(getParagraph(1), 3, "character "), "CharStyleName"));
}

DECLARE_RTFEXPORT_TEST(testFdo49934, "fdo49934.rtf")
{
    // Column break without columns defined should be a page break, but it was just ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo57886, "fdo57886.rtf")
{
    // Was 'int from <?> to <?> <?>'.
    CPPUNIT_ASSERT_EQUAL(OUString("int from {firstlower} to {firstupper} {firstbody}"), getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_RTFEXPORT_TEST(testFdo58076, "fdo58076.rtf")
{
    // An additional section was created, so the default page style didn't have the custom margins.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2251), getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1752), getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo57678, "fdo57678.rtf")
{
    // Paragraphs of the two tables were not converted to tables.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo54612, "fdo54612.rtf")
{
    // \dpptx without a \dppolycount caused a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo58933, "fdo58933.rtf")
{
    // The problem was that the table had an additional cell in its first line.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 4.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getCellNames().getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo44053, "fdo44053.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The with of the table's A1 and A2 cell should equal.
    CPPUNIT_ASSERT_EQUAL(getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators")[0].Position,
                         getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFEXPORT_TEST(testFdo48440, "fdo48440.rtf")
{
    // Page break was ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo58646line, "fdo58646line.rtf")
{
    // \line symbol was ignored
    getParagraph(1, "foo\nbar");
}

DECLARE_RTFEXPORT_TEST(testFdo78502, "fdo78502.rtf")
{
    // ";" separators were inserted as text
    getParagraph(1, "foo");
}

DECLARE_RTFEXPORT_TEST(testFdo58646, "fdo58646.rtf")
{
    // Page break was ignored inside a continuous section, on title page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo59419, "fdo59419.rtf")
{
    // Junk to be ignored broke import of the table.
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xTables->getCount());
}

DECLARE_RTFEXPORT_TEST(testHexCRLF, "hexcrlf.rtf")
{
    // hex-escaped \r and \n should create a paragraph break
    getParagraph(1, "foo");
    getParagraph(2, "bar");
    getParagraph(3, "baz");
    getParagraph(4, "");
    getParagraph(5, "quux");
}

DECLARE_RTFEXPORT_TEST(testFdo58076_2, "fdo58076-2.rtf")
{
    // Position of the picture wasn't correct.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(8345)), getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
}

DECLARE_RTFEXPORT_TEST(testFdo59638, "fdo59638.rtf")
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
            CPPUNIT_ASSERT_EQUAL(OUString(u"\uF0B7"), rProp.Value.get<OUString>());
            return;
        }
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_RTFEXPORT_TEST(testFdo60722, "fdo60722.rtf")
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

DECLARE_RTFEXPORT_TEST(testDoDhgtOld, "do-dhgt-old.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo61909, "fdo61909.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
    // Was the Writer default font.
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"), getProperty<OUString>(xTextRange, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(xTextRange, "CharBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo62288, "fdo62288.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo37716, "fdo37716.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFrames(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);
    // \nowrap got ignored, so Surround was text::WrapTextMode_PARALLEL
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(xFrames->getByIndex(0), "Surround"));
}

DECLARE_RTFEXPORT_TEST(testFdo51916, "fdo51916.rtf")
{
    // Complex nested table caused a crash.
}

DECLARE_RTFEXPORT_TEST(testFdo63023, "fdo63023.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    // Back color was black (0) in the header, due to missing color table in the substream.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xFFFF99), getProperty<sal_Int32>(getRun(getParagraphOfText(1, xHeaderText), 1), "CharBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo42109, "fdo42109.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // Make sure the page number is imported as a field in the B1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1), "TextPortionType"));
}

DECLARE_RTFEXPORT_TEST(testFdo62977, "fdo62977.rtf")
{
    // The middle character was imported as '?' instead of the proper unicode value.
    getRun(getParagraph(1), 1, OUString(u"\u5E74\uFF14\u6708"));
}

DECLARE_RTFEXPORT_TEST(testN818997, "n818997.rtf")
{
    // \page was ignored between two \shp tokens.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo64671, "fdo64671.rtf")
{
    // Additional '}' was inserted before the special character.
    getRun(getParagraph(1), 1, OUString(u"\u017D"));
}

DECLARE_RTFEXPORT_TEST(testFdo62044, "fdo62044.rtf")
{
    // The problem was that RTF import during copy&paste did not ignore existing paragraph styles.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo62044-paste.rtf", xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(10.f, getProperty<float>(xPropertySet, "CharHeight")); // Was 18, i.e. reset back to original value.
}

DECLARE_RTFEXPORT_TEST(testFdo70578, "fdo70578.rtf")
{
    // Style without explicit \s0 was not imported as the default style
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Subtitle"), uno::UNO_QUERY);
    uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"), xStyle->getParentStyle());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testN825305, "n825305.rtf")
{
    // The problem was that the textbox wasn't transparent, due to unimplemented fFilled == 0.
    uno::Reference<beans::XPropertyState> xPropertyState(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100), getProperty<sal_Int32>(getShape(2), "BackColorTransparency"));
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("BackColorTransparency");
    // Was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
}

DECLARE_RTFEXPORT_TEST(testTdf106953, "tdf106953.rtf")
{
    auto xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), aRule["IndentAt"].get<sal_Int32>());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRule["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testTdf112208, "tdf112208_hangingIndent.rtf")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-2000), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testParaBottomMargin, "para-bottom-margin.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo66040, "fdo66040.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo77996, "fdo77996.rtf")
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
    OUString aTitle(u"\u53A6\u95E8\u94A8\u4E1A\u80A1\u4EFD\u6709\u9650\u516C\u53F8");
    CPPUNIT_ASSERT_EQUAL(aTitle, xProps->getTitle());
    uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("jay"), getProperty<OUString>(xUDProps, "Operator"));

    // fdo#80486 also check that the ftnsep doesn't insert paragraph breaks
    getParagraph(1, aTitle);
}

DECLARE_RTFEXPORT_TEST(testFdo47802, "fdo47802.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // Shape inside table was ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo39001, "fdo39001.rtf")
{
    // Document was of 4 pages, \sect at the end of the doc wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo54900, "fdo54900.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testFdo64637, "fdo64637.rtf")
{
    // The problem was that the custom "Company" property was added twice, the second invocation resulted in an exception.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xDocumentPropertiesSupplier->getDocumentProperties()->getUserDefinedProperties(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getProperty<OUString>(xPropertySet, "Company"));
}

DECLARE_RTFEXPORT_TEST(testN820504, "n820504.rtf")
{
    // The shape was anchored at-page instead of at-character (that's incorrect as Word only supports at-character and as-character).
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testFdo67498, "fdo67498.rtf")
{
    // Left margin of the default page style wasn't set (was 2000).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(5954)), getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo63428, "hello.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo44715, "fdo44715.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Style information wasn't reset, which caused character height to be 16.
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(getParagraphOfText(2, xCell->getText()), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo68076, "fdo68076.rtf")
{
    // Encoding of the last char was wrong (more 'o' than 'y').
    OUString aExpected(u"\u041E\u0431\u044A\u0435\u043A\u0442 \u2013 \u0443");
    getParagraph(1, aExpected);
}

DECLARE_RTFEXPORT_TEST(testFdo69384, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("fdo69384-paste.rtf", xEnd);

    // Import got interrupted in the middle of style sheet table import,
    // resulting in missing styles and text.
    getStyles("ParagraphStyles")->getByName("Text body justified");
}

DECLARE_RTFEXPORT_TEST(testFdo70221, "fdo70221.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testCp1000018, "cp1000018.rtf")
{
    // The problem was that the empty paragraph at the end of the footnote got
    // lost during import.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    OUString const aExpected("Footnote first line.\n");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}
#endif

DECLARE_RTFEXPORT_TEST(testFdo94835, "fdo94835.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), xDraws->getCount());
}
DECLARE_RTFEXPORT_TEST(testNestedTable, "rhbz1065629.rtf")
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

DECLARE_RTFEXPORT_TEST(testFooterPara, "footer-para.rtf")
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

DECLARE_RTFEXPORT_TEST(testCp1000016, "hello.rtf")
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

DECLARE_RTFEXPORT_TEST(testFdo65090, "fdo65090.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had 3 cells, instead of a horizontally merged one and a normal one (2 -> 1 separator).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFEXPORT_TEST(testTableBorderDefaults, "fdo68779.rtf")
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

DECLARE_RTFEXPORT_TEST(testShpzDhgt, "shpz-dhgt.rtf")
{
    // Test that shpz has priority over dhgt and not the other way around.
    // Drawpage is sorted by ZOrder, so first should be red (back).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xff0000), getProperty<sal_Int32>(getShape(1), "FillColor"));
    // Second (front) should be green.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x00ff00), getProperty<sal_Int32>(getShape(2), "FillColor"));
}

DECLARE_RTFEXPORT_TEST(testLevelfollow, "levelfollow.rtf")
{
    uno::Reference<container::XIndexAccess> xNum1Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::LISTTAB), comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, tab

    uno::Reference<container::XIndexAccess> xNum2Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum2"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::SPACE), comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, space

    uno::Reference<container::XIndexAccess> xNum3Levels = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum3"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(SvxNumberFormat::NOTHING), comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))["LabelFollowedBy"].get<sal_Int16>()); // first level, nothing
}

DECLARE_RTFEXPORT_TEST(testCharColor, "char-color.rtf")
{
    // This was -1: character color wasn't set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x365F91), getProperty<sal_Int32>(getParagraph(1), "CharColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo69289, "fdo69289.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // There were only 2 cells (1 separators) in the table, should be 3 (2 separators).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFEXPORT_TEST(testDptxbxRelation, "dptxbx-relation.rtf")
{
    // This was FRAME, not PAGE_FRAME, even if dobxpage is in the document.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME, getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

DECLARE_RTFEXPORT_TEST(testDprectAnchor, "dprect-anchor.rtf")
{
    // This was at-page, which is not something Word supports, so clearly an import error.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER, getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testFdo76628, "fdo76628.rtf")
{
    OUString aExpected(u"\u041E\u0411\u0420\u0410\u0417\u0415\u0426");
    // Should be 'SAMPLE' in Russian, was garbage.
    getParagraph(1, aExpected);

    uno::Reference<text::XText> xHeaderText = getProperty< uno::Reference<text::XText> >(getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    OUString aExpectedHeader(u"\u041F\u043E\u0434\u0433\u043E\u0442\u043E\u0432\u043B\u0435\u043D\u043E");
    // Should be 'prepared' in Russian, was garbage.
    getParagraphOfText(1, xHeaderText, aExpectedHeader);
}

DECLARE_RTFEXPORT_TEST(testFdo77267, "fdo77267.rtf")
{
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testFdo75735, "fdo75735.rtf")
{
    // Number of tabstops in the second paragraph should be 3, was 6.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty< uno::Sequence<style::TabStop> >(getParagraph(2), "ParaTabStops").getLength());
}

DECLARE_RTFEXPORT_TEST(testFontOverride, "font-override.rtf")
{
    // This was "Times New Roman".
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"), getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo73241, "fdo73241.rtf")
{
    // This was 2, page break in table wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo80905, "fdo80905.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    xFields->nextElement();
    // The problem was that there was only one field in the document, but there should be true.
    CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(xFields->hasMoreElements()));
}

DECLARE_RTFEXPORT_TEST(testFdo82106, "fdo82106.rtf")
{
    // Tab was missing after footnote not containing a tab.
    getParagraph(2, "before\tafter");
}

DECLARE_RTFEXPORT_TEST(testFdo74229, "fdo74229.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, due to ignoring RTF_TRGAPH.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(67)), getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testFdo79959, "fdo79959.rtf")
{
    // This was false, as the style was imported as " Test", i.e. no whitespace stripping.
    CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(getStyles("ParagraphStyles")->hasByName("Test")));
}

DECLARE_RTFEXPORT_TEST(testFdo82078, "fdo82078.rtf")
{
    // This was awt::FontWeight::BOLD, i.e. the second run was bold, when it should be normal.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(getRun(getParagraph(1), 2), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testCsBold, "cs-bold.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run was bold, when it should be bold (applied character style without direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo84679, "fdo84679.rtf")
{
    // The problem was that the paragraph in A1 had some bottom margin, but it should not.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 282.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo83464, "fdo83464.rtf")
{
    // Problem was that the text in the textframe had wrong font.
    uno::Reference<text::XTextRange> xFrameText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFrameText->getString());
    // This was Times New Roman.
    CPPUNIT_ASSERT_EQUAL(OUString("Verdana"), getProperty<OUString>(getRun(getParagraphOfText(1, xFrameText->getText()), 1), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo86761, "fdo86761.rtf")
{
    // This was 26, even if the picture should have no border, due to fLine=0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_RTFEXPORT_TEST(testFdo82859, "fdo82859.rtf")
{
    // This was 0: "0xffffff" was converted to 0, i.e. the background was black instead of the default.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(getShape(1), "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo82076, "fdo82076.rtf")
{
    // Footnote position was wrong: should be at the end of the B1 cell.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // This resulted in container::NoSuchElementException: the footnote was at the start of the A1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote"), getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 2), "TextPortionType"));
}

DECLARE_RTFEXPORT_TEST(testFdo83204, "fdo83204.rtf")
{
    // This was Standard, \sN was ignored after \bkmkstart and \pard.
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"), getProperty<OUString>(getParagraph(1), "ParaStyleName"));
}

DECLARE_RTFEXPORT_TEST(testFdo85889pc, "fdo85889-pc.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected(u"\u00B1\u2265\u2264");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo85889pca, "fdo85889-pca.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected(u"\u00B1\u2017\u00BE");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo85889mac, "fdo85889-mac.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);

    OUString aExpected(u"\u00D2\u00DA\u00DB");
    CPPUNIT_ASSERT_EQUAL(aExpected, xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo72031, "fdo72031.rtf")
{
    OUString aExpected(u"\uF0C5");
    CPPUNIT_ASSERT_EQUAL(aExpected, getRun(getParagraph(1), 1)->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo86750, "fdo86750.rtf")
{
    // This was 'HYPERLINK#anchor', the URL of the hyperlink had the field type as a prefix, leading to broken links.
    CPPUNIT_ASSERT_EQUAL(OUString("#anchor"), getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
}

DECLARE_RTFEXPORT_TEST(testTdf88811, "tdf88811.rtf")
{
    // The problem was that shapes anchored to the paragraph that is moved into a textframe were lost, so this was 2.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xDrawPage->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo49893_2, "fdo49893-2.rtf")
{
    // Ensure that header text exists on each page (especially on second page)
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"),  parseDump("/root/page[3]/header/txt/text()"));
}

DECLARE_RTFEXPORT_TEST(testFdo89496, "fdo89496.rtf")
{
    // Just ensure that document is loaded and shape exists
    uno::Reference<drawing::XShape> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
}

DECLARE_RTFEXPORT_TEST(testFdo75614, "tdf75614.rtf")
{
    // Text after the footnote was missing, so this resulted in a css::container::NoSuchElementException.
    CPPUNIT_ASSERT_EQUAL(OUString("after."), getRun(getParagraph(1), 3)->getString());
}

DECLARE_RTFEXPORT_TEST(mathtype, "mathtype.rtf")
{
    OUString aFormula = getFormula(getRun(getParagraph(1), 1));
    CPPUNIT_ASSERT(!aFormula.isEmpty());
}

DECLARE_RTFEXPORT_TEST(testTdf86182, "tdf86182.rtf")
{
    // Writing mode was the default, i.e. text::WritingMode2::CONTEXT.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(getParagraph(1), "WritingMode"));
}

DECLARE_RTFEXPORT_TEST(testTdf91074, "tdf91074.rtf")
{
    // The file failed to load, as the border color was imported using the LineColor UNO property.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_LIGHTRED), getProperty<table::BorderLine2>(xShape, "TopBorder").Color);
}

DECLARE_RTFEXPORT_TEST(testTdf90260Nopar, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("tdf90260-nopar.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testTdf86814, "tdf86814.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold (applied paragraph style with direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD, getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testTdf108416, "tdf108416.rtf")
{
    uno::Reference<container::XNameAccess> xCharacterStyles(getStyles("CharacterStyles"));
    uno::Reference<beans::XPropertySet> xListLabel(xCharacterStyles->getByName("ListLabel 1"), uno::UNO_QUERY);
    // This was awt::FontWeight::BOLD, list numbering got an unexpected bold formatting.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL, getProperty<float>(xListLabel, "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testBinSkipping, "bin-skipping.rtf")
{
    // before, it was importing '/nMUST NOT IMPORT'
    CPPUNIT_ASSERT_EQUAL(OUString("text"), getRun(getParagraph(1), 1)->getString());
}

DECLARE_RTFEXPORT_TEST(testTdf92061, "tdf92061.rtf")
{
    // This was "C", i.e. part of the footnote ended up in the body text.
    CPPUNIT_ASSERT_EQUAL(OUString("body-after"), getRun(getParagraph(1), 3)->getString());
}

DECLARE_RTFEXPORT_TEST(testTdf92481, "tdf92481.rtf")
{
    // This was 0, RTF_WIDOWCTRL was not imported.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(2), getProperty<sal_Int8>(getParagraph(1), "ParaWidows"));
}

DECLARE_RTFEXPORT_TEST(testTdf94456, "tdf94456.rtf")
{
    // Paragraph left margin and first line indent wasn't imported correctly.

    // This was 1270.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(762), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    // This was -635.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-762), getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_RTFEXPORT_TEST(testTdf94435, "tdf94435.rtf")
{
    // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf54584, "tdf54584.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \PAGE was ignored, so no fields were in document -> exception was thrown
    CPPUNIT_ASSERT_NO_THROW_MESSAGE("No fields in document found: field \"\\PAGE\" was not properly read",
                                    xFields->nextElement());
}

DECLARE_RTFEXPORT_TEST(testTdf96308Deftab, "tdf96308-deftab.rtf")
{
    uno::Reference<lang::XMultiServiceFactory> xTextFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefaults(xTextFactory->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    // This was 1270 as \deftab was ignored on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(284)), getProperty<sal_Int32>(xDefaults, "TabStopDistance"));
}

DECLARE_RTFEXPORT_TEST(testLandscape, "landscape.rtf")
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

DECLARE_RTFEXPORT_TEST(testTdf97035, "tdf97035.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // First cell width of the second row should be 2299
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2299), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(1), "TableColumnSeparators")[0].Position);
}

DECLARE_RTFEXPORT_TEST(testTdf87034, "tdf87034.rtf")
{
    // This was A1BC34D, i.e. the first "super" text portion was mis-imported,
    // and was inserted instead right before the second "super" text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("A1B3C4D"), getParagraph(1)->getString());
}

DECLARE_RTFEXPORT_TEST(testClassificatonPasteLevels, "classification-confidential.rtf")
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

DECLARE_RTFEXPORT_TEST(testTdf95707, "tdf95707.rtf")
{
    // Graphic was replaced with a "Read-Error" placeholder.
    CPPUNIT_ASSERT(getProperty<OUString>(getShape(1), "GraphicURL") != "vnd.sun.star.GraphicObject:0000000000000000000000000000000000000000");
}

DECLARE_RTFEXPORT_TEST(testTdf96275, "tdf96275.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(3, xCell->getText());
    // This was text: the shape's frame was part of the 1st paragraph instead of the 3rd one.
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"), getProperty<OUString>(getRun(xParagraph, 1), "TextPortionType"));
}

DECLARE_RTFEXPORT_TEST(testTdf82073, "tdf82073.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was -1: the background color was automatic, not black.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf74795, "tdf74795.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, \trpaddl was ignored on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));

    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Make sure that the scope of the default is only one row.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf77349, "tdf77349.rtf")
{
    uno::Reference<container::XNamed> xImage(getShape(1), uno::UNO_QUERY);
    // This was empty: imported image wasn't named automatically.
    CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xImage->getName());
}

DECLARE_RTFEXPORT_TEST(testTdf50821, "tdf50821.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, \trpaddfl was mishandled on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(191), getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf100507, "tdf100507.rtf")
{
    // This was 0: left margin of the first paragraph was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6618), getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf44986, "tdf44986.rtf")
{
    // Check that the table at the second paragraph.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // Check the first row of the table, it should have two cells (one separator).
    // This was 0: the first row had no separators, so it had only one cell, which was too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty< uno::Sequence<text::TableColumnSeparator> >(xTableRows->getByIndex(0), "TableColumnSeparators").getLength());
}

DECLARE_RTFEXPORT_TEST(testTdf90697, "tdf90697.rtf")
{
    // We want section breaks to be seen as section breaks, not as page breaks,
    // so this document should have only one page, not three.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf104317, "tdf104317.rtf")
{
    // This failed to load, we tried to set CustomShapeGeometry on a line shape.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

DECLARE_RTFEXPORT_TEST(testTdf104744, "tdf104744.rtf")
{
    auto xRules = getProperty< uno::Reference<container::XIndexAccess> >(getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), aRule["IndentAt"].get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testTdf105852, "tdf105852.rtf")
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

DECLARE_RTFEXPORT_TEST(testTdf104287, "tdf104287.rtf")
{
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    // This failed, the bitmap had no valid anchor.
    CPPUNIT_ASSERT(xShape->getAnchor().is());
}

DECLARE_RTFEXPORT_TEST(testTdf105729, "tdf105729.rtf")
{
    // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc from style.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf106694, "tdf106694.rtf")
{
    auto aTabs = getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    // This was 0, tab position was incorrect, looked like it was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14605), aTabs[0].Position);
}

DECLARE_RTFEXPORT_TEST(testTdf107116, "tdf107116.rtf")
{
    // This was 0, upper border around text (and its distance) was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(120)), getProperty<sal_Int32>(getParagraph(2), "TopBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf106950, "tdf106950.rtf")
{
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    // This was ParagraphAdjust_LEFT, trying to set CharShadingValue on a
    // paragraph style thrown an exception, and remaining properties were not
    // set.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
