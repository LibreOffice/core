/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_fonts.h>

#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <tools/UnitConversion.hxx>

#include <ftninfo.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf104115)
{
    loadAndSave("tdf104115.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This found 0 nodes: the custom geometry was not written for the Bezier
    // curve -> Word refused to open the document.
    assertXPath(pXmlDoc, "//a:custGeom", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf103651, "tdf103651.docx")
{
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(1), 1),
                                                           u"TextField"_ustr);
    OUString sContent;
    xTextField->getPropertyValue(u"Content"_ustr) >>= sContent;
    // Comment in the first paragraph should not have smiley ( 0xf04a ).
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1), sContent.indexOf(u'\xf04a'));

    // this document has a w:kern setting in the DocDefault character properties.  Ensure it applies.
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), u"CharAutoKerning"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99227)
{
    loadAndSave("tdf99227.docx");
    // A drawing anchored as character to a footnote caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word / document.xml',
    // and before commit ebf767eeb2a169ba533e1b2ffccf16f41d95df35, the drawing was silently lost.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footnotes.xml"_ustr);

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.docx")
{
    // NOTE: this is now known to have nothing to do with considerWrapOnObjPos...

    // Given a Word 2007 file WITHOUT compat flag doNotVertAlignCellWithSp
    // the fly should be at the bottom of the cell (since the cell calls for bottom-align).
    // Because the fly is layout-in-cell (although it is also wrap-through),
    // it SHOULD try to stay within the cell (although it can escape if necessary).
    // TODO: Since it is not necessary to escape, it should push the paragraph marker upwards...

    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH,
                         getProperty<text::WrapTextMode>(getShape(1), u"Surround"_ustr));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::VertOrientation::BOTTOM,
        getProperty<sal_Int16>(xTable->getCellByName(u"A1"_ustr), u"VertOrient"_ustr));

    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    // For MSO compatibility, the textbox should be at the bottom of the cell, not below the cell
    // sal_Int32 nFlyBottom = getXPath(pXmlDoc, "//cell[1]//fly/infos/bounds", "bottom").toInt32();
    // CPPUNIT_ASSERT_MESSAGE("FlyBottom should be ~ 5810, not 6331", sal_Int32(5810), nFlyBottom);

    sal_Int32 nTextTop
        = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 3856", nTextTop > 3000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112446_frameStyle, "tdf112446_frameStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE,
                         getProperty<sal_Int16>(getShape(1), u"HoriOrient"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_footnoteStyle, "tdf82173_footnoteStyle.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was footnote text, which didn't match with newly created footnotes
    CPPUNIT_ASSERT_EQUAL(
        u"Footnote"_ustr,
        getProperty<OUString>(getParagraphOfText(1, xFootnoteText), u"ParaStyleName"_ustr));

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(u"Footnote Symbol"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58),
                         getProperty<sal_Int32>(xPageStyle, u"CharEscapementHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    xPageStyle.set(getStyles(u"CharacterStyles"_ustr)->getByName(u"Footnote anchor"_ustr),
                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58),
                         getProperty<sal_Int32>(xPageStyle, u"CharEscapementHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTGREEN, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    //tdf#118361 - in RTL locales, the footnote separator should still be left aligned.
    uno::Any aPageStyle = getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator LTR", sal_Int16(0),
                                 getProperty<sal_Int16>(aPageStyle, u"FootnoteLineAdjust"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_endnoteStyle, "tdf82173_endnoteStyle.docx")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    // character properties were previously not assigned to the footnote/endnote in-text anchor.
    CPPUNIT_ASSERT_EQUAL(24.0f, getProperty<float>(xEndnote->getAnchor(), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED,
                         getProperty<Color>(xEndnote->getAnchor(), u"CharColor"_ustr));

    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;
    // This was Endnote Symbol
    CPPUNIT_ASSERT_EQUAL(u"Endnote"_ustr, getProperty<OUString>(getParagraphOfText(1, xEndnoteText),
                                                                u"ParaStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL(Color(0x993300), getProperty<Color>(getParagraphOfText(1, xEndnoteText),
                                                             u"CharColor"_ustr));

    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"CharacterStyles"_ustr)->getByName(u"Endnote Symbol"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58),
                         getProperty<sal_Int32>(xPageStyle, u"CharEscapementHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTMAGENTA, getProperty<Color>(xPageStyle, u"CharColor"_ustr));

    xPageStyle.set(getStyles(u"CharacterStyles"_ustr)->getByName(u"Endnote anchor"_ustr),
                   uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(58),
                         getProperty<sal_Int32>(xPageStyle, u"CharEscapementHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTMAGENTA, getProperty<Color>(xPageStyle, u"CharColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf55427_footnote2endnote)
{
    auto verify = [this](bool bIsExport = false) {
        CPPUNIT_ASSERT_EQUAL(4, getPages());
        uno::Reference<beans::XPropertySet> xPageStyle(
            getStyles(u"ParagraphStyles"_ustr)->getByName(u"Footnote"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote style is rose color", Color(0xFF007F),
                                     getProperty<Color>(xPageStyle, u"CharColor"_ustr));
        xPageStyle.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Endnote"_ustr),
                       uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Endnote style is cyan3 color", Color(0x2BD0D2),
                                     getProperty<Color>(xPageStyle, u"CharColor"_ustr));

        SwDoc* pDoc = getSwDoc();
        // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote numbering type", SVX_NUM_ARABIC,
                                     pDoc->GetFootnoteInfo().m_aFormat.GetNumberingType());
        // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Endnote numbering type", SVX_NUM_ROMAN_LOWER,
                                     pDoc->GetEndNoteInfo().m_aFormat.GetNumberingType());

        uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();

        uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> xEndnotes = xEndnotesSupplier->getEndnotes();
        uno::Reference<text::XFootnote> xEndnote;
        xEndnotes->getByIndex(0) >>= xEndnote;
        uno::Reference<text::XText> xEndnoteText;
        xEndnotes->getByIndex(0) >>= xEndnoteText;

        // ODT footnote-at-document-end's closest DOCX match is an endnote, so the two imports will not exactly match by design.
        if (!bIsExport)
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("original footnote count", sal_Int32(5),
                                         xFootnotes->getCount());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote count", sal_Int32(1),
                                         xEndnotes->getCount());

            uno::Reference<text::XFootnote> xFootnote;
            xFootnotes->getByIndex(0) >>= xFootnote;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("original footnote's number", u"1"_ustr,
                                         xFootnote->getAnchor()->getString());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote's number", u"i"_ustr,
                                         xEndnote->getAnchor()->getString());

            uno::Reference<text::XText> xFootnoteText;
            xFootnotes->getByIndex(0) >>= xFootnoteText;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "original footnote style", u"Footnote"_ustr,
                getProperty<OUString>(getParagraphOfText(1, xFootnoteText), u"ParaStyleName"_ustr));
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "original endnote style", u"Endnote"_ustr,
                getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr));
        }
        else
        {
            // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
            CPPUNIT_ASSERT_EQUAL_MESSAGE("At-Document-End footnotes were converted into endnotes",
                                         sal_Int32(0), xFootnotes->getCount());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("At-Document-End footnotes became endnotes", sal_Int32(6),
                                         xEndnotes->getCount());

            CPPUNIT_ASSERT_EQUAL_MESSAGE("converted footnote's number", u"i"_ustr,
                                         xEndnote->getAnchor()->getString());
            xEndnotes->getByIndex(4) >>= xEndnote;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("original endnote's new number", u"v"_ustr,
                                         xEndnote->getAnchor()->getString());

            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "retained footnote style", u"Footnote"_ustr,
                getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr));
            xEndnotes->getByIndex(4) >>= xEndnoteText;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                "original endnote style", u"Endnote"_ustr,
                getProperty<OUString>(getParagraphOfText(1, xEndnoteText), u"ParaStyleName"_ustr));
        }
    };
    createSwDoc("tdf55427_footnote2endnote.odt");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104162, "tdf104162.docx")
{
    // This crashed: the comment field contained a table with a <w:hideMark/>.
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XElementAccess> xTextFields(xTextFieldsSupplier->getTextFields());
    CPPUNIT_ASSERT(xTextFields->hasElements());
}

DECLARE_OOXMLEXPORT_TEST(testTdf104150, "tdf104150.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0xff0000, i.e. red: background shape wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1),
                         getProperty<sal_Int32>(xPageStyle, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103976, "tdf103976.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    // This was 0, table style inheritance went wrong and w:afterLines had priority over w:after.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(convertTwipToMm100(60)),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));

    // tdf#116549: heading 2 style should not have a bottom border.
    uno::Reference<beans::XPropertySet> xStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Heading 2"_ustr), uno::UNO_QUERY);
    table::BorderLine2 aBottomBorder
        = getProperty<table::BorderLine2>(xStyle, u"BottomBorder"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), aBottomBorder.LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001, "tdf106001.docx")
{
    // This was 0 (1 on UI), while Word treats outliers as 100 (outlier = not in [1..600])
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int16>(100),
        getProperty<sal_Int16>(getRun(getParagraph(1), 1), u"CharScaleWidth"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106001_2)
{
    loadAndSave("tdf106001-2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // In test ODT CharScaleWidth = 900, this was not changed upon OOXML export to stay in [1..600], now it's clamped to 600
    // Note: we disregard what's set in pPr / rPr and only care about r / rPr
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:w", "val", u"600");
}

DECLARE_OOXMLEXPORT_TEST(testTdf99074, "tdf99074.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> const xController(xModel->getCurrentController(),
                                                                  uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xViewSettings(xController->getViewSettings());

    // The behavior changed - Word 2013 and 2016 ignore this setting on
    // import, and instead honor the user's setting.
    // Let's ignore the <w:view w:val="web"/> too.
    CPPUNIT_ASSERT(!getProperty<bool>(xViewSettings, u"ShowOnlineLayout"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testDefaultSectBreakCols, "default-sect-break-cols.docx")
{
    // First problem: the first two paragraphs did not have their own text section, so the whole document had two columns.
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, u"First."_ustr),
                                                           u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // Second problem: the page style had two columns, while it shouldn't have any.
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xPageStyle, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty<bool>(xTextColumns, u"SeparatorLineIsOn"_ustr);
    CPPUNIT_ASSERT(!bValue);
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnSeparator, "multi-column-separator-with-line.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, u"First data."_ustr),
                                                           u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be TRUE as the document contains separator line.
    bool bValue = getProperty<bool>(xTextColumns, u"SeparatorLineIsOn"_ustr);
    CPPUNIT_ASSERT(bValue);
}

DECLARE_OOXMLEXPORT_TEST(testUnbalancedColumns, "unbalanced-columns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(
        true, getProperty<bool>(xTextSections->getByIndex(2), u"DontBalanceTextColumns"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf121670_columnsInSectionsOnly,
                         "tdf121670_columnsInSectionsOnly.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "DontBalanceTextColumns?", true,
        getProperty<bool>(xTextSections->getByIndex(0), u"DontBalanceTextColumns"_ustr));

    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), u"TextSection"_ustr);
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of columns", sal_Int16(3), xTextColumns->getColumnCount());

    xTextSection.set(
        getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(3), u"TextSection"_ustr));
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns.set(
        getProperty<uno::Reference<text::XTextColumns>>(xTextSection, u"TextColumns"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of columns", sal_Int16(0), xTextColumns->getColumnCount());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106492)
{
    loadAndSave("tdf106492.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 4: an additional sectPr was added to the document.
    assertXPath(pXmlDoc, "//w:sectPr", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107104, "tdf107104.docx")
{
    CPPUNIT_ASSERT(getShape(1)->getSize().Width > 0);
    // This failed: the second arrow was invisible because it had zero width.
    CPPUNIT_ASSERT(getShape(2)->getSize().Width > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107033, "tdf107033.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    // This was 0: footnote separator was disabled even in case the document
    // had no footnotes.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25),
                         getProperty<sal_Int32>(xPageStyle, u"FootnoteLineRelativeWidth"_ustr));
}

#if HAVE_MORE_FONTS
CPPUNIT_TEST_FIXTURE(Test, testTdf107889)
{
    auto verify = [this]() {
        // This was 1, multi-page table was imported as a non-split fly.
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();
        assertXPath(pXmlDoc, "//tab", 2);
    };
    createSwDoc("tdf107889.docx");
    verify();
    saveAndReload(u"Office Open XML Text"_ustr);
    verify();
}
#endif

CPPUNIT_TEST_FIXTURE(Test, testTdf107837)
{
    loadAndReload("tdf107837.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was true, a balanced section from ODF turned into a non-balanced one after OOXML roundtrip.
    CPPUNIT_ASSERT_EQUAL(
        false, getProperty<bool>(xTextSections->getByIndex(0), u"DontBalanceTextColumns"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107684)
{
    loadAndReload("tdf107684.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    // This was 1, <w:outlineLvl> was duplicated for Heading1.
    assertXPath(pXmlDoc, "//w:style[@w:styleId='Heading1']/w:pPr/w:outlineLvl", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107618)
{
    loadAndReload("tdf107618.doc");
    // This was false, header was lost on export.
    uno::Reference<beans::XPropertySet> xPageStyle(
        getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, u"HeaderIsOn"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108682, "tdf108682.docx")
{
    auto aLineSpacing = getProperty<style::LineSpacing>(getParagraph(1), u"ParaLineSpacing"_ustr);
    // This was style::LineSpacingMode::PROP.
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::FIX, aLineSpacing.Mode);
    // 260 twips in mm100, this was a negative value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(459), aLineSpacing.Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf100075, "tdf100075.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);

    // There are two frames in document
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Ensure that frame#1 height is more that frame#2: if no hRul attribute
    // defined, MS Word will use hRul=auto if height is not defined,
    // and hRul=atLeast if height is provided. So frame#1 should be higher
    CPPUNIT_ASSERT(getProperty<sal_Int32>(xFrame1, u"Height"_ustr)
                   > getProperty<sal_Int32>(xFrame2, u"Height"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf105095, "tdf105095.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content
    // was lost on import.
    CPPUNIT_ASSERT_EQUAL(u"\tfootnote"_ustr, xTextRange->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf106062_nonHangingFootnote)
{
    loadAndReload("tdf106062_nonHangingFootnote.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes = xFootnotesSupplier->getFootnotes();
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE("Footnote starts with a tab", xTextRange->getString().startsWith("\t"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXTextfield, "activex_textbox.docx")
{
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.TextField"_ustr)));

    // Check textfield is multi-line
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, u"MultiLine"_ustr));

    uno::Reference<drawing::XControlShape> xControlShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape2.is());

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape2->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo2(xPropertySet2, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo2->supportsService(u"com.sun.star.form.component.TextField"_ustr)));

    // Check textfield is single-line
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xPropertySet2, u"MultiLine"_ustr));

    // Don't open in design mode when form controls exist
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XFormLayerAccess> xFormLayerAccess(xModel->getCurrentController(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT(!xFormLayerAccess->isFormDesignMode());
}

DECLARE_OOXMLEXPORT_TEST(testActiveXCheckbox, "activex_checkbox.docx")
{
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));

    // Check custom label
    CPPUNIT_ASSERT_EQUAL(u"Custom Caption"_ustr,
                         getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check background color (highlight system color)
    CPPUNIT_ASSERT_EQUAL(Color(0x316AC5),
                         getProperty<Color>(xPropertySet, u"BackgroundColor"_ustr));

    // Check Text color (active border system color)
    CPPUNIT_ASSERT_EQUAL(Color(0xD4D0C8), getProperty<Color>(xPropertySet, u"TextColor"_ustr));

    // Check state of the checkbox
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPropertySet, u"State"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AT_CHARACTER,
        getProperty<text::TextContentAnchorType>(xPropertySet2, u"AnchorType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testActiveXControlAlign)
{
    loadAndReload("activex_control_align.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // First check box aligned as a floating object
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY_THROW);

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Floating Check Box"_ustr,
                         getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AT_CHARACTER,
        getProperty<text::TextContentAnchorType>(xPropertySet2, u"AnchorType"_ustr));

    // Also check position and size
    uno::Reference<drawing::XShape> xShape(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4470), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1427), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5126), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2341), xShape->getPosition().Y);

    // Second check box aligned inline / as character
    xControlShape.set(getShape(2), uno::UNO_QUERY_THROW);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Inline Check Box"_ustr,
                         getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AS_CHARACTER,
        getProperty<text::TextContentAnchorType>(xPropertySet2, u"AnchorType"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(text::VertOrientation::TOP),
                         getProperty<sal_Int32>(xPropertySet2, u"VertOrient"_ustr));

    // Also check position and size
    xShape.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4410), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1083), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1085), xShape->getPosition().Y);

    // Also check the specific OOXML elements
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // For inline controls use w:object as parent element and pictureFrame shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shapetype", "spt", u"75");
    // For floating controls use w:pict as parent element and hostControl shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shapetype", "spt", u"201");

    // Have different shape ids
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shape", "id")
                   != getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shape", "id"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109184, "tdf109184.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Before table background color was white, should be transparent (auto).
    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell1, u"BackColor"_ustr));

    // Cell with auto color but with 15% fill, shouldn't be transparent.
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"B1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(Color(0xd8d8d8), getProperty<Color>(xCell2, u"BackColor"_ustr));

    // Cell with color defined (red).
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTRED, getProperty<Color>(xCell3, u"BackColor"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf111964)
{
    loadAndSave("tdf111964.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Unicode spaces that are not XML whitespace must not be trimmed
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[6]/w:t", vEnSpaces);
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark-shapetype.docx")
{
    uno::Reference<drawing::XShape> xShape1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    uno::Reference<beans::XPropertySet> xPropertySet2(xShape2, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xPropertySet1->getPropertyValue(u"TextAutoGrowHeight"_ustr),
                         xPropertySet2->getPropertyValue(u"TextAutoGrowHeight"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testActiveXControlAtRunEnd)
{
    loadAndReload("activex_control_at_run_end.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Two issues were here:
    //  1) second shape was not export (it is anchored to the end of the run)
    //  2) inline property was inherited to the second shape by mistake

    // First checkbox is the inlined one
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Inline Checkbox"_ustr,
                         getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AS_CHARACTER,
        getProperty<text::TextContentAnchorType>(xPropertySet2, u"AnchorType"_ustr));

    // Second check box anchored to character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"Floating Checkbox"_ustr,
                         getProperty<OUString>(xPropertySet, u"Label"_ustr));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        text::TextContentAnchorType_AT_CHARACTER,
        getProperty<text::TextContentAnchorType>(xPropertySet2, u"AnchorType"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXOptionButtonGroup, "activex_option_button_group.docx")
{
    // Optionbutton groups were not handled
    // The two optionbutton should have the same group name
    static constexpr OUString sGroupName = u"GroupX"_ustr;

    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, u"GroupName"_ustr));

    xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, u"GroupName"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, tdf112169)
{
    loadAndSave("tdf112169.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(6, getPages());
    // LO crashed while export because of character background color handling

    //tdf76683 - Cannot be negative number - use firstLine instead of hanging
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/numbering.xml"_ustr);
    assertXPathNoAttribute(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pPr/w:ind",
                           "hanging");
    assertXPath(pXmlDoc, "/w:numbering/w:abstractNum[1]/w:lvl[1]/w:pPr/w:ind", "firstLine", u"360");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf103090)
{
    loadAndSave("tdf103090.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Get bookmark name
    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:bookmarkStart", "name");

    // Ensure that name has no spaces
    CPPUNIT_ASSERT(bookmarkName.indexOf(" ") < 0);

    // Get PAGEREF field
    OUString fieldName = getXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText");

    // Ensure that PAGEREF field refers exactly our bookmark
    OUString expectedFieldName = " PAGEREF " + bookmarkName + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName, fieldName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf107111)
{
    loadAndSave("tdf107111.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Ensure that hyperlink and its properties are in place.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:hyperlink/w:r/w:rPr", 1);

    // Ensure that hyperlink properties do not contain <w:webHidden/>.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:hyperlink/w:r/w:rPr/w:webHidden", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789, "tdf90789.docx")
{
    uno::Reference<text::XTextContent> xShape(getShape(1), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xShape->getAnchor() != nullptr);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(),
                                                   uno::UNO_QUERY_THROW);
    xCtrl->select(uno::Any(xShape->getAnchor()));

    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl,
                                                                          uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xTextCursor(xTextViewCursorSupplier->getViewCursor(),
                                                      uno::UNO_SET_THROW);
    uno::Reference<text::XPageCursor> xPageCursor(xTextCursor, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xPageCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789_2, "tdf90789-2.docx")
{
    // Section break before frame and shape was ignored
    CPPUNIT_ASSERT_EQUAL(3, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354_2, "tdf104354-2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

    // top margin of the first paragraph and bottom margin of the last paragraph
    // is zero, when auto spacing is used.

    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaBottomMargin"_ustr));

    // top margin is not auto spacing
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(847),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), u"ParaBottomMargin"_ustr));

    // bottom margin is not auto spacing
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName(u"A3"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(847),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing, if the paragraph contains footnotes
    uno::Reference<text::XTextRange> xCell4(xTable->getCellByName(u"A4"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing is explicitly disabled, and no margins are defined.
    xCell.set(xTable->getCellByName(u"A5"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaBottomMargin"_ustr));

    // auto spacing on a paragraph
    uno::Reference<text::XTextTable> xTable2(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell5(xTable2->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf137593)
{
    loadAndSave("tdf137593.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // zero auto spacing, if the first paragraph contains text boxes
    // This was 280.
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:tc/w:p[1]/w:pPr/w:spacing", "before",
                u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115557)
{
    loadAndSave("tdf115557.docx");
    // A chart anchored to a footnote multiplied during import
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/footnotes.xml"_ustr);

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testAlignmentRelativeFromTopMarginDML)
{
    loadAndSave("tdf137641_RelativeFromTopMargin.docx");
    // Import as DML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"top");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"bottom");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"center");
}

CPPUNIT_TEST_FIXTURE(Test, testAlignmentRelativeFromTopMarginVML)
{
    loadAndSave("tdf137642_Vertical_Alignment_toppage.docx");
    // Import as VML.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"top");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"bottom");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                "wp:anchor/wp:positionV",
                "relativeFrom", u"topMargin");
    assertXPathContent(pXmlDoc,
                       "/w:document/w:body/w:p/w:r/mc:AlternateContent[3]/mc:Choice/w:drawing/"
                       "wp:anchor/wp:positionV/wp:align",
                       u"center");
}

CPPUNIT_TEST_FIXTURE(Test, testVmlShapeWithTextbox)
{
    loadAndSave("tdf41466_testVmlShapeWithTextbox.docx");
    // Import as VML.
    // tdf#41466: check whether VML DOCX shape with text is imported as shape with a text frame
    // (text box). These kind of shapes were imported only as text frames previously, losing the
    // preset shape geometry, in this case "wedgeRectCallout".
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // the wrong value was "rect" instead of "wedgeRectCallout"
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/"
                "mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/a:graphic/a:graphicData/wps:wsp/"
                "wps:spPr/a:prstGeom",
                "prst", u"wedgeRectCallout");
}

CPPUNIT_TEST_FIXTURE(Test, testLayoutFlowAltAlone)
{
    loadAndSave("layout-flow-alt-alone.docx");
    // moved from oox/qa/unit/vml.cxx
    // FIXME: now the DML part is checked, but we should check VML part in Fallback (too)
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:bodyPr",
                "vert", u"vert270");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
