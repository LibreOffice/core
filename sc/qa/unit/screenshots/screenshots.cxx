/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <test/xmltesttools.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/string.hxx>
#include <editeng/editids.hrc>
#include <editeng/editview.hxx>
#include <editeng/outliner.hxx>
#include <osl/conditn.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/srchitem.hxx>

#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <docuno.hxx>
#include <scabstdlg.hxx>
#include <reffact.hxx>

#include <sc.hrc>

using namespace css;

#if !defined(WNT) && !defined(MACOSX)
static const char* DATA_DIRECTORY = "/sc/qa/unit/screenshots/data/";
#endif

class ScScreenshotTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools
{
public:
    ScScreenshotTest();
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

#if !defined(WNT) && !defined(MACOSX)
    void testOpeningSomeDialog();
    //void testOpeningModelessDialogs();
#endif

    CPPUNIT_TEST_SUITE(ScScreenshotTest);
#if !defined(WNT) && !defined(MACOSX)
    CPPUNIT_TEST(testOpeningSomeDialog);
    //CPPUNIT_TEST(testOpeningModelessDialogs);
#endif
    CPPUNIT_TEST_SUITE_END();

private:
#if !defined(WNT) && !defined(MACOSX)
    ScModelObj* createDoc(const char* pName);

#endif

    uno::Reference<lang::XComponent> mxComponent;
};

ScScreenshotTest::ScScreenshotTest()
{
}

void ScScreenshotTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));
}

void ScScreenshotTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

#if !defined(WNT) && !defined(MACOSX)
ScModelObj* ScScreenshotTest::createDoc(const char* pName)
{
    if (mxComponent.is())
        mxComponent->dispose();
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + OUString::createFromAscii(pName), "com.sun.star.sheet.SpreadsheetDocument");
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj;
}

void ScScreenshotTest::testOpeningSomeDialog()
{
    ScModelObj* pModelObj = createDoc("empty.ods");
    ScDocument* pDoc = pModelObj->GetDocument();

    // display insert sheet dialog
    //uno::Sequence<beans::PropertyValue> aArgs;
    //comphelper::dispatchCommand(".uno:Insert", aArgs);

    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
    CPPUNIT_ASSERT( pFact != nullptr );

    ScViewData& rViewData = pViewShell->GetViewData();
    SCTAB nTabSelCount = rViewData.GetMarkData().GetSelectCount();
    std::unique_ptr<AbstractScInsertTableDlg> pDlg( pFact->CreateScInsertTableDlg(
           pViewShell->GetDialogParent(), rViewData, nTabSelCount, false));
    CPPUNIT_ASSERT( pDlg != nullptr );

    pDlg->Execute();

    std::unique_ptr<AbstractScDeleteCellDlg> pDlg2( pFact->CreateScDeleteCellDlg(
           pViewShell->GetDialogParent(), false));
    CPPUNIT_ASSERT( pDlg2 != nullptr );

    pDlg2->Execute();

    std::unique_ptr<AbstractScInsertContentsDlg> pDlg3( pFact->CreateScInsertContentsDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg3 != nullptr );

    pDlg3->Execute();

    std::unique_ptr<AbstractScColRowLabelDlg> pDlg4( pFact->CreateScColRowLabelDlg(
           pViewShell->GetDialogParent(), true, false));
    CPPUNIT_ASSERT( pDlg4 != nullptr );

    pDlg4->Execute();

    std::unique_ptr<AbstractScDataPilotDatabaseDlg> pDlg5( pFact->CreateScDataPilotDatabaseDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg5 != nullptr );

    pDlg5->Execute();

    std::unique_ptr<AbstractScDataPilotSourceTypeDlg> pDlg6( pFact->CreateScDataPilotSourceTypeDlg(
           pViewShell->GetDialogParent(), true));
    CPPUNIT_ASSERT( pDlg6 != nullptr );

    pDlg6->Execute();

    std::unique_ptr<AbstractScDeleteContentsDlg> pDlg7( pFact->CreateScDeleteContentsDlg(
           pViewShell->GetDialogParent()));
    CPPUNIT_ASSERT( pDlg7 != nullptr );

    pDlg7->Execute();

}

#endif

CPPUNIT_TEST_SUITE_REGISTRATION(ScScreenshotTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
