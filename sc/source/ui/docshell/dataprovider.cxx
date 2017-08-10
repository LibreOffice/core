/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include "officecfg/Office/Calc.hxx"
#include <stringutil.hxx>
#include <rtl/strbuf.hxx>

#if defined(_WIN32)
#if !defined __ORCUS_STATIC_LIB // avoid -Werror,-Wunused-macros
#define __ORCUS_STATIC_LIB
#endif
#endif
#include <orcus/csv_parser.hpp>

using namespace com::sun::star;

namespace sc {

namespace {

std::unique_ptr<SvStream> FetchStreamFromURL(const OUString& rURL, OStringBuffer& rBuffer)
{
    uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );

    uno::Reference< io::XInputStream > xStream;
    xStream = xFileAccess->openFileRead( rURL );

    const sal_Int32 BUF_LEN = 8000;
    uno::Sequence< sal_Int8 > buffer( BUF_LEN );

    sal_Int32 nRead = 0;
    while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
    {
        rBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
    }

    if ( nRead > 0 )
    {
        rBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
    }

    xStream->closeInput();

    SvStream* pStream = new SvMemoryStream(const_cast<char*>(rBuffer.getStr()), rBuffer.getLength(), StreamMode::READ);
    return std::unique_ptr<SvStream>(pStream);
}

}

ExternalDataSource::ExternalDataSource(const OUString& rURL,
        const OUString& rProvider, ScDocument* pDoc):
    maURL(rURL),
    maProvider(rProvider),
    mpDoc(pDoc)
{
}

ExternalDataSource::~ExternalDataSource()
{
}

void ExternalDataSource::setID(const OUString& rID)
{
    maID = rID;
}

const OUString& ExternalDataSource::getURL() const
{
    return maURL;
}

const OUString& ExternalDataSource::getProvider() const
{
    return maProvider;
}

const OUString& ExternalDataSource::getID() const
{
    return maID;
}

OUString ExternalDataSource::getDBName() const
{
    if (mpDBDataManager)
    {
        ScDBData* pDBData = mpDBDataManager->getDBData();
        if (pDBData)
            return pDBData->GetName();
    }
    return OUString();
}

void ExternalDataSource::setDBData(ScDBData* pDBData)
{
    if (!mpDBDataManager)
    {
        mpDBDataManager.reset(new ScDBDataManager(pDBData, false, mpDoc));
    }
    else
    {
        mpDBDataManager->SetDatabase(pDBData);
    }
}

double ExternalDataSource::getUpdateFrequency() const
{
    return mnUpdateFrequency;
}

void ExternalDataSource::refresh(ScDocument* pDoc)
{
    // no DB data available
    if (!mpDBDataManager)
        return;

    // if no data provider exists, try to create one
    if (!mpDataProvider)
        mpDataProvider = DataProviderFactory::getDataProvider(pDoc, maProvider, maURL, maID, mpDBDataManager.get());

    // if we still have not been able to create one, we can not refresh the data
    if (!mpDataProvider)
        return;

    mpDataProvider->Import();
}

ExternalDataMapper::ExternalDataMapper(ScDocument* /*pDoc*/)
    //mpDoc(pDoc)
{
}

ExternalDataMapper::~ExternalDataMapper()
{
}

void ExternalDataMapper::insertDataSource(const sc::ExternalDataSource& rSource)
{
    maDataSources.push_back(rSource);
}

const std::vector<sc::ExternalDataSource>& ExternalDataMapper::getDataSources() const
{
    return maDataSources;
}

std::vector<sc::ExternalDataSource>& ExternalDataMapper::getDataSources()
{
    return maDataSources;
}

DataProvider::~DataProvider()
{
}

Cell::Cell() : mfValue(0.0), mbValue(true) {}

Cell::Cell(const Cell& r) : mbValue(r.mbValue)
{
    if (r.mbValue)
        mfValue = r.mfValue;
    else
    {
        maStr.Pos = r.maStr.Pos;
        maStr.Size = r.maStr.Size;
    }
}

class CSVHandler
{
    Line& mrLine;
    size_t mnColCount;
    size_t mnCols;
    const char* mpLineHead;

public:
    CSVHandler( Line& rLine, size_t nColCount ) :
        mrLine(rLine), mnColCount(nColCount), mnCols(0), mpLineHead(rLine.maLine.getStr()) {}

    static void begin_parse() {}
    static void end_parse() {}
    static void begin_row() {}
    static void end_row() {}

    void cell(const char* p, size_t n)
    {
        if (mnCols >= mnColCount)
            return;

        Cell aCell;
        if (ScStringUtil::parseSimpleNumber(p, n, '.', ',', aCell.mfValue))
        {
            aCell.mbValue = true;
        }
        else
        {
            aCell.mbValue = false;
            aCell.maStr.Pos = std::distance(mpLineHead, p);
            aCell.maStr.Size = n;
        }
        mrLine.maCells.push_back(aCell);

        ++mnCols;
    }
};

CSVFetchThread::CSVFetchThread(ScDocument& rDoc, const OUString& mrURL, Idle* pIdle):
        Thread("CSV Fetch Thread"),
        mrDocument(rDoc),
        maURL (mrURL),
        mbTerminate(false),
        mpIdle(pIdle)
{
    maConfig.delimiters.push_back(',');
    maConfig.text_qualifier = '"';
}

CSVFetchThread::~CSVFetchThread()
{
}

bool CSVFetchThread::IsRequestedTerminate()
{
    osl::MutexGuard aGuard(maMtxTerminate);
    return mbTerminate;
}

void CSVFetchThread::RequestTerminate()
{
    osl::MutexGuard aGuard(maMtxTerminate);
    mbTerminate = true;
}

void CSVFetchThread::EndThread()
{
    RequestTerminate();
}

void CSVFetchThread::execute()
{
    OStringBuffer aBuffer(64000);
    std::unique_ptr<SvStream> pStream = FetchStreamFromURL(maURL, aBuffer);
    SCROW nCurRow = 0;
    SCCOL nCol = 0;
    while (pStream->good())
    {
        if (mbTerminate)
            break;

        Line aLine;
        aLine.maCells.clear();
        pStream->ReadLine(aLine.maLine);
        CSVHandler aHdl(aLine, MAXCOL);
        orcus::csv_parser<CSVHandler> parser(aLine.maLine.getStr(), aLine.maLine.getLength(), aHdl, maConfig);
        parser.parse();

        if (aLine.maCells.empty())
        {
            break;
        }

        nCol = 0;
        const char* pLineHead = aLine.maLine.getStr();
        for (auto& rCell : aLine.maCells)
        {
            if (rCell.mbValue)
            {
                mrDocument.SetValue(ScAddress(nCol, nCurRow, 0 /* Tab */), rCell.mfValue);
            }
            else
            {
                mrDocument.SetString(nCol, nCurRow, 0 /* Tab */, OUString(pLineHead+rCell.maStr.Pos, rCell.maStr.Size, RTL_TEXTENCODING_UTF8));
            }
            ++nCol;
        }
        nCurRow++;
    }
    SolarMutexGuard aGuard;
    mpIdle->Start();
}

CSVDataProvider::CSVDataProvider(ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pBDDataManager):
    maURL(rURL),
    mpDocument(pDoc),
    mpDBDataManager(pBDDataManager),
    mpLines(nullptr),
    mnLineCount(0),
    maIdle("CSVDataProvider CopyHandler")
{
    maIdle.SetInvokeHandler(LINK(this, CSVDataProvider, ImportFinishedHdl));
}

CSVDataProvider::~CSVDataProvider()
{
    if (mxCSVFetchThread.is())
    {
        mxCSVFetchThread->join();
    }
}

void CSVDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, (SCTAB)0);
    mxCSVFetchThread = new CSVFetchThread(*mpDoc, maURL, &maIdle);
    mxCSVFetchThread->launch();
}

IMPL_LINK_NOARG(CSVDataProvider, ImportFinishedHdl, Timer*, void)
{
    mpDBDataManager->WriteToDoc(*mpDoc, mpDBDataManager->getDBData());
    mxCSVFetchThread.clear();
    mpDoc.reset();
    Refresh();
}

void CSVDataProvider::Refresh()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDocument->GetDocumentShell());
    pDocShell->SetDocumentModified();
}

void ScDBDataManager::WriteToDoc(ScDocument& rDoc, ScDBData* pDBData)
{
    bool bShrunk = false;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;
    SCROW nEndRow = MAXROW;
    rDoc.ShrinkToUsedDataArea(bShrunk, 0, nStartCol, nStartRow, nEndCol, nEndRow, false, true, true);
    ScRange aClipRange(nStartCol, nStartRow, 0, nEndCol, nEndRow, 0);
    rDoc.SetClipArea(aClipRange);

    ScRange aDestRange;
    pDBData->GetArea(aDestRange);
    SCCOL nColSize = std::min<SCCOL>(aDestRange.aEnd.Col() - aDestRange.aStart.Col(), nEndCol);
    aDestRange.aEnd.SetCol(aDestRange.aStart.Col() + nColSize);

    SCROW nRowSize = std::min<SCCOL>(aDestRange.aEnd.Row() - aDestRange.aStart.Row(), nEndRow);
    aDestRange.aEnd.SetRow(aDestRange.aStart.Row() + nRowSize);

    ScMarkData aMark;
    aMark.SelectTable(0, true);
    mpDoc->CopyFromClip(aDestRange, aMark, InsertDeleteFlags::CONTENTS, nullptr, &rDoc);
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDoc->GetDocumentShell());
    pDocShell->PostPaint(aDestRange, PaintPartFlags::All);
}

ScDBDataManager::ScDBDataManager(ScDBData* pDBData,  bool /*bAllowResize*/, ScDocument* pDoc):
    mpDBData(pDBData),
    //mbAllowResize(bAllowResize),
    mpDoc(pDoc)
{
}

ScDBDataManager::~ScDBDataManager()
{
}

void ScDBDataManager::SetDatabase(ScDBData* pDbData)
{
    mpDBData = pDbData;
}

ScDBData* ScDBDataManager::getDBData()
{
    return mpDBData;
}

bool DataProviderFactory::isInternalDataProvider(const OUString& rProvider)
{
    return rProvider.startsWith("org.libreoffice.calc");
}

std::shared_ptr<DataProvider> DataProviderFactory::getDataProvider(ScDocument* pDoc, const OUString& rProvider, const OUString& rURL, const OUString& /*rID*/, ScDBDataManager* pManager)
{
    bool bInternal = DataProviderFactory::isInternalDataProvider(rProvider);
    if (bInternal)
    {
        if (rProvider == "org.libreoffice.calc.csv")
            return std::shared_ptr<DataProvider>(new CSVDataProvider(pDoc, rURL, pManager));
    }
    else
    {
        SAL_WARN("sc", "no external data provider supported yet");
        return std::shared_ptr<DataProvider>();
    }

    return std::shared_ptr<DataProvider>();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
