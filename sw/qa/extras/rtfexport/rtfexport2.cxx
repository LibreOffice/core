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
#include <comphelper/sequenceashashmap.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <bordertest.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }

    virtual std::unique_ptr<Resetter> preTest(const char* filename) override
    {
        m_aSavedSettings = Application::GetSettings();
        if (OString(filename) == "fdo48023.rtf")
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ru"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        else if (OString(filename) == "fdo44211.rtf")
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("lt"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    AllSettings m_aSavedSettings;
};

DECLARE_RTFEXPORT_TEST(testFdo45553, "fdo45553.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                       uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
        {
            uno::Reference<text::XTextRange> xRange(xRangeEnum->nextElement(), uno::UNO_QUERY);
            OUString aStr = xRange->getString();
            if (aStr == "space-before")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(120)),
                                     getProperty<sal_Int32>(xRange, "ParaTopMargin"));
            else if (aStr == "space-after")
                CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(240)),
                                     getProperty<sal_Int32>(xRange, "ParaBottomMargin"));
        }
    }
}

DECLARE_RTFEXPORT_TEST(testN192129, "n192129.rtf")
{
    // We expect that the result will be 16x16px.
    Size aExpectedSize(16, 16);
    MapMode aMap(MapUnit::Map100thMM);
    aExpectedSize = Application::GetDefaultDevice()->PixelToLogic(aExpectedSize, aMap);

    uno::Reference<text::XTextGraphicObjectsSupplier> xTextGraphicObjectsSupplier(mxComponent,
                                                                                  uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(
        xTextGraphicObjectsSupplier->getGraphicObjects(), uno::UNO_QUERY);
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

DECLARE_RTFEXPORT_TEST(testFdo45543, "fdo45543.rtf") { CPPUNIT_ASSERT_EQUAL(5, getLength()); }

DECLARE_RTFEXPORT_TEST(testFdo42465, "fdo42465.rtf") { CPPUNIT_ASSERT_EQUAL(3, getLength()); }

DECLARE_RTFEXPORT_TEST(testFdo45187, "fdo45187.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // There should be two shapes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // They should be anchored to different paragraphs.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xTextDocument->getText(),
                                                              uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor0
        = uno::Reference<text::XTextContent>(xDraws->getByIndex(0), uno::UNO_QUERY)->getAnchor();
    uno::Reference<text::XTextRange> xAnchor1
        = uno::Reference<text::XTextContent>(xDraws->getByIndex(1), uno::UNO_QUERY)->getAnchor();
    // Was 0 ("starts at the same position"), should be 1 ("starts before")
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xTextRangeCompare->compareRegionStarts(xAnchor0, xAnchor1));
}

DECLARE_RTFEXPORT_TEST(testN750757, "n750757.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xParaEnum->nextElement(), "ParaContextMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo45563, "fdo45563.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
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
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();

    // First paragraph: the parameter of \up was ignored
    uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                   uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPropertySet(xRangeEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(36), getProperty<sal_Int32>(xPropertySet, "CharEscapement"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100),
                         getProperty<sal_Int32>(xPropertySet, "CharEscapementHeight"));

    // Second paragraph: Word vs Writer border default problem
    CPPUNIT_ASSERT_EQUAL(
        sal_uInt32(26),
        getProperty<table::BorderLine2>(xParaEnum->nextElement(), "TopBorder").LineWidth);

    // Finally, make sure that we have two pages
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testN751020, "n751020.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(200)),
                         getProperty<sal_Int32>(xParaEnum->nextElement(), "ParaBottomMargin"));
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
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    while (xParaEnum->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xRangeEnumAccess(xParaEnum->nextElement(),
                                                                       uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xRangeEnum = xRangeEnumAccess->createEnumeration();
        while (xRangeEnum->hasMoreElements())
            CPPUNIT_ASSERT_EQUAL(style::CaseMap::UPPERCASE,
                                 getProperty<sal_Int16>(xRangeEnum->nextElement(), "CharCaseMap"));
    }
}

DECLARE_RTFEXPORT_TEST(testFdo81892, "fdo81892.rtf")
{
    // table was not centered
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                         getProperty<sal_Int16>(xTable, "HoriOrient"));

    // fdo#81893: paragraph with \page was not centered
    uno::Reference<text::XTextRange> xPara(getParagraph(2, "Performance"));
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testFdo45394, "fdo45394.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    OUString aActual = xHeaderText->getString();
    // Encoding in the header was wrong.
    OUString aExpected(u"\u041F\u041A \u0420\u0418\u041A");
    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);

    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo48104, "fdo48104.rtf") { CPPUNIT_ASSERT_EQUAL(2, getPages()); }

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
    uno::Reference<beans::XPropertySet> xFirstPage(xPageStyles->getByName("First Page"),
                                                   uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefault(xPageStyles->getByName("Standard"),
                                                 uno::UNO_QUERY);
    sal_Int32 nFirstTop = 0, nDefaultTop = 0, nDefaultHeader = 0;
    xFirstPage->getPropertyValue("TopMargin") >>= nFirstTop;
    xDefault->getPropertyValue("TopMargin") >>= nDefaultTop;
    xDefault->getPropertyValue("HeaderHeight") >>= nDefaultHeader;
    CPPUNIT_ASSERT_EQUAL(nFirstTop, nDefaultTop + nDefaultHeader);
}

DECLARE_RTFEXPORT_TEST(testFdo39053, "fdo39053.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    int nAsCharacter = 0;
    for (int i = 0; i < xDraws->getCount(); ++i)
        if (getProperty<text::TextContentAnchorType>(xDraws->getByIndex(i), "AnchorType")
            == text::TextContentAnchorType_AS_CHARACTER)
            nAsCharacter++;
    // The image in binary format was ignored.
    CPPUNIT_ASSERT_EQUAL(1, nAsCharacter);
}

DECLARE_RTFEXPORT_TEST(testFdo48356, "fdo48356.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
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
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    CPPUNIT_ASSERT(xParaEnum->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(
        style::LineSpacingMode::MINIMUM,
        getProperty<style::LineSpacing>(xParaEnum->nextElement(), "ParaLineSpacing").Mode);
}

DECLARE_RTFEXPORT_TEST(testFdo48193, "fdo48193.rtf") { CPPUNIT_ASSERT_EQUAL(7, getLength()); }

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
    sal_Int32 nExpected
        = xNumberSupplier->getNumberFormats()->addNewConverted("d MMMM yyyy", aUSLocale, aFRLocale);

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
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
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
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
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);

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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                         getProperty<sal_Int32>(getRun(getParagraph(1), 1), "CharBackColor"));
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
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());

    // The graphic was also empty
    uno::Reference<beans::XPropertySet> xGraphic(
        getProperty<uno::Reference<beans::XPropertySet>>(getShape(1), "Graphic"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(graphic::GraphicType::PIXEL,
                         getProperty<sal_Int8>(xGraphic, "GraphicType"));
}

DECLARE_RTFEXPORT_TEST(testFdo46966, "fdo46966.rtf")
{
    /*
     * The problem was the top margin was 1440 (1 inch), but it should be 720 (0.5 inch).
     *
     * xray ThisComponent.StyleFamilies.PageStyles.Default.TopMargin
     */
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(720)),
                         getProperty<sal_Int32>(xPropertySet, "TopMargin"));
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
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
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
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
}

DECLARE_RTFEXPORT_TEST(testFdo36089, "fdo36089.rtf")
{
    CPPUNIT_ASSERT_EQUAL(sal_Int16(-50),
                         getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEscapement"));
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
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
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
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo52475, "fdo52475.rtf")
{
    // The problem was that \chcbpat0 resulted in no color, instead of COL_AUTO.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1),
                         getProperty<sal_Int32>(getRun(getParagraph(1), 3), "CharBackColor"));
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
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/copypaste-pagestyle-paste.rtf", xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("PageStyles")->getByName("Standard"),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(21001),
                         getProperty<sal_Int32>(xPropertySet, "Width")); // Was letter, i.e. 21590
}

#if !defined(_WIN32)

DECLARE_RTFEXPORT_TEST(testCopyPasteFootnote, "copypaste-footnote.rtf")
{
    // The RTF import did not handle the case when the position wasn't the main document XText, but something different, e.g. a footnote.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    paste("rtfexport/data/copypaste-footnote-paste.rtf", xTextRange);

    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), xTextRange->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo63428, "hello.rtf")
{
    // Pasting content that contained an annotation caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/fdo63428.rtf", xEnd);

    // Additionally, commented range was imported as a normal comment.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(getParagraph(1), 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(getParagraph(1), 4), "TextPortionType"));
}

#endif

DECLARE_RTFEXPORT_TEST(testFdo69384, "fdo69384-paste.rtf")
{
    // Ensure non-default style is loaded
    getStyles("ParagraphStyles")->getByName("Text body justified");
    // Ensure default styles were modified, vs testFdo69384Inserted where it is not
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Text Body"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(68.f, getProperty<float>(xPropertySet, "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo69384Inserted, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/fdo69384-paste.rtf", xEnd);

    // During insert of the RTF document we do not change pre-existing styles
    // vs testFdo69384 where it is
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Text Body"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(12.f, getProperty<float>(xPropertySet, "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo61193, "hello.rtf")
{
    // Pasting content that contained a footnote caused a crash.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/fdo61193.rtf", xEnd);
}

DECLARE_RTFEXPORT_TEST(testTdf108123, "hello.rtf")
{
    // This crashed, the shape push/pop and table manager stack went out of
    // sync -> we tried to de-reference an empty stack.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/tdf108123.rtf", xEnd);
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
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    for (int i = 0; i < xDraws->getCount(); ++i)
    {
        sal_Int32 nFillColor = getProperty<sal_Int32>(xDraws->getByIndex(i), "FillColor");
        if (nFillColor == 0xc0504d) // red
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x9bbb59) // green
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
        else if (nFillColor == 0x4f81bd) // blue
            CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                                 getProperty<sal_Int32>(xDraws->getByIndex(i), "ZOrder"));
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
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(2540),
        getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testDppolyline, "dppolyline.rtf")
{
    // This was completely ignored, for now, just make sure we have all 4 lines.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo56512, "fdo56512.rtf")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(
        OUString("Anot"),
        getProperty<OUString>(getRun(getParagraph(1), 1, "Text "), "CharStyleName"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("ForeignTxt"),
        getProperty<OUString>(getRun(getParagraph(1), 3, "character "), "CharStyleName"));
}

DECLARE_RTFEXPORT_TEST(testFdo49934, "fdo49934.rtf")
{
    // Column break without columns defined should be a page break, but it was just ignored.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo57886, "fdo57886.rtf")
{
    // Was 'int from <?> to <?> <?>'.
    CPPUNIT_ASSERT_EQUAL(OUString("int from {firstlower} to {firstupper} {firstbody}"),
                         getFormula(getRun(getParagraph(1), 1)));
}

DECLARE_RTFEXPORT_TEST(testFdo58076, "fdo58076.rtf")
{
    // An additional section was created, so the default page style didn't have the custom margins.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("PageStyles")->getByName("Standard"),
                                               uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2251), getProperty<sal_Int32>(xStyle, "LeftMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1752), getProperty<sal_Int32>(xStyle, "RightMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "TopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(635), getProperty<sal_Int32>(xStyle, "BottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo57678, "fdo57678.rtf")
{
    // Paragraphs of the two tables were not converted to tables.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextTablesSupplier->getTextTables(),
                                                         uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xIndexAccess->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo54612, "fdo54612.rtf")
{
    // \dpptx without a \dppolycount caused a crash.
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testFdo58933, "fdo58933.rtf")
{
    // The problem was that the table had an additional cell in its first line.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was 4.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), xTable->getCellNames().getLength());
}

DECLARE_RTFEXPORT_TEST(testFdo44053, "fdo44053.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The with of the table's A1 and A2 cell should equal.
    CPPUNIT_ASSERT_EQUAL(getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), "TableColumnSeparators")[0]
                             .Position,
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(1), "TableColumnSeparators")[0]
                             .Position);
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
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(8345)),
                         getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
}

DECLARE_RTFEXPORT_TEST(testFdo59638, "fdo59638.rtf")
{
    // The problem was that w:lvlOverride inside w:num was ignores by dmapper.

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, Color(getProperty<sal_uInt32>(xShape, "FillColor")));

    xShape.set(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<sal_Int32>(xShape, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("b"), xShape->getString());
}

DECLARE_RTFEXPORT_TEST(testFdo61909, "fdo61909.rtf")
{
    uno::Reference<text::XTextRange> xTextRange = getRun(getParagraph(1), 1);
    // Was the Writer default font.
    CPPUNIT_ASSERT_EQUAL(OUString("Courier New"),
                         getProperty<OUString>(xTextRange, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, Color(getProperty<sal_uInt32>(xTextRange, "CharBackColor")));
}

DECLARE_RTFEXPORT_TEST(testFdo62288, "fdo62288.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
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
    uno::Reference<container::XIndexAccess> xFrames(xTextFramesSupplier->getTextFrames(),
                                                    uno::UNO_QUERY);
    // \nowrap got ignored, so Surround was text::WrapTextMode_PARALLEL
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_NONE,
                         getProperty<text::WrapTextMode>(xFrames->getByIndex(0), "Surround"));
}

DECLARE_RTFEXPORT_TEST(testFdo51916, "fdo51916.rtf")
{
    // Complex nested table caused a crash.
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
