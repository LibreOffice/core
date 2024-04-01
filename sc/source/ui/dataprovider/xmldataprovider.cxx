/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmldataprovider.hxx"
#include <datatransformation.hxx>
#include <salhelper/thread.hxx>
#include <filter.hxx>
#include <document.hxx>
#include <datamapper.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <orcusfilters.hxx>

namespace sc
{
class XMLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;
    OUString maID;
    ScOrcusImportXMLParam maParam;
    std::unique_ptr<ScOrcusXMLContext> mpXMLContext;
    const std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;
    std::function<void()> maImportFinishedHdl;

public:
    XMLFetchThread(ScDocument& rDoc, const OUString&, const ScOrcusImportXMLParam& rParam,
                   const OUString& rID, std::function<void()> aImportFinishedHdl,
                   std::vector<std::shared_ptr<sc::DataTransformation>>&& rTransformations);
    virtual void execute() override;
};

XMLFetchThread::XMLFetchThread(
    ScDocument& rDoc, const OUString& rURL, const ScOrcusImportXMLParam& rParam,
    const OUString& rID, std::function<void()> aImportFinishedHdl,
    std::vector<std::shared_ptr<sc::DataTransformation>>&& rTransformations)
    : salhelper::Thread("XML Fetch Thread")
    , mrDocument(rDoc)
    , maURL(rURL)
    , maID(rID)
    , maParam(rParam)
    , maDataTransformations(std::move(rTransformations))
    , maImportFinishedHdl(std::move(aImportFinishedHdl))
{
}

void XMLFetchThread::execute()
{
    ScOrcusFilters* pOrcus = ScFormatFilter::Get().GetOrcusFilters();
    if (!pOrcus)
        return;

    mpXMLContext = pOrcus->createXMLContext(mrDocument, maURL);
    if (!mpXMLContext)
        return;

    if (!maID.isEmpty())
    {
        ScOrcusImportXMLParam::RangeLink aRangeLink;
        aRangeLink.maPos = ScAddress(0, 0, 0);
        aRangeLink.maFieldPaths.push_back(OUStringToOString(maID, RTL_TEXTENCODING_UTF8));
        maParam.maRangeLinks.clear();
        maParam.maRangeLinks.push_back(aRangeLink);
    }
    // Do the import.
    SolarMutexGuard aGuard;
    mpXMLContext->importXML(maParam);

    for (auto& itr : maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    maImportFinishedHdl();
}

XMLDataProvider::XMLDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource)
    : DataProvider(rDataSource)
    , mpDocument(pDoc)
{
}

XMLDataProvider::~XMLDataProvider()
{
    if (mxXMLFetchThread.is())
    {
        SolarMutexReleaser aReleaser;
        mxXMLFetchThread->join();
    }
}

void XMLDataProvider::Import()
{
    // already importing data
    if (mpDoc)
        return;

    mpDoc.reset(new ScDocument(SCDOCMODE_CLIP));
    mpDoc->ResetClip(mpDocument, SCTAB(0));
    mxXMLFetchThread = new XMLFetchThread(*mpDoc, mrDataSource.getURL(),
                                          mrDataSource.getXMLImportParam(), mrDataSource.getID(),
                                          std::bind(&XMLDataProvider::ImportFinished, this),
                                          std::vector(mrDataSource.getDataTransformation()));
    mxXMLFetchThread->launch();

    if (mbDeterministic)
    {
        SolarMutexReleaser aReleaser;
        mxXMLFetchThread->join();
    }
}

void XMLDataProvider::ImportFinished() { mrDataSource.getDBManager()->WriteToDoc(*mpDoc); }

const OUString& XMLDataProvider::GetURL() const { return mrDataSource.getURL(); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
