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

#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <o3tl/safeint.hxx>
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

/// The most areas a part keeps as the places where its page may differ from what was compared
/// last. Past this many, the whole page is compared.
constexpr size_t constMostDirtyAreas = 64;

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
}

OString partIdOf(const SwPageFrame& rPage) { return OString::number(rPage.GetFrameId()); }

const SwPageFrame* pageOf(const SwRootFrame& rLayout, std::string_view rPartId)
{
    for (const SwFrame* pFrame = rLayout.Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (pFrame->IsPageFrame() && partIdOf(*static_cast<const SwPageFrame*>(pFrame)) == rPartId)
            return static_cast<const SwPageFrame*>(pFrame);
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
    std::vector<OString> aPageIds = partIdsOf(rLayout);
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

std::vector<OString> PartStore::invalidate(const SwRootFrame& rLayout,
                                           const std::vector<SwRect>& rAreas)
{
    std::vector<OString> aChanged;
    std::unordered_set<OString> aPresent;
    for (const SwFrame* pFrame = rLayout.Lower(); pFrame; pFrame = pFrame->GetNext())
    {
        if (!pFrame->IsPageFrame())
            continue;

        const OString aPartId = partIdOf(*static_cast<const SwPageFrame*>(pFrame));
        aPresent.insert(aPartId);
        auto aIterator = maParts.find(aPartId);
        if (aIterator == maParts.end() || !aIterator->second.maVersions.isServed())
            continue;

        // The objects of a page can reach past it, so the page is marked when an area touches
        // the page or any entry it holds.
        bool bTouched = std::any_of(rAreas.begin(), rAreas.end(), [pFrame](const SwRect& rArea) {
            return rArea.Overlaps(pFrame->getFrameArea());
        });
        for (size_t i = 0; !bTouched && i < aIterator->second.maEntryIds.size(); ++i)
        {
            auto aEntryIterator = maEntries.find(aIterator->second.maEntryIds[i]);
            if (aEntryIterator == maEntries.end())
                continue;
            const SwRect& rBox = aEntryIterator->second.maEntry.maBox;
            bTouched = std::any_of(rAreas.begin(), rAreas.end(),
                                   [&rBox](const SwRect& rArea) { return rArea.Overlaps(rBox); });
        }

        if (bTouched)
        {
            // A page that is dirty all over stays so. The areas add up until the page is compared
            // again, so past constMostDirtyAreas the whole page counts as dirty.
            PartState& rState = aIterator->second;
            if (!rState.mbDirty)
                rState.maDirtyAreas = rAreas;
            else if (!rState.maDirtyAreas.empty())
                rState.maDirtyAreas.insert(rState.maDirtyAreas.end(), rAreas.begin(),
                                           rAreas.end());
            if (rState.maDirtyAreas.size() > constMostDirtyAreas)
                rState.maDirtyAreas.clear();
            rState.mbDirty = true;
            aChanged.push_back(aPartId);
        }
    }

    // A served page that is gone is reported too. A gone page that no client holds is dropped
    // with its versions, since its part id names no page again.
    for (auto aIterator = maParts.begin(); aIterator != maParts.end();)
    {
        PartState& rState = aIterator->second;
        if (aPresent.contains(aIterator->first))
            ++aIterator;
        else if (!rState.maVersions.isServed())
            aIterator = maParts.erase(aIterator);
        else
        {
            rState.mbDirty = true;
            rState.maDirtyAreas.clear();
            aChanged.push_back(aIterator->first);
            ++aIterator;
        }
    }
    return aChanged;
}

void PartStore::refresh(PartState& rState, const SwPageFrame& rPage, bool bWhole)
{
    rState.mbDirty = false;
    const std::vector<SwRect> aDirtyAreas = std::move(rState.maDirtyAreas);
    rState.maDirtyAreas.clear();
    const bool bCompareAll = bWhole || aDirtyAreas.empty();
    auto aIsInDirtyArea = [&aDirtyAreas](const SwRect& rBox) {
        return rBox.IsEmpty()
               || std::any_of(aDirtyAreas.begin(), aDirtyAreas.end(),
                              [&rBox](const SwRect& rArea) { return rArea.Overlaps(rBox); });
    };

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
        // What an entry draws is compared where it moved or where its box lies in an area that
        // changed. An entry elsewhere draws what it drew before.
        const bool bCompareDrawing = bCompareAll || bPlaceChanged
                                     || aIsInDirtyArea(rRecord.maEntry.maBox)
                                     || aIsInDirtyArea(rEntry.maBox);
        const bool bDrawingChanged
            = bNewHere || (bCompareDrawing && !sameDrawing(rRecord.maEntry, rEntry));

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
        std::erase_if(maEntries, [&rPartId](const auto& rPair)
                      { return rPair.second.maPartId == rPartId; });
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

    // A full answer compares the whole page, and a dirty page is compared where it changed.
    if (rState.mbDirty || bEntryLeft || !bDelta)
        refresh(rState, *pPage, /*bWhole=*/!bDelta);

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
