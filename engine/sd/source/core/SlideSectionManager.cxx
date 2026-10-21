/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SlideSectionManager.hxx>
#include <DrawDocShell.hxx>
#include <SlideLink.hxx>
#include <UndoSlideSection.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <unomodel.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>

#include <algorithm>
#include <numeric>

#include <COKit/COKit.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/xmltools.hxx>
#include <sal/log.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/undo.hxx>
#include <svx/svdmodel.hxx>
#include <tools/json_writer.hxx>

using namespace ::com::sun::star;

namespace
{
bool isInUndoRedo(const SdDrawDocument& rDoc)
{
    sd::DrawDocShell* pShell = rDoc.GetDocSh();
    if (!pShell)
        return false;
    SfxUndoManager* pMgr = pShell->GetUndoManager();
    return pMgr && pMgr->IsDoing();
}
}

namespace sd
{
SlideSectionManager::SlideSectionManager(SdDrawDocument& rDoc)
    : mrDoc(rDoc)
{
}

void SlideSectionManager::SetSectionsFromPropertyValues(
    const cpo::uno::Sequence<beans::PropertyValue>& rSections)
{
    maSections.clear();
    maAnnouncedLinks.clear();

    if (!rSections.hasElements())
        return;

    // Build page-name-to-index map
    sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);
    std::map<OUString, sal_Int32> aNameToIndex;
    for (sal_uInt16 i = 0; i < nPageCount; ++i)
    {
        SdPage* pPage = mrDoc.GetSdPage(i, PageKind::Standard);
        if (pPage)
            aNameToIndex[pPage->GetName()] = i;
    }

    // Running index for position-based fallback when names don't match
    sal_Int32 nFallbackIdx = 0;
    for (const auto& rSectionProp : rSections)
    {
        cpo::uno::Sequence<beans::PropertyValue> aSectionProps;
        rSectionProp.Value >>= aSectionProps;

        SlideSection aSection;
        cpo::uno::Sequence<OUString> aSlideNames;
        cpo::uno::Sequence<OUString> aSourcePageGuids;

        for (const auto& rProp : aSectionProps)
        {
            if (rProp.Name == "Name")
                rProp.Value >>= aSection.maName;
            else if (rProp.Name == "Id")
                rProp.Value >>= aSection.maId;
            else if (rProp.Name == "SlideNameList")
                rProp.Value >>= aSlideNames;
            else if (rProp.Name == "SourceReference")
                rProp.Value >>= aSection.maLink.maSourceReference;
            else if (rProp.Name == "SourceSectionId")
                rProp.Value >>= aSection.maLink.maSourceSectionId;
            else if (rProp.Name == "SourceSectionName")
                rProp.Value >>= aSection.maLink.maSourceSectionName;
            else if (rProp.Name == "SourceModifiedTime")
                rProp.Value >>= aSection.maLink.maSourceModifiedTime;
            else if (rProp.Name == "SourcePageGuidList")
                rProp.Value >>= aSourcePageGuids;
        }
        aSection.maLink.maSourcePageGuids
            = comphelper::sequenceToContainer<std::vector<OUString>>(aSourcePageGuids);

        // The section starts at the first slide in its list.
        // Try name lookup first; fall back to position if names don't match
        // (e.g. after ODP round-trip where page names change).
        if (aSlideNames.hasElements())
        {
            auto it = aNameToIndex.find(aSlideNames[0]);
            if (it != aNameToIndex.end())
                aSection.mnStartIndex = it->second;
            else
                aSection.mnStartIndex = nFallbackIdx;
        }
        else
        {
            aSection.mnStartIndex = nFallbackIdx;
        }

        nFallbackIdx += aSlideNames.getLength();
        maSections.push_back(aSection);
    }

    // Sort by start index
    std::sort(maSections.begin(), maSections.end(),
              [](const SlideSection& a, const SlideSection& b) {
                  return a.mnStartIndex < b.mnStartIndex;
              });

    // A link is read with the slides the section spans once every section has its start, and a
    // link that does not describe those slides is dropped, so the section keeps its slides alone.
    for (sal_Int32 i = 0; i < GetSectionCount(); ++i)
    {
        SlideSectionLink& rLink = maSections[i].maLink;
        if (!rLink.IsLinked())
            continue;

        rLink = ValidateLink(rLink, i);
        SAL_WARN_IF(!rLink.IsLinked(), "sd", "dropping the unreadable link of a slide section");
    }

    // The sections read here are the ones the document starts with, so a later change is announced
    // against them.
    maAnnouncedLinks = GetLinkedSectionIds();
}

cpo::uno::Sequence<beans::PropertyValue> SlideSectionManager::GetSectionsAsPropertyValues() const
{
    sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);

    std::vector<beans::PropertyValue> aSectionsList;
    for (size_t i = 0; i < maSections.size(); ++i)
    {
        const SlideSection& rSection = maSections[i];

        // Determine end index (start of next section, or page count)
        sal_Int32 nEndIndex
            = (i + 1 < maSections.size()) ? maSections[i + 1].mnStartIndex : nPageCount;

        // Collect slide names for this section
        std::vector<OUString> aSlideNames;
        for (sal_Int32 j = rSection.mnStartIndex; j < nEndIndex; ++j)
        {
            SdPage* pPage = mrDoc.GetSdPage(static_cast<sal_uInt16>(j), PageKind::Standard);
            if (pPage)
                aSlideNames.push_back(pPage->GetName());
        }

        // Build section properties
        std::vector<beans::PropertyValue> aProps;
        aProps.push_back(comphelper::makePropertyValue(u"Name"_ustr, rSection.maName));
        if (!rSection.maId.isEmpty())
            aProps.push_back(comphelper::makePropertyValue(u"Id"_ustr, rSection.maId));
        aProps.push_back(comphelper::makePropertyValue(
            u"SlideNameList"_ustr, comphelper::containerToSequence(aSlideNames)));

        if (const SlideSectionLink* pLink = GetSectionLink(i))
        {
            aProps.push_back(
                comphelper::makePropertyValue(u"SourceReference"_ustr, pLink->maSourceReference));
            aProps.push_back(
                comphelper::makePropertyValue(u"SourceSectionId"_ustr, pLink->maSourceSectionId));
            aProps.push_back(comphelper::makePropertyValue(u"SourceSectionName"_ustr,
                                                           pLink->maSourceSectionName));
            if (!pLink->maSourceModifiedTime.isEmpty())
                aProps.push_back(comphelper::makePropertyValue(u"SourceModifiedTime"_ustr,
                                                               pLink->maSourceModifiedTime));
            aProps.push_back(comphelper::makePropertyValue(
                u"SourcePageGuidList"_ustr,
                comphelper::containerToSequence(pLink->maSourcePageGuids)));
        }

        aSectionsList.push_back(comphelper::makePropertyValue(
            u"Section"_ustr + OUString::number(i), comphelper::containerToSequence(aProps)));
    }

    return comphelper::containerToSequence(aSectionsList);
}

sal_Int32 SlideSectionManager::GetSectionCount() const { return maSections.size(); }

const SlideSection& SlideSectionManager::GetSection(sal_Int32 nIndex) const
{
    assert(nIndex >= 0 && o3tl::make_unsigned(nIndex) < maSections.size());
    return maSections[nIndex];
}

sal_Int32 SlideSectionManager::GetSectionSlideCount(sal_Int32 nSectionIndex) const
{
    if (nSectionIndex < 0 || nSectionIndex >= GetSectionCount())
        return 0;

    const sal_Int32 nEnd = (nSectionIndex + 1 < GetSectionCount())
                               ? maSections[nSectionIndex + 1].mnStartIndex
                               : static_cast<sal_Int32>(mrDoc.GetSdPageCount(PageKind::Standard));
    return std::max<sal_Int32>(0, nEnd - maSections[nSectionIndex].mnStartIndex);
}

std::vector<OUString> SlideSectionManager::GetSectionPageGuids(sal_Int32 nSectionIndex) const
{
    std::vector<OUString> aGuids;
    const sal_Int32 nCount = GetSectionSlideCount(nSectionIndex);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        const SdPage* pPage
            = mrDoc.GetSdPage(static_cast<sal_uInt16>(maSections[nSectionIndex].mnStartIndex + i),
                              PageKind::Standard);
        if (pPage)
            aGuids.push_back(pPage->GetGuid().getOUString());
    }
    return aGuids;
}

SlideSectionLink SlideSectionManager::ValidateLink(const SlideSectionLink& rLink,
                                                   sal_Int32 nSectionIndex) const
{
    if (!rLink.IsLinked() || SlideLink::GetSourceName(rLink.maSourceReference).isEmpty())
        return SlideSectionLink();

    if (!SlideLink::IsReadableSourceString(rLink.maSourceModifiedTime)
        || !SlideLink::IsReadableSourceString(rLink.maSourceSectionName))
        return SlideSectionLink();

    SlideSectionLink aLink(rLink);
    aLink.maPageGuids = GetSectionPageGuids(nSectionIndex);
    if (aLink.maPageGuids.empty() || aLink.maPageGuids.size() != aLink.maSourcePageGuids.size())
        return SlideSectionLink();

    aLink.maSourceSectionId = SlideLink::NormaliseGuid(rLink.maSourceSectionId);
    if (aLink.maSourceSectionId.isEmpty())
        return SlideSectionLink();

    for (OUString& rGuid : aLink.maSourcePageGuids)
    {
        rGuid = SlideLink::NormaliseGuid(rGuid);
        if (rGuid.isEmpty())
            return SlideSectionLink();
    }

    return aLink;
}

const SlideSectionLink* SlideSectionManager::GetSectionLink(sal_Int32 nSectionIndex) const
{
    if (nSectionIndex < 0 || nSectionIndex >= GetSectionCount())
        return nullptr;

    const SlideSectionLink& rLink = maSections[nSectionIndex].maLink;
    if (!rLink.IsLinked() || rLink.maPageGuids != GetSectionPageGuids(nSectionIndex))
        return nullptr;

    return &rLink;
}

bool SlideSectionManager::SetSectionLink(sal_Int32 nSectionIndex, const SlideSectionLink& rLink)
{
    if (nSectionIndex < 0 || nSectionIndex >= GetSectionCount())
        return false;

    SlideSectionLink aLink;
    if (rLink.IsLinked())
    {
        aLink = ValidateLink(rLink, nSectionIndex);
        if (!aLink.IsLinked())
            return false;
    }

    std::vector<SlideSection> aSnapshot;
    const bool bUndoEnabled = mrDoc.IsUndoEnabled();
    if (bUndoEnabled)
        aSnapshot = GetSectionsSnapshot();

    maSections[nSectionIndex].maLink = std::move(aLink);

    if (bUndoEnabled)
        mrDoc.AddUndo(std::make_unique<UndoSlideSection>(
            mrDoc,
            SdResId(rLink.IsLinked() ? STR_UNDO_LINK_SLIDE_SECTION : STR_UNDO_UNLINK_SLIDE_SECTION),
            std::move(aSnapshot)));
    NotifySectionsChanged();
    return true;
}

bool SlideSectionManager::AddInsertedSection(sal_Int32 nStartSlideIndex, sal_Int32 nSlideCount,
                                             const OUString& rName, const SlideSectionLink& rLink)
{
    const sal_Int32 nDeckSlideCount = mrDoc.GetSdPageCount(PageKind::Standard);
    const sal_Int32 nEnd = nStartSlideIndex + nSlideCount;
    if (nStartSlideIndex < 0 || nSlideCount <= 0 || nEnd > nDeckSlideCount)
        return false;

    // The new section has an undo action of its own, recorded before the one SetSectionLink
    // records for the link, so that an undo takes the link off first and the section after it.
    const std::vector<SlideSection> aSnapshot = GetSectionsSnapshot();
    if (mrDoc.IsUndoEnabled())
        mrDoc.AddUndo(
            std::make_unique<UndoSlideSection>(mrDoc, SdResId(STR_UNDO_ADD_SLIDE_SECTION)));

    // A section spans up to the start of the next one, so the slides that follow the new ones
    // need a section start of their own to stay out of the new section.
    if (nEnd < nDeckSlideCount && !IsSectionStart(nEnd))
    {
        const sal_Int32 nOuter = GetSectionIndexForSlide(nStartSlideIndex);
        if (nOuter >= 0 && maSections[nOuter].mnStartIndex == nStartSlideIndex)
        {
            // No section starts among the new slides, so the sections stay in order.
            maSections[nOuter].mnStartIndex = nEnd;
            NotifySectionsChanged();
        }
        else
            AddSection(nEnd, nOuter >= 0 ? maSections[nOuter].maName
                                         : SdResId(STR_DEFAULT_SLIDE_SECTION_NAME));
    }

    if (IsSectionStart(nStartSlideIndex))
    {
        RestoreSectionsSnapshot(aSnapshot);
        return false;
    }
    AddSection(nStartSlideIndex, rName);

    if (rLink.IsLinked() && !SetSectionLink(GetSectionIndexForSlide(nStartSlideIndex), rLink))
    {
        RestoreSectionsSnapshot(aSnapshot);
        return false;
    }
    return true;
}

std::vector<OUString> SlideSectionManager::GetLinkedSectionIds() const
{
    std::vector<OUString> aLinks;
    for (sal_Int32 i = 0; i < GetSectionCount(); ++i)
    {
        if (GetSectionLink(i))
            aLinks.push_back(maSections[i].maId);
    }
    return aLinks;
}

void SlideSectionManager::NotifyLinksIfChanged()
{
    std::vector<OUString> aLinks = GetLinkedSectionIds();
    if (aLinks == maAnnouncedLinks)
        return;

    maAnnouncedLinks = std::move(aLinks);
    SlideLink::NotifyLinksChanged(mrDoc);
}

void SlideSectionManager::ReplaceLinkedPage(const OUString& rOldPageGuid,
                                            const OUString& rNewPageGuid)
{
    const auto aHoldsOldPage = [&rOldPageGuid](const SlideSection& rSection)
    {
        return std::find(rSection.maLink.maPageGuids.begin(), rSection.maLink.maPageGuids.end(),
                         rOldPageGuid)
               != rSection.maLink.maPageGuids.end();
    };
    if (std::none_of(maSections.begin(), maSections.end(), aHoldsOldPage))
        return;

    // Undoing the replacement brings the old slide back, and the section holds it again.
    if (mrDoc.IsUndoEnabled())
        mrDoc.AddUndo(
            std::make_unique<UndoSlideSection>(mrDoc, SdResId(STR_UNDO_UPDATE_LINKED_SLIDES)));

    for (SlideSection& rSection : maSections)
        std::replace(rSection.maLink.maPageGuids.begin(), rSection.maLink.maPageGuids.end(),
                     rOldPageGuid, rNewPageGuid);
}

void SlideSectionManager::WriteSectionsJson(::tools::JsonWriter& rJsonWriter,
                                            std::string_view aArrayName) const
{
    const sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);
    const sal_Int32 nSectionCount = static_cast<sal_Int32>(maSections.size());
    auto aArray = rJsonWriter.startArray(aArrayName);
    for (sal_Int32 i = 0; i < nSectionCount; ++i)
    {
        const SlideSection& rSection = maSections[i];
        // A section spans up to the next section's first slide, and the last
        // section spans up to the end of the deck.
        const sal_Int32 nSectionSlideCount
            = (i + 1 < nSectionCount) ? maSections[i + 1].mnStartIndex - rSection.mnStartIndex
                                      : nPageCount - rSection.mnStartIndex;
        auto aNode = rJsonWriter.startStruct();
        rJsonWriter.put("name", rSection.maName);
        if (!rSection.maId.isEmpty())
            rJsonWriter.put("id", rSection.maId);
        rJsonWriter.put("startIndex", rSection.mnStartIndex);
        rJsonWriter.put("slideCount", nSectionSlideCount);
    }
}

sal_Int32 SlideSectionManager::GetSectionIndexForSlide(sal_Int32 nSlideIndex) const
{
    // Sections are sorted by start index; find the last section whose start <= nSlideIndex
    sal_Int32 nResult = -1;
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(maSections.size()); ++i)
    {
        if (maSections[i].mnStartIndex <= nSlideIndex)
            nResult = i;
        else
            break;
    }
    return nResult;
}

bool SlideSectionManager::IsSectionStart(sal_Int32 nSlideIndex) const
{
    for (const auto& rSection : maSections)
    {
        if (rSection.mnStartIndex == nSlideIndex)
            return true;
    }
    return false;
}

std::vector<sal_Int32> SlideSectionManager::GetSectionStartIndices() const
{
    std::vector<sal_Int32> aResult;
    aResult.reserve(maSections.size());
    for (const auto& rSection : maSections)
        aResult.push_back(rSection.mnStartIndex);
    return aResult;
}

void SlideSectionManager::AddSection(sal_Int32 nStartSlideIndex, const OUString& rName)
{
    SlideSection aNewSection;
    aNewSection.maName = rName;
    aNewSection.maId
        = OStringToOUString(comphelper::xml::generateGUIDString(), RTL_TEXTENCODING_ASCII_US);
    aNewSection.mnStartIndex = nStartSlideIndex;

    maSections.push_back(aNewSection);

    // Re-sort
    std::sort(maSections.begin(), maSections.end(),
              [](const SlideSection& a, const SlideSection& b) {
                  return a.mnStartIndex < b.mnStartIndex;
              });

    NotifySectionsChanged();
}

void SlideSectionManager::RemoveSection(sal_Int32 nSectionIndex)
{
    if (nSectionIndex < 0 || nSectionIndex >= static_cast<sal_Int32>(maSections.size()))
        return;

    maSections.erase(maSections.begin() + nSectionIndex);

    NotifySectionsChanged();
}

void SlideSectionManager::RemoveSectionSlides(sal_Int32 nSectionIndex)
{
    if (nSectionIndex < 0 || nSectionIndex >= static_cast<sal_Int32>(maSections.size()))
        return;

    // Compute the [nStart, nEnd) range of slides in this section.  The
    // section list is sorted by start index, so the next section's start
    // (or the document's slide count) marks the end of this section.
    const sal_Int32 nStart = maSections[nSectionIndex].mnStartIndex;
    const sal_Int32 nEnd
        = (nSectionIndex + 1 < static_cast<sal_Int32>(maSections.size()))
              ? maSections[nSectionIndex + 1].mnStartIndex
              : static_cast<sal_Int32>(mrDoc.GetSdPageCount(PageKind::Standard));

    try
    {
        rtl::Reference<SdXImpressDocument> xModel(mrDoc.getUnoModel());
        if (!xModel.is())
            return;

        cpo::uno::Reference<css::drawing::XDrawPages> xPages(xModel->getDrawPages(),
                                                             cpo::uno::UNO_SET_THROW);

        // Iterate in reverse so that each remove() does not shift the
        // indices of the slides we still have to delete.  OnSlideRemoved
        // fires per page delete and auto-erases the now-empty section
        // metadata, so we deliberately do not call RemoveSection() here.
        for (sal_Int32 i = nEnd - 1; i >= nStart; --i)
        {
            // Do not delete the last slide in the document.
            if (xPages->getCount() <= 1)
                break;

            cpo::uno::Reference<css::drawing::XDrawPage> xPage(xPages->getByIndex(i),
                                                               cpo::uno::UNO_QUERY_THROW);
            xPages->remove(xPage);
        }
    }
    catch (const cpo::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("sd", "SlideSectionManager::RemoveSectionSlides()");
    }
}

void SlideSectionManager::RenameSection(sal_Int32 nSectionIndex, const OUString& rNewName)
{
    if (nSectionIndex < 0 || nSectionIndex >= static_cast<sal_Int32>(maSections.size()))
        return;

    maSections[nSectionIndex].maName = rNewName;

    NotifySectionsChanged();
}

void SlideSectionManager::MoveSection(sal_Int32 nOldIndex, sal_Int32 nNewIndex)
{
    if (nOldIndex < 0 || nOldIndex >= static_cast<sal_Int32>(maSections.size()))
        return;
    if (nNewIndex < 0 || nNewIndex >= static_cast<sal_Int32>(maSections.size()))
        return;
    if (nOldIndex == nNewIndex)
        return;

    sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);

    // 1. Capture slide counts per section BEFORE any moves
    std::vector<sal_Int32> aSlideCounts;
    for (size_t i = 0; i < maSections.size(); ++i)
    {
        sal_Int32 nEnd = (i + 1 < maSections.size()) ? maSections[i + 1].mnStartIndex
                                                     : static_cast<sal_Int32>(nPageCount);
        aSlideCounts.push_back(nEnd - maSections[i].mnStartIndex);
    }

    // 2. Calculate slide range for the source section
    sal_Int32 nSrcStart = maSections[nOldIndex].mnStartIndex;
    sal_Int32 nSrcEnd = nSrcStart + aSlideCounts[nOldIndex];
    sal_Int32 nSrcCount = aSlideCounts[nOldIndex];

    if (nSrcCount <= 0)
        return;

    // 3. Mark the source section's slides as selected
    mrDoc.UnselectAllPages();
    for (sal_Int32 i = nSrcStart; i < nSrcEnd; ++i)
    {
        SdPage* pPage = mrDoc.GetSdPage(static_cast<sal_uInt16>(i), PageKind::Standard);
        if (pPage)
            mrDoc.SetSelected(pPage, true);
    }

    // 4. Calculate the target position
    // MoveSelectedPages takes 0-based page index to insert after (-1 = before first)
    sal_uInt16 nTargetPage;
    if (nNewIndex < nOldIndex)
    {
        // Moving up: place before the target section's first slide
        nTargetPage = (maSections[nNewIndex].mnStartIndex > 0)
                          ? static_cast<sal_uInt16>(maSections[nNewIndex].mnStartIndex - 1)
                          : sal_uInt16(-1);
    }
    else
    {
        // Moving down: place after the target section's last slide
        // MovePages() interprets nTargetPage as a 0-based standard-page index
        // in the current ordering (it does NOT pre-remove the selected pages),
        // so we pass the index of the target section's last slide directly.
        sal_Int32 nTargetEnd = maSections[nNewIndex].mnStartIndex + aSlideCounts[nNewIndex];
        nTargetPage = static_cast<sal_uInt16>(nTargetEnd - 1);
    }

    // 5. Move the pages in the document
    mrDoc.MoveSelectedPages(nTargetPage);

    // 6. Reorder section entries and their slide counts to match new order
    SlideSection aMovedSection = maSections[nOldIndex];
    sal_Int32 nMovedCount = aSlideCounts[nOldIndex];

    maSections.erase(maSections.begin() + nOldIndex);
    aSlideCounts.erase(aSlideCounts.begin() + nOldIndex);

    // After erasing nOldIndex, elements after it shift down by 1.
    // To place the moved section at nNewIndex in the original ordering:
    // - Moving up (nNewIndex < nOldIndex): insert at nNewIndex (no shift needed)
    // - Moving down (nNewIndex > nOldIndex): the target shifted down by 1,
    //   but we want to insert AFTER it, so nNewIndex - 1 + 1 = nNewIndex
    sal_Int32 nInsertAt = nNewIndex;
    maSections.insert(maSections.begin() + nInsertAt, aMovedSection);
    aSlideCounts.insert(aSlideCounts.begin() + nInsertAt, nMovedCount);

    // 7. Recalculate start indices from the (now correctly ordered) slide
    //    counts. Slides before the first section ("leading" slides that belong
    //    to no section) keep their place at the top of the deck, so the first
    //    section does not start at slide 0. Offset every recomputed start by
    //    that leading count. sum(aSlideCounts) is invariant across the reorder,
    //    so nPageCount - sum gives the leading-orphan count.
    sal_Int32 nSectionedSlides
        = std::accumulate(aSlideCounts.begin(), aSlideCounts.end(), sal_Int32(0));
    sal_Int32 nCurrentIdx = static_cast<sal_Int32>(nPageCount) - nSectionedSlides;
    for (size_t i = 0; i < maSections.size(); ++i)
    {
        maSections[i].mnStartIndex = nCurrentIdx;
        nCurrentIdx += aSlideCounts[i];
    }

    NotifySectionsChanged();
}

void SlideSectionManager::OnSlideInserted(sal_Int32 nNewStandardIndex)
{
    if (maSections.empty())
        return;

    // An undo puts the sections back on its own, but the slides a linked section spans change
    // here, which can give the section its link back.
    if (isInUndoRedo(mrDoc))
    {
        NotifyLinksIfChanged();
        return;
    }

    const bool bUndoEnabled = mrDoc.IsUndoEnabled();
    std::vector<SlideSection> aSnapshot;
    if (bUndoEnabled)
        aSnapshot = GetSectionsSnapshot();

    bool bChanged = false;
    for (auto& rSec : maSections)
    {
        if (rSec.mnStartIndex < nNewStandardIndex)
            continue;

        // Keep first section at 0 so the new slide is not orphaned.
        if (nNewStandardIndex == 0 && &rSec == &maSections.front()
            && rSec.mnStartIndex == 0)
            continue;

        rSec.mnStartIndex += 1;
        bChanged = true;
    }

    if (bChanged)
    {
        if (bUndoEnabled)
            mrDoc.AddUndo(std::make_unique<UndoSlideSection>(
                mrDoc, SdResId(STR_UNDO_INSERTPAGES), std::move(aSnapshot)));
        NotifySectionsChanged();
    }
    else
    {
        // The slide went into a section, which unlinks that section when it is linked.
        NotifyLinksIfChanged();
    }
}

void SlideSectionManager::OnSlideRemoved(sal_Int32 nOldStandardIndex)
{
    if (maSections.empty())
        return;

    // An undo puts the sections back on its own, but the slides a linked section spans change
    // here, which can give the section its link back.
    if (isInUndoRedo(mrDoc))
    {
        NotifyLinksIfChanged();
        return;
    }

    const bool bUndoEnabled = mrDoc.IsUndoEnabled();
    std::vector<SlideSection> aSnapshot;
    if (bUndoEnabled)
        aSnapshot = GetSectionsSnapshot();

    bool bChanged = false;
    for (auto& rSec : maSections)
    {
        if (rSec.mnStartIndex > nOldStandardIndex)
        {
            rSec.mnStartIndex -= 1;
            bChanged = true;
        }
    }

    const sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);
    auto it = maSections.begin();
    while (it != maSections.end())
    {
        const sal_Int32 nEnd = ((it + 1) != maSections.end())
                                   ? (it + 1)->mnStartIndex
                                   : static_cast<sal_Int32>(nPageCount);
        if (nEnd <= it->mnStartIndex)
        {
            it = maSections.erase(it);
            bChanged = true;
        }
        else
        {
            ++it;
        }
    }

    if (bChanged)
    {
        if (bUndoEnabled)
            mrDoc.AddUndo(std::make_unique<UndoSlideSection>(
                mrDoc, SdResId(STR_UNDO_DELETEPAGES), std::move(aSnapshot)));
        NotifySectionsChanged();
    }
    else
    {
        // The slide left a section, which unlinks that section when it is linked.
        NotifyLinksIfChanged();
    }
}

std::vector<SlideSection> SlideSectionManager::GetSectionsSnapshot() const { return maSections; }

void SlideSectionManager::RestoreSectionsSnapshot(const std::vector<SlideSection>& rSections)
{
    maSections = rSections;

    // Broadcast so listeners (e.g. the slide sorter) repaint section headers.
    SdrPage* pPage = mrDoc.GetSdPage(0, PageKind::Standard);
    if (pPage)
    {
        SdrHint aHint(SdrHintKind::PageOrderChange, pPage);
        mrDoc.Broadcast(aHint);
    }

    NotifySectionsChanged();
}

void SlideSectionManager::NotifySectionsChanged()
{
    NotifyLinksIfChanged();

    if (!comphelper::COKit::isActive())
        return;

    SfxViewShell* pCurrent = SfxViewShell::Current();
    if (!pCurrent)
        return;

    ::tools::JsonWriter aWriter;
    aWriter.put("commandName", ".uno:SlideSections");
    WriteSectionsJson(aWriter, "state");
    const OString aPayload = aWriter.finishAndGetAsOString();

    // Deliver the same simple payload to every view in this document so all
    // collaborators update; NotifyOtherViews wraps the payload in a different
    // shape that the browser's commandstatechanged handler does not expect.
    const ViewShellDocId nDocId = pCurrent->GetDocId();
    SfxViewShell* pView = SfxViewShell::GetFirst();
    while (pView)
    {
        if (pView->GetDocId() == nDocId)
            pView->viewCallback(COKitCallbackType::STATE_CHANGED, aPayload);
        pView = SfxViewShell::GetNext(*pView);
    }
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
