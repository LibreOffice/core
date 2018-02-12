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
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <unonames.hxx>

#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <svx/svdocirc.hxx>
#include <scitems.hxx>

#include <sc.hrc>

using namespace css;

namespace sc_apitest {

class ScAnchorTest : public CalcUnoApiTest
{
public:
    ScAnchorTest();

    virtual void tearDown() override;

    void testODFAnchorTypes();

    CPPUNIT_TEST_SUITE(ScAnchorTest);

    CPPUNIT_TEST(testODFAnchorTypes);
    CPPUNIT_TEST_SUITE_END();
private:

    uno::Reference< lang::XComponent > mxComponent;
};

ScAnchorTest::ScAnchorTest()
    : CalcUnoApiTest("sc/qa/unit/data/ods")
{
}

void ScAnchorTest::testODFAnchorTypes()
{
    OUString aFileURL;
    createFileURL("3AnchorTypes.ods", aFileURL);
    // open the document with graphic included
    uno::Reference< css::lang::XComponent > xComponent = loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Check whether graphic imported well
    ScDocument& rDoc = pDocSh->GetDocument();
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT(pDrawLayer);

    const SdrPage *pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);

    // Check 1st object: Page anchored
    SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT(pObject);
    ScAnchorType anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_PAGE, anchorType);

    // Check 2nd object: Cell anchored, resize with cell
    pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(1));
    CPPUNIT_ASSERT(pObject);
    anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL_RESIZE, anchorType);

     // Check 3rd object: Cell anchored
    pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(2));
    CPPUNIT_ASSERT(pObject);
    anchorType = ScDrawLayer::GetAnchorType(*pObject);
    CPPUNIT_ASSERT_EQUAL(SCA_CELL, anchorType);

    pDocSh->DoClose();
}

void ScAnchorTest::tearDown()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScAnchorTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
