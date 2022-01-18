/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/unoapi_test.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#include <docsh.hxx>
#include <document.hxx>
#include <attrib.hxx>
#include <scitems.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>

using namespace css;

class VBAMacroTest : public UnoApiTest
{
public:
    uno::Reference<lang::XComponent> mxComponent;

    VBAMacroTest()
        : UnoApiTest("/sc/qa/extras/testdocuments")
    {
    }

    virtual void tearDown() override
    {
        if (mxComponent.is())
        {
            mxComponent->dispose();
            mxComponent.set(nullptr);
        }

        test::BootstrapFixture::tearDown();
    }

    void testSimpleCopyAndPaste();
    void testMultiDocumentCopyAndPaste();

    CPPUNIT_TEST_SUITE(VBAMacroTest);
    CPPUNIT_TEST(testSimpleCopyAndPaste);
    CPPUNIT_TEST(testMultiDocumentCopyAndPaste);
    CPPUNIT_TEST_SUITE_END();
};

void VBAMacroTest::testSimpleCopyAndPaste()
{
    // Copy-paste values in the same sheet

    // Range(Cells(4, 3), Cells(6, 3)).Copy
    // Cells(4, 2).Activate
    // ActiveCell.PasteSpecial xlValues

    OUString aFileName;
    createFileURL(u"SimpleCopyPaste.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    // Check state
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(2, 5, 0)));

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 5, 0)));

    SfxObjectShell::CallXScript(
        mxComponent, "vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    // Copy from C4-C6
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(2, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(2, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(2, 5, 0)));

    // Paste to B4-B6
    CPPUNIT_ASSERT_EQUAL(10.0, rDoc.GetValue(ScAddress(1, 3, 0)));
    CPPUNIT_ASSERT_EQUAL(20.0, rDoc.GetValue(ScAddress(1, 4, 0)));
    CPPUNIT_ASSERT_EQUAL(30.0, rDoc.GetValue(ScAddress(1, 5, 0)));
}

void VBAMacroTest::testMultiDocumentCopyAndPaste()
{
    // Creates a new workbook (document) and copy-pastes values
    // between the documents.

    // Set CurrentWB = ActiveWorkbook
    // Workbooks.Add
    // Set NewWB = ActiveWorkbook
    // Cells(3, 2).Value = 200
    // Cells(4, 2).Value = 100
    // Range(Cells(3, 2), Cells(4, 2)).Copy
    // CurrentWB.Activate
    // Cells(2, 2).Activate
    // ActiveCell.PasteSpecial xlValues
    // ...

    OUString aFileName;
    createFileURL(u"MultiDocumentCopyPaste.xlsm", aFileName);
    mxComponent = loadFromDesktop(aFileName, "com.sun.star.sheet.SpreadsheetDocument");

    uno::Any aRet;
    uno::Sequence<sal_Int16> aOutParamIndex;
    uno::Sequence<uno::Any> aOutParam;
    uno::Sequence<uno::Any> aParams;

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);

    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = static_cast<ScDocShell*>(pFoundShell);
    ScDocument& rDoc = pDocSh->GetDocument();

    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));

    SfxObjectShell::CallXScript(
        mxComponent, "vnd.sun.Star.script:VBAProject.Module1.test?language=Basic&location=document",
        aParams, aRet, aOutParamIndex, aOutParam);

    CPPUNIT_ASSERT_EQUAL(200.0, rDoc.GetValue(ScAddress(1, 1, 0)));
    CPPUNIT_ASSERT_EQUAL(100.0, rDoc.GetValue(ScAddress(1, 2, 0)));
    CPPUNIT_ASSERT_EQUAL(0.0, rDoc.GetValue(ScAddress(1, 3, 0)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(VBAMacroTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
