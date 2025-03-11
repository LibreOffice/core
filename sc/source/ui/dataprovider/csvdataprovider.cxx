/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <datatransformation.hxx>
#include <datamapper.hxx>
#include <exception>
#include <stringutil.hxx>

#include <tools/stream.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
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

    void cell(std::string_view s, bool /*transient*/)
    {
        if (mnCol > mpDoc->MaxCol())
            return;

        double mfValue = 0.0;
        if (ScStringUtil::parseSimpleNumber(s.data(), s.size(), '.', ',', mfValue))
        {
            mpDoc->SetValue(mnCol, mnRow, 0, mfValue);
        }
        else
        {
            mpDoc->SetString(mnCol, mnRow, 0, OStringToOUString(s, RTL_TEXTENCODING_UTF8));
        }

        ++mnCol;
    }
};

}

namespace sc {
CSVFetchThread::CSVFetchThread(
    ScDocument& rDoc, OUString aURL, std::function<void()> aImportFinishedHdl,
    std::vector<std::shared_ptr<sc::DataTransformation>>&& rDataTransformations)
    : Thread("CSV Fetch Thread")
    , mrDocument(rDoc)
    , maURL(std::move(aURL))
    , mbTerminate(false)
    , maDataTransformations(std::move(rDataTransformations))
    , maImportFinishedHdl(std::move(aImportFinishedHdl))
    , mbIsParseError(false)
{
    maConfig.delimiters.push_back(',');
    maConfig.text_qualifier = '"';
}

CSVFetchThread::~CSVFetchThread()
{
}

bool CSVFetchThread::IsRequestedTerminate()
{
    return mbTerminate.load();
}

void CSVFetchThread::RequestTerminate()
{
    mbTerminate.store(true);
}

void CSVFetchThread::EndThread()
{
    RequestTerminate();
}

void CSVFetchThread::execute()
{
    OStringBuffer aBuffer(64000);
    DataProvider::FetchStreamFromURL(maURL, aBuffer);
    if (mbTerminate)
        return;

    CSVHandler aHdl(&mrDocument);
    orcus::csv_parser<CSVHandler> parser(aBuffer, aHdl, maConfig);

    try
    {
        parser.parse();
    }
    catch(const orcus::parse_error&)
    {
        mbIsParseError = true;
        mpLastException = std::current_exception();
        RequestTerminate();
        return;
    }

    for (const auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    maImportFinishedHdl();
}

CSVDataProvider::CSVDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource):
    DataProvider(rDataSource),
    mpDocument(pDoc)
{
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
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxCSVFetchThread = new CSVFetchThread(*mpDoc, mrDataSource.getURL(), std::bind(&CSVDataProvider::ImportFinished, this), std::vector(mrDataSource.getDataTransformation()));
    mxCSVFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxCSVFetchThread->join();

        // tdf#165658 An exception may have happened during the parsing of the file.
        // Since parsing happens in a separate thread, here we need to check if
        // something wrong happened and then rethrow the exception
        if (mxCSVFetchThread->IsParseError())
        {
            std::rethrow_exception(mxCSVFetchThread->GetLastException());
        }
    }
}

void CSVDataProvider::ImportFinished()
{
    mrDataSource.getDBManager()->WriteToDoc(*mpDoc);
    mpDoc.reset();
    Refresh();
}

void CSVDataProvider::Refresh()
{
    ScDocShell* pDocShell = mpDocument->GetDocumentShell();
    if (pDocShell)
        pDocShell->SetDocumentModified();
}

const OUString& CSVDataProvider::GetURL() const
{
    return mrDataSource.getURL();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
