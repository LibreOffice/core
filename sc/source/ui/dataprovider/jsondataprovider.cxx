/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "jsondataprovider.hxx"
#include <datatransformation.hxx>
#include <datatransformation.hxx>
#include <stringutil.hxx>

#include <orcus/json_parser.hpp>

namespace {

class JSONHandler
{
    ScDocument* mpDoc;
    SCCOL mnCol;
    SCROW mnRow;

public:
    JSONHandler(ScDocument* pDoc) :
        mpDoc(pDoc), mnCol(0), mnRow(0)
    {
    }

    // Called when the parsing begins.
    static void begin_parse() {}

    // Called when the parsing ends.
    static void end_parse() {}

    // Called when the opening brace of an array is encountered.
    static void begin_array() {}

    // Called when the closing brace of an array is encountered.
    void end_array() {    }

    // Called when the opening curly brace of an object is encountered.
    void begin_object() {}

    // Called when a key value string of an object is encountered.
    void object_key(const char* /*p*/, size_t /*n*/, bool /*transient*/) {
        //Use this block to set column headers
        /*
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
*/
    }
    // Called when the closing curly brace of an object is encountered.
    void end_object() {
         ++mnRow;
        mnCol = 0;
    }

    // Called when a boolean ‘true’ keyword is encountered.
    static void boolean_true() {}

    // Called when a boolean ‘false’ keyword is encountered.
    static void boolean_false() {}

    // Called when a ‘null’ keyword is encountered.
    static void null() {}

    // Called when a string value is encountered.
    void string(const char *p, size_t n, bool /*transient*/) {
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

    // Called when a numeric value is encountered.
    void number(double /*val*/) {}
};

}

namespace sc {

JSONFetchThread::JSONFetchThread(ScDocument& rDoc, const OUString& mrURL, std::function<void()> aImportFinishedHdl,
        const std::vector<std::shared_ptr<sc::DataTransformation>>& rDataTransformations):
        Thread("JSON Fetch Thread"),
        mrDocument(rDoc),
        maURL (mrURL),
        mbTerminate(false),
        maDataTransformations(rDataTransformations),
        maImportFinishedHdl(aImportFinishedHdl)
{

}

JSONFetchThread::~JSONFetchThread()
{
}

bool JSONFetchThread::IsRequestedTerminate()
{
    osl::MutexGuard aGuard(maMtxTerminate);
    return mbTerminate;
}

void JSONFetchThread::RequestTerminate()
{
    osl::MutexGuard aGuard(maMtxTerminate);
    mbTerminate = true;
}

void JSONFetchThread::EndThread()
{
    RequestTerminate();
}

void JSONFetchThread::execute()
{
    OStringBuffer aBuffer(64000);
    std::unique_ptr<SvStream> pStream = DataProvider::FetchStreamFromURL(maURL, aBuffer);
    if (mbTerminate)
        return;

    JSONHandler aHdl(&mrDocument);
    orcus::json_parser<JSONHandler> parser(aBuffer.getStr(), aBuffer.getLength(), aHdl);
    parser.parse();

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    maImportFinishedHdl();
}

JSONDataProvider::JSONDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource):
    DataProvider(rDataSource),
    mpDocument(pDoc)
{
}

JSONDataProvider::~JSONDataProvider()
{
    if (mxJSONFetchThread.is())
    {
        SolarMutexReleaser aReleaser;
        mxJSONFetchThread->join();
    }
}

void JSONDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxJSONFetchThread = new JSONFetchThread(*mpDoc, mrDataSource.getURL(), std::bind(&JSONDataProvider::ImportFinished, this), mrDataSource.getDataTransformation());
    mxJSONFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxJSONFetchThread->join();
    }
}

void JSONDataProvider::ImportFinished()
{
    mrDataSource.getDBManager()->WriteToDoc(*mpDoc);
    mpDoc.reset();
    Refresh();
}

void JSONDataProvider::Refresh()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDocument->GetDocumentShell());
    if (pDocShell)
        pDocShell->SetDocumentModified();
}

const OUString& JSONDataProvider::GetURL() const
{
    return mrDataSource.getURL();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
