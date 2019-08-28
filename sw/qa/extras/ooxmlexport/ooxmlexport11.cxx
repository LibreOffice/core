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
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>


#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>
#include <xmloff/odffields.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>
#include <com/sun/star/drawing/XControlShape.hpp>

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

DECLARE_OOXMLEXPORT_TEST(testTdf57589_hashColor, "tdf57589_hashColor.docx")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_SOLID, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTMAGENTA, getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor"));
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getParagraph(2), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(getParagraph(2), "ParaBackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf92524_autoColor, "tdf92524_autoColor.doc")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116436_rowFill, "tdf116436_rowFill.odt")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<table::XCell> xCell = xTable->getCellByName("A1");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xF8DF7C), getProperty<sal_Int32>(xCell, "BackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf46938_clearTabStop, "tdf46938_clearTabStop.docx")
{
    // Number of tabstops should be zero, overriding the one in the style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf82065_Ind_start_strict, "tdf82065_Ind_start_strict.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    bool bFoundIndentAt = false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "IndentAt")
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("IndentAt", double(6001), rProp.Value.get<double>(), 10 );
            bFoundIndentAt = true;
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("IndentAt defined", true, bFoundIndentAt);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112694, "tdf112694.docx")
{
    uno::Any aPageStyle = getStyles("PageStyles")->getByName("Standard");
    // Header was on when header for file was for explicit first pages only
    // (marked via <w:titlePg>).
    CPPUNIT_ASSERT(!getProperty<bool>(aPageStyle, "HeaderIsOn"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf115861, "tdf115861.docx")
{
    // Second item in the paragraph enumeration was a table, 2nd paragraph was
    // lost.
    CPPUNIT_ASSERT_EQUAL(OUString("(k)"), getParagraph(2)->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(u8"«Name»"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

DECLARE_OOXMLEXPORT_TEST(testParagraphSplitOnSectionBorder, "parasplit-on-section-border.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if(!pXmlDoc)
        return;

    // Test document has only two paragraphs. After splitting, it should contain
    // three of them.
    assertXPath(pXmlDoc, "//w:sectPr", 2);
    assertXPath(pXmlDoc, "//w:p", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf115719, "tdf115719.docx")
{
    // This was a single page, instead of pushing the textboxes to the second
    // page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf116410, "tdf116410.docx")
{
    // Opposite of the above, was 2 pages, should be 1 page.
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf117988, "tdf117988.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113183, "tdf113183.docx")
{
    // This was 2096, the horizontal positioning of the star shape affected the
    // positioning of the triangle one, so the triangle was outside the page
    // frame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(getShapeByName("triangle"),
                                                "HoriOrientPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testGraphicObjectFliph, "graphic-object-fliph.docx")
{
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnEvenPages"));
    CPPUNIT_ASSERT(getProperty<bool>(getShape(1), "HoriMirroredOnOddPages"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf113547, "tdf113547.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aProps(xLevels->getByIndex(0)); // 1st level
    // This was 0, first-line left margin of the numbering was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-635), aProps["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113399, "tdf113399.doc")
{
    // 0 padding was not preserved
    // In LO 0 is the defualt, but in OOXML format the default is 254 / 127
    uno::Reference<beans::XPropertySet> xPropSet(getShape(1), uno::UNO_QUERY);
    sal_Int32 nPaddingValue;
    xPropSet->getPropertyValue("TextLeftDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextRightDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextUpperDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
    xPropSet->getPropertyValue("TextLowerDistance") >>= nPaddingValue;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nPaddingValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf114703, "tdf114703.docx")
{
    uno::Reference<container::XIndexAccess> xRules
        = getProperty<uno::Reference<container::XIndexAccess>>(
            getStyles("NumberingStyles")->getByName("WWNum1"), "NumberingRules");
    // This was 0, level override "default" replaced the non-default value from
    // the abstract level.
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(-1000),
        comphelper::SequenceAsHashMap(xRules->getByIndex(0))["FirstLineIndent"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113258, "tdf113258.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a shape.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf44832_testSectionWithDifferentHeader, "tdf44832_section_new_header.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if(!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:headerReference", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf104354, "tdf104354.docx")
{
    uno::Reference<text::XTextRange> xShape(getShape(1), uno::UNO_QUERY);
    // This was 494, i.e. automatic spacing resulted in non-zero paragraph top
    // margin for the first paragraph in a text frame.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0),
                         getProperty<sal_Int32>(xShape->getStart(), "ParaTopMargin"));
    // still 494 in the second paragraph
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(494),
                         getProperty<sal_Int32>(xShape->getEnd(), "ParaTopMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf107035, "tdf107035.docx")
{
    // Select the second run containing the page number field
    auto xPgNumRun = getRun(getParagraph(1), 2, "1");

    // Check that the page number field colour is set to "automatic".
    sal_Int32 nPgNumColour = getProperty<sal_Int32>(xPgNumRun, "CharColor");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(COL_AUTO), nPgNumColour);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116976, "tdf116976.docx")
{
    // This was 0, reltive size of shape after bitmap was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(40),
                         getProperty<sal_Int16>(getShape(1), "RelativeWidth"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116985, "tdf116985.docx")
{
    // Body frame width is 10800, 40% is the requested relative width, with 144
    // spacing to text on the left/right side.  So ideal width would be 4032,
    // was 3431. Allow one pixel tolerance, though.
    sal_Int32 nWidth
        = parseDump("/root/page[1]/body/txt[1]/anchored/fly/infos/bounds", "width").toInt32();
    CPPUNIT_ASSERT(nWidth > 4000);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116801, "tdf116801.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // This raised a lang::IndexOutOfBoundsException, table was missing from
    // the import result.
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("D1"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("D1"), xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf112118_DOCX, "tdf112118.docx")
{
    // The resulting left margin width (2081) differs from its DOC counterpart from ww8export2.cxx,
    // because DOCX import does two conversions between mm/100 and twips on the route, loosing one
    // twip on the road and arriving with a value that is 2 mm/100 less. I don't see an obvious way
    // to avoid that.
    struct {
        const char* styleName;
        struct {
            const char* sideName;
            sal_Int32 nMargin;
            sal_Int32 nBorderDistance;
            sal_Int32 nBorderWidth;
        } sideParams[4];
    } styleParams[] = {                      // Margin (MS-style), border distance, border width
        {
            "Standard",
            {
                { "Top", 496, 847, 159 },    //  851 twip, 24 pt (from text), 4.5 pt
                { "Left", 2081, 706, 212 },  // 1701 twip, 20 pt (from text), 6.0 pt
                { "Bottom", 1401, 564, 35 }, // 1134 twip, 16 pt (from text), 1.0 pt
                { "Right", 3471, 423, 106 }  // 2268 twip, 12 pt (from text), 3.0 pt
            }
        },
        {
            "Converted1",
            {
                { "Top", 847, 496, 159 },    //  851 twip, 24 pt (from edge), 4.5 pt
                { "Left", 706, 2081, 212 },  // 1701 twip, 20 pt (from edge), 6.0 pt
                { "Bottom", 564, 1401, 35 }, // 1134 twip, 16 pt (from edge), 1.0 pt
                { "Right", 423, 3471, 106 }  // 2268 twip, 12 pt (from edge), 3.0 pt
            }
        }
    };
    auto xStyles = getStyles("PageStyles");

    for (const auto& style : styleParams)
    {
        const OUString sName = OUString::createFromAscii(style.styleName);
        uno::Reference<beans::XPropertySet> xStyle(xStyles->getByName(sName), uno::UNO_QUERY_THROW);
        for (const auto& side : style.sideParams)
        {
            const OUString sSide = OUString::createFromAscii(side.sideName);
            const OString sStage = OString(style.styleName) + " " + side.sideName;

            sal_Int32 nMargin = getProperty<sal_Int32>(xStyle, sSide + "Margin");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " margin width").getStr(),
                side.nMargin, nMargin);

            sal_Int32 nBorderDistance = getProperty<sal_Int32>(xStyle, sSide + "BorderDistance");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border distance").getStr(),
                side.nBorderDistance, nBorderDistance);

            table::BorderLine aBorder = getProperty<table::BorderLine>(xStyle, sSide + "Border");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OString(sStage + " border width").getStr(),
                side.nBorderWidth,
                sal_Int32(aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance));
        }
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf82177_outsideCellBorders, "tdf82177_outsideCellBorders.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(), uno::UNO_QUERY);
    uno::Reference< text::XTextTable > xTable( xTables->getByIndex(0), uno::UNO_QUERY );
    uno::Reference< table::XCell > xCell = xTable->getCellByName( "E4" );
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "TopBorder").LineWidth);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), getProperty<table::BorderLine2>(xCell, "LeftBorder").LineWidth);
}

DECLARE_OOXMLEXPORT_TEST(testTdf119143, "tdf119143.docx")
{
    // The runs inside <w:dir> were ignored
    const OUString sParaText = getParagraph(1)->getString();
    CPPUNIT_ASSERT_EQUAL(
        OUString::fromUtf8(
            u8"عندما يريد العالم أن يتكلّم ‬ ، فهو يتحدّث "
            u8"بلغة "
            u8"يونيكود. تسجّل الآن لحضور المؤتمر الدولي العاشر "
            u8"ليونيكود (Unicode Conference)، الذي سيعقد في 10-12 "
            u8"آذار 1997 بمدينة مَايِنْتْس، ألمانيا. و سيجمع المؤتمر "
            u8"بين خبراء من كافة قطاعات الصناعة على الشبكة "
            u8"العالمية انترنيت ويونيكود، حيث ستتم، على الصعيدين "
            u8"الدولي والمحلي على حد سواء مناقشة سبل استخدام "
            u8"يونكود في النظم القائمة وفيما يخص التطبيقات "
            u8"الحاسوبية، الخطوط، تصميم النصوص والحوسبة متعددة "
            u8"اللغات."),
        sParaText);
}

DECLARE_OOXMLEXPORT_TEST(testOpenDocumentAsReadOnly, "open-as-read-only.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    CPPUNIT_ASSERT(pTextDoc->GetDocShell()->IsSecurityOptOpenReadOnly());
}

DECLARE_OOXMLEXPORT_TEST(testTdf58944RepeatingTableHeader, "tdf58944-repeating-table-header.docx")
{
    // DOCX tables with more than 10 repeating header lines imported without repeating header lines
    // as a workaround for MSO's limitation of header line repetition
    xmlDocPtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // instead of showing only a part of it on page 2
    assertXPath(pDump, "/root/page[1]/body/tab", 1);
    assertXPath(pDump, "/root/page[1]/body/tab/row", 11);
    CPPUNIT_ASSERT_EQUAL(OUString("Test1"),
                         parseDump("/root/page[2]/body/tab/row[1]/cell[1]/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("Test2"),
                         parseDump("/root/page[2]/body/tab/row[2]/cell[1]/txt/text()"));
}

DECLARE_OOXMLEXPORT_TEST(testInputListExport, "tdf122186_input_list.odt")
{
    // We need to make sure we don't export the text itself next to the input list field
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[4]/w:t", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, "FrameRect");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24070), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24188), frameRect.Width);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak, "tdf123636_newlinePageBreak.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: split first empty paragraph in a section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 2, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak2, "tdf123636_newlinePageBreak2.docx")
{
    //WITHOUT SplitPgBreakAndParaMark: a following anchored shape should force a page break
    //CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 2, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2, ""), "NumberingStyleName"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak4, "tdf123636_newlinePageBreak4.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: an empty paragraph doesn't split (except if first paragraph).
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 3, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocPtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[2]/body/txt[1]/Text", 0);
}

DECLARE_OOXMLEXPORT_TEST(testImageCommentAtChar, "image-comment-at-char.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 1), "TextPortionType"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment start before the image was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("Annotation"),
                         getProperty<OUString>(getRun(xPara, 2), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Frame"),
                         getProperty<OUString>(getRun(xPara, 3), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("AnnotationEnd"),
                         getProperty<OUString>(getRun(xPara, 4), "TextPortionType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Text"),
                         getProperty<OUString>(getRun(xPara, 5), "TextPortionType"));
}


DECLARE_OOXMLIMPORT_TEST(testInvalidDateFormField, "invalid_date_form_field.docx")
{

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMarkAccess->getAllMarksCount());

    int nIndex = 0;
    for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
    {
        ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(aIter->get());

        if(!pFieldmark)
            continue;

        CPPUNIT_ASSERT(pFieldmark);
        CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

        // Check date field's parameters.
        const sw::mark::IFieldmark::parameter_map_t* const pParameters = pFieldmark->GetParameters();
        OUString sDateFormat;
        auto pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sDateFormat;
        }

        OUString sLang;
        pResult = pParameters->find(ODF_FORMDATE_DATEFORMAT_LANGUAGE);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sLang;
        }

        OUString sCurrentDate;
        pResult = pParameters->find(ODF_FORMDATE_CURRENTDATE);
        if (pResult != pParameters->end())
        {
            pResult->second >>= sCurrentDate;
        }

        // The first one has invalid date format (invalid = LO can't parse it)
        if(nIndex == 0)
        {

            CPPUNIT_ASSERT_EQUAL(OUString("YYYY.MM.DDT00:00:00Z"), sDateFormat);
            CPPUNIT_ASSERT_EQUAL(OUString("en-US"), sLang);
            CPPUNIT_ASSERT_EQUAL(OUString(""), sCurrentDate);

            CPPUNIT_ASSERT_EQUAL(sal_uLong(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFieldmark->GetMarkStart().nContent.GetIndex());
        }
        else if (nIndex == 1) // The second has wrong date
        {
            CPPUNIT_ASSERT_EQUAL(OUString("MM/DD/YY"), sDateFormat);
            CPPUNIT_ASSERT_EQUAL(OUString("en-US"), sLang);
            CPPUNIT_ASSERT_EQUAL(OUString("2019.06.34"), sCurrentDate);

            CPPUNIT_ASSERT_EQUAL(sal_uLong(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(14), pFieldmark->GetMarkStart().nContent.GetIndex());
        }
        else // The third one has wrong local
        {
            CPPUNIT_ASSERT_EQUAL(OUString("[NatNum12 MMMM=abbreviation]YYYY\". \"MMMM D."), sDateFormat);
            CPPUNIT_ASSERT_EQUAL(OUString("xxxx"), sLang);
            CPPUNIT_ASSERT_EQUAL(OUString("2019.06.11"), sCurrentDate);

            CPPUNIT_ASSERT_EQUAL(sal_uLong(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(33), pFieldmark->GetMarkStart().nContent.GetIndex());
        }
        ++nIndex;
    }
    CPPUNIT_ASSERT_EQUAL(int(3), nIndex);
}

DECLARE_OOXMLEXPORT_TEST(tdf127085, "tdf127085.docx")
{
    // Fill transparency was lost during export
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, "FillTransparence"));
}

DECLARE_OOXMLEXPORT_TEST(tdf119809, "tdf119809.docx")
{
    // Combobox without an item list lost during import
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
}

DECLARE_OOXMLEXPORT_TEST(tdf118169, "tdf118169.docx")
{
    // Unicode characters were converted to question marks.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString(u"őőőőőőőőőőőűűűű"), getProperty<OUString>(xPropertySet, "Label"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
