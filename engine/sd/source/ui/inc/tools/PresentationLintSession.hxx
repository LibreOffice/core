/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/link.hxx>
#include <vcl/idle.hxx>

#include <tools/PresentationLint.hxx>

namespace tools
{
class JsonWriter;
}

namespace sd
{
class ViewShellBase;
}

namespace sd::lint
{
class LintMeasureCache;

/** One cleanup run of one view, spoken to in JSON.

    The client asks for a scan, for a cleanup, or for the list as it stands. It gets one reply for
    every request it made and one event for every step the run takes. Every message goes out as a
    result of the cleanup command, so it reaches the view whose session sent it.

    A scan collects the findings and then works out what each of them would save, a finding to a
    step. A cleanup works out what each of its findings needs the same way and then makes every
    change inside one undo entry.

    Working one finding out means encoding an image, which the session has to keep short. It runs in
    slices on the idle loop: a slice stops after a set time, or as soon as the client has sent
    something, and a turn of the scheduler carries on from there. So the document stays usable
    throughout, however many findings a deck holds.

    A row of the list is named on the wire by a small number the session hands out, and the numbers
    of one list are told from those of another by the run they belong to. Nothing that comes from
    the document is an identity on the wire.

    The list gathered after a cleanup is taken back or made again also waits for a turn of the
    scheduler. The notification arrives from inside the undo entry, while the changes it holds are
    still in place, so the list is gathered once the undo has finished.

    Known limitation with two views on one document: each view has a session of its own, and a
    cleanup one view makes is not reported to the other. So the second view goes on showing rows
    for findings that are no longer there. Those rows are safe to press, because every cleanup
    checks the document again before it changes anything and one with nothing left to change makes
    no change at all. The rows go once that view scans again. */
class LintSession final
{
public:
    explicit LintSession(ViewShellBase& rBase);
    ~LintSession();

    LintSession(const LintSession&) = delete;
    LintSession& operator=(const LintSession&) = delete;

    /** What is to become of the session once a request has been dealt with. */
    enum class Outcome
    {
        /** The session keeps the run, the list and the encodings it holds. */
        Keep,

        /** The client has asked for the session to go, with the list and the kept encodings. */
        Release
    };

    /** Deals with one request, which is a JSON object naming an action and carrying the number the
        answer to it is to be sent under. Exactly one reply goes out for it, and the events of
        whatever the request set going follow that reply. A request that cannot be read at all is
        answered as a bad request. */
    Outcome handleRequest(std::u16string_view aRequestJson);

private:
    /** Why the list the client is about to hear was gathered. */
    enum class ListReason
    {
        /** The client asked for a scan. */
        Scan,

        /** A cleanup was taken back or made again. */
        Undo,

        /** A cleanup reached beyond the findings it was about. */
        Fix,

        /** The client asked for the list as it stands. */
        Request
    };

    /** One row of the list as the client last heard it. */
    struct Row
    {
        std::shared_ptr<LintFinding> mpFinding;

        /** Bytes dealing with the finding was last said to free up. Zero says the row carries no
            figure, which is what a row whose finding has not been measured carries. */
        sal_uInt64 mnSavingBytes = 0;

        /** Which image of its slide the client was last told this row is. */
        sal_Int32 mnImageNumber = 0;

        /** True once a figure for the row has been worked out, and true from the start for a row
            whose finding has no figure to work out. */
        bool mbMeasured = false;
    };

    /** What one slice of the measurement worked out: the rows that now carry a figure and the rows
        the measurement took off the list. */
    struct MeasureBatch
    {
        std::vector<std::pair<sal_Int32, sal_uInt64>> maSavings;
        std::vector<sal_Int32> maDropped;
    };

    ViewShellBase& mrBase;

    /** The settings the list on the wire was gathered under, and the ones the cleanups of that
        list work to. They are read from the request that asks for a scan. */
    LintOptions maOptions;

    std::optional<PresentationLint> moLint;

    /** The encodings the measurements of earlier scans worked out, kept for as long as the session
        lives. A scan of a deck that was scanned before at the same settings reads its figures out
        of this. */
    std::shared_ptr<LintMeasureCache> mpMeasureCache;

    /** What a cleanup hands its undo entry, so that taking the cleanup back or making it again
        asks for the list afresh. The session is the only holder of it, so an entry left over from
        a cleanup this session started finds nothing to call once the session has gone. */
    std::shared_ptr<LintUndoNotifier> mpUndoNotifier;

    /** Which list the numbers on the wire belong to. It goes up with every list gathered from the
        document, so any smaller number names a list that no longer exists. */
    sal_Int32 mnRun = 0;

    /** The rows of the list under the numbers the client knows them by, in the order the client
        lists them. */
    std::map<sal_Int32, Row> maRows;

    /** The number the client knows each listed finding by. Every row of the list is named here by
        exactly one number. */
    std::unordered_map<const LintFinding*, sal_Int32> maRowIdByFinding;

    /** How many rows stand for a finding there is something to do about. */
    std::size_t mnFixableRowCount = 0;

    /** Waiting to gather the list again after a cleanup was taken back or made again. The
        notification arrives from inside the undo entry, before the changes it holds have been taken
        back, so the list is gathered on this turn instead, once the undo has finished. */
    Idle maRescanIdle;

    /** Waiting to carry on with the slice of work the run that is on stopped in the middle of. */
    Idle maStepIdle;

    /** The findings a cleanup that is under way is going to deal with. Nothing about the document
        has been changed while this list is being worked through. */
    LintWorkList maFixList;

    /** The number the request that asked for the cleanup under way came in under, which the
        message saying how that cleanup ended goes out under. */
    sal_Int32 mnFixRequestId = 0;

    /** True while a cleanup deals with the whole list rather than one row. */
    bool mbFixIsBatch = false;

    /** True from the request for a scan until the list and every figure in it have gone out. */
    bool mbScanning = false;

    /** True from the request for a cleanup until the changes have been made. */
    bool mbFixing = false;

    /** True when the list is to be gathered again once the run that is on has ended. */
    bool mbRebuildPending = false;

    /** Why the list the next scan sends was gathered. */
    ListReason meListReason = ListReason::Scan;

    /** True when the run that is on was cancelled rather than reaching its end. */
    bool mbStopped = false;

    /** Bytes the findings the rows stand for would free up between them. It follows the rows: a
        row that joins the list brings its figure in, a row that leaves takes it out again, and a
        figure that arrives for a row already there moves it by the difference. */
    sal_uInt64 mnEstimatedBytes = 0;

    /** Bytes the last cleanup of the whole list freed up. */
    sal_uInt64 mnBatchSavedBytes = 0;

    /** Bytes a cleanup freed up as the client last heard them for the list it is drawing. */
    sal_uInt64 mnShownSavedBytes = 0;

    /** The whole percent the last message about how far a run has come stood at, or -1 while no
        such message has gone out for the run. */
    sal_Int32 mnShownPercent = -1;

    /** True while the document may not be changed, or its content may not leave the machine, or
        this one view holds the document read-only. */
    bool isReadOnly() const;

    /** True while a scan or a cleanup is under way, which is when a request for another one is
        refused. */
    bool isBusy() const { return mbScanning || mbFixing; }

    /** The name of the state the session is in, as it goes out on the wire. */
    std::string_view getStatusName() const;

    /** Sends the one reply the given request gets. An empty reason says the request was
        accepted. */
    void sendReply(sal_Int32 nRequestId, std::string_view aReason);

    /** The size in bytes of the file the document was loaded from, or last saved to. Zero for a
        document that has never been saved, and for one whose file is not a plain file this machine
        can look at. */
    sal_uInt64 getDocumentBytes() const;

    /** Writes what the whole list comes to: what its rows would free up between them, what a
        cleanup of the whole list did free up, how many rows there are, how many of them there is
        something to do about and how big the file of the document is. */
    void putTotals(tools::JsonWriter& rJson) const;

    /** Writes the rows whose text has changed since the client last heard it, and takes the new
        text over as the one the client knows. */
    void putChangedNumbers(tools::JsonWriter& rJson);

    /** Sends the whole list: the state of the run, the settings it was gathered under, how far the
        measurement has come, every row and what they come to. */
    void sendList(ListReason eReason);

    /** Sends how far a run of nTotal steps has come, and sends nothing while that has not moved by
        a whole percent. */
    void sendProgress(std::string_view aPhase, std::size_t nDone, std::size_t nTotal);

    /** Sends the figures one slice of the measurement worked out, and says nothing when that slice
        worked out none. */
    void sendMeasured(const MeasureBatch& rBatch);

    /** Sends the message that the scan has settled, with the row texts the numbering changed. */
    void sendFinished();

    /** Sends the message that a cleanup has ended, with the rows it took off the list. */
    void sendFixed(const std::vector<sal_Int32>& rRemoved, sal_uInt64 nSavedBytes, bool bCancelled,
                   bool bRescan);

    /** Reads the settings a scan is to work to out of the options the request carried. A setting
        the request left out keeps the value it has. */
    void readOptions(std::optional<sal_Int32> oResolution, std::optional<sal_Int32> oQuality,
                     std::optional<bool> oForPublication);

    /** Asks for the list to be gathered again. A request that arrives while a run is on waits for
        the run to end. A list asked for because a cleanup was taken back or made again waits for a
        turn of the scheduler, so that it tells of the document the undo left behind. */
    void requestRebuild(ListReason eReason);

    /** Gathers the findings, sends the list they make and arms the turn that starts working out
        what they would save. The figures reach the client as the slices work them out. */
    void startScan(ListReason eReason);

    /** Cancels the run that is on. A session with no run going is left as it is. */
    void stopRun();

    /** Sends the message that the scan has settled and starts the scan a request that came in
        during the run asked for. */
    void finishScan();

    /** Starts the scan a request that came in during a run asked for, now that the run is over. */
    void runPendingRebuild();

    /** Hands a number to every finding of the scan, in the order the client lists them, and lets
        go of the numbers of the list before it. */
    void buildRowTable();

    /** Takes the row of the given finding off the list and answers the number the client knew it
        by, or -1 when no row stood for it. */
    sal_Int32 dropRow(const LintFinding* pFinding);

    /** Reads the figure of one row off its finding again and moves what the list comes to by what
        that changed. */
    void refreshRowSaving(Row& rRow);

    /** Drops whatever measurement work is still waiting. */
    void cancelMeasurement();

    /** Puts what one measurement worked out into the list and into the batch the client is about to
        hear. */
    void processMeasureStep(const LintMeasureStep& rStep, MeasureBatch& rBatch);

    /** Works out what the findings still waiting would save and sends the figures that came out. It
        works out as many of them as its slice of time allows and waits for a turn of the scheduler
        for the rest. Where nothing is left to measure this brings the list into the shape the
        reader sees and ends the scan. */
    void runMeasurement();

    /** Lines up a cleanup of the given findings and arms the turn that works out what each of them
        needs. The changes are made once every finding of the run has what it needs. */
    void startFixRun(std::vector<std::shared_ptr<LintFinding>> aFindings, bool bBatch,
                     sal_Int32 nRequestId);

    /** Works out what the cleanups still waiting need. It works out as many of them as its slice of
        time allows and waits for a turn of the scheduler for the rest. Once they all have what they
        need the changes are made. */
    void advanceFixRun();

    /** Makes every change the prepared cleanup calls for, all inside one undo entry. */
    void applyFixRun();

    /** Drops a cleanup that has not made its changes yet, and lets the session take requests
        again. */
    void cancelFixRun();

    DECL_LINK(RescanHdl, Timer*, void);
    DECL_LINK(StepHdl, Timer*, void);
};

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
