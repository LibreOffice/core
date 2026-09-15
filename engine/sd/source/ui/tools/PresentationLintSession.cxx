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

#include <tools/PresentationLintSession.hxx>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <COKit/COKit.hxx>
#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <tools/json_writer.hxx>
#include <vcl/TaskStopwatch.hxx>

#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <tools/LintMeasureCache.hxx>

namespace sd::lint
{
namespace
{
/** The command every request arrives under and every answer goes out under. */
constexpr std::string_view gaCommandName{ ".uno:PresentationCleanup" };

/** True when the person at the other end has sent something that is still waiting to be read.

    The stopwatch of a stepped task asks Application::AnyInput, and the headless backend the kit
    runs on answers that with a plain no, so a slice has to ask the client itself. */
bool clientIsWaiting() { return comphelper::COKit::isActive() && comphelper::COKit::anyInput(); }

/** The name the client knows a kind of problem by. */
std::string_view getCategoryName(LintCategory eCategory)
{
    switch (eCategory)
    {
        case LintCategory::CroppedImage:
            return "croppedImage";
        case LintCategory::HiddenSlide:
            return "hiddenSlide";
        case LintCategory::UnusedMaster:
            return "unusedMaster";
        case LintCategory::NotesContent:
            return "notes";
        case LintCategory::OleObject:
            return "embeddedObject";
        case LintCategory::LargeImage:
            break;
    }

    return "image";
}

/** Sends one message to the client of the given view. The fields the given function writes make up
    the object the client reads the answer of a command out of. */
void sendResult(const ViewShellBase& rBase,
                const std::function<void(tools::JsonWriter&)>& rWriteFields)
{
    tools::JsonWriter aJson;
    aJson.put("commandName", gaCommandName);
    aJson.put("success", true);
    {
        auto aResultNode = aJson.startNode("result");
        rWriteFields(aJson);
    }

    rBase.viewCallback(COKitCallbackType::UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());
}

} // end of anonymous namespace

LintSession::LintSession(ViewShellBase& rBase)
    : mrBase(rBase)
    , mpMeasureCache(std::make_shared<LintMeasureCache>())
    , mpUndoNotifier(
          std::make_shared<LintUndoNotifier>([this] { requestRebuild(ListReason::Undo); }))
    , maRescanIdle("sd::lint::LintSession maRescanIdle")
    , maStepIdle("sd::lint::LintSession maStepIdle")
{
    // Both turns run at the lowest priority the scheduler has, well below the one the queued
    // updates of the view go out at. So the undo a rescan follows has been drawn, and the client
    // has been heard again, before the document is read afresh, and a run that has more work to do
    // lets everything else of this turn happen first. The lowest priority is also where the other
    // background work of the module waits, such as the spelling that runs while the document sits
    // there, so a long run shares the time with it.
    maRescanIdle.SetPriority(TaskPriority::LOWEST);
    maRescanIdle.SetInvokeHandler(LINK(this, LintSession, RescanHdl));
    maStepIdle.SetPriority(TaskPriority::LOWEST);
    maStepIdle.SetInvokeHandler(LINK(this, LintSession, StepHdl));
}

LintSession::~LintSession()
{
    // A turn that is still waiting would work on a list that is about to go, so both are dropped
    // first.
    maRescanIdle.Stop();
    maStepIdle.Stop();
    maFixList.clear();
    mbFixing = false;
    mbScanning = false;

    // The rows hold the findings they stand for, so they let go of them before the lint that
    // gathered them goes.
    maRows.clear();
    maRowIdByFinding.clear();
    moLint.reset();

    // The encodings the measurements kept are of use only to a scan of this session.
    if (mpMeasureCache)
    {
        mpMeasureCache->clear();
        mpMeasureCache.reset();
    }

    // The undo entries of the cleanups this session started outlive it, and letting go of the
    // notifier here is what leaves them with nobody to ask for a list.
    mpUndoNotifier.reset();
}

bool LintSession::isReadOnly() const
{
    // The session offers cleanups that change the document, so there is nothing it may do while the
    // document may not be changed or its content may not leave the machine. A document that is
    // writable can still be held open read-only by this one view, so the view is asked as well as
    // the document.
    DrawDocShell* pDocShell = mrBase.GetDocShell();
    return !pDocShell || pDocShell->IsReadOnly() || pDocShell->isExportLocked()
           || mrBase.IsKitReadOnlyView();
}

std::string_view LintSession::getStatusName() const
{
    // A cleanup whose whole list is worked out is about to make its changes, and until then it is
    // still working out what they are.
    if (mbFixing)
        return maFixList.hasPendingWork() ? "preparing" : "cleaning";

    // The findings are gathered inside the request that asks for them, so a scan the client can ask
    // about is one that is still waiting for the figures of its images.
    if (mbScanning && moLint && moLint->hasPendingMeasurement())
        return "measuring";

    return "idle";
}

LintSession::Outcome LintSession::handleRequest(std::u16string_view aRequestJson)
{
    boost::property_tree::ptree aTree;
    std::stringstream aStream(
        (std::string(OUStringToOString(aRequestJson, RTL_TEXTENCODING_UTF8))));
    try
    {
        boost::property_tree::read_json(aStream, aTree);
    }
    catch (const std::exception&)
    {
        // A request that cannot be read carries no number of its own, so the refusal goes out under
        // the number nothing the client sends can carry.
        sendReply(0, "badRequest");
        return Outcome::Keep;
    }

    const std::string aAction = aTree.get("action", std::string());
    const sal_Int32 nRequestId = aTree.get("request", sal_Int32(0));

    if (aAction == "release")
    {
        sendReply(nRequestId, {});
        return Outcome::Release;
    }

    if (aAction == "stop")
    {
        sendReply(nRequestId, {});
        stopRun();
        return Outcome::Keep;
    }

    if (aAction == "list")
    {
        sendReply(nRequestId, {});
        sendList(ListReason::Request);
        return Outcome::Keep;
    }

    if (aAction == "scan")
    {
        if (isBusy())
        {
            sendReply(nRequestId, "busy");
            return Outcome::Keep;
        }

        if (isReadOnly())
        {
            sendReply(nRequestId, "readOnly");
            return Outcome::Keep;
        }

        std::optional<sal_Int32> oResolution;
        std::optional<sal_Int32> oQuality;
        std::optional<bool> oForPublication;
        if (const auto oOptions = aTree.get_child_optional("options"))
        {
            if (const auto oValue = oOptions->get_optional<sal_Int32>("resolution"))
                oResolution = *oValue;
            if (const auto oValue = oOptions->get_optional<sal_Int32>("quality"))
                oQuality = *oValue;
            if (const auto oValue = oOptions->get_optional<bool>("forPublication"))
                oForPublication = *oValue;
        }

        readOptions(oResolution, oQuality, oForPublication);

        sendReply(nRequestId, {});
        startScan(ListReason::Scan);
        return Outcome::Keep;
    }

    if (aAction == "fix" || aAction == "goTo")
    {
        const bool bIsFix = aAction == "fix";
        if (bIsFix && isBusy())
        {
            sendReply(nRequestId, "busy");
            return Outcome::Keep;
        }

        if (bIsFix && isReadOnly())
        {
            sendReply(nRequestId, "readOnly");
            return Outcome::Keep;
        }

        const auto oRun = aTree.get_optional<sal_Int32>("run");
        const auto oRow = aTree.get_optional<sal_Int32>("row");
        if (!oRun || !oRow)
        {
            sendReply(nRequestId, "badRequest");
            return Outcome::Keep;
        }

        if (*oRun != mnRun)
        {
            sendReply(nRequestId, "stale");
            return Outcome::Keep;
        }

        const auto aFound = maRows.find(*oRow);

        // A row there is nothing to do about reads for a cleanup as a row that is no longer
        // there, so a cleanup of it is refused.
        if (aFound == maRows.end() || (bIsFix && !aFound->second.mpFinding->canFix()))
        {
            sendReply(nRequestId, "gone");
            return Outcome::Keep;
        }

        sendReply(nRequestId, {});

        if (bIsFix)
            startFixRun({ aFound->second.mpFinding }, false, nRequestId);
        else
            aFound->second.mpFinding->goTo(mrBase);

        return Outcome::Keep;
    }

    if (aAction == "fixAll")
    {
        if (isBusy())
        {
            sendReply(nRequestId, "busy");
            return Outcome::Keep;
        }

        if (isReadOnly())
        {
            sendReply(nRequestId, "readOnly");
            return Outcome::Keep;
        }

        const auto oRun = aTree.get_optional<sal_Int32>("run");
        if (!oRun)
        {
            sendReply(nRequestId, "badRequest");
            return Outcome::Keep;
        }

        if (*oRun != mnRun)
        {
            sendReply(nRequestId, "stale");
            return Outcome::Keep;
        }

        std::vector<std::shared_ptr<LintFinding>> aFindings;
        for (const auto& rEntry : maRows)
        {
            if (rEntry.second.mpFinding->canFix())
                aFindings.push_back(rEntry.second.mpFinding);
        }

        sendReply(nRequestId, {});

        if (aFindings.empty())
        {
            // There is nothing on the list to clean up, so the cleanup ends where it began and the
            // client hears as much.
            mbFixIsBatch = true;
            mnFixRequestId = nRequestId;
            sendFixed({}, 0, false, false);
            return Outcome::Keep;
        }

        startFixRun(std::move(aFindings), true, nRequestId);
        return Outcome::Keep;
    }

    sendReply(nRequestId, "badRequest");
    return Outcome::Keep;
}

void LintSession::readOptions(std::optional<sal_Int32> oResolution,
                              std::optional<sal_Int32> oQuality,
                              std::optional<bool> oForPublication)
{
    // The target the images are measured against is a resolution in dots per inch, and zero says
    // the images are left alone whatever they are stored at. So a figure below zero is read as
    // zero.
    if (oResolution)
        maOptions.mnImageResolution = std::max(sal_Int32(0), *oResolution);

    // The encoder takes a quality from 1 to 100, and a figure outside that is read as the nearest
    // one inside it.
    if (oQuality)
        maOptions.mnJPEGQuality = std::clamp(*oQuality, sal_Int32(1), sal_Int32(100));

    if (oForPublication)
        maOptions.mbForPublication = *oForPublication;
}

void LintSession::sendReply(sal_Int32 nRequestId, std::string_view aReason)
{
    sendResult(mrBase,
               [nRequestId, aReason](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "reply");
                   rJson.put("request", nRequestId);
                   rJson.put("ok", aReason.empty());
                   if (!aReason.empty())
                       rJson.put("reason", aReason);
               });
}

sal_uInt64 LintSession::getDocumentBytes() const
{
    const DrawDocShell* pDocShell = mrBase.GetDocShell();
    if (!pDocShell)
        return 0;

    const SfxMedium* pMedium = pDocShell->GetMedium();
    if (!pMedium)
        return 0;

    // The file is asked about by name, which costs one look at the file system. It is asked afresh
    // every time a total goes out, so a document saved in between is reported at the size it now
    // has. A document that lives anywhere other than a plain file this machine can look at answers
    // nothing here.
    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(pMedium->GetName(), aItem) != osl::FileBase::E_None)
        return 0;

    osl::FileStatus aStatus(osl_FileStatus_Mask_FileSize);
    if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None
        || !aStatus.isValid(osl_FileStatus_Mask_FileSize))
        return 0;

    return aStatus.getFileSize();
}

void LintSession::putTotals(tools::JsonWriter& rJson) const
{
    auto aTotalNode = rJson.startNode("total");
    rJson.put("estimated", mnEstimatedBytes);
    rJson.put("saved", mnShownSavedBytes);
    rJson.put("count", sal_Int32(maRows.size()));
    rJson.put("fixable", sal_Int32(mnFixableRowCount));
    rJson.put("documentBytes", getDocumentBytes());
}

void LintSession::putChangedNumbers(tools::JsonWriter& rJson)
{
    auto aRowsArray = rJson.startArray("rows");
    for (auto& rEntry : maRows)
    {
        // Taking a row off the list renumbers the images that share a slide with it, and that
        // number is the only figure of a row that a cleanup somewhere else can change.
        const sal_Int32 nImageNumber = rEntry.second.mpFinding->getFacts().mnImageNumber;
        if (nImageNumber == rEntry.second.mnImageNumber)
            continue;

        rEntry.second.mnImageNumber = nImageNumber;

        auto aRowNode = rJson.startStruct();
        rJson.put("id", rEntry.first);
        rJson.put("imageNumber", nImageNumber);
    }
}

void LintSession::sendList(ListReason eReason)
{
    const std::string_view aReason = [eReason]() -> std::string_view
    {
        switch (eReason)
        {
            case ListReason::Undo:
                return "undo";
            case ListReason::Fix:
                return "fix";
            case ListReason::Request:
                return "request";
            case ListReason::Scan:
                break;
        }
        return "scan";
    }();

    sendResult(mrBase,
               [this, aReason](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "list");
                   rJson.put("run", mnRun);
                   rJson.put("reason", aReason);
                   rJson.put("status", getStatusName());
                   {
                       auto aOptionsNode = rJson.startNode("options");
                       rJson.put("resolution", maOptions.mnImageResolution);
                       rJson.put("quality", maOptions.mnJPEGQuality);
                       rJson.put("forPublication", maOptions.mbForPublication);
                   }
                   {
                       // The kinds of problem the last scan looked for, whether or not it found
                       // any of them. A session that has not scanned yet names none.
                       auto aScannedArray = rJson.startArray("scanned");
                       if (mnRun != 0)
                       {
                           for (const LintCategory eCategory : getScannedCategories(maOptions))
                               rJson.putSimpleValue(
                                   OUString::createFromAscii(getCategoryName(eCategory)));
                       }
                   }
                   {
                       auto aMeasuredNode = rJson.startNode("measured");
                       rJson.put("done", sal_Int32(moLint ? moLint->getMeasuredCount() : 0));
                       rJson.put("total", sal_Int32(moLint ? moLint->getMeasureCount() : 0));
                   }
                   {
                       auto aRowsArray = rJson.startArray("rows");
                       for (auto& rEntry : maRows)
                       {
                           const LintFinding& rFinding = *rEntry.second.mpFinding;
                           const LintFindingFacts& rFacts = rFinding.getFacts();
                           rEntry.second.mnImageNumber = rFacts.mnImageNumber;

                           auto aRowNode = rJson.startStruct();
                           rJson.put("id", rEntry.first);
                           rJson.put("category", getCategoryName(rFinding.getCategory()));
                           rJson.put("slide", rFinding.getSlideIndex());
                           rJson.put("imageNumber", rFacts.mnImageNumber);
                           rJson.put("dpi", rFacts.mnEffectiveDPI);
                           rJson.put("hiddenPercent", rFacts.mnHiddenPercent);
                           rJson.put("name", rFacts.maName);
                           rJson.put("currentBytes", rFinding.getCurrentBytes());
                           rJson.put("saving", rEntry.second.mnSavingBytes);
                           rJson.put("measured", rEntry.second.mbMeasured);
                           rJson.put("canGoTo", rFinding.canGoto());
                           rJson.put("canFix", rFinding.canFix());
                       }
                   }
                   putTotals(rJson);
               });
}

void LintSession::sendProgress(std::string_view aPhase, std::size_t nDone, std::size_t nTotal)
{
    const sal_Int32 nPercent = nTotal == 0 ? 0 : sal_Int32(std::min(nDone, nTotal) * 100 / nTotal);

    // A run of a hundred findings moves the bar the client draws by a hundredth of its width at a
    // time. The message goes out when the whole percent it stands at changes, so a whole run costs
    // a hundred of these at the most.
    if (nPercent == mnShownPercent)
        return;

    mnShownPercent = nPercent;

    sendResult(mrBase,
               [this, aPhase, nDone, nTotal](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "progress");
                   rJson.put("run", mnRun);
                   rJson.put("phase", aPhase);
                   rJson.put("done", sal_Int32(nDone));
                   rJson.put("total", sal_Int32(nTotal));
               });
}

void LintSession::sendMeasured(const MeasureBatch& rBatch)
{
    // A batch that worked nothing out leaves the client with nothing to change.
    if (rBatch.maSavings.empty() && rBatch.maDropped.empty())
        return;

    sendResult(mrBase,
               [this, &rBatch](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "measured");
                   rJson.put("run", mnRun);
                   {
                       auto aRowsArray = rJson.startArray("rows");
                       for (const auto& rSaving : rBatch.maSavings)
                       {
                           auto aRowNode = rJson.startStruct();
                           rJson.put("id", rSaving.first);
                           rJson.put("saving", rSaving.second);
                       }
                   }
                   {
                       auto aDroppedArray = rJson.startArray("dropped");
                       for (const sal_Int32 nId : rBatch.maDropped)
                           rJson.putSimpleValue(nId);
                   }
                   {
                       auto aTotalNode = rJson.startNode("total");
                       rJson.put("estimated", mnEstimatedBytes);
                       rJson.put("documentBytes", getDocumentBytes());
                   }
               });
}

void LintSession::sendFinished()
{
    sendResult(mrBase,
               [this](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "finished");
                   rJson.put("run", mnRun);
                   rJson.put("stopped", mbStopped);

                   // The numbering of the rows of one slide is worked out again once the
                   // measurement has decided what stays, so a slide that lost one of two image
                   // rows reads differently from here on.
                   putChangedNumbers(rJson);
                   putTotals(rJson);
               });
}

void LintSession::sendFixed(const std::vector<sal_Int32>& rRemoved, sal_uInt64 nSavedBytes,
                            bool bCancelled, bool bRescan)
{
    sendResult(mrBase,
               [this, &rRemoved, nSavedBytes, bCancelled, bRescan](tools::JsonWriter& rJson)
               {
                   rJson.put("event", "fixed");
                   rJson.put("run", mnRun);
                   rJson.put("request", mnFixRequestId);
                   rJson.put("cancelled", bCancelled);
                   rJson.put("rescan", bRescan);

                   // What a cleanup of the whole list freed up is the figure the client shows in
                   // place of the estimate, and a cleanup of a single row is not reported that way.
                   rJson.put("saved", mbFixIsBatch ? nSavedBytes : sal_uInt64(0));
                   {
                       auto aRemovedArray = rJson.startArray("removed");
                       for (const sal_Int32 nId : rRemoved)
                           rJson.putSimpleValue(nId);
                   }

                   putChangedNumbers(rJson);
                   putTotals(rJson);
               });
}

void LintSession::requestRebuild(ListReason eReason)
{
    // A request that arrives while a run is on waits for the run to end. The list the run is
    // working on is about a document that has since moved on, so it is gathered again as soon as
    // the run is over.
    if (isBusy())
    {
        mbRebuildPending = true;
        meListReason = eReason;
        return;
    }

    // A cleanup that was taken back or made again says so from inside its own undo entry, and on an
    // undo it says so before the changes it holds have been taken back. So its list waits for a
    // turn of the scheduler, by which time the document is the one the reader is looking at.
    if (eReason == ListReason::Undo)
    {
        maRescanIdle.Start();
        return;
    }

    startScan(eReason);
}

void LintSession::startScan(ListReason eReason)
{
    // A scan starts only while no run is on: a request for one is refused as busy, and a rescan
    // asked for during a run waits for the run to end. The turns that would carry a run on are
    // stopped all the same, because what they would work on is the list that is being thrown away.
    maRescanIdle.Stop();
    maStepIdle.Stop();
    cancelMeasurement();

    mbRebuildPending = false;
    meListReason = eReason;
    mbStopped = false;
    mbScanning = true;

    // A run counts its steps from the first one, so the session lets go of how far the run before
    // this one had come.
    mnShownPercent = -1;

    SdDrawDocument* pDocument = mrBase.GetDocument();
    if (!pDocument)
    {
        finishScan();
        return;
    }

    // Build the lint again, so a changed setting decides what the whole list holds. Only the list
    // itself is gathered here, and the figures reach the client as the slices work them out.
    moLint.emplace(*pDocument, maOptions, mpMeasureCache);
    moLint->scanStructure();
    buildRowTable();

    // A cleanup of the whole list reports what it freed up, once. The figure is taken over here so
    // that every later list goes back to the estimate for what is still left to do.
    mnShownSavedBytes = mnBatchSavedBytes;
    mnBatchSavedBytes = 0;

    sendList(meListReason);
    meListReason = ListReason::Scan;

    // The list goes out before a single image is encoded, because the first slice waits for a turn
    // of the scheduler.
    maStepIdle.Start();
}

void LintSession::stopRun()
{
    // The client asked for the run to end, so the list stays as it is until the next request. A
    // rescan that was asked for goes the same way.
    mbRebuildPending = false;
    maRescanIdle.Stop();
    maStepIdle.Stop();

    // A cleanup changes nothing until every one of its findings has what it needs, so ending it
    // before that leaves the document alone and the list as it was.
    if (mbFixing)
    {
        cancelFixRun();
        return;
    }

    if (!mbScanning)
        return;

    mbStopped = true;

    // The findings still waiting stay on the list without a figure, and the numbering is worked out
    // over what is left, so the list reads the same as it would have had every one been measured.
    cancelMeasurement();
    finishScan();
}

void LintSession::finishScan()
{
    mbScanning = false;

    sendFinished();
    runPendingRebuild();
}

void LintSession::runPendingRebuild()
{
    // A request for the list that came in during the run is answered now that the run is over.
    if (mbRebuildPending && mrBase.GetDocument())
        startScan(meListReason);
}

void LintSession::buildRowTable()
{
    maRows.clear();
    maRowIdByFinding.clear();
    mnFixableRowCount = 0;
    mnEstimatedBytes = 0;

    // The numbers of this list are told from those of the list before it by the run they belong to,
    // so a request that names an older list is refused.
    ++mnRun;

    sal_Int32 nRowId = 0;
    for (const std::shared_ptr<LintFinding>& pFinding : moLint->getFindings())
    {
        Row aRow;
        aRow.mpFinding = pFinding;
        aRow.mnImageNumber = pFinding->getFacts().mnImageNumber;

        // A row starts without a figure. A finding that has no figure to work out counts as
        // measured from the start, and the rest are measured by the steps that follow.
        aRow.mbMeasured = !pFinding->needsMeasuring();

        if (pFinding->canFix())
            ++mnFixableRowCount;

        ++nRowId;
        maRowIdByFinding[pFinding.get()] = nRowId;
        maRows.emplace(nRowId, std::move(aRow));
    }
}

sal_Int32 LintSession::dropRow(const LintFinding* pFinding)
{
    const auto aFound = maRowIdByFinding.find(pFinding);
    if (aFound == maRowIdByFinding.end())
        return -1;

    const sal_Int32 nRowId = aFound->second;
    const auto aRow = maRows.find(nRowId);
    if (aRow != maRows.end())
    {
        mnEstimatedBytes -= aRow->second.mnSavingBytes;
        if (aRow->second.mpFinding->canFix())
            --mnFixableRowCount;

        maRows.erase(aRow);
    }

    maRowIdByFinding.erase(aFound);
    return nRowId;
}

void LintSession::refreshRowSaving(Row& rRow)
{
    // What the list comes to follows the figures the rows carry, so it moves by what this one row
    // moved by.
    mnEstimatedBytes -= rRow.mnSavingBytes;
    rRow.mnSavingBytes = rRow.mpFinding->getSavingBytes();
    rRow.mbMeasured = true;
    mnEstimatedBytes += rRow.mnSavingBytes;
}

void LintSession::cancelMeasurement()
{
    // The findings still waiting are given up on, so the list takes the shape it would have taken
    // had every one of them been measured.
    if (moLint)
        moLint->abandonMeasurement();
}

void LintSession::processMeasureStep(const LintMeasureStep& rStep, MeasureBatch& rBatch)
{
    if (rStep.mbDropped)
    {
        const sal_Int32 nRowId = dropRow(rStep.mpFinding.get());
        if (nRowId >= 0)
            rBatch.maDropped.push_back(nRowId);

        return;
    }

    // The rows are looked up by their finding every time, because a measurement before this one may
    // have taken a row off the list.
    const auto aFound = maRowIdByFinding.find(rStep.mpFinding.get());
    if (aFound == maRowIdByFinding.end())
        return;

    const auto aRow = maRows.find(aFound->second);
    if (aRow == maRows.end())
        return;

    refreshRowSaving(aRow->second);
    rBatch.maSavings.emplace_back(aFound->second, aRow->second.mnSavingBytes);
}

void LintSession::runMeasurement()
{
    if (!mrBase.GetDocument() || !moLint)
    {
        cancelMeasurement();
        finishScan();
        return;
    }

    // Working a finding out means encoding an image here. So this takes as many findings as its
    // slice of the time allows, and gives the time up at once when the client has sent something.
    // The figures that came out go out together at the end of the slice.
    TaskStopwatch aStopwatch;
    MeasureBatch aBatch;
    while (moLint->hasPendingMeasurement())
    {
        const LintMeasureStep aStep = moLint->measureNextFinding();
        if (!aStep.mpFinding)
            break;

        processMeasureStep(aStep, aBatch);

        if (clientIsWaiting() || !aStopwatch.continueIter())
            break;
    }

    sendMeasured(aBatch);

    // The findings this slice did not reach are taken up by a turn of the scheduler, which lets
    // everything else waiting for the main loop happen first.
    if (moLint->hasPendingMeasurement())
    {
        sendProgress("measuring", moLint->getMeasuredCount(), moLint->getMeasureCount());
        maStepIdle.Start();
        return;
    }

    moLint->finishMeasurement();
    finishScan();
}

void LintSession::startFixRun(std::vector<std::shared_ptr<LintFinding>> aFindings, bool bBatch,
                              sal_Int32 nRequestId)
{
    maFixList.start(std::move(aFindings));

    mbFixIsBatch = bBatch;
    mnFixRequestId = nRequestId;
    mbFixing = true;
    mbStopped = false;

    // A run counts its steps from the first one, so the session lets go of how far the run before
    // this one had come.
    mnShownPercent = -1;
    sendProgress("preparing", 0, maFixList.getCount());

    // The reply and the first progress event go out before a single image is encoded, because the
    // first slice waits for a turn of the scheduler.
    maStepIdle.Start();
}

void LintSession::advanceFixRun()
{
    if (!mrBase.GetDocument() || maFixList.isEmpty())
    {
        cancelFixRun();
        return;
    }

    // Working a cleanup out means encoding an image here. So this takes as many of them as its
    // slice of the time allows, and gives the time up at once when the client has sent something.
    // None of it reads or writes the document, so an edit that lands in between disturbs nothing.
    TaskStopwatch aStopwatch;
    while (LintWorkList::Entry* pEntry = maFixList.nextEntry())
    {
        pEntry->mpFinding->prepareFix();
        maFixList.markDone(*pEntry);

        if (clientIsWaiting() || !aStopwatch.continueIter())
            break;
    }

    // The cleanups this slice did not reach are taken up by a turn of the scheduler, which lets
    // everything else waiting for the main loop happen first.
    if (maFixList.hasPendingWork())
    {
        sendProgress("preparing", maFixList.getDoneCount(), maFixList.getCount());
        maStepIdle.Start();
        return;
    }

    // Every cleanup of the run has what it needs, so the client hears that the changes are about to
    // be made and then they are all made together, inside one undo entry.
    sendProgress("cleaning", maFixList.getCount(), maFixList.getCount());
    applyFixRun();
}

void LintSession::cancelFixRun()
{
    // What this run had worked out is no longer wanted, so the turn that would carry it on is
    // stopped and the list of it goes.
    maStepIdle.Stop();

    maFixList.clear();
    mbFixing = false;

    // Nothing about the document was changed and the list stands as it was.
    sendFixed({}, 0, true, false);
}

void LintSession::applyFixRun()
{
    // The name of a cleanup of the whole list is its own, and a cleanup of a single row takes the
    // name of the one finding it deals with.
    const OUString sUndoLabel = mbFixIsBatch
                                    ? SdResId(STR_LINT_UNDO_FIX_ALL)
                                    : maFixList.getEntries().front().mpFinding->getUndoLabel();
    sal_uInt64 nSavedBytes = 0;

    // A cleanup that reaches beyond the finding it is about leaves the rest of the list telling
    // of a document that has moved on, so the whole list is gathered again. Where every cleanup of
    // the run stays within its own finding, the rows around them still hold, and the figures they
    // carry are worth far more than a fresh scan would cost to work out again.
    const std::vector<LintWorkList::Entry>& rEntries = maFixList.getEntries();
    const bool bRescan
        = std::any_of(rEntries.begin(), rEntries.end(), [](const LintWorkList::Entry& rEntry)
                      { return rEntry.mpFinding->invalidatesOtherFindings(); });

    {
        // Everything the cleanup changes goes into one undo entry, so a single undo takes the
        // document back to where it was before the request arrived. The changes are all made here,
        // one after another with nothing in between, so nothing the person does can land inside
        // that entry and no finding can lose the object it is about halfway through.
        // The entry knows the session that made it, so taking the cleanup back or making it again
        // asks for the list to be gathered afresh from the document as it then stands.
        LintUndoGroup aUndoGroup(mrBase, sUndoLabel, mpUndoNotifier);

        for (const LintWorkList::Entry& rEntry : rEntries)
        {
            rEntry.mpFinding->fix(mrBase);
            nSavedBytes += rEntry.mpFinding->getRealizedSavingBytes();
        }
    }

    if (mbFixIsBatch)
        mnBatchSavedBytes = nSavedBytes;

    // The findings that were dealt with are the only ones the list loses, whether or not the
    // cleanup found anything left to change.
    std::vector<sal_Int32> aRemoved;
    aRemoved.reserve(rEntries.size());
    for (const LintWorkList::Entry& rEntry : rEntries)
    {
        const sal_Int32 nRowId = dropRow(rEntry.mpFinding.get());
        if (nRowId >= 0)
            aRemoved.push_back(nRowId);

        if (!bRescan && moLint)
            moLint->dropFinding(rEntry.mpFinding);
    }

    maFixList.clear();
    mbFixing = false;

    if (bRescan)
    {
        // The list the client holds tells of a document that has moved on, so what is left of it is
        // greyed out until the scan that follows brings a fresh one.
        sendFixed(aRemoved, nSavedBytes, false, true);
        requestRebuild(ListReason::Fix);
        return;
    }

    // A cleanup of the whole list reports what it freed up, once. The figure is taken over here so
    // that every later list goes back to the estimate for what is still left to do.
    mnShownSavedBytes = mnBatchSavedBytes;
    mnBatchSavedBytes = 0;

    sendFixed(aRemoved, nSavedBytes, false, false);
    runPendingRebuild();
}

IMPL_LINK_NOARG(LintSession, RescanHdl, Timer*, void)
{
    if (!mrBase.GetDocument())
        return;

    // A run that started after the rescan was asked for is working on the list this would throw
    // away, so the list waits for that run to end, the same way a request during a run does.
    if (isBusy())
    {
        mbRebuildPending = true;
        meListReason = ListReason::Undo;
        return;
    }

    // The undo or the redo that asked for this has finished, so the list is gathered from the
    // document as it now stands.
    startScan(ListReason::Undo);
}

IMPL_LINK_NOARG(LintSession, StepHdl, Timer*, void)
{
    // This is where the run that is on takes up the next slice of its work.
    if (mbScanning)
        runMeasurement();
    else if (mbFixing)
        advanceFixRun();
}

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
