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
#include <osl/conditn.hxx>
#include <rtl/strbuf.hxx>
#include <salhelper/thread.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <asciiopt.hxx>
#include <dbfunc.hxx>
#include <docsh.hxx>
#include <documentimport.hxx>
#include <impex.hxx>
#include <rangelst.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>

#include <queue>

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

void DataStream::MakeToolbarVisible(ScDocShell *pShell)
{
    css::uno::Reference< css::frame::XFrame > xFrame =
        pShell->GetViewData()->GetViewShell()->GetViewFrame()->GetFrame().GetFrameInterface();
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
    sfx2::SvBaseLink *pLink = 0;
    pLink = new DataStream( pShell, rURL, rRange, nLimit, rMove, nSettings );
    sfx2::LinkManager* pLinkManager = pShell->GetDocument()->GetLinkManager();
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
    , mpRange(new ScRange())
    , mpEndRange(NULL)
{
    mxThread = new datastreams::CallerThread( this );
    mxThread->launch();

    Decode(rURL, rRange, rMove, nSettings);

    mpStartRange.reset( new ScRange(*mpRange.get()) );
    sal_Int32 nHeight = mpRange->aEnd.Row() - mpRange->aStart.Row() + 1;
    nLimit = nHeight * (nLimit / nHeight);
    if (nLimit && mpRange->aStart.Row() + nLimit - 1 < MAXROW)
    {
        mpEndRange.reset( new ScRange(*mpRange) );
        mpEndRange->Move(0, nLimit - nHeight, 0);
    }
}

DataStream::~DataStream()
{
    if (mbRunning)
        Stop();
    mxThread->mbTerminate = true;
    mxThread->maStart.set();
    mxThread->join();
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

void DataStream::Decode( const OUString& rURL, const OUString& rRange,
        const OUString& rMove, sal_uInt32 nSettings)
{
    SvStream *pStream = 0;
    if (nSettings & SCRIPT_STREAM)
        pStream = new SvScriptStream(rURL);
    else
        pStream = new SvFileStream(rURL, STREAM_READ);
    mxReaderThread = new datastreams::ReaderThread( pStream );
    mxReaderThread->launch();

    mbValuesInLine = nSettings & VALUES_IN_LINE;
    if (!mbValuesInLine)
        return;

    mpRange->Parse(rRange, mpScDocument);
    
    if (rMove == "NO_MOVE")
        meMove = NO_MOVE;
    else if (rMove == "RANGE_DOWN")
        meMove = RANGE_DOWN;
    else if (rMove == "MOVE_DOWN")
        meMove = MOVE_DOWN;
}

void DataStream::Start()
{
    if (mbRunning)
        return;
    mbIsUndoEnabled = mpScDocument->IsUndoEnabled();
    mpScDocument->EnableUndo(false);
    mbRunning = true;
    mxThread->maStart.set();
}

void DataStream::Stop()
{
    if (!mbRunning)
        return;
    mbRunning = false;
    mpScDocument->EnableUndo(mbIsUndoEnabled);
}

void DataStream::MoveData()
{
    switch (meMove)
    {
        case RANGE_DOWN:
            if (mpRange->aStart == mpEndRange->aStart)
                meMove = MOVE_UP;
            break;
        case MOVE_UP:
            mpScDocument->DeleteRow(*mpStartRange);
            mpScDocument->InsertRow(*mpEndRange);
            break;
        case MOVE_DOWN:
            if (mpEndRange.get())
                mpScDocument->DeleteRow(*mpEndRange);
            mpScDocument->InsertRow(*mpRange);
            break;
        case NO_MOVE:
            break;
    }
}

bool DataStream::ImportData()
{
    SolarMutexGuard aGuard;
    MoveData();
    if (mbValuesInLine)
    {
        SCROW nHeight = mpRange->aEnd.Row() - mpRange->aStart.Row() + 1;
        OStringBuffer aBuf;
        while (nHeight--)
        {
            aBuf.append(ConsumeLine());
            aBuf.append('\n');
        }
        SvMemoryStream aMemoryStream((void *)aBuf.getStr(), aBuf.getLength(), STREAM_READ);
        ScImportExport aImport(mpScDocument, *mpRange);
        aImport.SetSeparator(',');
        aImport.ImportStream(aMemoryStream, OUString(), FORMAT_STRING);
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
    // ImportStream calls PostPaint for relevant area,
    // we need to call it explicitly only when moving rows.
    if (meMove == NO_MOVE)
        return mbRunning;

    if (meMove == RANGE_DOWN)
    {
        mpRange->Move(0, mpRange->aEnd.Row() - mpRange->aStart.Row() + 1, 0);
        mpScDocShell->GetViewData()->GetView()->AlignToCursor(
                mpRange->aStart.Col(), mpRange->aStart.Row(), SC_FOLLOW_JUMP);
    }
    SCROW aEndRow = mpEndRange.get() ? mpEndRange->aEnd.Row() : MAXROW;
    mpScDocShell->PostPaint( ScRange( mpStartRange->aStart, ScAddress( mpRange->aEnd.Col(),
                    aEndRow, mpRange->aStart.Tab()) ), PAINT_GRID );

    return mbRunning;
}

sfx2::SvBaseLink::UpdateResult DataStream::DataChanged(
        const OUString& , const css::uno::Any& )
{
    MakeToolbarVisible(mpScDocShell);
    Start();
    return SUCCESS;
}

void DataStream::Edit(Window* , const Link& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
