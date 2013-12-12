/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DATASTREAM_HXX
#define SC_DATASTREAM_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <address.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

#include <documentstreamaccess.hxx>

class ScDocShell;
class ScDocument;
class Window;

namespace sc {

namespace datastreams {
    class CallerThread;
    class ReaderThread;
}

typedef std::vector<OString> LinesList;

class DataStream : boost::noncopyable
{
    OString ConsumeLine();
    void MoveData();
    void Text2Doc();

public:
    enum MoveType { NO_MOVE, RANGE_DOWN, MOVE_DOWN, MOVE_UP };
    enum { SCRIPT_STREAM = 1, VALUES_IN_LINE = 2 };

    static void MakeToolbarVisible();
    static DataStream* Set(ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
            sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings);

    static MoveType ToMoveType( const OUString& rMoveStr );

    DataStream(
        ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings);

    ~DataStream();

    ScRange GetRange() const;
    const OUString& GetURL() const { return msURL; }
    const sal_Int32& GetLimit() const { return mnLimit; }
    MoveType GetMove() const;
    const sal_uInt32& GetSettings() const { return mnSettings; }
    bool IsRefreshOnEmptyLine() const;

    void Decode(
        const OUString& rURL, const ScRange& rRange, sal_Int32 nLimit,
        MoveType eMove, const sal_uInt32 nSettings);

    bool ImportData();
    void StartImport();
    void StopImport();

    void SetRefreshOnEmptyLine( bool bVal );

private:
    void Refresh();

private:
    ScDocShell* mpDocShell;
    ScDocument* mpDoc;
    DocumentStreamAccess maDocAccess;
    OUString msURL;
    sal_Int32 mnLimit;
    sal_uInt32 mnSettings;
    MoveType meOrigMove; // Initial move setting. This one gets saved to file.
    MoveType meMove; // move setting during streaming, which may change in the middle.
    bool mbRunning;
    bool mbValuesInLine;
    bool mbRefreshOnEmptyLine;
    LinesList* mpLines;
    size_t mnLinesCount;
    size_t mnLinesSinceRefresh;
    double mfLastRefreshTime;
    SCROW mnCurRow;
    ScRange maStartRange;
    ScRange maEndRange;
    rtl::Reference<datastreams::CallerThread> mxThread;
    rtl::Reference<datastreams::ReaderThread> mxReaderThread;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
