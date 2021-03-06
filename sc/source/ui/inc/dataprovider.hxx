/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <salhelper/thread.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <osl/mutex.hxx>
#include <document.hxx>

#include <rtl/strbuf.hxx>

#include <vector>
//#include <map>

#include <orcus/csv_parser.hpp>

class SvStream;
class ScDBData;

namespace sc {

class DataTransformation;
class ExternalDataSource;

class CSVFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;

    bool mbTerminate;
    osl::Mutex maMtxTerminate;

    orcus::csv::parser_config maConfig;

    std::vector<std::shared_ptr<sc::DataTransformation>> maDataTransformations;

    std::function<void()> maImportFinishedHdl;


public:
    CSVFetchThread(ScDocument& rDoc, const OUString&, std::function<void()> aImportFinishedHdl,
            const std::vector<std::shared_ptr<sc::DataTransformation>>& mrDataTransformations);
    virtual ~CSVFetchThread() override;

    void RequestTerminate();
    bool IsRequestedTerminate();
    void Terminate();
    void EndThread();

    virtual void execute() override;
};

/**
 * Abstract class for all data provider.
 *
 */
class DataProvider
{
protected:
    /**
     * If true make the threaded import deterministic for the tests.
     */
    bool mbDeterministic;
    sc::ExternalDataSource& mrDataSource;

public:
    DataProvider(sc::ExternalDataSource& rDataSource);

    virtual ~DataProvider();

    virtual void Import() = 0;

    virtual const OUString& GetURL() const = 0;

    static std::unique_ptr<SvStream> FetchStreamFromURL(const OUString&, OStringBuffer& rBuffer);

    void setDeterministic();
};

class CSVDataProvider : public DataProvider
{
    rtl::Reference<CSVFetchThread> mxCSVFetchThread;
    ScDocument* mpDocument;
    ScDocumentUniquePtr mpDoc;

    void Refresh();

public:
    CSVDataProvider (ScDocument* pDoc, sc::ExternalDataSource& rDataSource);
    virtual ~CSVDataProvider() override;

    virtual void Import() override;

    const OUString& GetURL() const override;
    void ImportFinished();
};

/**
 * This class handles the copying of the data from the imported
 * temporary document to the actual document. Additionally, in the future
 * we may decide to store data transformations in this class.
 *
 * In addition this class also handles how to deal with excess data by for example extending the ScDBData or by only showing the first or last entries.
 *
 * TODO: move the DataProvider::WriteToDoc here
 *
 */
class ScDBDataManager
{
    OUString maDBName;
    ScDocument* mpDoc;

public:
    ScDBDataManager(const OUString& rDBName, ScDocument* pDoc);
    ~ScDBDataManager();

    void SetDatabase(const OUString& rDBName);

    ScDBData* getDBData();

    void WriteToDoc(ScDocument& rDoc);
};

class DataProviderFactory
{
private:

    static bool isInternalDataProvider(const OUString& rProvider);

public:

    static std::shared_ptr<DataProvider> getDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource);

    static std::vector<OUString> getDataProviders();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
