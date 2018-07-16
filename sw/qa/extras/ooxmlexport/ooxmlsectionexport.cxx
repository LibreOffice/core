/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testMultiColumnLineSeparator, "multi-column-line-separator-SAVED.docx")
{
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:sectPr/w:cols", "sep", "false");
}

DECLARE_OOXMLEXPORT_TEST(testPgMargin, "testPgMargin.docx")
{
    xmlDocPtr pXmlDoc = parseExport();
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgMar", "left", "1440");
}

DECLARE_OOXMLEXPORT_TEST(testcolumnbreak, "columnbreak.docx")
{
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE,
                         getProperty<style::BreakType>(
                             getParagraph(5, "This is first line after col brk."), "BreakType"));
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[1]/w:br", "type", "column");
}

DECLARE_OOXMLEXPORT_TEST(testTdf64372_continuousBreaks, "tdf64372_continuousBreaks.docx")
{
    //There are no page breaks, so everything should be on the first page.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testFDO77812, "fdo77812.docx")
{
    /* Additional sectPr was getting inserted and hence Column properties
    * were getting added into this additional sectPr instead of Default setPr.
    */
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Check no additional section break is inserted.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:sectPr", 0);

    // Check w:cols comes under Default sectPr
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols", "num", "2");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[1]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:cols/w:col[2]", 1);
}

DECLARE_OOXMLEXPORT_TEST(testSectionProtection, "sectionprot.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:sectPr/w:formProt", "val", "true");
        assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:formProt", "val", "false");
    }

    if (xmlDocPtr pXmlSettings = parseExport("word/settings.xml"))
    {
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "enforcement", "true");
        assertXPath(pXmlSettings, "/w:settings/w:documentProtection", "edit", "forms");
    }
}

DECLARE_OOXMLEXPORT_TEST(testSectionHeader, "sectionprot.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/document.xml"))
    {
        assertXPath(pXmlDoc, "//w:headerReference", 1);
    }
}

DECLARE_OOXMLEXPORT_TEST(testFdo52208, "fdo52208.docx")
{
    // The problem was that the document had 2 pages instead of 1.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), xCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf97648_relativeWidth, "tdf97648_relativeWidth.docx")
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(7616), getShape(1)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(8001), getShape(2)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(4001), getShape(3)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_LEFT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(6), "ParaAdjust")));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sal_Int32(1600), getShape(4)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_RIGHT,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(8), "ParaAdjust")));

    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(getShape(1), "LeftMargin"));
    if (!mbExported)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Text should wrap above/below the line",
                                     text::WrapTextMode_NONE,
                                     getProperty<text::WrapTextMode>(getShape(1), "Surround"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER,
                             getProperty<sal_Int16>(getShape(2), "HoriOrient"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT,
                             getProperty<sal_Int16>(getShape(3), "HoriOrient"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT,
                             getProperty<sal_Int16>(getShape(4), "HoriOrient"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf46940_dontEquallyDistributeColumns,
                         "tdf46940_dontEquallyDistributeColumns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
    // This was false, columns before a section-page-break were balanced.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(2), "DontBalanceTextColumns"));
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(3), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf112352_nextPageColumns, "tdf112352_nextPageColumns.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(3), "TextSection");
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

DECLARE_OOXMLEXPORT_TEST(testDefaultSectBreakCols, "default-sect-break-cols.docx")
{
    // First problem: the first two paragraphs did not have their own text section, so the whole document had two columns.
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, "First."),
                                                           "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // Second problem: the page style had two columns, while it shouldn't have any.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    xTextColumns = getProperty<uno::Reference<text::XTextColumns>>(xPageStyle, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty<bool>(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(!bValue);
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnSeparator, "multi-column-separator-with-line.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(1, "First data."),
                                                           "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be TRUE as the document contains separator line.
    bool bValue = getProperty<bool>(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(bValue);
}

DECLARE_OOXMLEXPORT_TEST(testUnbalancedColumns, "unbalanced-columns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true,
                         getProperty<bool>(xTextSections->getByIndex(2), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106492, "tdf106492.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
        // This was 4: an additional sectPr was added to the document.
        assertXPath(pXmlDoc, "//w:sectPr", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107837, "tdf107837.odt")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(),
                                                          uno::UNO_QUERY);
    // This was true, a balanced section from ODF turned into a non-balanced one after OOXML roundtrip.
    CPPUNIT_ASSERT_EQUAL(false,
                         getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf94043, "tdf94043.docx")
{
    auto xTextSection
        = getProperty<uno::Reference<beans::XPropertySet>>(getParagraph(2), "TextSection");
    auto xTextColumns
        = getProperty<uno::Reference<text::XTextColumns>>(xTextSection, "TextColumns");
    // This was 0, the separator line was not visible due to 0 width.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2),
                         getProperty<sal_Int32>(xTextColumns, "SeparatorLineWidth"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
