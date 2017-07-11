/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return (OString(filename).endsWith(".docx"));
    }
};

class DocmTest : public SwModelTestBase
{
public:
    DocmTest()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "MS Word 2007 XML VBA")
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
}

DECLARE_OOXMLEXPORT_TEST(testDocm, "hello.docm")
{
    // Make sure that we check the name of the export filter.
    // This was application/vnd.ms-word.document.macroEnabled.main+xml when the
    // name of the import filter was checked.
    if (xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml"))
        assertXPath(pXmlDoc,
                    "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                    "ContentType",
                    "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
}

DECLARE_SW_ROUNDTRIP_TEST(testDocmSave, "hello.docm", nullptr, DocmTest)
{
    // This was
    // application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml,
    // we used the wrong content type for .docm files.
    if (xmlDocPtr pXmlDoc = parseExport("[Content_Types].xml"))
        assertXPath(pXmlDoc,
                    "/ContentType:Types/ContentType:Override[@PartName='/word/document.xml']",
                    "ContentType",
                    "application/vnd.ms-word.document.macroEnabled.main+xml");
}

DECLARE_SW_ROUNDTRIP_TEST(testBadDocm, "bad.docm", nullptr, DocmTest)
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // This was 'MS Word 2007 XML', broken docm files were not recognized.
    CPPUNIT_ASSERT_EQUAL(OUString("MS Word 2007 XML VBA"), pTextDoc->GetDocShell()->GetMedium()->GetFilter()->GetName());
}

DECLARE_OOXMLEXPORT_TEST(testTdf109063, "tdf109063.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, near-page-width table was imported as a TextFrame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_SW_ROUNDTRIP_TEST(testTdf108269, "tdf108269.docm", nullptr, DocmTest)
{
    if (!mbExported)
        return;

    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getComponentContext(m_xSFactory), maTempFile.GetURL());
    // This failed: VBA streams were not roundtripped via the doc-level
    // grab-bag.
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/vbaProject.bin"));
    CPPUNIT_ASSERT(xNameAccess->hasByName("word/vbaData.xml"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf95031, "tdf95031.docx")
{
    // This was 494, in-numbering paragraph's automating spacing was handled as visible spacing, while it should not.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106690, "tdf106690.docx")
{
    // This was 0, numbering rules with automatic spacing meant 0
    // before/autospacing for all text nodes, even for ones at the start/end of
    // a numbered text node block.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106970, "tdf106970.docx")
{
    // The second paragraph (first numbered one) had 0 bottom margin:
    // autospacing was even collapsed between different numbering styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(4), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf104420, "tdf104420_lostParagraph.docx")
{
    // the add/remove dummy paragraph was losing an entire header and paragraph
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf41542_borderlessPadding, "tdf41542_borderlessPadding.odt")
{
    // the page style's borderless padding should force this to 3 pages, not 1
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf46940_dontEquallyDistributeColumns, "tdf46940_dontEquallyDistributeColumns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
    // This was false, columns before a section-page-break were balanced.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(2), "DontBalanceTextColumns"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(3), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testRhbz988516, "rhbz988516.docx")
{
    // The problem was that the list properties of the footer leaked into body
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(1), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString("Enclosure 3"), getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(3), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(4), "NumberingStyleName"));

    // tdf#103975 The problem was that an empty paragraph with page break info was removed.
    CPPUNIT_ASSERT_EQUAL( 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103389, "tdf103389.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // No geometry was exported for the second canvas
    // Check both canvases' geometry
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", "rect");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wpg:wgp/wps:wsp/wps:spPr/a:prstGeom", "prst", "rect");
}

DECLARE_OOXMLEXPORT_TEST(testTdf84678, "tdf84678.docx")
{
    // This was 0, left margin inside a shape+text wasn't imported from DOCX.
    // 360000 EMU, but layout uses twips.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(567), parseDump("/root/page/body/txt/anchored/fly/infos/prtBounds", "left").toInt32());

}

DECLARE_OOXMLEXPORT_TEST(testTdf103544, "tdf103544.docx")
{
    // We have two shapes: a frame and an image
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xDrawPage->getCount());

    // Image was lost because of the frame export
    uno::Reference<beans::XPropertySet> xImage(getShape(1), uno::UNO_QUERY);
    auto xGraphic = getProperty<uno::Reference<graphic::XGraphic> >(xImage, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103573, "tdf103573.docx")
{
    // Relative positions to the left or right margin (MS Word naming) was not handled.
    uno::Reference<beans::XPropertySet> xShapeProperties( getShape(1), uno::UNO_QUERY );
    sal_Int16 nValue;
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to left page border", text::RelOrientation::PAGE_LEFT, nValue);

    xShapeProperties.set( getShape(2), uno::UNO_QUERY );
    xShapeProperties->getPropertyValue("HoriOrient") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally", text::HoriOrientation::CENTER, nValue);
    xShapeProperties->getPropertyValue("HoriOrientRelation") >>= nValue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Not centered horizontally relatively to right page border", text::RelOrientation::PAGE_RIGHT, nValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf79329, "tdf79329.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // This was 1: only the inner, not the outer table was created.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xTables->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf103982, "tdf103982.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    sal_Int32 nDistB = getXPath(pXmlDoc, "//wp:anchor", "distB").toInt32();
    // This was -260350, which is not a valid value for an unsigned type.
    CPPUNIT_ASSERT(nDistB >= 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104115, "tdf104115.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // This found 0 nodes: the custom geometry was not written for the Bezier
    // curve -> Word refused to open the document.
    assertXPath(pXmlDoc, "//a:custGeom", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf103651, "tdf103651.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 1), "TextField");
    OUString sContent;
    xTextField->getPropertyValue("Content") >>= sContent;
    // Comment in the first paragraph should not have smiley ( 0xf04a ).
    CPPUNIT_ASSERT_EQUAL( sal_Int32( -1 ) , sContent.indexOf( sal_Unicode( 0xf04a ) ));
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
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 0xff0000, i.e. red: background shape wasn't ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(xPageStyle, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf103976, "tdf103976.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, table style inheritance went wrong and w:afterLines had priority over w:after.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(convertTwipToMm100(60)), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001, "tdf106001.docx")
{
    // This was 0 (1 on UI), while Word treats outliers as 100 (outlier = not in [1..600])
    CPPUNIT_ASSERT_EQUAL( static_cast<sal_Int16>( 100 ), getProperty<sal_Int16>(getRun(getParagraph(1), 1), "CharScaleWidth" ));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106001_2, "tdf106001-2.odt")
{
    // In test ODT CharScaleWidth = 900, this was not changed upon OOXML export to stay in [1..600], now it's clamped to 600
    // Note: we disregard what's set in pPr / rPr and only care about r / rPr
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:rPr/w:w","val","600");
}

DECLARE_OOXMLEXPORT_TEST(testTdf99074, "tdf99074.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XViewSettingsSupplier> const xController(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> const xViewSettings(
        xController->getViewSettings());
    // This was false, Web Layout was ignored on import.
    CPPUNIT_ASSERT(getProperty<bool>(xViewSettings, "ShowOnlineLayout"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107104, "tdf107104.docx")
{
    CPPUNIT_ASSERT(getShape(1)->getSize().Width > 0);
    // This failed: the second arrow was invisible because it had zero width.
    CPPUNIT_ASSERT(getShape(2)->getSize().Width > 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107033, "tdf107033.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 0: footnote separator was disabled even in case the document
    // had no footnotes.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testDefaultSectBreakCols, "default-sect-break-cols.docx")
{
    // First problem: the first two paragraphs did not have their own text section, so the whole document had two columns.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, "First."), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    // Second problem: the page style had two columns, while it shouldn't have any.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xPageStyle, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be FALSE as the document does not contain separator line.
    bool bValue = getProperty< bool >(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(!bValue) ;
}

DECLARE_OOXMLEXPORT_TEST(testMultiColumnSeparator, "multi-column-separator-with-line.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1, "First data."), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());
    // Check for the Column Separator value.It should be TRUE as the document contains separator line.
    bool  bValue = getProperty< bool >(xTextColumns, "SeparatorLineIsOn");
    CPPUNIT_ASSERT(bValue);
}

DECLARE_OOXMLEXPORT_TEST(testUnbalancedColumns, "unbalanced-columns.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was false, last section was balanced, but it's unbalanced in Word.
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTextSections->getByIndex(2), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107889, "tdf107889.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf108682, "tdf108682.docx")
{
    auto aLineSpacing = getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing");
    // This was style::LineSpacingMode::PROP.
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::FIX, aLineSpacing.Mode);
    // 260 twips in mm100, this was a negative value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(459), aLineSpacing.Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107684, "tdf107684.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/styles.xml"))
        // This was 1, <w:outlineLvl> was duplicated for Heading1.
        assertXPath(pXmlDoc, "//w:style[@w:styleId='Heading1']/w:pPr/w:outlineLvl", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
