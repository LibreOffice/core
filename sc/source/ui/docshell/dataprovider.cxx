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

ExternalDataMapper::ExternalDataMapper(ScDocShell* pDocShell, const OUString& rURL, const OUString& rName, SCTAB nTab,
    SCCOL nCol1,SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool& bSuccess):
    maRange (ScRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab)),
    mpDocShell(pDocShell),
    mpDataProvider (new CSVDataProvider(mpDocShell, rURL, maRange)),
    mpDBCollection (pDocShell->GetDocument().GetDBCollection())
{
    bSuccess = true;
    ScDBCollection::NamedDBs& rNamedDBS = mpDBCollection->getNamedDBs();
    if(!rNamedDBS.insert (new ScDBData (rName, nTab, nCol1, nRow1, nCol2, nRow2)))
        bSuccess = false;
}

ExternalDataMapper::~ExternalDataMapper()
{
}

void ExternalDataMapper::StartImport()
{
    mpDataProvider->StartImport();
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

CSVFetchThread::CSVFetchThread(ScDocument& rDoc, const OUString& mrURL, size_t nColCount):
        Thread("ReaderThread"),
        mpStream(nullptr),
        mrDocument(rDoc),
        maURL (mrURL),
        mnColCount(nColCount),
        mbTerminate(false)
{
    maConfig.delimiters.push_back(',');
    maConfig.text_qualifier = '"';
    mrDocument.InsertTab(0, "blah");
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
    mpStream = FetchStreamFromURL(maURL, aBuffer);
    if (mpStream->good())
    {
        LinesType* pLines = new LinesType(10);
        SCROW nCurRow = 0;
        for (Line & rLine : *pLines)
        {
            rLine.maCells.clear();
            mpStream->ReadLine(rLine.maLine);
            CSVHandler aHdl(rLine, mnColCount);
            orcus::csv_parser<CSVHandler> parser(rLine.maLine.getStr(), rLine.maLine.getLength(), aHdl, maConfig);
            parser.parse();

            if (rLine.maCells.empty())
            {
                return;
            }

            SCCOL nCol = 0;
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

CSVDataProvider::CSVDataProvider(ScDocShell* pDocShell, const OUString& rURL, const ScRange& rRange):
    maURL(rURL),
    mrRange(rRange),
    mpDocShell(pDocShell),
    mpDocument(&pDocShell->GetDocument()),
    mpLines(nullptr),
    mnLineCount(0),
    mbImportUnderway(false)
{
}

CSVDataProvider::~CSVDataProvider()
{
}

void CSVDataProvider::StartImport()
{
    if (mbImportUnderway)
        return;

    if (!mxCSVFetchThread.is())
    {
        ScDocument aDoc;
        mxCSVFetchThread = new CSVFetchThread(aDoc, maURL, mrRange.aEnd.Col() - mrRange.aStart.Col() + 1);
        mxCSVFetchThread->launch();
        if (mxCSVFetchThread.is())
        {
            mxCSVFetchThread->EndThread();
            mxCSVFetchThread->join();
        }

        WriteToDoc(aDoc);
    }

    Refresh();
}

void CSVDataProvider::Refresh()
{
    mpDocShell->DoHardRecalc();
    mpDocShell->SetDocumentModified();
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

void CSVDataProvider::WriteToDoc(ScDocument& rDoc)
{
    double* pfValue;
    for (int nRow = mrRange.aStart.Row(); nRow < mrRange.aEnd.Row(); ++nRow)
    {
        for (int nCol = mrRange.aStart.Col(); nCol < mrRange.aEnd.Col(); ++nCol)
        {
            ScAddress aAddr = ScAddress(nCol, nRow, mrRange.aStart.Tab());
            pfValue = rDoc.GetValueCell(aAddr);

            if (pfValue == nullptr)
            {
                OUString aString = rDoc.GetString(nCol, nRow, mrRange.aStart.Tab());
                mpDocument->SetString(nCol, nRow, mrRange.aStart.Tab(), aString);
            }
            else
            {
                mpDocument->SetValue(nCol, nRow, mrRange.aStart.Tab(), *pfValue);
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
