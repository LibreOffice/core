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
#include <rtl/strbuf.hxx>
#include <salhelper/thread.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <arealink.hxx>
#include <asciiopt.hxx>
#include <datastreamdlg.hxx>
#include <dbfunc.hxx>
#include <docsh.hxx>
#include <documentimport.hxx>
#include <impex.hxx>
#include <rangelst.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <stringutil.hxx>

#include <config_orcus.h>

#if ENABLE_ORCUS
#if defined WNT
#define __ORCUS_STATIC_LIB
#endif
#include <orcus/csv_parser.hpp>
#endif

#include <queue>

namespace sc {

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
            TimeValue const aTime = {0, 100000};
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
    // Each DataStream needs a destination area in order to be exported.
    // There can be only one ScAreaLink / DataStream per cell.
    // So - if we don't need range (DataStream with mbValuesInLine == false),
    // just find a free cell for now.
    ScRange aDestArea;
    if (rRange.IsValid())
        aDestArea = rRange;

    sfx2::LinkManager* pLinkManager = pShell->GetDocument()->GetLinkManager();
    sal_uInt16 nLinkPos = 0;
    while (nLinkPos < pLinkManager->GetLinks().size())
    {
        sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[nLinkPos];
        if (!rRange.IsValid())
        {
            if ( (pBase->ISA(ScAreaLink) && static_cast<ScAreaLink*>
                        (&(*pBase))->GetDestArea().aStart == aDestArea.aStart)
                || (pBase->ISA(DataStream) && static_cast<DataStream*>
                        (&(*pBase))->GetRange().aStart == aDestArea.aStart) )
            {
                aDestArea.Move(0, 1, 0);
                nLinkPos = 0;
                continue;
            }
            else
                ++nLinkPos;
        }
        else if ( (pBase->ISA(ScAreaLink) && static_cast<ScAreaLink*>
                    (&(*pBase))->GetDestArea().aStart == aDestArea.aStart)
                || (pBase->ISA(DataStream) && static_cast<DataStream*>
                    (&(*pBase))->GetRange().aStart == aDestArea.aStart) )
        {
            pLinkManager->Remove( pBase );
        }
        else
            ++nLinkPos;
    }

    DataStream* pLink = new DataStream(pShell, rURL, aDestArea, nLimit, eMove, nSettings);
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rURL, NULL, NULL );
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
    meMove(NO_MOVE),
    mbRunning(false),
    mpLines(0),
    mnLinesCount(0),
    mnRepaintCounter(0),
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
    if (mpEndRange)
        aRange.aEnd = mpEndRange->aEnd;
    return aRange;
}

OUString DataStream::GetMove() const
{
    switch (meMove)
    {
        case MOVE_DOWN:
            return OUString("MOVE_DOWN");
        case MOVE_UP:
            return OUString("MOVE_UP");
        case NO_MOVE:
            return OUString("NO_MOVE");
        case RANGE_DOWN:
            return OUString("RANGE_DOWN");
        default:
            ;
    }
    return OUString();
}

void DataStream::Decode(const OUString& rURL, const ScRange& rRange,
        sal_Int32 nLimit, MoveType eMove, const sal_uInt32 nSettings)
{
    msURL = rURL;
    mnLimit = nLimit;
    meMove = eMove;
    mnSettings = nSettings;
    mpEndRange.reset( NULL );

    mbValuesInLine = mnSettings & VALUES_IN_LINE;

    mnCurRow = rRange.aStart.Row();

    ScRange aRange = rRange;
    if (aRange.aStart.Row() != aRange.aEnd.Row())
        // We only allow this range to be one row tall.
        aRange.aEnd.SetRow(aRange.aStart.Row());

    maStartRange = aRange;
    if (nLimit && aRange.aStart.Row() + nLimit - 1 < MAXROW)
    {
        mpEndRange.reset(new ScRange(aRange));
        mpEndRange->Move(0, nLimit-1, 0);
    }
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
    Repaint();
}

void DataStream::Repaint()
{
    SCROW nEndRow = mpEndRange ? mpEndRange->aEnd.Row() : MAXROW;
    ScRange aRange(maStartRange.aStart);
    aRange.aEnd = ScAddress(maStartRange.aEnd.Col(), nEndRow, maStartRange.aStart.Tab());

    mpDocShell->PostPaint(aRange, PAINT_GRID);
    mnRepaintCounter = 0;
}

void DataStream::Broadcast()
{
    mpDoc->BroadcastCells(maBroadcastRanges, SC_HINT_DATACHANGED);
    maBroadcastRanges.RemoveAll();
}

void DataStream::MoveData()
{
    if (!mpEndRange)
        return;

    switch (meMove)
    {
        case RANGE_DOWN:
        {
            if (mnCurRow == mpEndRange->aStart.Row())
                meMove = MOVE_UP;
        }
        break;
        case MOVE_UP:
        {
            // Remove the top row and shift the remaining rows upward. Then
            // insert a new row at the end row position.
            ScRange aRange = maStartRange;
            aRange.aEnd = mpEndRange->aEnd;
            maDocAccess.shiftRangeUp(aRange);
        }
        break;
        case MOVE_DOWN:
        {
            // Remove the end row and shift the remaining rows downward by
            // inserting a new row at the top row.
            ScRange aRange = maStartRange;
            aRange.aEnd = mpEndRange->aEnd;
            maDocAccess.shiftRangeDown(aRange);
        }
        break;
        case NO_MOVE:
        default:
            ;
    }
}

IMPL_LINK_NOARG(DataStream, RefreshHdl)
{
    ImportData();
    return 0;
}

#if ENABLE_ORCUS

namespace {

/**
 * This handler handles a single line CSV input.
 */
class CSVHandler
{
    DocumentStreamAccess& mrDoc;
    ScAddress maPos;
    SCROW mnRow;
    SCCOL mnCol;
    SCCOL mnEndCol;
    SCTAB mnTab;

public:
    CSVHandler( DocumentStreamAccess& rDoc, const ScAddress& rPos, SCCOL nEndCol ) :
        mrDoc(rDoc), maPos(rPos), mnEndCol(nEndCol) {}

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
                mrDoc.setNumericCell(maPos, fVal);
            else
                mrDoc.setStringCell(maPos, aStr);
        }
        maPos.IncCol();
    }
};

}

void DataStream::Text2Doc()
{
    OString aLine = ConsumeLine();
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.text_qualifier = '"';
    CSVHandler aHdl(maDocAccess, ScAddress(maStartRange.aStart.Col(), mnCurRow, maStartRange.aStart.Tab()), maStartRange.aEnd.Col());
    orcus::csv_parser<CSVHandler> parser(aLine.getStr(), aLine.getLength(), aHdl, aConfig);
    parser.parse();

    ++mnRepaintCounter;
}

#else

void DataStream::Text2Doc() {}

#endif

bool DataStream::ImportData()
{
    SolarMutexGuard aGuard;
    if (ScDocShell::GetViewData()->GetViewShell()->NeedsRepaint())
        return mbRunning;

    MoveData();
    if (mbValuesInLine)
    {
        // do CSV import
        Text2Doc();
    }
    else
    {
#if 0 // TODO : temporarily disable this code.
        ScDocumentImport aDocImport(*mpDoc);
        // read more lines at once but not too much
        for (int i = 0; i < 10; ++i)
        {
            OUString sLine( OStringToOUString(ConsumeLine(), RTL_TEXTENCODING_UTF8) );
            if (sLine.indexOf(',') <= 0)
                continue;

            OUString sAddress( sLine.copy(0, sLine.indexOf(',')) );
            OUString sValue( sLine.copy(sLine.indexOf(',') + 1) );
            ScAddress aAddress;
            aAddress.Parse(sAddress, mpDoc);
            if (!aAddress.IsValid())
                continue;

            if (sValue == "0" || ( sValue.indexOf(':') == -1 && sValue.toDouble() ))
                aDocImport.setNumericCell(aAddress, sValue.toDouble());
            else
                aDocImport.setStringCell(aAddress, sValue);
            maBroadcastRanges.Join(aAddress);
        }
        aDocImport.finalize();
#endif
    }

    if (meMove == NO_MOVE)
        return mbRunning;

    if (meMove == RANGE_DOWN)
    {
        ++mnCurRow;
//      mpDocShell->GetViewData()->GetView()->AlignToCursor(
//              maStartRange.aStart.Col(), mnCurRow, SC_FOLLOW_JUMP);
    }

    if (mnRepaintCounter > 200)
        Repaint();

    return mbRunning;
}

sfx2::SvBaseLink::UpdateResult DataStream::DataChanged(
        const OUString& , const css::uno::Any& )
{
    MakeToolbarVisible();
    StopImport();
    bool bStart = true;
    if (mnSettings & SCRIPT_STREAM && !mxReaderThread.is() &&
        officecfg::Office::Common::Security::Scripting::MacroSecurityLevel::get() >= 1)
    {
        MessageDialog aQBox( NULL, "QueryRunStreamScriptDialog", "modules/scalc/ui/queryrunstreamscriptdialog.ui");
        aQBox.set_primary_text( aQBox.get_primary_text().replaceFirst("%URL", msURL) );
        if (RET_YES != aQBox.Execute())
            bStart = false;
    }
    if (bStart)
        StartImport();
    return SUCCESS;
}

void DataStream::Edit( Window* pWindow, const Link& )
{
    DataStreamDlg aDialog(mpDocShell, pWindow);
    aDialog.Init(msURL, maStartRange, mnLimit, meMove, mnSettings);
    if (aDialog.Execute() == RET_OK)
    {
        bool bWasRunning = mbRunning;
        StopImport();
        aDialog.StartStream(this);
        if (bWasRunning)
            StartImport();
    }
}

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
