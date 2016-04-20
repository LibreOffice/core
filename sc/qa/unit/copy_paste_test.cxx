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

#include "docsh.hxx"
#include "tabvwsh.hxx"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#include "helper/qahelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScCopyPasteTest : public ScBootstrapFixture
{
public:
    ScCopyPasteTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCopyPasteXLS();

    CPPUNIT_TEST_SUITE(ScCopyPasteTest);
    CPPUNIT_TEST(testCopyPasteXLS);
    CPPUNIT_TEST_SUITE_END();

private:

    uno::Reference<uno::XInterface> m_xCalcComponent;
};

// tdf#83366
void ScCopyPasteTest::testCopyPasteXLS()
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(::comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT( xDesktop.is() );

    // create a frame
    Reference< frame::XFrame > xTargetFrame = xDesktop->findFrame( "_blank", 0 );
    CPPUNIT_ASSERT( xTargetFrame.is() );

    // 1. Open the document
    ScDocShellRef xDocSh = loadDoc("chartx2.", FORMAT_XLS);
    CPPUNIT_ASSERT_MESSAGE("Failed to load chartx2.xls.", xDocSh.Is());

    uno::Reference< frame::XModel2 > xModel2 ( xDocSh->GetModel(), UNO_QUERY );
    CPPUNIT_ASSERT( xModel2.is() );

    Reference< frame::XController2 > xController ( xModel2->createDefaultViewController( xTargetFrame ), UNO_QUERY );
    CPPUNIT_ASSERT( xController.is() );

    // introduce model/view/controller to each other
    xController->attachModel( xModel2.get() );
    xModel2->connectController( xController.get() );
    xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
    xController->attachFrame( xTargetFrame );
    xModel2->setCurrentController( xController.get() );

    ScDocument& rDoc = xDocSh->GetDocument();

    // Get the document controller
    ScTabViewShell* pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // 2. Highlight B2:C5
    ScRange aSrcRange;
    sal_uInt16 nRes = aSrcRange.Parse("B2:C5", &rDoc, rDoc.GetAddressConvention());
    CPPUNIT_ASSERT_MESSAGE("Failed to parse.", (nRes & SCA_VALID) != 0);

    ScMarkData aMark;
    aMark.SetMarkArea(aSrcRange);

    pViewShell->GetViewData().GetMarkData().SetMarkArea(aSrcRange);

    // 3. Copy
    ScDocument aClipDoc(SCDOCMODE_CLIP);
    pViewShell->GetViewData().GetView()->CopyToClip(&aClipDoc, false, false, false, false);

    // 4. Close the document (Ctrl-W)
    xDocSh->DoClose();

    // 5. Create a new Spreadsheet
    Sequence < beans::PropertyValue > args(1);
    args[0].Name = "Hidden";
    args[0].Value <<= true;

    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL(
            "private:factory/scalc",
            "_blank",
            0,
            args );
    CPPUNIT_ASSERT( xComponent.is() );

    // Get the document model
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(xComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);

    xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(xDocSh != nullptr);

    // Get the document controller
    pViewShell = xDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT(pViewShell != nullptr);

    // 6. Paste
    pViewShell->GetViewData().GetView()->PasteFromClip(InsertDeleteFlags::ALL, &aClipDoc);

    xDocSh->DoClose();
}

ScCopyPasteTest::ScCopyPasteTest()
      : ScBootstrapFixture( "/sc/qa/unit/data" )
{
}

void ScCopyPasteTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScCopyPasteTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScCopyPasteTest);

CPPUNIT_PLUGIN_IMPLEMENT();


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
