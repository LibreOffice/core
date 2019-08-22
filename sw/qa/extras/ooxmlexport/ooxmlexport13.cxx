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
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>
#include <editeng/frmdiritem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <xmloff/odffields.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>

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

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF, "tdf121374_sectionHF.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    //CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
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
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24070), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24188), frameRect.Width);
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
