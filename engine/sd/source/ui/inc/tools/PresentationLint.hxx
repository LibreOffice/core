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
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <sddllapi.h>

class SdDrawDocument;
class SdrPage;
class SfxUndoManager;

namespace sd
{
class ViewShellBase;
}

namespace sd::lint
{
class LintMeasureCache;

/** The kind of problem a finding describes. */
enum class LintCategory
{
    LargeImage,
    HiddenSlide,
    UnusedMaster,
    NotesContent,
    OleObject
};

/** The settings the scan and the later fix step both work from. */
struct LintOptions
{
    /** Target resolution in dots per inch. Zero means images are left alone. */
    sal_Int32 mnImageResolution = 150;

    /** JPEG quality from 1 to 100 for the images that are encoded again. */
    sal_Int32 mnJPEGQuality = 80;

    /** True when the deck is prepared for handing out, and the stricter rules hold as well. */
    bool mbForPublication = false;
};

/** The figures one finding is described by, which the client writes into a row in the language of
    its own view. A field that has no meaning for the kind of finding keeps its empty value. */
struct LintFindingFacts
{
    /** Which image of its slide this one is, counting from one. Zero says the image stands alone
        for its slide, so the row has no number to carry. */
    sal_Int32 mnImageNumber = 0;

    /** Dots per inch the image is stored at, worked out against the largest size it is drawn at.
        Zero for a finding that is not about an image, and for an image the document keeps no
        encoded data for. */
    sal_Int32 mnEffectiveDPI = 0;

    /** The share of the picture a crop hides, from zero to a hundred. Zero for a finding that is
        not about a cropped picture. */
    sal_Int32 mnHiddenPercent = 0;

    /** What the finding is about by name, such as a master slide. Empty for a finding whose row
        names no such thing. */
    OUString maName;
};

/** One problem found in a document, together with the figures that describe it. */
class SD_DLLPUBLIC LintFinding
{
public:
    LintFinding(SdDrawDocument& rDoc, LintCategory eCategory, sal_uInt64 nCurrentBytes,
                sal_Int32 nSlideIndex);
    virtual ~LintFinding();

    LintCategory getCategory() const { return meCategory; }

    const LintFindingFacts& getFacts() const { return maFacts; }

    /** Bytes the document spends on this finding today, or zero when there is no such number. */
    sal_uInt64 getCurrentBytes() const { return mnCurrentBytes; }

    /** Zero-based number of the standard page, or -1 for a finding about the whole document. */
    sal_Int32 getSlideIndex() const { return mnSlideIndex; }

    /** True when goTo moves the view somewhere useful. */
    virtual bool canGoto() const { return false; }

    virtual void goTo(ViewShellBase& rViewShellBase);

    /** True when this finding has something that can be changed. False for a finding with
        nothing to change, such as an embedded object that carries no picture to put in its
        place. */
    virtual bool canFix() const { return true; }

    /** Does everything a fix needs worked out before it changes anything, and keeps the result for
        the fix to use. Nothing about the document is read or written here, so a caller is free to
        run this for one finding at a time and let the person carry on working in between. Returns
        false when there turns out to be nothing to do after all. A finding whose fix needs no
        preparation answers whether it has a fix at all.

        A fix runs this itself when it was not run beforehand, so a caller that has no use for the
        two halves can call fix on its own. */
    virtual bool prepareFix() { return canFix(); }

    /** True when carrying out the cleanup of this finding can change what the other findings of the
        same scan say, either because it reaches objects they are about or because it changes the
        shape of the document around them. A finding that has not worked the question out for its
        own kind of cleanup answers true, which is the safe answer for any rule. */
    virtual bool invalidatesOtherFindings() const { return true; }

    /** Makes every change the cleanup of this finding calls for. The changes go into the undo list
        action that is open at the time, and a fix that finds nothing left to do makes no change at
        all. Preparing the fix beforehand leaves this with only the changes to the document to
        make. */
    virtual void fix(ViewShellBase& rViewShellBase);

    /** Works out what dealing with this finding would free up and remembers it. Returns false when
        the measurement shows the finding is not worth listing after all. A finding whose saving
        needs no working out answers true and does nothing. */
    virtual bool measureSaving();

    /** True when measureSaving has real work to do for this finding. */
    virtual bool needsMeasuring() const { return false; }

    /** Bytes the document would stop spending once this finding is dealt with. Zero when that
        number is not known, which is also what an unmeasured finding answers. */
    virtual sal_uInt64 getSavingBytes() const { return 0; }

    /** Bytes a fix that has already run did free up, or zero when no fix ran or the number is not
        known. */
    virtual sal_uInt64 getRealizedSavingBytes() const { return 0; }

    /** Tells the finding it is the nRowNumber-th of the several findings of its kind that name one
        slide, counting from one. A number of zero says the finding stands alone for its slide, and
        a finding whose row has no room for a number leaves it be. */
    virtual void setRowNumber(sal_Int32 nRowNumber);

    /** Name the undo stack shows for the entry the fix of this finding leaves behind. */
    OUString getUndoLabel() const;

protected:
    SdDrawDocument& mrDoc;
    LintCategory meCategory;
    LintFindingFacts maFacts;
    sal_uInt64 mnCurrentBytes;
    sal_Int32 mnSlideIndex;
};

/** Holds what to call when a cleanup is taken back or made again. Whoever wants to hear about that
    keeps the one shared pointer to it, so it goes when that holder goes. An undo entry names it by
    a weak pointer and calls it only while the holder is still there, which is what lets the
    entries of a cleanup outlive whoever asked for them. */
class SD_DLLPUBLIC LintUndoNotifier
{
public:
    explicit LintUndoNotifier(std::function<void()> aCallback);

    /** Calls what the owner asked to be called, and does nothing for an owner that asked for
        nothing. */
    void notify() const;

private:
    std::function<void()> maCallback;
};

/** Holds one named undo list action open for as long as it lives, so everything a fix changes ends
    up as a single entry on the undo stack. The entry carries the id of the view it was opened for,
    which is the view that is then allowed to take it back. An action group that stays empty, as it
    does when a fix finds nothing left to change, is dropped again on the way out.

    A group that is given a notifier puts one more action in the entry, which calls that notifier
    whenever the entry is taken back or made again. That action goes in only once the group has
    something else in it, so an entry that would have been dropped is still dropped. */
class SD_DLLPUBLIC LintUndoGroup
{
public:
    LintUndoGroup(ViewShellBase& rViewShellBase, const OUString& rLabel,
                  std::weak_ptr<LintUndoNotifier> pNotifier = {});
    ~LintUndoGroup();

    LintUndoGroup(const LintUndoGroup&) = delete;
    LintUndoGroup& operator=(const LintUndoGroup&) = delete;

private:
    SfxUndoManager* mpUndoManager = nullptr;
    std::weak_ptr<LintUndoNotifier> mpNotifier;
};

/** What one step of the step-wise measurement did. */
struct LintMeasureStep
{
    /** The finding the step measured, or null when there was nothing left to measure. */
    std::shared_ptr<LintFinding> mpFinding;

    /** True when the measurement showed the finding is not worth a row, so it is off the list. */
    bool mbDropped = false;
};

/** The findings of one run, in the order the work takes them, and how far the work has come.

    The run walks the entries once from front to back. A finding keeps its place for as long as
    the run lasts, and one that goes off the list part way through counts as finished where it
    stands. */
class SD_DLLPUBLIC LintWorkList
{
public:
    /** One finding of the run and whether the work on it is done. */
    struct Entry
    {
        std::shared_ptr<LintFinding> mpFinding;

        /** True once the work on this finding is done. */
        bool mbDone = false;
    };

    /** Starts a run over the given findings, in the order the work takes them. */
    void start(std::vector<std::shared_ptr<LintFinding>> aFindings);

    /** Drops every entry and ends the run. */
    void clear();

    /** Ends the run and drops the entries whose work never got done. What is left is what the run
        finished, so nothing is owed any more. */
    void abandon();

    bool isEmpty() const { return maEntries.empty(); }

    /** How many findings the run has to work through altogether. */
    std::size_t getCount() const { return maEntries.size(); }

    /** How many of those findings the run has finished. */
    std::size_t getDoneCount() const { return mnDoneCount; }

    const std::vector<Entry>& getEntries() const { return maEntries; }

    /** True while the work on some finding of the run is still owed. */
    bool hasPendingWork() const { return mnDoneCount < maEntries.size(); }

    /** The next entry whose work is still owed, or null once the run has reached the end of the
        list. */
    Entry* nextEntry();

    /** Writes down that the work on the entry is done. */
    void markDone(Entry& rEntry);

    /** Writes off whatever the run still owed for the given finding. */
    void markFindingDone(const LintFinding* pFinding);

private:
    /** The findings of the run, in the order the work takes them. */
    std::vector<Entry> maEntries;

    /** How many entries the run has finished. */
    std::size_t mnDoneCount = 0;

    /** How far along the entries the work has come. Every entry before it has been handed out
        once. */
    std::size_t mnCursor = 0;
};

/** Runs every lint rule over a document and collects what they find. Working out what an image
    would save costs real time, so that part is kept apart from collecting: scanStructure gathers
    the whole list at once, and the findings that carry a figure are then measured one at a time, a
    step to a finding. */
class SD_DLLPUBLIC PresentationLint
{
public:
    /** A cache keeps what the measurements of images worked out. Handing the same one to every
        scan of a document leaves an image that has been measured before with its figure already
        known.
        An empty pointer means every measurement is worked out afresh. */
    PresentationLint(SdDrawDocument& rDoc, LintOptions aOptions,
                     std::shared_ptr<LintMeasureCache> pMeasureCache = {});

    /** Throws away the findings of an earlier scan, collects them again and measures every one of
        them that carries a figure. */
    void scan();

    /** Throws away the findings of an earlier scan and collects them again, measuring nothing. The
        findings that carry a figure are lined up for the measurement steps that follow. */
    void scanStructure();

    /** True while a finding is still waiting for its saving to be worked out. */
    bool hasPendingMeasurement() const { return maMeasureList.hasPendingWork(); }

    /** Works out what the next such finding would save. A finding that turns out to have too little
        or nothing to offer comes off the list, which the answer says. */
    LintMeasureStep measureNextFinding();

    /** Brings the list into the shape the reader sees once every measurement has run. Running it a
        second time changes nothing. */
    void finishMeasurement();

    /** Gives up on the findings still waiting to be measured. They stay on the list without a
        figure, which is what a finding whose saving is not known carries anyway, and the list
        takes the shape it would have taken had the measurement run to the end. */
    void abandonMeasurement();

    /** How many findings the measurement has to work through altogether. */
    std::size_t getMeasureCount() const { return maMeasureList.getCount(); }

    /** How many of those findings have been measured so far. */
    std::size_t getMeasuredCount() const { return maMeasureList.getDoneCount(); }

    /** Takes the given finding off the list, the way a measurement that found nothing worth
        listing takes one off. Whatever the measurement still owed for it counts as finished, and
        the numbers that tell the rows of one slide apart are worked out again over what is left. */
    void dropFinding(const std::shared_ptr<LintFinding>& rpFinding);

    /** What the last scan found, grouped by category. Inside a group the finding the document
        spends the most bytes on comes first, and the findings that carry no byte count follow in
        scan order. */
    const std::vector<std::shared_ptr<LintFinding>>& getFindings() const { return maFindings; }

private:
    SdDrawDocument& mrDoc;
    LintOptions maOptions;
    std::shared_ptr<LintMeasureCache> mpMeasureCache;
    std::vector<std::shared_ptr<LintFinding>> maFindings;

    /** The findings whose saving is worth working out, in the order the steps take them. */
    LintWorkList maMeasureList;

    /** True once finishMeasurement has run over the current list. */
    bool mbMeasurementFinished = false;

    /** Lines up the collected findings that carry a figure, in the order the steps take them. */
    void buildMeasureList();

    /** Puts the finding of an entry that has just been measured where the list wants it: off the
        list when the figure that came out is too small to be worth reading. */
    LintMeasureStep finishEntry(LintWorkList::Entry& rEntry, bool bWorthListing);

    /** Brings the collected findings into the order getFindings hands them out in. */
    void sortForDisplay();

    /** Hands a number to each of the findings that would otherwise read alike. */
    void numberRows();
};

/** Pieces the framework and the rule implementations share inside the sd library. */
namespace detail
{
/** Zero-based number of the slide the page belongs to. A notes page maps to its slide, a master
    page or an unknown page to -1. */
sal_Int32 getSlideIndexOfPage(SdDrawDocument& rDoc, const SdrPage* pPage);

/** Collects the findings about the drawing objects: the images that carry more pixels than the deck
    needs, and, for a deck that is being prepared for readers, the embedded objects. Both rules read
    the same objects, and one walk over the document serves them together. */
void collectObjectFindings(SdDrawDocument& rDoc, const LintOptions& rOptions,
                           const std::shared_ptr<LintMeasureCache>& rpMeasureCache,
                           std::vector<std::shared_ptr<LintFinding>>& rFindings);
void collectHiddenSlides(SdDrawDocument& rDoc,
                         std::vector<std::shared_ptr<LintFinding>>& rFindings);
void collectUnusedMasters(SdDrawDocument& rDoc,
                          std::vector<std::shared_ptr<LintFinding>>& rFindings);
void collectNotesContent(SdDrawDocument& rDoc,
                         std::vector<std::shared_ptr<LintFinding>>& rFindings);

} // end of namespace detail

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
