/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DATAPROVIDER_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DATAPROVIDER_HXX

#include <memory>
#include <salhelper/thread.hxx>
#include <tools/stream.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <address.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <dbdata.hxx>
#include <document.hxx>
#include <vcl/idle.hxx>

#include "docsh.hxx"
#include "scdllapi.h"
#include "datamapper.hxx"

#include <queue>

#include "officecfg/Office/Calc.hxx"

#if defined(_WIN32)
#define __ORCUS_STATIC_LIB
#endif
#include <orcus/csv_parser.hpp>

namespace sc {

class DataProvider;
class CSVDataProvider;
class ScDBDataManager;

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

typedef std::vector<Line> LinesType;

class CSVFetchThread : public salhelper::Thread
{
    ScDocument& mrDocument;
    OUString maURL;

    bool mbTerminate;
    osl::Mutex maMtxTerminate;

    std::queue<LinesType*> maPendingLines;

    orcus::csv::parser_config maConfig;

    Idle* mpIdle;


public:
    CSVFetchThread(ScDocument& rDoc, const OUString&, Idle* pIdle);
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
public:
    virtual ~DataProvider() = 0;

    virtual void Import() = 0;

    virtual const OUString& GetURL() const = 0;
};

class CSVDataProvider : public DataProvider
{
    OUString maURL;
    rtl::Reference<CSVFetchThread> mxCSVFetchThread;
    ScDocument* mpDocument;
    ScDBDataManager* mpDBDataManager;
    LinesType* mpLines;
    size_t mnLineCount;
    std::unique_ptr<ScDocument> mpDoc;
    Idle maIdle;

    void Refresh();
    Line GetLine();

public:
    CSVDataProvider (ScDocument* pDoc, const OUString& rURL, ScDBDataManager* pDBManager);
    virtual ~CSVDataProvider() override;

    virtual void Import() override;

    const OUString& GetURL() const override { return maURL; }
    DECL_LINK( ImportFinishedHdl, Timer*, void );
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
    ScDBDataManager(const OUString& rDBName, bool bAllowResize, ScDocument* pDoc);
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

    static std::shared_ptr<DataProvider> getDataProvider(ScDocument* pDoc, const OUString& rProvider, const OUString& rURL, const OUString& rID, ScDBDataManager* pManager);

    static std::vector<OUString> getDataProviders();
};

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
