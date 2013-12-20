/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datastream.hxx>

#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <officecfg/Office/Common.hxx>
#include <osl/conditn.hxx>
#include <osl/time.h>
#include <rtl/strbuf.hxx>
#include <salhelper/thread.hxx>
#include <sfx2/viewfrm.hxx>
#include <datastreamdlg.hxx>
#include <docsh.hxx>
#include <rangelst.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <stringutil.hxx>
#include <documentlinkmgr.hxx>

#include <config_orcus.h>

#if ENABLE_ORCUS
#if defined WNT
#define __ORCUS_STATIC_LIB
#endif
#include <orcus/csv_parser.hpp>
#endif

#include <queue>

namespace sc {

inline double getNow()
{
    TimeValue now;
    osl_getSystemTime(&now);
    return static_cast<double>(now.Seconds) + static_cast<double>(now.Nanosec) / 1000000000.0;
}

namespace datastreams {

class CallerThread : public salhelper::Thread
{
    DataStream *mpDataStream;
public:
    osl::Condition maStart;
    bool mbTerminate;

    CallerThread(DataStream *pData):
        Thread("CallerThread")
        ,mpDataStream(pData)
        ,mbTerminate(false)
    {}

private:
    virtual void execute()
    {
        while (!mbTerminate)
        {
            // wait for a small amount of time, so that
            // painting methods have a chance to be called.
            // And also to make UI more responsive.
            TimeValue const aTime = {0, 1000};
            maStart.wait();
            maStart.reset();
            if (!mbTerminate)
                while (mpDataStream->ImportData())
                    wait(aTime);
        };
    }
};

class ReaderThread : public salhelper::Thread
{
    SvStream *mpStream;
public:
    bool mbTerminateReading;
    osl::Condition maProduceResume;
    osl::Condition maConsumeResume;
    osl::Mutex maLinesProtector;
    std::queue<LinesList* > maPendingLines;
    std::queue<LinesList* > maUsedLines;

    ReaderThread(SvStream *pData):
        Thread("ReaderThread")
        ,mpStream(pData)
        ,mbTerminateReading(false)
    {
    }

    virtual ~ReaderThread()
    {
        delete mpStream;
        while (!maPendingLines.empty())
        {
            delete maPendingLines.front();
            maPendingLines.pop();
        }
        while (!maUsedLines.empty())
        {
            delete maUsedLines.front();
            maUsedLines.pop();
        }
    }

    void endThread()
    {
        mbTerminateReading = true;
        maProduceResume.set();
        join();
    }

private:
    virtual void execute() SAL_OVERRIDE
    {
        while (!mbTerminateReading)
        {
            LinesList *pLines = 0;
            osl::ResettableMutexGuard aGuard(maLinesProtector);
            if (!maUsedLines.empty())
            {
                pLines = maUsedLines.front();
                maUsedLines.pop();
                aGuard.clear(); // unlock
            }
            else
            {
                aGuard.clear(); // unlock
                pLines = new LinesList(10);
            }
            for (size_t i = 0; i < pLines->size(); ++i)
                mpStream->ReadLine( pLines->at(i) );
            aGuard.reset(); // lock
            while (!mbTerminateReading && maPendingLines.size() >= 8)
            { // pause reading for a bit
                aGuard.clear(); // unlock
                maProduceResume.wait();
                maProduceResume.reset();
                aGuard.reset(); // lock
            }
            maPendingLines.push(pLines);
            maConsumeResume.set();
            if (!mpStream->good())
                mbTerminateReading = true;
        }
    }
};

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
    sc::DocumentLinkManager& rMgr = pShell->GetDocument()->GetDocLinkManager();
    rMgr.setDataStream(pLink);
    return pLink;
}

DataStream::MoveType DataStream::ToMoveType( const OUString& rMoveStr )
{
    if (rMoveStr == "RANGE_DOWN")
        return RANGE_DOWN;
    if (rMoveStr == "MOVE_DOWN")
        return MOVE_DOWN;
    if (rMoveStr == "MOVE_UP")
        return MOVE_UP;

    return NO_MOVE; // default
}

DataStream::DataStream(ScDocShell *pShell, const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, sal_uInt32 nSettings) :
    mpDocShell(pShell),
    mpDoc(mpDocShell->GetDocument()),
    maDocAccess(*mpDoc),
    meOrigMove(NO_MOVE),
    meMove(NO_MOVE),
    mbRunning(false),
    mbValuesInLine(false),
    mbRefreshOnEmptyLine(false),
    mpLines(0),
    mnLinesCount(0),
    mnLinesSinceRefresh(0),
    mfLastRefreshTime(0.0),
    mnCurRow(0)
{
    mxThread = new datastreams::CallerThread( this );
    mxThread->launch();

    Decode(rURL, rRange, nLimit, eMove, nSettings);
}

DataStream::~DataStream()
{
    if (mbRunning)
        StopImport();
    mxThread->mbTerminate = true;
    mxThread->maStart.set();
    mxThread->join();
    if (mxReaderThread.is())
        mxReaderThread->endThread();
    delete mpLines;
}

OString DataStream::ConsumeLine()
{
    if (!mpLines || mnLinesCount >= mpLines->size())
    {
        mnLinesCount = 0;
        if (mxReaderThread->mbTerminateReading)
            return OString();
        osl::ResettableMutexGuard aGuard(mxReaderThread->maLinesProtector);
        if (mpLines)
            mxReaderThread->maUsedLines.push(mpLines);
        while (mxReaderThread->maPendingLines.empty())
        {
            aGuard.clear(); // unlock
            mxReaderThread->maConsumeResume.wait();
            mxReaderThread->maConsumeResume.reset();
            aGuard.reset(); // lock
        }
        mpLines = mxReaderThread->maPendingLines.front();
        mxReaderThread->maPendingLines.pop();
        if (mxReaderThread->maPendingLines.size() <= 4)
            mxReaderThread->maProduceResume.set(); // start producer again
    }
    return mpLines->at(mnLinesCount++);
}

ScRange DataStream::GetRange() const
{
    ScRange aRange = maStartRange;
    aRange.aEnd = maEndRange.aEnd;
    return aRange;
}

bool DataStream::IsRefreshOnEmptyLine() const
{
    return mbRefreshOnEmptyLine;
}

DataStream::MoveType DataStream::GetMove() const
{
    return meOrigMove;
}

void DataStream::Decode(const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, const sal_uInt32 nSettings)
{
    msURL = rURL;
    mnLimit = nLimit;
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
        SvStream *pStream = 0;
        if (mnSettings & SCRIPT_STREAM)
            pStream = new SvScriptStream(msURL);
        else
            pStream = new SvFileStream(msURL, STREAM_READ);
        mxReaderThread = new datastreams::ReaderThread( pStream );
        mxReaderThread->launch();
    }
    mbRunning = true;
    maDocAccess.reset();
    mxThread->maStart.set();
}

void DataStream::StopImport()
{
    if (!mbRunning)
        return;

    mbRunning = false;
    Refresh();
}

void DataStream::SetRefreshOnEmptyLine( bool bVal )
{
    mbRefreshOnEmptyLine = bVal;
}

void DataStream::Refresh()
{
    Application::Yield();

    // Hard recalc will repaint the grid area.
    mpDocShell->DoHardRecalc(true);
    mpDocShell->SetDocumentModified(true);

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
            // Remove the top row and shift the remaining rows upward. Then
            // insert a new row at the end row position.
            ScRange aRange = maStartRange;
            aRange.aEnd = maEndRange.aEnd;
            maDocAccess.shiftRangeUp(aRange);
        }
        break;
        case MOVE_DOWN:
        {
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
}

#if ENABLE_ORCUS

namespace {

struct StrVal
{
    ScAddress maPos;
    OUString maStr;

    StrVal( const ScAddress& rPos, const OUString& rStr ) : maPos(rPos), maStr(rStr) {}
};

struct NumVal
{
    ScAddress maPos;
    double mfVal;

    NumVal( const ScAddress& rPos, double fVal ) : maPos(rPos), mfVal(fVal) {}
};

typedef std::vector<StrVal> StrValArray;
typedef std::vector<NumVal> NumValArray;

/**
 * This handler handles a single line CSV input.
 */
class CSVHandler
{
    ScAddress maPos;
    SCCOL mnEndCol;

    StrValArray maStrs;
    NumValArray maNums;

public:
    CSVHandler( const ScAddress& rPos, SCCOL nEndCol ) : maPos(rPos), mnEndCol(nEndCol) {}

    void begin_parse() {}
    void end_parse() {}
    void begin_row() {}
    void end_row() {}

    void cell(const char* p, size_t n)
    {
        if (maPos.Col() <= mnEndCol)
        {
            OUString aStr(p, n, RTL_TEXTENCODING_UTF8);
            double fVal;
            if (ScStringUtil::parseSimpleNumber(aStr, '.', ',', fVal))
                maNums.push_back(NumVal(maPos, fVal));
            else
                maStrs.push_back(StrVal(maPos, aStr));
        }
        maPos.IncCol();
    }

    const StrValArray& getStrs() const { return maStrs; }
    const NumValArray& getNums() const { return maNums; }
};

}

void DataStream::Text2Doc()
{
    OString aLine = ConsumeLine();
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.text_qualifier = '"';
    CSVHandler aHdl(ScAddress(maStartRange.aStart.Col(), mnCurRow, maStartRange.aStart.Tab()), maStartRange.aEnd.Col());
    orcus::csv_parser<CSVHandler> parser(aLine.getStr(), aLine.getLength(), aHdl, aConfig);
    parser.parse();

    const StrValArray& rStrs = aHdl.getStrs();
    const NumValArray& rNums = aHdl.getNums();
    if (rStrs.empty() && rNums.empty() && mbRefreshOnEmptyLine)
    {
        // Empty line detected.  Trigger refresh and discard it.
        Refresh();
        return;
    }

    MoveData();
    {
        StrValArray::const_iterator it = rStrs.begin(), itEnd = rStrs.end();
        for (; it != itEnd; ++it)
            maDocAccess.setStringCell(it->maPos, it->maStr);
    }

    {
        NumValArray::const_iterator it = rNums.begin(), itEnd = rNums.end();
        for (; it != itEnd; ++it)
            maDocAccess.setNumericCell(it->maPos, it->mfVal);
    }

    if (meMove == NO_MOVE)
        return;

    if (meMove == RANGE_DOWN)
    {
        ++mnCurRow;
//      mpDocShell->GetViewData()->GetView()->AlignToCursor(
//              maStartRange.aStart.Col(), mnCurRow, SC_FOLLOW_JUMP);
    }

    if (getNow() - mfLastRefreshTime > 0.1 && mnLinesSinceRefresh > 200)
        // Refresh no more frequently than every 0.1 second, and wait until at
        // least we have processed 200 lines.
        Refresh();

    ++mnLinesSinceRefresh;
}

#else

void DataStream::Text2Doc() {}

#endif

bool DataStream::ImportData()
{
    SolarMutexGuard aGuard;
    if (!mbValuesInLine)
        // We no longer support this mode. To be deleted later.
        return false;

    if (ScDocShell::GetViewData()->GetViewShell()->NeedsRepaint())
        return mbRunning;

    Text2Doc();
    return mbRunning;
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
