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
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/util/URL.hpp>
#include <test/bootstrapfixture.hxx>
#include <unotools/tempfile.hxx>
#include <unotest/macros_test.hxx>
#include <docsh.hxx>
#include <editutil.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fontitem.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>

using namespace css::lang;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScPDFExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    Reference<XComponent> mxComponent;
    Reference<frame::XFrame> xTargetFrame;

public:
    ScPDFExportTest() {}

    virtual void setUp() override final;
    virtual void tearDown() override final;

    // helpers
private:
    std::shared_ptr<utl::TempFile> exportToPdf(uno::Reference<frame::XModel>& xModel,
                                               const ScRange& range);

    static bool hasFontInPdf(const std::shared_ptr<utl::TempFile>& pXPathFile,
                             const char* sFontName, bool& bFound);

    void setFont(ScFieldEditEngine& rEE, sal_Int32 nStart, sal_Int32 nEnd,
                 const OUString& rFontName);

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

    mxDesktop.set(
        css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));

    {
        uno::Reference<frame::XDesktop2> xDesktop = mxDesktop;
        CPPUNIT_ASSERT(xDesktop.is());

        // Create spreadsheet
        uno::Sequence<beans::PropertyValue> args(1);
        args[0].Name = "Hidden";
        args[0].Value <<= true;
        mxComponent = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
        CPPUNIT_ASSERT(mxComponent.is());

        // create a frame
        xTargetFrame = xDesktop->findFrame("_blank", 0);
        CPPUNIT_ASSERT(xTargetFrame.is());

        uno::Reference<frame::XModel> xModel
            = uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY);
        uno::Reference<frame::XModel2> xModel2(xModel, UNO_QUERY);
        CPPUNIT_ASSERT(xModel2.is());

        Reference<frame::XController2> xController(
            xModel2->createDefaultViewController(xTargetFrame), UNO_QUERY);
        CPPUNIT_ASSERT(xController.is());

        // introduce model/view/controller to each other
        xController->attachModel(xModel2.get());
        xModel2->connectController(xController.get());

        xTargetFrame->setComponent(xController->getComponentWindow(), xController.get());
        xController->attachFrame(xTargetFrame);

        xModel2->setCurrentController(xController.get());
    }
}

void ScPDFExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

bool ScPDFExportTest::hasFontInPdf(const std::shared_ptr<utl::TempFile>& pXPathFile,
                                   const char* sFontName, bool& bFound)
{
    SvStream* pStream = pXPathFile->GetStream(StreamMode::STD_READ);
    CPPUNIT_ASSERT(pStream);

    // get file size
    pStream->Seek(STREAM_SEEK_TO_END);
    const std::size_t nFileSize = pStream->Tell();
    if (nFileSize == 0)
        return false;

    // read file content
    char* pBuffer = new char[nFileSize];
    pStream->Seek(STREAM_SEEK_TO_BEGIN);
    const std::size_t nRead = pStream->ReadBytes(pBuffer, nFileSize);
    if (nRead == nFileSize)
    {
        const std::string haystack(pBuffer, pBuffer + nFileSize);
        const std::string needle(sFontName);
        const std::size_t n = haystack.find(needle);
        bFound = (n != std::string::npos);
    }
    delete[] pBuffer;

    // close and return the status
    pStream = nullptr;
    pXPathFile->CloseStream();
    return (nRead == nFileSize);
}

std::shared_ptr<utl::TempFile> ScPDFExportTest::exportToPdf(uno::Reference<frame::XModel>& xModel,
                                                            const ScRange& range)
{
    // create temp file name
    std::shared_ptr<utl::TempFile> pTempFile(new utl::TempFile());
    pTempFile->EnableKillingFile();
    OUString sFileURL = pTempFile->GetURL();
    // Note: under Windows path path should be with "/" delimiters instead of "\\"
    // due to usage of INetURLObject() that converts "\\" to hexdecimal notation.
    ::osl::FileBase::getFileURLFromSystemPath(sFileURL, sFileURL);

    // get XSpreadsheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // select requested cells to print
    // query for the XCellRange interface
    uno::Reference<table::XCellRange> rCellRange(rSheet, UNO_QUERY);
    uno::Reference<table::XCellRange> xCellRange = rCellRange->getCellRangeByPosition(
        range.aStart.Col(), range.aStart.Row(), range.aEnd.Col(), range.aEnd.Row());
    {
        uno::Reference<frame::XController> xController = xModel->getCurrentController();
        CPPUNIT_ASSERT(xController.is());

        uno::Reference<view::XSelectionSupplier> xSelection(xController, uno::UNO_QUERY_THROW);
        CPPUNIT_ASSERT(xSelection.is());

        uno::Any rCellRangeAny(xCellRange);
        xSelection->select(rCellRangeAny);
    }

    // init special pdf export params
    css::uno::Sequence<css::beans::PropertyValue> aFilterData(3);
    aFilterData[0].Name = "Selection";
    aFilterData[0].Value <<= xCellRange;
    aFilterData[1].Name = "Printing";
    aFilterData[1].Value <<= sal_Int32(2);
    aFilterData[2].Name = "ViewPDFAfterExport";
    aFilterData[2].Value <<= true;

    // init set of params for storeToURL() call
    css::uno::Sequence<css::beans::PropertyValue> seqArguments(3);
    seqArguments[0].Name = "FilterData";
    seqArguments[0].Value <<= aFilterData;
    seqArguments[1].Name = "FilterName";
    seqArguments[1].Value <<= OUString("calc_pdf_Export");
    seqArguments[2].Name = "URL";
    seqArguments[2].Value <<= sFileURL;

    // call storeToURL()
    uno::Reference<lang::XComponent> xComponent(mxComponent, UNO_QUERY_THROW);
    uno::Reference<css::frame::XStorable> xStorable(xComponent, UNO_QUERY);
    xStorable->storeToURL(sFileURL, seqArguments);

    // return file object with generated PDF
    return pTempFile;
}

void ScPDFExportTest::setFont(ScFieldEditEngine& rEE, sal_Int32 nStart, sal_Int32 nEnd,
                              const OUString& rFontName)
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = 0;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxFontItem aItem(FAMILY_MODERN, rFontName, "", PITCH_VARIABLE, RTL_TEXTENCODING_UTF8,
                      EE_CHAR_FONTINFO);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, aSel);
}

// Selection was not taken into account during export into PDF
void ScPDFExportTest::testExportRange_Tdf120161()
{
    // create test document
    uno::Reference<frame::XModel> xModel
        = uno::Reference<frame::XModel>(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("First Sheet", 0);
    uno::Reference<sheet::XSpreadsheet> rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // 2. Setup data
    {
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
        CPPUNIT_ASSERT(xDocSh.get() != nullptr);

        // put some content into the first row with default font
        ScDocument& rDoc = xDocSh->GetDocument();
        for (unsigned int r = 0; r < 1; ++r)
            for (unsigned int c = 0; c < 14; ++c)
                rDoc.SetValue(ScAddress(c, r, 0), (r + 1) * (c + 1));

        // set "Text" to H1 cell with "DejaVuSans" font
        ScFieldEditEngine& rEE = rDoc.GetEditEngine();
        rEE.Clear();
        rEE.SetText("Text");
        setFont(rEE, 0, 4, "DejaVuSans"); // set font for first 4 chars
        rDoc.SetEditText(ScAddress(7, 0, 0), rEE.CreateTextObject());
    }

    // A1:G1
    {
        ScRange range1(0, 0, 0, 6, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasFontInPdf(pXPathFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(false, bFound);
    }

    // G1:H1
    {
        ScRange range1(6, 0, 0, 7, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasFontInPdf(pXPathFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // H1:I1
    {
        ScRange range1(7, 0, 0, 8, 0, 0);
        std::shared_ptr<utl::TempFile> pXPathFile = exportToPdf(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasFontInPdf(pXPathFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPDFExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
