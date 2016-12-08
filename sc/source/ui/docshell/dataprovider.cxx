/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <dataprovider.hxx>
#include <config_orcus.h>
#include "officecfg/Office/Calc.hxx"
#include <stringutil.hxx>

#if ENABLE_ORCUS
#if defined(_WIN32)
#define __ORCUS_STATIC_LIB
#endif
#include <orcus/csv_parser.hpp>
#endif

namespace sc {

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

#if ENABLE_ORCUS

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

#endif

CSVFetchThread::CSVFetchThread(SvStream *pData, size_t nColCount):
        Thread("ReaderThread"),
        mpStream(pData),
        mnColCount(nColCount),
        mbTerminate(false)
    {
#if ENABLE_ORCUS
        maConfig.delimiters.push_back(',');
        maConfig.text_qualifier = '"';
#endif
    }

CSVFetchThread::~CSVFetchThread()
{
    delete mpStream;
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
        LinesType* pLines = new LinesType(10);

        // Read & store new lines from stream.
        for (Line & rLine : *pLines)
        {
            rLine.maCells.clear();
            mpStream->ReadLine(rLine.maLine);
#if ENABLE_ORCUS
            CSVHandler aHdl(rLine, mnColCount);
            orcus::csv_parser<CSVHandler> parser(rLine.maLine.getStr(), rLine.maLine.getLength(), aHdl, maConfig);
            parser.parse();
#endif
        }

        if (!mpStream->good())
            RequestTerminate();
}

void CSVFetchThread::EmptyLineQueue( std::queue<LinesType*>& rQueue)
{
    while (!rQueue.empty())
    {
        delete rQueue.front();
        rQueue.pop();
    }
}

CSVDataProvider::CSVDataProvider(const OUString& rURL, const ScRange& rRange):
    maURL(rURL),
    mnRefreshRate(100),
    mrRange(rRange),
    mbImportUnderway(false)
{
}

CSVDataProvider::~CSVDataProvider()
{
    if(mbImportUnderway)
        StopImport();
}

void CSVDataProvider::StartImport()
{
    if (mbImportUnderway)
    return;

    if (!mxCSVFetchThread.is())
    {
        SvStream *pStream = nullptr;
        pStream = new SvFileStream(maURL, StreamMode::READ);
        mxCSVFetchThread = new CSVFetchThread(pStream, mrRange.aEnd.Col() - mrRange.aStart.Col() + 1);
        mxCSVFetchThread->launch();
    }
    mbImportUnderway = true;

    maImportTimer.Start();
}

void CSVDataProvider::StopImport()
{
    if (!mbImportUnderway)
        return;

    mbImportUnderway = false;
    Refresh();
    maImportTimer.Stop();
}

void CSVDataProvider::Refresh()
{

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
