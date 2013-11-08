/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <datastreams.hxx>

#include <osl/conditn.hxx>
#include <rtl/strbuf.hxx>
#include <salhelper/thread.hxx>
#include <asciiopt.hxx>
#include <docsh.hxx>
#include <impex.hxx>

namespace datastreams {

class CallerThread : public salhelper::Thread
{
    DataStreams *mpDataStreams;
public:
    osl::Condition maStart;
    bool mbTerminate;

    CallerThread(DataStreams *pData):
        Thread("CallerThread")
        ,mpDataStreams(pData)
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
                while (mpDataStreams->ImportData())
                    wait(aTime);
        };
    }
};

}

DataStreams::DataStreams(ScDocShell *pScDocShell):
    mpScDocShell(pScDocShell)
    , mpScDocument(mpScDocShell->GetDocument())
    , mbMove(false)
{
    mxThread = new datastreams::CallerThread( this );
    mxThread->launch();
}

DataStreams::~DataStreams()
{
    if (mbRunning)
        Stop();
    mxThread->mbTerminate = true;
    mxThread->maStart.set();
    mxThread->join();
}

void DataStreams::Start()
{
    mbIsUndoEnabled = mpScDocument->IsUndoEnabled();
    mpScDocument->EnableUndo(false);
    mbRunning = true;
    mxThread->maStart.set();
}

void DataStreams::Stop()
{
    mbRunning = false;
    mpScDocument->EnableUndo(mbIsUndoEnabled);
    mbMove = false;
}

void DataStreams::Set(const OUString& rUrl, bool bIsScript, const OUString& rRange)
{
    mpRange.reset ( new ScRange() );
    mpRange->Parse(rRange, mpScDocument);
    if (bIsScript)
        mpStream.reset( new SvScriptStream(rUrl) );
    else
        mpStream.reset( new SvFileStream(rUrl, STREAM_READ) );
}

void DataStreams::SetMove(sal_Int32 nLimit)
{
    mpEndRange.reset( NULL );
    mbMove = true;
    sal_Int32 nHeight = mpRange->aEnd.Row() - mpRange->aStart.Row() + 1;
    nLimit = nHeight * (nLimit / nHeight);
    if (nLimit && mpRange->aStart.Row() + nLimit - 1 < MAXROW)
    {
        mpEndRange.reset( new ScRange(*mpRange) );
        mpEndRange->Move(0, nLimit - nHeight, 0);
    }
}

void DataStreams::Move()
{
    if (!mbMove)
        return;
    if (mpEndRange.get())
    {
        mpScDocument->DeleteRow(*mpEndRange);
    }
    mpScDocument->InsertRow(*mpRange);
}

bool DataStreams::ImportData()
{
    if (!mpStream->good())
    {
        // if there is a problem with SvStream, stop running
        mbRunning = false;
        return mbRunning;
    }

    SolarMutexGuard aGuard;
    Move();
    SCROW nHeight = mpRange->aEnd.Row() - mpRange->aStart.Row() + 1;
    OStringBuffer aBuf;
    OString sTmp;
    while (nHeight--)
    {
        mpStream->ReadLine(sTmp);
        aBuf.append(sTmp);
        aBuf.append('\n');
    }
    SvMemoryStream aMemoryStream((void *)aBuf.getStr(), aBuf.getLength(), STREAM_READ);
    ScImportExport aImport(mpScDocument, *mpRange);
    aImport.SetSeparator(',');
    aImport.ImportStream(aMemoryStream, OUString(), FORMAT_STRING);
    // ImportStream calls PostPaint for relevant area,
    // we need to call it explicitly only when moving rows.
    if (!mbMove)
        return mbRunning;

    SCROW aEndRow = mpEndRange.get() ? mpEndRange->aEnd.Row() : MAXROW;
    mpScDocShell->PostPaint( ScRange( mpRange->aStart, ScAddress( mpRange->aEnd.Col(),
                    aEndRow, mpRange->aStart.Tab()) ), PAINT_GRID );

    return mbRunning;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
