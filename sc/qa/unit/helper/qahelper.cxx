/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "qahelper.hxx"
#include "csv_handler.hxx"
#include "debughelper.hxx"
#include <drwlayer.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/storagehelper.hxx>
#include <compiler.hxx>
#include <conditio.hxx>
#include <stlsheet.hxx>
#include <formulacell.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>
#include <tools/UnitConversion.hxx>
#include <tools/urlobj.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/justifyitem.hxx>
#include <formula/errorcodes.hxx>
#include <clipcontext.hxx>
#include <clipparam.hxx>
#include <cppunit/Asserter.h>
#include <cppunit/AdditionalMessage.h>
#include <refundo.hxx>
#include <sal/log.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/gridprinter.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <undoblk.hxx>
#include <unotools/tempfile.hxx>
#include <scdll.hxx>
#include <scitems.hxx>
#include <stringutil.hxx>
#include <tokenarray.hxx>

#include <orcus/csv_parser.hpp>

#include <cstdlib>
#include <fstream>

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>


using namespace com::sun::star;
using namespace ::com::sun::star::uno;

FormulaGrammarSwitch::FormulaGrammarSwitch(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGrammar) :
    mpDoc(pDoc), meOldGrammar(pDoc->GetGrammar())
{
    mpDoc->SetGrammar(eGrammar);
}

FormulaGrammarSwitch::~FormulaGrammarSwitch()
{
    mpDoc->SetGrammar(meOldGrammar);
}

// calc data structure pretty printer
std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr)
{
    rStrm << "Col: " << rAddr.Col() << " Row: " << rAddr.Row() << " Tab: " << rAddr.Tab() << "\n";
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const ScRange& rRange)
{
    rStrm << "ScRange: " << rRange.aStart << rRange.aEnd << "\n";
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const ScRangeList& rList)
{
    rStrm << "ScRangeList: \n";
    for(size_t i = 0; i < rList.size(); ++i)
        rStrm << rList[i];
    return rStrm;
}

std::ostream& operator<<(std::ostream& rStrm, const OpCode& rCode)
{
    rStrm << static_cast<sal_uInt16>(rCode);
    return rStrm;
}

const FileFormat ScBootstrapFixture::aFileFormats[] = {
    { "ods" , "calc8", "", ODS_FORMAT_TYPE },
    { "xls" , "MS Excel 97", "calc_MS_EXCEL_97", XLS_FORMAT_TYPE },
    { "xlsx", "Calc Office Open XML" , "Office Open XML Spreadsheet", XLSX_FORMAT_TYPE },
    { "xlsm", "Calc Office Open XML" , "Office Open XML Spreadsheet", XLSX_FORMAT_TYPE },
    { "csv" , "Text - txt - csv (StarCalc)", "generic_Text", CSV_FORMAT_TYPE },
    { "html" , "calc_HTML_WebQuery", "generic_HTML", HTML_FORMAT_TYPE },
    { "123" , "Lotus", "calc_Lotus", LOTUS123_FORMAT_TYPE },
    { "dif", "DIF", "calc_DIF", DIF_FORMAT_TYPE },
    { "xml", "MS Excel 2003 XML Orcus", "calc_MS_Excel_2003_XML", XLS_XML_FORMAT_TYPE },
    { "xlsb", "Calc MS Excel 2007 Binary", "MS Excel 2007 Binary", XLSB_XML_FORMAT_TYPE },
    { "fods", "OpenDocument Spreadsheet Flat XML", "calc_ODS_FlatXML", FODS_FORMAT_TYPE },
    { "gnumeric", "Gnumeric Spreadsheet", "Gnumeric XML", GNUMERIC_FORMAT_TYPE },
    { "xltx", "Calc Office Open XML Template", "Office Open XML Spreadsheet Template", XLTX_FORMAT_TYPE }
};

bool testEqualsWithTolerance( tools::Long nVal1, tools::Long nVal2, tools::Long nTol )
{
    return ( std::abs( nVal1 - nVal2 ) <= nTol );
}

void ScModelTestBase::loadFile(const OUString& aFileName, std::string& aContent)
{
    OString aOFileName = OUStringToOString(aFileName, RTL_TEXTENCODING_UTF8);

#ifdef ANDROID
    size_t size;
    if (strncmp(aOFileName.getStr(), "/assets/", sizeof("/assets/")-1) == 0) {
        const char *contents = (const char *) lo_apkentry(aOFileName.getStr(), &size);
        if (contents != 0) {
            aContent = std::string(contents, size);
            return;
        }
    }
#endif

    std::ifstream aFile(aOFileName.getStr());

    OString aErrorMsg = "Could not open csv file: " + aOFileName;
    CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), aFile);
    std::ostringstream aOStream;
    aOStream << aFile.rdbuf();
    aFile.close();
    aContent = aOStream.str();
}

void ScModelTestBase::testFile(const OUString& aFileName, ScDocument& rDoc, SCTAB nTab, StringType aStringFormat)
{
    csv_handler aHandler(&rDoc, nTab, aStringFormat);
    orcus::csv::parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    aConfig.trim_cell_value = false;

    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<csv_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv::parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }
}

void ScModelTestBase::testCondFile(const OUString& aFileName, ScDocument* pDoc, SCTAB nTab)
{
    conditional_format_handler aHandler(pDoc, nTab);
    orcus::csv::parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<conditional_format_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv::parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }
}

void ScModelTestBase::testFormats(ScDocument* pDoc,std::u16string_view sFormat)
{
    //test Sheet1 with csv file
    OUString aCSVFileName = createFilePath(u"contentCSV/numberFormat.csv");
    testFile(aCSVFileName, *pDoc, 0, StringType::PureString);
    //need to test the color of B3
    //it's not a font color!
    //formatting for B5: # ??/100 gets lost during import

    //test Sheet2
    const ScPatternAttr* pPattern = pDoc->GetPattern(0, 0, 1);
    vcl::Font aFont;
    pPattern->GetFont(aFont,SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font size should be 10", tools::Long(200), aFont.GetFontSize().getHeight());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font color should be black", COL_AUTO, aFont.GetColor());
    pPattern = pDoc->GetPattern(0,1,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font size should be 12", tools::Long(240), aFont.GetFontSize().getHeight());
    pPattern = pDoc->GetPattern(0,2,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be italic", ITALIC_NORMAL, aFont.GetItalic());
    pPattern = pDoc->GetPattern(0,4,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold", WEIGHT_BOLD, aFont.GetWeight());
    pPattern = pDoc->GetPattern(1,0,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be blue", COL_BLUE, aFont.GetColor());
    pPattern = pDoc->GetPattern(1,1,1);
    pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out with a single line", STRIKEOUT_SINGLE, aFont.GetStrikeout());
    //some tests on sheet2 only for ods
    if (sFormat == u"calc8")
    {
        pPattern = pDoc->GetPattern(1,2,1);
        pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be striked out with a double line", STRIKEOUT_DOUBLE, aFont.GetStrikeout());
        pPattern = pDoc->GetPattern(1,3,1);
        pPattern->GetFont(aFont, SC_AUTOCOL_RAW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be underlined with a dotted line", LINESTYLE_DOTTED, aFont.GetUnderline());
        //check row height import
        //disable for now until we figure out cause of win tinderboxes test failures
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(256), pDoc->GetRowHeight(0,1) ); //0.178in
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(304), pDoc->GetRowHeight(1,1) ); //0.211in
        //CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(477), pDoc->GetRowHeight(5,1) ); //0.3311in
        //check column width import
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(555), pDoc->GetColWidth(4,1) );  //0.3854in
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(1280), pDoc->GetColWidth(5,1) ); //0.889in
        CPPUNIT_ASSERT_EQUAL( static_cast<sal_uInt16>(4153), pDoc->GetColWidth(6,1) ); //2.8839in
        //test case for i53253 where a cell has text with different styles and space between the text.
        OUString aTestStr = pDoc->GetString(3,0,1);
        OUString aKnownGoodStr("text14 space");
        CPPUNIT_ASSERT_EQUAL( aKnownGoodStr, aTestStr );
        //test case for cell text with line breaks.
        aTestStr = pDoc->GetString(3,5,1);
        aKnownGoodStr = "Hello,\nCalc!";
        CPPUNIT_ASSERT_EQUAL( aKnownGoodStr, aTestStr );
    }
    pPattern = pDoc->GetPattern(1,4,1);
    Color aColor = pPattern->GetItem(ATTR_BACKGROUND).GetColor();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("background color should be green", COL_LIGHTGREEN, aColor);
    pPattern = pDoc->GetPattern(2,0,1);
    SvxCellHorJustify eHorJustify = pPattern->GetItem(ATTR_HOR_JUSTIFY).GetValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned centre horizontally", SvxCellHorJustify::Center, eHorJustify);
    //test alignment
    pPattern = pDoc->GetPattern(2,1,1);
    eHorJustify = pPattern->GetItem(ATTR_HOR_JUSTIFY).GetValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned right horizontally", SvxCellHorJustify::Right, eHorJustify);
    pPattern = pDoc->GetPattern(2,2,1);
    eHorJustify = pPattern->GetItem(ATTR_HOR_JUSTIFY).GetValue();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("cell content should be aligned block horizontally", SvxCellHorJustify::Block, eHorJustify);

    //test Sheet3 only for ods and xlsx
    if ( sFormat == u"calc8" || sFormat == u"Calc Office Open XML" )
    {
        aCSVFileName = createFilePath(u"contentCSV/conditionalFormatting.csv");
        testCondFile(aCSVFileName, pDoc, 2);
        // test parent cell style import ( fdo#55198 )
        if ( sFormat == u"Calc Office Open XML" )
        {
            pPattern = pDoc->GetPattern(1,1,3);
            ScStyleSheet* pStyleSheet = const_cast<ScStyleSheet*>(pPattern->GetStyleSheet());
            // check parent style name
            OUString sExpected("Excel Built-in Date");
            OUString sResult = pStyleSheet->GetName();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("parent style for Sheet4.B2 is 'Excel Built-in Date'", sExpected, sResult);
            // check  align of style
            SfxItemSet& rItemSet = pStyleSheet->GetItemSet();
            eHorJustify = rItemSet.Get( ATTR_HOR_JUSTIFY ).GetValue();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("'Excel Built-in Date' style should be aligned centre horizontally", SvxCellHorJustify::Center, eHorJustify);
            // check date format ( should be just month e.g. 29 )
            sResult =pDoc->GetString( 1,1,3 );
            sExpected = "29";
            CPPUNIT_ASSERT_EQUAL_MESSAGE("'Excel Built-in Date' style should just display month", sExpected, sResult );

            // check actual align applied to cell, should be the same as
            // the style
            eHorJustify = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("cell with 'Excel Built-in Date' style should be aligned centre horizontally", SvxCellHorJustify::Center, eHorJustify);
        }
    }

    ScConditionalFormat* pCondFormat = pDoc->GetCondFormat(0,0,2);
    const ScRangeList& rRange = pCondFormat->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,0,2,3,0,2)), rRange);

    pCondFormat = pDoc->GetCondFormat(0,1,2);
    const ScRangeList& rRange2 = pCondFormat->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(0,1,2,0,1,2)), rRange2);

    pCondFormat = pDoc->GetCondFormat(1,1,2);
    const ScRangeList& rRange3 = pCondFormat->GetRange();
    CPPUNIT_ASSERT_EQUAL(ScRangeList(ScRange(1,1,2,3,1,2)), rRange3);
}

const SdrOle2Obj* ScModelTestBase::getSingleOleObject(ScDocument& rDoc, sal_uInt16 nPage)
{
    // Retrieve the chart object instance from the 2nd page (for the 2nd sheet).
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    if (!pDrawLayer)
    {
        cout << "Failed to retrieve the drawing layer object." << endl;
        return nullptr;
    }

    const SdrPage* pPage = pDrawLayer->GetPage(nPage);
    if (!pPage)
    {
        cout << "Failed to retrieve the page object." << endl;
        return nullptr;
    }

    if (pPage->GetObjCount() != 1)
    {
        cout << "This page should contain one drawing object." << endl;
        return nullptr;
    }

    const SdrObject* pObj = pPage->GetObj(0);
    if (!pObj)
    {
        cout << "Failed to retrieve the drawing object." << endl;
        return nullptr;
    }

    if (pObj->GetObjIdentifier() != SdrObjKind::OLE2)
    {
        cout << "This is not an OLE2 object." << endl;
        return nullptr;
    }

    return static_cast<const SdrOle2Obj*>(pObj);
}

const SdrOle2Obj* ScModelTestBase::getSingleChartObject(ScDocument& rDoc, sal_uInt16 nPage)
{
    const SdrOle2Obj* pObj = getSingleOleObject(rDoc, nPage);

    if (!pObj)
        return pObj;

    if (!pObj->IsChart())
    {
        cout << "This should be a chart object." << endl;
        return nullptr;
    }

    return pObj;
}

static std::vector<OUString> getChartRangeRepresentations(const SdrOle2Obj& rChartObj)
{
    std::vector<OUString> aRangeReps;

    // Make sure the chart object has correct range references.
    Reference<frame::XModel> xModel = rChartObj.getXModel();
    if (!xModel.is())
    {
        cout << "Failed to get the embedded object interface." << endl;
        return aRangeReps;
    }

    Reference<chart2::XChartDocument> xChartDoc(xModel, UNO_QUERY);
    if (!xChartDoc.is())
    {
        cout << "Failed to get the chart document interface." << endl;
        return aRangeReps;
    }

    Reference<chart2::data::XDataSource> xDataSource(xChartDoc, UNO_QUERY);
    if (!xDataSource.is())
    {
        cout << "Failed to get the data source interface." << endl;
        return aRangeReps;
    }

    Sequence<Reference<chart2::data::XLabeledDataSequence> > xDataSeqs = xDataSource->getDataSequences();
    if (!xDataSeqs.hasElements())
    {
        cout << "There should be at least one data sequences." << endl;
        return aRangeReps;
    }

    Reference<chart2::data::XDataReceiver> xDataRec(xChartDoc, UNO_QUERY);
    if (!xDataRec.is())
    {
        cout << "Failed to get the data receiver interface." << endl;
        return aRangeReps;
    }

    Sequence<OUString> aRangeRepSeqs = xDataRec->getUsedRangeRepresentations();
    comphelper::sequenceToContainer(aRangeReps, aRangeRepSeqs);

    return aRangeReps;
}

ScRangeList ScModelTestBase::getChartRanges(ScDocument& rDoc, const SdrOle2Obj& rChartObj)
{
    std::vector<OUString> aRangeReps = getChartRangeRepresentations(rChartObj);
    ScRangeList aRanges;
    for (size_t i = 0, n = aRangeReps.size(); i < n; ++i)
    {
        ScRange aRange;
        ScRefFlags nRes = aRange.Parse(aRangeReps[i], rDoc, rDoc.GetAddressConvention());
        if (nRes & ScRefFlags::VALID)
            // This is a range address.
            aRanges.push_back(aRange);
        else
        {
            // Parse it as a single cell address.
            ScAddress aAddr;
            nRes = aAddr.Parse(aRangeReps[i], rDoc, rDoc.GetAddressConvention());
            CPPUNIT_ASSERT_MESSAGE("Failed to parse a range representation.", (nRes & ScRefFlags::VALID));
            aRanges.push_back(aAddr);
        }
    }

    return aRanges;
}

ScTokenArray* getTokens(ScDocument& rDoc, const ScAddress& rPos)
{
    ScFormulaCell* pCell = rDoc.GetFormulaCell(rPos);
    if (!pCell)
    {
        OUString aStr = rPos.Format(ScRefFlags::VALID);
        cerr << aStr << " is not a formula cell." << endl;
        return nullptr;
    }

    return pCell->GetCode();
}

bool checkOutput(
    const ScDocument* pDoc, const ScRange& aOutRange,
    const std::vector<std::vector<const char*>>& aCheck, const char* pCaption )
{
    bool bResult = true;
    const ScAddress& s = aOutRange.aStart;
    const ScAddress& e = aOutRange.aEnd;
    svl::GridPrinter printer(e.Row() - s.Row() + 1, e.Col() - s.Col() + 1, CALC_DEBUG_OUTPUT != 0);
    SCROW nOutRowSize = e.Row() - s.Row() + 1;
    SCCOL nOutColSize = e.Col() - s.Col() + 1;
    for (SCROW nRow = 0; nRow < nOutRowSize; ++nRow)
    {
        for (SCCOL nCol = 0; nCol < nOutColSize; ++nCol)
        {
            OUString aVal = pDoc->GetString(nCol + s.Col(), nRow + s.Row(), s.Tab());
            printer.set(nRow, nCol, aVal);
            const char* p = aCheck[nRow][nCol];
            if (p)
            {
                OUString aCheckVal = OUString::createFromAscii(p);
                bool bEqual = aCheckVal == aVal;
                if (!bEqual)
                {
                    std::cout << "Expected: " << aCheckVal << "  Actual: " << aVal << std::endl;
                    bResult = false;
                }
            }
            else if (!aVal.isEmpty())
            {
                std::cout << "Empty cell expected" << std::endl;
                bResult = false;
            }
        }
    }
    printer.print(pCaption);
    return bResult;
}

OUString toString(
    ScDocument& rDoc, const ScAddress& rPos, ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram)
{
    ScCompiler aComp(rDoc, rPos, rArray, eGram);
    OUStringBuffer aBuf;
    aComp.CreateStringFromTokenArray(aBuf);
    return aBuf.makeStringAndClear();
}

ScDocShellRef ScBootstrapFixture::load(
    const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
    const OUString& rTypeName, SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
     sal_Int32 nFilterVersion, const OUString* pPassword )
{
    auto pFilter = std::make_shared<SfxFilter>(
        rFilter,
        OUString(), nFilterFlags, nClipboardID, rTypeName, OUString(),
        rUserData, "private:factory/scalc");
    pFilter->SetVersion(nFilterVersion);

    ScDocShellRef xDocShRef = new ScDocShell;
    xDocShRef->GetDocument().EnableUserInteraction(false);
    SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ );
    pSrcMed->SetFilter(pFilter);
    pSrcMed->UseInteractionHandler(false);
    SfxItemSet* pSet = pSrcMed->GetItemSet();
    if (pPassword)
    {
        pSet->Put(SfxStringItem(SID_PASSWORD, *pPassword));
    }
    pSet->Put(SfxUInt16Item(SID_MACROEXECMODE,css::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN));
    SAL_INFO( "sc.qa", "about to load " << rURL );
    if (!xDocShRef->DoLoad(pSrcMed))
    {
        xDocShRef->DoClose();
        // load failed.
        xDocShRef.clear();
    }

    return xDocShRef;
}

ScDocShellRef ScBootstrapFixture::loadDoc(
    std::u16string_view rFileName, sal_Int32 nFormat, bool bCheckErrorCode )
{
    OUString aFileExtension = OUString::fromUtf8(aFileFormats[nFormat].pName);
    OUString aFileName;
    createFileURL( rFileName, aFileExtension, aFileName );

    OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
    OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
    SfxFilterFlags nFormatType = aFileFormats[nFormat].nFormatType;
    SotClipboardFormatId nClipboardId = SotClipboardFormatId::NONE;
    if (nFormatType != SfxFilterFlags::NONE)
        nClipboardId = SotClipboardFormatId::STARCALC_8;

    ScDocShellRef xDocShRef = load(aFileName, aFilterName, OUString(), aFilterType, nFormatType, nClipboardId, static_cast<sal_uIntPtr>(nFormatType));
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to load " + OUStringToOString(rFileName, RTL_TEXTENCODING_UTF8)).getStr(), xDocShRef.is());

    if (bCheckErrorCode)
    {
        CPPUNIT_ASSERT(!xDocShRef->GetErrorCode());
    }

    return xDocShRef;
}

ScBootstrapFixture::ScBootstrapFixture( const OUString& rsBaseString ) : m_aBaseString( rsBaseString ) {}
ScBootstrapFixture::~ScBootstrapFixture() {}

void ScBootstrapFixture::createFileURL(
    std::u16string_view aFileBase, std::u16string_view aFileExtension, OUString& rFilePath)
{
    // m_aBaseString and aFileBase may contain multiple segments, so use
    // GetNewAbsURL instead of insertName for them:
    INetURLObject url(m_directories.getSrcRootURL());
    url.setFinalSlash();
    url.GetNewAbsURL(m_aBaseString, &url);
    url.insertName(aFileExtension, true);
    url.GetNewAbsURL(OUString::Concat(aFileBase) + aFileExtension, &url);
    rFilePath = url.GetMainURL(INetURLObject::DecodeMechanism::NONE);
}

void ScBootstrapFixture::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScBootstrapFixture::tearDown()
{
    uno::Reference< lang::XComponent >( m_xCalcComponent, UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

void ScSimpleBootstrapFixture::setUp()
{
    BootstrapFixture::setUp();

    ScDLL::Init();

    m_xDocShell
        = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT | SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS
                         | SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);
    m_xDocShell->DoInitUnitTest();

    m_pDoc = &m_xDocShell->GetDocument();
}

void ScSimpleBootstrapFixture::tearDown()
{
    m_xDocShell->DoClose();
    m_xDocShell.clear();

    test::BootstrapFixture::tearDown();
}

void ScModelTestBase::createScDoc(const char* pName, const char* pPassword)
{
    if (!pName)
        load("private:factory/scalc");
    else
        loadFromURL(OUString::createFromAscii(pName), pPassword);

    uno::Reference<lang::XServiceInfo> xServiceInfo(mxComponent, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.sheet.SpreadsheetDocument"));
}

ScDocument* ScModelTestBase::getScDoc()
{
    ScModelObj* pModelObj = dynamic_cast<ScModelObj*>(mxComponent.get());
    CPPUNIT_ASSERT(pModelObj);
    return pModelObj->GetDocument();
}

ScDocShell* ScModelTestBase::getScDocShell()
{
    SfxObjectShell* pFoundShell = SfxObjectShell::GetShellFromComponent(mxComponent);
    CPPUNIT_ASSERT_MESSAGE("Failed to access document shell", pFoundShell);
    ScDocShell* pDocSh = dynamic_cast<ScDocShell*>(pFoundShell);
    CPPUNIT_ASSERT(pDocSh);
    return pDocSh;
}

ScTabViewShell* ScModelTestBase::getViewShell()
{
    ScDocShell* pDocSh = getScDocShell();
    ScTabViewShell* pTabViewShell = pDocSh->GetBestViewShell(false);
    CPPUNIT_ASSERT_MESSAGE("No ScTabViewShell", pTabViewShell);
    return pTabViewShell;
}

void ScModelTestBase::miscRowHeightsTest( TestParam const * aTestValues, unsigned int numElems)
{
    for ( unsigned int index=0; index<numElems; ++index )
    {
        const std::u16string_view sFileName = aTestValues[ index ].sTestDoc;
        const OUString sExportType =  aTestValues[ index ].sExportType;
        loadFromURL(sFileName);

        if ( !sExportType.isEmpty() )
            saveAndReload(sExportType);

        ScDocument* pDoc = getScDoc();

        for (int i=0; i<aTestValues[ index ].nRowData; ++i)
        {
            SCROW nRow = aTestValues[ index ].pData[ i].nStartRow;
            SCROW nEndRow = aTestValues[ index ].pData[ i ].nEndRow;
            SCTAB nTab = aTestValues[ index ].pData[ i ].nTab;
            int nExpectedHeight = aTestValues[ index ].pData[ i ].nExpectedHeight;
            if ( nExpectedHeight == -1 )
                nExpectedHeight = convertTwipToMm100(ScGlobal::GetStandardRowHeight());
            bool bCheckOpt = ( ( aTestValues[ index ].pData[ i ].nCheck & CHECK_OPTIMAL ) == CHECK_OPTIMAL );
            for ( ; nRow <= nEndRow; ++nRow )
            {
                SAL_INFO( "sc.qa", " checking row " << nRow << " for height " << nExpectedHeight );
                int nHeight = convertTwipToMm100(pDoc->GetRowHeight(nRow, nTab, false));
                if ( bCheckOpt )
                {
                    bool bOpt = !(pDoc->GetRowFlags( nRow, nTab ) & CRFlags::ManualSize);
                    CPPUNIT_ASSERT_EQUAL(aTestValues[ index ].pData[ i ].bOptimal, bOpt);
                }
                CPPUNIT_ASSERT_EQUAL(nExpectedHeight, nHeight);
            }
        }
    }
}

ScRange ScSimpleBootstrapFixture::insertRangeData(
    ScDocument* pDoc, const ScAddress& rPos, const std::vector<std::vector<const char*>>& rData )
{
    if (rData.empty())
        return ScRange(ScAddress::INITIALIZE_INVALID);

    ScAddress aPos = rPos;

    SCCOL nColWidth = 1;
    for (const std::vector<const char*>& rRow : rData)
        nColWidth = std::max<SCCOL>(nColWidth, rRow.size());

    ScRange aRange(aPos);
    aRange.aEnd.IncCol(nColWidth-1);
    aRange.aEnd.IncRow(rData.size()-1);

    clearRange(pDoc, aRange);

    for (const std::vector<const char*>& rRow : rData)
    {
        aPos.SetCol(rPos.Col());

        for (const char* pStr : rRow)
        {
            if (!pStr)
            {
                aPos.IncCol();
                continue;
            }

            OUString aStr(pStr, strlen(pStr), RTL_TEXTENCODING_UTF8);

            ScSetStringParam aParam; // Leave default.
            aParam.meStartListening = sc::NoListening;
            pDoc->SetString(aPos, aStr, &aParam);

            aPos.IncCol();
        }

        aPos.IncRow();
    }

    pDoc->StartAllListeners(aRange);
    printRange(pDoc, aRange, "Range data content");
    return aRange;
}

ScUndoCut* ScSimpleBootstrapFixture::cutToClip(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pClipDoc, bool bCreateUndo)
{
    ScDocument* pSrcDoc = &rDocSh.GetDocument();

    ScClipParam aClipParam(rRange, true);
    ScMarkData aMark(pSrcDoc->GetSheetLimits());
    aMark.SetMarkArea(rRange);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark, false, false);

    // Taken from ScViewFunc::CutToClip()
    ScDocumentUniquePtr pUndoDoc;
    if (bCreateUndo)
    {
        pUndoDoc.reset(new ScDocument( SCDOCMODE_UNDO ));
        pUndoDoc->InitUndoSelected( *pSrcDoc, aMark );
        // all sheets - CopyToDocument skips those that don't exist in pUndoDoc
        ScRange aCopyRange = rRange;
        aCopyRange.aStart.SetTab(0);
        aCopyRange.aEnd.SetTab(pSrcDoc->GetTableCount()-1);
        pSrcDoc->CopyToDocument( aCopyRange,
                (InsertDeleteFlags::ALL & ~InsertDeleteFlags::OBJECTS) | InsertDeleteFlags::NOCAPTIONS,
                false, *pUndoDoc );
    }

    aMark.MarkToMulti();
    pSrcDoc->DeleteSelection( InsertDeleteFlags::ALL, aMark );
    aMark.MarkToSimple();

    if (pUndoDoc)
        return new ScUndoCut( &rDocSh, rRange, rRange.aEnd, aMark, std::move(pUndoDoc) );

    return nullptr;
}

void ScSimpleBootstrapFixture::copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc)
{
    ScClipParam aClipParam(rRange, false);
    ScMarkData aMark(pSrcDoc->GetSheetLimits());
    aMark.SetMarkArea(rRange);
    pSrcDoc->CopyToClip(aClipParam, pClipDoc, &aMark, false, false);
}

void ScSimpleBootstrapFixture::pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc)
{
    ScMarkData aMark(pDestDoc->GetSheetLimits());
    aMark.SetMarkArea(rDestRange);
    pDestDoc->CopyFromClip(rDestRange, aMark, InsertDeleteFlags::ALL, nullptr, pClipDoc);
}

ScUndoPaste* ScSimpleBootstrapFixture::createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange, ScDocumentUniquePtr pUndoDoc)
{
    ScDocument& rDoc = rDocSh.GetDocument();
    ScMarkData aMarkData(rDoc.GetSheetLimits());
    aMarkData.SetMarkArea(rRange);
    std::unique_ptr<ScRefUndoData> pRefUndoData(new ScRefUndoData(&rDoc));

    return new ScUndoPaste(
        &rDocSh, rRange, aMarkData, std::move(pUndoDoc), nullptr, InsertDeleteFlags::ALL, std::move(pRefUndoData), false);
}

void ScSimpleBootstrapFixture::pasteOneCellFromClip(ScDocument* pDestDoc, const ScRange& rDestRange, ScDocument* pClipDoc, InsertDeleteFlags eFlags)
{
    ScMarkData aMark(pDestDoc->GetSheetLimits());
    aMark.SetMarkArea(rDestRange);
    sc::CopyFromClipContext aCxt(*pDestDoc, nullptr, pClipDoc, eFlags, false, false);
    aCxt.setDestRange(rDestRange.aStart.Col(), rDestRange.aStart.Row(),
            rDestRange.aEnd.Col(), rDestRange.aEnd.Row());
    aCxt.setTabRange(rDestRange.aStart.Tab(), rDestRange.aEnd.Tab());
    pDestDoc->CopyOneCellFromClip(aCxt, rDestRange.aStart.Col(), rDestRange.aStart.Row(),
            rDestRange.aEnd.Col(), rDestRange.aEnd.Row());
}

void ScSimpleBootstrapFixture::setCalcAsShown(ScDocument* pDoc, bool bCalcAsShown)
{
    ScDocOptions aOpt = pDoc->GetDocOptions();
    aOpt.SetCalcAsShown(bCalcAsShown);
    pDoc->SetDocOptions(aOpt);
}

ScDocShell* ScSimpleBootstrapFixture::findLoadedDocShellByName(std::u16string_view rName)
{
    ScDocShell* pShell = static_cast<ScDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<ScDocShell>, false));
    while (pShell)
    {
        SfxMedium* pMedium = pShell->GetMedium();
        if (pMedium)
        {
            OUString aName = pMedium->GetName();
            if (aName == rName)
                return pShell;
        }
        pShell = static_cast<ScDocShell*>(SfxObjectShell::GetNext(*pShell, checkSfxObjectShell<ScDocShell>, false));
    }
    return nullptr;
}

bool ScSimpleBootstrapFixture::insertRangeNames(
    ScDocument* pDoc, ScRangeName* pNames, const RangeNameDef* p, const RangeNameDef* pEnd)
{
    ScAddress aA1(0, 0, 0);
    for (; p != pEnd; ++p)
    {
        ScRangeData* pNew = new ScRangeData(
            *pDoc,
            OUString::createFromAscii(p->mpName),
            OUString::createFromAscii(p->mpExpr),
            aA1, ScRangeData::Type::Name,
            formula::FormulaGrammar::GRAM_ENGLISH);
        pNew->SetIndex(p->mnIndex);
        bool bSuccess = pNames->insert(pNew);
        if (!bSuccess)
        {
            cerr << "Insertion failed." << endl;
            return false;
        }
    }

    return true;
}

OUString ScSimpleBootstrapFixture::getRangeByName(ScDocument* pDoc, const OUString& aRangeName)
{
    ScRangeData* pName = pDoc->GetRangeName()->findByUpperName(aRangeName.toAsciiUpperCase());
    CPPUNIT_ASSERT(pName);
    return pName->GetSymbol(pDoc->GetGrammar());
}

#if CALC_DEBUG_OUTPUT != 0
void ScSimpleBootstrapFixture::printFormula(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab, const char* pCaption)
{
    if (pCaption != nullptr)
        cout << pCaption << ", ";
    cout << nCol << "/" << nRow << ": " << pDoc->GetFormula(nCol, nRow, nTab);
    cout << endl;
}
#else
// Avoid unused parameter warning
void ScSimpleBootstrapFixture::printFormula(ScDocument*, SCCOL, SCROW, SCTAB, const char*) {}
#endif

void ScSimpleBootstrapFixture::printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption,
                const bool printFormula)
{
    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();
    svl::GridPrinter printer(nRow2 - nRow1 + 1, nCol2 - nCol1 + 1, CALC_DEBUG_OUTPUT != 0);
    for (SCROW nRow = nRow1; nRow <= nRow2; ++nRow)
    {
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            ScAddress aPos(nCol, nRow, rRange.aStart.Tab());
            ScRefCellValue aCell(*pDoc, aPos);
            OUString aVal = printFormula ? pDoc->GetFormula(nCol, nRow, rRange.aStart.Tab())
                                         : ScCellFormat::GetOutputString(*pDoc, aPos, aCell);
            printer.set(nRow - nRow1, nCol - nCol1, aVal);
        }
    }
    printer.print(pCaption);
}

void ScSimpleBootstrapFixture::printRange(ScDocument* pDoc, const ScRange& rRange, const OString& rCaption,
                const bool printFormula)
{
    printRange(pDoc, rRange, rCaption.getStr(), printFormula);
}

void ScSimpleBootstrapFixture::clearRange(ScDocument* pDoc, const ScRange& rRange)
{
    ScMarkData aMarkData(pDoc->GetSheetLimits());
    aMarkData.SetMarkArea(rRange);
    pDoc->DeleteArea(
        rRange.aStart.Col(), rRange.aStart.Row(),
        rRange.aEnd.Col(), rRange.aEnd.Row(), aMarkData, InsertDeleteFlags::CONTENTS);
}

void ScSimpleBootstrapFixture::clearSheet(ScDocument* pDoc, SCTAB nTab)
{
    ScRange aRange(0,0,nTab,pDoc->MaxCol(),pDoc->MaxRow(),nTab);
    clearRange(pDoc, aRange);
}

bool ScSimpleBootstrapFixture::checkFormulaPosition(ScDocument& rDoc, const ScAddress& rPos)
{
    OUString aStr(rPos.Format(ScRefFlags::VALID));
    const ScFormulaCell* pFC = rDoc.GetFormulaCell(rPos);
    if (!pFC)
    {
        cerr << "Formula cell expected at " << aStr << " but not found." << endl;
        return false;
    }

    if (pFC->aPos != rPos)
    {
        OUString aStr2(pFC->aPos.Format(ScRefFlags::VALID));
        cerr << "Formula cell at " << aStr << " has incorrect position of " << aStr2 << endl;
        return false;
    }

    return true;
}

bool ScSimpleBootstrapFixture::checkFormulaPositions(
    ScDocument& rDoc, SCTAB nTab, SCCOL nCol, const SCROW* pRows, size_t nRowCount)
{
    ScAddress aPos(nCol, 0, nTab);
    for (size_t i = 0; i < nRowCount; ++i)
    {
        SCROW nRow = pRows[i];
        aPos.SetRow(nRow);

        if (!checkFormulaPosition(rDoc, aPos))
        {
            OUString aStr(aPos.Format(ScRefFlags::VALID));
            cerr << "Formula cell position failed at " << aStr << "." << endl;
            return false;
        }
    }
    return true;
}

std::unique_ptr<ScTokenArray> ScSimpleBootstrapFixture::compileFormula(
    ScDocument* pDoc, const OUString& rFormula,
    formula::FormulaGrammar::Grammar eGram )
{
    ScAddress aPos(0,0,0);
    ScCompiler aComp(*pDoc, aPos, eGram);
    return aComp.CompileString(rFormula);
}

void ScSimpleBootstrapFixture::clearFormulaCellChangedFlag( ScDocument& rDoc, const ScRange& rRange )
{
    const ScAddress& s = rRange.aStart;
    const ScAddress& e = rRange.aEnd;
    for (SCTAB nTab = s.Tab(); nTab <= e.Tab(); ++nTab)
    {
        for (SCCOL nCol = s.Col(); nCol <= e.Col(); ++nCol)
        {
            for (SCROW nRow = s.Row(); nRow <= e.Row(); ++nRow)
            {
                ScAddress aPos(nCol, nRow, nTab);
                ScFormulaCell* pFC = rDoc.GetFormulaCell(aPos);
                if (pFC)
                    pFC->SetChanged(false);
            }
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
