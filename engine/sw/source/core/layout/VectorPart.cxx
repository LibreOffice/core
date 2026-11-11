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

#include <VectorPart.hxx>

#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <viewimp.hxx>
#include <viewsh.hxx>

#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <svx/VectorObjectContent.hxx>
#include <tools/json_writer.hxx>

#include <algorithm>
#include <unordered_set>

namespace sw::vector
{
namespace
{
/// The parent the page entry names: none, so a walk up the parents of an entry ends at the page.
constexpr sal_Int64 constNoParentId = -1;

/// A drawing object keeps its primitives over a change of its model, so it compares by what they
/// decompose to, and one that moved decomposes to other coordinates. A frame builds its primitives
/// anew each time and compares by them.
bool sameDrawing(const Entry& rOne, const Entry& rOther)
{
    if (rOne.meKind != rOther.meKind)
        return false;
    if (!rOne.maDrawn.empty() || !rOther.maDrawn.empty())
        return rOne.maDrawn == rOther.maDrawn;
    return rOne.maPrimitives == rOther.maPrimitives;
}

/// The key the record of an entry is filed under. The page entry has the same id on every page, so
/// it is filed under the page.
sal_uInt64 recordKeyOf(sal_uInt64 nId, const SwPageFrame& rPage)
{
    return nId == constPageEntryId ? (sal_uInt64(3) << 40) + rPage.GetFrameId() : nId;
}

bool samePlace(const Entry& rOne, const SwRect& rOneBox, const Entry& rOther,
               const SwRect& rOtherBox)
{
    return rOneBox == rOtherBox && rOne.moParent == rOther.moParent
           && rOne.mbIsFollow == rOther.mbIsFollow && rOne.mbHasFollow == rOther.mbHasFollow
           && rOne.moAnchor == rOther.moAnchor;
}

/// Whether the layout is idle: no view of the document runs an action, formats the layout, paints
/// or has its paint locked.
bool isLayoutIdle(const SwRootFrame& rLayout)
{
    if (SwRootFrame::IsInPaint())
        return false;

    const SwViewShell* pShell = rLayout.GetCurrShell();
    if (!pShell)
        return true;

    for (const SwViewShell& rShell : pShell->GetRingContainer())
    {
        if (rShell.ActionPend() || rShell.Imp()->IsAction() || rShell.IsPaintInProgress()
            || rShell.IsPaintLocked())
            return false;
    }
    return true;
}
}

OString partIdOf(const SwPageFrame& rPage) { return OString::number(rPage.GetFrameId()); }

const SwPageFrame* pageOf(const SwRootFrame& rLayout, std::string_view rPartId)
{
    // The pages are searched by the number rPartId reads as. The page with that frame id is the
    // one named when its part id is equal to rPartId.
    const sal_Int64 nFrameId = o3tl::toInt64(rPartId);
    for (const SwFrame* pFrame = rLayout.Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (!pFrame->IsPageFrame() || pFrame->GetFrameId() != nFrameId)
            continue;
        const SwPageFrame* pPage = static_cast<const SwPageFrame*>(pFrame);
        return partIdOf(*pPage) == rPartId ? pPage : nullptr;
    }
    return nullptr;
}

std::vector<OString> partIdsOf(const SwRootFrame& rLayout)
{
    std::vector<OString> aPartIds;
    for (const SwFrame* pFrame = rLayout.Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (pFrame->IsPageFrame())
            aPartIds.push_back(partIdOf(*static_cast<const SwPageFrame*>(pFrame)));
    }
    return aPartIds;
}

bool PartStore::recordPageIds(const SwRootFrame& rLayout)
{
    std::vector<sal_uInt32> aPageIds;
    aPageIds.reserve(maPageIds.size());
    for (const SwFrame* pFrame = rLayout.Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (pFrame->IsPageFrame())
            aPageIds.push_back(pFrame->GetFrameId());
    }
    if (aPageIds == maPageIds)
        return false;
    maPageIds = std::move(aPageIds);
    return true;
}

bool PartStore::isServed(std::string_view rPartId) const
{
    auto aIterator = maParts.find(OString(rPartId));
    return aIterator != maParts.end() && aIterator->second.maVersions.isServed();
}

void PartStore::forget(std::string_view rPartId)
{
    auto aIterator = maParts.find(OString(rPartId));
    if (aIterator == maParts.end())
        return;

    // An entry that moved to another page by now is kept for that page.
    std::erase_if(maEntries,
                  [&rPartId](const auto& rPair) { return rPair.second.maPartId == rPartId; });

    // The versions go on counting from where they are, so a version a client held before names
    // no content the part serves later.
    PartState aKept;
    aKept.maVersions = aIterator->second.maVersions;
    aKept.maVersions.forget();
    aIterator->second = std::move(aKept);
}

std::vector<OString> PartStore::invalidate(const SwRootFrame& rLayout,
                                           const std::vector<SwRect>& rAreas)
{
    std::vector<OString> aChanged;
    if (maParts.empty())
        return aChanged;

    // The map holds the parts by the frame id of their page. The part id is that number in
    // decimal. A part leaves the map when its page is found, so the parts of the pages that are
    // gone remain.
    std::unordered_map<sal_uInt32, std::pair<const OString, PartState>*> aParts;
    for (std::pair<const OString, PartState>& rPart : maParts)
        aParts.emplace(o3tl::toUInt32(std::string_view(rPart.first)), &rPart);

    for (const SwFrame* pFrame = rLayout.Lower(); pFrame && !aParts.empty();
         pFrame = pFrame->GetNext())
    {
        if (!pFrame->IsPageFrame())
            continue;

        auto aPartIterator = aParts.find(pFrame->GetFrameId());
        if (aPartIterator == aParts.end())
            continue;
        const OString& rPartId = aPartIterator->second->first;
        PartState& rState = aPartIterator->second->second;
        aParts.erase(aPartIterator);
        if (!rState.maVersions.isServed())
            continue;

        // The objects of a page can reach past it, so the page is marked when an area touches
        // the page or any entry it holds.
        bool bTouched = std::any_of(rAreas.begin(), rAreas.end(), [pFrame](const SwRect& rArea) {
            return rArea.Overlaps(pFrame->getFrameArea());
        });
        for (size_t i = 0; !bTouched && i < rState.maEntryIds.size(); ++i)
        {
            auto aEntryIterator = maEntries.find(rState.maEntryIds[i]);
            if (aEntryIterator == maEntries.end())
                continue;
            const SwRect& rBox = aEntryIterator->second.maEntry.maBox;
            bTouched = std::any_of(rAreas.begin(), rAreas.end(),
                                   [&rBox](const SwRect& rArea) { return rArea.Overlaps(rBox); });
        }

        if (bTouched)
        {
            rState.mbDirty = true;
            aChanged.push_back(rPartId);
        }
    }

    // A served page that is gone is reported too. A gone page that no client holds is dropped
    // with its versions, since its part id names no page again.
    for (const auto& rGone : aParts)
    {
        PartState& rState = rGone.second->second;
        if (rState.maVersions.isServed())
        {
            rState.mbDirty = true;
            aChanged.push_back(rGone.second->first);
        }
        else
        {
            const OString aPartId(rGone.second->first);
            maParts.erase(aPartId);
        }
    }
    return aChanged;
}

std::vector<OString> PartStore::takeDeferredPushes(const SwRootFrame& rLayout)
{
    std::vector<OString> aDeferred;
    if (!maDeferredPushes.empty() && isLayoutIdle(rLayout))
        aDeferred.swap(maDeferredPushes);
    return aDeferred;
}

void PartStore::refresh(PartState& rState, const SwPageFrame& rPage)
{
    rState.mbDirty = false;

    const OString aPartId = partIdOf(rPage);
    const Point aPageOrigin(rPage.getFrameArea().Pos());
    const sal_uInt64 nNextVersion = rState.maVersions.getVersion() + 1;
    bool bChanged = false;

    const std::unordered_set<sal_uInt64> aHeld(rState.maEntryIds.begin(),
                                               rState.maEntryIds.end());
    std::vector<Entry> aEntries = decomposePage(rPage);

    std::vector<sal_uInt64> aOrder;
    aOrder.reserve(aEntries.size());
    for (Entry& rEntry : aEntries)
    {
        aOrder.push_back(rEntry.mnId);

        SwRect aPageBox(rEntry.maBox);
        aPageBox.Pos(aPageBox.Pos() - aPageOrigin);

        const sal_uInt64 nKey = recordKeyOf(rEntry.mnId, rPage);
        auto aIterator = maEntries.find(nKey);
        if (aIterator == maEntries.end())
        {
            RecordedEntry aRecord;
            aRecord.maEntry = std::move(rEntry);
            aRecord.maPartId = aPartId;
            aRecord.maPageBox = aPageBox;
            aRecord.mnDrawnVersion = nNextVersion;
            aRecord.mnPlacedVersion = nNextVersion;
            maEntries.emplace(nKey, std::move(aRecord));
            bChanged = true;
            continue;
        }

        RecordedEntry& rRecord = aIterator->second;
        // An entry that came over from another page is new to the readers of this one, so it is
        // written whole.
        const bool bNewHere = rRecord.maPartId != aPartId || !aHeld.contains(rEntry.mnId);
        const bool bPlaceChanged
            = bNewHere || !samePlace(rRecord.maEntry, rRecord.maPageBox, rEntry, aPageBox);
        const bool bDrawingChanged = bNewHere || !sameDrawing(rRecord.maEntry, rEntry);

        if (bDrawingChanged)
            rRecord.mnDrawnVersion = nNextVersion;
        if (bPlaceChanged)
            rRecord.mnPlacedVersion = nNextVersion;
        bChanged = bChanged || bDrawingChanged || bPlaceChanged;

        rRecord.maEntry = std::move(rEntry);
        rRecord.maPartId = aPartId;
        rRecord.maPageBox = aPageBox;
    }

    // Entries that left the page are forgotten, unless another page holds them now.
    const std::unordered_set<sal_uInt64> aNow(aOrder.begin(), aOrder.end());
    for (const sal_uInt64 nId : rState.maEntryIds)
    {
        if (aNow.contains(nId))
            continue;
        const sal_uInt64 nKey = recordKeyOf(nId, rPage);
        auto aIterator = maEntries.find(nKey);
        if (aIterator != maEntries.end() && aIterator->second.maPartId == aPartId)
            maEntries.erase(aIterator);
    }

    const bool bOrderChanged = aOrder != rState.maOrder;
    if (bOrderChanged)
        rState.maOrder = aOrder;
    rState.maEntryIds = std::move(aOrder);

    // What changed was given the next version, which the part takes now.
    if (bOrderChanged)
        rState.maVersions.countUpOrder();
    else if (bChanged)
        rState.maVersions.countUp();
}

void PartStore::writeEntry(tools::JsonWriter& rWriter, const RecordedEntry& rRecord,
                           bool bDrawing, drawinglayer::Primitive2dJsonProcessor& rProcessor)
{
    const Entry& rEntry = rRecord.maEntry;
    const SwRect& rBox = rRecord.maPageBox;

    auto aNode = rWriter.startStruct();
    rWriter.put("id", rEntry.mnId);
    if (rEntry.moParent)
        rWriter.put("parent", *rEntry.moParent);
    else
        rWriter.put("parent", constNoParentId);
    rWriter.put("kind", nameOf(rEntry.meKind));
    const tools::Rectangle aBox(rBox.Pos(), rBox.SSize());
    svx::vector::writeEntryBox(rWriter, aBox, svx::vector::boxTransformation(aBox));
    if (rEntry.mbIsFollow)
        rWriter.put("isFollow", true);
    if (rEntry.mbHasFollow)
        rWriter.put("hasFollow", true);
    if (rEntry.moAnchor)
        rWriter.put("anchor", *rEntry.moAnchor);

    // An entry that only moved goes without primitives, and the reader keeps the ones it has.
    if (bDrawing)
    {
        auto aPrimitives = rWriter.startArray("primitives");
        rProcessor.decomposeAndWrite(rEntry.maPrimitives);
    }
}

void PartStore::write(tools::JsonWriter& rWriter, const SwRootFrame& rLayout,
                      std::string_view rPartId, sal_Int64 nSinceVersion, bool bPush)
{
    // A push goes out only for a part that a client holds.
    if (bPush && !isServed(rPartId))
        return;

    // A page is decomposed while the layout is idle. While it is busy nothing is written, and a
    // push is kept as a deferred push.
    if (!isLayoutIdle(rLayout))
    {
        if (bPush
            && std::find(maDeferredPushes.begin(), maDeferredPushes.end(), rPartId)
                   == maDeferredPushes.end())
            maDeferredPushes.emplace_back(rPartId);
        return;
    }

    const SwPageFrame* pPage = pageOf(rLayout, rPartId);
    auto aWriteCommonHeader = [&](bool bDelta) {
        svx::vector::writeAnswerHeader(rWriter, bDelta, constModePrintLayout, rPartId,
                                       getEpoch());
        // The primitives of an entry are placed relative to the top left of its box.
        rWriter.put("coordinates", "entry");
    };

    if (!pPage)
    {
        // A page that is gone is answered with the header alone, and the store drops it with its
        // versions. Every frame has an id of its own, so the part id names no page again.
        aWriteCommonHeader(/*bDelta=*/false);
        forget(rPartId);
        maParts.erase(OString(rPartId));
        return;
    }

    const OString aPartId(rPartId);
    PartState& rState = maParts[aPartId];
    nSinceVersion = rState.maVersions.stepFrom(nSinceVersion, bPush);
    const bool bDelta = nSinceVersion >= 0;

    // An entry of the page that another page holds by now left the page, even when no change
    // marked the page yet.
    const bool bEntryLeft
        = std::any_of(rState.maOrder.begin(), rState.maOrder.end(), [&](sal_uInt64 nId) {
              auto aIterator = maEntries.find(recordKeyOf(nId, *pPage));
              return aIterator != maEntries.end() && aIterator->second.maPartId != aPartId;
          });

    // A full answer compares the page with its record, and so does a delta for a dirty page.
    if (rState.mbDirty || bEntryLeft || !bDelta)
        refresh(rState, *pPage);

    if (bPush && !rState.maVersions.isChangedSince(nSinceVersion))
        return;

    aWriteCommonHeader(bDelta);
    svx::vector::writeAnswerVersions(rWriter, nSinceVersion, rState.maVersions.getVersion());

    if (bDelta && rState.maVersions.getOrderVersion() > o3tl::make_unsigned(nSinceVersion))
    {
        auto aOrder = rWriter.startArray("order");
        for (const sal_uInt64 nId : rState.maOrder)
            rWriter.putSimpleValue(nId);
    }

    drawinglayer::Primitive2dJsonProcessor aProcessor(rWriter);
    maResources.attachTo(aProcessor);
    {
        auto aObjects = rWriter.startArray("objects");
        for (const sal_uInt64 nId : rState.maOrder)
        {
            const sal_uInt64 nKey = recordKeyOf(nId, *pPage);
            auto aIterator = maEntries.find(nKey);
            if (aIterator == maEntries.end())
                continue;
            const RecordedEntry& rRecord = aIterator->second;

            if (!bDelta || rRecord.mnDrawnVersion > o3tl::make_unsigned(nSinceVersion))
                writeEntry(rWriter, rRecord, /*bDrawing=*/true, aProcessor);
            else if (rRecord.mnPlacedVersion > o3tl::make_unsigned(nSinceVersion))
                writeEntry(rWriter, rRecord, /*bDrawing=*/false, aProcessor);
        }
    }

    rState.maVersions.noteWritten(bPush);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
