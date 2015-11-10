/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <unonames.hxx>

#include "tabvwsh.hxx"
#include "docsh.hxx"

#include "sc.hrc"

using namespace css;

namespace sc_apitest {

#define NUMBER_OF_TESTS 1

class ScConditionalFormatTest : public CalcUnoApiTest
{
public:
    ScConditionalFormatTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    uno::Reference< uno::XInterface > init();
    void testCondFormat();
    void testUndoAnchor();

    CPPUNIT_TEST_SUITE(ScConditionalFormatTest);
    CPPUNIT_TEST(testCondFormat);
    CPPUNIT_TEST(testUndoAnchor);
    CPPUNIT_TEST_SUITE_END();
private:

    static sal_Int32 nTest;
    static uno::Reference< lang::XComponent > mxComponent;
};

sal_Int32 ScConditionalFormatTest::nTest = 0;
uno::Reference< lang::XComponent > ScConditionalFormatTest::mxComponent;

ScConditionalFormatTest::ScConditionalFormatTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

uno::Reference< uno::XInterface > ScConditionalFormatTest::init()
{
    // get the test file
    OUString aFileURL;
    createFileURL("new_cond_format_test.ods", aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT_MESSAGE("Component not loaded",mxComponent.is());

    // get the first sheet
    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), uno::UNO_QUERY_THROW);

    return xSheet;
}

void ScConditionalFormatTest::testCondFormat()
{
    uno::Reference< sheet::XSpreadsheet > aSheet(init(), uno::UNO_QUERY_THROW);
    uno::Reference< beans::XPropertySet > xProps;
    uno::Reference< sheet::XSheetConditionalEntries > xSheetConditionalEntries;

    xProps.set(aSheet->getCellRangeByPosition(1, 1, 1, 21), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), xSheetConditionalEntries->getCount());

    uno::Sequence< beans::PropertyValue > aPropertyValueList(5);
    aPropertyValueList[0].Name = SC_UNONAME_STYLENAME;
    aPropertyValueList[0].Value <<= uno::makeAny<OUString>("Result2");
    aPropertyValueList[1].Name = SC_UNONAME_FORMULA1;
    aPropertyValueList[1].Value <<= uno::makeAny<OUString>("$Sheet1.$B$2");
    aPropertyValueList[2].Name = SC_UNONAME_FORMULA2;
    aPropertyValueList[2].Value <<= uno::makeAny<OUString>("$Sheet1.$A$2");
    aPropertyValueList[3].Name = SC_UNONAME_OPERATOR;
    aPropertyValueList[3].Value <<= sheet::ConditionOperator_EQUAL;
    aPropertyValueList[4].Name = SC_UNONAME_SOURCEPOS;
    aPropertyValueList[4].Value <<= uno::makeAny(table::CellAddress(0, 0, 16));
    xSheetConditionalEntries->addNew(aPropertyValueList);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSheetConditionalEntries->getCount());
    xProps->setPropertyValue(SC_UNONAME_CONDFMT, uno::makeAny(xSheetConditionalEntries));

    xProps.set(aSheet->getCellByPosition(0, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheetConditionalEntries->getCount());

    // clear conditional formatting in cell $B$17
    xProps.set(aSheet->getCellByPosition(1, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xSheetConditionalEntries->getCount());
    xSheetConditionalEntries->clear();
    xProps->setPropertyValue(SC_UNONAME_CONDFMT, uno::makeAny(xSheetConditionalEntries));

    xProps.set(aSheet->getCellByPosition(1, 16), uno::UNO_QUERY_THROW);
    xSheetConditionalEntries.set(xProps->getPropertyValue(SC_UNONAME_CONDFMT), uno::UNO_QUERY_THROW);
    // This was 1 before - conditional formats were not removed
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xSheetConditionalEntries->getCount());
}

void ScConditionalFormatTest::testUndoAnchor()
{
    const OString sFailedMessage = OString("Failed on :");
    OUString aFileURL;
    createFileURL("document_with_linked_graphic.ods", aFileURL);
    // open the document with graphic included
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    // Check whether graphic imported well
    ScDocument& rDoc = xDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDrawLayer != nullptr );

    const SdrPage *pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pPage != nullptr );

    SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject != nullptr );
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pObject->IsLinkedGraphic() );

    const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), !rGraphicObj.IsSwappedOut());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pViewShell != nullptr );

    // Get the draw view of the document
    ScDrawView* pDrawView = pViewShell->GetViewData().GetScDrawView();
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDrawView != nullptr );

    // Select graphic object
    pDrawView->MarkNextObj();
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), pDrawView->AreObjectsMarked() );

    // Set Cell Anchor
    ScDrawLayer::SetCellAnchoredFromPosition(*pObject, rDoc, 0);
    // Check state
    ScAnchorType oldType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), oldType == SCA_CELL );

    // Change all selected objects to page anchor
    pViewShell->GetViewData().GetDispatcher().Execute(SID_ANCHOR_PAGE);
    // Check state
    ScAnchorType newType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), newType == SCA_PAGE );

    // Undo and check its result.
    SfxUndoManager* pUndoMgr = rDoc.GetUndoManager();
    CPPUNIT_ASSERT(pUndoMgr);
    pUndoMgr->Undo();

    // Check anchor type
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), oldType == ScDrawLayer::GetAnchorType(*pObject) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), newType == ScDrawLayer::GetAnchorType(*pObject) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

    ScDrawLayer::SetPageAnchored(*pObject);
    // Check state
    oldType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), oldType == SCA_PAGE );

    // Change all selected objects to cell anchor
    pViewShell->GetViewData().GetDispatcher().Execute(SID_ANCHOR_CELL);
    // Check state
    newType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), newType == SCA_CELL );

    pUndoMgr->Undo();

    // Check anchor type
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), oldType == ScDrawLayer::GetAnchorType(*pObject) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());

    pUndoMgr->Redo();

    // Check anchor type
    CPPUNIT_ASSERT_MESSAGE( sFailedMessage.getStr(), newType == ScDrawLayer::GetAnchorType(*pObject) );
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), GRAPHIC_BITMAP, rGraphicObj.GetGraphic().GetType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( sFailedMessage.getStr(), sal_uLong(864900), rGraphicObj.GetSizeBytes());
}

void ScConditionalFormatTest::setUp()
{
    nTest++;
    CalcUnoApiTest::setUp();
}

void ScConditionalFormatTest::tearDown()
{
    if (nTest == NUMBER_OF_TESTS)
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScConditionalFormatTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
