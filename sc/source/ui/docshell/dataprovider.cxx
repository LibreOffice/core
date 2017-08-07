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
        const OUString& rProvider):
    maURL(rURL),
    maProvider(rProvider),
    mnUpdateFrequency(0)
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
        mpDBDataManager.reset(new ScDBDataManager(pDBData, false));
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

CSVFetchThread::CSVFetchThread(ScDocument& rDoc, const OUString& mrURL):
        Thread("CSV Fetch Thread"),
        mrDocument(rDoc),
        maURL (mrURL),
        mbTerminate(false)
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
    if (pStream->good())
    {
        LinesType aLines(10);
        SCROW nCurRow = 0;
        SCCOL nCol = 0;
        for (Line & rLine : aLines)
        {
            if (mbTerminate)
                return;

            rLine.maCells.clear();
            pStream->ReadLine(rLine.maLine);
            CSVHandler aHdl(rLine, MAXCOL);
            orcus::csv_parser<CSVHandler> parser(rLine.maLine.getStr(), rLine.maLine.getLength(), aHdl, maConfig);
            parser.parse();

            if (rLine.maCells.empty())
            {
                return;
            }

            nCol = 0;
            const char* pLineHead = rLine.maLine.getStr();
            for (auto& rCell : rLine.maCells)
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
    }
}

osl::Mutex& CSVFetchThread::GetLinesMutex()
{
    return maMtxLines;
}

bool CSVFetchThread::HasNewLines()
{
    return !maPendingLines.empty();
}

void CSVFetchThread::WaitForNewLines()
{
    maCondConsume.wait();
    maCondConsume.reset();
}

LinesType* CSVFetchThread::GetNewLines()
{
    LinesType* pLines = maPendingLines.front();
    maPendingLines.pop();
    return pLines;
}

void CSVFetchThread::ResumeFetchStream()
{
    maCondReadStream.set();
}

CSVDataProvider::CSVDataProvider(ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pBDDataManager):
    maURL(rURL),
    mpDocument(pDoc),
    mpDBDataManager(pBDDataManager),
    mpLines(nullptr),
    mnLineCount(0)
{
}

CSVDataProvider::~CSVDataProvider()
{
}

void CSVDataProvider::Import()
{
    ScDocument aDoc(SCDOCMODE_CLIP);
    aDoc.ResetClip(mpDocument, (SCTAB)0);
    mxCSVFetchThread = new CSVFetchThread(aDoc, maURL);
    mxCSVFetchThread->launch();
    if (mxCSVFetchThread.is())
    {
        mxCSVFetchThread->join();
    }

    WriteToDoc(aDoc, mpDBDataManager->getDBData());

    Refresh();
}

void CSVDataProvider::Refresh()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDocument->GetDocumentShell());
    pDocShell->DoHardRecalc();
    pDocShell->SetDocumentModified();
}

Line CSVDataProvider::GetLine()
{
    if (!mpLines || mnLineCount >= mpLines->size())
    {
        if (mxCSVFetchThread->IsRequestedTerminate())
            return Line();

        osl::ResettableMutexGuard aGuard(mxCSVFetchThread->GetLinesMutex());
        while (!mxCSVFetchThread->HasNewLines() && !mxCSVFetchThread->IsRequestedTerminate())
        {
            aGuard.clear();
            mxCSVFetchThread->WaitForNewLines();
            aGuard.reset();
        }

        mpLines = mxCSVFetchThread->GetNewLines();
        mxCSVFetchThread->ResumeFetchStream();
    }

    return mpLines->at(mnLineCount++);
}

// TODO: why don't we use existing copy functionality
void CSVDataProvider::WriteToDoc(ScDocument& rDoc, ScDBData* pDBData)
{
    bool bShrunk = false;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;
    SCROW nEndRow = MAXROW;
    rDoc.ShrinkToUsedDataArea(bShrunk, 0, nStartCol, nStartRow, nEndCol, nEndRow, false, true, true);

    ScRange aDestRange;
    pDBData->GetArea(aDestRange);
    double* pfValue;
    for (int nRow = nStartRow; nRow < nEndRow; ++nRow)
    {
        for (int nCol = nStartCol; nCol < nEndCol; ++nCol)
        {
            ScAddress aAddr = ScAddress(nCol, nRow, 0);
            pfValue = rDoc.GetValueCell(aAddr);

            if (pfValue == nullptr)
            {
                OUString aString = rDoc.GetString(nCol, nRow, 0);
                mpDocument->SetString(aDestRange.aStart.Col() + nCol, aDestRange.aStart.Row() + nRow, aDestRange.aStart.Tab(), aString);
            }
            else
            {
                mpDocument->SetValue(aDestRange.aStart.Col() + nCol, aDestRange.aStart.Row() + nRow, aDestRange.aStart.Tab(), *pfValue);
            }
        }
    }
}

ScDBDataManager::ScDBDataManager(ScDBData* pDBData,  bool /*bAllowResize*/):
    mpDBData(pDBData)
    //mbAllowResize(bAllowResize)
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
