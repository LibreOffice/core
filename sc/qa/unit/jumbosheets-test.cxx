/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapixml_test.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/keycodes.hxx>
#include <comphelper/servicehelper.hxx>
#include <svx/svdpage.hxx>

#include <docsh.hxx>
#include <defaultsoptions.hxx>
#include <scmod.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <scdll.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Tests for sheets larger than 1024 columns and/or 1048576 rows. */

class ScJumboSheetsTest : public UnoApiXmlTest
{
public:
    ScJumboSheetsTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testRoundtripColumn2000Ods();
    void testRoundtripColumn2000Xlsx();
    void testRoundtripColumnRangeOds();
    void testRoundtripColumnRangeXlsx();
    void testRoundtripNamedRangesOds();
    void testRoundtripNamedRangesXlsx();
    void testNamedRangeNameConflict();
    void testTdf134553();
    void testTdf134392();
    void testTdf147509();
    void testTdf133033();
    void testTdf109061();

    CPPUNIT_TEST_SUITE(ScJumboSheetsTest);

    CPPUNIT_TEST(testRoundtripColumn2000Ods);
    CPPUNIT_TEST(testRoundtripColumn2000Xlsx);
    CPPUNIT_TEST(testRoundtripColumnRangeOds);
    CPPUNIT_TEST(testRoundtripColumnRangeXlsx);
    CPPUNIT_TEST(testRoundtripNamedRangesOds);
    CPPUNIT_TEST(testRoundtripNamedRangesXlsx);
    CPPUNIT_TEST(testNamedRangeNameConflict);
    CPPUNIT_TEST(testTdf134553);
    CPPUNIT_TEST(testTdf134392);
    CPPUNIT_TEST(testTdf147509);
    CPPUNIT_TEST(testTdf133033);
    CPPUNIT_TEST(testTdf109061);

    CPPUNIT_TEST_SUITE_END();

private:
    void testRoundtripColumn2000(std::u16string_view name, const char* format);
    void testRoundtripNamedRanges(std::u16string_view name, const char* format);
};

void ScJumboSheetsTest::testRoundtripColumn2000Ods()
{
    testRoundtripColumn2000(u"ods/value-in-column-2000.ods", "calc8");
}

void ScJumboSheetsTest::testRoundtripColumn2000Xlsx()
{
    testRoundtripColumn2000(u"xlsx/value-in-column-2000.xlsx", "Calc Office Open XML");
}

void ScJumboSheetsTest::testRoundtripColumn2000(std::u16string_view name, const char* format)
{
    loadFromFile(name);
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        // Check the value at BXX1 (2000th column).
        CPPUNIT_ASSERT_EQUAL(-5.0, pDoc->GetValue(1999, 0, 0));
        // Check the formula referencing the value.
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), pDoc->GetFormula(0, 0, 0));
        // Recalc and check value in the reference.
        pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, pDoc->GetValue(0, 0, 0));
    }

    saveAndReload(OUString::createFromAscii(format));
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);

        ScDocument* pDoc = pModelObj->GetDocument();
        // Check again.
        CPPUNIT_ASSERT_EQUAL(-5.0, pDoc->GetValue(1999, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), pDoc->GetFormula(0, 0, 0));
        pDoc->CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, pDoc->GetValue(0, 0, 0));
    }
}

void ScJumboSheetsTest::testRoundtripColumnRangeOds()
{
    loadFromFile(u"ods/sum-whole-column-row.ods");
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        // Check the formula referencing the whole-row range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), pDoc->GetFormula(0, 0, 0));
        // Check the formula referencing the whole-column range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), pDoc->GetFormula(1, 0, 0));
    }

    saveAndReload("calc8");
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);

        ScDocument* pDoc = pModelObj->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), pDoc->GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), pDoc->GetFormula(1, 0, 0));
        xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPath(pXmlDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[1]"_ostr,
                    "formula"_ostr, "of:=SUM([.2:.2])");
        assertXPath(pXmlDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[2]"_ostr,
                    "formula"_ostr, "of:=SUM([.C:.C])");
    }
}

void ScJumboSheetsTest::testRoundtripColumnRangeXlsx()
{
    loadFromFile(u"ods/sum-whole-column-row.ods");
    saveAndReload("Calc Office Open XML");
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);

        ScDocument* pDoc = pModelObj->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), pDoc->GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), pDoc->GetFormula(1, 0, 0));
        xmlDocUniquePtr pXmlDoc = parseExport("xl/worksheets/sheet1.xml");
        CPPUNIT_ASSERT(pXmlDoc);
        assertXPathContent(pXmlDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]/x:f"_ostr,
                           "SUM(2:2)");
        assertXPathContent(pXmlDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[2]/x:f"_ostr,
                           "SUM(C:C)");
    }
}

void ScJumboSheetsTest::testRoundtripNamedRanges(std::u16string_view name, const char* format)
{
    loadFromFile(name);

    std::pair<OUString, OUString> ranges[] = { { "CELLBXX1", "$Sheet1.$BXX$1" },
                                               { "CELLSA4_AMJ4", "$Sheet1.$A$4:$AMJ$4" },
                                               { "CELLSBXX1_BXX10", "$Sheet1.$BXX$1:$BXX$10" },
                                               { "CELLSBXX1_BXX10_RELATIVE", "$Sheet1.BXX1:BXX10" },
                                               { "CELLSE1_E1024", "$Sheet1.$E$1:$E$1024" },
                                               { "CELLSE1_E2000000", "$Sheet1.$E$1:$E$2000000" },
                                               { "COLUMN_E", "$Sheet1.$E:$E" },
                                               { "ROW_4", "$Sheet1.$4:$4" } };
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        for (const auto& range : ranges)
        {
            ScRangeData* rangeData = pDoc->GetRangeName()->findByUpperName(range.first);
            CPPUNIT_ASSERT(rangeData);
            CPPUNIT_ASSERT_EQUAL(range.second, rangeData->GetSymbol());
        }
    }

    saveAndReload(OUString::createFromAscii(format));
    {
        ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
        CPPUNIT_ASSERT(pModelObj);
        ScDocument* pDoc = pModelObj->GetDocument();
        for (const auto& range : ranges)
        {
            ScRangeData* rangeData = pDoc->GetRangeName()->findByUpperName(range.first);
            CPPUNIT_ASSERT(rangeData);
            CPPUNIT_ASSERT_EQUAL(range.second, rangeData->GetSymbol());
        }
    }
}

void ScJumboSheetsTest::testRoundtripNamedRangesOds()
{
    testRoundtripNamedRanges(u"ods/ranges-column-2000.ods", "calc8");
}

void ScJumboSheetsTest::testRoundtripNamedRangesXlsx()
{
    testRoundtripNamedRanges(u"ods/ranges-column-2000.ods", "Calc Office Open XML");
}

void ScJumboSheetsTest::testNamedRangeNameConflict()
{
    // The document contains named ranges named 'num1' and 'num2', that should be still treated
    // as named references even though with 16k columns those are normally NUM1 and NUM2 cells.
    loadFromFile(u"ods/named-range-conflict.ods");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    pDoc->CalcAll();
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(10022, 0, 0)); // NUM1
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(10022, 1, 0)); // NUM2
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(0, 0, 0)); // = num1
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(0, 1, 0)); // = sheet2.num2
    CPPUNIT_ASSERT_EQUAL(0.0, pDoc->GetValue(0, 2, 0)); // = SUM(NUM1:NUM2) (not named ranges)
    pDoc->SetValue(10022, 0, 0, 100); // NUM1
    pDoc->SetValue(10022, 1, 0, 200); // NUM2
    pDoc->CalcAll();
    // First two are the same, the sum changes.
    CPPUNIT_ASSERT_EQUAL(2.0, pDoc->GetValue(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(300.0, pDoc->GetValue(0, 2, 0));
}

void ScJumboSheetsTest::testTdf134553()
{
    loadFromFile(u"xlsx/tdf134553.xlsx");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);
    const SdrObject* pOleObj = pPage->GetObj(0);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(pOleObj);

    // Sorry, the charts so severely suffer from DPI dependency, that I can't find motivation
    // to add huge tolerances (around 350!) here to make it pass on non-default DPI, with no
    // guarantee that the test would have any value after that. So just skip it.
    // FIXME: the DPI check should be removed when either (1) the test is fixed to work with
    // non-default DPI; or (2) unit tests on Windows are made to use svp VCL plugin.
    if (!IsDefaultDPI())
        return;

    CPPUNIT_ASSERT_EQUAL(tools::Long(12741), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(7620), pOleObj->GetLogicRect().getOpenHeight());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4574), pOleObj->GetLogicRect().getX());
    CPPUNIT_ASSERT_EQUAL(tools::Long(437), pOleObj->GetLogicRect().getY());

    ScTabViewShell* pViewShell = ScDocShell::GetViewData()->GetViewShell();
    pViewShell->SelectObject(u"Diagram 1");

    dispatchCommand(mxComponent, ".uno:Cut", {});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    dispatchCommand(mxComponent, ".uno:Paste", {});

    pOleObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pOleObj);

    CPPUNIT_ASSERT_EQUAL(tools::Long(12741), pOleObj->GetLogicRect().getOpenWidth());
    CPPUNIT_ASSERT_EQUAL(tools::Long(7620), pOleObj->GetLogicRect().getOpenHeight());
    CPPUNIT_ASSERT_EQUAL(tools::Long(1700), pOleObj->GetLogicRect().getX());
    // tdf#147458: Without the fix in place, this test would have failed with
    // - Expected: 2117
    // - Actual  : -7421
    CPPUNIT_ASSERT_EQUAL(tools::Long(2117), pOleObj->GetLogicRect().getY());
}

void ScJumboSheetsTest::testTdf134392()
{
    // Without the fix in place, the file would have crashed
    loadFromFile(u"xlsx/tdf134392.xlsx");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    pDoc->CalcAll(); // perform hard re-calculation.
}

void ScJumboSheetsTest::testTdf147509()
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    ScDocument* pDoc = pModelObj->GetDocument();

    pDoc->SetString(0, 0, 0, "A");
    pDoc->SetString(1, 0, 0, "B");

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), ScDocShell::GetViewData()->GetCurY());

    dispatchCommand(mxComponent, ".uno:SelectColumn", {});

    dispatchCommand(mxComponent, ".uno:InsertColumnsAfter", {});

    CPPUNIT_ASSERT_EQUAL(OUString("A"), pDoc->GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected:
    // - Actual  : B
    CPPUNIT_ASSERT_EQUAL(OUString(""), pDoc->GetString(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("B"), pDoc->GetString(ScAddress(2, 0, 0)));
}

void ScJumboSheetsTest::testTdf133033()
{
    mxComponent = loadFromDesktop("private:factory/scalc");
    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), ScDocShell::GetViewData()->GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16777215), ScDocShell::GetViewData()->GetCurY());
}

void ScJumboSheetsTest::testTdf109061()
{
    // Without the fix in place, the file would have crashed
    loadFromFile(u"xlsx/tdf109061.xlsx");

    ScModelObj* pModelObj = comphelper::getFromUnoTunnel<ScModelObj>(mxComponent);
    CPPUNIT_ASSERT(pModelObj);
    ScDocument* pDoc = pModelObj->GetDocument();
    pDoc->CalcAll(); // perform hard re-calculation.

    CPPUNIT_ASSERT_EQUAL(6.0, pDoc->GetValue(1, 3, 0));
}

ScJumboSheetsTest::ScJumboSheetsTest()
    : UnoApiXmlTest("/sc/qa/unit/data/")
{
}

void ScJumboSheetsTest::setUp()
{
    UnoApiXmlTest::setUp();

    //Init before GetDefaultsOptions
    ScDLL::Init();

    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(true);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);
}

void ScJumboSheetsTest::tearDown()
{
    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(false);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);

    UnoApiXmlTest::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScJumboSheetsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
