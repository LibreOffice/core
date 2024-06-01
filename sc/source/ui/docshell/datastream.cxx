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
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <stringutil.hxx>
#include <documentlinkmgr.hxx>
#include <o3tl/enumarray.hxx>

#include <officecfg/Office/Calc.hxx>

#include <orcus/csv_parser.hpp>

#include <atomic>
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
    size_t mnColCount;
    size_t mnCols;
    const char* mpLineHead;

public:
    CSVHandler( DataStream::Line& rLine, size_t nColCount ) :
        mrLine(rLine), mnColCount(nColCount), mnCols(0), mpLineHead(rLine.maLine.getStr()) {}

    static void begin_parse() {}
    static void end_parse() {}
    static void begin_row() {}
    static void end_row() {}

    void cell(std::string_view s, bool /*transient*/)
    {
        if (mnCols >= mnColCount)
            return;

        DataStream::Cell aCell;
        if (ScStringUtil::parseSimpleNumber(s.data(), s.size(), '.', ',', aCell.mfValue))
        {
            aCell.mbValue = true;
        }
        else
        {
            aCell.mbValue = false;
            aCell.maStr.Pos = std::distance(mpLineHead, s.data());
            aCell.maStr.Size = s.size();
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
    size_t mnColCount;
    std::atomic<bool> mbTerminate;

    std::queue<DataStream::LinesType> maPendingLines;
    std::queue<DataStream::LinesType> maUsedLines;
    std::mutex maMtxLines;

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
        return mbTerminate.load();
    }

    void requestTerminate()
    {
        mbTerminate.store(true);
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

    DataStream::LinesType popNewLines()
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

    bool hasNewLines() const
    {
        return !maPendingLines.empty();
    }

    void pushUsedLines( DataStream::LinesType pLines )
    {
        maUsedLines.push(std::move(pLines));
    }

    std::mutex& getLinesMutex()
    {
        return maMtxLines;
    }

private:
    virtual void execute() override
    {
        while (!isTerminateRequested())
        {
            std::optional<DataStream::LinesType> oLines;
            std::unique_lock aGuard(maMtxLines);

            if (!maUsedLines.empty())
            {
                // Re-use lines from previous runs.
                oLines = std::move(maUsedLines.front());
                maUsedLines.pop();
                aGuard.unlock(); // unlock
            }
            else
            {
                aGuard.unlock(); // unlock
                oLines.emplace(10);
            }

            // Read & store new lines from stream.
            for (DataStream::Line & rLine : *oLines)
            {
                rLine.maCells.clear();
                mpStream->ReadLine(rLine.maLine);
                CSVHandler aHdl(rLine, mnColCount);
                orcus::csv_parser<CSVHandler> parser(rLine.maLine, aHdl, maConfig);
                parser.parse();
            }

            aGuard.lock(); // lock
            while (!isTerminateRequested() && maPendingLines.size() >= 8)
            {
                // pause reading for a bit
                aGuard.unlock(); // unlock
                maCondReadStream.wait();
                maCondReadStream.reset();
                aGuard.lock(); // lock
            }
            maPendingLines.push(std::move(*oLines));
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
    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return;

    css::uno::Reference< css::frame::XFrame > xFrame =
        pViewData->GetViewShell()->GetViewFrame().GetFrame().GetFrameInterface();
    if (!xFrame.is())
        return;

    css::uno::Reference< css::beans::XPropertySet > xPropSet(xFrame, css::uno::UNO_QUERY);
    if (!xPropSet.is())
        return;

    css::uno::Reference< css::frame::XLayoutManager > xLayoutManager;
    xPropSet->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
    if (!xLayoutManager.is())
        return;

    static constexpr OUString sResourceURL( u"private:resource/toolbar/datastreams"_ustr );
    css::uno::Reference< css::ui::XUIElement > xUIElement = xLayoutManager->getElement(sResourceURL);
    if (!xUIElement.is())
    {
        xLayoutManager->createElement( sResourceURL );
        xLayoutManager->showElement( sResourceURL );
    }
}

DataStream* DataStream::Set(
    ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
    sal_Int32 nLimit, MoveType eMove)
{
    DataStream* pLink = new DataStream(pShell, rURL, rRange, nLimit, eMove);
    sc::DocumentLinkManager& rMgr = pShell->GetDocument().GetDocLinkManager();
    rMgr.setDataStream(pLink);
    return pLink;
}

DataStream::DataStream(ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove) :
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
    maImportTimer("sc DataStream maImportTimer"),
    mbIsFirst(true),
    mbIsUpdate(false)
{
    maImportTimer.SetTimeout(0);
    maImportTimer.SetInvokeHandler( LINK(this, DataStream, ImportTimerHdl) );

    Decode(rURL, rRange, nLimit, eMove);
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
    moLines.reset();
}

DataStream::Line DataStream::ConsumeLine()
{
    if (!moLines || mnLinesCount >= moLines->size())
    {
        mnLinesCount = 0;
        if (mxReaderThread->isTerminateRequested())
            return Line();

        std::unique_lock aGuard(mxReaderThread->getLinesMutex());
        if (moLines)
        {
            mxReaderThread->pushUsedLines(std::move(*moLines));
            moLines.reset();
        }

        while (!mxReaderThread->hasNewLines())
        {
            aGuard.unlock(); // unlock
            mxReaderThread->waitForNewLines();
            aGuard.lock(); // lock
        }

        moLines = mxReaderThread->popNewLines();
        mxReaderThread->resumeReadStream();
    }
    return moLines->at(mnLinesCount++);
}

ScRange DataStream::GetRange() const
{
    ScRange aRange = maStartRange;
    aRange.aEnd = maEndRange.aEnd;
    return aRange;
}

void DataStream::Decode(const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove)
{
    msURL = rURL;
    meMove = eMove;
    meOrigMove = eMove;

    mbValuesInLine = true; // always true.

    mnCurRow = rRange.aStart.Row();

    ScRange aRange = rRange;
    if (aRange.aStart.Row() != aRange.aEnd.Row())
        // We only allow this range to be one row tall.
        aRange.aEnd.SetRow(aRange.aStart.Row());

    maStartRange = aRange;
    maEndRange = aRange;
    const auto & rDoc = mpDocShell->GetDocument();
    if (nLimit == 0)
    {
        // Unlimited
        maEndRange.aStart.SetRow(rDoc.MaxRow());
    }
    else if (nLimit > 0)
    {
        // Limited.
        maEndRange.aStart.IncRow(nLimit-1);
        if (maEndRange.aStart.Row() > rDoc.MaxRow())
            maEndRange.aStart.SetRow(rDoc.MaxRow());
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

    ScViewData* pViewData = ScDocShell::GetViewData();
    if (!pViewData)
        return false;

    if (pViewData->GetViewShell()->NeedsRepaint())
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
