/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/borderline.hxx>
#include <sal/config.h>
#include <helper/qahelper.hxx>
#include <sal/log.hxx>
#include <svx/svdpage.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <vcl/scheduler.hxx>
#include <vcl/svapp.hxx>

#include <conditio.hxx>
#include <document.hxx>
#include <scitems.hxx>

#include <chrono>

#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>

#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <editeng/brushitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScMacrosTest : public ScModelTestBase
{
public:
    ScMacrosTest();
    virtual void setUp() override;
};

// I suppose you could say this test doesn't really belong here, OTOH
// we need a full document to run the test ( it related originally to an
// imported Excel VBA macro ) It's convenient and fast to unit test
// this the problem this way. Perhaps in the future there will be some sort
// of slowcheck tests ( requiring a full document environment in the scripting
// module, we could move the test there then ) - relates to fdo#67547
CPPUNIT_TEST_FIXTURE(ScMacrosTest, testMSP)
{
    createScDoc("MasterScriptProviderProblem.ods");

    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.Module1.TestMSP?language=Basic&location=document"_ustr);
    OUString sResult;
    aRet >>= sResult;

    SAL_INFO("sc.qa", "Result is " << sResult );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TestMSP ( for fdo#67547) failed", u"OK"_ustr, sResult);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testPasswordProtectedStarBasic)
{
    createScDoc("testTypePassword.ods");
    ScDocument* pDoc = getScDoc();

    // User defined types
    executeMacro(u"vnd.sun.Star.script:Standard.Module1.LoadAndExecuteTest?language=Basic&location=document"_ustr);

    OUString aValue = pDoc->GetString(0,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User defined types script did not change the value of Sheet1.A1", u"success"_ustr, aValue);

    // Big Module

    executeMacro(u"vnd.sun.Star.script:MyLibrary.BigModule.bigMethod?language=Basic&location=document"_ustr);

    aValue = pDoc->GetString(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Big module script did not change the value of Sheet1.B1", u"success"_ustr, aValue);

    // far big method tdf#94617

    executeMacro(u"vnd.sun.Star.script:MyLibrary.BigModule.farBigMethod?language=Basic&location=document"_ustr);

    aValue = pDoc->GetString(2,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Far Method script did not change the value of Sheet1.C1", u"success"_ustr, aValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf142391)
{
    createScDoc("tdf142391.ods");
    ScDocument* pDoc = getScDoc();

    // User defined types
    executeMacro(
        u"vnd.sun.Star.script:Standard.Module1.LoadAndExecuteTest?language=Basic&location=document"_ustr);
    OUString aValue = pDoc->GetString(0, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User defined types script did not change the value of Sheet1.A1",
                                 u"success"_ustr, aValue);

    // Big Module
    executeMacro(
        u"vnd.sun.Star.script:MyLibrary.BigModule.bigMethod?language=Basic&location=document"_ustr);
    aValue = pDoc->GetString(1, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Big module script did not change the value of Sheet1.B1",
                                 u"success"_ustr, aValue);

    // tdf#142391 - method exceeds 0xffff offset for methods
    executeMacro(
        u"vnd.sun.Star.script:MyLibrary.BigModule.farBigMethod?language=Basic&location=document"_ustr);
    aValue = pDoc->GetString(2, 0, 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Far Method script did not change the value of Sheet1.C1",
                                 u"success"_ustr, aValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testStarBasic)
{
    createScDoc("StarBasic.ods");
    ScDocument* pDoc = getScDoc();

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document"_ustr);
    double aValue = pDoc->GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("script did not change the value of Sheet1.A1",2.0, aValue, 0.00001);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testRowColumn)
{
    createScDoc("StarBasic.ods");
    ScDocument* pDoc = getScDoc();

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Macro_RowHeight?language=Basic&location=document"_ustr);

    sal_uInt16 nHeight = o3tl::convert(pDoc->GetRowHeight(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Macro_ColumnWidth?language=Basic&location=document"_ustr);
    sal_uInt16 nWidth  = o3tl::convert(pDoc->GetColWidth(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4001), nWidth);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf146742)
{
    createScDoc("tdf146742.ods");

    // Export to ODS and reload the file
    saveAndReload(TestFilter::ODS);
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, pDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"2"_ustr, pDoc->GetString(0,1,0));

    CPPUNIT_ASSERT_EQUAL(u"TRUE"_ustr, pDoc->GetString(1,0,0));
    // Without the fix in place, this test would have failed with
    // - Expected: FALSE
    // - Actual  : TRUE
    CPPUNIT_ASSERT_EQUAL(u"FALSE"_ustr, pDoc->GetString(1,1,0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testMacroButtonFormControlXlsxExport)
{
    // Given a button form control with an associated macro:
    createScDoc("macro-button-form-control.xlsm");

    // When exporting to XLSM:
    save(TestFilter::XLSM);

    // Then make sure that the macro is associated with the control:
    xmlDocUniquePtr pSheetDoc = parseExport(u"xl/worksheets/sheet1.xml"_ustr);
    CPPUNIT_ASSERT(pSheetDoc);
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:controlPr' no attribute 'macro' exist
    // i.e. the macro was lost on export.
    assertXPath(pSheetDoc, "//x:controlPr", "macro", u"Module1.Button1_Click");

    // Then also make sure that there is no defined name for the macro, which is only needed for
    // XLS:
    xmlDocUniquePtr pWorkbookDoc = parseExport(u"xl/workbook.xml"_ustr);
    CPPUNIT_ASSERT(pWorkbookDoc);
    assertXPath(pWorkbookDoc, "//x:workbook/definedNames", 0);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf104902)
{
    createScDoc("tdf104902.ods");

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.display_bug?language=Basic&location=document"_ustr);

    // Export to ODS
    saveAndReload(TestFilter::ODS);
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"string no newlines"_ustr, pDoc->GetString(0, 0, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: string with
    // newlines
    // - Actual  : string withnewlines
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), pDoc->GetString(0, 1, 0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf64639)
{
    createScDoc("tdf64639.ods");
    ScDocument* pDoc = getScDoc();

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    // Add and delete the chart a few times
    // Without the fix in place, this test would have crashed here
    for (size_t i = 0; i < 5; ++i)
    {
        executeMacro(u"vnd.sun.Star.script:Standard.Module1.DrawGraph?language=Basic&location=document"_ustr);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

        executeMacro(u"vnd.sun.Star.script:Standard.Module1.DeleteGraph?language=Basic&location=document"_ustr);

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf142033)
{
    createScDoc("tdf142033.ods");

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.display_bug?language=Basic&location=document"_ustr);

    // Export to ODS
    saveAndReload(TestFilter::ODS);
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_EQUAL(u"string no newlines"_ustr, pDoc->GetString(0,0,0));
    CPPUNIT_ASSERT_EQUAL(u"string no newlines"_ustr, pDoc->GetString(0,1,0));

    // Without the fix in place, this test would have failed with
    // - Expected: string with
    // newlines
    // - Actual  : string withnewlines
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), pDoc->GetString(1,0,0));
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), pDoc->GetString(1,1,0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf89920)
{
    createScDoc("tdf89920.ods");

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.SearchAndReplaceNewline?language=Basic&"
                 "location=document"_ustr);

    // Export to ODS
    saveAndReload(TestFilter::ODS);

    xmlDocUniquePtr pContentXml = parseExport(u"content.xml"_ustr);
    CPPUNIT_ASSERT(pContentXml);

    assertXPathContent(pContentXml,
                       "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                       "table:table-row[1]/table:table-cell[1]/text:p[1]",
                       u"aa bb");

    // Without the fix in place, this test would have failed here with
    // - Expression: xmlXPathNodeSetGetLength(pXmlNodes) > 0
    assertXPathContent(pContentXml,
                       "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                       "table:table-row[1]/table:table-cell[1]/text:p[2]",
                       u"cc dd");
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testPasswordProtectedUnicodeString)
{
    const OUString sCorrectString(u"English Русский 中文"_ustr);
    static constexpr OUString sMacroURL(
        u"vnd.sun.Star.script:Protected.Module1.TestUnicodeString?language=Basic&location=document"_ustr);
    static constexpr OUString sLibName(u"Protected"_ustr);

    createScDoc("tdf57113.ods");

    // Check that loading password-protected macro image correctly loads Unicode strings
    {
        Any aRet = executeMacro(sMacroURL);

        OUString aReturnValue;
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sCorrectString, aReturnValue);
    }

    // Unlock and load the library, to regenerate the image on save
    css::uno::Reference<css::document::XEmbeddedScripts> xES(mxComponent, UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainer> xLC(xES->getBasicLibraries(),
                                                            UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(xLC, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xPasswd->isLibraryPasswordProtected(sLibName));
    CPPUNIT_ASSERT(!xPasswd->isLibraryPasswordVerified(sLibName));
    CPPUNIT_ASSERT(xPasswd->verifyLibraryPassword(sLibName, u"password"_ustr));
    xLC->loadLibrary(sLibName);
    CPPUNIT_ASSERT(xLC->isLibraryLoaded(sLibName));

    // Password protected documents can't be validated
    skipValidation();

    // Now check that saving stores Unicode data correctly in image's string pool
    saveAndReload(TestFilter::ODS);

    {
        Any aRet = executeMacro(sMacroURL);

        OUString aReturnValue;
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sCorrectString, aReturnValue);
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testPasswordProtectedArrayInUserType)
{
    static constexpr OUString sMacroURL(
        u"vnd.sun.Star.script:Protected.Module1.TestMyType?language=Basic&location=document"_ustr);
    static constexpr OUString sLibName(u"Protected"_ustr);

    createScDoc("ProtectedArrayInCustomType.ods");

    // Check that loading password-protected macro image correctly loads array bounds
    {
        Any aRet = executeMacro(sMacroURL);

        sal_Int16 nReturnValue;
        aRet >>= nReturnValue;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nReturnValue);
    }

    // Unlock and load the library, to regenerate the image on save
    css::uno::Reference<css::document::XEmbeddedScripts> xES(mxComponent, UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainer> xLC(xES->getBasicLibraries(),
        UNO_QUERY_THROW);
    css::uno::Reference<css::script::XLibraryContainerPassword> xPasswd(xLC, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xPasswd->isLibraryPasswordProtected(sLibName));
    CPPUNIT_ASSERT(!xPasswd->isLibraryPasswordVerified(sLibName));
    CPPUNIT_ASSERT(xPasswd->verifyLibraryPassword(sLibName, u"password"_ustr));
    xLC->loadLibrary(sLibName);
    CPPUNIT_ASSERT(xLC->isLibraryLoaded(sLibName));

    // Password protected documents can't be validated
    skipValidation();

    // Now check that saving stores array bounds correctly
    saveAndReload(TestFilter::ODS);

    {
        Any aRet = executeMacro(sMacroURL);

        sal_Int16 nReturnValue;
        aRet >>= nReturnValue;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nReturnValue);
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf114427)
{
    createScDoc("tdf114427.ods");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xDraws(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDraws->getCount());

    // Without the fix in place, it would have crashed here
    executeMacro(u"vnd.sun.Star.script:Standard.Module1.DeletingFrame?language=Basic&location=document"_ustr);

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDraws->getCount());
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf131296_legacy)
{
    // For legacy password-protected library images, we must correctly get the constants' values,
    // and also - for Integer - the type.
    const std::vector<std::pair<OUString, OUString>> aTests({
        { "TestIntConst", "Integer: 123" },
        { "TestLongConst", "Double: 123" },
        { "TestSingleConst", "Double: 123" },
        { "TestDoubleConst", "Double: 123" },
    });

    createScDoc("tdf131296_legacy.ods");
    {
        for (auto& [sTestName, sExpected] : aTests)
        {
            Any aRet = executeMacro("vnd.sun.Star.script:Protected.Module1." + sTestName
                                            + "?language=Basic&location=document");

            OUString aReturnValue;
            aRet >>= aReturnValue;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestName.toUtf8().getStr(), sExpected, aReturnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf131296_new)
{
    // For new password-protected library images, we must correctly get both the constants' values
    // and their types.
    const std::vector<std::pair<OUString, OUString>> aTests({
        { "TestIntConst", "Integer: 123" },
        { "TestLongConst", "Long: 123" },
        { "TestSingleConst", "Single: 123" },
        { "TestDoubleConst", "Double: 123" },
        { "TestCurrencyConst", "Currency: 123.0000" },
    });

    createScDoc("tdf131296_new.ods");
    {
        for (auto& [sTestName, sExpected] : aTests)
        {
            Any aRet = executeMacro("vnd.sun.Star.script:Protected.Module1." + sTestName
                                            + "?language=Basic&location=document");

            OUString aReturnValue;
            aRet >>= aReturnValue;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestName.toUtf8().getStr(), sExpected, aReturnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf46119)
{
    createScDoc("tdf46119.ods");
    ScDocument* pDoc = getScDoc();

    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document"_ustr);

    CPPUNIT_ASSERT_EQUAL(u"0.074"_ustr, pDoc->GetString(2, 24, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.067"_ustr, pDoc->GetString(2, 25, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.273"_ustr, pDoc->GetString(2, 26, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.259"_ustr, pDoc->GetString(2, 27, 0));

    CPPUNIT_ASSERT_EQUAL(u"0.097"_ustr, pDoc->GetString(3, 24, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.087"_ustr, pDoc->GetString(3, 25, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.311"_ustr, pDoc->GetString(3, 26, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.296"_ustr, pDoc->GetString(3, 27, 0));

    CPPUNIT_ASSERT_EQUAL(u"0.149"_ustr, pDoc->GetString(4, 24, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.134"_ustr, pDoc->GetString(4, 25, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.386"_ustr, pDoc->GetString(4, 26, 0));
    CPPUNIT_ASSERT_EQUAL(u"0.366"_ustr, pDoc->GetString(4, 27, 0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf128218)
{
    createScDoc("tdf128218.ods");

    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.Module1.TestRAND?language=Basic&location=document"_ustr);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: Double
    // - Actual  : Object()

    CPPUNIT_ASSERT_EQUAL(u"Double"_ustr, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf71271)
{
    createScDoc();
    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(u"CodeName"_ustr, uno::Any(u"NewCodeName"_ustr));
    }

    saveAndReload(TestFilter::ODS);

    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
        OUString sCodeName;
        uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
        // Without the fix in place the codename would not have been saved
        xProps->getPropertyValue(u"CodeName"_ustr) >>= sCodeName;
        CPPUNIT_ASSERT_EQUAL(u"NewCodeName"_ustr, sCodeName);
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf43003)
{
    createScDoc("tdf43003.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, the values of the specified cells won't be changed
    pDoc->SetValue(0, 0, 0, 2);
    CPPUNIT_ASSERT_EQUAL(3.0, pDoc->GetValue(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(4.0, pDoc->GetValue(2, 0, 0));
}


CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf75263)
{
    createScDoc("tdf75263.xlsm");

    {
        ScDocument* pDoc = getScDoc();
        pDoc->CalcAll();

        // A1 contains formula with user-defined function, and the function is defined in VBA.
        CPPUNIT_ASSERT_EQUAL(u"проба"_ustr, pDoc->GetString(0, 0, 0));
    }

    saveAndReload(TestFilter::XLSM);

    {
        ScDocument* pDoc = getScDoc();
        pDoc->CalcAll();

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: проба (sample)
        // - Actual  : ?????
        CPPUNIT_ASSERT_EQUAL(u"проба"_ustr, pDoc->GetString(0, 0, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf133887)
{
    createScDoc("tdf133887.ods");

    css::uno::Any aRet;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    css::uno::Sequence<css::uno::Any> aParams{ css::uno::Any(sal_Int16(0)) };

    SfxObjectShell::CallXScript(
        mxComponent,
        u"vnd.sun.Star.script:Standard.Module1.TestInvoke?language=Basic&location=document"_ustr, aParams,
        aRet, aOutParamIndex, aOutParam);

    double aReturnValue;
    aOutParam[0] >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: 6.75
    // - Actual  : 7

    CPPUNIT_ASSERT_EQUAL(6.75, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf133889)
{
    createScDoc("tdf133889.ods");

    css::uno::Any aRet;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    css::uno::Sequence<css::uno::Any> aParams{ css::uno::Any(sal_Int32(0)) };

    SfxObjectShell::CallXScript(
        mxComponent,
        u"vnd.sun.Star.script:Standard.Module1.TestInvoke?language=Basic&location=document"_ustr, aParams,
        aRet, aOutParamIndex, aOutParam);

    sal_Int32 aReturnValue;
    aOutParam[0] >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: 100000
    // - Actual  : 0

    CPPUNIT_ASSERT_EQUAL(sal_Int32(100000), aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf143582)
{
    createScDoc("tdf143582.ods");

    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.Module1.TestScriptInvoke?language=Basic&location=document"_ustr);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: Test6
    // - Actual  : TeTest8
    CPPUNIT_ASSERT_EQUAL(u"Test6"_ustr, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf144085)
{
    createScDoc("tdf144085.ods");

    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.Module1.TestScriptInvoke?language=Basic&location=document"_ustr);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: $Sheet1.$B$5:$E$17
    // - Actual  : $Sheet1.$B$5:$C$10
    CPPUNIT_ASSERT_EQUAL(u"$Sheet1.$B$5:$E$17"_ustr, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf125800)
{
    createScDoc("tdf125800.ods");
    ScDocument* pDoc = getScDoc();

    ScConditionalFormat* pFormat = pDoc->GetCondFormat(1, 2, 0);
    CPPUNIT_ASSERT(!pFormat);

    // Without the fix in place, this test would have failed with
    // - Expression: false
    // - Unexpected dialog:  Error: Inadmissible value or data type. Index out of defined range.
    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.cf.doItForThisSheetindexThisRange?language=Basic&location=document"_ustr);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    pFormat = pDoc->GetCondFormat(1, 2, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Condition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Direct, pCondition->GetOperation());
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf130307)
{
    createScDoc("tdf130307.ods");

    Any aRet = executeMacro(u"vnd.sun.Star.script:Standard.Module1.ForEachSheets?language=Basic&location=document"_ustr);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have crashed here
    CPPUNIT_ASSERT_EQUAL(u"Sheet1Sheet2"_ustr, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf144970)
{
    createScDoc("tdf144970.ods");
    ScDocument* pDoc = getScDoc();

    formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
    pDoc->SetGrammar(eGram);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    // Without the fix in place, this test would have failed with
    // - Expression: false
    // - Unexpected dialog:  Error: BASIC runtime error.
    // An exception occurred
    // Type: com.sun.star.lang.IllegalArgumentException
    executeMacro(u"vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document"_ustr);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf138646)
{
    createScDoc("tdf138646.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, changing the grammar from GRAM_NATIVE to either GRAM_NATIVE_XL_A1
    // or GRAM_NATIVE_XL_R1C1 would cause a Basic exception/error in the following script.
    pDoc->SetGrammar(formula::FormulaGrammar::Grammar::GRAM_NATIVE_XL_R1C1);

    const std::vector<std::pair<OUString, OUString>> aTests({
        { "GlobalNamedCell", "GlobalNamedCell" },
        { "GlobalNamedCellSheet", "GlobalNamedCell" },
        { "LocalNamedCell", "LocalNamedCell" },
        { "LocalNamedCellAccessError", "Exception" }
    });

    {
        for (auto& [sTestName, sExpected] : aTests)
        {
            Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1." + sTestName
                                            + "?language=Basic&location=document");

            OUString aReturnValue;
            aRet >>= aReturnValue;
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sTestName.toUtf8().getStr(), sExpected, aReturnValue);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf105558)
{
    createScDoc("tdf105558.ods");
    ScDocument* pDoc = getScDoc();

    // Without the fix in place, this test would have failed with
    // - Expected: 5.5
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(5.5, pDoc->GetValue(0, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf107572)
{
    createScDoc();

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(
            u"Function Main\n"
                     "  thisComponent.Sheets(0).getCellRangeByName(\"A1:F14\").autoformat(\"Default Style\")\n"
                     "End Function\n"_ustr));

    // Without the fix in place, this test would have crashed
    executeMacro(u"vnd.sun.Star.script:TestLibrary.TestModule.Main?language=Basic&location=document"_ustr);

    ScDocument* pDoc = getScDoc();

    SCCOL startCol = 0, endCol = 4;
    SCROW startRow = 0, endRow = 12;

    // Check autoformat has benn applied
    for (SCCOL col = startCol; col <= endCol; ++col)
    {
        for (SCROW row = startRow; row <= endRow; ++row)
        {
            const ScPatternAttr* pAttr = pDoc->GetPattern(col, row, 0);
            const SvxBoxItem& rBorder = pAttr->GetItem(ATTR_BORDER);

            CPPUNIT_ASSERT_EQUAL(COL_BLACK, rBorder.GetTop()->GetColor());
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, rBorder.GetLeft()->GetColor());
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testShapeLayerId)
{
    createScDoc();

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(
            u"Function TestLayerID\n"
                     "  xShape = thisComponent.createInstance(\"com.sun.star.drawing.TextShape\")\n"
                     "  thisComponent.DrawPages(0).Add(xShape)\n"
                     "  origID = xShape.LayerID\n"
                     "  On Error Goto handler\n"
                     "  xShape.LayerID = 257 ' 1 if wrongly converted to unsigned 8-bit type\n"
                     "  TestLayerID = origID & \" \" & xShape.LayerID ' Should not happen\n"
                     "  Exit Function\n"
                     "handler:\n"
                     "  ' This is expected to happen\n"
                     "  TestLayerID = origID & \" Expected runtime error happened\"\n"
                     "End Function\n"_ustr));

    Any aRet = executeMacro(u"vnd.sun.Star.script:TestLibrary.TestModule.TestLayerID?language=Basic&location=document"_ustr);
    // Without the fix in place, this test would have failed in non-debug builds with
    // - Expected : <Any: (string) 0 Expected runtime error happened>
    // - Actual   : <Any: (string) 0 1>
    // In debug builds, it would crash on assertion inside strong_int ctor.
    // The LayerID property of com.sun.star.drawing.Shape service has 'short' IDL type.
    // The expected run-time error is because there are only 5 layers there.
    CPPUNIT_ASSERT_EQUAL(Any(u"0 Expected runtime error happened"_ustr), aRet);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testFunctionAccessIndirect)
{
    OUString aFileName = loadFromFile(u"tdf120161.ods"); // just some document with known values in cells

    const OUString aReference = "'" + aFileName + "'#$Sheet1.A1";

    css::uno::Reference<css::sheet::XFunctionAccess> xFunc(
        comphelper::getProcessServiceFactory()->createInstance(u"com.sun.star.sheet.FunctionAccess"_ustr),
        UNO_QUERY_THROW);

    // tdf#148040: without the fix in place, this would have failed with:
    //   An uncaught exception of type com.sun.star.lang.IllegalArgumentException
    // because of disallowed external link update (needed to obtain the cell value).
    css::uno::Any aResult = xFunc->callFunction(u"INDIRECT"_ustr, {css::uno::Any(aReference)});
    CPPUNIT_ASSERT_EQUAL(css::uno::Any(u"a1"_ustr), aResult);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf147122)
{
    createScDoc();

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(
            u"Function TestMergedSelection\n"
                     // Insert test string into cell A1
                     "  oActiveSheet = ThisComponent.CurrentController.ActiveSheet\n"
                     "  oActiveCell = oActiveSheet.getCellRangeByName(\"A1\")\n"
                     "  oActiveCell.setString(\"This is a test\")\n"
                     // Merge A1:B2 cell range and return the content of the merged range
                     "  oRange = oActiveSheet.getCellRangeByName(\"A1:B2\")\n"
                     "  ThisComponent.getCurrentController.Select(oRange)\n"
                     "  oActiveCell = ThisComponent.CurrentSelection\n"
                     "  oActiveCell.Merge(True)\n"
                     "  TestMergedSelection = ThisComponent.getCurrentSelection().getString()\n"
                     "End Function\n"_ustr));

    Any aRet = executeMacro(u"vnd.sun.Star.script:TestLibrary.TestModule.TestMergedSelection?"
                            "language=Basic&location=document"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expression: false
    // - Unexpected dialog: Error: BASIC runtime error.
    // Property or method not found: getString.
    CPPUNIT_ASSERT_EQUAL(Any(u"This is a test"_ustr), aRet);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf154803)
{
    createScDoc();

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(
            u"Function TestExtendedMergedSelection\n"
                     // Merge A1:B2 cell range
                     "  oActiveSheet = ThisComponent.CurrentController.ActiveSheet\n"
                     "  oRange = oActiveSheet.getCellRangeByName(\"A1:B2\")\n"
                     "  ThisComponent.getCurrentController.Select(oRange)\n"
                     "  oActiveCell = ThisComponent.CurrentSelection\n"
                     "  oActiveCell.Merge(True)\n"
                     // Select A1:B3 range and check for its implementation name
                     "  oRange = oActiveSheet.getCellRangeByName(\"A1:B3\")\n"
                     "  ThisComponent.getCurrentController.Select(oRange)\n"
                     "  TestExtendedMergedSelection = ThisComponent.CurrentSelection.ImplementationName\n"
                     "End Function\n"_ustr));

    Any aRet = executeMacro(u"vnd.sun.Star.script:TestLibrary.TestModule.TestExtendedMergedSelection?"
                            "language=Basic&location=document"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expected : ScCellRangeObj
    // - Actual   : ScCellObj
    // i.e. the selection was interpreted as a single cell instead of a range
    CPPUNIT_ASSERT_EQUAL(Any(u"ScCellRangeObj"_ustr), aRet);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf116127)
{
    createScDoc();

    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary(u"TestLibrary"_ustr);
    xLibrary->insertByName(
        u"TestModule"_ustr,
        uno::Any(u"Function TestClearContents\n"
            // Insert test string into cell A1
            "  oActiveSheet = ThisComponent.CurrentController.ActiveSheet\n"
            "  oActiveCell = oActiveSheet.getCellRangeByName(\"A1\")\n"
            "  oActiveCell.setString(\"Italic Test\")\n"
            // Create a text cursor and change the first letter to italic
            "  oCursor = oActiveCell.Text.createTextCursor()\n"
            "  oCursor.gotoStart(False)\n"
            "  oCursor.goRight(1, True)\n"
            "  oCursor.CharPosture = com.sun.star.awt.FontSlant.ITALIC\n"
            // Clear contents using EDITATTR cell flag to clear the italic char posture
            "  oActiveCell.clearContents(com.sun.star.sheet.CellFlags.EDITATTR)\n"
            // Check the char posture of the first letter
            "  oCursor.gotoStart(False)\n"
            "  oCursor.goRight(1, True)\n"
            "  TestClearContents = oCursor.CharPosture <> com.sun.star.awt.FontSlant.ITALIC\n"
            "End Function\n"_ustr));

    Any aRet = executeMacro(u"vnd.sun.Star.script:TestLibrary.TestModule.TestClearContents?"
                            "language=Basic&location=document"_ustr);
    // Without the fix in place, this test would have failed with
    // - Expected : true
    // - Actual   : false
    // i.e. the formatting within parts of the cell contents (EDITATTR) were not deleted
    CPPUNIT_ASSERT_EQUAL(Any(true), aRet);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf159412)
{
    // Run a macro, that itself calls two other functions using invoke,
    // passing a small integer value to arguments of types Long and Double
    createScDoc("tdf159412.fods");

    css::uno::Any aRet;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    css::uno::Sequence<css::uno::Any> aParams;

    SfxObjectShell::CallXScript(
        mxComponent,
        u"vnd.sun.Star.script:Standard.Module1.TestInvoke?language=Basic&location=document"_ustr,
        aParams, aRet, aOutParamIndex, aOutParam);

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: 1 Long/2 Double
    // - Actual  : 0 Long/0 Double
    // i.e., the passed 1 and 2 values were lost.

    CPPUNIT_ASSERT_EQUAL(u"1 Long/2 Double"_ustr, aReturnValue);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf47479)
{
    createScDoc();
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
    auto xSheets = xDoc->getSheets().queryThrow<container::XIndexAccess>();
    auto xSheet = xSheets->getByIndex(0).queryThrow<sheet::XCellRangeMovement>();
    auto xSheetAddressable = xSheet.queryThrow<sheet::XCellRangeAddressable>();
    auto xColRowRange = xSheet.queryThrow<table::XColumnRowRange>();
    auto xColAddressable
        = xColRowRange->getColumns()->getByIndex(1).queryThrow<sheet::XCellRangeAddressable>();
    auto xRowAddressable
        = xColRowRange->getRows()->getByIndex(1).queryThrow<sheet::XCellRangeAddressable>();
    css::table::CellRangeAddress origSheetRange = xSheetAddressable->getRangeAddress();
    css::table::CellRangeAddress origColRange = xColAddressable->getRangeAddress();
    css::table::CellRangeAddress origRowRange = xRowAddressable->getRangeAddress();
    css::table::CellRangeAddress addressToRemove(origSheetRange.Sheet, 1, 1, 1, 1);
    CPPUNIT_ASSERT_EQUAL(SCTAB(0), origSheetRange.Sheet);

    xSheet->removeRange(addressToRemove, css::sheet::CellDeleteMode_UP);

    auto currentRange = xSheetAddressable->getRangeAddress();
    CPPUNIT_ASSERT_EQUAL(origSheetRange.Sheet, currentRange.Sheet);
    CPPUNIT_ASSERT_EQUAL(origSheetRange.StartColumn, currentRange.StartColumn);
    CPPUNIT_ASSERT_EQUAL(origSheetRange.StartRow, currentRange.StartRow);
    CPPUNIT_ASSERT_EQUAL(origSheetRange.EndColumn, currentRange.EndColumn);
    // Without the fix, this would fail with
    // - Expected: 1048575
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(origSheetRange.EndRow, currentRange.EndRow);

    xSheet->removeRange(addressToRemove, css::sheet::CellDeleteMode_LEFT);

    currentRange = xColAddressable->getRangeAddress();
    CPPUNIT_ASSERT_EQUAL(origColRange.Sheet, currentRange.Sheet);
    CPPUNIT_ASSERT_EQUAL(origColRange.StartColumn, currentRange.StartColumn);
    // Without the fix, this would fail with
    // - Expected: 0
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(origColRange.StartRow, currentRange.StartRow);
    CPPUNIT_ASSERT_EQUAL(origColRange.EndColumn, currentRange.EndColumn);
    CPPUNIT_ASSERT_EQUAL(origColRange.EndRow, currentRange.EndRow);

    xSheet->removeRange(origColRange, css::sheet::CellDeleteMode_UP);

    currentRange = xRowAddressable->getRangeAddress();
    CPPUNIT_ASSERT_EQUAL(origRowRange.Sheet, currentRange.Sheet);
    // Without the fix, this would fail with
    // - Expected: 0
    // - Actual  : 2
    CPPUNIT_ASSERT_EQUAL(origRowRange.StartColumn, currentRange.StartColumn);
    CPPUNIT_ASSERT_EQUAL(origRowRange.StartRow, currentRange.StartRow);
    CPPUNIT_ASSERT_EQUAL(origRowRange.EndColumn, currentRange.EndColumn);
    CPPUNIT_ASSERT_EQUAL(origRowRange.EndRow, currentRange.EndRow);

    // tdf#167178: make sure that adding a sheet before the current one keeps the addressables
    // pointing to the correct sheet

    ScDocument& rDoc = *getScDoc();
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), rDoc.GetTableCount());

    dispatchCommand(mxComponent, u".uno:Insert"_ustr,
                    { comphelper::makePropertyValue(u"Name"_ustr, u"NewTab"_ustr),
                      comphelper::makePropertyValue(u"Index"_ustr, sal_Int16(1)) });

    CPPUNIT_ASSERT_EQUAL(SCTAB(2), rDoc.GetTableCount());

    // Without the fix, these were 0.
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), xSheetAddressable->getRangeAddress().Sheet);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), xColAddressable->getRangeAddress().Sheet);
    CPPUNIT_ASSERT_EQUAL(SCTAB(1), xRowAddressable->getRangeAddress().Sheet);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf161948NaturalSortAPI)
{
    // Since LO 26.2 the feature natural sort is available in the API.
    // Here we test, that is can be used in Basic macros.
    createScDoc("tdf161948_NaturalSort.ods");
    ScDocument* pDoc = getScDoc();
    // The source has "K3", "K10", "K104", "K23", "K2" in Range A2:A6 and label in A1.
    // The result goes to range C1:C6.

    // Enable natural sorting "double" and sort. Examine cell C2
    executeMacro(u"vnd.sun.star.script:Standard.SortTest.EnableNaturalSort"
                 "?language=Basic&location=document"_ustr);
    OUString sCellContent = pDoc->GetString(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(u"K2"_ustr, sCellContent);

    // Enable default alphanumerically sorting and sort. Examine cell C2.
    executeMacro(u"vnd.sun.star.script:Standard.SortTest.EnableAlphaNumericSort"
                 "?language=Basic&location=document"_ustr);
    sCellContent = pDoc->GetString(2, 1, 0);
    CPPUNIT_ASSERT_EQUAL(u"K10"_ustr, sCellContent);
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf81003_DateCellToVbaUDF)
{
    // The document contains a formula in B1 with a user-defined spreadsheet function from a
    // VBA-compatibility module, taking a date from A1.
    // Without the fix, this failed with
    // - Expression: xComponent.is()
    // - loading failed: file://.../vba-date.fods
    // because there was a failure evaluating the function at the loading time, and loading failed.
    // The reason was that the variable representing the cell as a VBA-compatibility object, with a
    // css::bridge::oleautomation::Date as its value, didn't convert to Basic's Date when passed to
    // the user-defined function:
    createScDoc("vba-date.fods");
    ScDocument* pDoc = getScDoc();
    // Check value of B1. The serial date for 2025-08-29 is 45898.
    CPPUNIT_ASSERT_EQUAL(45898.0, pDoc->GetValue(1, 0, 0));
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf161948NaturalSortDispatcher)
{
    // Since LibreOffice 26.2 the feature natural sort is available in the API. Prior to that,
    // natural sort by macro was only possible by using the dispatcher.
    // Here we test with a Basic macro, that the old way still works.
    createScDoc("tdf161948_NaturalSort_OldWay.ods");
    ScDocument* pDoc = getScDoc();

    // The source has "ID", "K3", "K10", "K104", "K23", "K2" in Range A1:A6.

    // The macro enables natural sorting via parameter NaturalSort=true of SID_SORT.
    executeMacro(u"vnd.sun.star.script:Standard.SortTest.Dispatcher_NaturalSort"
                 "?language=Basic&location=document"_ustr);

    // Verify sort result. Sorting via dispatcher is always 'inplace' thus results are in A1:A6.
    const std::array<OUString, 6> aExpectedNaturalSort
        = { u"ID"_ustr, u"K2"_ustr, u"K3"_ustr, u"K10"_ustr, u"K23"_ustr, u"K104"_ustr };
    for (SCROW nRow = 0; nRow <= 5; nRow++) // ScAddress(col, row, tab)
    {
        CPPUNIT_ASSERT_EQUAL(aExpectedNaturalSort[nRow], pDoc->GetString(0, nRow, 0));
    }

    // Same test for alpha-numeric sort, that is NaturalSort=false.
    executeMacro(u"vnd.sun.star.script:Standard.SortTest.Dispatcher_AlphaNumeric"
                 "?language=Basic&location=document"_ustr);

    // Verify sort result
    const std::array<OUString, 6> aExpectedAlphaNumeric
        = { u"ID"_ustr, u"K10"_ustr, u"K104"_ustr, u"K2"_ustr, u"K23"_ustr, u"K3"_ustr };
    for (SCROW nRow = 0; nRow <= 5; nRow++)
    {
        CPPUNIT_ASSERT_EQUAL(aExpectedAlphaNumeric[nRow], pDoc->GetString(0, nRow, 0));
    }
}

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testTdf168750)
{
    // A class module's global names must not be visible without instantiated object
    createScDoc("udf_plus_class_module.ods");
    ScDocument* pDoc = getScDoc();

    // B1 must have "FALSE", i.e. no errors happened during the load time.
    // Without the fix, this would fail with
    // - Unexpected dialog:  Error: BASIC runtime error. Argument is not optional.
    // Which indicates, that the class module function was unexpectedly called.
    CPPUNIT_ASSERT_EQUAL(u"FALSE"_ustr, pDoc->GetString(1, 0, 0));
}

#if defined(_WIN32) // DDE calls work only on Windows currently
CPPUNIT_TEST_FIXTURE(ScMacrosTest, testDdePoke)
{
    // Get a document with a text in A1, and use its Basic macro to send that text into the
    // same document's B2 using DDEPoke call:
    createScDoc("DdePoke.fods");
    ScDocument* pDoc = getScDoc();
    // A1 has a text:
    CPPUNIT_ASSERT_EQUAL(u"Hello from Sender"_ustr, pDoc->GetString(0, 0, 0));
    // B2 is empty initially:
    CPPUNIT_ASSERT(pDoc->GetString(1, 1, 0).isEmpty());

    executeMacro(u"vnd.sun.star.script:Standard.Module1.SendDataWithDDEPoke"
                 "?language=Basic&location=document"_ustr);

    // B2 has the expected text now:
    CPPUNIT_ASSERT_EQUAL(u"Hello from Sender"_ustr, pDoc->GetString(1, 1, 0));
}
#endif

CPPUNIT_TEST_FIXTURE(ScMacrosTest, testVbaOnTime)
{
    // A document with a sub that sets value in its A1, and a sub scheduling the other sub's
    // execution using Application.OnTime:
    createScDoc("OnTime.fods");
    ScDocument* pDoc = getScDoc();
    // A1 is empty initially:
    CPPUNIT_ASSERT(pDoc->IsEmptyData(0, 0, 0, 0, 0));

    executeMacro(u"vnd.sun.star.script:Standard.Module1.ScheduleOnTime"
                 "?language=Basic&location=document"_ustr);

    // A1 is still empty, because the timer has not yet fired:
    CPPUNIT_ASSERT(pDoc->IsEmptyData(0, 0, 0, 0, 0));

    // Let the timer fire and run the OnTime sub. The minimal timeout of VbaTimer is 50 ms:
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(50ms);
    Scheduler::ProcessEventsToIdle();

    // A1 has the expected number now:
    CPPUNIT_ASSERT_EQUAL(42.0, pDoc->GetValue(0, 0, 0));
}

ScMacrosTest::ScMacrosTest()
      : ScModelTestBase(u"/sc/qa/extras/testdocuments"_ustr)
{
}

void ScMacrosTest::setUp()
{
    Application::SetAppName("CppunitTest_sc_macros_test");
    ScModelTestBase::setUp();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
