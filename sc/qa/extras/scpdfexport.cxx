/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <test/bootstrapfixture.hxx>
#include <test/htmltesttools.hxx>
#include <test/xmltesttools.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotest/macros_test.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>

#include <docsh.hxx>

#include <sal/config.h>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <config_features.h>

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <sfx2/sfxsids.hrc>

#include <userdat.hxx>
#include <docsh.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <scitems.hxx>
#include <document.hxx>
#include <cellform.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <editutil.hxx>
#include <scopetools.hxx>
#include <cellvalue.hxx>
#include <docfunc.hxx>
#include <generalfunction.hxx>
#include <postit.hxx>
#include <tokenstringcontext.hxx>
#include <chgtrack.hxx>
#include <validat.hxx>
#include <attrib.hxx>
#include <global.hxx>
#include <scmod.hxx>

#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <tabprotection.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/section.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/colritem.hxx>
#include <formula/grammar.hxx>
#include <unotools/useroptions.hxx>
#include <tools/datetime.hxx>
#include <svl/zformat.hxx>

#include <test/xmltesttools.hxx>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <comphelper/storagehelper.hxx>

#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <comphelper/dispatchcommand.hxx>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/frame/XStorable.hpp>

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace utl;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScPDFExportTest : public test::BootstrapFixture, public unotest::MacrosTest, public XmlTestTools, public HtmlTestTools
{
    Reference<XComponent> mxComponent;
    Reference< frame::XFrame > xTargetFrame;

public:
    ScPDFExportTest()
    {}

    virtual void setUp() override final;
    virtual void tearDown() override final;

    // helpers
private:
    void dispatchCommand(const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rArguments);
    std::shared_ptr<utl::TempFile> exportToPdf(uno::Reference< frame::XModel > & xModel, const ScRange & range);
    static bool hasFontInPdf(const std::shared_ptr<utl::TempFile> & pXPathFile, const char * sFontName);

    // unit tests
public:
    void testExportRange_Tdf120161();

    CPPUNIT_TEST_SUITE(ScPDFExportTest);
    CPPUNIT_TEST(testExportRange_Tdf120161);
    CPPUNIT_TEST_SUITE_END();
};

void ScPDFExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));

    {
        uno::Reference< frame::XDesktop2 > xDesktop = mxDesktop;
        CPPUNIT_ASSERT( xDesktop.is() );

        // create a frame
        xTargetFrame = xDesktop->findFrame( "_blank", 0 );
        CPPUNIT_ASSERT( xTargetFrame.is() );

        // Create spreadsheet
        uno::Sequence< beans::PropertyValue > args(1);
        args[0].Name = "Hidden";
        args[0].Value <<= true;
        mxComponent = xDesktop->loadComponentFromURL(
                "private:factory/scalc",
                "_blank",
                0,
                args );
        CPPUNIT_ASSERT( mxComponent.is() );

        uno::Reference< frame::XModel > xModel = uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY);
        uno::Reference< frame::XModel2 > xModel2 ( xModel, UNO_QUERY );
        CPPUNIT_ASSERT( xModel2.is() );

        Reference< frame::XController2 > xController ( xModel2->createDefaultViewController( xTargetFrame ), UNO_QUERY );
        CPPUNIT_ASSERT( xController.is() );

        // introduce model/view/controller to each other
        xController->attachModel( xModel2.get() );
        xModel2->connectController( xController.get() );

        xTargetFrame->setComponent( xController->getComponentWindow(), xController.get() );
        xController->attachFrame( xTargetFrame );

        xModel2->setCurrentController( xController.get() );
    }
}

void ScPDFExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

void ScPDFExportTest::dispatchCommand(const OUString& rCommand, const uno::Sequence<beans::PropertyValue>& rArguments)
{
    uno::Reference< lang::XComponent > xComponent(mxComponent, UNO_QUERY_THROW);

    uno::Reference<frame::XDispatchProvider> xDispatchProvider(xTargetFrame, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xDispatchProvider.is());

    util::URL aCommandURL;
    aCommandURL.Complete = rCommand;

    OUString targetFrame;
    uno::Reference<frame::XDispatch> xDisp = xDispatchProvider->queryDispatch(aCommandURL, targetFrame, 0);
    CPPUNIT_ASSERT(xDisp.is());

    xDisp->dispatch(aCommandURL, rArguments);

//    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xComponentContext));
//    CPPUNIT_ASSERT(xDispatchHelper.is());
//    xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rArguments);
}

bool ScPDFExportTest::hasFontInPdf(const std::shared_ptr<utl::TempFile> & pXPathFile, const char * sFontName)
{
    return true; // TODO:
}

std::shared_ptr<utl::TempFile> ScPDFExportTest::exportToPdf(uno::Reference< frame::XModel > & xModel, const ScRange & range)
{
    std::shared_ptr<utl::TempFile> pTempFile(new utl::TempFile());
    pTempFile->EnableKillingFile();
    const OUString sURL = utl::TempFile::CreateTempName() + ".pdf";
    const OUString sFileURL = "file:///" + sURL;

    // get XSpreadsheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference< sheet::XSpreadsheet > rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // select requested cells to print
    // query for the XCellRange interface
    uno::Reference< table::XCellRange > rCellRange(rSheet, UNO_QUERY);
    uno::Reference< table::XCellRange > xCellRange = rCellRange->getCellRangeByPosition(range.aStart.Col(), range.aStart.Row(), range.aEnd.Col(), range.aEnd.Row());
    {
        uno::Reference<frame::XController> xController = xModel->getCurrentController();
        CPPUNIT_ASSERT(xController.is());

        uno::Reference< view::XSelectionSupplier > xSelection(xController, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xSelection.is());

        uno::Any rCellRangeAny(xCellRange);
        xSelection->select(rCellRangeAny);
    }

    // init special pdf export params
    css::uno::Sequence< css::beans::PropertyValue > aFilterData(3);
    aFilterData[0].Name = "Selection";
    aFilterData[0].Value <<= xCellRange;
    aFilterData[1].Name = "Printing";
    aFilterData[1].Value <<= (sal_Int8)2;
    aFilterData[2].Name = "ViewPDFAfterExport";
    aFilterData[2].Value <<= true;

    // init set of params for storeToURL() call
    css::uno::Sequence< css::beans::PropertyValue > seqArguments(3);
    seqArguments[0].Name = "FilterData";
    seqArguments[0].Value <<= aFilterData;
    seqArguments[1].Name = "FilterName";
    seqArguments[1].Value <<= OUString("calc_pdf_Export");
    seqArguments[2].Name = "URL";
    seqArguments[2].Value <<= sFileURL;

    //uno::Reference<uno::XComponentContext> xComponentContext = comphelper::getProcessComponentContext();
    uno::Reference<uno::XComponentContext> xComponentContext = comphelper::getComponentContext(getMultiServiceFactory());
    CPPUNIT_ASSERT(xComponentContext.is());

    if (1)
    {
        uno::Reference< lang::XComponent > xComponent(mxComponent, UNO_QUERY_THROW);
        uno::Reference< css::frame::XStorable> xStorable(xComponent, UNO_QUERY);
        xStorable->storeToURL(sFileURL, seqArguments);
    }
    else
    {
        dispatchCommand(".uno:ExportToPDF", seqArguments);
    }

    return pTempFile;
}

void ScPDFExportTest::testExportRange_Tdf120161()
{
    // create test document
    uno::Reference< frame::XModel > xModel = uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets (xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("First Sheet", 0);
    uno::Reference< sheet::XSpreadsheet > rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // A1:G1
    {
        ScRange range1(0, 0, 0, 6, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        CPPUNIT_ASSERT_EQUAL(false, hasFontInPdf(pXPathFile, "DejaVuSans"));
    }

    // G1:H1
    {
        ScRange range1(6, 0, 0, 7, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        CPPUNIT_ASSERT_EQUAL(true, hasFontInPdf(pXPathFile, "DejaVuSans"));
    }

    // H1:I1
    {
        ScRange range1(7, 0, 0, 8, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        CPPUNIT_ASSERT_EQUAL(true, hasFontInPdf(pXPathFile, "DejaVuSans"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPDFExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
