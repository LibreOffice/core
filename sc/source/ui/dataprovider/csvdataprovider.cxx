/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include "datatransformation.hxx"
#include <stringutil.hxx>

#if defined(_WIN32)
#if !defined __ORCUS_STATIC_LIB // avoid -Werror,-Wunused-macros
#define __ORCUS_STATIC_LIB
#endif
#endif
#include <orcus/csv_parser.hpp>

namespace {

class CSVHandler
{
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;

public:
    CSVHandler(ScDocument* pDoc) :
        mpDoc(pDoc), mnCol(0), mnRow(0)
    {
    }

    static void begin_parse() {}
    static void end_parse() {}
    static void begin_row() {}
    void end_row()
    {
        ++mnRow;
        mnCol = 0;
    }

    void cell(const char* p, size_t n)
    {
        if (mnCol > MAXCOL)
            return;

        double mfValue = 0.0;
        if (ScStringUtil::parseSimpleNumber(p, n, '.', ',', mfValue))
        {
            mpDoc->SetValue(mnCol, mnRow, 0, mfValue);
        }
        else
        {
            OString aStr(p, n);
            mpDoc->SetString(mnCol, mnRow, 0, OStringToOUString(aStr, RTL_TEXTENCODING_UTF8));
        }

        ++mnCol;
    }
};

}

namespace sc {

CSVFetchThread::CSVFetchThread(ScDocument& rDoc, const OUString& mrURL, Idle* pIdle,
        const std::vector<std::shared_ptr<sc::DataTransformation>>& rDataTransformations):
        Thread("CSV Fetch Thread"),
        mrDocument(rDoc),
        maURL (mrURL),
        mbTerminate(false),
        maDataTransformations(rDataTransformations),
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
    if (mbTerminate)
        return;

    CSVHandler aHdl(&mrDocument);
    orcus::csv_parser<CSVHandler> parser(aBuffer.getStr(), aBuffer.getLength(), aHdl, maConfig);
    parser.parse();

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
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
        SolarMutexReleaser aReleaser;
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
    mxCSVFetchThread = new CSVFetchThread(*mpDoc, maURL, &maIdle, mpDBDataManager->getDataTransformation());
    mxCSVFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxCSVFetchThread->join();
    }
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
