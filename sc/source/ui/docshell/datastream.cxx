/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datastream.hxx>
#include <datastreamgettime.hxx>

#include <com/sun/star/frame/XLayoutManager.hpp>
#include <osl/conditn.hxx>
#include <osl/time.h>
#include <salhelper/thread.hxx>
#include <sfx2/viewfrm.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <stringutil.hxx>
#include <documentlinkmgr.hxx>
#include <o3tl/enumarray.hxx>

#include <officecfg/Office/Calc.hxx>

#include <orcus/csv_parser.hpp>

#include <queue>

namespace com::sun::star::ui { class XUIElement; }

namespace sc {

static o3tl::enumarray<DebugTime, double> fTimes { 0.0, 0.0, 0.0 };

double datastream_get_time(DebugTime nIdx)
{
    return fTimes[ nIdx ];
}

namespace {

double getNow()
{
    TimeValue now;
    osl_getSystemTime(&now);
    return static_cast<double>(now.Seconds) + static_cast<double>(now.Nanosec) / 1000000000.0;
}

class CSVHandler
{
    DataStream::Line& mrLine;
    size_t const mnColCount;
    size_t mnCols;
    const char* mpLineHead;

public:
    CSVHandler( DataStream::Line& rLine, size_t nColCount ) :
        mrLine(rLine), mnColCount(nColCount), mnCols(0), mpLineHead(rLine.maLine.getStr()) {}

    static void begin_parse() {}
    static void end_parse() {}
    static void begin_row() {}
    static void end_row() {}

    void cell(const char* p, size_t n, bool /*transient*/)
    {
        if (mnCols >= mnColCount)
            return;

        DataStream::Cell aCell;
        if (ScStringUtil::parseSimpleNumber(p, n, '.', ',', aCell.mfValue))
        {
            aCell.mbValue = true;
        }
        else
        {
            aCell.mbValue = false;
            aCell.maStr.Pos = std::distance(mpLineHead, p);
            aCell.maStr.Size = n;
        }
        mrLine.maCells.push_back(aCell);

        ++mnCols;
    }
};

}

namespace datastreams {

class ReaderThread : public salhelper::Thread
{
    std::unique_ptr<SvStream> mpStream;
    size_t const mnColCount;
    bool mbTerminate;
    osl::Mutex maMtxTerminate;

    std::queue<std::unique_ptr<DataStream::LinesType>> maPendingLines;
    std::queue<std::unique_ptr<DataStream::LinesType>> maUsedLines;
    osl::Mutex maMtxLines;

    osl::Condition maCondReadStream;
    osl::Condition maCondConsume;

    orcus::csv::parser_config maConfig;

public:

    ReaderThread(std::unique_ptr<SvStream> pData, size_t nColCount):
        Thread("ReaderThread"),
        mpStream(std::move(pData)),
        mnColCount(nColCount),
        mbTerminate(false)
    {
        maConfig.delimiters.push_back(',');
        maConfig.text_qualifier = '"';
    }

    bool isTerminateRequested()
    {
        osl::MutexGuard aGuard(maMtxTerminate);
        return mbTerminate;
    }

    void requestTerminate()
    {
        osl::MutexGuard aGuard(maMtxTerminate);
        mbTerminate = true;
    }

    void endThread()
    {
        requestTerminate();
        maCondReadStream.set();
    }

    void waitForNewLines()
    {
        maCondConsume.wait();
        maCondConsume.reset();
    }

    std::unique_ptr<DataStream::LinesType> popNewLines()
    {
        auto pLines = std::move(maPendingLines.front());
        maPendingLines.pop();
        return pLines;
    }

    void resumeReadStream()
    {
        if (maPendingLines.size() <= 4)
            maCondReadStream.set(); // start producer again
    }

    bool hasNewLines()
    {
        return !maPendingLines.empty();
    }

    void pushUsedLines( std::unique_ptr<DataStream::LinesType> pLines )
    {
        maUsedLines.push(std::move(pLines));
    }

    osl::Mutex& getLinesMutex()
    {
        return maMtxLines;
    }

private:
    virtual void execute() override
    {
        while (!isTerminateRequested())
        {
            std::unique_ptr<DataStream::LinesType> pLines;
            osl::ResettableMutexGuard aGuard(maMtxLines);

            if (!maUsedLines.empty())
            {
                // Re-use lines from previous runs.
                pLines = std::move(maUsedLines.front());
                maUsedLines.pop();
                aGuard.clear(); // unlock
            }
            else
            {
                aGuard.clear(); // unlock
                pLines.reset(new DataStream::LinesType(10));
            }

            // Read & store new lines from stream.
            for (DataStream::Line & rLine : *pLines)
            {
                rLine.maCells.clear();
                mpStream->ReadLine(rLine.maLine);
                CSVHandler aHdl(rLine, mnColCount);
                orcus::csv_parser<CSVHandler> parser(rLine.maLine.getStr(), rLine.maLine.getLength(), aHdl, maConfig);
                parser.parse();
            }

            aGuard.reset(); // lock
            while (!isTerminateRequested() && maPendingLines.size() >= 8)
            {
                // pause reading for a bit
                aGuard.clear(); // unlock
                maCondReadStream.wait();
                maCondReadStream.reset();
                aGuard.reset(); // lock
            }
            maPendingLines.push(std::move(pLines));
            maCondConsume.set();
            if (!mpStream->good())
                requestTerminate();
        }
    }
};

}

DataStream::Cell::Cell() : mfValue(0.0), mbValue(true) {}

DataStream::Cell::Cell( const Cell& r ) : mbValue(r.mbValue)
{
    if (r.mbValue)
        mfValue = r.mfValue;
    else
    {
        maStr.Pos = r.maStr.Pos;
        maStr.Size = r.maStr.Size;
    }
}

void DataStream::MakeToolbarVisible()
{
    css::uno::Reference< css::frame::XFrame > xFrame =
        ScDocShell::GetViewData()->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
    if (!xFrame.is())
        return;

    css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue("LayoutManager") >>= xLayoutManager;
    if (!xLayoutManager.is())
        return;

    const OUString sResourceURL( "private:resource/toolbar/datastreams" );
    css::uno::Reference< css::ui::XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
    if (!xUIElement.is())
    {
        xLayoutManager->createElement( sResourceURL );
        xLayoutManager->showElement( sResourceURL );
    }
}

DataStream* DataStream::Set(
    ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
    sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings)
{
    DataStream* pLink = new DataStream(pShell, rURL, rRange, nLimit, eMove, nSettings);
    sc::DocumentLinkManager& rMgr = pShell->GetDocument().GetDocLinkManager();
    rMgr.setDataStream(pLink);
    return pLink;
}

DataStream::DataStream(ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings) :
    mpDocShell(pShell),
    maDocAccess(mpDocShell->GetDocument()),
    meOrigMove(NO_MOVE),
    meMove(NO_MOVE),
    mbRunning(false),
    mbValuesInLine(false),
    mbRefreshOnEmptyLine(false),
    mnLinesCount(0),
    mnLinesSinceRefresh(0),
    mfLastRefreshTime(0.0),
    mnCurRow(0),
    mbIsFirst(true),
    mbIsUpdate(false)
{
    maImportTimer.SetTimeout(0);
    maImportTimer.SetInvokeHandler( LINK(this, DataStream, ImportTimerHdl) );

    Decode(rURL, rRange, nLimit, eMove, nSettings);
}

DataStream::~DataStream()
{
    if (mbRunning)
        StopImport();

    if (mxReaderThread.is())
    {
        mxReaderThread->endThread();
        mxReaderThread->join();
    }
    mpLines.reset();
}

DataStream::Line DataStream::ConsumeLine()
{
    if (!mpLines || mnLinesCount >= mpLines->size())
    {
        mnLinesCount = 0;
        if (mxReaderThread->isTerminateRequested())
            return Line();

        osl::ResettableMutexGuard aGuard(mxReaderThread->getLinesMutex());
        if (mpLines)
            mxReaderThread->pushUsedLines(std::move(mpLines));

        while (!mxReaderThread->hasNewLines())
        {
            aGuard.clear(); // unlock
            mxReaderThread->waitForNewLines();
            aGuard.reset(); // lock
        }

        mpLines = mxReaderThread->popNewLines();
        mxReaderThread->resumeReadStream();
    }
    return mpLines->at(mnLinesCount++);
}

ScRange DataStream::GetRange() const
{
    ScRange aRange = maStartRange;
    aRange.aEnd = maEndRange.aEnd;
    return aRange;
}

void DataStream::Decode(const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, const sal_uInt32 nSettings)
{
    msURL = rURL;
    meMove = eMove;
    meOrigMove = eMove;
    mnSettings = nSettings;

    mbValuesInLine = true; // always true.

    mnCurRow = rRange.aStart.Row();

    ScRange aRange = rRange;
    if (aRange.aStart.Row() != aRange.aEnd.Row())
        // We only allow this range to be one row tall.
        aRange.aEnd.SetRow(aRange.aStart.Row());

    maStartRange = aRange;
    maEndRange = aRange;
    if (nLimit == 0)
    {
        // Unlimited
        maEndRange.aStart.SetRow(MAXROW);
    }
    else if (nLimit > 0)
    {
        // Limited.
        maEndRange.aStart.IncRow(nLimit-1);
        if (maEndRange.aStart.Row() > MAXROW)
            maEndRange.aStart.SetRow(MAXROW);
    }

    maEndRange.aEnd.SetRow(maEndRange.aStart.Row());
}

void DataStream::StartImport()
{
    if (mbRunning)
        return;

    if (!mxReaderThread.is())
    {
        std::unique_ptr<SvStream> pStream(new SvFileStream(msURL, StreamMode::READ));
        mxReaderThread = new datastreams::ReaderThread(std::move(pStream), maStartRange.aEnd.Col() - maStartRange.aStart.Col() + 1);
        mxReaderThread->launch();
    }
    mbRunning = true;
    maDocAccess.reset();

    maImportTimer.Start();
}

void DataStream::StopImport()
{
    if (!mbRunning)
        return;

    mbRunning = false;
    Refresh();
    maImportTimer.Stop();
}

void DataStream::SetRefreshOnEmptyLine( bool bVal )
{
    mbRefreshOnEmptyLine = bVal;
}

void DataStream::Refresh()
{
    Application::Yield();

    double fStart = getNow();

    // Hard recalc will repaint the grid area.
    mpDocShell->DoHardRecalc();
    mpDocShell->SetDocumentModified();

    fTimes[ DebugTime::Recalc ] = getNow() - fStart;

    mfLastRefreshTime = getNow();
    mnLinesSinceRefresh = 0;
}

void DataStream::MoveData()
{
    switch (meMove)
    {
        case RANGE_DOWN:
        {
            if (mnCurRow == maEndRange.aStart.Row())
                meMove = MOVE_UP;
        }
        break;
        case MOVE_UP:
        {
            mbIsUpdate = true;
            // Remove the top row and shift the remaining rows upward. Then
            // insert a new row at the end row position.
            ScRange aRange = maStartRange;
            aRange.aEnd = maEndRange.aEnd;
            maDocAccess.shiftRangeUp(aRange);
        }
        break;
        case MOVE_DOWN:
        {
            mbIsUpdate = true;
            // Remove the end row and shift the remaining rows downward by
            // inserting a new row at the top row.
            ScRange aRange = maStartRange;
            aRange.aEnd = maEndRange.aEnd;
            maDocAccess.shiftRangeDown(aRange);
        }
        break;
        case NO_MOVE:
        default:
            ;
    }
    if(mbIsFirst && mbIsUpdate)
    {
        sal_Int32 nStreamTimeout = officecfg::Office::Calc::DataStream::UpdateTimeout::get();
        maImportTimer.SetTimeout(nStreamTimeout);
        mbIsFirst = false;
    }
}

void DataStream::Text2Doc()
{
    Line aLine = ConsumeLine();
    if (aLine.maCells.empty() && mbRefreshOnEmptyLine)
    {
        // Empty line detected.  Trigger refresh and discard it.
        Refresh();
        return;
    }

    double fStart = getNow();

    MoveData();
    {
        SCCOL nCol = maStartRange.aStart.Col();
        const char* pLineHead = aLine.maLine.getStr();
        for (const Cell& rCell : aLine.maCells)
        {
            if (rCell.mbValue)
            {
                maDocAccess.setNumericCell(
                    ScAddress(nCol, mnCurRow, maStartRange.aStart.Tab()), rCell.mfValue);
            }
            else
            {
                maDocAccess.setStringCell(
                    ScAddress(nCol, mnCurRow, maStartRange.aStart.Tab()),
                    OUString(pLineHead+rCell.maStr.Pos, rCell.maStr.Size, RTL_TEXTENCODING_UTF8));
            }
            ++nCol;
        }
    }

    fTimes[ DebugTime::Import ] = getNow() - fStart;

    if (meMove == NO_MOVE)
        return;

    if (meMove == RANGE_DOWN)
    {
        ++mnCurRow;
//      mpDocShell->GetViewData().GetView()->AlignToCursor(
//              maStartRange.aStart.Col(), mnCurRow, SC_FOLLOW_JUMP);
    }

    if (getNow() - mfLastRefreshTime > 0.1 && mnLinesSinceRefresh > 200)
        // Refresh no more frequently than every 0.1 second, and wait until at
        // least we have processed 200 lines.
        Refresh();

    ++mnLinesSinceRefresh;
}

bool DataStream::ImportData()
{
    if (!mbValuesInLine)
        // We no longer support this mode. To be deleted later.
        return false;

    if (ScDocShell::GetViewData()->GetViewShell()->NeedsRepaint())
        return mbRunning;

    Text2Doc();
    return mbRunning;
}

IMPL_LINK_NOARG(DataStream, ImportTimerHdl, Timer *, void)
{
    if (ImportData())
        maImportTimer.Start();
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
