/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
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
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <ftninfo.hxx>
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
        return OString(filename).endsWith(".docx");
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

DECLARE_OOXMLEXPORT_TEST(testTdf79272_strictDxa, "tdf79272_strictDxa.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4318), getProperty<sal_Int32>(xTables->getByIndex(0), "Width"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109306, "tdf109306.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    // Both types of relative width specification (pct): simple integers (in fiftieths of percent)
    // and floats with "%" unit specification must be treated correctly
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(0), "IsWidthRelative"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(9), getProperty<sal_Int16>(xTables->getByIndex(0), "RelativeWidth"));

    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xTables->getByIndex(1), "IsWidthRelative"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(80), getProperty<sal_Int16>(xTables->getByIndex(1), "RelativeWidth"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf97648_relativeWidth,"tdf97648_relativeWidth.docx")
{
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(7616), getShape(1)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(8001), getShape(2)->getSize().Width, 10);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(4001), getShape(3)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_LEFT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(6), "ParaAdjust")) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( sal_Int32(1600), getShape(4)->getSize().Width, 10);
    CPPUNIT_ASSERT_EQUAL( style::ParagraphAdjust_RIGHT, static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(8), "ParaAdjust")) );


    CPPUNIT_ASSERT_EQUAL( sal_Int32(0), getProperty<sal_Int32>(getShape(1), "LeftMargin") );
    if (!mbExported)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Text should wrap above/below the line", text::WrapTextMode_NONE, getProperty<text::WrapTextMode>(getShape(1), "Surround"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::CENTER, getProperty<sal_Int16>(getShape(2), "HoriOrient"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::RIGHT, getProperty<sal_Int16>(getShape(3), "HoriOrient"));
        CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::LEFT, getProperty<sal_Int16>(getShape(4), "HoriOrient"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf104061_tableSectionColumns,"tdf104061_tableSectionColumns.docx")
{
    CPPUNIT_ASSERT_MESSAGE("There should be two or three pages", getPages() <= 3 );

    //tdf#95114 - follow style is Text Body - DOCX test
    uno::Reference< beans::XPropertySet > properties(getStyles("ParagraphStyles")->getByName("annotation subject"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("annotation text"), getProperty<OUString>(properties, "FollowStyle"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf98700_keepWithNext, "tdf98700_keepWithNext.odt")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading style keeps with next", true, getProperty<bool>(getParagraph(1), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Default style doesn't keep with next", false, getProperty<bool>(getParagraph(2), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 1 style inherits keeps with next", true, getProperty<bool>(getParagraph(3), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Heading 2 style disabled keep with next", false, getProperty<bool>(getParagraph(4), "ParaKeepTogether"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text Body style toggled off keep with next", false, getProperty<bool>(getParagraph(5), "ParaKeepTogether"));

    //tdf#95114 - follow style is Text Body - ODT test
    uno::Reference< beans::XPropertySet > properties(getStyles("ParagraphStyles")->getByName("Heading 1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Text body"), getProperty<OUString>(properties, "FollowStyle"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf112352_nextPageColumns, "tdf112352_nextPageColumns.docx")
{
    uno::Reference<beans::XPropertySet> xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(2), "TextSection");
    uno::Reference<text::XTextColumns> xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(2), xTextColumns->getColumnCount());

    xTextSection = getProperty< uno::Reference<beans::XPropertySet> >(getParagraph(3), "TextSection");
    xTextColumns = getProperty< uno::Reference<text::XTextColumns> >(xTextSection, "TextColumns");
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), xTextColumns->getColumnCount());
}

DECLARE_OOXMLEXPORT_TEST(testTdf109310_endnoteStyleForMSO, "tdf109310_endnoteStyleForMSO.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/endnotes.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:endnotes/w:endnote[@w:id='2']/w:p/w:r[1]/w:rPr/w:rStyle", "w:val");
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

DECLARE_OOXMLEXPORT_TEST(testTdf106132, "tdf106132.docx")
{
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    // This was 250, <wps:bodyPr ... rIns="0" ...> was ignored for an outer shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(xShape, "TextRightDistance"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf37153, "tdf37153_considerWrapOnObjPos.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::WrapTextMode_THROUGH, getProperty<text::WrapTextMode>(getShape(1), "Surround"));

    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::VertOrientation::BOTTOM, getProperty<sal_Int16>(xTable->getCellByName("A1"), "VertOrient"));

    //For MSO compatibility, the textbox should be at the top of the cell, not at the bottom - despite VertOrientation::BOTTOM
    xmlDocPtr pXmlDoc = parseLayoutDump();
    sal_Int32 nFlyTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[1]/txt/anchored/fly/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("FlyTop should be 2865, not 5649", nFlyTop < sal_Int32(3000));
    sal_Int32 nTextTop  = getXPath(pXmlDoc, "/root/page/body/tab/row/cell[2]/txt[1]/infos/bounds", "top").toInt32();
    CPPUNIT_ASSERT_MESSAGE("TextTop should be 3856", nTextTop > 3000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112446_frameStyle, "tdf112446_frameStyle.docx")
{
    CPPUNIT_ASSERT_EQUAL(text::HoriOrientation::NONE, getProperty<sal_Int16>(getShape(1), "HoriOrient"));
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

    //tdf#118361 - in RTL locales, the footnote separator should still be left aligned.
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Footnote separator LTR", sal_Int16(0), getProperty<sal_Int16>(aPageStyle, "FootnoteLineAdjust"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf55427_footnote2endnote, "tdf55427_footnote2endnote.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("ParagraphStyles")->getByName("Footnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote style is rose color", sal_Int32(0xFF007F), getProperty< sal_Int32 >(xPageStyle, "CharColor") );
    xPageStyle.set(getStyles("ParagraphStyles")->getByName("Endnote"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote style is cyan3 color", sal_Int32(0x2BD0D2), getProperty< sal_Int32 >(xPageStyle, "CharColor") );

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    // The footnote numbering type of ARABIC will not transfer over when those footnotes are converted to endnotes.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Footnote numbering type", SVX_NUM_ARABIC, pDoc->GetFootnoteInfo().aFormat.GetNumberingType() );
    // The original document has a real endnote using ROMAN_LOWER numbering, so that setting MUST remain unchanged.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Endnote numbering type", SVX_NUM_ROMAN_LOWER, pDoc->GetEndNoteInfo().aFormat.GetNumberingType() );

    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);

    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xEndnotes(xEndnotesSupplier->getEndnotes(), uno::UNO_QUERY);
    uno::Reference<text::XFootnote> xEndnote;
    xEndnotes->getByIndex(0) >>= xEndnote;
    uno::Reference<text::XText> xEndnoteText;
    xEndnotes->getByIndex(0) >>= xEndnoteText;

    // ODT footnote-at-document-end's closest DOCX match is an endnote, so the two imports will not exactly match by design.
    if (!mbExported)
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote count", sal_Int32(5), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote count", sal_Int32(1), xEndnotes->getCount() );

        uno::Reference<text::XFootnote> xFootnote;
        xFootnotes->getByIndex(0) >>= xFootnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote's number", OUString("1"), xFootnote->getAnchor()->getString() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's number", OUString("i"), xEndnote->getAnchor()->getString() );

        uno::Reference<text::XText> xFootnoteText;
        xFootnotes->getByIndex(0) >>= xFootnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original footnote style", OUString("Footnote"), getProperty<OUString>(getParagraphOfText(1, xFootnoteText), "ParaStyleName") );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", OUString("Endnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
    }
    else
    {
        // These asserted items are major differences in the conversion from footnote to endnote, NOT necessary conditions for a proper functioning document.
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes were converted into endnotes", sal_Int32(0), xFootnotes->getCount() );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "At-Document-End footnotes became endnotes", sal_Int32(6), xEndnotes->getCount() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "converted footnote's number", OUString("i"), xEndnote->getAnchor()->getString() );
        xEndnotes->getByIndex(4) >>= xEndnote;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote's new number", OUString("v"), xEndnote->getAnchor()->getString() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE( "retained footnote style", OUString("Footnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
        xEndnotes->getByIndex(4) >>= xEndnoteText;
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "original endnote style", OUString("Endnote"), getProperty<OUString>(getParagraphOfText(1, xEndnoteText), "ParaStyleName") );
    }
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

    // The behavior changed - Word 2013 and 2016 ignore this setting on
    // import, and instead honor the user's setting.
    // Let's ignore the <w:view w:val="web"/> too.
    CPPUNIT_ASSERT(!getProperty<bool>(xViewSettings, "ShowOnlineLayout"));
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
    CPPUNIT_ASSERT_EQUAL( OUString("\tfootnote"), xTextRange->getString() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf106062_nonHangingFootnote, "tdf106062_nonHangingFootnote.odt")
{
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xFootnotes(xFootnotesSupplier->getFootnotes(), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange(xFootnotes->getByIndex(0), uno::UNO_QUERY);
    // This failed, tab between the footnote number and the footnote content was lost on import.
    CPPUNIT_ASSERT_MESSAGE( "Footnote starts with a tab", xTextRange->getString().startsWith("\t") );
}

DECLARE_OOXMLEXPORT_TEST( testActiveXCheckbox, "activex_checkbox.docx" )
{
    uno::Reference<drawing::XControlShape> xControlShape( getShape(1), uno::UNO_QUERY );
    CPPUNIT_ASSERT( xControlShape.is() );

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet( xControlShape->getControl(), uno::UNO_QUERY );
    uno::Reference<lang::XServiceInfo> xServiceInfo( xPropertySet, uno::UNO_QUERY );
    CPPUNIT_ASSERT_EQUAL( true, bool( xServiceInfo->supportsService( "com.sun.star.form.component.CheckBox" ) ) );

    // Check custom label
    CPPUNIT_ASSERT_EQUAL( OUString( "Custom Caption" ), getProperty<OUString>(xPropertySet, "Label") );

    // Check background color (highlight system color)
    CPPUNIT_ASSERT_EQUAL( sal_Int32( 0x316AC5 ), getProperty<sal_Int32>(xPropertySet, "BackgroundColor") );

    // Check Text color (active border system color)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD4D0C8), getProperty<sal_Int32>(xPropertySet, "TextColor"));

    // Check state of the checkbox
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPropertySet, "State"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXControlAlign, "activex_control_align.odt")
{
    // First check box aligned as a floating object
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( "com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Check Box"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));

    // Also check position and size
    uno::Reference<drawing::XShape> xShape(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4470), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1427), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5126), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2341), xShape->getPosition().Y);

    // Second check box aligned inline / as character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Check Box"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(text::VertOrientation::TOP),getProperty<sal_Int32>(xPropertySet2,"VertOrient"));

    // Also check position and size
    xShape.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4410), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1083), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1085), xShape->getPosition().Y);

    // Also check the specific OOXML elements
    xmlDocPtr pXmlDoc = parseExport();
    CPPUNIT_ASSERT(pXmlDoc);
    // For inline controls use w:object as parent element and pictureFrame shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shapetype", "spt", "75");
    // For floating controls use w:pict as parent element and hostControl shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shapetype", "spt", "201");

     // Have different shape ids
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shape", "id") !=
        getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shape", "id"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf109184, "tdf109184.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Before table background color was white, should be transparent (auto).
    uno::Reference<text::XTextRange> xCell1(xTable->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1), getProperty<sal_Int32>(xCell1, "BackColor"));

    // Cell with auto color but with 15% fill, shouldn't be transparent.
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName("B1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xd8d8d8), getProperty<sal_Int32>(xCell2, "BackColor"));

    // Cell with color defined (red).
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0xff0000), getProperty<sal_Int32>(xCell3, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf111964, "tdf111964.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Unicode spaces that are not XML whitespace must not be trimmed
    const sal_Unicode sWSReference [] { 0x2002, 0x2002, 0x2002, 0x2002, 0x2002, 0 };
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", sWSReference);
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark-shapetype.docx")
{
    uno::Reference<drawing::XShape> xShape1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape1.is());
    uno::Reference<beans::XPropertySet> xPropertySet1(xShape1, uno::UNO_QUERY);

    uno::Reference<drawing::XShape> xShape2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape2.is());
    uno::Reference<beans::XPropertySet> xPropertySet2(xShape2, uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(xPropertySet1->getPropertyValue("TextAutoGrowHeight"), xPropertySet2->getPropertyValue("TextAutoGrowHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXControlAtRunEnd, "activex_control_at_run_end.odt")
{
    // Two issues were here:
    //  1) second shape was not export (it is anchored to the end of the run)
    //  2) inline property was inherited to the second shape by mistake

    // First checkbox is the inlined one
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService( "com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));

    // Second check box anchored to character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,getProperty<text::TextContentAnchorType>(xPropertySet2,"AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXOptionButtonGroup, "activex_option_button_group.docx")
{
    // Optionbutton groups were not handled
    // The two optionbutton should have the same group name
    const OUString sGroupName = "GroupX";

    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, "GroupName"));

    xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, "GroupName"));
}

DECLARE_OOXMLEXPORT_TEST(tdf112169, "tdf112169.odt")
{
    // LO crashed while export because of character background color handling
}

DECLARE_OOXMLEXPORT_TEST(testTdf103090, "tdf103090.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

    // Get bookmark name
    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:bookmarkStart", "name");

    // Ensure that name has no spaces
    CPPUNIT_ASSERT(bookmarkName.indexOf(" ") < 0);

    // Get PAGEREF field
    OUString fieldName = getXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:instrText");

    // Ensure that PAGEREF field refers exactly our bookmark
    OUString expectedFieldName(" PAGEREF ");
    expectedFieldName += bookmarkName;
    expectedFieldName += " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName, fieldName);
}

DECLARE_OOXMLEXPORT_TEST(testTdf107111, "tdf107111.docx")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;

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
    uno::Reference<view::XSelectionSupplier> xCtrl(xModel->getCurrentController(), uno::UNO_QUERY_THROW);
    xCtrl->select(uno::makeAny(xShape->getAnchor()));

    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xCtrl, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextViewCursor> xTextCursor(xTextViewCursorSupplier->getViewCursor(), uno::UNO_QUERY_THROW);
    uno::Reference<text::XPageCursor> xPageCursor(xTextCursor.get(), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xPageCursor->getPage());
}

DECLARE_OOXMLEXPORT_TEST(testTdf90789_2, "tdf90789-2.docx")
{
    // Section break before frame and shape was ignored
    CPPUNIT_ASSERT_EQUAL( 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354_2, "tdf104354-2.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    // top margin of the first paragraph and bottom margin of the last paragraph
    // is zero, when auto spacing is used.

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), "ParaBottomMargin"));

    // top margin is not auto spacing
    uno::Reference<text::XTextRange> xCell2(xTable->getCellByName("A2"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(847), getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell2->getText()), "ParaBottomMargin"));

    // bottom margin is not auto spacing
    uno::Reference<text::XTextRange> xCell3(xTable->getCellByName("A3"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(847), getProperty<sal_Int32>(getParagraphOfText(1, xCell3->getText()), "ParaBottomMargin"));

    // auto spacing, if the paragraph contains footnotes
    uno::Reference<text::XTextRange> xCell4(xTable->getCellByName("A4"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell4->getText()), "ParaBottomMargin"));

    // auto spacing on a paragraph
    uno::Reference<text::XTextTable> xTable2(xTables->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell5(xTable2->getCellByName("A1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraphOfText(1, xCell5->getText()), "ParaBottomMargin"));
}


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
