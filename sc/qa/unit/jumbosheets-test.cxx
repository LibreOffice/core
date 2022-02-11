/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>
#include <vcl/keycodes.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <test/xmltesttools.hxx>

#include <defaultsoptions.hxx>
#include <scmod.hxx>
#include <viewdata.hxx>
#include <tabvwsh.hxx>
#include <com/sun/star/frame/Desktop.hpp>

#include "helper/qahelper.hxx"
#include "helper/xpath.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Tests for sheets larger than 1024 columns and/or 1048576 rows. */

class ScJumboSheetsTest : public test::FiltersTest, public ScBootstrapFixture, public XmlTestTools
{
public:
    ScJumboSheetsTest();

    virtual bool load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                      SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                      unsigned int nFilterVersion) override;

    virtual void setUp() override;
    virtual void tearDown() override;

    void testRoundtripColumn2000Ods();
    void testRoundtripColumn2000Xlsx();
    void testRoundtripColumnRange();
    void testTdf134392();
    void testTdf133033();

    CPPUNIT_TEST_SUITE(ScJumboSheetsTest);

    CPPUNIT_TEST(testRoundtripColumn2000Ods);
    CPPUNIT_TEST(testRoundtripColumn2000Xlsx);
    CPPUNIT_TEST(testRoundtripColumnRange);
    CPPUNIT_TEST(testTdf134392);
    CPPUNIT_TEST(testTdf133033);

    CPPUNIT_TEST_SUITE_END();

protected:
    virtual void registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx) override;

private:
    void testRoundtripColumn2000(std::u16string_view name, int format);
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

bool ScJumboSheetsTest::load(const OUString& rFilter, const OUString& rURL,
                             const OUString& rUserData, SfxFilterFlags nFilterFlags,
                             SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData, OUString(),
                                                       nFilterFlags, nClipboardID, nFilterVersion);
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScJumboSheetsTest::testRoundtripColumn2000Ods()
{
    testRoundtripColumn2000(u"value-in-column-2000.", FORMAT_ODS);
}

void ScJumboSheetsTest::testRoundtripColumn2000Xlsx()
{
    testRoundtripColumn2000(u"value-in-column-2000.", FORMAT_XLSX);
}

void ScJumboSheetsTest::testRoundtripColumn2000(std::u16string_view name, int format)
{
    ScDocShellRef xDocSh1 = loadDoc(name, format);
    CPPUNIT_ASSERT(xDocSh1.is());

    {
        ScDocument& rDoc = xDocSh1->GetDocument();
        // Check the value at BXX1 (2000th column).
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(1999, 0, 0));
        // Check the formula referencing the value.
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), rDoc.GetFormula(0, 0, 0));
        // Recalc and check value in the reference.
        rDoc.CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(0, 0, 0));
    }

    ScDocShellRef xDocSh2 = saveAndReload(*xDocSh1, format);
    CPPUNIT_ASSERT(xDocSh2.is());

    {
        // Check again.
        ScDocument& rDoc = xDocSh2->GetDocument();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(1999, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=BXX1"), rDoc.GetFormula(0, 0, 0));
        rDoc.CalcAll();
        CPPUNIT_ASSERT_EQUAL(-5.0, rDoc.GetValue(0, 0, 0));
    }

    xDocSh1->DoClose();
    xDocSh2->DoClose();
}

void ScJumboSheetsTest::testRoundtripColumnRange()
{
    ScDocShellRef xDocSh1 = loadDoc(u"sum-whole-column-row.", FORMAT_ODS);
    CPPUNIT_ASSERT(xDocSh1.is());

    {
        ScDocument& rDoc = xDocSh1->GetDocument();
        // Check the formula referencing the whole-row range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        // Check the formula referencing the whole-column range.
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
    }

    std::shared_ptr<utl::TempFile> exportedFile;
    ScDocShellRef xDocSh2 = saveAndReloadNoClose(*xDocSh1, FORMAT_ODS, &exportedFile);
    CPPUNIT_ASSERT(xDocSh2.is());

    {
        ScDocument& rDoc = xDocSh2->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
        xmlDocUniquePtr pDoc = XPathHelper::parseExport(exportedFile, m_xSFactory, "content.xml");
        CPPUNIT_ASSERT(pDoc);
        assertXPath(pDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[1]",
                    "formula", "of:=SUM([.2:.2])");
        assertXPath(pDoc,
                    "/office:document-content/office:body/office:spreadsheet/table:table/"
                    "table:table-row[1]/table:table-cell[2]",
                    "formula", "of:=SUM([.C:.C])");
    }

    ScDocShellRef xDocSh3 = saveAndReloadNoClose(*xDocSh1, FORMAT_XLSX, &exportedFile);
    CPPUNIT_ASSERT(xDocSh3.is());

    {
        ScDocument& rDoc = xDocSh3->GetDocument();
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(2:2)"), rDoc.GetFormula(0, 0, 0));
        CPPUNIT_ASSERT_EQUAL(OUString("=SUM(C:C)"), rDoc.GetFormula(1, 0, 0));
        xmlDocUniquePtr pDoc
            = XPathHelper::parseExport(exportedFile, m_xSFactory, "xl/worksheets/sheet1.xml");
        CPPUNIT_ASSERT(pDoc);
        assertXPathContent(pDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[1]/x:f", "SUM(2:2)");
        assertXPathContent(pDoc, "/x:worksheet/x:sheetData/x:row[1]/x:c[2]/x:f", "SUM(C:C)");
    }

    xDocSh1->DoClose();
    xDocSh2->DoClose();
    xDocSh3->DoClose();
}

void ScJumboSheetsTest::testTdf134392()
{
    // Without the fix in place, the file would have crashed
    ScDocShellRef xDocSh = loadDoc(u"tdf134392.", FORMAT_XLSX);
    CPPUNIT_ASSERT(xDocSh.is());

    ScDocument& rDoc = xDocSh->GetDocument();
    rDoc.CalcAll(); // perform hard re-calculation.
    xDocSh->DoClose();
}

void ScJumboSheetsTest::testTdf133033()
{
    // Create an empty document
    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xDesktop.is());

    Sequence<beans::PropertyValue> args{ comphelper::makePropertyValue("Hidden", true) };

    m_xCalcComponent = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
    CPPUNIT_ASSERT(m_xCalcComponent.is());

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(m_xCalcComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh);

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell);

    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(m_xCalcComponent.get());
    CPPUNIT_ASSERT(pModelObj);

    pModelObj->postKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, KEY_DOWN | KEY_MOD1);
    Scheduler::ProcessEventsToIdle();

    ScViewData& rViewData = pViewShell->GetViewData();

    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), rViewData.GetCurX());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(16777215), rViewData.GetCurY());
}

ScJumboSheetsTest::ScJumboSheetsTest()
    : ScBootstrapFixture("sc/qa/unit/data")
{
}

void ScJumboSheetsTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());

    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(true);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);
}

void ScJumboSheetsTest::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();

    ScDefaultsOptions aDefaultsOption = SC_MOD()->GetDefaultsOptions();
    aDefaultsOption.SetInitJumboSheets(false);
    SC_MOD()->SetDefaultsOptions(aDefaultsOption);
}

void ScJumboSheetsTest::registerNamespaces(xmlXPathContextPtr& pXmlXPathCtx)
{
    XmlTestTools::registerOOXMLNamespaces(pXmlXPathCtx);
    XmlTestTools::registerODFNamespaces(pXmlXPathCtx);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScJumboSheetsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
