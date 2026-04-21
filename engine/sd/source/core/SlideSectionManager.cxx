/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SlideSectionManager.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/xmltools.hxx>
#include <svx/svdmodel.hxx>

using namespace ::com::sun::star;

namespace sd
{
SlideSectionManager::SlideSectionManager(SdDrawDocument& rDoc)
    : mrDoc(rDoc)
{
}

void SlideSectionManager::SetSectionsFromPropertyValues(
    const uno::Sequence<beans::PropertyValue>& rSections)
{
    maSections.clear();

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
        uno::Sequence<beans::PropertyValue> aSectionProps;
        rSectionProp.Value >>= aSectionProps;

        SlideSection aSection;
        uno::Sequence<OUString> aSlideNames;

        for (const auto& rProp : aSectionProps)
        {
            if (rProp.Name == "Name")
                rProp.Value >>= aSection.maName;
            else if (rProp.Name == "Id")
                rProp.Value >>= aSection.maId;
            else if (rProp.Name == "SlideNameList")
                rProp.Value >>= aSlideNames;
        }

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
}

uno::Sequence<beans::PropertyValue> SlideSectionManager::GetSectionsAsPropertyValues() const
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
}

void SlideSectionManager::RemoveSection(sal_Int32 nSectionIndex)
{
    if (nSectionIndex < 0 || nSectionIndex >= static_cast<sal_Int32>(maSections.size()))
        return;

    maSections.erase(maSections.begin() + nSectionIndex);
}

void SlideSectionManager::RenameSection(sal_Int32 nSectionIndex, const OUString& rNewName)
{
    if (nSectionIndex < 0 || nSectionIndex >= static_cast<sal_Int32>(maSections.size()))
        return;

    maSections[nSectionIndex].maName = rNewName;
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

    // 7. Recalculate start indices from the (now correctly ordered) slide counts
    sal_Int32 nCurrentIdx = 0;
    for (size_t i = 0; i < maSections.size(); ++i)
    {
        maSections[i].mnStartIndex = nCurrentIdx;
        nCurrentIdx += aSlideCounts[i];
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
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
