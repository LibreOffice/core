/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XAutoTextGroup.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XTextContentAppend.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/rdf/URI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <comphelper/propertyvalue.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/graphicfilter.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <AnnotationWin.hxx>
#include <flyfrm.hxx>
#include <fmtanchr.hxx>
#include <unotextrange.hxx>

using namespace ::com::sun::star;

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/core/unocore/data/";
}

/// Covers sw/source/core/unocore/ fixes.
class SwCoreUnocoreTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf119081)
{
    // Load a doc with a nested table in it.
    load(DATA_DIRECTORY, "tdf119081.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Enter outer A1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/3);
    // Enter inner A1.
    pWrtShell->Right(CRSR_SKIP_CELLS, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false,
                     /*bVisual=*/true);
    // Enter outer B1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);

    SwDoc* pDoc = pDocShell->GetDoc();
    SwPaM& rCursor = pWrtShell->GetCurrentShellCursor();
    uno::Reference<text::XTextRange> xInsertPosition
        = SwXTextRange::CreateXTextRange(*pDoc, *rCursor.GetPoint(), nullptr);
    uno::Reference<text::XTextAppend> xTextAppend(xInsertPosition->getText(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.uno.RuntimeException
    xTextAppend->insertTextPortion("x", {}, xInsertPosition);

    // Verify that the string is indeed inserted.
    pWrtShell->Left(CRSR_SKIP_CELLS, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false,
                    /*bVisual=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pWrtShell->GetCurrentShellCursor().GetText());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
