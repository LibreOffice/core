/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>

#include <tools/UnitConversion.hxx>
#include <comphelper/sequenceashashmap.hxx>

using namespace css;

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/rtfexport/data/"_ustr, u"Rich Text Format"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFdo63023)
{
    auto verify = [this]() {
        uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
        // Back color was black (0) in the header, due to missing color table in the substream.
        CPPUNIT_ASSERT_EQUAL(Color(0xFFFF99),
                             getProperty<Color>(getRun(getParagraphOfText(1, xHeaderText), 1),
                                                u"CharBackColor"_ustr));
    };
    createSwDoc("fdo63023.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo42109)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        // Make sure the page number is imported as a field in the B1 cell.
        CPPUNIT_ASSERT_EQUAL(
            u"TextField"_ustr,
            getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 1),
                                  u"TextPortionType"_ustr));
    };
    createSwDoc("fdo42109.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo62977)
{
    auto verify = [this]() {
        // The middle character was imported as '?' instead of the proper unicode value.
        getRun(getParagraph(1), 1, u"\u5E74\uFF14\u6708"_ustr);
    };
    createSwDoc("fdo62977.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN818997)
{
    auto verify = [this]() {
        // \page was ignored between two \shp tokens.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("n818997.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN818997B)
{
    auto verify = [this]() {
        // \page was ignored between two \shp tokens - this time not IsFirstParagraphInSection.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
    };
    createSwDoc("n818997B.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_anchoredAfterPgBreak)
{
    auto verify = [this]() {
        // An anchored TO character image (followed by nothing) anchors before the page break, no split.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//page[1]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_anchoredAfterPgBreak.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_anchoredAfterPgBreak2)
{
    auto verify = [this]() {
        // An anchored TO character image, followed by more characters moves to the following page
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_anchoredAfterPgBreak2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_anchoredAfterPgBreak4)
{
    auto verify = [this]() {
        // An anchored TO character image (followed by nothing) anchors before the page break, no split.
        // This differs from #1 only in that it has a preceding character run before the page break.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//page[1]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_anchoredAfterPgBreak4.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_anchoredAfterPgBreak5)
{
    auto verify = [this]() {
        // Two anchored TO character images (followed by nothing) splits & anchors after the page break
        // This differs from #1 only in that it has two anchored images.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//page[1]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_anchoredAfterPgBreak5.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_inlineAfterPgBreak)
{
    auto verify = [this]() {
        // An inline AS character image moves to the following page when after the page break.
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(3, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_inlineAfterPgBreak.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf153613_inlineAfterPgBreak2)
{
    auto verify = [this]() {
        // An inline AS character image moves to the following page when after the page break.
        // The difference from the previous test is that it is not the first character run
        CPPUNIT_ASSERT_EQUAL(2, getPages());
        CPPUNIT_ASSERT_EQUAL(4, getParagraphs());

        xmlDocUniquePtr pLayout = parseLayoutDump();
        CPPUNIT_ASSERT_EQUAL(u"x"_ustr, getXPathContent(pLayout, "//page[1]/body/txt[2]"_ostr));
        assertXPath(pLayout, "//page[2]//anchored"_ostr, 1);
    };
    createSwDoc("tdf153613_inlineAfterPgBreak2.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo64671)
{
    auto verify = [this]() {
        // Additional '}' was inserted before the special character.
        getRun(getParagraph(1), 1, u"\u017D"_ustr);
    };
    createSwDoc("fdo64671.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo62044)
{
    createSwDoc("fdo62044.rtf");
    // The problem was that RTF import during copy&paste did not ignore existing paragraph styles.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xEnd = xText->getEnd();
    paste(u"rtfexport/data/fdo62044-paste.rtf", u"com.sun.star.comp.Writer.RtfFilter"_ustr, xEnd);

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        10.f, getProperty<float>(xPropertySet,
                                 u"CharHeight"_ustr)); // Was 18, i.e. reset back to original value.
}

CPPUNIT_TEST_FIXTURE(Test, testFdo70578)
{
    auto verify = [this]() {
        // Style without explicit \s0 was not imported as the default style
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"ParagraphStyles"_ustr)->getByName(u"Subtitle"_ustr), uno::UNO_QUERY);
        uno::Reference<style::XStyle> xStyle(xPropertySet, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Standard"_ustr, xStyle->getParentStyle());
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(xPropertySet, u"ParaTopMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(xPropertySet, u"ParaBottomMargin"_ustr));
    };
    createSwDoc("fdo70578.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN825305)
{
    auto verify = [this]() {
        // The problem was that the textbox wasn't transparent, due to unimplemented fFilled == 0.
        uno::Reference<beans::XPropertyState> xPropertyState(getShape(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(Color(0x000064),
                             getProperty<Color>(getShape(2), u"BackColorTransparency"_ustr));
        beans::PropertyState ePropertyState
            = xPropertyState->getPropertyState(u"BackColorTransparency"_ustr);
        // Was beans::PropertyState_DEFAULT_VALUE.
        CPPUNIT_ASSERT_EQUAL(beans::PropertyState_DIRECT_VALUE, ePropertyState);
    };
    createSwDoc("n825305.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106953)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));
        auto xRules = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), u"NumberingRules"_ustr);
        comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             aRule[u"FirstLineIndent"_ustr].get<sal_Int32>());
    };
    createSwDoc("tdf106953.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf112208)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-2000),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaFirstLineIndent"_ustr));
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("tdf112208_hangingIndent.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testParaBottomMargin)
{
    auto verify = [this]() {
        uno::Reference<beans::XPropertySet> xPropertySet(
            getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(353),
                             getProperty<sal_Int32>(xPropertySet, u"ParaBottomMargin"_ustr));

        // This was 353, i.e. bottom margin of the paragraph was 0.35cm instead of 0.
        // The reason why this is 0 despite the default style containing \sa200
        // is that Word will actually interpret \sN (or \pard which apparently
        // implies \s0) as "set style N and for every attribute of that style,
        // set an attribute with default value on the paragraph"
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaBottomMargin"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                             getProperty<sal_Int32>(getParagraph(1), u"ParaTopMargin"_ustr));
    };
    createSwDoc("para-bottom-margin.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testParaStyleBottomMargin2)
{
    createSwDoc("para-style-bottom-margin-2.rtf");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353),
                         getProperty<sal_Int32>(xPropertySet, u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        style::LineSpacingMode::PROP,
        getProperty<style::LineSpacing>(xPropertySet, u"ParaLineSpacing"_ustr).Mode);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(115),
        getProperty<style::LineSpacing>(xPropertySet, u"ParaLineSpacing"_ustr).Height);

    // the derived style contains \sa200, as does its parent
    uno::Reference<beans::XPropertySet> xPropertySet1(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"List Paragraph"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353),
                         getProperty<sal_Int32>(xPropertySet1, u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        style::LineSpacingMode::PROP,
        getProperty<style::LineSpacing>(xPropertySet1, u"ParaLineSpacing"_ustr).Mode);
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(115),
        getProperty<style::LineSpacing>(xPropertySet1, u"ParaLineSpacing"_ustr).Height);
    // for the paragraph there is no \saN, so it should default to 0
    auto const xPara(getParagraph(1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::PROP,
                         getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Mode);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(115),
                         getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo66040)
{
    auto verify = [this]() {
        // This was 0 (no shapes were imported), we want two textframes.
        CPPUNIT_ASSERT_EQUAL(2, getShapes());

        // The second paragraph of the first shape should be actually a table, with "A" in its A1 cell.
        uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
        uno::Reference<text::XText> xText = xTextRange->getText();
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2, xText), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"A"_ustr, uno::Reference<text::XTextRange>(
                                            xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY_THROW)
                                            ->getString());

        // Make sure the second shape has the correct position and size.
        uno::Reference<drawing::XShape> xShape(getShape(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14420),
                             getProperty<sal_Int32>(xShape, u"HoriOrientPosition"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-1032),
                             getProperty<sal_Int32>(xShape, u"VertOrientPosition"_ustr));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(14000), xShape->getSize().Width);
        CPPUNIT_ASSERT_EQUAL(sal_Int32(21001), xShape->getSize().Height);
    };
    createSwDoc("fdo66040.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77996)
{
    auto verify = [this]() {
        // all styles were imported as name "0"
        uno::Reference<container::XNameAccess> xChars(getStyles(u"CharacterStyles"_ustr));
        CPPUNIT_ASSERT(!xChars->hasByName(u"0"_ustr));
        CPPUNIT_ASSERT(xChars->hasByName(u"strong"_ustr));
        CPPUNIT_ASSERT(xChars->hasByName(u"author"_ustr));
        uno::Reference<container::XNameAccess> xParas(getStyles(u"ParagraphStyles"_ustr));
        CPPUNIT_ASSERT(!xParas->hasByName(u"0"_ustr));
        CPPUNIT_ASSERT(xParas->hasByName(u"extract2"_ustr));
        // some document properties were lost
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<document::XDocumentProperties> xProps(
            xDocumentPropertiesSupplier->getDocumentProperties());
        CPPUNIT_ASSERT_EQUAL(u"Aln Lin (Bei Jing)"_ustr, xProps->getAuthor());
        OUString aTitle(u"\u53A6\u95E8\u94A8\u4E1A\u80A1\u4EFD\u6709\u9650\u516C\u53F8"_ustr);
        CPPUNIT_ASSERT_EQUAL(aTitle, xProps->getTitle());
        uno::Reference<beans::XPropertySet> xUDProps(xProps->getUserDefinedProperties(),
                                                     uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"jay"_ustr, getProperty<OUString>(xUDProps, u"Operator"_ustr));

        // fdo#80486 also check that the ftnsep doesn't insert paragraph breaks
        getParagraph(1, aTitle);
    };
    createSwDoc("fdo77996.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo47802)
{
    auto verify = [this]() {
        // Shape inside table was ignored.
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
    };
    createSwDoc("fdo47802.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo39001)
{
    auto verify = [this]() {
        // Document was of 4 pages, \sect at the end of the doc wasn't ignored.
        CPPUNIT_ASSERT_EQUAL(3, getPages());
    };
    createSwDoc("fdo39001.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo54900)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // Paragraph was aligned to left, should be center.
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                             static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(
                                 getParagraphOfText(1, xCell->getText()), u"ParaAdjust"_ustr)));
    };
    createSwDoc("fdo54900.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo64637)
{
    auto verify = [this]() {
        // The problem was that the custom "Company" property was added twice, the second invocation resulted in an exception.
        uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
            mxComponent, uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(
            xDocumentPropertiesSupplier->getDocumentProperties()->getUserDefinedProperties(),
            uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"bbb"_ustr, getProperty<OUString>(xPropertySet, u"Company"_ustr));
    };
    createSwDoc("fdo64637.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testN820504)
{
    auto verify = [this]() {
        // The shape was anchored at-page instead of at-character (that's incorrect as Word only supports at-character and as-character).
        CPPUNIT_ASSERT_EQUAL(
            text::TextContentAnchorType_AT_CHARACTER,
            getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
    };
    createSwDoc("n820504.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo67498)
{
    auto verify = [this]() {
        // Left margin of the default page style wasn't set (was 2000).
        CPPUNIT_ASSERT_EQUAL(
            sal_Int32(convertTwipToMm100(5954)),
            getProperty<sal_Int32>(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr),
                                   u"LeftMargin"_ustr));
    };
    createSwDoc("fdo67498.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo44715)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // Style information wasn't reset, which caused character height to be 16.
        CPPUNIT_ASSERT_EQUAL(
            12.f, getProperty<float>(getParagraphOfText(2, xCell->getText()), u"CharHeight"_ustr));
    };
    createSwDoc("fdo44715.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo68076)
{
    auto verify = [this]() {
        // Encoding of the last char was wrong (more 'o' than 'y').
        getParagraph(1, u"\u041E\u0431\u044A\u0435\u043A\u0442 \u2013 \u0443"_ustr);
    };
    createSwDoc("fdo68076.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo70221)
{
    auto verify = [this]() {
        // The picture was imported twice.
        CPPUNIT_ASSERT_EQUAL(1, getShapes());
    };
    createSwDoc("fdo70221.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCp1000018)
{
    auto verify = [this]() {
        // The problem was that the empty paragraph at the end of the footnote got
        // lost during import.
        uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
        uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);

        OUString const aActual = xTextRange->getString();

        CPPUNIT_ASSERT_EQUAL(u"Footnote first line." SAL_NEWLINE_STRING ""_ustr, aActual);
    };
    createSwDoc("cp1000018.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo94835)
{
    auto verify = [this]() {
        // The picture was imported twice.
        CPPUNIT_ASSERT_EQUAL(19, getShapes());
    };
    createSwDoc("fdo94835.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}
CPPUNIT_TEST_FIXTURE(Test, testNestedTable)
{
    auto verify = [this]() {
        // nested table in second cell was missing
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(),
                                                                      uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Responsable Commercial:"_ustr, xPara->getString());
        xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
        xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
        xParaEnum = xParaEnumAccess->createEnumeration();
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Nom: John Doe"_ustr, xPara->getString());

        // outer table: background color, borders for B1/B2 cell
        xTable.set(xTables->getByIndex(2), uno::UNO_QUERY);
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 fullPtSolid(sal_Int32(COL_BLACK), 0, 35, 0,
                                       table::BorderLineStyle::SOLID, 35);
        CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(fullPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(Color(0xCC0000), getProperty<Color>(xCell, u"BackColor"_ustr));
        xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 halfPtSolid(sal_Int32(COL_BLACK), 0, 18, 0,
                                       table::BorderLineStyle::SOLID, 18);
        CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell, u"BackColor"_ustr));
        xCell.set(xTable->getCellByName(u"B2"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(halfPtSolid,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell, u"BackColor"_ustr));

        // \sect at the end resulted in spurious page break
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("rhbz1065629.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFooterPara)
{
    auto verify = [this]() {
        // check that paragraph properties in footer are imported
        uno::Reference<text::XText> xFooterText = getProperty<uno::Reference<text::XText>>(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"FooterTextFirst"_ustr);
        uno::Reference<text::XTextContent> xParagraph = getParagraphOrTable(1, xFooterText);
        CPPUNIT_ASSERT_EQUAL(
            u"All Rights Reserved."_ustr,
            uno::Reference<text::XTextRange>(xParagraph, uno::UNO_QUERY_THROW)->getString());
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(style::ParagraphAdjust_CENTER),
            getProperty</*style::ParagraphAdjust*/ sal_Int16>(xParagraph, u"ParaAdjust"_ustr));
    };
    createSwDoc("footer-para.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107413)
{
    auto verify = [this]() {
        CPPUNIT_ASSERT_EQUAL(1, getPages());

        xmlDocUniquePtr pDump = parseLayoutDump();
        const double nLeftFooter
            = getXPath(pDump, "/root/page[1]/footer/infos/bounds"_ostr, "left"_ostr).toDouble();
        const double nRightFooter
            = getXPath(pDump, "/root/page[1]/footer/infos/bounds"_ostr, "right"_ostr).toDouble();
        const double nTopFooter
            = getXPath(pDump, "/root/page[1]/footer/infos/bounds"_ostr, "top"_ostr).toDouble();
        const double nBottomFooter
            = getXPath(pDump, "/root/page[1]/footer/infos/bounds"_ostr, "bottom"_ostr).toDouble();

        // Without the fix in place, this test would have failed with
        // - Expected: 1
        // - Actual  : 0
        // - In <>, XPath '/root/page[1]/footer/txt/anchored/fly/infos/bounds' number of nodes is incorrect
        const double nLeftFly
            = getXPath(pDump, "/root/page[1]/footer/txt/anchored/fly/infos/bounds"_ostr,
                       "left"_ostr)
                  .toDouble();
        const double nRightFly
            = getXPath(pDump, "/root/page[1]/footer/txt/anchored/fly/infos/bounds"_ostr,
                       "right"_ostr)
                  .toDouble();
        const double nTopFly
            = getXPath(pDump, "/root/page[1]/footer/txt/anchored/fly/infos/bounds"_ostr, "top"_ostr)
                  .toDouble();
        const double nBottomFly
            = getXPath(pDump, "/root/page[1]/footer/txt/anchored/fly/infos/bounds"_ostr,
                       "bottom"_ostr)
                  .toDouble();

        CPPUNIT_ASSERT_EQUAL(nLeftFooter, nLeftFly);
        CPPUNIT_ASSERT_EQUAL(nRightFooter, nRightFly);
        CPPUNIT_ASSERT_EQUAL(nBottomFooter, nBottomFly);
        CPPUNIT_ASSERT_EQUAL(nTopFooter + 1056.0, nTopFly);
    };
    createSwDoc("tdf107413.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCp1000016)
{
    auto verify = [this]() {
        // The single-line document had a second fake empty para on Windows.
        CPPUNIT_ASSERT_EQUAL(1, getParagraphs());
    };
    createSwDoc("hello.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo65090)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTextTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XTableRows> xTableRows = xTextTable->getRows();
        // The first row had 3 cells, instead of a horizontally merged one and a normal one (2 -> 1 separator).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)
                                 .getLength());
    };
    createSwDoc("fdo65090.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTableBorderDefaults)
{
    auto verify = [this]() {
        // table borders without \brdrw were not imported
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 solid(sal_Int32(COL_BLACK), 0, 26, 0, table::BorderLineStyle::SOLID, 26);
        CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                    getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(solid,
                                    getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr));

        xTable.set(xTables->getByIndex(1), uno::UNO_QUERY);
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 dotted(sal_Int32(COL_BLACK), 0, 26, 0, table::BorderLineStyle::DOTTED,
                                  26);
        CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                    getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(dotted,
                                    getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr));

        xTable.set(xTables->getByIndex(2), uno::UNO_QUERY);
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 doubled(sal_Int32(COL_BLACK), 26, 26, 26, table::BorderLineStyle::DOUBLE,
                                   79);
        CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                    getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(doubled,
                                    getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr));

        xTable.set(xTables->getByIndex(3), uno::UNO_QUERY);
        xCell.set(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xCell.is());
        table::BorderLine2 thinThickMG(sal_Int32(COL_BLACK), 14, 26, 14,
                                       table::BorderLineStyle::THINTHICK_MEDIUMGAP, 53);
        CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                    getProperty<table::BorderLine2>(xCell, u"LeftBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                    getProperty<table::BorderLine2>(xCell, u"RightBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                    getProperty<table::BorderLine2>(xCell, u"TopBorder"_ustr));
        CPPUNIT_ASSERT_BORDER_EQUAL(thinThickMG,
                                    getProperty<table::BorderLine2>(xCell, u"BottomBorder"_ustr));
    };
    createSwDoc("fdo68779.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testShpzDhgt)
{
    auto verify = [this]() {
        // Test that shpz has priority over dhgt and not the other way around.
        // Drawpage is sorted by ZOrder, so first should be red (back).
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(getShape(1), u"FillColor"_ustr));
        // Second (front) should be green.
        CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(getShape(2), u"FillColor"_ustr));
    };
    createSwDoc("shpz-dhgt.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testLevelfollow)
{
    auto verify = [this]() {
        uno::Reference<container::XIndexAccess> xNum1Levels
            = getProperty<uno::Reference<container::XIndexAccess>>(
                getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr),
                u"NumberingRules"_ustr);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(SvxNumberFormat::LISTTAB),
            comphelper::SequenceAsHashMap(xNum1Levels->getByIndex(0))[u"LabelFollowedBy"_ustr]
                .get<sal_Int16>()); // first level, tab

        uno::Reference<container::XIndexAccess> xNum2Levels
            = getProperty<uno::Reference<container::XIndexAccess>>(
                getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum2"_ustr),
                u"NumberingRules"_ustr);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(SvxNumberFormat::SPACE),
            comphelper::SequenceAsHashMap(xNum2Levels->getByIndex(0))[u"LabelFollowedBy"_ustr]
                .get<sal_Int16>()); // first level, space

        uno::Reference<container::XIndexAccess> xNum3Levels
            = getProperty<uno::Reference<container::XIndexAccess>>(
                getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum3"_ustr),
                u"NumberingRules"_ustr);
        CPPUNIT_ASSERT_EQUAL(
            sal_Int16(SvxNumberFormat::NOTHING),
            comphelper::SequenceAsHashMap(xNum3Levels->getByIndex(0))[u"LabelFollowedBy"_ustr]
                .get<sal_Int16>()); // first level, nothing
    };
    createSwDoc("levelfollow.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCharColor)
{
    auto verify = [this]() {
        // This was -1: character color wasn't set.
        CPPUNIT_ASSERT_EQUAL(Color(0x365F91),
                             getProperty<Color>(getParagraph(1), u"CharColor"_ustr));
    };
    createSwDoc("char-color.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo69289)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
        // There were only 2 cells (1 separators) in the table, should be 3 (2 separators).
        CPPUNIT_ASSERT_EQUAL(sal_Int32(2),
                             getProperty<uno::Sequence<text::TableColumnSeparator>>(
                                 xTableRows->getByIndex(0), u"TableColumnSeparators"_ustr)
                                 .getLength());
    };
    createSwDoc("fdo69289.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDptxbxRelation)
{
    auto verify = [this]() {
        // This was FRAME, not PAGE_FRAME, even if dobxpage is in the document.
        CPPUNIT_ASSERT_EQUAL(text::RelOrientation::PAGE_FRAME,
                             getProperty<sal_Int16>(getShape(1), u"HoriOrientRelation"_ustr));
    };
    createSwDoc("dptxbx-relation.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testDprectAnchor)
{
    auto verify = [this]() {
        // This was at-page, which is not something Word supports, so clearly an import error.
        CPPUNIT_ASSERT_EQUAL(
            text::TextContentAnchorType_AT_CHARACTER,
            getProperty<text::TextContentAnchorType>(getShape(1), u"AnchorType"_ustr));
    };
    createSwDoc("dprect-anchor.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo76628)
{
    auto verify = [this]() {
        // Should be 'SAMPLE' in Russian, was garbage.
        getParagraph(1, u"\u041E\u0411\u0420\u0410\u0417\u0415\u0426"_ustr);

        uno::Reference<text::XText> xHeaderText = getProperty<uno::Reference<text::XText>>(
            getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), u"HeaderText"_ustr);
        // Should be 'prepared' in Russian, was garbage.
        getParagraphOfText(
            1, xHeaderText,
            u"\u041F\u043E\u0434\u0433\u043E\u0442\u043E\u0432\u043B\u0435\u043D\u043E"_ustr);
    };
    createSwDoc("fdo76628.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo77267)
{
    auto verify = [this]() {
        // Paragraph was aligned to left, should be center.
        CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_CENTER,
                             static_cast<style::ParagraphAdjust>(
                                 getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
    };
    createSwDoc("fdo77267.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo75735)
{
    auto verify = [this]() {
        // Number of tabstops in the second paragraph should be 3, was 6.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), getProperty<uno::Sequence<style::TabStop>>(
                                               getParagraph(2), u"ParaTabStops"_ustr)
                                               .getLength());
    };
    createSwDoc("fdo75735.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFontOverride)
{
    auto verify = [this]() {
        // This was "Times New Roman".
        CPPUNIT_ASSERT_EQUAL(
            u"Arial"_ustr, getProperty<OUString>(getRun(getParagraph(1), 1), u"CharFontName"_ustr));
    };
    createSwDoc("font-override.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo73241)
{
    auto verify = [this]() {
        // This was 2, page break in table wasn't ignored.
        CPPUNIT_ASSERT_EQUAL(1, getPages());
    };
    createSwDoc("fdo73241.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo80905)
{
    auto verify = [this]() {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(
            xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        xFields->nextElement();
        // The problem was that there was only one field in the document, but there should be true.
        CPPUNIT_ASSERT_EQUAL(true, static_cast<bool>(xFields->hasMoreElements()));
    };
    createSwDoc("fdo80905.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82106)
{
    auto verify = [this]() {
        // Tab was missing after footnote not containing a tab.
        getParagraph(2, u"before\tafter"_ustr);
    };
    createSwDoc("fdo82106.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo74229)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // This was 0, due to ignoring RTF_TRGAPH.
        CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(67)),
                             getProperty<sal_Int32>(xCell, u"RightBorderDistance"_ustr));
    };
    createSwDoc("fdo74229.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo79959)
{
    auto verify = [this]() {
        // This was false, as the style was imported as " Test", i.e. no whitespace stripping.
        CPPUNIT_ASSERT_EQUAL(
            true, static_cast<bool>(getStyles(u"ParagraphStyles"_ustr)->hasByName(u"Test"_ustr)));
    };
    createSwDoc("fdo79959.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82078)
{
    auto verify = [this]() {
        // This was awt::FontWeight::BOLD, i.e. the second run was bold, when it should be normal.
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::NORMAL,
                             getProperty<float>(getRun(getParagraph(1), 2), u"CharWeight"_ustr));
    };
    createSwDoc("fdo82078.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testCsBold)
{
    auto verify = [this]() {
        // This was awt::FontWeight::NORMAL, i.e. the first run was bold, when it should be bold (applied character style without direct formatting).
        CPPUNIT_ASSERT_EQUAL(awt::FontWeight::BOLD,
                             getProperty<float>(getRun(getParagraph(1), 1), u"CharWeight"_ustr));
    };
    createSwDoc("cs-bold.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo84679)
{
    auto verify = [this]() {
        // The problem was that the paragraph in A1 had some bottom margin, but it should not.
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // This was 282.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                             getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()),
                                                    u"ParaBottomMargin"_ustr));
    };
    createSwDoc("fdo84679.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123393)
{
    auto verify = [this]() {
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
        // Without the accompanying fix in place, this test would have failed with
        // 'Expected: 7; Actual  : 10', i.e. font size was too large.
        CPPUNIT_ASSERT_EQUAL(7.f,
                             getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1),
                                                u"CharHeight"_ustr));
    };
    createSwDoc("tdf123393.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo83464)
{
    auto verify = [this]() {
        // Problem was that the text in the textframe had wrong font.
        uno::Reference<text::XTextRange> xFrameText(getShape(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"Hello"_ustr, xFrameText->getString());
        // This was Times New Roman.
        CPPUNIT_ASSERT_EQUAL(
            u"Verdana"_ustr,
            getProperty<OUString>(getRun(getParagraphOfText(1, xFrameText->getText()), 1),
                                  u"CharFontName"_ustr));
    };
    createSwDoc("fdo83464.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo86761)
{
    auto verify = [this]() {
        // This was 26, even if the picture should have no border, due to fLine=0.
        CPPUNIT_ASSERT_EQUAL(
            static_cast<sal_uInt32>(0),
            getProperty<table::BorderLine2>(getShape(1), u"TopBorder"_ustr).LineWidth);
    };
    createSwDoc("fdo86761.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82859)
{
    auto verify = [this]() {
        // This was 0: "0xffffff" was converted to 0, i.e. the background was black instead of the default.
        CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<Color>(getShape(1), u"BackColor"_ustr));
    };
    createSwDoc("fdo82859.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82076)
{
    auto verify = [this]() {
        // Footnote position was wrong: should be at the end of the B1 cell.
        uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                        uno::UNO_QUERY);
        uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
        // This resulted in container::NoSuchElementException: the footnote was at the start of the A1 cell.
        CPPUNIT_ASSERT_EQUAL(
            u"Footnote"_ustr,
            getProperty<OUString>(getRun(getParagraphOfText(1, xCell->getText()), 2),
                                  u"TextPortionType"_ustr));
    };
    createSwDoc("fdo82076.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

CPPUNIT_TEST_FIXTURE(Test, testFdo83204)
{
    auto verify = [this]() {
        // This was Standard, \sN was ignored after \bkmkstart and \pard.
        CPPUNIT_ASSERT_EQUAL(u"Heading 1"_ustr,
                             getProperty<OUString>(getParagraph(1), u"ParaStyleName"_ustr));
    };
    createSwDoc("fdo83204.rtf");
    verify();
    saveAndReload(mpFilter);
    verify();
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
