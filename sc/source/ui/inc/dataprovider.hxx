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

#include <salhelper/thread.hxx>
#include <tools/stream.hxx>
#include <rtl/ustring.hxx>
#include <address.hxx>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <queue>

#include <config_orcus.h>
#include "officecfg/Office/Calc.hxx"

#if ENABLE_ORCUS
#if defined(_WIN32)
#define __ORCUS_STATIC_LIB
#endif
#include <orcus/csv_parser.hpp>
#endif

namespace sc {

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
    SvStream *mpStream;
    size_t mnColCount;

    bool mbTerminate;
    osl::Mutex maMtxTerminate;

#if ENABLE_ORCUS
    orcus::csv::parser_config maConfig;
#endif

    virtual void execute() override;

public:
    CSVFetchThread(SvStream*);
    virtual ~CSVFetchThread() override;

    void RequestTerminate();
    bool IsRequestedTerminate();
    void Terminate();
    void EndThread();
    void EmptyLineQueue(std::queue<LinesType*>& );
};

class DataProvider
{
    virtual ~DataProvider() = 0;

    virtual void StartImport() = 0;
    virtual void StopImport() = 0;
    virtual void Refresh() = 0;
    virtual void SetUrl(OUString) = 0;
    virtual void SetRefreshRate(double) = 0;

    virtual ScRange GetRange() const = 0;
    virtual const OUString& GetURL() const = 0;

protected:
    virtual void DecodeURL() = 0;
};

class CSVDataProvider : public DataProvider
{
    OUString maURL;
    double mnRefreshRate;

    bool mbImportUnderway;

public:
    CSVDataProvider (OUString& rUrl);
    virtual ~CSVDataProvider() override;

    virtual void StartImport() override;
    virtual void StopImport() override;
    virtual void Refresh() override;
    virtual void SetUrl(OUString) override;
    virtual void SetRefreshRate(double mnRefreshRate) override;

    ScRange GetRange() const override;
    const OUString& GetURL() const override { return maURL; }

private:
    virtual void DecodeURL() override;
};

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
