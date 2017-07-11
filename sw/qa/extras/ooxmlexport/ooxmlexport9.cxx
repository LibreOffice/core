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
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
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

DECLARE_OOXMLEXPORT_TEST(testFdo55381, "fdo55381.docx")
{
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    CPPUNIT_ASSERT_EQUAL(sal_Int16(4), xCursor->getPage());
    //TODO: frames not located on the correct pages
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

DECLARE_OOXMLEXPORT_TEST(testTdf92045, "tdf92045.docx")
{
    // This was true, <w:effect w:val="none"/> resulted in setting the blinking font effect.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(getRun(getParagraph(1), 1), "CharFlash"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf106690Cell, "tdf106690-cell.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    // This was 0, bottom margin of the second paragraph in the A1 table cell
    // had a reduced auto-space, just because of a next paragraph in the A2
    // cell.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf106970, "tdf106970.docx")
{
    // The second paragraph (first numbered one) had 0 bottom margin:
    // autospacing was even collapsed between different numbering styles.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(2), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(3), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraph(4), "ParaBottomMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testKern, "kern.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoKerning"));
    // This failed: kerning was also enabled for the second paragraph.
    CPPUNIT_ASSERT(!getProperty<bool>(getRun(getParagraph(2), 1), "CharAutoKerning"));

    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Default Style"), uno::UNO_QUERY);
    //tdf107801: kerning normally isn't enabled by default for .docx
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be false", false, getProperty<bool>(xStyle, "CharAutoKerning"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf89377, "tdf89377_tableWithBreakBeforeParaStyle.docx")
{
    // the paragraph style should set table's text-flow break-before-page
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );

    uno::Reference<beans::XPropertySet> xStyle(getStyles("ParagraphStyles")->getByName("Default Style"), uno::UNO_QUERY);
    //tdf107801: kerning info wasn't exported previously.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("AutoKern should be true", true, getProperty<bool>(xStyle, "CharAutoKerning"));
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

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(tdf105490_negativeMargins, "tdf105490_negativeMargins.docx")
{
    // negative margins should change to minimal margins, not default margins.
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf104061_tableSectionColumns,"tdf104061_tableSectionColumns.docx")
{
    CPPUNIT_ASSERT_MESSAGE("There should be two or three pages", getPages() <= 3 );
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

// base class to supply a helper method for testHFLinkToPrev
class testHFBase : public Test
{
protected:
    OUString
    getHFText(const uno::Reference<style::XStyle>& xPageStyle,
              const OUString &sPropName)
    {
        auto xTextRange = getProperty< uno::Reference<text::XTextRange> >(
            xPageStyle, sPropName);
        return xTextRange->getString();
    }
};

DECLARE_SW_EXPORT_TEST(testHFLinkToPrev, "headerfooter-link-to-prev.docx", nullptr, testHFBase)
{
    uno::Reference<container::XNameAccess> xPageStyles = getStyles("PageStyles");

    // get a page cursor
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(
        xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY);

    // get LO page style for page 1, corresponding to docx section 1 first page
    xCursor->jumpToFirstPage();
    OUString pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    uno::Reference<style::XStyle> xPageStyle(
        xPageStyles->getByName(pageStyleName), uno::UNO_QUERY);
    // check page 1 header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for section 1 only"));

    // get LO page style for page 2, corresponding to docx section 1
    xCursor->jumpToPage(2);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for section 1 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for section 1 only"));

    // get LO page style for page 4, corresponding to docx section 2 first page
    xCursor->jumpToPage(4);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for sections 2 and 3 only"));

    // get LO page style for page 5, corresponding to docx section 2
    xCursor->jumpToPage(5);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for sections 2 and 3 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for sections 2 and 3 only"));

    // get LO page style for page 7, corresponding to docx section 3 first page
    xCursor->jumpToPage(7);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("First page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("First page footer for sections 2 and 3 only"));

    // get LO page style for page 8, corresponding to docx section 3
    xCursor->jumpToPage(8);
    pageStyleName = getProperty<OUString>(xCursor, "PageStyleName");
    xPageStyle.set( xPageStyles->getByName(pageStyleName), uno::UNO_QUERY );
    // check header & footer text
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderTextLeft"),
        OUString("Even page header for sections 2 and 3 only"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterTextLeft"),
        OUString("Even page footer for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "HeaderText"),
        OUString("Odd page header for all sections"));
    CPPUNIT_ASSERT_EQUAL(getHFText(xPageStyle, "FooterText"),
        OUString("Odd page footer for sections 2 and 3 only"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakB, "tdf103975_notPageBreakB.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(4), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL( 4, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakC, "tdf103975_notPageBreakC.docx")
{
    // turn on View Formatting Marks to see these documents.
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(1), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(4), "TextSection");
    CPPUNIT_ASSERT(xTextSection.is());
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());

    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL( 4, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakD, "tdf103975_notPageBreakD.docx")
{
    // The problem was that the column break was moving outside of the columns, making a page break.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf103975_notPageBreakE, "tdf103975_notPageBreakE.docx")
{
    // The problem was that the column break was getting lost.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_COLUMN_BEFORE, getProperty<style::BreakType>(getParagraph(2), "BreakType"));
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

DECLARE_OOXMLEXPORT_TEST(testBnc519228OddBreaks, "bnc519228_odd-breaksB.docx")
{
    // Check that all the normal styles are not set as right-only, those should be only those used after odd page breaks.
    uno::Reference<beans::XPropertySet> defaultStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), defaultStyle->getPropertyValue("PageStyleLayout"));
    uno::Reference<beans::XPropertySet> firstPage( getStyles("PageStyles")->getByName("First Page"), uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), firstPage->getPropertyValue("PageStyleLayout"));

    OUString page1StyleName = getProperty<OUString>( getParagraph( 1, "This is the first page." ), "PageDescName");
    uno::Reference<beans::XPropertySet> page1Style(getStyles("PageStyles")->getByName(page1StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT), page1Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page1Style, "HeaderText"), "This is the header for odd pages");

    // Page2 comes from follow of style for page 1 and should be a normal page. Also check the two page style have the same properties,
    // since page style for page1 was created from page style for page 2.
    uno::Any page2StyleAny = uno::Reference<beans::XPropertySet>(
        getParagraph(2, "This is page 2, which is obviously an even page."),
        uno::UNO_QUERY_THROW)->getPropertyValue("PageDescName");
    CPPUNIT_ASSERT_EQUAL(uno::Any(), page2StyleAny);
    OUString page2StyleName = getProperty<OUString>( page1Style, "FollowStyle" );
    uno::Reference<beans::XPropertySet> page2Style(getStyles("PageStyles")->getByName(page2StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_ALL), page2Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page2Style, "HeaderTextLeft"), "This is the even header");
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page2Style, "HeaderTextRight"), "This is the header for odd pages");
    CPPUNIT_ASSERT_EQUAL(getProperty<sal_Int32>(page1Style, "TopMargin"), getProperty<sal_Int32>(page2Style, "TopMargin"));

    OUString page5StyleName = getProperty<OUString>( getParagraph( 4, "Then an odd break after an odd page, should lead us to page #5." ), "PageDescName");
    uno::Reference<beans::XPropertySet> page5Style(getStyles("PageStyles")->getByName(page5StyleName), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(uno::makeAny(style::PageStyleLayout_RIGHT), page5Style->getPropertyValue("PageStyleLayout"));
    getParagraphOfText( 1, getProperty< uno::Reference<text::XText> >(page5Style, "HeaderText"), "This is the header for odd pages");
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
    CPPUNIT_ASSERT_EQUAL( sal_Int32( -1 ) , sContent.indexOf( u'\xf04a' ));

    // this document has a w:kern setting in the DocDefault character properties.  Ensure it applies.
    CPPUNIT_ASSERT(getProperty<bool>(getRun(getParagraph(1), 1), "CharAutoKerning"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf99227, "tdf99227.docx")
{
    // A drawing anchored as character to a footnote caused write past end of document.xml at export to docx.
    // After that, importing after export failed with
    // SAXParseException: '[word/document.xml line 2]: Extra content at the end of the document', Stream 'word / document.xml',
    // and before commit ebf767eeb2a169ba533e1b2ffccf16f41d95df35, the drawing was silently lost.
    xmlDocPtr pXmlDoc = parseExport("word/footnotes.xml");
    if (!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "//w:footnote/w:p/w:r/w:drawing", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_footnoteStyle, "tdf82173_footnoteStyle.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);

    uno::Reference<text::XText> xFootnoteText;
    xFootnotes->getByIndex(0) >>= xFootnoteText;
    // This was footnote text, which didn't match with newly created footnotes
    CPPUNIT_ASSERT_EQUAL(OUString("Footnote"), getProperty<OUString>(getParagraphOfText(1, xFootnoteText), "ParaStyleName"));

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("CharacterStyles")->getByName("Footnote Characters"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, "CharEscapementHeight") );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0x00FF00), getProperty< sal_Int32 >(xPageStyle, "CharColor") );

    xPageStyle.set(getStyles("CharacterStyles")->getByName("Footnote anchor"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, "CharEscapementHeight") );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0x00FF00), getProperty< sal_Int32 >(xPageStyle, "CharColor") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf82173_endnoteStyle, "tdf82173_endnoteStyle.docx")
{
    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    // character properties were previously not assigned to the footnote/endnote in-text anchor.
    CPPUNIT_ASSERT_EQUAL( 24.0f, getProperty< float >(xEndnote->getAnchor(), "CharHeight") );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0xFF0000), getProperty< sal_Int32 >(xEndnote->getAnchor(), "CharColor") );

    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;
    // This was Endnote Symbol
    CPPUNIT_ASSERT_EQUAL(OUString("Endnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x993300), getProperty<sal_Int32>(getParagraphOfText(1, xEndnoteText), "CharColor"));

    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("CharacterStyles")->getByName("Endnote Characters"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, "CharEscapementHeight") );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0xFF00FF), getProperty< sal_Int32 >(xPageStyle, "CharColor") );

    xPageStyle.set(getStyles("CharacterStyles")->getByName("Endnote anchor"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( sal_Int32(58),       getProperty< sal_Int32 >(xPageStyle, "CharEscapementHeight") );
    CPPUNIT_ASSERT_EQUAL( sal_Int32(0xFF00FF), getProperty< sal_Int32 >(xPageStyle, "CharColor") );
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

DECLARE_OOXMLEXPORT_TEST(testTdf106492, "tdf106492.docx")
{
    if (xmlDocPtr pXmlDoc = parseExport())
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
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    // This was 0: footnote separator was disabled even in case the document
    // had no footnotes.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(25), getProperty<sal_Int32>(xPageStyle, "FootnoteLineRelativeWidth"));
}

#if HAVE_MORE_FONTS
DECLARE_OOXMLEXPORT_TEST(testTdf107889, "tdf107889.docx")
{
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // This was 1, multi-page table was imported as a floating one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDrawPage->getCount());
}
#endif

DECLARE_OOXMLEXPORT_TEST(testTdf107837, "tdf107837.odt")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTextSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // This was true, a balanced section from ODF turned into a non-balanced one after OOXML roundtrip.
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xTextSections->getByIndex(0), "DontBalanceTextColumns"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107684, "tdf107684.odt")
{
    if (xmlDocPtr pXmlDoc = parseExport("word/styles.xml"))
        // This was 1, <w:outlineLvl> was duplicated for Heading1.
        assertXPath(pXmlDoc, "//w:style[@w:styleId='Heading1']/w:pPr/w:outlineLvl", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107618, "tdf107618.doc")
{
    // This was false, header was lost on export.
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf108682, "tdf108682.docx")
{
    auto aLineSpacing = getProperty<style::LineSpacing>(getParagraph(1), "ParaLineSpacing");
    // This was style::LineSpacingMode::PROP.
    CPPUNIT_ASSERT_EQUAL(style::LineSpacingMode::FIX, aLineSpacing.Mode);
    // 260 twips in mm100, this was a negative value.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(459), aLineSpacing.Height);
}

DECLARE_OOXMLEXPORT_TEST(testTdf100075, "tdf100075.docx")
{
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(), uno::UNO_QUERY);

    // There are two frames in document
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xIndexAccess->getCount());

    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xIndexAccess->getByIndex(1), uno::UNO_QUERY);

    // Ensure that frame#1 height is more that frame#2: if no hRul attribute
    // defined, MS Word will use hRul=auto if height is not defined,
    // and hRul=atLeast if height is provided. So frame#1 should be higher
    CPPUNIT_ASSERT(getProperty<sal_Int32>(xFrame1, "Height") > getProperty<sal_Int32>(xFrame2, "Height"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf105095, "tdf105095.docx")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content
    // was lost on import.
    CPPUNIT_ASSERT(xTextRange->getString().endsWith("\tfootnote"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
