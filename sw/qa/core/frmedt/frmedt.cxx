/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

#include <svx/svdpage.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>
#include <frameformats.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <swdtflvr.hxx>
#include <caption.hxx>
#include <view.hxx>
#include <formatflysplit.hxx>
#include <itabenum.hxx>
#include <frmmgr.hxx>
#include <UndoManager.hxx>

/// Covers sw/source/core/frmedt/ fixes.
class SwCoreFrmedtTest : public SwModelTestBase
{
public:
    SwCoreFrmedtTest()
        : SwModelTestBase(u"/sw/qa/core/frmedt/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testTextboxReanchor)
{
    // Load a document with a textframe and a textbox(shape+textframe).
    createSwDoc("textbox-reanchor.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SdrPage* pDrawPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pDrawShape = pDrawPage->GetObj(1);
    CPPUNIT_ASSERT_EQUAL(u"draw shape"_ustr, pDrawShape->GetName());

    // Select the shape of the textbox.
    Point aPoint;
    SwWrtShell* pShell = pDoc->GetDocShell()->GetWrtShell();
    pShell->SelectObj(aPoint, /*nFlag=*/0, pDrawShape);

    // Anchor the shape of the textbox into its own textframe.
    SdrObject* pTextFrameObj = pDrawPage->GetObj(2);
    SwFrameFormat* pTextFrameFormat = FindFrameFormat(pTextFrameObj);
    CPPUNIT_ASSERT_EQUAL(u"Frame2"_ustr, pTextFrameFormat->GetName());
    SwFrameFormat* pDrawShapeFormat = FindFrameFormat(pDrawShape);
    SwNodeOffset nOldAnchor = pDrawShapeFormat->GetAnchor().GetAnchorNode()->GetIndex();
    pShell->FindAnchorPos(pTextFrameObj->GetLastBoundRect().Center(), true);
    SwNodeOffset nNewAnchor = pDrawShapeFormat->GetAnchor().GetAnchorNode()->GetIndex();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 9
    // i.e. SwFEShell allowed to anchor the textframe of a textbox into itself.
    CPPUNIT_ASSERT_EQUAL(nOldAnchor, nNewAnchor);
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testVertPosFromBottomBoundingBox)
{
    // Insert a shape and anchor it vertically in a way, so its position is from the top of the page
    // bottom margin area.
    createSwDoc();
    uno::Reference<css::lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.RectangleShape"_ustr), uno::UNO_QUERY);
    xShape->setSize(awt::Size(10000, 10000));
    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
    xShapeProps->setPropertyValue(u"AnchorType"_ustr,
                                  uno::Any(text::TextContentAnchorType_AT_CHARACTER));
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, uno::Any(text::VertOrientation::NONE));
    xShapeProps->setPropertyValue(u"VertOrientRelation"_ustr,
                                  uno::Any(text::RelOrientation::PAGE_PRINT_AREA_BOTTOM));
    xShapeProps->setPropertyValue(u"VertOrientPosition"_ustr,
                                  uno::Any(static_cast<sal_Int32>(-11000)));
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    xDrawPageSupplier->getDrawPage()->add(xShape);

    // Get the absolute position of the top of the page bottom margin area.
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    SwTwips nPagePrintAreaBottom
        = getXPath(pXmlDoc, "//page/infos/prtBounds"_ostr, "bottom"_ostr).toInt32();

    // Calculate the allowed bounding box of the shape, e.g. the shape's position & size dialog uses
    // this to limit the vertical position to sensible values.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    SwRect aBoundRect;
    RndStdIds eAnchorType = RndStdIds::FLY_AT_CHAR;
    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    const auto& rFrameFormats = *pDoc->GetFrameFormats();
    const SwFormatAnchor* pFormatAhchor = &rFrameFormats[0]->GetAnchor();
    sal_Int16 eHoriRelOrient = text::RelOrientation::PAGE_FRAME;
    sal_Int16 eVertRelOrient = text::RelOrientation::PAGE_PRINT_AREA_BOTTOM;
    bool bFollowTextFlow = false;
    bool bMirror = false;
    Size aPercentSize;
    pWrtShell->CalcBoundRect(aBoundRect, eAnchorType, eHoriRelOrient, eVertRelOrient, pFormatAhchor,
                             bFollowTextFlow, bMirror, nullptr, &aPercentSize);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -14705
    // - Actual  : -1134
    // i.e. UI did not allow anchoring a shape 10cm above the bottom of the page due to wrong
    // bounding box.
    CPPUNIT_ASSERT_EQUAL(-1 * nPagePrintAreaBottom, aBoundRect.Pos().getY());
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testPasteFlyInTextBox)
{
    // Given a document that contains a textbox, which contains an sw image (fly frame)
    createSwDoc("paste-fly-in-textbox.docx");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SwDoc* pDoc = pDocShell->GetDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pDoc->GetSpzFrameFormats()->GetFormatCount());
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetSpzFrameFormats()->GetFormatCount());
    TransferableDataHelper aHelper(pTransfer);

    // When pasting that to an empty document.
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Then we should have the image only once: 3 formats (draw+fly formats for the textbox and a
    // fly format for the image).
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 3
    // - Actual  : 4
    // i.e. the image was pasted twice.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), pDoc->GetSpzFrameFormats()->GetFormatCount());
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testTextBoxSelectCursorPos)
{
    // Given a document with a fly+draw format pair (textbox):
    createSwDoc("paste-fly-in-textbox.docx");

    // When selecting the fly format:
    SwDoc* pDoc = getSwDoc();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    SdrObject* pFlyObject = pPage->GetObj(1);
    SwContact* pFlyContact = static_cast<SwContact*>(pFlyObject->GetUserCall());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_FLYFRMFMT), pFlyContact->GetFormat()->Which());
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SelectObj(Point(), 0, pFlyObject);

    // Then make sure the cursor is the anchor of the draw format:
    SdrObject* pDrawObject = pPage->GetObj(0);
    SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pDrawObject->GetUserCall());
    SwFrameFormat* pDrawFormat = pDrawContact->GetFormat();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(RES_DRAWFRMFMT), pDrawFormat->Which());
    SwNodeOffset nAnchor = pDrawFormat->GetAnchor().GetContentAnchor()->GetNode().GetIndex();
    SwNodeOffset nCursor = pWrtShell->GetCurrentShellCursor().GetPointNode().GetIndex();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 15 (anchor of draw format)
    // - Actual  : 6 (in-fly-format position)
    // i.e. the cursor had a broken position after trying to select the fly format.
    CPPUNIT_ASSERT_EQUAL(nAnchor, nCursor);
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testSplitFlyInsertCaption)
{
    // Given a document with a full-page floating table:
    createSwDoc("floating-table-caption.docx");

    // When trying to insert a caption below that table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->GotoTable(u"Table1"_ustr);
    InsCaptionOpt aOpt;
    SwView& rView = pWrtShell->GetView();
    aOpt.SetCategory(u"Table"_ustr);
    aOpt.SetCaption(u"Numbers English-German"_ustr);
    // After, not before.
    aOpt.SetPos(1);
    // Without the accompanying fix in place, this call never finished, layout didn't handle content
    // after the table in a floating table.
    rView.InsertCaption(&aOpt);

    // Then make sure the insertion finishes and now this is just a plain table-in-frame:
    SwDoc* pDoc = getSwDoc();
    sw::SpzFrameFormats& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    CPPUNIT_ASSERT(!pFly->GetAttrSet().GetFlySplit().GetValue());
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testSplitFlyUnfloat)
{
    // Given a document with a floating table:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    CPPUNIT_ASSERT(pDoc->GetUndoManager().IsUndoEnabled());
    pDoc->GetUndoManager().EnableUndo(false);
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlyFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT(rFlyFormats.empty());
    // Insert a table:
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwInsertTableOptions aTableOptions(SwInsertTableFlags::DefaultBorder, 0);
    pWrtShell->InsertTable(aTableOptions, /*nRows=*/2, /*nCols=*/1);
    pWrtShell->MoveTable(GotoPrevTable, fnTableStart);
    pWrtShell->GoPrevCell();
    pWrtShell->Insert(u"A1"_ustr);
    pWrtShell->GoNextCell();
    pWrtShell->Insert(u"A2"_ustr);
    // Select cell:
    pWrtShell->SelAll();
    // Select table:
    pWrtShell->SelAll();
    // Wrap the table in a text frame:
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(RndStdIds::FLY_AT_PARA, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    CPPUNIT_ASSERT(!rFlyFormats.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));
    pDoc->GetUndoManager().EnableUndo(true);

    // When marking that frame and unfloating it:
    selectShape(1);
    pWrtShell->UnfloatFlyFrame();

    // Then make sure the frame is removed, but the table is still part of the document:
    // Without the accompanying fix in place (empty SwFEShell::UnfloatFlyFrame()), this test would
    // have failed, the frame was not removed.
    CPPUNIT_ASSERT(rFlyFormats.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));

    // When undoing the conversion to inline:
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetUndoManager().GetUndoActionCount());
    pDoc->GetUndoManager().Undo();

    // Then the undo stack had 2 undo actions and undo-all crashed.
    CPPUNIT_ASSERT(!rFlyFormats.empty());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetTableFrameFormatCount(/*bUsed=*/true));
}

CPPUNIT_TEST_FIXTURE(SwCoreFrmedtTest, testInsertOnGrfNodeAsChar)
{
    // Given a selected as-char image:
    createSwDoc();
    SwDoc* pDoc = getSwDocShell()->GetDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    {
        SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
        SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
        aFrameSet.Put(aAnchor);
        Graphic aGrf;
        pWrtShell->SwFEShell::Insert(OUString(), OUString(), &aGrf, &aFrameSet);
    }

    // When inserting another as-char image:
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AS_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGrf;
    // Without the accompanying fix in place, this call crashed, we try to set a graphic node as an
    // anchor of an as-char image (which should be a text node).
    pWrtShell->SwFEShell::Insert(OUString(), OUString(), &aGrf, &aFrameSet);

    // Then make sure that the anchor of the second image is next to the first anchor:
    CPPUNIT_ASSERT(pDoc->GetSpzFrameFormats());
    sw::FrameFormats<sw::SpzFrameFormat*>& rFormats = *pDoc->GetSpzFrameFormats();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), rFormats.size());
    const sw::SpzFrameFormat& rFormat1 = *rFormats[0];
    const SwPosition* pAnchor1 = rFormat1.GetAnchor().GetContentAnchor();
    const sw::SpzFrameFormat& rFormat2 = *rFormats[1];
    const SwPosition* pAnchor2 = rFormat2.GetAnchor().GetContentAnchor();
    CPPUNIT_ASSERT_EQUAL(pAnchor1->nNode, pAnchor2->nNode);
    CPPUNIT_ASSERT_EQUAL(pAnchor1->GetContentIndex() + 1, pAnchor2->GetContentIndex());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
