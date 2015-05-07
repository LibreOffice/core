/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/calc_unoapi_test.hxx>
#include <test/util/xreplaceable.hxx>
#include <test/util/xsearchable.hxx>
#include <test/sheet/xprintareas.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include "docsh.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"

using namespace css;
using namespace css::uno;

namespace sc_apitest
{

class ScTableSheetObj : public CalcUnoApiTest, apitest::XSearchable, apitest::XReplaceable, apitest::XPrintAreas
{
public:
    ScTableSheetObj();

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    virtual uno::Reference< uno::XInterface > init() SAL_OVERRIDE;
    void testCopyPasteXLS();

    CPPUNIT_TEST_SUITE(ScTableSheetObj);
    CPPUNIT_TEST(testFindAll);
    CPPUNIT_TEST(testFindNext);
    CPPUNIT_TEST(testFindFirst);
    CPPUNIT_TEST(testReplaceAll);
    CPPUNIT_TEST(testCreateReplaceDescriptor);
    // XPrintAreas
    CPPUNIT_TEST(testSetAndGetPrintTitleColumns);
    CPPUNIT_TEST(testSetAndGetPrintTitleRows);
    CPPUNIT_TEST(testCopyPasteXLS);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference< lang::XComponent > mxComponent;
};

ScTableSheetObj::ScTableSheetObj():
    CalcUnoApiTest("/sc/qa/extras/testdocuments"),
    apitest::XSearchable(OUString("test"), 4),
    apitest::XReplaceable(OUString("searchReplaceString"), OUString("replaceReplaceString"))
{
}

uno::Reference< uno::XInterface > ScTableSheetObj::init()
{
    OUString aFileURL;
    createFileURL(OUString("ScTableSheetObj.ods"), aFileURL);
    if(!mxComponent.is())
        mxComponent = loadFromDesktop(aFileURL, "com.sun.star.sheet.SpreadsheetDocument");
    CPPUNIT_ASSERT(mxComponent.is());

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > xSheet( xIndex->getByIndex(0), UNO_QUERY_THROW);

    return xSheet;
}

// tdf#83366
void ScTableSheetObj::testCopyPasteXLS()
{
    OUString aFileURL;
    const OString sFailedMessage = OString("Failed on :");
    createFileURL(OUString("DemoSummerOlympics.xls"), aFileURL);
    uno::Reference< com::sun::star::lang::XComponent > xComponent =
        loadFromDesktop(aFileURL);
    CPPUNIT_ASSERT(xComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != NULL);

    // 1. Open the document
    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != NULL);

    // 2. Highlight I5:I24
    ScRange aSrcRange;
    sal_uInt16 nRes = aSrcRange.Parse("I5:I24", &rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);

    ScMarkData aMark;
    aMark.SetMarkArea(aSrcRange);

    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);

    // 3. Copy
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);


    // 4. Close the document (Ctrl-W)
    closeDocument(xComponent);
    xComponent.clear();

    // 5. Create a new Spreadsheet
    xComponent = loadFromDesktop("private:factory/scalc");

    // Get the document model
    pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != NULL);

    // Get the document controller
    pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != NULL);

    // 6. Paste
    pViewShell->GetViewData().GetView()->PasteFromClip(IDF_ALL, &aClipDoc);

    closeDocument(xComponent);
    xComponent.clear();
}

void ScTableSheetObj::setUp()
{
    CalcUnoApiTest::setUp();
}

void ScTableSheetObj::tearDown()
{
    if (mxComponent.is())
    {
        closeDocument(mxComponent);
        mxComponent.clear();
    }

    CalcUnoApiTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScTableSheetObj);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
