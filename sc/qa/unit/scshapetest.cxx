/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>

#include <sfx2/dispatch.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdpage.hxx>

#include <docsh.hxx>
#include <drwlayer.hxx>
#include <tabvwsh.hxx>
#include <userdat.hxx>

#include <sc.hrc> // defines of slot-IDs

using namespace css;

namespace sc_apitest
{
class ScShapeTest : public CalcUnoApiTest
{
public:
    ScShapeTest();

    void testFitToCellSize();
    void testCustomShapeCellAnchoredRotatedShape();

    CPPUNIT_TEST_SUITE(ScShapeTest);
    CPPUNIT_TEST(testFitToCellSize);
    CPPUNIT_TEST(testCustomShapeCellAnchoredRotatedShape);
    CPPUNIT_TEST_SUITE_END();
};

ScShapeTest::ScShapeTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

static OUString lcl_compareRectWithTolerance(const tools::Rectangle& rExpected,
                                             const tools::Rectangle& rActual,
                                             const sal_Int32 nTolerance)
{
    OUString sErrors;
    if (labs(rExpected.Left() - rActual.Left()) > nTolerance)
        sErrors += "\nLeft expected " + OUString::number(rExpected.Left()) + " actual "
                   + OUString::number(rActual.Left()) + " Tolerance "
                   + OUString::number(nTolerance);
    if (labs(rExpected.Top() - rActual.Top()) > nTolerance)
        sErrors += "\nTop expected " + OUString::number(rExpected.Top()) + " actual "
                   + OUString::number(rActual.Top()) + " Tolerance " + OUString::number(nTolerance);
    if (labs(rExpected.GetWidth() - rActual.GetWidth()) > nTolerance)
        sErrors += "\nWidth expected " + OUString::number(rExpected.GetWidth()) + " actual "
                   + OUString::number(rActual.GetWidth()) + " Tolerance "
                   + OUString::number(nTolerance);
    if (labs(rExpected.GetHeight() - rActual.GetHeight()) > nTolerance)
        sErrors += "\nHeight expected " + OUString::number(rExpected.GetHeight()) + " actual "
                   + OUString::number(rActual.GetHeight()) + " Tolerance "
                   + OUString::number(nTolerance);
    return sErrors;
}

void ScShapeTest::testFitToCellSize()
{
    // The document has a cell anchored custom shape. Applying
    // FitToCellSize should resize and position the shape so,
    // that it fits into its anchor cell. That did not happened.
    OUString aFileURL;
    createFileURL("tdf119191_FitToCellSize.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Get the document controller
    ScTabViewShell* pViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT(pDrawView);

    // Select the shape
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT(pDrawView->AreObjectsMarked());

    // Fit selected shape into cell
    pViewShell->GetViewData().GetDispatcher().Execute(SID_FITCELLSIZE);

    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    const tools::Rectangle aCellRect = rDoc.GetMMRect(1, 1, 1, 1, 0);
    const OUString sErrors(lcl_compareRectWithTolerance(aCellRect, rShapeRect, 1));
    CPPUNIT_ASSERT_EQUAL(OUString(), sErrors);

    pDocSh->DoClose();
}

void ScShapeTest::testCustomShapeCellAnchoredRotatedShape()
{
    // The example doc contains a cell anchored custom shape that is rotated
    // and sheared. Error was, that the shape lost position and size on
    // loading.
    OUString aFileURL;
    createFileURL("tdf119191_transformedShape.ods", aFileURL);
    uno::Reference<css::lang::XComponent> xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Get the shape
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    SdrObjCustomShape* pObj = dynamic_cast<SdrObjCustomShape*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObj);

    // Check Position and Size
    tools::Rectangle aRect(2400, 751, 5772, 3693); // expected snap rect
    rDoc.SetDrawPageSize(0); // trigger recalcpos
    const tools::Rectangle& rShapeRect(pObj->GetSnapRect());
    const OUString sPosSizeErrors(lcl_compareRectWithTolerance(aRect, rShapeRect, 1));
    CPPUNIT_ASSERT_EQUAL(OUString(), sPosSizeErrors);

    // Check anchor
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);

    const OUString sActual("start col " + OUString::number(pData->maStart.Col()) + " row "
                           + OUString::number(pData->maStart.Row()) + " end col "
                           + OUString::number(pData->maEnd.Col()) + " row "
                           + OUString::number(pData->maEnd.Row()));
    CPPUNIT_ASSERT_EQUAL(OUString("start col 1 row 1 end col 2 row 8"), sActual);

    pDocSh->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScShapeTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
