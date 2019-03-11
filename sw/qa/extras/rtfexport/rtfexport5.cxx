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
        if (OString(filename) == "fdo72031.rtf")
        {
            std::unique_ptr<Resetter> pResetter(
                new Resetter([this]() { Application::SetSettings(this->m_aSavedSettings); }));
            AllSettings aSettings(m_aSavedSettings);
            aSettings.SetLanguageTag(LanguageTag("ru"));
            Application::SetSettings(aSettings);
            return pResetter;
        }
        return nullptr;
    }

protected:
    /// Blacklist handling.
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to
        // test.
        return OString(filename).endsWith(".rtf");
    }

    AllSettings m_aSavedSettings;
};

DECLARE_RTFEXPORT_TEST(testFdo63023, "fdo63023.rtf")
{
    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    // Back color was black (0) in the header, due to missing color table in the substream.
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(0xFFFF99),
        getProperty<sal_Int32>(getRun(getParagraphOfText(1, xHeaderText), 1), "CharBackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo42109, "fdo42109.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // Make sure the page number is imported as a field in the B1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("TextField"),
                         getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1),
                                               "TextPortionType"));
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
    paste("rtfexport/data/fdo62044-paste.rtf", xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        10.f, getProperty<float>(xPropertySet,
                                 "CharHeight")); // Was 18, i.e. reset back to original value.
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(100),
                         getProperty<sal_Int32>(getShape(2), "BackColorTransparency"));
    beans::PropertyState ePropertyState = xPropertyState->getPropertyState("BackColorTransparency");
    // Was beans::PropertyState_DEFAULT_VALUE.
    CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
}

DECLARE_RTFEXPORT_TEST(testTdf106953, "tdf106953.rtf")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    auto xRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aRule["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testTdf112208, "tdf112208_hangingIndent.rtf")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-2000),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testParaBottomMargin, "para-bottom-margin.rtf")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Standard"), uno::UNO_QUERY);
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
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // This was 0 (no shapes were imported), we want two textframes.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xDraws->getCount());

    // The second paragraph of the first shape should be actually a table, with "A" in its A1 cell.
    uno::Reference<text::XTextRange> xTextRange(xDraws->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        OUString("A"),
        uno::Reference<text::XTextRange>(xTable->getCellByName("A1"), uno::UNO_QUERY)->getString());

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
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xProps(
        xDocumentPropertiesSupplier->getDocumentProperties());
    CPPUNIT_ASSERT_EQUAL(OUString("Aln Lin (Bei Jing)"), xProps->getAuthor());
    OUString aTitle(u"\u53A6\u95E8\u94A8\u4E1A\u80A1\u4EFD\u6709\u9650\u516C\u53F8");
    CPPUNIT_ASSERT_EQUAL(aTitle, xProps->getTitle());
    uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(),
                                                 uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("jay"), getProperty<OUString>(xUDProps, "Operator"));

    // fdo#80486 also check that the ftnsep doesn't insert paragraph breaks
    getParagraph(1, aTitle);
}

DECLARE_RTFEXPORT_TEST(testFdo47802, "fdo47802.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
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
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                         static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                             getParagraphOfText(1, xCell->getText()), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testFdo64637, "fdo64637.rtf")
{
    // The problem was that the custom "Company" property was added twice, the second invocation resulted in an exception.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(
        xDocumentPropertiesSupplier->getDocumentProperties()->getUserDefinedProperties(),
        uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("bbb"), getProperty<OUString>(xPropertySet, "Company"));
}

DECLARE_RTFEXPORT_TEST(testN820504, "n820504.rtf")
{
    // The shape was anchored at-page instead of at-character (that's incorrect as Word only supports at-character and as-character).
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testFdo67498, "fdo67498.rtf")
{
    // Left margin of the default page style wasn't set (was 2000).
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(convertTwipToMm100(5954)),
        getProperty<sal_Int32>(getStyles("PageStyles")->getByName("Standard"), "LeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testFdo44715, "fdo44715.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Style information wasn't reset, which caused character height to be 16.
    CPPUNIT_ASSERT_EQUAL(12.f,
                         getProperty<float>(getParagraphOfText(2, xCell->getText()), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo68076, "fdo68076.rtf")
{
    // Encoding of the last char was wrong (more 'o' than 'y').
    OUString aExpected(u"\u041E\u0431\u044A\u0435\u043A\u0442 \u2013 \u0443");
    getParagraph(1, aExpected);
}

DECLARE_RTFEXPORT_TEST(testFdo70221, "fdo70221.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xDraws->getCount());
}

DECLARE_RTFEXPORT_TEST(testCp1000018, "cp1000018.rtf")
{
    // The problem was that the empty paragraph at the end of the footnote got
    // lost during import.
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(),
                                                       uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);

    OUString const aExpected = "Footnote first line." SAL_NEWLINE_STRING;
    OUString const aActual = xTextRange->getString();

    CPPUNIT_ASSERT_EQUAL(aExpected, aActual);
}

DECLARE_RTFEXPORT_TEST(testFdo94835, "fdo94835.rtf")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                   uno::UNO_QUERY);
    // The picture was imported twice.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(19), xDraws->getCount());
}
DECLARE_RTFEXPORT_TEST(testNestedTable, "rhbz1065629.rtf")
{
    // nested table in second cell was missing
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
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
    table::BorderLine2 fullPtSolid(sal_Int32(COL_BLACK), 0, 35, 0, table::BorderLineStyle::SOLID,
                                   35);
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid, getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
                                getProperty<table::BorderLine2>(xCell, "BottomBorder"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xCC0000), getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 halfPtSolid(sal_Int32(COL_BLACK), 0, 18, 0, table::BorderLineStyle::SOLID,
                                   18);
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffffff),
                         getProperty<sal_Int32>(xCell, "BackColor"));
    xCell.set(xTable->getCellByName("B2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xffffffff),
                         getProperty<sal_Int32>(xCell, "BackColor"));

    // \sect at the end resulted in spurious page break
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testFooterPara, "footer-para.rtf")
{
    // check that paragraph properties in footer are imported
    uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("First Page"), "FooterText");
    uno::Reference<text::XTextContent> xParagraph = getParagraphOrTable(1, xFooterText);
    CPPUNIT_ASSERT_EQUAL(OUString("All Rights Reserved."),
                         uno::Reference<text::XTextRange>(xParagraph, uno::UNO_QUERY)->getString());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(style::ParagraphAdjust_CENTER),
        getProperty</*style::ParagraphAdjust*/ sal_Int16>(xParagraph, "ParaAdjust"));
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
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // The first row had 3 cells, instead of a horizontally merged one and a normal one (2 -> 1 separator).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
}

DECLARE_RTFEXPORT_TEST(testTableBorderDefaults, "fdo68779.rtf")
{
    // table borders without \brdrw were not imported
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 solid(sal_Int32(COL_BLACK), 0, 26, 0, table::BorderLineStyle::SOLID, 26);
    CPPUNIT_ASSERT_BORDER_EQUAL(solid, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid, getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(solid, getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(1), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 dotted(sal_Int32(COL_BLACK), 0, 26, 0, table::BorderLineStyle::DOTTED, 26);
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted, getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(dotted, getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(2), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 doubled(sal_Int32(COL_BLACK), 26, 26, 26, table::BorderLineStyle::DOUBLE,
                               79);
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled, getProperty<table::BorderLine2>(xCell, "TopBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(doubled, getProperty<table::BorderLine2>(xCell, "BottomBorder"));

    xTable.set(xTables->getByIndex(3), uno::UNO_QUERY);
    xCell.set(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xCell.is());
    table::BorderLine2 thinThickMG(sal_Int32(COL_BLACK), 14, 26, 14,
                                   table::BorderLineStyle::THINTHICK_MEDIUMGAP, 53);
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG, getProperty<table::BorderLine2>(xCell, "LeftBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG, getProperty<table::BorderLine2>(xCell, "RightBorder"));
    CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG, getProperty<table::BorderLine2>(xCell, "TopBorder"));
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
    uno::Reference<container::XIndexAccess> xNum1Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(SvxNumberFormat::LISTTAB),
        comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))["LabelFollowedBy"]
            .get<sal_Int16>()); // first level, tab

    uno::Reference<container::XIndexAccess> xNum2Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum2"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(SvxNumberFormat::SPACE),
        comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))["LabelFollowedBy"]
            .get<sal_Int16>()); // first level, space

    uno::Reference<container::XIndexAccess> xNum3Levels
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum3"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(SvxNumberFormat::NOTHING),
        comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))["LabelFollowedBy"]
            .get<sal_Int16>()); // first level, nothing
}

DECLARE_RTFEXPORT_TEST(testCharColor, "char-color.rtf")
{
    // This was -1: character color wasn't set.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x365F91), getProperty<sal_Int32>(getParagraph(1), "CharColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo69289, "fdo69289.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // There were only 2 cells (1 separators) in the table, should be 3 (2 separators).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
}

DECLARE_RTFEXPORT_TEST(testDptxbxRelation, "dptxbx-relation.rtf")
{
    // This was FRAME, not PAGE_FRAME, even if dobxpage is in the document.
    CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                         getProperty<sal_Int16>(getShape(1), "HoriOrientRelation"));
}

DECLARE_RTFEXPORT_TEST(testDprectAnchor, "dprect-anchor.rtf")
{
    // This was at-page, which is not something Word supports, so clearly an import error.
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

DECLARE_RTFEXPORT_TEST(testFdo76628, "fdo76628.rtf")
{
    OUString aExpected(u"\u041E\u0411\u0420\u0410\u0417\u0415\u0426");
    // Should be 'SAMPLE' in Russian, was garbage.
    getParagraph(1, aExpected);

    uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
        getStyles("PageStyles")->getByName("Standard"), "HeaderText");
    OUString aExpectedHeader(
        u"\u041F\u043E\u0434\u0433\u043E\u0442\u043E\u0432\u043B\u0435\u043D\u043E");
    // Should be 'prepared' in Russian, was garbage.
    getParagraphOfText(1, xHeaderText, aExpectedHeader);
}

DECLARE_RTFEXPORT_TEST(testFdo77267, "fdo77267.rtf")
{
    // Paragraph was aligned to left, should be center.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testFdo75735, "fdo75735.rtf")
{
    // Number of tabstops in the second paragraph should be 3, was 6.
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(3),
        getProperty<uno::Sequence<style::TabStop>>(getParagraph(2), "ParaTabStops").getLength());
}

DECLARE_RTFEXPORT_TEST(testFontOverride, "font-override.rtf")
{
    // This was "Times New Roman".
    CPPUNIT_ASSERT_EQUAL(OUString("Arial"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo73241, "fdo73241.rtf")
{
    // This was 2, page break in table wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_RTFEXPORT_TEST(testFdo80905, "fdo80905.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
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
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, due to ignoring RTF_TRGAPH.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(67)),
                         getProperty<sal_Int32>(xCell, "RightBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testFdo79959, "fdo79959.rtf")
{
    // This was false, as the style was imported as " Test", i.e. no whitespace stripping.
    CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(getStyles("ParagraphStyles")->hasByName("Test")));
}

DECLARE_RTFEXPORT_TEST(testFdo82078, "fdo82078.rtf")
{
    // This was awt::FontWeight::BOLD, i.e. the second run was bold, when it should be normal.
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                         getProperty<float>(getRun(getParagraph(1), 2), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testCsBold, "cs-bold.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run was bold, when it should be bold (applied character style without direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo84679, "fdo84679.rtf")
{
    // The problem was that the paragraph in A1 had some bottom margin, but it should not.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 282.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf123393, "tdf123393.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 7; Actual  : 10', i.e. font size was too large.
    CPPUNIT_ASSERT_EQUAL(
        7.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testFdo83464, "fdo83464.rtf")
{
    // Problem was that the text in the textframe had wrong font.
    uno::Reference<text::XTextRange> xFrameText(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xFrameText->getString());
    // This was Times New Roman.
    CPPUNIT_ASSERT_EQUAL(
        OUString("Verdana"),
        getProperty<OUString>(getRun(getParagraphOfText(1, xFrameText->getText()), 1),
                              "CharFontName"));
}

DECLARE_RTFEXPORT_TEST(testFdo86761, "fdo86761.rtf")
{
    // This was 26, even if the picture should have no border, due to fLine=0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0),
                         getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineWidth);
}

DECLARE_RTFEXPORT_TEST(testFdo82859, "fdo82859.rtf")
{
    // This was 0: "0xffffff" was converted to 0, i.e. the background was black instead of the default.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(getShape(1), "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testFdo82076, "fdo82076.rtf")
{
    // Footnote position was wrong: should be at the end of the B1 cell.
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("B1"), uno::UNO_QUERY);
    // This resulted in container::NoSuchElementException: the footnote was at the start of the A1 cell.
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote"),
                         getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 2),
                                               "TextPortionType"));
}

DECLARE_RTFEXPORT_TEST(testFdo83204, "fdo83204.rtf")
{
    // This was Standard, \sN was ignored after \bkmkstart and \pard.
    CPPUNIT_ASSERT_EQUAL(OUString("Heading 1"),
                         getProperty<OUString>(getParagraph(1), "ParaStyleName"));
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
    CPPUNIT_ASSERT_EQUAL(OUString("#anchor"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkURL"));
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
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"), parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"), parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("HEADER"), parseDump("/root/page[3]/header/txt/text()"));
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
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB,
                         getProperty<sal_Int16>(getParagraph(1), "WritingMode"));
}

DECLARE_RTFEXPORT_TEST(testTdf91074, "tdf91074.rtf")
{
    // The file failed to load, as the border color was imported using the LineColor UNO property.
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(COL_LIGHTRED),
                         getProperty<table::BorderLine2>(xShape, "TopBorder").Color);
}

DECLARE_RTFEXPORT_TEST(testTdf90260Nopar, "hello.rtf")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText(xTextDocument->getText(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste("rtfexport/data/tdf90260-nopar.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testTdf86814, "tdf86814.rtf")
{
    // This was awt::FontWeight::NORMAL, i.e. the first run wasn't bold, when it should be bold (applied paragraph style with direct formatting).
    CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                         getProperty<float>(getRun(getParagraph(1), 1), "CharWeight"));
}

DECLARE_RTFEXPORT_TEST(testTdf108416, "tdf108416.rtf")
{
    uno::Reference<container::XNameAccess> xCharacterStyles(getStyles("CharacterStyles"));
    uno::Reference<beans::XPropertySet> xListLabel(xCharacterStyles->getByName("ListLabel 1"),
                                                   uno::UNO_QUERY);
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int8>(2),
                         getProperty<sal_Int8>(getParagraph(1), "ParaWidows"));
}

DECLARE_RTFEXPORT_TEST(testTdf94456, "tdf94456.rtf")
{
    // Paragraph left margin and first line indent wasn't imported correctly.

    // This was 1270.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(762),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
    // This was -635.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-762),
                         getProperty<sal_Int32>(getParagraph(1), "ParaFirstLineIndent"));
}

DECLARE_RTFEXPORT_TEST(testTdf94435, "tdf94435.rtf")
{
    // This was style::ParagraphAdjust_LEFT, \ltrpar undone the effect of \qc.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf54584, "tdf54584.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    // \PAGE was ignored, so no fields were in document -> exception was thrown
    CPPUNIT_ASSERT_NO_THROW_MESSAGE(
        "No fields in document found: field \"\\PAGE\" was not properly read",
        xFields->nextElement());
}

DECLARE_RTFEXPORT_TEST(testTdf96308Deftab, "tdf96308-deftab.rtf")
{
    uno::Reference<lang::XMultiServiceFactory> xTextFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefaults(
        xTextFactory->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY);
    // This was 1270 as \deftab was ignored on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(284)),
                         getProperty<sal_Int32>(xDefaults, "TabStopDistance"));
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
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);

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
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // First cell width of the second row should be 2300
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2299), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                              xTableRows->getByIndex(1), "TableColumnSeparators")[0]
                                              .Position);
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
    paste("rtfexport/data/classification-yes.rtf", xEnd);
    CPPUNIT_ASSERT_EQUAL(aOld, xText->getString());
}

DECLARE_RTFEXPORT_TEST(testTdf95707, "tdf95707.rtf")
{
    // Graphic was replaced with a "Read-Error" placeholder.
    uno::Reference<graphic::XGraphic> xGraphic;
    xGraphic = getProperty<uno::Reference<graphic::XGraphic>>(getShape(1), "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);
}

DECLARE_RTFEXPORT_TEST(testTdf96275, "tdf96275.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(3, xCell->getText());
    // This was text: the shape's frame was part of the 1st paragraph instead of the 3rd one.
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xParagraph, 1), "TextPortionType"));
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(635),
                         getProperty<sal_Int32>(xCell, "LeftBorderDistance"));

    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    // Make sure that the scope of the default is only one row.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
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
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(191),
                         getProperty<sal_Int32>(xCell, "LeftBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf100507, "tdf100507.rtf")
{
    // This was 0: left margin of the first paragraph was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(6618),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf44986, "tdf44986.rtf")
{
    // Check that the table at the second paragraph.
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTable->getRows(), uno::UNO_QUERY);
    // Check the first row of the table, it should have two cells (one separator).
    // This was 0: the first row had no separators, so it had only one cell, which was too wide.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
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
    auto xRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
    // This was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270), aRule["IndentAt"].get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testTdf105852, "tdf105852.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows(xTextTable->getRows(), uno::UNO_QUERY);
    // All rows but last were merged -> there were only 2 rows
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), xTableRows->getCount());
    // The first row must have 4 cells.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(0), "TableColumnSeparators")
                                           .getLength());
    // The third row must have 1 merged cell.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                           xTableRows->getByIndex(2), "TableColumnSeparators")
                                           .getLength());
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
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf106694, "tdf106694.rtf")
{
    auto aTabs = getProperty<uno::Sequence<style::TabStop>>(getParagraph(1), "ParaTabStops");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aTabs.getLength());
    // This was 0, tab position was incorrect, looked like it was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(14605), aTabs[0].Position);
}

DECLARE_RTFEXPORT_TEST(testTdf107116, "tdf107116.rtf")
{
    // This was 0, upper border around text (and its distance) was missing.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(120)),
                         getProperty<sal_Int32>(getParagraph(2), "TopBorderDistance"));
}

DECLARE_RTFEXPORT_TEST(testTdf106950, "tdf106950.rtf")
{
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    // This was ParagraphAdjust_LEFT, trying to set CharShadingValue on a
    // paragraph style thrown an exception, and remaining properties were not
    // set.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_CENTER,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_RTFEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    auto xShape(getShape(1));
    // Without the accompanying fix in place, this test would have failed with
    // 'Unknown property: RotateAngle', i.e. export lost the rotation, and then
    // import created a Writer picture (instead of a Draw one).
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
