/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/XTextTable.hpp>

#include <editeng/escapementitem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <xmloff/odffields.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <editsh.hxx>
#include <frmatr.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr) {}
};

// TODO: the re-import doesn't work just yet, but that isn't a regression...
CPPUNIT_TEST_FIXTURE(Test, testFlyInFly)
{
    createSwDoc("ooo39250-1-min.rtf");
    save(mpFilter);
    // check that anchor of text frame is in other text frame
    uno::Reference<text::XTextContent> const xAnchored(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchored.is());
    CPPUNIT_ASSERT_EQUAL(u"Frame1"_ustr/*generated name*/, uno::Reference<container::XNamed>(xAnchored, uno::UNO_QUERY_THROW)->getName());
    uno::Reference<text::XText> const xAnchorText(xAnchored->getAnchor()->getText());
    uno::Reference<text::XTextFrame> const xAnchorFrame(xAnchorText, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xAnchorFrame.is());
    CPPUNIT_ASSERT_EQUAL(u"Frame3"_ustr, uno::Reference<container::XNamed>(xAnchorFrame, uno::UNO_QUERY_THROW)->getName());
}

DECLARE_OOXMLEXPORT_TEST(testTdf125778_lostPageBreakTOX, "tdf125778_lostPageBreakTOX.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf126994_lostPageBreak, "tdf126994_lostPageBreak.docx")
{
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 3, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf155690, "tdf155690.docx")
{
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xBookmarks = xBookmarksSupplier->getBookmarks();
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row1_1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        // the problem was that the start was after the H
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row1_2"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row1_3"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"ello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row1_4"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row2_1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row2_1"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row2_3"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"ello world"_ustr, xMark->getAnchor()->getString());
    }
    {
        uno::Reference<text::XTextContent> xMark(xBookmarks->getByName(u"row2_4"_ustr), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xMark.is());
        CPPUNIT_ASSERT_EQUAL(u"Hello world"_ustr, xMark->getAnchor()->getString());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121374_sectionHF)
{
    loadAndReload("tdf121374_sectionHF.odt");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xFooterText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, u"FooterText"_ustr);
    CPPUNIT_ASSERT_EQUAL( u"footer"_ustr, xFooterText->getString() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 6, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 6, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121374_sectionHF2)
{
    loadAndReload("tdf121374_sectionHF2.doc");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, u"HeaderText"_ustr);
    CPPUNIT_ASSERT( xHeaderText->getString().startsWith("virkamatka-anomus") );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121666_lostPage)
{
    loadAndSave("tdf121666_lostPage.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:br"_ostr, "type"_ostr, u"page"_ustr);
    // The second page break is exported too.
    // Before this fix, if a node had both section break and page break, then only the section break was exported.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:br"_ostr, "type"_ostr, u"page"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:sectPr/w:type"_ostr, "val"_ostr, u"nextPage"_ustr);
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
    saveAndReload(u"Office Open XML Text"_ustr);
    bool bBreakOnPara3 = getProperty<style::BreakType>(getParagraph(3), u"BreakType"_ustr) == style::BreakType_COLUMN_BEFORE;
    bool bBreakOnPara4 = getProperty<style::BreakType>(getParagraph(4), u"BreakType"_ustr) == style::BreakType_COLUMN_BEFORE;
    CPPUNIT_ASSERT(bBreakOnPara3 || bBreakOnPara4);
}

DECLARE_OOXMLEXPORT_TEST(testTdf95848, "tdf95848.docx")
{
    OUString listId;
    OUString listStyle;
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"ListId"_ustr) >>= listId);
        CPPUNIT_ASSERT_EQUAL(u"1.1.1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, u"NumberingStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"1.1.2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(2), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        // different numbering style
        OUString listStyle3;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle3);
        CPPUNIT_ASSERT(listStyle3.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle3 != listStyle);
        // but same list
        CPPUNIT_ASSERT_EQUAL(u"1.1.3"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf95848_2, "tdf95848_2.docx")
{
    OUString listId;
    OUString listStyle;
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"ListId"_ustr) >>= listId);
        CPPUNIT_ASSERT_EQUAL(u"1)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        // check indent of list style
        auto xLevels = getProperty<uno::Reference<container::XIndexAccess>>(xPara, u"NumberingRules"_ustr);
        uno::Sequence<beans::PropertyValue> aLevel;
        xLevels->getByIndex(0) >>= aLevel; // top level
        sal_Int32 nIndent = std::find_if(std::cbegin(aLevel), std::cend(aLevel), [](const beans::PropertyValue& rValue) { return rValue.Name == "FirstLineIndent"; })->Value.get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(-635), nIndent);
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // but same list
        CPPUNIT_ASSERT_EQUAL(u"2)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        // check indent of list style - override
        auto xLevels = getProperty<uno::Reference<container::XIndexAccess>>(xPara, u"NumberingRules"_ustr);
        uno::Sequence<beans::PropertyValue> aLevel;
        xLevels->getByIndex(0) >>= aLevel; // top level
        sal_Int32 nIndent = std::find_if(std::cbegin(aLevel), std::cend(aLevel), [](const beans::PropertyValue& rValue) { return rValue.Name == "FirstLineIndent"; })->Value.get<sal_Int32>();
        CPPUNIT_ASSERT_EQUAL(sal_Int32(9366), nIndent);
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        // different numbering style
        OUString listStyle3;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle3);
        CPPUNIT_ASSERT(listStyle3.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle3 != listStyle);
        // and different list
        CPPUNIT_ASSERT_EQUAL(u"1."_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT(listId !=  getProperty<OUString>(xPara, u"ListId"_ustr));
    }
    {
        // continue the first list
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, u"NumberingStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"3)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(0), getProperty<sal_Int16>(xPara, u"NumberingLevel"_ustr));
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, u"NumberingStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"4)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf108496, "tdf108496.docx")
{
    OUString listId;
    OUString listStyle;
    // Lists with override
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"ListId"_ustr) >>= listId);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, u"NumberingStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // restarted numeration due to override
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        // restarted numeration due to override
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
    }

    // Lists without override
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle);
        CPPUNIT_ASSERT(listStyle.startsWith("WWNum"));
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"ListId"_ustr) >>= listId);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(listStyle, getProperty<OUString>(xPara, u"NumberingStyleName"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(11), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        CPPUNIT_ASSERT(listStyle2 != listStyle);
        // numeration is continued
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(12), uno::UNO_QUERY);
        // different numbering style
        OUString listStyle2;
        CPPUNIT_ASSERT(xPara->getPropertyValue(u"NumberingStyleName"_ustr) >>= listStyle2);
        CPPUNIT_ASSERT(listStyle2.startsWith("WWNum"));
        // numeration is continued
        CPPUNIT_ASSERT_EQUAL(u"4"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
        CPPUNIT_ASSERT_EQUAL(listId, getProperty<OUString>(xPara, u"ListId"_ustr));
    }
}

DECLARE_OOXMLEXPORT_TEST(testTdf126723, "tdf126723.docx")
{
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), getProperty<sal_Int32>(getParagraph(2), u"ParaLeftMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testendingSectionProps, "endingSectionProps.docx")
{
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles(u"PageStyles"_ustr)->getByName(u"Standard"_ustr), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xHeaderText = getProperty< uno::Reference<text::XTextRange> >(xPageStyle, u"HeaderText"_ustr);
    CPPUNIT_ASSERT_EQUAL( u"General header"_ustr, xHeaderText->getString());

    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(0), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("# of paragraphs", 2, getParagraphs());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section is RightToLeft", text::WritingMode2::RL_TB, getProperty<sal_Int16>(xSect, u"WritingMode"_ustr));
    //regression: tdf124637
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(2540), getProperty<sal_Int32>(xSect, "SectionLeftMargin"));
}

DECLARE_OOXMLEXPORT_TEST(testTbrlTextbox, "tbrl-textbox.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with 'Expected: -90;
    // Actual: 0', i.e. tbRl writing direction was imported as lrTb.
    // Note: Implementation was changed to use WritingMode property instead of TextPreRotateAngle.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL90,
                         getProperty<sal_Int16>(xPropertySet, u"WritingMode"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testBtlrShape, "btlr-textbox.docx")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_DRAWFRMFMT), rFormats[0]->Which());
    CPPUNIT_ASSERT_EQUAL(o3tl::narrowing<sal_uInt16>(RES_FLYFRMFMT), rFormats[1]->Which());
    // Without the accompanying fix in place, this test would have failed with 'Expected: 5, Actual:
    // 4', i.e. the textbox inherited its writing direction instead of having an explicit btlr
    // value.
    CPPUNIT_ASSERT_EQUAL(SvxFrameDirection::Vertical_LR_BT,
                         rFormats[1]->GetAttrSet().GetFrameDir().GetValue());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127316_autoEscapement)
{
    loadAndReload("tdf127316_autoEscapement.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // This should be roughly .8*35% of the ORIGINAL(non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 28% (6.72pt) becomes roughly 42% for the 16pt font.
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1), u"CharEscapement"_ustr), 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(42.f, getProperty<float>(getRun(xPara, 2), u"CharEscapement"_ustr), 1);

    // Subscripts are different. Automatic escapement SHOULD BE limited by the font bottom line(?)
    // and so the calculations ought to be different. There is room for a lot of export improvement here.
    xPara.set(getParagraph(2));
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Normal text "_ustr), u"CharEscapement"_ustr), 0);
    // Negative escapements (subscripts) were decreasing by 1% every round-trip due to bad manual rounding.
    // This should be roughly .2*35% of the ORIGINAL (non-reduced) size. However, during export the
    // proportional height has to be changed into direct formatting, which then changes the relative percent.
    // In this case, a 24pt font, proportional at 65% becomes roughly a 16pt font.
    // Thus an escapement of 7% (1.68pt) becomes roughly 10.5% for the 16pt font.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Subscript", -10.f, getProperty<float>(getRun(xPara, 2), u"CharEscapement"_ustr), 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99602_subscript_charStyleSize, "tdf99602_subscript_charStyleSize.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    // The word "Base" should not be subscripted.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Base"_ustr), u"CharEscapement"_ustr), 0);
    // The word "Subscript" should be 48pt, subscripted by 25% (12pt).
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -25.f, getProperty<float>(getRun(xPara, 2, u"Subscript"_ustr), u"CharEscapement"_ustr), 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf99602_charStyleSubscript, "tdf99602_charStyleSubscript.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    // The word "Base" should not be subscripted.
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.f, getProperty<float>(getRun(xPara, 1, u"Base"_ustr), u"CharEscapement"_ustr), 0);
    // The word "Subscript" should be 48pt, automatically subscripted, and automatic proportioned.
    CPPUNIT_ASSERT_EQUAL( sal_Int16(DFLT_ESC_AUTO_SUB), getProperty<sal_Int16>(getRun(xPara, 2, u"Subscript"_ustr), u"CharEscapement"_ustr) );
    CPPUNIT_ASSERT_EQUAL( sal_Int16(DFLT_ESC_PROP), getProperty<sal_Int16>(getRun(xPara, 2), u"CharEscapementHeight"_ustr) );
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99602_charStyleSubscript2)
{
    loadAndReload("tdf99602_charStyleSubscript2.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // *_In styles_*, don't let the proportionality/escapement affect the fontsize - otherwise it starts doubling up,
    // so instead just throw away the values and use the default settings instead - meaning fontsize is unaffected.
    // subscript custom: Proportional size is 80%, lower by 25%.
    uno::Reference<beans::XPropertySet> xStyle(getStyles(u"CharacterStyles"_ustr)->getByName(u"subscript custom"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("CharStyle has 12pt font size", 12.f, getProperty<float>(xStyle, u"CharHeight"_ustr));
    // subscript larger font: Proportional size is 80%, lowered by DFLT_ESC_AUTO_SUB
    xStyle.set(getStyles(u"CharacterStyles"_ustr)->getByName(u"subscript larger font"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Auto CharStyle has 12pt font size", 12.f, getProperty<float>(xStyle, u"CharHeight"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf124637_sectionMargin, "tdf124637_sectionMargin.docx")
{
    uno::Reference<text::XTextSectionsSupplier> xTextSectionsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xSections(xTextSectionsSupplier->getTextSections(), uno::UNO_QUERY);
    // sections 0 and 1 must be related to footnotes...
    uno::Reference<beans::XPropertySet> xSect(xSections->getByIndex(2), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Section Left Margin", sal_Int32(0), getProperty<sal_Int32>(xSect, u"SectionLeftMargin"_ustr));
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
    CPPUNIT_ASSERT_EQUAL(OUString(), getProperty<OUString>(getParagraph(2, u""_ustr), u"NumberingStyleName"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak3, "tdf123636_newlinePageBreak3.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //proof case: split any non-empty paragraphs, not just the first paragraph of a section.
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 5, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[1]/body/txt[3]/SwParaPortion/SwLineLayout/SwParaPortion[1]"_ostr, "portion"_ostr, u"Last line on page 1"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123636_newlinePageBreak4, "tdf123636_newlinePageBreak4.docx")
{
    //MS Compatibility flag: SplitPgBreakAndParaMark
    //special case: an empty paragraph doesn't split (except if first paragraph).
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Paragraphs", 3, getParagraphs() );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Number of Pages", 2, getPages() );

    xmlDocUniquePtr pDump = parseLayoutDump();
    assertXPath(pDump, "/root/page[2]/body/txt[1]/SwParaPortion"_ostr, 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf118947_tableStyle, "tdf118947_tableStyle.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Table grid settings set line-spacing to 250% instead of single-spacing, which is set as a document default."_ustr, xPara->getString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextBody has 10pt font size", 11.f, getProperty<float>(xPara, u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TextBody has 1pt space below paragraph", sal_Int32(35), getProperty<sal_Int32>(xPara, u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table has 10pt space above paragraph", sal_Int32(353), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table style sets 0 right margin", sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("TextBody has 1.07 line-spacing", sal_Int16(107), getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height, 1);
    // table-style based paragraph background color
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Missing paragraph background color in cell A1", Color(0xCCFFCC), getProperty<Color>(xPara, u"ParaBackColor"_ustr));

    // This cell is affected by compatSetting overrideTableStyleFontSizeAndJustification=0 (the default value)
    xCell.set(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    xParaEnumAccess.set(xCell->getText(), uno::UNO_QUERY);
    xParaEnum = xParaEnumAccess->createEnumeration();
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Notice that this is 8pt font, right aligned in compatibility mode."_ustr, xPara->getString());
    // Even though not specified, Table-Style distributes the properties in DocDefault. DocDefault fontsize is 8pt.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compat mode has 8pt font size", 8.f, getProperty<float>(getRun(xPara,1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Normal has 0pt space below paragraph", sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table sets 10pt space above paragraph", sal_Int32(353), getProperty<sal_Int32>(xPara, u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Table style sets 0 right margin", sal_Int32(0), getProperty<sal_Int32>(xPara, u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Table sets 2.5 line-spacing", sal_Int16(250), getProperty<style::LineSpacing>(xPara, u"ParaLineSpacing"_ustr).Height, 1);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Paragraph background color in cell A2", sal_Int32(-1), getProperty<sal_Int32>(xPara, u"ParaBackColor"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Compat mode overrides left adjust", style::ParagraphAdjust_RIGHT,
                                 static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118947_tableStyle2, "tdf118947_tableStyle2.docx")
{
    uno::Reference<text::XTextTable> xTable(getParagraphOrTable(1), uno::UNO_QUERY);
    // This cell is affected by compatSetting overrideTableStyleFontSizeAndJustification=1 (no goofy exception)
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A2"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(u"Notice that this is 12pt font, left aligned in non-compatibility mode."_ustr, xPara->getString());
    // Even though not specified, Table-Style tries to distribute the properties in DocDefault. DocDefault fontsize is 8pt.
    // However, this is overridden by the default style's specified fontsize of 12 and left justify.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non-Compat mode has 12pt font size", 12.f, getProperty<float>(getRun(xPara,1), u"CharHeight"_ustr));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Non-Compat mode keeps the style's left adjust", style::ParagraphAdjust_LEFT,
                                 static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(xPara, u"ParaAdjust"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, tdf123912_protectedForm)
{
    loadAndReload("tdf123912_protectedForm.odt");
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
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Section1 is protected", false, getProperty<bool>(xSect, u"IsProtected"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(tdf124600b, "tdf124600b.docx")
{
    // <wp:anchor allowOverlap="0"> was lost on roundtrip, we always wrote "1" on export.
    bool bAllowOverlap1 = getProperty<bool>(getShape(1), u"AllowOverlap"_ustr);
    CPPUNIT_ASSERT(!bAllowOverlap1);
    bool bAllowOverlap2 = getProperty<bool>(getShape(2), u"AllowOverlap"_ustr);
    CPPUNIT_ASSERT(!bAllowOverlap2);
}

CPPUNIT_TEST_FIXTURE(Test, testDateControl)
{
    loadAndReload("empty-date-control.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Check that we exported the empty date control correctly
    // Date form field is converted to date content control.

    uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(1), 1), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    bool bDate{};
    xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
    CPPUNIT_ASSERT(bDate);

    OUString sDateFormat;
    xContentControlProps->getPropertyValue(u"DateFormat"_ustr) >>= sDateFormat;

    OUString sLang;
    xContentControlProps->getPropertyValue(u"DateLanguage"_ustr) >>= sLang;

    OUString sCurrentDate;
    xContentControlProps->getPropertyValue(u"CurrentDate"_ustr) >>= sCurrentDate;

    CPPUNIT_ASSERT_EQUAL(u"dd/MM/yyyy"_ustr, sDateFormat);
    CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, sLang);
    CPPUNIT_ASSERT_EQUAL(u""_ustr, sCurrentDate);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121867)
{
    loadAndReload("tdf121867.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwEditShell* pEditShell = pTextDoc->GetDocShell()->GetEditShell();
    CPPUNIT_ASSERT(pEditShell);
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
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(1), u"ParaAdjust"_ustr)));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_BLOCK,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(1), u"ParaLastLineAdjust"_ustr)));

    CPPUNIT_ASSERT_EQUAL(
        style::ParagraphAdjust_BLOCK,
        static_cast<style::ParagraphAdjust>(getProperty<sal_Int16>(getParagraph(2), u"ParaAdjust"_ustr)));
    CPPUNIT_ASSERT_EQUAL(style::ParagraphAdjust_LEFT,
                         static_cast<style::ParagraphAdjust>(
                             getProperty<sal_Int16>(getParagraph(2), u"ParaLastLineAdjust"_ustr)));
}

CPPUNIT_TEST_FIXTURE(Test, testInputListExport)
{
    loadAndReload("tdf122186_input_list.odt");
    if (!isExported()) // importing the ODT, an input field
    {
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields());
        uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Any aField = xFields->nextElement();
        uno::Reference<lang::XServiceInfo> xServiceInfo(aField, uno::UNO_QUERY);
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.textfield.DropDown"_ustr));
    }
    else // importing the DOCX, a content control
    {
        uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(1), 1), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValues> aListItems;
        xContentControlProps->getPropertyValue(u"ListItems"_ustr) >>= aListItems;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), aListItems.getLength());
        comphelper::SequenceAsHashMap aMap0(aListItems[0]);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, aMap0[u"Value"_ustr].get<OUString>());
        comphelper::SequenceAsHashMap aMap1(aListItems[1]);
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, aMap1[u"Value"_ustr].get<OUString>());
        comphelper::SequenceAsHashMap aMap2(aListItems[2]);
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, aMap2[u"Value"_ustr].get<OUString>());
        uno::Reference<container::XEnumerationAccess> xContentEnumAccess(xContentControl, uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xContentEnum = xContentEnumAccess->createEnumeration();
        uno::Reference<text::XTextRange> xContent(xContentEnum->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xContent->getString());
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

CPPUNIT_TEST_FIXTURE(Test, testTdf116371)
{
    loadAndReload("tdf116371.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Make sure the rotation is exported correctly, and size not distorted
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4700.0, getProperty<double>(xShape, u"RotateAngle"_ustr), 10);
    auto frameRect = getProperty<awt::Rectangle>(xShape, u"FrameRect"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24063), frameRect.Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(24179), frameRect.Width);
}

CPPUNIT_TEST_FIXTURE(Test, testFrameSizeExport)
{
    loadAndSave("floating-tables-anchor.docx");
    // Make sure the table width is 4000
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tblPr/w:tblW"_ostr, "w"_ostr, u"4000"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf119201, "tdf119201.docx")
{
    // Visibility of shapes wasn't imported/exported, for now base printable property on that, too
    auto xShape(getShape(1));
    CPPUNIT_ASSERT_MESSAGE("First shape should be visible.", getProperty<bool>(xShape, u"Visible"_ustr));
    CPPUNIT_ASSERT_MESSAGE("First shape should be printable.", getProperty<bool>(xShape, u"Printable"_ustr));
    xShape = getShapeByName(u"Rectangle 1");
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be visible.", !getProperty<bool>(xShape, u"Visible"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be printable.", !getProperty<bool>(xShape, u"Printable"_ustr));
    xShape = getShapeByName(u"Oval 2");
    CPPUNIT_ASSERT_MESSAGE("Third shape should be visible.", getProperty<bool>(xShape, u"Visible"_ustr));
    CPPUNIT_ASSERT_MESSAGE("Third shape should be printable.", getProperty<bool>(xShape, u"Printable"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf156484, "tdf156484.docx")
{
    auto xShape(getShape(1));
    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT_MESSAGE("First shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("First shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
    xShape = getShape(2);
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Second shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
    xShape = getShape(3);
    CPPUNIT_ASSERT_MESSAGE("Third shape should not be visible.", !getProperty<bool>(xShape, "Visible"));
    CPPUNIT_ASSERT_MESSAGE("Third shape should not be printable.", !getProperty<bool>(xShape, "Printable"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf124594, "tdf124594.docx")
{
    xmlDocUniquePtr pDump = parseLayoutDump();
    // Without the accompanying fix in place, this test would have failed, as the portion text was
    // only "Er horte leise Schritte hinter", which means the 1st line of the 2nd paragraph was
    // split into two by a Special portion, i.e. the top margin of the shape was too large.
    assertXPath(pDump, "/root/page/body/txt[2]/SwParaPortion/SwLineLayout[1]/SwLinePortion[1]"_ostr, "portion"_ostr,
                u"Er horte leise Schritte hinter sich. Das bedeutete nichts Gutes. Wer wu"_ustr); // ... until the bookmark.
}

CPPUNIT_TEST_FIXTURE(Test, testTextInput)
{
    loadAndSave("textinput.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);


    // test the exported DOCX

    // no hint, empty
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[3]/w:instrText"_ostr, u" FILLIN \"\""_ustr);
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[5]/w:rPr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[6]/w:fldChar"_ostr, "fldCharType"_ostr, u"end"_ustr);

    // no hint, content
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[3]/w:instrText"_ostr, u" FILLIN \"\""_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[5]/w:t"_ostr, u"content without hint"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[6]/w:fldChar"_ostr, "fldCharType"_ostr, u"end"_ustr);

    // hint, empty
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[3]/w:instrText"_ostr, u" FILLIN \"hint empty\""_ustr);
    assertXPathChildren(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[5]"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[5]/w:rPr"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[6]/w:fldChar"_ostr, "fldCharType"_ostr, u"end"_ustr);

    // hint, content
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[3]/w:instrText"_ostr, u" FILLIN \"hint content\""_ustr);
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[5]/w:t"_ostr, u"content with hint"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:r[6]/w:fldChar"_ostr, "fldCharType"_ostr, u"end"_ustr);

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
        CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.textfield.Input"_ustr));
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
        CPPUNIT_ASSERT_EQUAL(uno::Any(sContent), xPropertySet->getPropertyValue(u"Content"_ustr));
        CPPUNIT_ASSERT_EQUAL(sContent, xText->getAnchor()->getString());
        CPPUNIT_ASSERT_EQUAL(uno::Any(sHint), xPropertySet->getPropertyValue(u"Hint"_ustr));
        nElements++;
    }
    while (xFields->hasMoreElements());
    CPPUNIT_ASSERT_EQUAL(4, nElements);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123460, "tdf123460.docx")
{
    // check paragraph mark deletion at terminating moveFrom
    CPPUNIT_ASSERT(getParagraph( 2 )->getString().startsWith("Nunc"));
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(getParagraph( 2 ), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL( u""_ustr, xRun->getString());
    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(hasProperty(xRun, u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Delete"_ustr,getProperty<OUString>(xRun, u"RedlineType"_ustr));
    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xRun->getString().endsWith("tellus."));
    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(hasProperty(xRun, u"Bookmark"_ustr));

    // The paragraph marker's formatting.
    xRun.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,getProperty<OUString>(xRun, u"TextPortionType"_ustr));
    CPPUNIT_ASSERT(xRun->getString().isEmpty());

    // deleted paragraph mark at the end of the second paragraph
    if (isExported())
    {
        // there is no run after the MoveBookmark
        CPPUNIT_ASSERT(!xRunEnum->hasMoreElements());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146140)
{
    loadAndSave("tdf123460.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    // This was 1 (put end of paragraph of the previous moveFrom into a w:del,
    // resulting double deletions at the same position, which is an
    // ODT back-compatibility issue described in tdf#107292)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:del"_ostr, 0);
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:moveFrom"_ostr, 1);
}

//tdf#125298: fix charlimit restrictions in bookmarknames and field references if they contain non-ascii characters
CPPUNIT_TEST_FIXTURE(Test, testTdf125298)
{
    loadAndSave("tdf125298_crossreflink_nonascii_charlimit.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // check whether test file keeps non-ascii values or not
    OUString bookmarkName1 = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart[1]"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e1rv\u00edzt\u0171r\u0151_t\u00fck\u00f6rf\u00far\u00f3g\u00e9p"), bookmarkName1);

    OUString bookmarkName2 = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:bookmarkStart[1]"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("\u00e91\u00e12\u01713\u01514\u00fa5\u00f66\u00fc7\u00f38\u00ed9"), bookmarkName2);
    OUString fieldName1 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[5]/w:r[2]/w:instrText[1]"_ostr);
    OUString expectedFieldName1 = " REF " + bookmarkName1 + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName1, fieldName1);
    OUString fieldName2 = getXPathContent(pXmlDoc, "/w:document/w:body/w:p[7]/w:r[2]/w:instrText[1]"_ostr);
    OUString expectedFieldName2 = " REF " + bookmarkName2 + " \\h ";
    CPPUNIT_ASSERT_EQUAL(expectedFieldName2, fieldName2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121784, "tdf121784.docx")
{
    // check tracked insertion of footnotes
    CPPUNIT_ASSERT_EQUAL( u"Text1"_ustr, getParagraph( 1 )->getString());
    CPPUNIT_ASSERT_EQUAL( u""_ustr, getRun( getParagraph( 1 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 2), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr,getProperty<OUString>(getRun(getParagraph(1), 2), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"1"_ustr, getRun( getParagraph( 1 ), 3 )->getString());

    // check tracked insertion of endnotes
    CPPUNIT_ASSERT_EQUAL( u"texti"_ustr, getParagraph( 2 )->getString());
    CPPUNIT_ASSERT_EQUAL( u""_ustr, getRun( getParagraph( 2 ), 2 )->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 2), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr,getProperty<OUString>(getRun(getParagraph(2), 2), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL( u"i"_ustr, getRun( getParagraph( 2 ), 3 )->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTbrlFrameVml, "tbrl-frame-vml.docx")
{
    uno::Reference<beans::XPropertySet> xTextFrame(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTextFrame.is());

    if (isExported())
    {
        // DML import: creates a TextBox, eaVert read back as TB_RL in TextWritingMode

        auto eMode = getProperty<text::WritingMode>(xTextFrame, u"TextWritingMode"_ustr);
        CPPUNIT_ASSERT_EQUAL(text::WritingMode::WritingMode_TB_RL, eMode);
    }
    else
    {
        // VML import: creates a TextFrame.

        auto nActual = getProperty<sal_Int16>(xTextFrame, u"WritingMode"_ustr);
        // Without the accompanying fix in place, this test would have failed with 'Expected: 2; Actual:
        // 4', i.e. writing direction was inherited from page, instead of explicit tbrl.
        CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL, nActual);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119037)
{
    loadAndSave("tdf119037.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"single"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"single"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"single"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"single"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"dotted"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"dotted"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"dotted"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"dotted"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"dashed"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"dashed"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"dashed"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"dashed"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"dashSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"dashSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"dashSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"dashSmallGap"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"dotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"dotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"dotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"dotDash"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"dotDotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"dotDotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"dotDotDash"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"dotDotDash"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"double"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"double"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"thinThickSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"thinThickSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"thinThickSmallGap"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[17]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"thinThickSmallGap"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:top"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:left"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:right"_ostr, "val"_ostr, u"double"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[19]/w:pPr/w:pBdr/w:bottom"_ostr, "val"_ostr, u"double"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125657)
{
    loadAndSave("tdf125657.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    auto checkAttrIsInt = [&](const OString& sAttrName) {
        OUString sAttr = getXPath(pXmlDoc,
                                  "/w:document/w:body/w:p[1]/w:r[1]/w:drawing/wp:inline/a:graphic/"
                                  "a:graphicData/pic:pic/pic:blipFill/a:srcRect"_ostr,
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
    checkAttrIsInt("l"_ostr);
    checkAttrIsInt("t"_ostr);
    checkAttrIsInt("r"_ostr);
    checkAttrIsInt("b"_ostr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf125324, "tdf125324.docx")
{
    discardDumpedLayout();
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    assertXPath(pXmlDoc, "/root/page/body/txt[2]/anchored/fly/tab/infos/bounds"_ostr, "top"_ostr, u"4193"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf78657)
{
    loadAndSave("tdf78657_picture_hyperlink.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    xmlDocUniquePtr pXmlRels = parseExport(u"word/_rels/document.xml.rels"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/wp:docPr/a:hlinkClick"_ostr, 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:drawing/wp:inline/a:graphic/a:graphicData/pic:pic/pic:nvPicPr/pic:cNvPr/a:hlinkClick"_ostr, 1);
    assertXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Target='http://www.google.com']"_ostr, "TargetMode"_ostr, u"External"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testBtlrFrame)
{
    loadAndReload("btlr-frame.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    uno::Reference<beans::XPropertySet> xPropertySet(getShape(1), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // -270; Actual: 0', i.e. the writing direction of the frame was lost.
    // Note: Implementation was changed to use WritingMode property instead of TextPreRotateAngle.
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR,
                         getProperty<sal_Int16>(xPropertySet, u"WritingMode"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125518)
{
    loadAndSave("tdf125518.odt");
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);


    // First diagram is anchored
    OUString anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:drawing/wp:anchor/wp:docPr"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"Object1"_ustr, anchorName);

    // Second diagram has anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r[1]/w:drawing/wp:anchor/wp:docPr"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"Objekt1"_ustr, anchorName);

    // Third diagram has no anchor
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:r[2]/w:drawing/wp:inline/wp:docPr"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"Object2"_ustr, anchorName);

    // 4th diagram has anchor too
    anchorName = getXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:r[3]/w:drawing/wp:anchor/wp:docPr"_ostr, "name"_ostr);
    CPPUNIT_ASSERT_EQUAL(u"Object3"_ustr, anchorName);
}

DECLARE_OOXMLEXPORT_TEST(testImageCommentAtChar, "image-comment-at-char.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 1), u"TextPortionType"_ustr));
    // Without the accompanying fix in place, this test would have failed with 'Expected:
    // Annotation; Actual: Frame', i.e. the comment start before the image was lost.
    CPPUNIT_ASSERT_EQUAL(u"Annotation"_ustr,
                         getProperty<OUString>(getRun(xPara, 2), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Frame"_ustr,
                         getProperty<OUString>(getRun(xPara, 3), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"AnnotationEnd"_ustr,
                         getProperty<OUString>(getRun(xPara, 4), u"TextPortionType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Text"_ustr,
                         getProperty<OUString>(getRun(xPara, 5), u"TextPortionType"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131594)
{
    loadAndSave("tdf131594.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // lnNumType should not be exported if w:countBy="0"
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:lnNumType"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121663)
{
    loadAndSave("tdf121663.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // auto distance of line numbering is 0.5 cm
    assertXPath(pXmlDoc, "//w:lnNumType"_ostr, "distance"_ostr, u"283"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testInvalidDateFormField, "invalid_date_form_field.docx")
{

    uno::Reference<container::XEnumerationAccess> xParagraph(getParagraph(1), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();

    int nIndex = 0;
    while (xPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        if (aPortionType != "ContentControl")
        {
            continue;
        }

        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        bool bDate{};
        xContentControlProps->getPropertyValue(u"Date"_ustr) >>= bDate;
        CPPUNIT_ASSERT(bDate);

        // Check date content control's parameters.
        OUString sDateFormat;
        xContentControlProps->getPropertyValue(u"DateFormat"_ustr) >>= sDateFormat;

        OUString sLang;
        xContentControlProps->getPropertyValue(u"DateLanguage"_ustr) >>= sLang;

        OUString sCurrentDate;
        xContentControlProps->getPropertyValue(u"CurrentDate"_ustr) >>= sCurrentDate;

        // The first one has invalid date format (invalid = LO can't parse it)
        if(nIndex == 0)
        {

            CPPUNIT_ASSERT_EQUAL(u"YYYY.MM.DDT00:00:00Z"_ustr, sDateFormat);
            CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, sLang);
            CPPUNIT_ASSERT_EQUAL(u""_ustr, sCurrentDate);

        }
        else if (nIndex == 1) // The second has wrong date
        {
            CPPUNIT_ASSERT_EQUAL(u"MM/DD/YY"_ustr, sDateFormat);
            CPPUNIT_ASSERT_EQUAL(u"en-US"_ustr, sLang);
            CPPUNIT_ASSERT_EQUAL(u"2019.06.34T00:00:00Z"_ustr, sCurrentDate);

        }
        else // The third one has wrong local
        {
            CPPUNIT_ASSERT_EQUAL(u"[NatNum12 MMMM=abbreviation]YYYY\". \"MMMM D."_ustr, sDateFormat);
            CPPUNIT_ASSERT_EQUAL(u"xxxx"_ustr, sLang);
            CPPUNIT_ASSERT_EQUAL(u"2019.06.11T00:00:00Z"_ustr, sCurrentDate);

        }
        ++nIndex;
    }
    CPPUNIT_ASSERT_EQUAL(int(3), nIndex);
}

DECLARE_OOXMLEXPORT_TEST(tdf127085, "tdf127085.docx")
{
    // Fill transparency was lost during export
    uno::Reference<beans::XPropertySet> xShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(50), getProperty<sal_Int16>(xShape, u"FillTransparence"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(tdf119809, "tdf119809.docx")
{
    // Combobox without an item list lost during import
    if (getShapes() > 0)
    {
        uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
        uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.ComboBox"_ustr)));
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<OUString> >(xPropertySet, u"StringItemList"_ustr).getLength());
    }
    else
    {
        // DropDown was imported as content control
        // First run: bookmark
        uno::Reference<beans::XPropertySet> xTextPortion(getRun(getParagraph(1), 2), uno::UNO_QUERY);
        OUString aPortionType;
        xTextPortion->getPropertyValue(u"TextPortionType"_ustr) >>= aPortionType;
        CPPUNIT_ASSERT_EQUAL(u"ContentControl"_ustr, aPortionType);
        uno::Reference<text::XTextContent> xContentControl;
        xTextPortion->getPropertyValue(u"ContentControl"_ustr) >>= xContentControl;
        uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValues> aListItems;
        xContentControlProps->getPropertyValue(u"ListItems"_ustr) >>= aListItems;
        CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), aListItems.getLength());
    }
}

DECLARE_OOXMLEXPORT_TEST(tdf118169, "tdf118169.docx")
{
    // Unicode characters were converted to question marks.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(true, bool(xServiceInfo->supportsService(u"com.sun.star.form.component.CheckBox"_ustr)));
    CPPUNIT_ASSERT_EQUAL(u"őőőőőőőőőőőűűűű"_ustr, getProperty<OUString>(xPropertySet, u"Label"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127116)
{
    loadAndSave("tdf127116.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);


    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127339)
{
    loadAndSave("tdf127339.docx");
    xmlDocUniquePtr pXmlRels = parseExport(u"word/_rels/document.xml.rels"_ustr);

    assertXPathNoAttribute(pXmlRels, "/rels:Relationships/rels:Relationship[@Target='#bookmark']"_ostr, "TargetMode"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127362)
{
    loadAndSave("tdf127362.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    xmlDocUniquePtr pXmlRels = parseExport(u"word/_rels/document.xml.rels"_ustr);

    OUString bookmarkName = "#" + getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlRels, "/rels:Relationships/rels:Relationship[@Id='rId3']"_ostr, "Target"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127605)
{
    loadAndSave("tdf127605.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127732)
{
    loadAndSave("internal_hyperlink_frame.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/mc:AlternateContent/mc:Fallback/w:pict/v:rect/v:textbox/w:txbxContent/w:p/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127733)
{
    loadAndSave("internal_hyperlink_ole.odt");
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);


    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127734)
{
    loadAndSave("internal_hyperlink_region.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127735)
{
    loadAndSave("internal_hyperlink_table.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    OUString bookmarkName = getXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:tc[1]/w:p/w:bookmarkStart"_ostr, "name"_ostr);
    OUString anchor = getXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink"_ostr, "anchor"_ostr);
    CPPUNIT_ASSERT_EQUAL(anchor, bookmarkName);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf123628)
{
    loadAndSave("tdf123628.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:hyperlink/w:r/w:rPr/w:rStyle"_ostr, "val"_ostr, u"Hyperlink"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='Hyperlink']/w:name"_ostr, "val"_ostr, u"Hyperlink"_ustr);
}

DECLARE_OOXMLEXPORT_TEST(testTdf127741, "tdf127741.docx")
{
    uno::Reference<text::XTextRange> xPara = getParagraph(1);
    uno::Reference<beans::XPropertySet> xRun(getRun(xPara,1), uno::UNO_QUERY);
    OUString unVisitedStyleName = getProperty<OUString>(xRun, u"UnvisitedCharStyleName"_ustr);
    CPPUNIT_ASSERT(unVisitedStyleName.equalsIgnoreAsciiCase("Internet Link"));
    OUString visitedStyleName = getProperty<OUString>(xRun, u"VisitedCharStyleName"_ustr);
    CPPUNIT_ASSERT(visitedStyleName.equalsIgnoreAsciiCase("Visited Internet Link"));
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142693_hugePaperSizeImport)
{
    loadAndSave("tdf142693_hugePaperSizeImport.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgSz"_ostr, "w"_ostr, u"90369"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:pgSz"_ostr, "h"_ostr, u"104372"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127925)
{
    loadAndSave("tdf127925.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlStyles = parseExport(u"word/styles.xml"_ustr);
    assertXPath(pXmlStyles, "/w:styles/w:style[@w:styleId='FollowedHyperlink']/w:name"_ostr, "val"_ostr, u"FollowedHyperlink"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf127579)
{
    loadAndSave("tdf127579.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:hyperlink/w:r/w:rPr/w:rStyle"_ostr, "val"_ostr, u"Hyperlink"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128304)
{
    loadAndReload("tdf128304.odt");
    CPPUNIT_ASSERT_EQUAL(4, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    css::text::WritingMode eMode;
    uno::Reference<beans::XPropertySet> xProps1(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps1->getPropertyValue(u"TextWritingMode"_ustr) >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps2(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps2->getPropertyValue(u"TextWritingMode"_ustr) >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps3(getShape(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps3->getPropertyValue(u"TextWritingMode"_ustr) >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);

    uno::Reference<beans::XPropertySet> xProps4(getShape(4), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xProps4->getPropertyValue(u"TextWritingMode"_ustr) >>= eMode);
    CPPUNIT_ASSERT_EQUAL(css::text::WritingMode::WritingMode_TB_RL, eMode);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
