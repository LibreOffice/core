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
#include <sal/log.hxx>
#include <unotools/tempfile.hxx>
#include <svx/svdpage.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <conditio.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <scitems.hxx>

#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>

#include <com/sun/star/script/XLibraryContainerPassword.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <editeng/brushitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Macros test */

class ScMacrosTest : public UnoApiXmlTest
{
protected:
    void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;
public:
    ScMacrosTest();

    void testStarBasic();
    void testMSP();
    void testPasswordProtectedStarBasic();
    void testTdf114427();
    void testRowColumn();
    void testTdf104902();
    void testTdf64639();
    void testTdf142033();
    void testPasswordProtectedUnicodeString();
    void testPasswordProtectedArrayInUserType();
    void testTdf131296_legacy();
    void testTdf131296_new();
    void testTdf46119();
    void testTdf128218();
    void testTdf71271();
    void testTdf43003();
    void testTdf75263();
    void testTdf133887();
    void testTdf133889();
    void testTdf144970();
    void testTdf138646();
    void testTdf105558();
    void testTdf143582();
    void testTdf144085();
    void testTdf125800();
    void testTdf130307();
    void testTdf146742();
    void testMacroButtonFormControlXlsxExport();
    void testTdf107572();
    void testShapeLayerId();
    void testFunctionAccessIndirect();

    CPPUNIT_TEST_SUITE(ScMacrosTest);
    CPPUNIT_TEST(testStarBasic);
    CPPUNIT_TEST(testMSP);
    CPPUNIT_TEST(testPasswordProtectedStarBasic);
    CPPUNIT_TEST(testTdf114427);
    CPPUNIT_TEST(testRowColumn);
    CPPUNIT_TEST(testTdf104902);
    CPPUNIT_TEST(testTdf64639);
    CPPUNIT_TEST(testTdf142033);
    CPPUNIT_TEST(testPasswordProtectedUnicodeString);
    CPPUNIT_TEST(testPasswordProtectedArrayInUserType);
    CPPUNIT_TEST(testTdf131296_legacy);
    CPPUNIT_TEST(testTdf131296_new);
    CPPUNIT_TEST(testTdf46119);
    CPPUNIT_TEST(testTdf128218);
    CPPUNIT_TEST(testTdf128218);
    CPPUNIT_TEST(testTdf71271);
    CPPUNIT_TEST(testTdf43003);
    CPPUNIT_TEST(testTdf75263);
    CPPUNIT_TEST(testTdf133887);
    CPPUNIT_TEST(testTdf133889);
    CPPUNIT_TEST(testTdf144970);
    CPPUNIT_TEST(testTdf138646);
    CPPUNIT_TEST(testTdf105558);
    CPPUNIT_TEST(testTdf143582);
    CPPUNIT_TEST(testTdf144085);
    CPPUNIT_TEST(testTdf125800);
    CPPUNIT_TEST(testTdf130307);
    CPPUNIT_TEST(testTdf146742);
    CPPUNIT_TEST(testMacroButtonFormControlXlsxExport);
    CPPUNIT_TEST(testTdf107572);
    CPPUNIT_TEST(testShapeLayerId);
    CPPUNIT_TEST(testFunctionAccessIndirect);

    CPPUNIT_TEST_SUITE_END();
};

void ScMacrosTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
}

// I suppose you could say this test doesn't really belong here, OTOH
// we need a full document to run the test ( it related originally to an
// imported Excel VBA macro ) It's convenient and fast to unit test
// this the problem this way. Perhaps in the future there will be some sort
// of slowcheck tests ( requiring a full document environment in the scripting
// module, we could move the test there then ) - relates to fdo#67547
void ScMacrosTest::testMSP()
{
    loadFromURL(u"MasterScriptProviderProblem.ods");

    Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1.TestMSP?language=Basic&location=document");
    OUString sResult;
    aRet >>= sResult;

    SAL_INFO("sc.qa", "Result is " << sResult );
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TestMSP ( for fdo#67547) failed", OUString("OK"), sResult);
}

void ScMacrosTest::testPasswordProtectedStarBasic()
{
    loadFromURL(u"testTypePassword.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    // User defined types

    executeMacro("vnd.sun.Star.script:Standard.Module1.LoadAndExecuteTest?language=Basic&location=document");

    OUString aValue = rDoc.GetString(0,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("User defined types script did not change the value of Sheet1.A1", OUString("success"), aValue);

    // Big Module

    executeMacro("vnd.sun.Star.script:MyLibrary.BigModule.bigMethod?language=Basic&location=document");

    aValue = rDoc.GetString(1,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Big module script did not change the value of Sheet1.B1", OUString("success"), aValue);

    // far big method tdf#94617

    executeMacro("vnd.sun.Star.script:MyLibrary.BigModule.farBigMethod?language=Basic&location=document");

    aValue = rDoc.GetString(2,0,0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Far Method script did not change the value of Sheet1.C1", OUString("success"), aValue);
}

void ScMacrosTest::testStarBasic()
{
    loadFromURL(u"StarBasic.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    executeMacro("vnd.sun.Star.script:Standard.Module1.Macro1?language=Basic&location=document");
    double aValue = rDoc.GetValue(0,0,0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("script did not change the value of Sheet1.A1",2.0, aValue, 0.00001);
}

void ScMacrosTest::testRowColumn()
{
    loadFromURL(u"StarBasic.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    executeMacro("vnd.sun.Star.script:Standard.Module1.Macro_RowHeight?language=Basic&location=document");

    sal_uInt16 nHeight = o3tl::convert(rDoc.GetRowHeight(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(2000), nHeight);

    executeMacro("vnd.sun.Star.script:Standard.Module1.Macro_ColumnWidth?language=Basic&location=document");
    sal_uInt16 nWidth  = o3tl::convert(rDoc.GetColWidth(0, 0), o3tl::Length::twip, o3tl::Length::mm100);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(4001), nWidth);
}

void ScMacrosTest::testTdf146742()
{
    loadFromURL(u"tdf146742.ods");

    // Export to ODS and reload the file
    saveAndReload("calc8");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("1"), rDoc.GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("2"), rDoc.GetString(ScAddress(0,1,0)));

    CPPUNIT_ASSERT_EQUAL(OUString("TRUE"), rDoc.GetString(ScAddress(1,0,0)));
    // Without the fix in place, this test would have failed with
    // - Expected: FALSE
    // - Actual  : TRUE
    CPPUNIT_ASSERT_EQUAL(OUString("FALSE"), rDoc.GetString(ScAddress(1,1,0)));
}

void ScMacrosTest::testMacroButtonFormControlXlsxExport()
{
    // Given a button form control with an associated macro:
    loadFromURL(u"macro-button-form-control.xlsm");

    // When exporting to XLSM:
    save("Calc MS Excel 2007 VBA XML");

    // Then make sure that the macro is associated with the control:
    xmlDocUniquePtr pSheetDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pSheetDoc);
    // Without the fix in place, this test would have failed with:
    // - XPath '//x:controlPr' no attribute 'macro' exist
    // i.e. the macro was lost on export.
    assertXPath(pSheetDoc, "//x:controlPr", "macro", "Module1.Button1_Click");

    // Then also make sure that there is no defined name for the macro, which is only needed for
    // XLS:
    xmlDocUniquePtr pWorkbookDoc = parseExport("xl/workbook.xml");
    CPPUNIT_ASSERT(pWorkbookDoc);
    assertXPath(pWorkbookDoc, "//x:workbook/definedNames", 0);
}

void ScMacrosTest::testTdf104902()
{
    loadFromURL(u"tdf104902.ods");

    executeMacro("vnd.sun.Star.script:Standard.Module1.display_bug?language=Basic&location=document");

    // Export to ODS
    saveAndReload("calc8");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("string no newlines"), rDoc.GetString(ScAddress(0, 0, 0)));

    // Without the fix in place, this test would have failed with
    // - Expected: string with
    // newlines
    // - Actual  : string withnewlines
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), rDoc.GetString(ScAddress(0, 1, 0)));
}

void ScMacrosTest::testTdf64639()
{
    loadFromURL(u"tdf64639.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    // Add and delete the chart a few times
    // Without the fix in place, this test would have crashed here
    for (size_t i = 0; i < 5; ++i)
    {
        executeMacro("vnd.sun.Star.script:Standard.Module1.DrawGraph?language=Basic&location=document");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

        executeMacro("vnd.sun.Star.script:Standard.Module1.DeleteGraph?language=Basic&location=document");

        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());
    }
}

void ScMacrosTest::testTdf142033()
{
    loadFromURL(u"tdf142033.ods");

    executeMacro("vnd.sun.Star.script:Standard.Module1.display_bug?language=Basic&location=document");

    // Export to ODS
    saveAndReload("calc8");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(OUString("string no newlines"), rDoc.GetString(ScAddress(0,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString("string no newlines"), rDoc.GetString(ScAddress(0,1,0)));

    // Without the fix in place, this test would have failed with
    // - Expected: string with
    // newlines
    // - Actual  : string withnewlines
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), rDoc.GetString(ScAddress(1,0,0)));
    CPPUNIT_ASSERT_EQUAL(OUString(u"string with" + OUStringChar(u'\xA') + u"newlines"), rDoc.GetString(ScAddress(1,1,0)));
}

void ScMacrosTest::testPasswordProtectedUnicodeString()
{
    const OUString sCorrectString(u"English Русский 中文");
    static const OUStringLiteral sMacroURL(
        u"vnd.sun.Star.script:Protected.Module1.TestUnicodeString?language=Basic&location=document");
    static const OUStringLiteral sLibName(u"Protected");

    loadFromURL(u"tdf57113.ods");

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
    CPPUNIT_ASSERT(xPasswd->verifyLibraryPassword(sLibName, "password"));
    xLC->loadLibrary(sLibName);
    CPPUNIT_ASSERT(xLC->isLibraryLoaded(sLibName));

    // Now check that saving stores Unicode data correctly in image's string pool
    saveAndReload("calc8");

    {
        Any aRet = executeMacro(sMacroURL);

        OUString aReturnValue;
        aRet >>= aReturnValue;
        CPPUNIT_ASSERT_EQUAL(sCorrectString, aReturnValue);
    }
}

void ScMacrosTest::testPasswordProtectedArrayInUserType()
{
    static const OUStringLiteral sMacroURL(
        u"vnd.sun.Star.script:Protected.Module1.TestMyType?language=Basic&location=document");
    static const OUStringLiteral sLibName(u"Protected");

    loadFromURL(u"ProtectedArrayInCustomType.ods");

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
    CPPUNIT_ASSERT(xPasswd->verifyLibraryPassword(sLibName, "password"));
    xLC->loadLibrary(sLibName);
    CPPUNIT_ASSERT(xLC->isLibraryLoaded(sLibName));

    // Now check that saving stores array bounds correctly
    saveAndReload("calc8");

    {
        Any aRet = executeMacro(sMacroURL);

        sal_Int16 nReturnValue;
        aRet >>= nReturnValue;
        CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nReturnValue);
    }
}

void ScMacrosTest::testTdf114427()
{
    loadFromURL(u"tdf114427.ods");

    uno::Reference< sheet::XSpreadsheetDocument > xDoc(mxComponent, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< drawing::XDrawPageSupplier > xDrawPageSupplier( xIA->getByIndex(0), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xDraws(xDrawPageSupplier->getDrawPage(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDraws->getCount());

    // Without the fix in place, it would have crashed here
    executeMacro("vnd.sun.Star.script:Standard.Module1.DeletingFrame?language=Basic&location=document");

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), xDraws->getCount());
}

void ScMacrosTest::testTdf131296_legacy()
{
    // For legacy password-protected library images, we must correctly get the constants' values,
    // and also - for Integer - the type.
    const std::vector<std::pair<OUString, OUString>> aTests({
        { "TestIntConst", "Integer: 123" },
        { "TestLongConst", "Double: 123" },
        { "TestSingleConst", "Double: 123" },
        { "TestDoubleConst", "Double: 123" },
    });

    loadFromURL(u"tdf131296_legacy.ods");
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

void ScMacrosTest::testTdf131296_new()
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

    loadFromURL(u"tdf131296_new.ods");
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

void ScMacrosTest::testTdf46119()
{
    loadFromURL(u"tdf46119.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    executeMacro("vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document");

    CPPUNIT_ASSERT_EQUAL(OUString("0.074"), rDoc.GetString(ScAddress(2, 24, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.067"), rDoc.GetString(ScAddress(2, 25, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.273"), rDoc.GetString(ScAddress(2, 26, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.259"), rDoc.GetString(ScAddress(2, 27, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("0.097"), rDoc.GetString(ScAddress(3, 24, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.087"), rDoc.GetString(ScAddress(3, 25, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.311"), rDoc.GetString(ScAddress(3, 26, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.296"), rDoc.GetString(ScAddress(3, 27, 0)));

    CPPUNIT_ASSERT_EQUAL(OUString("0.149"), rDoc.GetString(ScAddress(4, 24, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.134"), rDoc.GetString(ScAddress(4, 25, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.386"), rDoc.GetString(ScAddress(4, 26, 0)));
    CPPUNIT_ASSERT_EQUAL(OUString("0.366"), rDoc.GetString(ScAddress(4, 27, 0)));
}

void ScMacrosTest::testTdf128218()
{
    loadFromURL(u"tdf128218.ods");

    Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1.TestRAND?language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: Double
    // - Actual  : Object()

    CPPUNIT_ASSERT_EQUAL(OUString("Double"), aReturnValue);
}

void ScMacrosTest::testTdf71271()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
        xProps->setPropertyValue("CodeName", uno::Any(OUString("NewCodeName")));
    }

    saveAndReload("");

    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIndex(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XSpreadsheet> xSheet(xIndex->getByIndex(0), uno::UNO_QUERY_THROW);
        OUString sCodeName;
        uno::Reference<beans::XPropertySet> xProps(xSheet, uno::UNO_QUERY_THROW);
        // Without the fix in place the codename would not have been saved
        xProps->getPropertyValue("CodeName") >>= sCodeName;
        CPPUNIT_ASSERT_EQUAL(OUString("NewCodeName"), sCodeName);
    }
}

void ScMacrosTest::testTdf43003()
{
    loadFromURL(u"tdf43003.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, the values of the specified cells won't be changed
    rDoc.SetValue(ScAddress(0, 0, 0), 2);
    CPPUNIT_ASSERT_EQUAL(3.0, rDoc.GetValue(ScAddress(1, 0, 0)));
    CPPUNIT_ASSERT_EQUAL(4.0, rDoc.GetValue(ScAddress(2, 0, 0)));
}


void ScMacrosTest::testTdf75263()
{
    loadFromURL(u"tdf75263.xlsm");

    {
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
        ScDocument& rDoc = pDocSh->GetDocument();
        rDoc.CalcAll();

        // A1 contains formula with user-defined function, and the function is defined in VBA.
        CPPUNIT_ASSERT_EQUAL(OUString(u"проба"), rDoc.GetString(ScAddress(0, 0, 0)));
    }

    saveAndReload("Calc MS Excel 2007 VBA XML");

    {
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
        ScDocument& rDoc = pDocSh->GetDocument();
        rDoc.CalcAll();

        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: проба (sample)
        // - Actual  : ?????
        CPPUNIT_ASSERT_EQUAL(OUString(u"проба"), rDoc.GetString(ScAddress(0, 0, 0)));
    }
}

void ScMacrosTest::testTdf133887()
{
    loadFromURL(u"tdf133887.ods");

    css::uno::Any aRet;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    css::uno::Sequence<css::uno::Any> aParams{ css::uno::Any(sal_Int16(0)) };

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:Standard.Module1.TestInvoke?language=Basic&location=document", aParams,
        aRet, aOutParamIndex, aOutParam);

    double aReturnValue;
    aOutParam[0] >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: 6.75
    // - Actual  : 7

    CPPUNIT_ASSERT_EQUAL(6.75, aReturnValue);
}

void ScMacrosTest::testTdf133889()
{
    loadFromURL(u"tdf133889.ods");

    css::uno::Any aRet;
    css::uno::Sequence<sal_Int16> aOutParamIndex;
    css::uno::Sequence<css::uno::Any> aOutParam;
    css::uno::Sequence<css::uno::Any> aParams{ css::uno::Any(sal_Int32(0)) };

    SfxObjectShell::CallXScript(
        mxComponent,
        "vnd.sun.Star.script:Standard.Module1.TestInvoke?language=Basic&location=document", aParams,
        aRet, aOutParamIndex, aOutParam);

    sal_Int32 aReturnValue;
    aOutParam[0] >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: 100000
    // - Actual  : 0

    CPPUNIT_ASSERT_EQUAL(sal_Int32(100000), aReturnValue);
}

void ScMacrosTest::testTdf143582()
{
    loadFromURL(u"tdf143582.ods");

    Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1.TestScriptInvoke?language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: Test6
    // - Actual  : TeTest8
    CPPUNIT_ASSERT_EQUAL(OUString("Test6"), aReturnValue);
}

void ScMacrosTest::testTdf144085()
{
    loadFromURL(u"tdf144085.ods");

    Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1.TestScriptInvoke?language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have failed with
    // - Expected: $Sheet1.$B$5:$E$17
    // - Actual  : $Sheet1.$B$5:$C$10
    CPPUNIT_ASSERT_EQUAL(OUString("$Sheet1.$B$5:$E$17"), aReturnValue);
}

void ScMacrosTest::testTdf125800()
{
    loadFromURL(u"tdf125800.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    ScConditionalFormat* pFormat = rDoc.GetCondFormat(1, 2, 0);
    CPPUNIT_ASSERT(!pFormat);

    // Without the fix in place, this test would have failed with
    // - Expression: false
    // - Unexpected dialog:  Error: Inadmissible value or data type. Index out of defined range.
    Any aRet = executeMacro("vnd.sun.Star.script:Standard.cf.doItForThisSheetindexThisRange?language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;

    pFormat = rDoc.GetCondFormat(1, 2, 0);
    CPPUNIT_ASSERT(pFormat);

    const ScFormatEntry* pEntry = pFormat->GetEntry(0);
    CPPUNIT_ASSERT(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScFormatEntry::Type::Condition, pEntry->GetType());

    const ScCondFormatEntry* pCondition = static_cast<const ScCondFormatEntry*>(pEntry);
    CPPUNIT_ASSERT_EQUAL(ScConditionMode::Direct, pCondition->GetOperation());
}

void ScMacrosTest::testTdf130307()
{
    loadFromURL(u"tdf130307.ods");

    Any aRet = executeMacro("vnd.sun.Star.script:Standard.Module1.ForEachSheets?language=Basic&location=document");

    OUString aReturnValue;
    aRet >>= aReturnValue;

    // Without the fix in place, this test would have crashed here
    CPPUNIT_ASSERT_EQUAL(OUString("Sheet1Sheet2"), aReturnValue);
}

void ScMacrosTest::testTdf144970()
{
    loadFromURL(u"tdf144970.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_A1;
    rDoc.SetGrammar(eGram);

    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    const SdrPage* pPage = pDrawLayer->GetPage(0);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pPage->GetObjCount());

    // Without the fix in place, this test would have failed with
    // - Expression: false
    // - Unexpected dialog:  Error: BASIC runtime error.
    // An exception occurred
    // Type: com.sun.star.lang.IllegalArgumentException
    executeMacro("vnd.sun.Star.script:Standard.Module1.Main?language=Basic&location=document");

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());
}

void ScMacrosTest::testTdf138646()
{
    loadFromURL(u"tdf138646.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Without the fix in place, changing the grammar from GRAM_NATIVE to either GRAM_NATIVE_XL_A1
    // or GRAM_NATIVE_XL_R1C1 would cause a Basic exception/error in the following script.
    pDocSh->GetDocument().SetGrammar(formula::FormulaGrammar::Grammar::GRAM_NATIVE_XL_R1C1);

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

void ScMacrosTest::testTdf105558()
{
    loadFromURL(u"tdf105558.ods");

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT(pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);
    ScDocument& rDoc = xDocSh->GetDocument();

    // Without the fix in place, this test would have failed with
    // - Expected: 5.5
    // - Actual  : 0
    CPPUNIT_ASSERT_EQUAL(5.5, rDoc.GetValue(ScAddress(0, 0, 0)));
}

void ScMacrosTest::testTdf107572()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName(
        "TestModule",
        uno::Any(
            OUString("Function Main\n"
                     "  thisComponent.Sheets(0).getCellRangeByName(\"A1:F14\").autoformat(\"Default\")\n"
                     "End Function\n")));

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);

    // Without the fix in place, this test would have crashed
    executeMacro("vnd.sun.Star.script:TestLibrary.TestModule.Main?language=Basic&location=document");

    ScDocument& rDoc = pDocSh->GetDocument();

    //Check the autoformat has been applied
    for (SCCOL i = 0; i < 5; ++i)
    {
        const ScPatternAttr* pAttr = rDoc.GetPattern(i, 0, 0);
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();

        CPPUNIT_ASSERT_EQUAL(Color(0x0, 0x0, 0x80), rColor);
    }

    for (SCROW i = 1; i < 13; ++i)
    {
        const ScPatternAttr* pAttr = rDoc.GetPattern(0, i, 0);
        const SfxPoolItem& rItem = pAttr->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground = static_cast<const SvxBrushItem&>(rItem);
        const Color& rColor = rBackground.GetColor();

        CPPUNIT_ASSERT_EQUAL(Color(0x4d, 0x4d, 0x4d), rColor);

        const ScPatternAttr* pAttr2 = rDoc.GetPattern(5, i, 0);
        const SfxPoolItem& rItem2 = pAttr2->GetItem(ATTR_BACKGROUND);
        const SvxBrushItem& rBackground2 = static_cast<const SvxBrushItem&>(rItem2);
        const Color& rColor2 = rBackground2.GetColor();

        CPPUNIT_ASSERT_EQUAL(Color(0xcc, 0xcc, 0xcc), rColor2);
    }
}

void ScMacrosTest::testShapeLayerId()
{
    mxComponent = loadFromDesktop("private:factory/scalc");

    // insert initial library
    css::uno::Reference<css::document::XEmbeddedScripts> xDocScr(mxComponent, UNO_QUERY_THROW);
    auto xLibs = xDocScr->getBasicLibraries();
    auto xLibrary = xLibs->createLibrary("TestLibrary");
    xLibrary->insertByName(
        "TestModule",
        uno::Any(
            OUString("Function TestLayerID\n"
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
                     "End Function\n")));

    Any aRet = executeMacro("vnd.sun.Star.script:TestLibrary.TestModule.TestLayerID?language=Basic&location=document");
    // Without the fix in place, this test would have failed in non-debug builds with
    // - Expected : <Any: (string) 0 Expected runtime error happened>
    // - Actual   : <Any: (string) 0 1>
    // In debug builds, it would crash on assertion inside strong_int ctor.
    // The LayerID property of com.sun.star.drawing.Shape service has 'short' IDL type.
    // The expected run-time error is because there are only 5 layers there.
    CPPUNIT_ASSERT_EQUAL(Any(OUString("0 Expected runtime error happened")), aRet);
}

void ScMacrosTest::testFunctionAccessIndirect()
{
    OUString aFileName = loadFromURL(u"tdf120161.ods"); // just some document with known values in cells

    const OUString aReference = "'" + aFileName + "'#$Sheet1.A1";

    css::uno::Reference<css::sheet::XFunctionAccess> xFunc(
        comphelper::getProcessServiceFactory()->createInstance("com.sun.star.sheet.FunctionAccess"),
        UNO_QUERY_THROW);

    // tdf#148040: without the fix in place, this would have failed with:
    //   An uncaught exception of type com.sun.star.lang.IllegalArgumentException
    // because of disallowed external link update (needed to obtain the cell value).
    css::uno::Any aResult = xFunc->callFunction("INDIRECT", {css::uno::Any(aReference)});
    CPPUNIT_ASSERT_EQUAL(css::uno::Any(OUString("a1")), aResult);
}

ScMacrosTest::ScMacrosTest()
      : UnoApiXmlTest("/sc/qa/extras/testdocuments")
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScMacrosTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
