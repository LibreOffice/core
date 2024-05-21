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

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/propertysequence.hxx>
#include <unotools/tempfile.hxx>
#include <docsh.hxx>
#include <editutil.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fontitem.hxx>
#include <comphelper/propertyvalue.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>

#if USE_TLS_NSS
#include <nss.h>
#endif
#include <vcl/filter/pdfdocument.hxx>
#include <tools/zcodec.hxx>
#include <o3tl/string_view.hxx>

using namespace css::lang;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class ScPDFExportTest : public UnoApiTest
{
public:
    ScPDFExportTest();
    ~ScPDFExportTest();

    // helpers
private:
    void exportToPDF(const uno::Reference<frame::XModel>& xModel, const ScRange& range);

    void exportToPDFWithUnoCommands(const OUString& rRange);

    bool hasTextInPdf(const char* sText, bool& bFound);

    void setFont(ScFieldEditEngine& rEE, sal_Int32 nStart, sal_Int32 nEnd,
                 const OUString& rFontName);

    // unit tests
public:
    void testMediaShapeScreen_Tdf159094();
    void testExportRange_Tdf120161();
    void testExportFitToPage_Tdf103516();
    void testUnoCommands_Tdf120161();
    void testTdf64703_hiddenPageBreak();
    void testTdf159068();
    void testTdf159067();
    void testTdf159066();
    void testTdf159065();
    void testTdf123870();
    void testTdf143978();
    void testTdf120190();
    void testTdf84012();
    void testTdf78897();
    void testForcepoint97();

    CPPUNIT_TEST_SUITE(ScPDFExportTest);
    CPPUNIT_TEST(testMediaShapeScreen_Tdf159094);
    CPPUNIT_TEST(testExportRange_Tdf120161);
    CPPUNIT_TEST(testExportFitToPage_Tdf103516);
    CPPUNIT_TEST(testUnoCommands_Tdf120161);
    CPPUNIT_TEST(testTdf64703_hiddenPageBreak);
    CPPUNIT_TEST(testTdf159068);
    CPPUNIT_TEST(testTdf159067);
    CPPUNIT_TEST(testTdf159066);
    CPPUNIT_TEST(testTdf159065);
    CPPUNIT_TEST(testTdf123870);
    CPPUNIT_TEST(testTdf143978);
    CPPUNIT_TEST(testTdf120190);
    CPPUNIT_TEST(testTdf84012);
    CPPUNIT_TEST(testTdf78897);
    CPPUNIT_TEST(testForcepoint97);
    CPPUNIT_TEST_SUITE_END();
};

ScPDFExportTest::ScPDFExportTest()
    : UnoApiTest(u"sc/qa/extras/testdocuments/"_ustr)
{
}

ScPDFExportTest::~ScPDFExportTest()
{
#if USE_TLS_NSS
    NSS_Shutdown();
#endif
}

bool ScPDFExportTest::hasTextInPdf(const char* sText, bool& bFound)
{
    SvStream* pStream = maTempFile.GetStream(StreamMode::STD_READ);
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
    maTempFile.CloseStream();
    return (nRead == nFileSize);
}

void ScPDFExportTest::exportToPDF(const uno::Reference<frame::XModel>& xModel, const ScRange& range)
{
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
        comphelper::makePropertyValue(u"Selection"_ustr, xCellRange),
        comphelper::makePropertyValue(u"Printing"_ustr, sal_Int32(2)),
        comphelper::makePropertyValue(u"ViewPDFAfterExport"_ustr, true),
        comphelper::makePropertyValue(u"PDFUACompliance"_ustr, true)
    };

    // init set of params for storeToURL() call
    css::uno::Sequence<css::beans::PropertyValue> seqArguments{
        comphelper::makePropertyValue(u"FilterData"_ustr, aFilterData),
        comphelper::makePropertyValue(u"FilterName"_ustr, u"calc_pdf_Export"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, maTempFile.GetURL())
    };

    // call storeToURL()
    uno::Reference<lang::XComponent> xComponent(mxComponent, UNO_SET_THROW);
    uno::Reference<css::frame::XStorable> xStorable(xComponent, UNO_QUERY);
    xStorable->storeToURL(maTempFile.GetURL(), seqArguments);
}

void ScPDFExportTest::exportToPDFWithUnoCommands(const OUString& rRange)
{
    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "ToPoint", uno::Any(rRange) } });
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aArgs);

    dispatchCommand(mxComponent, u".uno:DefinePrintArea"_ustr, {});

    uno::Sequence<beans::PropertyValue> aFilterData(comphelper::InitPropertySequence(
        { { "ViewPDFAfterExport", uno::Any(true) }, { "Printing", uno::Any(sal_Int32(2)) } }));

    uno::Sequence<beans::PropertyValue> aDescriptor(
        comphelper::InitPropertySequence({ { "FilterName", uno::Any(u"calc_pdf_Export"_ustr) },
                                           { "FilterData", uno::Any(aFilterData) },
                                           { "URL", uno::Any(maTempFile.GetURL()) } }));

    dispatchCommand(mxComponent, u".uno:ExportToPDF"_ustr, aDescriptor);
}

void ScPDFExportTest::setFont(ScFieldEditEngine& rEE, sal_Int32 nStart, sal_Int32 nEnd,
                              const OUString& rFontName)
{
    ESelection aSel;
    aSel.nStartPara = aSel.nEndPara = 0;
    aSel.nStartPos = nStart;
    aSel.nEndPos = nEnd;

    SfxItemSet aItemSet = rEE.GetEmptyItemSet();
    SvxFontItem aItem(FAMILY_MODERN, rFontName, u""_ustr, PITCH_VARIABLE, RTL_TEXTENCODING_UTF8,
                      EE_CHAR_FONTINFO);
    aItemSet.Put(aItem);
    rEE.QuickSetAttribs(aItemSet, aSel);
}

void ScPDFExportTest::testMediaShapeScreen_Tdf159094()
{
    loadFromFile(u"tdf159094.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:B8
    ScRange aRange(0, 0, 0, 1, 7, 0);

    // Without the fix, this test would crash on export media file to pdf
    exportToPDF(xModel, aRange);
}

// Selection was not taken into account during export into PDF
void ScPDFExportTest::testExportRange_Tdf120161()
{
    // create test document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName(u"First Sheet"_ustr, 0);
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
        rEE.SetTextCurrentDefaults(u"Text"_ustr);
        setFont(rEE, 0, 4, u"DejaVuSans"_ustr); // set font for first 4 chars
        rDoc.SetEditText(ScAddress(7, 0, 0), rEE.CreateTextObject());
    }

    // A1:G1
    {
        ScRange range1(0, 0, 0, 6, 0, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(false, bFound);
    }

    // G1:H1
    {
        ScRange range1(6, 0, 0, 7, 0, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // H1:I1
    {
        ScRange range1(7, 0, 0, 8, 0, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testExportFitToPage_Tdf103516()
{
    // create test document
    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<sheet::XSpreadsheetDocument> xDoc(xModel, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIndex(xSheets, uno::UNO_QUERY_THROW);
    xSheets->insertNewByName(u"First Sheet"_ustr, 0);
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
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("/Count 2>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // A1:L80: 4-page export
    {
        ScRange range1(0, 0, 0, 11, 79, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("/Count 4>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // set fit to page: width=1 page, height=0 (automatic)
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamSupp(xDoc, UNO_QUERY_THROW);
    uno::Reference<container::XNameAccess> xStyleFamiliesNames(xStyleFamSupp->getStyleFamilies(),
                                                               UNO_SET_THROW);
    uno::Reference<container::XNameAccess> xPageStyles(
        xStyleFamiliesNames->getByName(u"PageStyles"_ustr), UNO_QUERY_THROW);
    uno::Any aDefaultStyle = xPageStyles->getByName(u"Default"_ustr);
    uno::Reference<beans::XPropertySet> xProp(aDefaultStyle, UNO_QUERY_THROW);

    uno::Any aScaleX, aScaleY;
    sal_Int16 nScale;
    aScaleX <<= static_cast<sal_Int16>(1);
    xProp->setPropertyValue(u"ScaleToPagesX"_ustr, aScaleX);
    aScaleX = xProp->getPropertyValue(u"ScaleToPagesX"_ustr);
    aScaleX >>= nScale;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), nScale);

    aScaleY = xProp->getPropertyValue(u"ScaleToPagesY"_ustr);
    aScaleY >>= nScale;
    CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nScale);

    // A1:G50 with fit to page width=1: slightly smaller zoom results only 1-page export
    {
        ScRange range1(0, 0, 0, 6, 49, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("/Count 1>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // A1:L80 with fit to page width=1: slightly smaller zoom results only 1-page export
    {
        ScRange range1(0, 0, 0, 11, 79, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("/Count 1>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testUnoCommands_Tdf120161()
{
    loadFromFile(u"tdf120161.ods");

    // A1:G1
    {
        exportToPDFWithUnoCommands(u"A1:G1"_ustr);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(false, bFound);
    }

    // G1:H1
    {
        exportToPDFWithUnoCommands(u"G1:H1"_ustr);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }

    // H1:I1
    {
        exportToPDFWithUnoCommands(u"H1:I1"_ustr);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("DejaVuSans", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testTdf64703_hiddenPageBreak()
{
    loadFromFile(u"tdf64703_hiddenPageBreak.ods");

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:A11: 4-page export
    {
        ScRange range1(0, 0, 0, 0, 10, 0);
        exportToPDF(xModel, range1);
        bool bFound = false;
        CPPUNIT_ASSERT(hasTextInPdf("/Count 4>>", bFound));
        CPPUNIT_ASSERT_EQUAL(true, bFound);
    }
}

void ScPDFExportTest::testTdf159068()
{
    loadFromFile(u"tdf159068.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:C3
    ScRange range1(0, 0, 0, 2, 2, 0);
    exportToPDF(xModel, range1);

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);

    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    auto nArtifact(0);
    auto nLine(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n ";
            if (o3tl::starts_with(line, "/Artifact BMC"))
                nArtifact++;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected: 5 (Artifact: Header, Footer, Rectangle, DetectiveArrow, ValidationCircle)
    // - Actual  : 2 (Artifact: Header, Footer)
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nArtifact)>(5), nArtifact);
}

void ScPDFExportTest::testTdf159067()
{
    loadFromFile(u"tdf159067.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:B3
    ScRange range1(0, 0, 0, 1, 2, 0);
    exportToPDF(xModel, range1);

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);

    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    auto nArtifact(0);
    auto nLine(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n ";
            if (o3tl::starts_with(line, "/Artifact BMC"))
                nArtifact++;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected: 3 (Artifact: Header, Footer, TextBox)
    // - Actual  : 2 (Artifact: Header, Footer)
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nArtifact)>(3), nArtifact);
}

void ScPDFExportTest::testTdf159066()
{
    loadFromFile(u"tdf159066.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:E5
    ScRange range1(0, 0, 0, 4, 4, 0);
    exportToPDF(xModel, range1);

    bool bFound = false;
    CPPUNIT_ASSERT(hasTextInPdf("/Alt<", bFound));

    // The OLE object contains alternative text description
    CPPUNIT_ASSERT_EQUAL(true, bFound);
}

void ScPDFExportTest::testTdf159065()
{
    loadFromFile(u"tdf159065.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:A3
    ScRange range1(0, 0, 0, 0, 2, 0);
    exportToPDF(xModel, range1);

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);

    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    auto nLink(0);
    auto nLine(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n ";
            if (o3tl::starts_with(line, "/Link<</MCID") && o3tl::ends_with(line, ">>BDC"))
                nLink++;
        }
    }

    // The tagged PDF file have to contains two link annotation
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nLink)>(2), nLink);
}

void ScPDFExportTest::testTdf123870()
{
    loadFromFile(u"tdf123870.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:G4
    ScRange range1(0, 0, 0, 6, 4, 0);
    exportToPDF(xModel, range1);

    vcl::filter::PDFDocument aDocument;
    SvFileStream aStream(maTempFile.GetURL(), StreamMode::READ);
    CPPUNIT_ASSERT(aDocument.Read(aStream));

    // The document has one page.
    std::vector<vcl::filter::PDFObjectElement*> aPages = aDocument.GetPages();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aPages.size());

    vcl::filter::PDFObjectElement* pContents = aPages[0]->LookupObject("Contents"_ostr);
    CPPUNIT_ASSERT(pContents);
    vcl::filter::PDFStreamElement* pStream = pContents->GetStream();
    CPPUNIT_ASSERT(pStream);
    SvMemoryStream& rObjectStream = pStream->GetMemory();
    // Uncompress it.
    SvMemoryStream aUncompressed;
    ZCodec aZCodec;
    aZCodec.BeginCompression();
    rObjectStream.Seek(0);
    aZCodec.Decompress(rObjectStream, aUncompressed);
    CPPUNIT_ASSERT(aZCodec.EndCompression());

    auto pStart = static_cast<const char*>(aUncompressed.GetData());
    const char* const pEnd = pStart + aUncompressed.GetSize();

    enum
    {
        Default,
        Artifact,
        Tagged
    } state
        = Default;

    auto nLine(0);
    auto nTagged(0);
    auto nArtifacts(0);
    while (true)
    {
        ++nLine;
        auto const pLine = ::std::find(pStart, pEnd, '\n');
        if (pLine == pEnd)
        {
            break;
        }
        std::string_view const line(pStart, pLine - pStart);
        pStart = pLine + 1;
        if (!line.empty() && line[0] != '%')
        {
            ::std::cerr << nLine << ": " << line << "\n ";
            if (o3tl::ends_with(line, "/Artifact BMC"))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Artifact;
                ++nArtifacts;
            }
            else if ((o3tl::starts_with(line, "/P<</MCID") && o3tl::ends_with(line, ">>BDC"))
                     || (o3tl::starts_with(line, "/Figure<</MCID")
                         && o3tl::ends_with(line, ">>BDC")))
            {
                CPPUNIT_ASSERT_EQUAL_MESSAGE("unexpected nesting", Default, state);
                state = Tagged;
                ++nTagged;
            }
            else if (line == "EMC")
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected end", state != Default);
                state = Default;
            }
            else if (nLine > 1) // first line is expected "0.1 w"
            {
                CPPUNIT_ASSERT_MESSAGE("unexpected content outside MCS", state != Default);
            }
        }
    }
    // text in cell + 1 shape
    CPPUNIT_ASSERT_EQUAL(static_cast<decltype(nTagged)>(9), nTagged);
    // header, footer, background color, color scale, shadow, cell border
    CPPUNIT_ASSERT(nArtifacts >= 6);
}

void ScPDFExportTest::testTdf143978()
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    loadFromFile(u"tdf143978.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:A2
    ScRange range1(0, 0, 0, 0, 1, 0);
    exportToPDF(xModel, range1);
    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(2, nPageObjectCount);

    // Without the fix in place, this test would have failed with
    // - Expected: Dies ist viel zu viel Text
    // - Actual  : Dies ist vie
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject1 = pPdfPage->getObject(0);
    OUString sText1 = pPageObject1->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"Dies ist viel zu viel Text"_ustr, sText1);

    // and it would also have failed with
    // - Expected: 2021-11-17
    // - Actual  : ###
    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject2 = pPdfPage->getObject(1);
    OUString sText2 = pPageObject2->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"2021-11-17"_ustr, sText2);
}

void ScPDFExportTest::testTdf120190()
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    mxComponent = loadFromDesktop(u"private:factory/scalc"_ustr);
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_MESSAGE("no calc document", xDoc.is());

    uno::Reference<sheet::XSpreadsheets> xSheets(xDoc->getSheets(), uno::UNO_SET_THROW);
    uno::Reference<container::XIndexAccess> xIA(xSheets, uno::UNO_QUERY_THROW);
    uno::Reference<sheet::XSpreadsheet> xSheet0(xIA->getByIndex(0), uno::UNO_QUERY_THROW);

    xSheet0->getCellByPosition(0, 0)->setFormula(u"=5&CHAR(10)&6"_ustr);

    uno::Sequence<beans::PropertyValue> aArgs
        = comphelper::InitPropertySequence({ { "ToPoint", uno::Any(u"A1"_ustr) } });
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aArgs);

    dispatchCommand(mxComponent, u".uno:ConvertFormulaToValue"_ustr, {});

    // A1
    ScRange range1(0, 0, 0, 0, 0, 0);
    exportToPDF(xModel, range1);

    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    int nPageObjectCount = pPdfPage->getObjectCount();
    CPPUNIT_ASSERT_EQUAL(5, nPageObjectCount);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject1 = pPdfPage->getObject(0);
    OUString sText1 = pPageObject1->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, sText1);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject2 = pPdfPage->getObject(1);
    OUString sText2 = pPageObject2->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"Page "_ustr, sText2);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject3 = pPdfPage->getObject(2);
    OUString sText3 = pPageObject3->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sText3);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject4 = pPdfPage->getObject(3);
    OUString sText4 = pPageObject4->getText(pTextPage);

    // Without the fix in place, this test would have failed with
    // - Expected: 5
    // - Actual  : 56
    CPPUNIT_ASSERT_EQUAL(u"5"_ustr, sText4);

    std::unique_ptr<vcl::pdf::PDFiumPageObject> pPageObject5 = pPdfPage->getObject(4);
    OUString sText5 = pPageObject5->getText(pTextPage);
    CPPUNIT_ASSERT_EQUAL(u"6"_ustr, sText5);
}

void ScPDFExportTest::testTdf84012()
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    loadFromFile(u"tdf84012.ods");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1
    ScRange range1(0, 0, 0, 0, 0, 0);
    exportToPDF(xModel, range1);
    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    int nChars = pTextPage->countChars();
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());

    // Without the fix in place, this test would have failed with
    // - Expected: Blah blah (blah, blah)
    // - Actual  : Blah blah
    CPPUNIT_ASSERT_EQUAL(u"Blah blah (blah, blah)"_ustr, aActualText);
}

void ScPDFExportTest::testTdf78897()
{
    std::shared_ptr<vcl::pdf::PDFium> pPDFium = vcl::pdf::PDFiumLibrary::get();
    if (!pPDFium)
    {
        return;
    }

    loadFromFile(u"tdf78897.xls");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // C3:D3
    ScRange range1(2, 2, 0, 3, 2, 0);
    exportToPDF(xModel, range1);
    // Parse the export result with pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    CPPUNIT_ASSERT_EQUAL(1, pPdfDocument->getPageCount());

    // Get the first page
    std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(/*nIndex=*/0);
    CPPUNIT_ASSERT(pPdfPage);
    std::unique_ptr<vcl::pdf::PDFiumTextPage> pTextPage = pPdfPage->getTextPage();

    int nChars = pTextPage->countChars();
    std::vector<sal_uInt32> aChars(nChars);
    for (int i = 0; i < nChars; i++)
        aChars[i] = pTextPage->getUnicode(i);
    OUString aActualText(aChars.data(), aChars.size());

    // Without the fix in place, this test would have failed with
    // - Expected:  11.00 11.00
    // - Actual  :  11.00 ###
    CPPUNIT_ASSERT_EQUAL(u" 11.00 11.00 "_ustr, aActualText);
}

// just needs to not crash on export to pdf
void ScPDFExportTest::testForcepoint97()
{
    loadFromFile(u"forcepoint97.xlsx");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);

    // A1:H81
    ScRange range1(0, 0, 0, 7, 81, 0);
    exportToPDF(xModel, range1);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScPDFExportTest);
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
