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

#include "docsh.hxx"
#include "scdllapi.h"

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

class SC_DLLPUBLIC ExternalDataMapper
{
    ScRange maRange;
    ScDocShell* mpDocShell;
    std::unique_ptr<DataProvider> mpDataProvider;
    ScDocument maDocument;
    ScDBCollection* mpDBCollection;
    std::shared_ptr<ScDBDataManager> mpDBDataManager;

public:
    ExternalDataMapper(ScDocShell* pDocShell, const OUString& rUrl, const OUString& rName,
        SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCOL2, SCROW nRow2, bool bAllowResize, bool& bSuccess);

    ~ExternalDataMapper();

    void StartImport();
};

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
    std::unique_ptr<SvStream> mpStream;
    ScDocument& mrDocument;
    OUString maURL;
    size_t mnColCount;
    ScDBDataManager* mpDBDataManager;

    bool mbTerminate;
    osl::Mutex maMtxTerminate;

    std::queue<LinesType*> maPendingLines;
    osl::Mutex maMtxLines;

    osl::Condition maCondReadStream;
    osl::Condition maCondConsume;

    orcus::csv::parser_config maConfig;

    virtual void execute() override;

public:
    CSVFetchThread(ScDocument& rDoc, ScDBDataManager*, const OUString&, size_t);
    virtual ~CSVFetchThread() override;

    void RequestTerminate();
    bool IsRequestedTerminate();
    void Terminate();
    void EndThread();
    void EmptyLineQueue(std::queue<LinesType*>& );
    osl::Mutex& GetLinesMutex();
    bool HasNewLines();
    void WaitForNewLines();
    LinesType* GetNewLines();
    void ResumeFetchStream();
};

class DataProvider
{
public:
    virtual ~DataProvider() = 0;

    virtual void StartImport() = 0;
    virtual void Refresh() = 0;
    virtual void WriteToDoc(ScDocument&) = 0;

    virtual ScRange GetRange() const = 0;
    virtual const OUString& GetURL() const = 0;
};

class CSVDataProvider : public DataProvider
{
    OUString maURL;
    ScRange mrRange;
    rtl::Reference<CSVFetchThread> mxCSVFetchThread;
    ScDocShell* mpDocShell;
    ScDocument* mpDocument;
    ScDBDataManager* mpDBDataManager;
    LinesType* mpLines;
    size_t mnLineCount;

    bool mbImportUnderway;


public:
    CSVDataProvider (ScDocShell* pDocShell, const OUString& rUrl, ScRange& rRange, ScDBDataManager*);
    virtual ~CSVDataProvider() override;

    virtual void StartImport() override;
    virtual void Refresh() override;
    virtual void WriteToDoc(ScDocument&) override;
    Line GetLine();

    ScRange GetRange() const override
    {
        return mrRange;
    }
    const OUString& GetURL() const override { return maURL; }
};

class ScDBDataManager
{
    ScDBData* mpDBData;
    ScRange maSourceRange;
    ScRange maDestinationRange;
    bool mbAllowResize;

public:
    ScDBDataManager(ScDBData*, bool);
    ~ScDBDataManager();

    bool IsResizeAllowed();
    bool Resize();
    bool RequiresResize(SCROW&, SCCOL&);

    void SetDatabase(ScDBData*);
    void SetSourceRange(SCCOL, SCROW);
    void SetDestinationRange(ScRange&);

    ScRange& GetDestinationRange();
    ScRange& GetSourceRange();
};

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
