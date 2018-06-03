/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_JSONDATAPROVIDER_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_JSONDATAPROVIDER_HXX

#include <dataprovider.hxx>

namespace sc {

class JSONFetchThread;
class DataTransformation;

class JSONFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;

    bool mbTerminate;
    osl::Mutex maMtxTerminate;

    std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;

    std::function<void()> maImportFinishedHdl;


public:
    JSONFetchThread(ScDocument& rDoc, const OUString&, std::function<void()> aImportFinishedHdl,
            const std::vector<std::shared_ptr<sc::DataTransformation>>& mrDataTransformations);
    virtual ~JSONFetchThread() override;

    void RequestTerminate();
    bool IsRequestedTerminate();
    void Terminate();
    void EndThread();

    virtual void execute() override;
};

class JSONDataProvider : public DataProvider
{
    rtl::Reference<JSONFetchThread> mxJSONFetchThread;
    ScDocument* mpDocument;
    ScDocumentUniquePtr mpDoc;

    void Refresh();

public:
    JSONDataProvider (ScDocument* pDoc, sc::ExternalDataSource& rDataSource);
    virtual ~JSONDataProvider() override;

    virtual void Import() override;

    const OUString& GetURL() const override;
    void ImportFinished();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
