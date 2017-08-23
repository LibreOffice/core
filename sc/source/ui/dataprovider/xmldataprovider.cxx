/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmldataprovider.hxx"
#include "datatransformation.hxx"

namespace sc {

class XMLFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;
    OUString maID;
    const std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;

    Idle* mpIdle;

public:
    XMLFetchThread(ScDocument& rDoc, const OUString&, const OUString& rID, Idle* pIdle,
            const std::vector<std::shared_ptr<sc::DataTransformation>>& rTransformations);

    virtual void execute() override;
};

void XMLFetchThread::execute()
{
    for (auto& itr: maDataTransformations)
    {
        itr->Transform(mrDocument);
    }

    SolarMutexGuard aGuard;
    mpIdle->Start();
}

XMLDataProvider::XMLDataProvider(ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pDBManager,
        const OUString& rID):
    maURL(rURL),
    maID(rID),
    mpDocument(pDoc),
    mpDBDataManager(pDBManager),
    maIdle("XMLDataProvider CopyHandler")
{
    maIdle.SetInvokeHandler(LINK(this, XMLDataProvider, ImportFinishedHdl));
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
}

const OUString& XMLDataProvider::GetURL() const
{
    return maURL;
}

std::map<OUString, OUString> XMLDataProvider::getDataSourcesForURL(const OUString& /*rURL*/)
{
    std::map<OUString, OUString> aMap;

    return aMap;
}

IMPL_LINK_NOARG(XMLDataProvider, ImportFinishedHdl, Timer*, void)
{
    mpDBDataManager->WriteToDoc(*mpDoc);
    mxXMLFetchThread.clear();
    mpDoc.reset();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
