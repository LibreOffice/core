/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_DATASTREAM_HXX
#define INCLUDED_SC_SOURCE_UI_INC_DATASTREAM_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <vcl/timer.hxx>
#include <address.hxx>

#include <vector>

#include <documentstreamaccess.hxx>

class ScDocShell;
class ScDocument;
namespace vcl { class Window; }

namespace sc {

namespace datastreams {
    class CallerThread;
    class ReaderThread;
}

class DataStream
{
public:
    DataStream(const DataStream&) = delete;
    const DataStream& operator=(const DataStream&) = delete;

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

    enum MoveType { NO_MOVE, RANGE_DOWN, MOVE_DOWN, MOVE_UP };
    enum { VALUES_IN_LINE = 2 };

    static void MakeToolbarVisible();
    static DataStream* Set(ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
            sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings);

    DataStream(
        ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings);

    ~DataStream();

    ScRange GetRange() const;
    const OUString& GetURL() const { return msURL; }
    MoveType GetMove() const { return meOrigMove;}
    bool IsRefreshOnEmptyLine() const { return mbRefreshOnEmptyLine;}

    void Decode(
        const OUString& rURL, const ScRange& rRange, sal_Int32 nLimit,
        MoveType eMove, const sal_uInt32 nSettings);

    bool ImportData();
    void StartImport();
    void StopImport();

    void SetRefreshOnEmptyLine( bool bVal );

private:
    Line ConsumeLine();
    void MoveData();
    void Text2Doc();
    void Refresh();

    DECL_LINK( ImportTimerHdl, Timer*, void );

private:
    ScDocShell* mpDocShell;
    DocumentStreamAccess maDocAccess;
    OUString msURL;
    sal_uInt32 mnSettings;
    MoveType meOrigMove; // Initial move setting. This one gets saved to file.
    MoveType meMove; // move setting during streaming, which may change in the middle.
    bool mbRunning;
    bool mbValuesInLine;
    bool mbRefreshOnEmptyLine;
    LinesType* mpLines;
    size_t mnLinesCount;
    size_t mnLinesSinceRefresh;
    double mfLastRefreshTime;
    SCROW mnCurRow;
    ScRange maStartRange;
    ScRange maEndRange;

    Timer maImportTimer;

    rtl::Reference<datastreams::ReaderThread> mxReaderThread;
    bool mbIsFirst;
    bool mbIsUpdate;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
