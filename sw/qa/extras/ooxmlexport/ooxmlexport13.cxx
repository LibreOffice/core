/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XTextSectionsSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <editeng/escapementitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <xmloff/odffields.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx")
            || filename == std::string_view("ooo39250-1-min.rtf");
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

DECLARE_OOXMLEXPORT_TEST(testTdf125778_lostPageBreakTOX, "tdf125778_lostPageBreakTOX.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf126994_lostPageBreak, "tdf126994_lostPageBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF, "tdf121374_sectionHF.odt")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "FooterText");
    CPPUNIT_ASSERT_EQUAL( OUString("footer"), xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf121374_sectionHF2, "tdf121374_sectionHF2.doc")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121666_lostPage)
{
    loadAndSave("tdf121666_lostPage.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br", "type", "page");
    // The second page break is exported too.
    // Before this fix, if a node had both section break and page break, then only the section break was exported.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:br", "type", "page");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:sectPr/w:type", "val", "nextPage");
}

DECLARE_OOXMLEXPORT_TEST(testTdf140182_extraPagebreak, "tdf140182_extraPagebreak.docx")
{
    // Table, page break, section break should be only 2 pages
    // 2 breaks would normally results in 3 pages, but page break + section break is a special case
    // that is handled so to break only 1 page that result only 2 pages.
    // Because of the table, a hack (m_bDummyParaAddedForTableInSection) is set for the entire section,
    // that canceled the page break + section break special case handling, resulting 3 pages.
    // The accompanying fix eliminates this cancellation.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121659_loseColumnBrNextToShape)
{
    loadAndSave("tdf121659_loseColumnBrNextToShape.docx");
    // The third paragraph contains a manual column break and a shape.
    // The column break was moved into the shape during the first import
    // (messing also the shape position), and eliminated during the second import,
    // losing the 2-column text layout. As a workaround, split the paragraph
    // moving the column break into the fourth paragraph instead of losing it.
    reload("Office Open XML Text", "tdf121659_loseColumnBrNextToShape.docx");
    bool bBreakOnPara3 = getProperty<style::BreakType>(getParagraph(3), "BreakType") == style::BreakType_COLUMN_BEFORE;
    bool bBreakOnPara4 = getProperty<style::BreakType>(getParagraph(4), "BreakType") == style::BreakType_COLUMN_BEFORE;
    CPPUNIT_ASSERT(bBreakOnPara3 || bBreakOnPara4);
}

DECLARE_OOXMLEXPORT_TEST(testTdf95848, "tdf95848.docx")
{
    OUString listId;
    OUString listStyle;
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("ListId") >>= listId);
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, "NumberingStyleName"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        // different numbering style
        OUString listStyle3;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle3);
        CPPUNIT_ASSERT(listStyle3.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle3 != listStyle);
        // but same list
        CPPUNIT_ASSERT_EQUAL(OUString("1.1.3"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf95848_2, "tdf95848_2.docx")
{
    OUString listId;
    OUString listStyle;
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("ListId") >>= listId);
        CPPUNIT_ASSERT_EQUAL(OUString("1)"), getProperty<OUString>(xPara, "ListLabelString"));
        // check indent of list style
        auto xLevels = getProperty<uno::Reference<container::XIndexAccess>>(xPara, "NumberingRules");
        uno::Sequence<beans::PropertyValue> aLevel;
        xLevels->getByIndex(0) >>= aLevel; // top level
        sal_Int32 nIndent = std::find_if(std::cbegin(aLevel), std::cend(aLevel), [](const beans::PropertyValue& rValue) { return rValue.Name == "FirstLineIndent"; })->Value.get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), nIndent);
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // but same list
        CPPUNIT_ASSERT_EQUAL(OUString("2)"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        // check indent of list style - override
        auto xLevels = getProperty<uno::Reference<container::XIndexAccess>>(xPara, "NumberingRules");
        uno::Sequence<beans::PropertyValue> aLevel;
        xLevels->getByIndex(0) >>= aLevel; // top level
        sal_Int32 nIndent = std::find_if(std::cbegin(aLevel), std::cend(aLevel), [](const beans::PropertyValue& rValue) { return rValue.Name == "FirstLineIndent"; })->Value.get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9366), nIndent);
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        // different numbering style
        OUString listStyle3;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle3);
        CPPUNIT_ASSERT(listStyle3.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle3 != listStyle);
        // and different list
        CPPUNIT_ASSERT_EQUAL(OUString("1."), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT(listId !=  getProperty<OUString>(xPara, "ListId"));
    }
    {
        // continue the first list
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, "NumberingStyleName"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        CPPUNIT_ASSERT_EQUAL(OUString("3)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, "NumberingLevel"));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, "NumberingStyleName"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        CPPUNIT_ASSERT_EQUAL(OUString("4)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf108496, "tdf108496.docx")
{
    OUString listId;
    OUString listStyle;
    // Lists with override
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("ListId") >>= listId);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, "NumberingStyleName"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // restarted numeration due to override
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        // restarted numeration due to override
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
    }

    // Lists without override
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue("ListId") >>= listId);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, "NumberingStyleName"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
        CPPUNIT_ASSERT_EQUAL(OUString("2"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(11), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // numeration is continued
        CPPUNIT_ASSERT_EQUAL(OUString("3"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(12), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue("NumberingStyleName") >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        // numeration is continued
        CPPUNIT_ASSERT_EQUAL(OUString("4"), getProperty<OUString>(xPara, "ListLabelString"));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, "ListId"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf126723, "tdf126723.docx")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), "ParaLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testendingSectionProps, "endingSectionProps.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, "HeaderText");
    CPPUNIT_ASSERT_EQUAL( OUString("General header"), xHeaderText->getString());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section is RightToLeft", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xSect, "WritingMode"));
    //regression: tdf124637
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(2540), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
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
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_DRAWFRMFMT), rFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_FLYFRMFMT), rFormats[1]->Which());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 5, Actual:
    // 4', i.e. the textbox inherited its writing direction instead of having an explicit btlr
    // value.
    CPPUNIT_ASSERT_EQUAL(SvxFrameDirection::Vertical_LR_BT,
                         rFormats[1]->GetAttrSet().GetFrameDir().GetValue());
}

DECLARE_OOXMLEXPORT_TEST(testTdf127316_autoEscapement, "tdf127316_autoEscapement.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This should be roughly .8*35% of the ORIGINAL(non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 28% (6.72pt) becomes roughly 42% for the 16pt font.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), "CharEscapement"), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.f, getProperty<float>(getRun(xPara, 2), "CharEscapement"), 1);

    // Subscripts are different. Automatic escapement SHOULD BE limited by the font bottom line(?)
    // and so the calculations ought to be different. There is room for a lot of export improvement here.
    xPara.set(getParagraph(2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Normal text "), "CharEscapement"), 0);
    // Negative escapements (subscripts) were decreasing by 1% every round-trip due to bad manual rounding.
    // This should be roughly .2*35% of the ORIGINAL (non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 7% (1.68pt) becomes roughly 10.5% for the 16pt font.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Subscript", -10.f, getProperty<float>(getRun(xPara, 2), "CharEscapement"), 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99602_subscript_charStyleSize, "tdf99602_subscript_charStyleSize.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    // The word "Base" should not be subscripted.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Base"), "CharEscapement"), 0);
    // The word "Subscript" should be 48pt, subscripted by 25% (12pt).
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -25.f, getProperty<float>(getRun(xPara, 2, "Subscript"), "CharEscapement"), 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99602_charStyleSubscript, "tdf99602_charStyleSubscript.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    // The word "Base" should not be subscripted.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, "Base"), "CharEscapement"), 0);
    // The word "Subscript" should be 48pt, automatically subscripted, and automatic proportioned.
    CPPUNIT_ASSERT_EQUAL( sal_Int16(DFLT_ESC_AUTO_SUB), getProperty<sal_Int16>(getRun(xPara, 2, "Subscript"), "CharEscapement") );
    CPPUNIT_ASSERT_EQUAL( sal_Int16(DFLT_ESC_PROP), getProperty<sal_Int16>(getRun(xPara, 2), "CharEscapementHeight") );
}

DECLARE_OOXMLEXPORT_TEST(testTdf99602_charStyleSubscript2, "tdf99602_charStyleSubscript2.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // *_In styles_*, don't let the proportionality/escapement affect the fontsize - otherwise it starts doubling up,
    // so instead just throw away the values and use the default settings instead - meaning fontsize is unaffected.
    // subscript custom: Proportional size is 80%, lower by 25%.
    uno::Reference<beans::XPropertySet> xStyle(getStyles("CharacterStyles")->getByName("subscript custom"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CharStyle has 12pt font size", 12.f, getProperty<float>(xStyle, "CharHeight"));
    // subscript larger font: Proportional size is 80%, lowered by DFLT_ESC_AUTO_SUB
    xStyle.set(getStyles("CharacterStyles")->getByName("subscript larger font"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto CharStyle has 12pt font size", 12.f, getProperty<float>(xStyle, "CharHeight"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf124637_sectionMargin, "tdf124637_sectionMargin.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // sections 0 and 1 must be related to footnotes...
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(2), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(0), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
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

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak3, "tdf123636_newlinePageBreak3.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //proof case: split any non-empty paragraphs, not just the first paragraph of a section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 5, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[1]/body/txt[3]/Text[1]", "Portion", "Last line on page 1");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak4, "tdf123636_newlinePageBreak4.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: an empty paragraph doesn't split (except if first paragraph).
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 3, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[2]/body/txt[1]/Text", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf118947_tableStyle, "tdf118947_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Table grid settings set line-spacing to 250% instead of single-spacing, which is set as a document default."), xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextBody has 10pt font size", 11.f, getProperty<float>(xPara, "CharHeight"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextBody has 1pt space below paragraph", sal_Int32(35), getProperty<sal_Int32>(xPara, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table has 10pt space above paragraph", sal_Int32(353), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table style sets 0 right margin", sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaRightMargin"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("TextBody has 1.07 line-spacing", sal_Int16(107), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height, 1);
    // table-style based paragraph background color
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing paragraph background color in cell A1", Color(0xCCFFCC), Color(ColorTransparency, getProperty<sal_Int32>(xPara, "ParaBackColor")));

    // This cell is affected by compatSetting overrideTableStyleFontSizeAndJustification=0 (the default value)
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Notice that this is 8pt font, right aligned in compatibility mode."), xPara->getString());
    // Even though not specified, Table-Style distributes the properties in DocDefault. DocDefault fontsize is 8pt.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compat mode has 8pt font size", 8.f, getProperty<float>(getRun(xPara,1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Normal has 0pt space below paragraph", sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaBottomMargin"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table sets 10pt space above paragraph", sal_Int32(353), getProperty<sal_Int32>(xPara, "ParaTopMargin"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table style sets 0 right margin", sal_Int32(0), getProperty<sal_Int32>(xPara, "ParaRightMargin"));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table sets 2.5 line-spacing", sal_Int16(250), getProperty<style::LineSpacing>(xPara, "ParaLineSpacing").Height, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph background color in cell A2", sal_Int32(-1), getProperty<sal_Int32>(xPara, "ParaBackColor"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compat mode overrides left adjust", style::ParagraphAdjust_RIGHT,
                                 static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118947_tableStyle2, "tdf118947_tableStyle2.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    // This cell is affected by compatSetting overrideTableStyleFontSizeAndJustification=1 (no goofy exception)
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A2"), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString("Notice that this is 12pt font, left aligned in non-compatibility mode."), xPara->getString());
    // Even though not specified, Table-Style tries to distribute the properties in DocDefault. DocDefault fontsize is 8pt.
    // However, this is overridden by the default style's specified fontsize of 12 and left justify.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non-Compat mode has 12pt font size", 12.f, getProperty<float>(getRun(xPara,1), "CharHeight"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non-Compat mode keeps the style's left adjust", style::ParagraphAdjust_LEFT,
                                 static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, "ParaAdjust")));
}

DECLARE_OOXMLEXPORT_TEST(tdf123912_protectedForm, "tdf123912_protectedForm.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compatibility: Protect form", true,
                                 pDoc->getIDocumentSettingAccess().get( DocumentSettingId::PROTECT_FORM ) );

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);
    if ( xSect.is() )
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", false, getProperty<bool>(xSect, "IsProtected"));
}

DECLARE_OOXMLEXPORT_TEST(tdf124600b, "tdf124600b.docx")
{
    // <wp:anchor allowOverlap="0"> was lost on roundtrip, we always wrote "1" on export.
    bool bAllowOverlap1 = getProperty<bool>(getShape(1), "AllowOverlap");
    CPPUNIT_ASSERT(!bAllowOverlap1);
    bool bAllowOverlap2 = getProperty<bool>(getShape(2), "AllowOverlap");
    CPPUNIT_ASSERT(!bAllowOverlap2);
}

CPPUNIT_TEST_FIXTURE(Test, testDateControl)
{
    loadAndReload("empty-date-control.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Check that we exported the empty date control correctly
    // Date form field is converted to date content control.

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();


    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), pMarkAccess->getAllMarksCount());
    ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*pMarkAccess->getAllMarksBegin());

    CPPUNIT_ASSERT(pFieldmark);
    CPPUNIT_ASSERT_EQUAL(OUString(ODF_FORMDATE), pFieldmark->GetFieldname());

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

    CPPUNIT_ASSERT_EQUAL(OUString("dd/MM/yyyy"), sDateFormat);
    CPPUNIT_ASSERT_EQUAL(OUString("en-US"), sLang);
    CPPUNIT_ASSERT_EQUAL(OUString(""), sCurrentDate);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121867, "tdf121867.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 3; Actual  : 0', i.e. page width zoom was lost on export.
    CPPUNIT_ASSERT_EQUAL(SvxZoomType::PAGEWIDTH, pEditShell->GetViewOptions()->GetZoomType());
}

DECLARE_OOXMLEXPORT_TEST(testParaAdjustDistribute, "para-adjust-distribute.docx")
{
    // Without the accompanying fix in place, this test would have failed with
    // 'Expected: 2; Actual  : 0', i.e. the first paragraph's ParaAdjust was
    // left, not block.
    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), "ParaLastLineAdjust")));

    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(2), "ParaAdjust")));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(2), "ParaLastLineAdjust")));
}

DECLARE_OOXMLEXPORT_TEST(testInputListExport, "tdf122186_input_list.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    if (!mbExported || getShapes() == 0) // importing the ODT, an input field
    {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.textfield.DropDown"));
    }
    else // importing the DOCX, a form control
    {
        uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox"));
        CPPUNIT_ASSERT(getProperty<bool>(xPropertySet, "Dropdown"));
        auto const items(getProperty<uno::Sequence<OUString>>(xPropertySet, "StringItemList"));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(3), items.getLength());
        CPPUNIT_ASSERT_EQUAL(OUString("1"), items[0]);
        CPPUNIT_ASSERT_EQUAL(OUString("2"), items[1]);
        CPPUNIT_ASSERT_EQUAL(OUString("3"), items[2]);
        CPPUNIT_ASSERT_EQUAL(OUString("1"), getProperty<OUString>(xPropertySet, "DefaultText"));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf123435, "tdf123435.docx")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    // Without the fix in place, it would have failed with
    // - Expected: 2
    // - Actual  : 1
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
}

DECLARE_OOXMLEXPORT_TEST(testTdf116371, "tdf116371.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, "RotateAngle"), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, "FrameRect");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24063), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24179), frameRect.Width);
}

CPPUNIT_TEST_FIXTURE(Test, testFrameSizeExport)
{
    loadAndSave("floating-tables-anchor.docx");
    // Make sure the table width is 4000
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW", "w", "4000");
}

DECLARE_OOXMLEXPORT_TEST(testTdf119201, "tdf119201.docx")
{
    // Visibility of shapes wasn't imported/exported, for now base printable property on that, too
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_MESSAGE("First shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("First shape should be printable.", getProperty<bool>(xShape, "Printable"));
    xShape = getShapeByName(u"Rectangle 1");
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
    xShape = getShapeByName(u"Oval 2");
    CPPUNIT_ASSERT_MESSAGE("Third shape should be visible.", getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Third shape should be printable.", getProperty<bool>(xShape, "Printable"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf124594, "tdf124594.docx")
{
    xmlDocUniquePtr pDump = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, as the portion text was
    // only "Er horte leise Schritte hinter", which means the 1st line of the 2nd paragraph was
    // split into two by a Special portion, i.e. the top margin of the shape was too large.
    assertXPath(pDump, "/root/page/body/txt[2]/Text[1]", "Portion",
                "Er horte leise Schritte hinter sich. Das bedeutete nichts Gutes. Wer wu"); // ... until the bookmark.
}

CPPUNIT_TEST_FIXTURE(Test, testTextInput)
{
    loadAndSave("textinput.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");


    // test the exported DOCX

    // no hint, empty
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:instrText", " FILLIN \"\"");
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]/w:rPr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:fldChar", "fldCharType", "end");

    // no hint, content
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:instrText", " FILLIN \"\"");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:t", "content without hint");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[6]/w:fldChar", "fldCharType", "end");

    // hint, empty
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[3]/w:instrText", " FILLIN \"hint empty\"");
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[5]", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[5]/w:rPr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[6]/w:fldChar", "fldCharType", "end");

    // hint, content
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[3]/w:instrText", " FILLIN \"hint content\"");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[5]/w:t", "content with hint");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[6]/w:fldChar", "fldCharType", "end");

    // test the imported DOCX
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    int nElements = 0;

    do
    {
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.textfield.Input"));
        uno::Reference<beans::XPropertySet> xPropertySet(aField, uno::UNO_QUERY);
        uno::Reference<text::XTextContent> xText(aField, uno::UNO_QUERY);

        // why is the enumeration not in the same order then the fields in the document?
        // it seems to be stable and the navigation in the GUI is actually correct.
        OUString sContent, sHint;
        switch (nElements)
        {
        case 1:
            sContent = "content with hint";
            sHint = "hint content";
            break;
        case 2:
            sHint = "hint empty";
            break;
        case 3:
            sContent = "content without hint";
            break;
        }
        CPPUNIT_ASSERT_EQUAL(uno::makeAny(sContent), xPropertySet->getPropertyValue("Content"));
        CPPUNIT_ASSERT_EQUAL(sContent, xText->getAnchor()->getString());
        CPPUNIT_ASSERT_EQUAL(uno::makeAny(sHint), xPropertySet->getPropertyValue("Hint"));
        nElements++;
    }
    while (xFields->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(4, nElements);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123460, "tdf123460.docx")
{
    // check paragraph mark deletion at terminating moveFrom
    CPPUNIT_ASSERT_EQUAL(true,getParagraph( 2 )->getString().startsWith("Nunc"));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 1 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(true, getRun( getParagraph( 2 ), 3 )->getString().endsWith("tellus."));
    // deleted paragraph mark at the end of the second paragraph
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 5), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Delete"),getProperty<OUString>(getRun(getParagraph(2), 5), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 6 )->getString());
}

//tdf#125298: fix charlimit restrictions in bookmarknames and field references if they contain non-ascii characters
CPPUNIT_TEST_FIXTURE(Test, testTdf125298)
{
    loadAndSave("tdf125298_crossreflink_nonascii_charlimit.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // check whether test file keeps non-ascii values or not
    OUString bookmarkName1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart[1]", "name");
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e1rv\u00edzt\u0171r\u0151_t\u00fck\u00f6rf\u00far\u00f3g\u00e9p"), bookmarkName1);

    OUString bookmarkName2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:bookmarkStart[1]", "name");
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e91\u00e12\u01713\u01514\u00fa5\u00f66\u00fc7\u00f38\u00ed9"), bookmarkName2);
    OUString fieldName1 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[2]/w:instrText[1]");
    OUString expectedFieldName1 = " REF " + bookmarkName1 + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName1, fieldName1);
    OUString fieldName2 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[7]/w:r[2]/w:instrText[1]");
    OUString expectedFieldName2 = " REF " + bookmarkName2 + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName2, fieldName2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121784, "tdf121784.docx")
{
    // check tracked insertion of footnotes
    CPPUNIT_ASSERT_EQUAL( OUString( "Text1" ), getParagraph( 1 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 1 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(1), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "1" ), getRun( getParagraph( 1 ), 3 )->getString());

    // check tracked insertion of endnotes
    CPPUNIT_ASSERT_EQUAL( OUString( "texti" ), getParagraph( 2 )->getString());
    CPPUNIT_ASSERT_EQUAL( OUString( "" ), getRun( getParagraph( 2 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL(OUString("Insert"),getProperty<OUString>(getRun(getParagraph(2), 2), "RedlineType"));
    CPPUNIT_ASSERT_EQUAL( OUString( "i" ), getRun( getParagraph( 2 ), 3 )->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTbrlFrameVml, "tbrl-frame-vml.docx")
{
    uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextFrame.is());

    if (mbExported)
    {
        // DML import: creates a TextBox, eaVert read back as TB_RL in TextWritingMode

        auto eMode = getProperty<text::WritingMode>(xTextFrame, "TextWritingMode");
        CPPUNIT_ASSERT_EQUAL(text::WritingMode::WritingMode_TB_RL, eMode);
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

CPPUNIT_TEST_FIXTURE(Test, testTdf119037)
{
    loadAndSave("tdf119037.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:top", "val", "single");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:left", "val", "single");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:right", "val", "single");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:bottom", "val", "single");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:top", "val", "dotted");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:left", "val", "dotted");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:right", "val", "dotted");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:bottom", "val", "dotted");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:top", "val", "dashed");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:left", "val", "dashed");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:right", "val", "dashed");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:bottom", "val", "dashed");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:top", "val", "dashSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:left", "val", "dashSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:right", "val", "dashSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:bottom", "val", "dashSmallGap");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:top", "val", "dotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:left", "val", "dotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:right", "val", "dotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:bottom", "val", "dotDash");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:top", "val", "dotDotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:left", "val", "dotDotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:right", "val", "dotDotDash");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:bottom", "val", "dotDotDash");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:top", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:left", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:right", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:bottom", "val", "double");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:top", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:left", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:right", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:bottom", "val", "double");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:top", "val", "thinThickSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:left", "val", "thinThickSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:right", "val", "thinThickSmallGap");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:bottom", "val", "thinThickSmallGap");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:top", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:left", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:right", "val", "double");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:bottom", "val", "double");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125657)
{
    loadAndSave("tdf125657.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    auto checkAttrIsInt = [&](const OString& sAttrName) {
        OUString sAttr = getXPath(pXmlDoc,
                                  "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:inline/a:graphic/"
                                  "a:graphicData/pic:pic/pic:blipFill/a:srcRect",
                                  sAttrName);
        OString sAssertMsg("Attribute " + sAttrName + " value " + sAttr.toUtf8()
                           + " is not a valid integer");
        CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(), !sAttr.isEmpty());
        // Only decimal characters allowed, optionally prepended with '-'; no '.'
        CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(),
                               sAttr[0] == '-' || (sAttr[0] >= '0' && sAttr[0] <= '9'));
        for (sal_Int32 i = 1; i < sAttr.getLength(); ++i) {
            CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(), sAttr[i] >= '0');
            CPPUNIT_ASSERT_MESSAGE(sAssertMsg.getStr(), sAttr[i] <= '9');
        }
    };
    // check that we export all coordinates of srcRect as integers
    checkAttrIsInt("l");
    checkAttrIsInt("t");
    checkAttrIsInt("r");
    checkAttrIsInt("b");
}

DECLARE_OOXMLEXPORT_TEST(testTdf125324, "tdf125324.docx")
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/tab/infos/bounds", "top", "4193");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf78657)
{
    loadAndSave("tdf78657_picture_hyperlink.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    xmlDocUniquePtr pXmlRels = parseExport("word/_rels/document.xml.rels");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/wp:docPr/a:hlinkClick", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:nvPicPr/pic:cNvPr/a:hlinkClick", 1);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Target='http://www.google.com']", "TargetMode", "External");
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrame)
{
    loadAndReload("btlr-frame.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aGeometry(xPropertySet->getPropertyValue("CustomShapeGeometry"));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // -270; Actual: 0', i.e. the writing direction of the frame was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-270),
                         aGeometry["TextPreRotateAngle"].get<sal_Int32>());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125518)
{
    loadAndSave("tdf125518.odt");
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");


    // First diagram is anchored
    OUString anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object1"), anchorName);

    // Second diagram has anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Objekt1"), anchorName);

    // Third diagram has no anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:r[2]/w:drawing/wp:inline/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object2"), anchorName);

    // 4th diagram has anchor too
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:r[3]/w:drawing/wp:anchor/wp:docPr", "name");
    CPPUNIT_ASSERT_EQUAL(OUString("Object3"), anchorName);
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

CPPUNIT_TEST_FIXTURE(Test, testTdf131594)
{
    loadAndSave("tdf131594.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // lnNumType should not be exported if w:countBy="0"
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:lnNumType", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121663)
{
    loadAndSave("tdf121663.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    // auto distance of line numbering is 0.5 cm
    assertXPath(pXmlDoc, "//w:lnNumType", "distance", "283");
}

DECLARE_OOXMLEXPORT_TEST(testInvalidDateFormField, "invalid_date_form_field.docx")
{

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    CPPUNIT_ASSERT_EQUAL(sal_Int32(6), pMarkAccess->getAllMarksCount());

    int nIndex = 0;
    for(auto aIter = pMarkAccess->getAllMarksBegin(); aIter != pMarkAccess->getAllMarksEnd(); ++aIter)
    {
        ::sw::mark::IFieldmark* pFieldmark = dynamic_cast<::sw::mark::IFieldmark*>(*aIter);

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

            CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(5), pFieldmark->GetMarkStart().nContent.GetIndex());
        }
        else if (nIndex == 1) // The second has wrong date
        {
            CPPUNIT_ASSERT_EQUAL(OUString("MM/DD/YY"), sDateFormat);
            CPPUNIT_ASSERT_EQUAL(OUString("en-US"), sLang);
            CPPUNIT_ASSERT_EQUAL(OUString("2019.06.34"), sCurrentDate);

            CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(15), pFieldmark->GetMarkStart().nContent.GetIndex());
        }
        else // The third one has wrong local
        {
            CPPUNIT_ASSERT_EQUAL(OUString("[NatNum12 MMMM=abbreviation]YYYY\". \"MMMM D."), sDateFormat);
            CPPUNIT_ASSERT_EQUAL(OUString("xxxx"), sLang);
            CPPUNIT_ASSERT_EQUAL(OUString("2019.06.11"), sCurrentDate);

            CPPUNIT_ASSERT_EQUAL(SwNodeOffset(9), pFieldmark->GetMarkStart().nNode.GetIndex());
            CPPUNIT_ASSERT_EQUAL(sal_Int32(35), pFieldmark->GetMarkStart().nContent.GetIndex());
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
    if (getShapes() > 0)
    {
        uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<OUString> >(xPropertySet, "StringItemList").getLength());
    }
    else
    {
        // ComboBox was imported as DropDown text field
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.textfield.DropDown"));

        uno::Sequence<OUString> aItems = getProperty< uno::Sequence<OUString> >(aField, "Items");
        CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aItems.getLength());
    }
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

CPPUNIT_TEST_FIXTURE(Test, testTdf127116)
{
    loadAndSave("tdf127116.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");


    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127339)
{
    loadAndSave("tdf127339.docx");
    xmlDocUniquePtr pXmlRels = parseExport("word/_rels/document.xml.rels");

    assertXPathNoAttribute(pXmlRels, "/rels:Relationships/rels:Relationship[@Target='#bookmark']", "TargetMode");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127362)
{
    loadAndSave("tdf127362.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    xmlDocUniquePtr pXmlRels = parseExport("word/_rels/document.xml.rels");

    OUString bookmarkName = "#" + getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']", "Target");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127605)
{
    loadAndSave("tdf127605.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127732)
{
    loadAndSave("internal_hyperlink_frame.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127733)
{
    loadAndSave("internal_hyperlink_ole.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");


    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127734)
{
    loadAndSave("internal_hyperlink_region.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127735)
{
    loadAndSave("internal_hyperlink_table.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:bookmarkStart", "name");
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink", "anchor");
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123628)
{
    loadAndSave("tdf123628.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink/w:r/w:rPr/w:rStyle", "val", "InternetLink");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='InternetLink']/w:name", "val", "Hyperlink");
}

DECLARE_OOXMLEXPORT_TEST(testTdf127741, "tdf127741.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    OUString unVisitedStyleName = getProperty<OUString>(xRun, "UnvisitedCharStyleName");
    CPPUNIT_ASSERT(unVisitedStyleName.equalsIgnoreAsciiCase("Internet Link"));
    OUString visitedStyleName = getProperty<OUString>(xRun, "VisitedCharStyleName");
    CPPUNIT_ASSERT(visitedStyleName.equalsIgnoreAsciiCase("Visited Internet Link"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142693_hugePaperSizeImport)
{
    loadAndSave("tdf142693_hugePaperSizeImport.docx");
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgSz", "w", "90369");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgSz", "h", "104372");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127925)
{
    loadAndSave("tdf127925.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlStyles = parseExport("word/styles.xml");
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='VisitedInternetLink']/w:name", "val", "FollowedHyperlink");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127579)
{
    loadAndSave("tdf127579.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink/w:r/w:rPr/w:rStyle", "val", "InternetLink");
}

DECLARE_OOXMLEXPORT_TEST(testTdf128304, "tdf128304.odt")
{
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    css::text::WritingMode eMode;
    uno::Reference<beans::XPropertySet> xProps1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps1->getPropertyValue("TextWritingMode") >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps2->getPropertyValue("TextWritingMode") >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps3(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps3->getPropertyValue("TextWritingMode") >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps4(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps4->getPropertyValue("TextWritingMode") >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
