/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <svx/svdpage.hxx>

#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <drawdoc.hxx>
#include <wrtsh.hxx>
#include <frameformats.hxx>
#include <textboxhelper.hxx>

#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>

/// Covers sw/source/core/draw/ fixes.
class SwCoreDrawTest : public SwModelTestBase
{
public:
    SwCoreDrawTest()
        : SwModelTestBase(u"/sw/qa/core/draw/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(SwCoreDrawTest, testTextboxDeleteAsChar)
{
    // Load a document with an as-char shape in it that has a textbox and an image in it.
    createSwDoc("as-char-textbox.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SdrPage* pPage = pDoc->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0);
    sal_Int32 nActual = pPage->GetObjCount();
    // 3 objects on the draw page: a shape + fly frame pair and a Writer image.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), nActual);

    // Select the shape of the textbox and delete it.
    SdrObject* pObject = pPage->GetObj(0);
    pWrtShell->SelectObj(Point(), 0, pObject);
    pWrtShell->DelSelectedObj();
    nActual = pPage->GetObjCount();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 2
    // i.e. the fly frame of the shape and the inner Writer image was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nActual);
}

CPPUNIT_TEST_FIXTURE(SwCoreDrawTest, testTextboxUndoOrdNum)
{
    // Given a document with 5 frame formats:
    // - picture
    // - draw format + fly format and a picture in it
    // - picture
    createSwDoc("textbox-undo-ordnum.docx");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const auto& rFormats = *pDoc->GetSpzFrameFormats();
    // Test the state before del + undo.
    for (const auto& pFormat : rFormats)
    {
        const SwFrameFormat* pFlyFormat
            = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT);
        if (!pFlyFormat)
        {
            continue;
        }

        sal_Int32 nDrawOrdNum = pFormat->FindRealSdrObject()->GetOrdNum();
        sal_Int32 nFlyOrdNum = pFlyFormat->FindRealSdrObject()->GetOrdNum();
        CPPUNIT_ASSERT_EQUAL(nDrawOrdNum + 1, nFlyOrdNum);
    }

    // When selecting the first page, deleting the selection and undoing:
    pWrtShell->Down(true, 3);
    pWrtShell->DelLeft();
    pWrtShell->Undo();

    // Then the z-order of the fly format should be still the z-order of the draw format + 1, when
    // the fly and draw formats form a textbox pair.
    for (const auto& pFormat : rFormats)
    {
        const SwFrameFormat* pFlyFormat
            = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT);
        if (!pFlyFormat)
        {
            continue;
        }

        sal_Int32 nDrawOrdNum = pFormat->FindRealSdrObject()->GetOrdNum();
        sal_Int32 nFlyOrdNum = pFlyFormat->FindRealSdrObject()->GetOrdNum();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 4
        // - Actual  : 2
        // i.e. the fly format was behind the draw format, not visible.
        CPPUNIT_ASSERT_EQUAL(nDrawOrdNum + 1, nFlyOrdNum);
    }
}

CPPUNIT_TEST_FIXTURE(SwCoreDrawTest, testTdf107727FrameBorder)
{
    // Load a document with a textframe without border, one with only left border
    createSwDoc("tdf107727_FrameBorder.odt");

    // Export to RTF and reload
    saveAndReload(u"Rich Text Format"_ustr);

    // Get frame without border and inspect it.
    uno::Reference<text::XTextFramesSupplier> xTextFramesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xIndexAccess(xTextFramesSupplier->getTextFrames(),
                                                         uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame0(xIndexAccess->getByIndex(0), uno::UNO_QUERY);
    auto aBorder = getProperty<table::BorderLine2>(xFrame0, u"LeftBorder"_ustr);
    // fo:border="none" is not available via API, and aBorder.LineWidth has wrong value (why?).
    sal_uInt32 nBorderWidth
        = aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance;
    // Without patch it failed with Expected 0, Actual 26
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(0), nBorderWidth);

    // Get frame with left border and inspect it.
    uno::Reference<beans::XPropertySet> xFrame1(xIndexAccess->getByIndex(1), uno::UNO_QUERY);
    aBorder = getProperty<table::BorderLine2>(xFrame1, u"LeftBorder"_ustr);
    // Without patch it failed with Expected 127, Actual 26. Default border width was used.
    nBorderWidth = aBorder.OuterLineWidth + aBorder.InnerLineWidth + aBorder.LineDistance;
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(127), nBorderWidth);
    // Without patch it failed with Expected Color: R:0 G:0 B:255 A:0, Actual Color: R:0 G:0 B:0 A:0.
    // Default border color was used.
    CPPUNIT_ASSERT_EQUAL(COL_LIGHTBLUE, Color(ColorTransparency, aBorder.Color));
}

CPPUNIT_TEST_FIXTURE(SwCoreDrawTest, testSdtTextboxHeader)
{
    // Given a 2 page document, same header on both pages, content control in the header and
    // shape+fly pair (textbox) anchored in the same header
    // When loading that document, then make sure that layout doesn't fail with an assertion because
    // the "master SdrObj should have the highest index" invariant doesn't hold:
    createSwDoc("sdt-textbox-header.docx");
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
