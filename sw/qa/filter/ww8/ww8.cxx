/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>

#include <docsh.hxx>
#include <formatcontentcontrol.hxx>
#include <wrtsh.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <frameformats.hxx>
#include <formatflysplit.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <ftnfrm.hxx>
#include <IDocumentSettingAccess.hxx>
#include <sortedobjs.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <ftnidx.hxx>
#include <tabfrm.hxx>
#include <cntfrm.hxx>
#include <colfrm.hxx>
#include <fmtftntx.hxx>

namespace
{
/**
 * Covers sw/source/filter/ww8/ fixes.
 *
 * Note that these tests are meant to be simple: either load a file and assert some result or build
 * a document model with code, export and assert that result.
 *
 * Keep using the various sw_<format>import/export suites for multiple filter calls inside a single
 * test.
 */
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/filter/ww8/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorderDocImport)
{
    // Given a document with a border distance that is larger than the margin, when loading that
    // document:
    createSwDoc("negative-page-border.doc");

    // Then make sure we map that to a negative border distance (move border from the edge of body
    // frame towards the center of the page, not towards the edge of the page):
    uno::Reference<container::XNameAccess> xStyleFamily = getStyles(u"PageStyles"_ustr);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 501
    // - Actual  : 342
    // i.e. the border properties influenced the margin, which was 284 twips in the sprmSDyaTop
    // SPRM.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue(u"TopBorder"_ustr).get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue(u"TopBorderDistance"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}

CPPUNIT_TEST_FIXTURE(Test, testPlainTextContentControlExport)
{
    // Given a document with a plain text content control around a text portion:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xMSF(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"test"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->gotoEnd(/*bExpand=*/true);
    uno::Reference<text::XTextContent> xContentControl(
        xMSF->createInstance(u"com.sun.star.text.ContentControl"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    xContentControlProps->setPropertyValue(u"PlainText"_ustr, uno::Any(true));
    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    // When exporting to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:text' number of nodes is incorrect
    // i.e. the plain text content control was turned into a rich text one on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:text"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxComboBoxContentControlExport)
{
    // Given a document with a combo box content control around a text portion:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::COMBO_BOX);

    // When exporting to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:sdt/w:sdtPr/w:comboBox' number of nodes is incorrect
    // i.e. the combo box content control was turned into a drop-down one on export.
    assertXPath(pXmlDoc, "//w:sdt/w:sdtPr/w:comboBox"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxHyperlinkShape)
{
    // Given a document with a hyperlink at char positions 0 -> 6 and a shape with text anchored at
    // char position 6:
    createSwDoc();
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xText->insertString(xCursor, u"beforeafter"_ustr, /*bAbsorb=*/false);
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->goRight(/*nCount=*/6, /*bExpand=*/true);
    uno::Reference<beans::XPropertySet> xCursorProps(xCursor, uno::UNO_QUERY);
    xCursorProps->setPropertyValue(u"HyperLinkURL"_ustr, uno::Any(u"http://www.example.com/"_ustr));
    xCursor->gotoStart(/*bExpand=*/false);
    xCursor->goRight(/*nCount=*/6, /*bExpand=*/false);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(5000, 5000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"AnchorType"_ustr,
                                  uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<text::XTextContent> xShapeContent(xShape, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xShapeContent, /*bAbsorb=*/false);
    xShapeProps->setPropertyValue(u"TextBox"_ustr, uno::Any(true));

    // When saving this document to DOCX, then make sure we don't crash on export (due to an
    // assertion failure for not-well-formed XML output):
    save(u"Office Open XML Text"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxContentControlDropdownEmptyDisplayText)
{
    // Given a document with a dropdown content control, the only list item has no display text
    // (only a value):
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->InsertContentControl(SwContentControlType::DROP_DOWN_LIST);

    // When saving to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure that no display text attribute is written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//w:sdt/w:sdtPr/w:dropDownList/w:listItem' unexpected 'displayText' attribute
    // i.e. we wrote an empty attribute instead of omitting it.
    assertXPathNoAttribute(pXmlDoc, "//w:sdt/w:sdtPr/w:dropDownList/w:listItem"_ostr,
                           "displayText"_ostr);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxSymbolFontExport)
{
    // Create document with symbol character and font Wingdings
    mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();

    xText->insertString(xCursor, u""_ustr, true);

    uno::Reference<text::XTextRange> xRange = xCursor;
    uno::Reference<beans::XPropertySet> xTextProps(xRange, uno::UNO_QUERY);
    xTextProps->setPropertyValue(u"CharFontName"_ustr, uno::Any(u"Wingdings"_ustr));
    xTextProps->setPropertyValue(u"CharFontNameAsian"_ustr, uno::Any(u"Wingdings"_ustr));
    xTextProps->setPropertyValue(u"CharFontNameComplex"_ustr, uno::Any(u"Wingdings"_ustr));
    xTextProps->setPropertyValue(u"CharFontCharSet"_ustr, uno::Any(awt::CharSet::SYMBOL));

    // When exporting to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure the expected markup is used:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "//w:p/w:r/w:sym"_ostr, 1);
    assertXPath(pXmlDoc, "//w:p/w:r/w:sym[1]"_ostr, "font"_ostr, u"Wingdings"_ustr);
    assertXPath(pXmlDoc, "//w:p/w:r/w:sym[1]"_ostr, "char"_ostr, u"f0e0"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDocxFloatingTableExport)
{
    // Given a document with a floating table:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    // Insert a table:
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, 1, 1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    // Select it:
    pWrtShell->SelAll();
    // Wrap in a fly:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    // Mark it as a floating table:
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    auto pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();

    // When saving to docx:
    save(u"Office Open XML Text"_ustr);

    // Then make sure we write a floating table, not a textframe containing a table:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '//w:tbl/w:tblPr/w:tblpPr' number of nodes is incorrect
    // i.e. no floating table was exported.
    assertXPath(pXmlDoc, "//w:tbl/w:tblPr/w:tblpPr"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testDocFloatingTableImport)
{
    // Given a document with 2 pages:
    createSwDoc("floattable-compat14.doc");

    // When laying out that document:
    calcLayout();

    // Make sure that the table is split between page 1 and page 2:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    // Without the accompanying fix in place, this test would have failed, the fly frame was not
    // split between page 1 and page 2.
    CPPUNIT_ASSERT(pPage1->GetNext());
}

CPPUNIT_TEST_FIXTURE(Test, testWrapThroughLayoutInCell)
{
    // Given a document with a shape, "keep inside text boundaries" is off, wrap type is set to
    // "through":
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"AnchorType"_ustr,
                                  uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProps->setPropertyValue(u"Surround"_ustr, uno::Any(text::WrapTextMode_THROUGH));
    xShapeProps->setPropertyValue(u"HoriOrientRelation"_ustr,
                                  uno::Any(text::RelOrientation::FRAME));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // When saving to docx:
    save(u"Office Open XML Text"_ustr);

    // Then make sure that layoutInCell is undoing the effect of the import-time tweak:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - attribute 'layoutInCell' of '//wp:anchor' incorrect value.
    // i.e. layoutInCell was disabled, leading to bad layout in Word.
    assertXPath(pXmlDoc, "//wp:anchor"_ostr, "layoutInCell"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, test3Endnotes)
{
    // Given a DOC file with 3 endnotes:
    createSwDoc("3endnotes.doc");

    // When laying out that document:
    calcLayout();

    // Then make sure that all 3 endnotes are on the last page, like in Word:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    SwPageFrame* pPage = pLayout->GetLastPage();
    SwContentFrame* pLastContent = pPage->FindLastBodyContent();
    SwFrame* pSectionFrame = pLastContent->GetNext();
    auto pColumnFrame = pSectionFrame->GetLower()->DynCastColumnFrame();
    SwFootnoteContFrame* pFootnoteCont = pColumnFrame->FindFootnoteCont();
    int nEndnotes = 0;
    for (SwFrame* pLower = pFootnoteCont->GetLower(); pLower; pLower = pLower->GetNext())
    {
        ++nEndnotes;
    }
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 1
    // i.e. only 1 endnote was on the last page, the other 2 was not moved to the end of the
    // document, which is incorrect.
    CPPUNIT_ASSERT_EQUAL(3, nEndnotes);
}

CPPUNIT_TEST_FIXTURE(Test, testDoNotBreakWrappedTables)
{
    // Given a document with the DO_NOT_BREAK_WRAPPED_TABLES compat mode enabled:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
    rIDSA.set(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES, true);

    // When saving to docx:
    save(u"Office Open XML Text"_ustr);

    // Then make sure the compat flag is serialized:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/w:settings/w:compat/w:doNotBreakWrappedTables' number of nodes is incorrect
    // i.e. <w:doNotBreakWrappedTables> was not written.
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:doNotBreakWrappedTables"_ostr, 1);
}

CPPUNIT_TEST_FIXTURE(Test, testAllowTextAfterFloatingTableBreak)
{
    // Given a document with the ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK compat mode enabled:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
    rIDSA.set(DocumentSettingId::ALLOW_TEXT_AFTER_FLOATING_TABLE_BREAK, true);

    // When saving to docx:
    save(u"Office Open XML Text"_ustr);

    // Then make sure the compat flag is serialized:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '/w:settings/w:compat/w:compatSetting[@w:name='allowTextAfterFloatingTableBreak']' number of nodes is incorrect
    // i.e. the compat flag was lost on export.
    assertXPath(
        pXmlDoc,
        "/w:settings/w:compat/w:compatSetting[@w:name='allowTextAfterFloatingTableBreak']"_ostr,
        "val"_ostr, u"1"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCfDontBreakWrappedTables)
{
    // Given a document with fDontBreakWrappedTables:
    // When importing that document:
    createSwDoc("dont-break-wrapped-tables.doc");

    // Then make sure that the matching compat flag is set:
    SwDoc* pDoc = getSwDoc();
    IDocumentSettingAccess& rIDSA = pDoc->getIDocumentSettingAccess();
    bool bDontBreakWrappedTables = rIDSA.get(DocumentSettingId::DO_NOT_BREAK_WRAPPED_TABLES);
    // Without the accompanying fix in place, this test would have failed, the compat flag was not
    // set.
    CPPUNIT_ASSERT(bDontBreakWrappedTables);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCFloatingTableHiddenAnchor)
{
    // Given a document with a normal table and a floating table with a hidden anchor:
    createSwDoc("floattable-hidden-anchor.doc");

    // When laying out that document:
    xmlDocUniquePtr pLayout = parseLayoutDump();

    // Then make sure that both tables are visible:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the floating table was lost.
    assertXPath(pLayout, "//tab"_ostr, 2);
}

CPPUNIT_TEST_FIXTURE(Test, testDOCVerticalFlyOffset)
{
    // Given a document with 2 pages, a floating table on the first page and an inline table on the
    // second page:
    createSwDoc("floattable-vertical-fly-offset.doc");

    // When laying out that document:
    calcLayout();

    // Then make sure that the tables don't overlap:
    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = dynamic_cast<SwPageFrame*>(pLayout->Lower());
    CPPUNIT_ASSERT(pPage1);
    CPPUNIT_ASSERT(pPage1->GetSortedObjs());
    const SwSortedObjs& rPage1Objs = *pPage1->GetSortedObjs();
    // Page 1 has a floating table:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rPage1Objs.size());
    auto pPage2 = dynamic_cast<SwPageFrame*>(pPage1->GetNext());
    // Without the accompanying fix in place, this test would have failed, there was no second page.
    CPPUNIT_ASSERT(pPage2);
    SwFrame* pBody2 = pPage2->GetLower();
    SwFrame* pTable2 = pBody2->GetLower();
    CPPUNIT_ASSERT(pTable2);
    // Page 2 starts with an inline table:
    CPPUNIT_ASSERT(pTable2->IsTabFrame());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableThenFloattable)
{
    // Given a document that contains a floating table, immediately followed by an other floating
    // table:
    // When importing the document & laying it out:
    createSwDoc("floattable-then-floattable.doc");
    calcLayout();

    // Then make sure that the two floating table has different anchors:
    SwDoc* pDoc = getSwDoc();
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    auto pFly1 = rFlys[0];
    SwNodeOffset nFly1Anchor = pFly1->GetAttrSet().GetAnchor().GetAnchorContentNode()->GetIndex();
    auto pFly2 = rFlys[1];
    SwNodeOffset nFly2Anchor = pFly2->GetAttrSet().GetAnchor().GetAnchorContentNode()->GetIndex();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 42
    // - Actual  : 41
    // i.e. the two anchor positions were the same instead of first anchor followed by the second
    // anchor.
    CPPUNIT_ASSERT_EQUAL(nFly1Anchor + 1, nFly2Anchor);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableOverlapNeverDOCXExport)
{
    // Given a document with a floating table, overlap is not allowed:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert2(u"before table"_ustr);
    // Insert a table:
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/1, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    // Select table:
    pWrtShell->SelAll();
    // Wrap the table in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    // Allow the text frame to split:
    pWrtShell->StartAllAction();
    auto& rFlys = *pDoc->GetSpzFrameFormats();
    auto pFly = rFlys[0];
    SwAttrSet aSet(pFly->GetAttrSet());
    aSet.Put(SwFormatFlySplit(true));
    // Don't allow overlap:
    SwFormatWrapInfluenceOnObjPos aInfluence;
    aInfluence.SetAllowOverlap(false);
    aSet.Put(aInfluence);
    pDoc->SetAttr(aSet, *pFly);
    pWrtShell->EndAllAction();

    // When saving to DOCX:
    save(u"Office Open XML Text"_ustr);

    // Then make sure that the overlap=never markup is written:
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // - XPath '//w:tblPr/w:tblOverlap' number of nodes is incorrect
    // i.e. <w:tblOverlap> was not written.
    assertXPath(pXmlDoc, "//w:tblPr/w:tblOverlap"_ostr, "val"_ostr, u"never"_ustr);
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableOverlapNeverDOCImport)
{
    // Given a document with two floating tables, the second has sprmTFNoAllowOverlap=1 set:
    // When importing that document:
    createSwDoc("floattable-tbl-overlap.doc");

    // Then make sure the second table is marked as "can't overlap":
    SwDoc* pDoc = getSwDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[1];
    // Without the accompanying fix in place, this test would have failed, the fly had the default
    // "can overlap".
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetWrapInfluenceOnObjPos().GetAllowOverlap());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableFootnote)
{
    // Given a document with a floating table and a footnote inside:
    // When importing that document:
    createSwDoc("floattable-footnote.doc");

    // Then make sure we both have a fly frame and a footnote:
    SwDoc* pDoc = getSwDoc();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFlys.size());
    SwFootnoteIdxs& rFootnotes = pDoc->GetFootnoteIdxs();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), rFootnotes.size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyInInlineTableDOC)
{
    // Outer inline table on pages 1 -> 2 -> 3, inner floating table on pages 2 -> 3:
    // When laying out that document:
    createSwDoc("floattable-in-inlinetable.doc");

    SwDoc* pDoc = getSwDoc();
    SwRootFrame* pLayout = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    auto pPage1 = pLayout->Lower()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage1);
    {
        SwFrame* pBody = pPage1->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(!pTab->GetPrecede());
        CPPUNIT_ASSERT(pTab->GetFollow());
    }
    auto pPage2 = pPage1->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage2);
    {
        SwFrame* pBody = pPage2->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(pTab->GetPrecede());
        // Without the accompanying fix in place, this test would have failed, the outer table was
        // missing on page 3.
        CPPUNIT_ASSERT(pTab->GetFollow());
    }
    auto pPage3 = pPage2->GetNext()->DynCastPageFrame();
    CPPUNIT_ASSERT(pPage3);
    {
        SwFrame* pBody = pPage3->FindBodyCont();
        auto pTab = pBody->GetLower()->DynCastTabFrame();
        CPPUNIT_ASSERT(pTab->GetPrecede());
        CPPUNIT_ASSERT(!pTab->GetFollow());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testNullPointerDereference)
{
    // Given a document with multiple pages:
    // When loading that document:
    // Without the accompanying fix in place, this test would have crashed due to null pointer access
    createSwDoc("null-pointer-dereference.doc");
    CPPUNIT_ASSERT_EQUAL(6, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testEndnotesAtSectEnd)
{
    // Given a document, endnotes at collected at section end:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->SplitNode();
    pWrtShell->Up(/*bSelect=*/false);
    pWrtShell->Insert("x");
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SwSectionData aSection(SectionType::Content, pWrtShell->GetUniqueSectionName());
    pWrtShell->StartAction();
    SfxItemSetFixed<RES_FTN_AT_TXTEND, RES_FRAMEDIR> aSet(pWrtShell->GetAttrPool());
    aSet.Put(SwFormatEndAtTextEnd(FTNEND_ATTXTEND));
    pWrtShell->InsertSection(aSection, &aSet);
    pWrtShell->EndAction();
    pWrtShell->InsertFootnote(OUString(), /*bEndNote=*/true);

    // When saving to DOCX:
    save("Office Open XML Text");

    // Then make sure the endnote position is section end:
    xmlDocUniquePtr pXmlDoc = parseExport("word/settings.xml");
    OUString aPos = getXPath(pXmlDoc, "/w:settings/w:endnotePr/w:pos"_ostr, "val"_ostr);
    // Without the accompanying fix in place, this test would have failed with:
    // - XPath '/w:settings/w:endnotePr/w:pos' number of nodes is incorrect
    // i.e. the default position was used: document end.
    CPPUNIT_ASSERT_EQUAL(OUString("sectEnd"), aPos);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
