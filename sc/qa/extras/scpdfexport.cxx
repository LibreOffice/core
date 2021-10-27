/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <config_oox.h>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotools/tempfile.hxx>
#include <unotest/macros_test.hxx>
#include <docsh.hxx>
#include <editutil.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fontitem.hxx>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif

using namespace css::lang;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScPDFExportTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    Reference<XComponent> mxComponent;
    Reference<frame::XFrame> xTargetFrame;

public:
    ScPDFExportTest() {}
    ~ScPDFExportTest();

    virtual void setUp() override final;
    virtual void tearDown() override final;

    // helpers
private:
    std::shared_ptr<utl::TempFile> exportToPDF(const uno::Reference<frame::XModel>& xModel,
                                               const ScRange& range);

    std::shared_ptr<utl::TempFile> exportToPDFWithUnoCommands(const OUString& rRange);

    static bool hasTextInPdf(const std::shared_ptr<utl::TempFile>& pPDFFile, const char* sText,
                             bool& bFound);

    void setFont(ScFieldEditEngine& rEE, sal_Int32 nStart, sal_Int32 nEnd,
                 const OUString& rFontName);

    // unit tests
public:
    void testExportRange_Tdf120161();
    void testExportFitToPage_Tdf103516();
    void testUnoCommands_Tdf120161();
    void testTdf64703_hiddenPageBreak();

    CPPUNIT_TEST_SUITE(ScPDFExportTest);
    CPPUNIT_TEST(testExportRange_Tdf120161);
    CPPUNIT_TEST(testExportFitToPage_Tdf103516);
    CPPUNIT_TEST(testUnoCommands_Tdf120161);
    CPPUNIT_TEST(testTdf64703_hiddenPageBreak);
    CPPUNIT_TEST_SUITE_END();
};

constexpr OUStringLiteral DATA_DIRECTORY = u"/sc/qa/extras/testdocuments/";

ScPDFExportTest::~ScPDFExportTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

void ScPDFExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(
        css::frame::Desktop::create(comphelper::getComponentContext(getMultiServiceFactory())));

    {
        uno::Reference<frame::XDesktop2> xDesktop = mxDesktop;
        CPPUNIT_ASSERT(xDesktop.is());

        // Create spreadsheet
        uno::Sequence<beans::PropertyValue> args{ comphelper::makePropertyValue("Hidden", true) };
        mxComponent = xDesktop->loadComponentFromURL("private:factory/scalc", "_blank", 0, args);
        CPPUNIT_ASSERT(mxComponent.is());

        // create a frame
        xTargetFrame = xDesktop->findFrame("_blank", 0);
        CPPUNIT_ASSERT(xTargetFrame.is());

        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Reference<frame::XModel2> xModel2(xModel, UNO_QUERY);
        CPPUNIT_ASSERT(xModel2.is());

        Reference<frame::XController2> xController
            = xModel2->createDefaultViewController(xTargetFrame);
        CPPUNIT_ASSERT(xController.is());

        // introduce model/view/controller to each other
        xController->attachModel(xModel2);
        xModel2->connectController(xController);

        xTargetFrame->setComponent(xController->getComponentWindow(), xController);
        xController->attachFrame(xTargetFrame);

        xModel2->setCurrentController(xController);
    }
}

void ScPDFExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

bool ScPDFExportTest::hasTextInPdf(const std::shared_ptr<utl::TempFile>& pPDFFile,
                                   const char* sText, bool& bFound)
{
    SvStream* pStream = pPDFFile->GetStream(StreamMode::STD_READ);
    CPPUNIT_ASSERT(pStream);

    // get file size
    const std::size_t nFileSize = pStream->TellEnd();
    if (nFileSize == 0)
        return false;

    // read file content
    char* pBuffer = new char[nFileSize];
    pStream->Seek(STREAM_SEEK_TO_BEGIN);
    const std::size_t nRead = pStream->ReadBytes(pBuffer, nFileSize);
    if (nRead == nFileSize)
    {
        const std::string haystack(pBuffer, pBuffer + nFileSize);
        const std::string needle(sText);
        const std::size_t n = haystack.find(needle);
        bFound = (n != std::string::npos);
    }
    delete[] pBuffer;

    // close and return the status
    pStream = nullptr;
    pPDFFile->CloseStream();
    return (nRead == nFileSize);
}

std::shared_ptr<utl::TempFile>
ScPDFExportTest::exportToPDF(const uno::Reference<frame::XModel>& xModel, const ScRange& range)
{
    // create temp file name
    auto pTempFile = std::make_shared<utl::TempFile>();
    pTempFile->EnableKillingFile();
    OUString sFileURL = pTempFile->GetURL();
    // Note: under Windows path path should be with "/" delimiters instead of "\\"
    // due to usage of INetURLObject() that converts "\\" to hexadecimal notation.
    ::osl::FileBase::getFileURLFromSystemPath(sFileURL, sFileURL);

    // get XSpreadsheet
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // select requested cells to print
    // query for the XCellRange interface
    uno::Reference<table::XCellRange> xCellRange = rSheet->getCellRangeByPosition(
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
    css::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("Selection", xCellRange),
        comphelper::makePropertyValue("Printing", sal_Int32(2)),
        comphelper::makePropertyValue("ViewPDFAfterExport", true)
    };

    // init set of params for storeToURL() call
    css::uno::Sequence<css::beans::PropertyValue> seqArguments{
        comphelper::makePropertyValue("FilterData", aFilterData),
        comphelper::makePropertyValue("FilterName", OUString("calc_pdf_Export")),
        comphelper::makePropertyValue("URL", sFileURL)
    };

    // call storeToURL()
    uno::Reference<lang::XComponent> xComponent(mxComponent, UNO_SET_THROW);
    uno::Reference<css::frame::XStorable> xStorable(xComponent, UNO_QUERY);
    xStorable->storeToURL(sFileURL, seqArguments);

    // return file object with generated PDF
    return pTempFile;
}

std::shared_ptr<utl::TempFile> ScPDFExportTest::exportToPDFWithUnoCommands(const OUString& rRange)
{
    // create temp file name
    auto pTempFile = std::make_shared<utl::TempFile>();
    pTempFile->EnableKillingFile();
    OUString sFileURL = pTempFile->GetURL();
    // Note: under Windows path path should be with "/" delimiters instead of "\\"
    // due to usage of INetURLObject() that converts "\\" to hexadecimal notation.
    ::osl::FileBase::getFileURLFromSystemPath(sFileURL, sFileURL);

    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "ToPoint", uno::makeAny(rRange) } });
    dispatchCommand(mxComponent, ".uno:GoToCell", aArgs);

    dispatchCommand(mxComponent, ".uno:DefinePrintArea", {});

    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ViewPDFAfterExport", uno::Any(true) }, { "Printing", uno::Any(sal_Int32(2)) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(OUString("calc_pdf_Export")) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(sFileURL) } }));

    dispatchCommand(mxComponent, ".uno:ExportToPDF", aDescriptor);

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
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("First Sheet", 0);
    uno::Reference<sheet::XSpreadsheet> rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // 2. Setup data
    {
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
        CPPUNIT_ASSERT(xDocSh);

        // put some content into the first row with default font
        ScDocument& rDoc = xDocSh->GetDocument();
        for (unsigned int r = 0; r < 1; ++r)
            for (unsigned int c = 0; c < 14; ++c)
                rDoc.SetValue(ScAddress(c, r, 0), (r + 1) * (c + 1));

        // set "Text" to H1 cell with "DejaVuSans" font
        ScFieldEditEngine& rEE = rDoc.GetEditEngine();
        rEE.Clear();
        rEE.SetTextCurrentDefaults("Text");
        setFont(rEE, 0, 4, "DejaVuSans"); // set font for first 4 chars
        rDoc.SetEditText(ScAddress(7, 0, 0), rEE.CreateTextObject());
    }

    // A1:G1
    {
        ScRange range1(0, 0, 0, 6, 0, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(false, bFound);
    }

    // G1:H1
    {
        ScRange range1(6, 0, 0, 7, 0, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // H1:I1
    {
        ScRange range1(7, 0, 0, 8, 0, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testExportFitToPage_Tdf103516()
{
    // create test document
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName("First Sheet", 0);
    uno::Reference<sheet::XSpreadsheet> rSheet(xIndex->getByIndex(0), UNO_QUERY_THROW);

    // 2. Setup data
    {
        SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
        CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
        ScDocShellRef xDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
        CPPUNIT_ASSERT(xDocSh);

        // put some content into the table
        ScDocument& rDoc = xDocSh->GetDocument();
        for (unsigned int r = 0; r < 80; ++r)
            for (unsigned int c = 0; c < 12; ++c)
                rDoc.SetValue(ScAddress(c, r, 0), (r + 1) * (c + 1));
    }

    // A1:G50: 2-page export
    {
        ScRange range1(0, 0, 0, 6, 49, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "/Count 2>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // A1:L80: 4-page export
    {
        ScRange range1(0, 0, 0, 11, 79, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "/Count 4>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // set fit to page: width=1 page, height=0 (automatic)
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamSupp(xDoc, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xStyleFamiliesNames(xStyleFamSupp->getStyleFamilies(),
                                                               UNO_SET_THROW);
    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamiliesNames->getByName("PageStyles"),
                                                       UNO_QUERY_THROW);
    uno::Any aDefaultStyle = xPageStyles->getByName("Default");
    uno::Reference<beans::XPropertySet> xProp(aDefaultStyle, UNO_QUERY_THROW);

    uno::Any aScaleX, aScaleY;
    sal_Int16 nScale;
    aScaleX <<= static_cast<sal_Int16>(1);
    xProp->setPropertyValue("ScaleToPagesX", aScaleX);
    aScaleX = xProp->getPropertyValue("ScaleToPagesX");
    aScaleX >>= nScale;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nScale);

    aScaleY = xProp->getPropertyValue("ScaleToPagesY");
    aScaleY >>= nScale;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nScale);

    // A1:G50 with fit to page width=1: slightly smaller zoom results only 1-page export
    {
        ScRange range1(0, 0, 0, 6, 49, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "/Count 1>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // A1:L80 with fit to page width=1: slightly smaller zoom results only 1-page export
    {
        ScRange range1(0, 0, 0, 11, 79, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "/Count 1>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testUnoCommands_Tdf120161()
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf120161.ods",
                                  "com.sun.star.sheet.SpreadsheetDocument");

    // A1:G1
    {
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDFWithUnoCommands("A1:G1");
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(false, bFound);
    }

    // G1:H1
    {
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDFWithUnoCommands("G1:H1");
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // H1:I1
    {
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDFWithUnoCommands("H1:I1");
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testTdf64703_hiddenPageBreak()
{
    mxComponent = loadFromDesktop(m_directories.getURLFromSrc(DATA_DIRECTORY)
                                      + "tdf64703_hiddenPageBreak.ods",
                                  "com.sun.star.sheet.SpreadsheetDocument");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:A11: 4-page export
    {
        ScRange range1(0, 0, 0, 0, 10, 0);
        std::shared_ptr<utl::TempFile> pPDFFile = exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf(pPDFFile, "/Count 4>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPDFExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
