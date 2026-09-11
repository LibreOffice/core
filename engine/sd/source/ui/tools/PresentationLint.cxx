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

#include <tools/PresentationLint.hxx>

#include <algorithm>
#include <map>
#include <utility>

#include <svl/undo.hxx>
#include <svx/svdpage.hxx>

#include <DrawDocShell.hxx>
#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>

namespace sd::lint
{
namespace
{
/** One entry in the undo list of a cleanup that carries no change of its own. Taking the list back
    or making it again calls the notifier the group was given, for as long as that notifier is
    still there. */
class LintUndoNotifyAction final : public SfxUndoAction
{
public:
    explicit LintUndoNotifyAction(std::weak_ptr<LintUndoNotifier> pNotifier)
        : mpNotifier(std::move(pNotifier))
    {
    }

    virtual void Undo() override { notifyOwner(); }
    virtual void Redo() override { notifyOwner(); }

    /** The name of the whole entry is the one the cleanup was given, and this part of it adds
        nothing to it. */
    virtual OUString GetComment() const override { return OUString(); }

private:
    void notifyOwner() const
    {
        if (std::shared_ptr<LintUndoNotifier> pNotifier = mpNotifier.lock())
            pNotifier->notify();
    }

    std::weak_ptr<LintUndoNotifier> mpNotifier;
};

} // end of anonymous namespace

LintFinding::LintFinding(SdDrawDocument& rDoc, LintCategory eCategory, sal_uInt64 nCurrentBytes,
                         sal_Int32 nSlideIndex)
    : mrDoc(rDoc)
    , meCategory(eCategory)
    , mnCurrentBytes(nCurrentBytes)
    , mnSlideIndex(nSlideIndex)
{
}

LintFinding::~LintFinding() = default;

void LintFinding::goTo(ViewShellBase&) {}

void LintFinding::fix(ViewShellBase&) {}

bool LintFinding::measureSaving() { return true; }

void LintFinding::setRowNumber(sal_Int32) {}

OUString LintFinding::getUndoLabel() const
{
    switch (meCategory)
    {
        case LintCategory::LargeImage:
            return SdResId(STR_LINT_UNDO_COMPRESS_IMAGE);
        case LintCategory::HiddenSlide:
            return SdResId(STR_LINT_UNDO_DELETE_HIDDEN_SLIDE);
        case LintCategory::UnusedMaster:
            return SdResId(STR_LINT_UNDO_DELETE_UNUSED_MASTER);
        case LintCategory::NotesContent:
            return SdResId(STR_LINT_UNDO_CLEAR_NOTES);
        case LintCategory::OleObject:
            return SdResId(STR_LINT_UNDO_REPLACE_OBJECT);
    }

    return OUString();
}

LintUndoNotifier::LintUndoNotifier(std::function<void()> aCallback)
    : maCallback(std::move(aCallback))
{
}

void LintUndoNotifier::notify() const
{
    if (maCallback)
        maCallback();
}

LintUndoGroup::LintUndoGroup(ViewShellBase& rViewShellBase, const OUString& rLabel,
                             std::weak_ptr<LintUndoNotifier> pNotifier)
    : mpNotifier(std::move(pNotifier))
{
    SdDrawDocument* pDocument = rViewShellBase.GetDocument();
    DrawDocShell* pDocShell = rViewShellBase.GetDocShell();
    if (!pDocument || !pDocShell || !pDocument->IsUndoEnabled())
        return;

    SfxUndoManager* pUndoManager = pDocShell->GetUndoManager();
    if (!pUndoManager)
        return;

    mpUndoManager = pUndoManager;
    mpUndoManager->EnterListAction(rLabel, rLabel, 0, rViewShellBase.GetViewShellId());
}

LintUndoGroup::~LintUndoGroup()
{
    if (!mpUndoManager)
        return;

    // The call is worth an action of its own only alongside a change that can be taken back, so a
    // group that changed nothing is dropped on the way out.
    if (!mpNotifier.expired()
        && mpUndoManager->GetUndoActionCount(SfxUndoManager::CurrentLevel) > 0)
        mpUndoManager->AddUndoAction(std::make_unique<LintUndoNotifyAction>(mpNotifier));

    mpUndoManager->LeaveListAction();
}


namespace detail
{
sal_Int32 getSlideIndexOfPage(SdDrawDocument& rDoc, const SdrPage* pPage)
{
    if (!pPage)
        return -1;

    // A notes page counts as the slide it belongs to, so an object pasted into the speaker notes
    // is reported under that slide's number.
    const sal_uInt16 nPageCount = rDoc.GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        if (rDoc.GetSdPage(nPage, PageKind::Standard) == pPage
            || rDoc.GetSdPage(nPage, PageKind::Notes) == pPage)
            return sal_Int32(nPage);
    }

    return -1;
}

} // end of namespace detail

void LintWorkList::start(std::vector<std::shared_ptr<LintFinding>> aFindings)
{
    clear();

    maEntries.reserve(aFindings.size());
    for (std::shared_ptr<LintFinding>& rpFinding : aFindings)
        maEntries.push_back({ std::move(rpFinding), false });
}

void LintWorkList::clear()
{
    maEntries.clear();
    mnDoneCount = 0;
    mnCursor = 0;
}

void LintWorkList::abandon()
{
    std::erase_if(maEntries, [](const Entry& rEntry) { return !rEntry.mbDone; });

    // What is left is finished to the last entry, so nothing is owed and there is nowhere further
    // for the cursor to go.
    mnDoneCount = maEntries.size();
    mnCursor = maEntries.size();
}

LintWorkList::Entry* LintWorkList::nextEntry()
{
    while (mnCursor < maEntries.size())
    {
        Entry& rEntry = maEntries[mnCursor];

        // The steps pick up where the one before them left off, so each entry is handed out once
        // and the whole run costs one pass over the list.
        ++mnCursor;

        if (!rEntry.mbDone)
            return &rEntry;
    }

    return nullptr;
}

void LintWorkList::markDone(Entry& rEntry)
{
    if (rEntry.mbDone)
        return;

    rEntry.mbDone = true;
    ++mnDoneCount;
}

void LintWorkList::markFindingDone(const LintFinding* pFinding)
{
    // The entry stays where it is and counts as finished, which leaves the count of what is still
    // owed true to the list and keeps the cursor of the run where it stands.
    for (Entry& rEntry : maEntries)
    {
        if (rEntry.mpFinding.get() == pFinding)
            markDone(rEntry);
    }
}

PresentationLint::PresentationLint(SdDrawDocument& rDoc, LintOptions aOptions,
                                   std::shared_ptr<LintMeasureCache> pMeasureCache)
    : mrDoc(rDoc)
    , maOptions(aOptions)
    , mpMeasureCache(std::move(pMeasureCache))
{
}

void PresentationLint::scan()
{
    scanStructure();

    // The whole measurement runs here, one step to a finding, so the list is complete by the time
    // this returns.
    while (hasPendingMeasurement())
    {
        if (!measureNextFinding().mpFinding)
            break;
    }

    finishMeasurement();
}

void PresentationLint::scanStructure()
{
    maFindings.clear();
    mbMeasurementFinished = false;

    detail::collectObjectFindings(mrDoc, maOptions, mpMeasureCache, maFindings);
    detail::collectHiddenSlides(mrDoc, maFindings);
    detail::collectUnusedMasters(mrDoc, maFindings);

    if (maOptions.mbForPublication)
        detail::collectNotesContent(mrDoc, maFindings);

    sortForDisplay();
    numberRows();
    buildMeasureList();
}

void PresentationLint::buildMeasureList()
{
    // Working out what an image would save means encoding it again, which takes real time. So the
    // findings that carry a figure are lined up here and measured a step at a time: a deck carrying
    // very many oversized images takes longer to report its figures while the document goes on
    // taking input, and the reader can stop the measurement.
    std::vector<std::shared_ptr<LintFinding>> aToMeasure;
    for (const std::shared_ptr<LintFinding>& rpFinding : maFindings)
    {
        if (rpFinding->needsMeasuring())
            aToMeasure.push_back(rpFinding);
    }

    maMeasureList.start(std::move(aToMeasure));
}

LintMeasureStep PresentationLint::finishEntry(LintWorkList::Entry& rEntry, bool bWorthListing)
{
    // A cleanup that frees up only a few kilobytes costs a row to read and, for an image, some of
    // its detail, so a measured gain under this floor is left out of the list altogether.
    constexpr sal_uInt64 nMinimumSavingBytes = 64 * 1024;

    maMeasureList.markDone(rEntry);

    LintMeasureStep aStep;
    aStep.mpFinding = rEntry.mpFinding;

    // A measurement that turns up nothing to gain takes the row off the list, so the list never
    // offers a cleanup that would leave the document no smaller.
    if (!bWorthListing)
    {
        aStep.mbDropped = true;
    }
    else
    {
        // A finding whose figure the measurement could not work out answers zero here, and the
        // floor has nothing to say about it.
        const sal_uInt64 nSavingBytes = aStep.mpFinding->getSavingBytes();
        aStep.mbDropped = nSavingBytes > 0 && nSavingBytes < nMinimumSavingBytes;
    }

    if (!aStep.mbDropped)
        return aStep;

    std::erase(maFindings, aStep.mpFinding);
    return aStep;
}

LintMeasureStep PresentationLint::measureNextFinding()
{
    LintWorkList::Entry* pEntry = maMeasureList.nextEntry();
    if (!pEntry)
        return LintMeasureStep();

    return finishEntry(*pEntry, pEntry->mpFinding->measureSaving());
}

void PresentationLint::finishMeasurement()
{
    if (mbMeasurementFinished)
        return;

    mbMeasurementFinished = true;

    // The rows the measurement dropped leave the findings around them alone for their slide, so
    // the numbers that told them apart are worked out again over what is left.
    numberRows();
}

void PresentationLint::abandonMeasurement()
{
    // What is left of this run is no longer wanted, so the entries that never got their turn go.
    maMeasureList.abandon();

    finishMeasurement();
}

void PresentationLint::dropFinding(const std::shared_ptr<LintFinding>& rpFinding)
{
    if (!rpFinding)
        return;

    std::erase(maFindings, rpFinding);

    maMeasureList.markFindingDone(rpFinding.get());

    // The findings that are left keep whatever they say about themselves. Only the numbers that
    // tell the rows of one slide apart are about the list as a whole, so those are worked out
    // again.
    numberRows();
}

void PresentationLint::sortForDisplay()
{
    // The findings stay grouped by category, and inside a group the one the document spends the
    // most bytes on comes first, which is the order the savings come out in as well. A finding
    // with no byte count keeps the order the scan met it in, after those that carry one, because a
    // zero sorts last.
    auto isListedBefore =
        [](const std::shared_ptr<LintFinding>& rpLeft, const std::shared_ptr<LintFinding>& rpRight)
    {
        if (rpLeft->getCategory() != rpRight->getCategory())
            return rpLeft->getCategory() < rpRight->getCategory();

        return rpLeft->getCurrentBytes() > rpRight->getCurrentBytes();
    };

    std::stable_sort(maFindings.begin(), maFindings.end(), isListedBefore);
}

void PresentationLint::numberRows()
{
    // Two findings of one kind about one slide come out as rows that read alike, so each is handed
    // its place among them. The count is taken over the findings that are on the list right now, so
    // a finding the measurement dropped leaves no gap in the numbers, and a finding that is left
    // alone for its slide is told so and drops the number it had.
    std::map<std::pair<LintCategory, sal_Int32>, sal_Int32> aCountPerSlide;
    for (const std::shared_ptr<LintFinding>& rpFinding : maFindings)
        ++aCountPerSlide[{ rpFinding->getCategory(), rpFinding->getSlideIndex() }];

    std::map<std::pair<LintCategory, sal_Int32>, sal_Int32> aNumberPerSlide;
    for (const std::shared_ptr<LintFinding>& rpFinding : maFindings)
    {
        const std::pair<LintCategory, sal_Int32> aKey{ rpFinding->getCategory(),
                                                       rpFinding->getSlideIndex() };
        const sal_Int32 nRowNumber = ++aNumberPerSlide[aKey];
        rpFinding->setRowNumber(aCountPerSlide[aKey] > 1 ? nRowNumber : 0);
    }
}

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
