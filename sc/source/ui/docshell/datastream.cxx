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

#include <queue>

namespace datastreams {

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

DataStream* DataStream::Set(ScDocShell *pShell, const OUString& rURL, const OUString& rRange,
        sal_Int32 nLimit, const OUString& rMove, sal_uInt32 nSettings)
{
    // Each DataStream needs a destination area in order to be exported.
    // There can be only one ScAreaLink / DataStream per cell.
    // So - if we don't need range (DataStream with mbValuesInLine == false),
    // just find a free cell for now.
    sfx2::LinkManager* pLinkManager = pShell->GetDocument()->GetLinkManager();
    ScRange aDestArea;
    aDestArea.Parse(rRange, pShell->GetDocument());
    sal_uInt16 nLinkPos = 0;
    while (nLinkPos < pLinkManager->GetLinks().size())
    {
        sfx2::SvBaseLink* pBase = *pLinkManager->GetLinks()[nLinkPos];
        if (rRange.isEmpty())
        {
            if ( (pBase->ISA(ScAreaLink) && dynamic_cast<ScAreaLink*>
                        (&(*pBase))->GetDestArea().aStart == aDestArea.aStart)
                || (pBase->ISA(DataStream) && dynamic_cast<DataStream*>
                        (&(*pBase))->GetRange().aStart == aDestArea.aStart) )
            {
                aDestArea.Move(0, 1, 0);
                nLinkPos = 0;
                continue;
            }
            else
                ++nLinkPos;
        }
        else if ( (pBase->ISA(ScAreaLink) && dynamic_cast<ScAreaLink*>
                    (&(*pBase))->GetDestArea().aStart == aDestArea.aStart)
                || (pBase->ISA(DataStream) && dynamic_cast<DataStream*>
                    (&(*pBase))->GetRange().aStart == aDestArea.aStart) )
        {
            pLinkManager->Remove( pBase );
        }
        else
            ++nLinkPos;
    }

    sfx2::SvBaseLink *pLink = 0;
    pLink = new DataStream( pShell, rURL, rRange, nLimit, rMove, nSettings );
    pLinkManager->InsertFileLink( *pLink, OBJECT_CLIENT_FILE, rURL, NULL, NULL );
    return dynamic_cast<DataStream*>(pLink);
}

DataStream::DataStream(ScDocShell *pShell, const OUString& rURL, const OUString& rRange,
        sal_Int32 nLimit, const OUString& rMove, sal_uInt32 nSettings)
    : mpScDocShell(pShell)
    , mpScDocument(mpScDocShell->GetDocument())
    , meMove(NO_MOVE)
    , mbRunning(false)
    , mpLines(0)
    , mnLinesCount(0)
{
    SetRefreshHandler(LINK( this, DataStream, RefreshHdl ));
    SetRefreshControl(mpScDocument->GetRefreshTimerControlAddress());
    SetTimeout( 1 );
    Decode(rURL, rRange, nLimit, rMove, nSettings);
}

DataStream::~DataStream()
{
    if (mbRunning)
        StopImport();
    if (mxReaderThread.is())
        mxReaderThread->endThread();
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

void DataStream::Decode(const OUString& rURL, const OUString& rRange,
        sal_Int32 nLimit, const OUString& rMove, const sal_uInt32 nSettings)
{
    msURL = rURL;
    msRange = rRange;
    mnLimit = nLimit;
    msMove = rMove;
    mnSettings = nSettings;
    mpEndRange.reset( NULL );

    mbValuesInLine = mnSettings & VALUES_IN_LINE;

    if (msMove == "NO_MOVE")
        meMove = NO_MOVE;
    else if (msMove == "RANGE_DOWN")
        meMove = RANGE_DOWN;
    else if (msMove == "MOVE_DOWN")
        meMove = MOVE_DOWN;

    maRange.Parse(msRange);
    maStartRange = maRange;
    sal_Int32 nHeight = maRange.aEnd.Row() - maRange.aStart.Row() + 1;
    nLimit = nHeight * (nLimit / nHeight);
    if (nLimit && maRange.aStart.Row() + nLimit - 1 < MAXROW)
    {
        mpEndRange.reset( new ScRange(maRange) );
        mpEndRange->Move(0, nLimit - nHeight, 0);
    }
}

void DataStream::StartImport()
{
    if (mbRunning)
        return;
    mbIsUndoEnabled = mpScDocument->IsUndoEnabled();
    mpScDocument->EnableUndo(false);
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
    AutoTimer::Start();
}

void DataStream::StopImport()
{
    if (!mbRunning)
        return;
    mbRunning = false;
    AutoTimer::Stop();
    mpScDocument->EnableUndo(mbIsUndoEnabled);
}

void DataStream::MoveData()
{
    switch (meMove)
    {
        case RANGE_DOWN:
            if (maRange.aStart == mpEndRange->aStart)
                meMove = MOVE_UP;
            break;
        case MOVE_UP:
            mpScDocument->DeleteRow(maStartRange);
            mpScDocument->InsertRow(*mpEndRange);
            break;
        case MOVE_DOWN:
            if (mpEndRange.get())
                mpScDocument->DeleteRow(*mpEndRange);
            mpScDocument->InsertRow(maRange);
            break;
        case NO_MOVE:
            break;
    }
}

IMPL_LINK_NOARG(DataStream, RefreshHdl)
{
    ImportData();
    return 0;
}

//  lcl_ScanString and Text2Doc is simplified version
//  of code from sc/source/ui/docshell/impex.cxx
const sal_Unicode* lcl_ScanString( const sal_Unicode* p, OUString& rString, sal_Unicode cStr)
{
    const sal_Unicode* p0 = p;
    for( ;; )
    {
        if (!*p)
            break;
        if (*p == cStr)
        {
            if (*++p != cStr)
                break;
            p++;
        }
        else
            p++;
    }
    if (p0 < p)
        if (rString.getLength() + (p - p0) <= STRING_MAXLEN)
            rString += OUString( p0, sal::static_int_cast<sal_Int32>( p - p0 ) );
    return p;
}

void DataStream::Text2Doc()
{
    sal_Unicode cSep(',');
    sal_Unicode cStr('"');
    SCCOL nStartCol = maRange.aStart.Col();
    SCROW nStartRow = maRange.aStart.Row();
    SCCOL nEndCol = maRange.aEnd.Col();
    SCROW nEndRow = maRange.aEnd.Row();
    OUString aCell;
    SCROW nRow = nStartRow;
    ScDocumentImport aDocImport(*mpScDocument);
    while (nRow <= nEndRow)
    {
        SCCOL nCol = nStartCol;
        OUString sLine( OStringToOUString(ConsumeLine(), RTL_TEXTENCODING_UTF8) );
        const sal_Unicode* p = sLine.getStr();
        while (*p)
        {
            aCell = "";
            const sal_Unicode* q = p;
            while (*p && *p != cSep)
            {
                // Always look for a pairing quote and ignore separator in between.
                while (*p && *p == cStr)
                    q = p = lcl_ScanString(p, aCell, cStr);
                // All until next separator or quote.
                while (*p && *p != cSep && *p != cStr)
                    ++p;
                if (aCell.getLength() + (p - q) <= STRING_MAXLEN)
                    aCell += OUString( q, sal::static_int_cast<sal_Int32>( p - q ) );
                q = p;
            }
            if (*p)
                ++p;
            if (nCol <= nEndCol && nRow <= nEndRow)
            {
                ScAddress aAddress(nCol, nRow, maRange.aStart.Tab());
                if (aCell == "0" || ( aCell.indexOf(':') == -1 && aCell.toDouble() ))
                    aDocImport.setNumericCell(aAddress, aCell.toDouble());
                else
                    aDocImport.setStringCell(aAddress, aCell);
            }
            ++nCol;
        }
        ++nRow;
    }
    aDocImport.finalize();
    mpScDocShell->PostPaint( maRange, PAINT_GRID );
}

bool DataStream::ImportData()
{
    MoveData();
    if (mbValuesInLine)
    {
        // do CSV import
        Text2Doc();
    }
    else
    {
        ScRangeList aRangeList;
        ScDocumentImport aDocImport(*mpScDocument);
        // read more lines at once but not too much
        for (int i = 0; i < 10; ++i)
        {
            OUString sLine( OStringToOUString(ConsumeLine(), RTL_TEXTENCODING_UTF8) );
            if (sLine.indexOf(',') <= 0)
                continue;

            OUString sAddress( sLine.copy(0, sLine.indexOf(',')) );
            OUString sValue( sLine.copy(sLine.indexOf(',') + 1) );
            ScAddress aAddress;
            aAddress.Parse(sAddress, mpScDocument);
            if (!aAddress.IsValid())
                continue;

            if (sValue == "0" || ( sValue.indexOf(':') == -1 && sValue.toDouble() ))
                aDocImport.setNumericCell(aAddress, sValue.toDouble());
            else
                aDocImport.setStringCell(aAddress, sValue);
            aRangeList.Join(aAddress);
        }
        aDocImport.finalize();
        mpScDocShell->PostPaint( aRangeList, PAINT_GRID );
    }
    if (meMove == NO_MOVE)
        return mbRunning;

    if (meMove == RANGE_DOWN)
    {
        maRange.Move(0, maRange.aEnd.Row() - maRange.aStart.Row() + 1, 0);
        mpScDocShell->GetViewData()->GetView()->AlignToCursor(
                maRange.aStart.Col(), maRange.aStart.Row(), SC_FOLLOW_JUMP);
    }
    SCROW aEndRow = mpEndRange.get() ? mpEndRange->aEnd.Row() : MAXROW;
    mpScDocShell->PostPaint( ScRange( maStartRange.aStart, ScAddress( maRange.aEnd.Col(),
                    aEndRow, maRange.aStart.Tab()) ), PAINT_GRID );

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

void DataStream::Edit(Window* pWindow, const Link& )
{
    DataStreamDlg aDialog(mpScDocShell, pWindow);
    aDialog.Init(msURL, msRange, mnLimit, msMove, mnSettings);
    if (aDialog.Execute() == RET_OK)
    {
        bool bWasRunning = mbRunning;
        StopImport();
        aDialog.StartStream(this);
        if (bWasRunning)
            StartImport();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
