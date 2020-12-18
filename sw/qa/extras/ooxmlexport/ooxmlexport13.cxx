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
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>
#include <editeng/frmdiritem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <tools/lineend.hxx>
#include <xmloff/odffields.hxx>
#include <oox/drawingml/drawingmltypes.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/extras/ooxmlexport/data/";

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(DATA_DIRECTORY, "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx") || OString(filename) == "ooo39250-1-min.rtf";
    }
};

// TODO: the re-import doesn't work just yet, but that isn't a regression...
DECLARE_SW_EXPORT_TEST(testFlyInFly, "ooo39250-1-min.rtf", nullptr, Test)
{
    // check that anchor of text frame is in other text frame
    uno::Reference<text::XTextContent> const xAnchored(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchored.is());
    CPPUNIT_ASSERT_EQUAL(OUString(""), uno::Reference<container::XNamed>(xAnchored, uno::UNO_QUERY_THROW)->getName());
    uno::Reference<text::XText> const xAnchorText(xAnchored->getAnchor()->getText());
    uno::Reference<text::XTextFrame> const xAnchorFrame(xAnchorText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchorFrame.is());
    CPPUNIT_ASSERT_EQUAL(OUString("Frame2"), uno::Reference<container::XNamed>(xAnchorFrame, uno::UNO_QUERY_THROW)->getName());
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF, "tdf121374_sectionHF.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    CPPUNIT_ASSERT_MESSAGE("Number of pages", getPages() > 2);
    //CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf126723, "tdf126723.docx")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf121867, "tdf121867.odt")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 3; Actual  : 0', i.e. page width zoom was lost on export.
    CPPUNIT_ASSERT_EQUAL(SvxZoomType::PAGEWIDTH, pEditShell->GetViewOptions()->GetZoomType());
}

DECLARE_OOXMLEXPORT_TEST(testTbrlTextbox, "tbrl-textbox.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // Without the accompanying fix in place, this test would have failed with 'Expected: -90;
    // Actual: 0', i.e. tbRl writing direction was imported as lrTb.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-90),
                         aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testBtlrShape, "btlr-textbox.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT), rFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_FLYFRMFMT), rFormats[1]->Which());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 5, Actual:
    // 4', i.e. the textbox inherited its writing direction instead of having an explicit btlr
    // value.
    CPPUNIT_ASSERT_EQUAL(SvxFrameDirection::Vertical_LR_BT,
                         rFormats[1]->GetAttrSet().GetFrameDir().GetValue());
}

DECLARE_OOXMLEXPORT_TEST(testFrameSizeExport, "floating-tables-anchor.docx")
{
    // Make sure the table width is 4000
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW", "w", "4000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf124594, "tdf124594.docx")
{
    xmlDocPtr pDump = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, as the portion text was
    // only "Er horte leise Schritte hinter", which means the 1st line of the 2nd paragraph was
    // split into two by a Special portion, i.e. the top margin of the shape was too large.
    assertXPath(pDump, "/root/page/body/txt[2]/Text[1]", "Portion",
                "Er horte leise Schritte hinter sich. Das bedeutete nichts Gutes. Wer wurde ihm ");
}

DECLARE_OOXMLEXPORT_TEST(testTextInput, "textinput.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    // Ensure we have a formfield
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[3]/w:instrText", " FORMTEXT ");
    // and it's content is not gone
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:r[5]/w:t", "SomeText");
}

DECLARE_OOXMLIMPORT_TEST(testTdf123460, "tdf123460.docx")
{
    // check paragraph mark deletion at terminating moveFrom
    CPPUNIT_ASSERT_EQUAL(true,getParagraph( 2 )->getString().startsWith("Nunc"));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(2), 1), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, getRun( getParagraph( 2 ), 2 )->getString().endsWith("tellus."));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 3 )->getString());
    bool bCaught = false;
    try
    {
        getRun( getParagraph( 2 ), 4 );
    }
    catch (container::NoSuchElementException&)
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT_EQUAL(true, bCaught);
}

DECLARE_OOXMLEXPORT_TEST(testTdf125324, "tdf125324.docx")
{
    discardDumpedLayout();
    xmlDocPtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/tab/infos/bounds", "top", "4193");
}

DECLARE_OOXMLEXPORT_TEST(testTbrlFrameVml, "tbrl-frame-vml.docx")
{
    uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextFrame.is());

    if (mbExported)
    {
        // DML import: creates a TextBox.

        comphelper::SequenceAsHashMap aGeometry(xTextFrame->getPropertyValue("CustomShapeGeometry"));
        // Without the accompanying fix in place, this test would have failed with 'Expected: -90;
        // Actual: 0', i.e. the tblr writing mode was lost during DML export of a TextFrame.
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-90), aGeometry["TextPreRotateAngle"].get<sal_Int32>());
    }
    else
    {
        // VML import: creates a TextFrame.

        auto nActual = getProperty<sal_Int16>(xTextFrame, "WritingMode");
        // Without the accompanying fix in place, this test would have failed with 'Expected: 2; Actual:
        // 4', i.e. writing direction was inherited from page, instead of explicit tbrl.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
    }
}

DECLARE_OOXMLEXPORT_TEST(testBtlrFrame, "btlr-frame.odt")
{
    if (!mbExported)
    {
        return;
    }

    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // -270; Actual: 0', i.e. the writing direction of the frame was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-270),
                         aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

DECLARE_OOXMLEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, "FrameRect");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24063), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24179), frameRect.Width);
}

DECLARE_OOXMLEXPORT_TEST(testTdf138953, "croppedAndRotated.odt")
{
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_EQUAL(27000.0, getProperty<double>(xShape, "RotateAngle"));
    auto frameRect = getProperty<css::awt::Rectangle>(xShape, "FrameRect");
    // Before the fix, original object size (i.e., before cropping) was written to spPr in OOXML,
    // and the resulting object size was much larger than should be.
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12961), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8664), frameRect.Width);
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

DECLARE_OOXMLEXPORT_TEST(testTdf129353, "tdf129353.docx")
{
    CPPUNIT_ASSERT_EQUAL(8, getParagraphs());
    getParagraph(2, "Bibliography");
    getParagraph(4, "Christie, A. (1922). The Secret Adversary. ");
    CPPUNIT_ASSERT_EQUAL(OUString(), getParagraph(8)->getString());

    uno::Reference<text::XDocumentIndexesSupplier> xIndexSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexes = xIndexSupplier->getDocumentIndexes();
    uno::Reference<text::XDocumentIndex> xIndex(xIndexes->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xTextRange = xIndex->getAnchor();
    uno::Reference<text::XText> xText = xTextRange->getText();
    uno::Reference<text::XTextCursor> xTextCursor = xText->createTextCursor();
    xTextCursor->gotoRange(xTextRange->getStart(), false);
    xTextCursor->gotoRange(xTextRange->getEnd(), true);
    OUString aIndexString(convertLineEnd(xTextCursor->getString(), LineEnd::LINEEND_LF));

    // Check that all the pre-rendered entries are correct, including trailing spaces
    CPPUNIT_ASSERT_EQUAL(OUString("\n" // starting with an empty paragraph
                                  "Christie, A. (1922). The Secret Adversary. \n"
                                  "\n"
                                  "Verne, J. G. (1870). Twenty Thousand Leagues Under the Sea. \n"
                                  ""), // ending with an empty paragraph
                         aIndexString);
}

DECLARE_OOXMLIMPORT_TEST(testTdf128207, "tdf128207.docx")
{
    //There was the charts on each other, because their horizontal and vertical position was 0!
    CPPUNIT_ASSERT_EQUAL(sal_Int32(12), getProperty<sal_Int32>(getShape(1), "HoriOrientPosition"));
}

DECLARE_OOXMLEXPORT_TEST(testContSectBreakHeaderFooter, "cont-sect-break-header-footer.docx")
{
    // Load a document with a continuous section break on page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 1"),
                         parseDump("/root/page[1]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 1"),
                         parseDump("/root/page[1]/footer/txt/text()"));
    // Make sure the header stays like this; if we naively just update the page style name of the
    // first para on page 2, then this would be 'Header, section 2', which is incorrect.
    CPPUNIT_ASSERT_EQUAL(OUString("First page header, section 2"),
                         parseDump("/root/page[2]/header/txt/text()"));
    CPPUNIT_ASSERT_EQUAL(OUString("First page footer, section 2"),
                         parseDump("/root/page[2]/footer/txt/text()"));
    // This is inherited from page 2.
    CPPUNIT_ASSERT_EQUAL(OUString("Header, section 2"),
                         parseDump("/root/page[3]/header/txt/text()"));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - xpath should match exactly 1 node
    // i.e. the footer had no text (inherited from page 2), while the correct behavior is to provide
    // the own footer text.
    CPPUNIT_ASSERT_EQUAL(OUString("Footer, section 3"),
                         parseDump("/root/page[3]/footer/txt/text()"));

    // Without the export fix in place, the import-export-import test would have failed with:
    // - Expected: Header, section 2
    // - Actual  : First page header, section 2
    // i.e. both the header and the footer on page 3 was wrong.

    // Additional problem: top margin on page 3 was wrong.
    if (mbExported)
    {
        xmlDocPtr pXml = parseExport("word/document.xml");
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 2200
        // - Actual  : 2574
        // i.e. the top margin on page 3 was too large and now matches the value from the input
        // document.
        assertXPath(pXml, "/w:document/w:body/w:sectPr/w:pgMar", "top", "2200");
    }
}

DECLARE_OOXMLEXPORT_TEST(testArabicZeroNumbering, "arabic-zero-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 64
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testZeroLineSpacing)
{
    // Create the doc model.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    style::LineSpacing aSpacing;
    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
    aSpacing.Height = 0;
    xParagraph->setPropertyValue("ParaLineSpacing", uno::makeAny(aSpacing));

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: atLeast
    // - Actual  : auto
    // i.e. the minimal linespacing was lost on export.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "lineRule", "atLeast");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:pPr/w:spacing", "line", "0");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testUserField)
{
    // Create an in-memory empty document with a user field.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XDependentTextField> xField(
        xFactory->createInstance("com.sun.star.text.TextField.User"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xMaster(
        xFactory->createInstance("com.sun.star.text.FieldMaster.User"), uno::UNO_QUERY);
    xMaster->setPropertyValue("Name", uno::makeAny(OUString("foo")));
    xField->attachTextFieldMaster(xMaster);
    xField->getTextFieldMaster()->setPropertyValue("Content", uno::makeAny(OUString("bar")));
    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    xText->insertTextContent(xText->createTextCursor(), xField, /*bAbsorb=*/false);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // Without the accompanying fix in place, this test would have failed, the user field was
    // exported as <w:t>User Field foo = bar</w:t>.
    assertXPathContent(pXmlDoc, "//w:p/w:r[2]/w:instrText", " DOCVARIABLE foo ");
    assertXPathContent(pXmlDoc, "//w:p/w:r[4]/w:t", "bar");

    // Make sure that not only the variables, but also their values are written.
    pXmlDoc = parseExport("word/settings.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "name", "foo");
    assertXPath(pXmlDoc, "//w:docVars/w:docVar", "val", "bar");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testTableStyleConfNested)
{
    // Create the doc model.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "table-style-conf-nested.docx";
    loadURL(aURL, nullptr);

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    validate(maTempFile.GetFileName(), test::OOXML);
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    // Without the accompanying fix in place, this test would have failed, as the custom table cell
    // border properties were lost, so the outer A2 cell started to have borders, not present in the
    // doc model.
    assertXPath(pXmlDoc, "//w:body/w:tbl/w:tr/w:tc[2]/w:tcPr/w:tcBorders/w:top", "val", "nil");
}

CPPUNIT_TEST_FIXTURE(SwModelTestBase, testSemiTransparentText)
{
    // Create an in-memory empty document.
    loadURL("private:factory/swriter", nullptr);

    // Set text to half-transparent and type a character.
    uno::Reference<beans::XPropertySet> xParagraph(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xParagraph.is());
    sal_Int16 nTransparence = 75;
    xParagraph->setPropertyValue("CharTransparence", uno::makeAny(nTransparence));
    uno::Reference<text::XTextRange> xTextRange(xParagraph, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextRange.is());
    xTextRange->setString("x");

    // Export to docx.
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("Office Open XML Text");
    xStorable->storeToURL(maTempFile.GetURL(), aMediaDescriptor.getAsConstPropertyValueList());
    mbExported = true;
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    OString aXPath
        = "/w:document/w:body/w:p/w:r/w:rPr/w14:textFill/w14:solidFill/w14:srgbClr/w14:alpha";
    double fValue = getXPath(pXmlDoc, aXPath, "val").toDouble();
    sal_Int16 nActual = basegfx::fround(fValue / oox::drawingml::PER_PERCENT);

    // Without the accompanying fix in place, this test would have failed, as the w14:textFill
    // element was missing.
    CPPUNIT_ASSERT_EQUAL(nTransparence, nActual);
}

CPPUNIT_TEST_FIXTURE(Test, testArabicZeroNumberingFootnote)
{
    // Create a document, set footnote numbering type to ARABIC_ZERO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::ARABIC_ZERO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for ARABIC_ZERO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "decimalZero");
}

CPPUNIT_TEST_FIXTURE(Test, testChicagoNumberingFootnote)
{
    // Create a document, set footnote numbering type to SYMBOL_CHICAGO.
    loadURL("private:factory/swriter", nullptr);
    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFootnoteSettings
        = xFootnotesSupplier->getFootnoteSettings();
    sal_uInt16 nNumberingType = style::NumberingType::SYMBOL_CHICAGO;
    xFootnoteSettings->setPropertyValue("NumberingType", uno::makeAny(nNumberingType));

    // Insert a footnote.
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContent> xFootnote(
        xFactory->createInstance("com.sun.star.text.Footnote"), uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextContentAppend> xTextContentAppend(xTextDocument->getText(),
                                                                uno::UNO_QUERY);
    xTextContentAppend->appendTextContent(xFootnote, {});

    reload("Office Open XML Text", "");

    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    // Without the accompanying fix in place, this test would have failed with:
    // XPath '/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt' number of nodes is incorrect
    // because the exporter had no idea what markup to use for SYMBOL_CHICAGO.
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:footnotePr/w:numFmt", "val", "chicago");
}

DECLARE_ODFEXPORT_TEST(testArabicZero3Numbering, "arabic-zero3-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO3.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO3),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero4Numbering, "arabic-zero4-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 66
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO4.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO4),
                         aMap["NumberingType"].get<sal_uInt16>());
}

DECLARE_ODFEXPORT_TEST(testArabicZero5Numbering, "arabic-zero5-numbering.docx")
{
    auto xNumberingRules
        = getProperty<uno::Reference<container::XIndexAccess>>(getParagraph(1), "NumberingRules");
    comphelper::SequenceAsHashMap aMap(xNumberingRules->getByIndex(0));
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 67
    // - Actual  : 4
    // i.e. numbering type was ARABIC, not ARABIC_ZERO5.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(style::NumberingType::ARABIC_ZERO5),
                         aMap["NumberingType"].get<sal_uInt16>());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
