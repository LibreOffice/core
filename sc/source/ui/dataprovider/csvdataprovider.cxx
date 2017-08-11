/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <stringutil.hxx>

#if defined(_WIN32)
#if !defined __ORCUS_STATIC_LIB // avoid -Werror,-Wunused-macros
#define __ORCUS_STATIC_LIB
#endif
#endif
#include <orcus/csv_parser.hpp>

namespace {

struct Cell
{
    struct Str
    {
        size_t Pos;
        size_t Size;
    };

    union
    {
        Str maStr;
        double mfValue;
    };

    bool mbValue;

    Cell();
    Cell( const Cell& r );
};

struct Line
{
    OString maLine;
    std::vector<Cell> maCells;
};

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

}

namespace sc {

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
    std::unique_ptr<SvStream> pStream = DataProvider::FetchStreamFromURL(maURL, aBuffer);
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
    mpDBDataManager->WriteToDoc(*mpDoc);
    mxCSVFetchThread.clear();
    mpDoc.reset();
    Refresh();
}

void CSVDataProvider::Refresh()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDocument->GetDocumentShell());
    pDocShell->SetDocumentModified();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
