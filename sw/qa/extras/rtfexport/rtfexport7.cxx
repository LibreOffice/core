/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <tools/UnitConversion.hxx>

using namespace css;

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/rtfexport/data/", "Rich Text Format")
    {
    }
};

DECLARE_RTFEXPORT_TEST(testPictureWrapPolygon, "picture-wrap-polygon.rtf")
{
    // The problem was that the wrap polygon was ignored during import.
    drawing::PointSequenceSequence aSeqSeq
        = getProperty<drawing::PointSequenceSequence>(getShape(1), "ContourPolyPolygon");
    // This was 0: the polygon list was empty.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aSeqSeq.getLength());

    drawing::PointSequence aSeq = aSeqSeq[0];
    CPPUNIT_ASSERT_EQUAL(sal_Int32(11), aSeq.getLength());

    // The shape also didn't have negative top / left coordinates.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-1177)),
                         getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(-67)),
                         getProperty<sal_Int32>(getShape(1), "VertOrientPosition"));
}

DECLARE_RTFEXPORT_TEST(testTdf113408, "tdf113408.rtf")
{
    // This was 0, left margin was not inherited from style properly.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1270),
                         getProperty<sal_Int32>(getParagraph(1), "ParaLeftMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testAbi10039)
{
    loadAndReload("abi10039.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure we don't just crash on export, and additionally the shape should not be inline (as it's at-page anchored originally).
    CPPUNIT_ASSERT(text::TextContentAnchorType_AS_CHARACTER
                   != getProperty<text::TextContentAnchorType>(getShape(1), "AnchorType"));
}

CPPUNIT_TEST_FIXTURE(Test, testAbi10076)
{
    loadAndReload("abi10076.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // Just make sure that we don't crash after exporting a fully calculated layout.
}

DECLARE_RTFEXPORT_TEST(testEm, "em.rtf")
{
    // Test all possible \acc* control words.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::NONE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 2), "CharEmphasis"));
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::ACCENT_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 3), "CharEmphasis"));
    // This was missing.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::CIRCLE_ABOVE,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 4), "CharEmphasis"));
    // This one, too.
    CPPUNIT_ASSERT_EQUAL(text::FontEmphasis::DOT_BELOW,
                         getProperty<sal_Int16>(getRun(getParagraph(1), 5), "CharEmphasis"));
}

DECLARE_RTFEXPORT_TEST(testNumberingFont, "numbering-font.rtf")
{
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles("CharacterStyles")->getByName("ListLabel 1"), uno::UNO_QUERY);
    // This was Liberation Serif, i.e. custom font of the numbering itself ("1.\t") was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("Impact"), getProperty<OUString>(xStyle, "CharFontName"));
    CPPUNIT_ASSERT_EQUAL(72.f, getProperty<float>(xStyle, "CharHeight"));
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82860)
{
    loadAndReload("fdo82860.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // The problem was that:
    // 1) The import tried to use fieldmarks for SHAPE fields
    // 2) The exporter did not handle "shape with textbox" text.
    uno::Reference<text::XTextRange> xTextRange(getShape(1), uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextRange->getText();
    CPPUNIT_ASSERT_EQUAL(OUString("hello"), getParagraphOfText(1, xText)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFdo82858)
{
    loadAndReload("fdo82858.docx");
    // This was table::BorderLineStyle::SOLID, exporter failed to write explicit no line when line color was written.
    CPPUNIT_ASSERT_EQUAL(table::BorderLineStyle::NONE,
                         getProperty<table::BorderLine2>(getShape(1), "TopBorder").LineStyle);
}

DECLARE_RTFEXPORT_TEST(testTdf104936, "tdf104936.rtf")
{
    uno::Reference<text::XTextRange> xShape1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xShape1, "ZOrder"));
    // This failed, the shape without text covered the shape with text.
    CPPUNIT_ASSERT(xShape1->getString().isEmpty());
    uno::Reference<text::XTextRange> xShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), getProperty<sal_Int32>(xShape2, "ZOrder"));
    CPPUNIT_ASSERT_EQUAL(OUString("Hello"), xShape2->getString());
}

DECLARE_RTFEXPORT_TEST(testTableRtl, "table-rtl.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    // This was text::WritingMode2::LR_TB, i.e. direction of the table was ignored.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::RL_TB, getProperty<sal_Int16>(xTable, "WritingMode"));
}

DECLARE_RTFEXPORT_TEST(testNumOverrideStart, "num-override-start.rtf")
{
    // The numbering on the second level was "3.1", not "1.3".
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(1),
        comphelper::SequenceAsHashMap(xRules->getByIndex(0))["StartWith"].get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(
        sal_Int16(3),
        comphelper::SequenceAsHashMap(xRules->getByIndex(1))["StartWith"].get<sal_Int16>());
}

DECLARE_RTFEXPORT_TEST(testFdo82006, "fdo82006.rtf")
{
    // These were 176 (100 twips), as \sbauto and \sbbefore were ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf104081, "tdf104081.rtf")
{
    // These were 494 (280 twips), as \htmautsp was ignored.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(100)),
                         getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(100)),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88583)
{
    loadAndReload("tdf88583.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This was FillStyle_NONE, as background color was missing from the color table during export.
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID,
                         getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(Color(0x00cc00), getProperty<Color>(getParagraph(1), "FillColor"));
}

DECLARE_RTFEXPORT_TEST(testMargmirror, "margmirror.rtf")
{
    // \margmirror was not handled, this was PageStyleLayout_ALL.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(style::PageStyleLayout_MIRRORED,
                         getProperty<style::PageStyleLayout>(xPageStyle, "PageStyleLayout"));
}

DECLARE_RTFEXPORT_TEST(testSautoupd, "sautoupd.rtf")
{
    // \sautoupd was ignored during import and export.
    uno::Reference<beans::XPropertySet> xHeading1(
        getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xHeading1, "IsAutoUpdate"));
    uno::Reference<beans::XPropertySet> xHeading2(
        getStyles("ParagraphStyles")->getByName("Heading 2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xHeading2, "IsAutoUpdate"));
}

DECLARE_RTFEXPORT_TEST(testHyphauto, "hyphauto.rtf")
{
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getParagraph(1), "ParaIsHyphenation"));
}

DECLARE_RTFEXPORT_TEST(testHyphpar, "hyphpar.rtf")
{
    // Hyphenation was enabled for all 3 paragraphs, but it should be disabled for the 2nd one.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getParagraph(2), "ParaIsHyphenation"));
}

DECLARE_RTFEXPORT_TEST(testTdf108955, "tdf108955.rtf")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID,
                         getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    // This was 0xffffff, i.e. non-white background was overwritten from the paragraph style.
    CPPUNIT_ASSERT_EQUAL(Color(0xffff99), getProperty<Color>(getParagraph(1), "FillColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf80708, "tdf80708.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    // This was 2, i.e. the second table had 3 cols, now 2 as expected.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<uno::Sequence<text::TableColumnSeparator>>(
                             xTableRows->getByIndex(0), "TableColumnSeparators")
                             .getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testHyperlinkWithoutURL)
{
    createSwDoc("tdf90421.fodt");
    // Change the hyperlink, so its URL is empty.
    uno::Reference<beans::XPropertySet> xRun(getRun(getParagraph(1), 2), uno::UNO_QUERY);
    xRun->setPropertyValue("HyperLinkURL", uno::Any(OUString()));
    saveAndReload("Rich Text Format");
    SvMemoryStream aMemoryStream;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    aStream.ReadStream(aMemoryStream);
    OString aData(static_cast<const char*>(aMemoryStream.GetData()), aMemoryStream.GetSize());
    // This was some positive number, i.e. we exported a hyperlink with an empty URL.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), aData.indexOf("HYPERLINK"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf92521)
{
    loadAndReload("tdf92521.odt");
    // There should be a page break that's in the middle of the document: right after the table.
    // But there wasn't, so this was 1.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_RTFEXPORT_TEST(testTdf114309, "tdf114309.rtf")
{
    // Without the fix in place, this test would have failed with
    // - the property is of unexpected type or void: TextSection
    auto xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1), "TextSection");
    auto xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
}

DECLARE_RTFEXPORT_TEST(testTdf94043, "tdf94043.rtf")
{
    auto xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), "TextSection");
    auto xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(xTextColumns, "SeparatorLineWidth"));

    CPPUNIT_ASSERT_EQUAL(7, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testTdf94377, "tdf94377.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    uno::Reference<beans::XPropertySet> xPropertySet(xFields->nextElement(), uno::UNO_QUERY);
    auto xText = getProperty<uno::Reference<text::XText>>(xPropertySet, "TextRange");
    // This failed, as:
    // 1) multiple paragraphs were not exported, so the text was "Asdf10asdf12".
    // 2) direct formatting of runs were not exported, so this was 12 (the document default).
    CPPUNIT_ASSERT_EQUAL(
        10.f, getProperty<float>(getRun(getParagraphOfText(1, xText, "Asdf10"), 1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL(
        12.f, getProperty<float>(getRun(getParagraphOfText(2, xText, "asdf12"), 1), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testTdf104079, "tdf104079.rtf")
{
    bool bFound = false;
    int nIndex = 0;
    while (!bFound)
    {
        uno::Reference<text::XTextRange> xParagraph = getParagraph(++nIndex);
        if (!xParagraph->getString().startsWith("toc3"))
            continue;

        bFound = true;
        // This was 0, 3rd paragraph of ToC lost its bottom paragraph margin.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(212),
                             getProperty<sal_Int32>(xParagraph, "ParaBottomMargin"));
    }
}

DECLARE_RTFEXPORT_TEST(testPageBackground, "page-background.rtf")
{
    // The problem was that \background was ignored.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0x92D050), getProperty<Color>(xPageStyle, "BackColor"));
}

DECLARE_RTFEXPORT_TEST(testTdf96175, "tdf96175.rtf")
{
    // The problem that a user defined property named "Company" was lost on export.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertyContainer> xUserDefinedProperties
        = xDocumentProperties->getUserDefinedProperties();
    // This resulted in a beans::UnknownPropertyException.
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"),
                         getProperty<OUString>(xUserDefinedProperties, "Company"));
}

DECLARE_RTFEXPORT_TEST(testRedline, "redline.rtf")
{
    CPPUNIT_ASSERT_EQUAL(OUString("Rebecca Lopez"),
                         getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineAuthor"));
    CPPUNIT_ASSERT_EQUAL(OUString("Dorothy Jones"),
                         getProperty<OUString>(getRun(getParagraph(2), 2), "RedlineAuthor"));
}

DECLARE_RTFEXPORT_TEST(testCustomDocProps, "custom-doc-props.rtf")
{
    // Custom document properties were not improved, this resulted in a beans::UnknownPropertyException.
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(
        mxComponent, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties
        = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertyContainer> xUserDefinedProperties
        = xDocumentProperties->getUserDefinedProperties();
    CPPUNIT_ASSERT_EQUAL(
        OUString("2016-03-08T10:55:18,531376147"),
        getProperty<OUString>(xUserDefinedProperties,
                              "urn:bails:IntellectualProperty:Authorization:StartValidity"));
    CPPUNIT_ASSERT_EQUAL(
        OUString("None"),
        getProperty<OUString>(xUserDefinedProperties,
                              "urn:bails:IntellectualProperty:Authorization:StopValidity"));
    // Test roundtrip of numbers. This failed as getProperty() did not find "n".
    CPPUNIT_ASSERT_EQUAL(42.0, getProperty<double>(xUserDefinedProperties, "n"));
    // Test boolean "yes".
    CPPUNIT_ASSERT(getProperty<bool>(xUserDefinedProperties, "by"));
    // Test boolean "no".
    CPPUNIT_ASSERT(!getProperty<bool>(xUserDefinedProperties, "bn"));

    // Test roundtrip of date in general, and year/month/day in particular.
    util::DateTime aDate = getProperty<util::DateTime>(xUserDefinedProperties, "d");
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2016), aDate.Year);
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(1), aDate.Month);
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(30), aDate.Day);

    // Test real number.
    CPPUNIT_ASSERT_EQUAL(3.14, getProperty<double>(xUserDefinedProperties, "pi"));
}

DECLARE_RTFEXPORT_TEST(testTdf65642, "tdf65642.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(
        style::NumberingType::CHARS_UPPER_LETTER_N,
        getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
    // The second page's restart value: this was 0.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1),
                         getProperty<sal_Int32>(getParagraph(2), "PageNumberOffset"));
}

DECLARE_RTFEXPORT_TEST(testPgnlcltr, "pgnlcltr.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(
        style::NumberingType::CHARS_LOWER_LETTER_N,
        getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgnucrm, "pgnucrm.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(
        style::NumberingType::ROMAN_UPPER,
        getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgnlcrm, "pgnlcrm.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ARABIC.
    CPPUNIT_ASSERT_EQUAL(
        style::NumberingType::ROMAN_LOWER,
        getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));
}

DECLARE_RTFEXPORT_TEST(testPgndec, "pgndec.rtf")
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    // The second page's numbering type: this was style::NumberingType::ROMAN_LOWER.
    CPPUNIT_ASSERT_EQUAL(
        style::NumberingType::ARABIC,
        getProperty<sal_Int16>(xPageStyles->getByName(pageStyleName), "NumberingType"));

    // tdf#82111 ensure a paragraph exists before a section break.
    // This was only two paragraphs, and both page number fields were in one para on page 2 ("11").
    getParagraph(2, "1");
    CPPUNIT_ASSERT_EQUAL(3, getParagraphs());
}

DECLARE_RTFEXPORT_TEST(testTdf98806, "tdf98806.rtf")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xBookmark(
        xBookmarksSupplier->getBookmarks()->getByName("bookmark"), uno::UNO_QUERY);
    // This was empty, bookmark in table wasn't imported correctly.
    CPPUNIT_ASSERT_EQUAL(OUString("BBB"), xBookmark->getAnchor()->getString());
}

DECLARE_RTFEXPORT_TEST(testTdf61901, "tdf61901.rtf")
{
    // Test the file directly, as current RTF import gives the correct font name with and without the fix.
    if (isExported())
    {
        SvStream* pStream = maTempFile.GetStream(StreamMode::READ);
        OString sLine;
        while (pStream->ReadLine(sLine))
        {
            sal_Int32 nIndex = sLine.indexOf("\\loch\\loch");
            if (nIndex != -1)
            {
                // Make sure that \hich is always written after a \loch\loch.
                OString sRemaining = sLine.copy(nIndex);
                CPPUNIT_ASSERT(sRemaining.indexOf("\\hich") != -1);
            }
        }
    }
}

DECLARE_RTFEXPORT_TEST(testTdf103925, "tdf103925.rtf")
{
    // This was true, \animtext0 resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_RTFEXPORT_TEST(testTdf104228, "tdf104228.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("C1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCell->getText());
    // This was 2103, implicit 0 as direct formatting was ignored on the
    // paragraph (and the style had this larger value).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf104085, "tdf104085.rtf")
{
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(properties->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (beans::PropertyValue const& prop : std::as_const(aProps))
    {
        if (prop.Name == "BulletChar")
            return;
    }
    CPPUNIT_FAIL("no BulletChar property");
}

DECLARE_RTFEXPORT_TEST(testTdf113550, "tdf113550.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xParagraph = getParagraphOfText(1, xCell->getText());
    // This was 2501, 0 as direct formatting was ignored on the paragraph (and
    // the style had this larger value).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xParagraph, "ParaLeftMargin"));
}

DECLARE_RTFEXPORT_TEST(testLeveljcCenter, "leveljc-center.rtf")
{
    // Tests that \leveljc1 is mapped to Adjust=Center for a numbering rule.
    uno::Reference<text::XTextRange> xPara(getParagraph(1));
    uno::Reference<beans::XPropertySet> properties(xPara, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(properties->getPropertyValue("NumberingRules"),
                                                    uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps;
    for (beans::PropertyValue const& prop : std::as_const(aProps))
    {
        if (prop.Name == "Adjust")
        {
            sal_Int16 nValue = 0;
            CPPUNIT_ASSERT(prop.Value >>= nValue);
            CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, nValue);
            return;
        }
    }
    CPPUNIT_FAIL("no Adjust property");
}

DECLARE_RTFEXPORT_TEST(testHyperlinkTarget, "hyperlink-target.rtf")
{
    // This was empty, hyperlink target was lost on import.
    CPPUNIT_ASSERT_EQUAL(OUString("_blank"),
                         getProperty<OUString>(getRun(getParagraph(1), 1), "HyperLinkTarget"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107620)
{
    loadAndReload("tdf107620.docx");
    // This failed, RTF export didn't write the \htmautsp compat flag, the
    // original bugdoc resulting in 2 pages instead of 1.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSettings(
        xFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
    bool bAddParaTableSpacing = true;
    xSettings->getPropertyValue("AddParaTableSpacing") >>= bAddParaTableSpacing;
    CPPUNIT_ASSERT(!bAddParaTableSpacing);
}

DECLARE_RTFEXPORT_TEST(testTdf104937, "tdf104937.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    auto aSeparators = getProperty<uno::Sequence<text::TableColumnSeparator>>(
        xTableRows->getByIndex(1), "TableColumnSeparators");
    // First table's second row had 9 cells (so 8 separators).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(8), aSeparators.getLength());
    // This was 3174, i.e. last cell was wider than expected, while others were
    // narrower.
    CPPUNIT_ASSERT_GREATER(static_cast<sal_Int16>(4500), aSeparators[7].Position);
}

DECLARE_RTFEXPORT_TEST(testTdf112507, "tdf112507.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XTableRows> xTableRows = xTable->getRows();
    auto aSeparators = getProperty<uno::Sequence<text::TableColumnSeparator>>(
        xTableRows->getByIndex(1), "TableColumnSeparators");
    // First table's second row had 3 cells (so 2 separators).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), aSeparators.getLength());
    // This was 3333, i.e. the B2 cell was too narrow and the text needed 2 lines.
    CPPUNIT_ASSERT_GREATEREQUAL(5000, aSeparators[1].Position - aSeparators[0].Position);
}

DECLARE_RTFEXPORT_TEST(testTdf107480, "tdf107480.rtf")
{
    // These were 176 (100 twips), as \htmautsp was parsed too late.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getParagraph(1), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(1), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(convertTwipToMm100(280)),
                         getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
}

DECLARE_RTFEXPORT_TEST(testWatermark, "watermark.rtf")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("WatermarkRTF"), xShape->getString());

    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    OUString aFont;
    float nFontSize;

    // Check transparency
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));

    // Check font family
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("CharFontName") >>= aFont);
    CPPUNIT_ASSERT_EQUAL(OUString("DejaVu Serif"), aFont);

    // Check font size
    CPPUNIT_ASSERT(xPropertySet->getPropertyValue("CharHeight") >>= nFontSize);
    CPPUNIT_ASSERT_EQUAL(float(66), nFontSize);
}

DECLARE_RTFEXPORT_TEST(testTdf109790, "tdf109790.rtf")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(2), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // Style information was reset, which caused character height to be 22.
    CPPUNIT_ASSERT_EQUAL(
        10.f, getProperty<float>(getRun(getParagraphOfText(1, xCell->getText()), 1), "CharHeight"));
}

DECLARE_RTFEXPORT_TEST(testTdf112211, "tdf112211.rtf")
{
    // This was 0, \fi in a list level definition was not imported.
    auto xRules = getProperty<uno::Reference<container::XIndexAccess>>(
        getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    comphelper::SequenceAsHashMap aRule(xRules->getByIndex(0));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-635), aRule["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_RTFEXPORT_TEST(testTdf113202, "tdf113202.rtf")
{
    // This failed, contextual spacing in 4th paragraph was lost.
    CPPUNIT_ASSERT(getProperty<bool>(getParagraph(4), "ParaContextMargin"));
}

DECLARE_RTFEXPORT_TEST(testTdf156030, "tdf156030.rtf")
{
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // As usual, fields given by FieldsAccess are not in same order as in the document
    std::vector<std::vector<OUString>> aExpectedValues = {
        { "1 FORMULA 2", "true value 1", "false value 1" }, // #1, condition should be 1 = 2
        { "", "", "" }, // #9, not enough field params
        { "1 1 FORMULA 1 2 1 true value 8 fal", "se", "value 8" }, // #8, nonsense in field params
        { "1 1 FORMULA 1 2 1 true value 7 false", "value", "7" }, // #7, another parse error
        { "1 < 2", "true value 6", "false value 6" }, // #6
        { "1 > 2", "true value 5", "false value 5" }, // #5
        { "1 <> 2", "true value 4", "false value 4" }, // #4
        { "1 != 2", "true value 3", "false value 3" }, // #3
        { "1 FORMULA FORMULA 2", "true value 2", "false value 2" }, // #2, condition expected 1 == 2
    };
    uno::Reference<beans::XPropertySet> xPropertySet;
    OUString sValue;

    for (const auto& aValues : aExpectedValues)
    {
        xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xPropertySet.is());
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("Condition") >>= sValue);
        CPPUNIT_ASSERT_EQUAL(aValues[0], sValue);
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("TrueContent") >>= sValue);
        CPPUNIT_ASSERT_EQUAL(aValues[1], sValue);
        CPPUNIT_ASSERT(xPropertySet->getPropertyValue("FalseContent") >>= sValue);
        CPPUNIT_ASSERT_EQUAL(aValues[2], sValue);
    }

    // No more fields
    CPPUNIT_ASSERT(!xFields->hasMoreElements());
}

DECLARE_RTFEXPORT_TEST(testTdf153195, "tdf153195.rtf")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);

    // Table margin test (cannot be set to exact value, it jumps between -1991 and -1983)
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(-2182),
                                 getProperty<sal_Int32>(xTables->getByIndex(0), "LeftMargin"), 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(-2182),
                                 getProperty<sal_Int32>(xTables->getByIndex(1), "LeftMargin"), 10);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
