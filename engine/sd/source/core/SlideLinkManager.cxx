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

#include <SlideLinkManager.hxx>

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

#include <COKit/COKit.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <svl/undo.hxx>
#include <tools/json_writer.hxx>

#include <DrawDocShell.hxx>
#include <SlideLink.hxx>
#include <SlideSectionManager.hxx>
#include <UndoSlideLink.hxx>
#include <UndoSlideSection.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <unomodel.hxx>

using namespace ::com::sun::star;

namespace sd
{
namespace
{
constexpr OUString gSlideKindName = u"slide"_ustr;
constexpr OUString gSectionKindName = u"section"_ustr;

/// The slide of rSourceDoc that the slide link rRecord follows: by the identifier of the source
/// slide, by its position, or by its name. Nothing when the file holds no slide for it.
const SdPage* findSourcePage(const SdDrawDocument& rSourceDoc, const SlideLinkRecord& rRecord)
{
    const sal_uInt16 nCount = rSourceDoc.GetSdPageCount(PageKind::Standard);

    if (rRecord.maSourceId.startsWith("{"))
    {
        const sal_Int32 nIndex = SlideLink::GetPageIndex(rSourceDoc, rRecord.maSourceId);
        if (nIndex >= 0)
            return rSourceDoc.GetSdPage(static_cast<sal_uInt16>(nIndex), PageKind::Standard);
    }

    // A link to the position of a slide reads the slide standing there, whatever that slide is
    // named now.
    const sal_Int32 nPosition = SlideLink::GetNamedPosition(rRecord.maSourceId);
    if (nPosition > 0)
        return nPosition <= nCount ? rSourceDoc.GetSdPage(static_cast<sal_uInt16>(nPosition - 1),
                                                          PageKind::Standard)
                                   : nullptr;

    if (!rRecord.maName.isEmpty())
    {
        for (sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            const SdPage* pSourcePage = rSourceDoc.GetSdPage(nIndex, PageKind::Standard);
            if (pSourcePage && pSourcePage->GetName() == rRecord.maName)
                return pSourcePage;
        }
    }

    return nullptr;
}

/// The name a slide link records for a slide named rName: rName itself when it is a name the slide
/// has of its own and the record can keep it, and none otherwise.
OUString getOwnRecordableName(const OUString& rName)
{
    return SlideLink::GetNamedPosition(rName) == 0 && SlideLink::IsReadableSourceString(rName)
               ? rName
               : OUString();
}

/// The name the slide link rRecord records once it has read the slide named rReadName: a link to
/// a position keeps the name of that position, and any other link takes the name the slide has of
/// its own, or none when the slide has none or the record cannot keep it.
OUString getRefreshedName(const SlideLinkRecord& rRecord, const OUString& rReadName)
{
    if (SlideLink::GetNamedPosition(rRecord.maSourceId) > 0)
        return rRecord.maName;

    return getOwnRecordableName(rReadName);
}

/// The key the announced list keeps for one link.
OUString makeKey(const SlideLinkRecord& rRecord)
{
    return GetSlideLinkKindName(rRecord.meKind) + ":" + rRecord.maId;
}

bool isSameUnit(const SlideLinkRecord& rOne, const SlideLinkRecord& rOther)
{
    return rOne.meKind == rOther.meKind && rOne.maId == rOther.maId;
}

/// Reports the slides rSlides as ones the file held nothing for.
void addNotUpdated(std::vector<OString>* pNotUpdated, const std::vector<OUString>& rSlides)
{
    if (!pNotUpdated)
        return;

    for (const OUString& rSlide : rSlides)
        pNotUpdated->push_back(OUStringToOString(rSlide, RTL_TEXTENCODING_ASCII_US));
}

}

OUString GetSlideLinkKindName(SlideLinkKind eKind)
{
    switch (eKind)
    {
        case SlideLinkKind::Slide:
            return gSlideKindName;
        case SlideLinkKind::Section:
            return gSectionKindName;
    }
    return OUString();
}

std::optional<SlideLinkKind> ParseSlideLinkKind(std::u16string_view rName)
{
    if (rName == gSlideKindName)
        return SlideLinkKind::Slide;
    if (rName == gSectionKindName)
        return SlideLinkKind::Section;
    return std::nullopt;
}

SlideLinkManager::SlideLinkManager(SdDrawDocument& rDoc)
    : mrDoc(rDoc)
{
}

cpo::uno::Sequence<beans::PropertyValue> SlideLinkManager::GetLinksAsPropertyValues() const
{
    std::vector<beans::PropertyValue> aList;
    const std::vector<SlideLinkRecord> aLinks = GetLinks();
    for (size_t nLink = 0; nLink < aLinks.size(); ++nLink)
    {
        const SlideLinkRecord& rRecord = aLinks[nLink];
        const std::vector<beans::PropertyValue> aProperties{
            comphelper::makePropertyValue(u"Unit"_ustr, GetSlideLinkKindName(rRecord.meKind)),
            comphelper::makePropertyValue(u"Id"_ustr, rRecord.maId),
            comphelper::makePropertyValue(u"Source"_ustr, rRecord.maSource),
            comphelper::makePropertyValue(u"SourceId"_ustr, rRecord.maSourceId),
            comphelper::makePropertyValue(u"Name"_ustr, rRecord.maName),
            comphelper::makePropertyValue(u"LastModifiedTime"_ustr, rRecord.maLastModifiedTime),
            comphelper::makePropertyValue(u"Slides"_ustr,
                                          comphelper::containerToSequence(rRecord.maSlides))
        };
        aList.push_back(
            comphelper::makePropertyValue(u"Link"_ustr + OUString::number(nLink),
                                          comphelper::containerToSequence(aProperties)));
    }
    return comphelper::containerToSequence(aList);
}

void SlideLinkManager::SetLinksFromPropertyValues(
    const cpo::uno::Sequence<beans::PropertyValue>& rLinks)
{
    maLinks.clear();

    for (const beans::PropertyValue& rLink : rLinks)
    {
        cpo::uno::Sequence<beans::PropertyValue> aProperties;
        rLink.Value >>= aProperties;

        SlideLinkRecord aRecord;
        OUString aUnit;
        cpo::uno::Sequence<OUString> aSlides;
        for (const beans::PropertyValue& rProp : aProperties)
        {
            if (rProp.Name == "Unit")
                rProp.Value >>= aUnit;
            else if (rProp.Name == "Id")
                rProp.Value >>= aRecord.maId;
            else if (rProp.Name == "Source")
                rProp.Value >>= aRecord.maSource;
            else if (rProp.Name == "SourceId")
                rProp.Value >>= aRecord.maSourceId;
            else if (rProp.Name == "Name")
                rProp.Value >>= aRecord.maName;
            else if (rProp.Name == "LastModifiedTime")
                rProp.Value >>= aRecord.maLastModifiedTime;
            else if (rProp.Name == "Slides")
                rProp.Value >>= aSlides;
        }
        aRecord.maSlides = comphelper::sequenceToContainer<std::vector<OUString>>(aSlides);

        const std::optional<SlideLinkKind> oKind = ParseSlideLinkKind(aUnit);
        if (!oKind)
        {
            SAL_WARN("sd", "dropping a slide link of the unknown unit " << aUnit);
            continue;
        }
        aRecord.meKind = *oKind;

        // A link is read against the slides its unit spans once every page and section is in,
        // and a link that does not describe those slides is dropped, so the unit keeps its slides
        // alone.
        std::optional<SlideLinkRecord> oRecord = Validate(aRecord);
        if (!oRecord)
        {
            SAL_WARN("sd", "dropping the unreadable slide link of " << aRecord.maId);
            continue;
        }
        Put({ std::move(*oRecord) });
    }

    // The links read here are the ones the document starts with, so a later change is announced
    // against them.
    maAnnouncedLinks = GetHoldingKeys();
}

std::vector<OUString> SlideLinkManager::GetUnitSlides(const SlideLinkRecord& rRecord) const
{
    switch (rRecord.meKind)
    {
        case SlideLinkKind::Slide:
            if (SlideLink::GetPageIndex(mrDoc, rRecord.maId) >= 0)
                return { rRecord.maId };
            return {};
        case SlideLinkKind::Section:
        {
            const SlideSectionManager& rSections = mrDoc.GetSectionManager();
            return rSections.GetSectionPageGuids(rSections.GetSectionIndexForId(rRecord.maId));
        }
    }
    return {};
}

bool SlideLinkManager::Holds(const SlideLinkRecord& rRecord) const
{
    return !rRecord.maSlides.empty() && rRecord.maSlides == GetUnitSlides(rRecord);
}

bool SlideLinkManager::Holds(const SlideLinkRecord& rRecord,
                             const std::map<OUString, sal_Int32>& rPageIndices) const
{
    if (rRecord.meKind == SlideLinkKind::Slide)
        return rRecord.maSlides.size() == 1 && rRecord.maSlides.front() == rRecord.maId
               && rPageIndices.contains(rRecord.maId);

    return Holds(rRecord);
}

std::optional<SlideLinkRecord> SlideLinkManager::Validate(const SlideLinkRecord& rRecord) const
{
    if (!SlideLink::IsReadablePersistentLink(rRecord.maSource)
        || !SlideLink::IsReadableSourceString(rRecord.maName)
        || !SlideLink::IsReadableSourceString(rRecord.maLastModifiedTime))
        return std::nullopt;

    SlideLinkRecord aRecord(rRecord);
    aRecord.maId = SlideLink::NormaliseGuid(rRecord.maId);
    if (aRecord.maId.isEmpty())
        return std::nullopt;

    switch (aRecord.meKind)
    {
        case SlideLinkKind::Slide:
            // The source slide is found by its identifier, by its position, or by its name, so
            // the link names at least one of those.
            if (rRecord.maSourceId.startsWith("{"))
            {
                aRecord.maSourceId = SlideLink::NormaliseGuid(rRecord.maSourceId);
                if (aRecord.maSourceId.isEmpty())
                    return std::nullopt;
            }
            else if (!rRecord.maSourceId.isEmpty()
                     && SlideLink::GetNamedPosition(rRecord.maSourceId) <= 0)
                return std::nullopt;

            if (aRecord.maSourceId.isEmpty() && aRecord.maName.isEmpty())
                return std::nullopt;
            break;
        case SlideLinkKind::Section:
            aRecord.maSourceId = SlideLink::NormaliseGuid(rRecord.maSourceId);
            if (aRecord.maSourceId.isEmpty())
                return std::nullopt;
            break;
    }

    for (OUString& rSlide : aRecord.maSlides)
    {
        rSlide = SlideLink::NormaliseGuid(rSlide);
        if (rSlide.isEmpty())
            return std::nullopt;
    }

    if (!Holds(aRecord))
        return std::nullopt;

    return aRecord;
}

std::vector<SlideLinkRecord> SlideLinkManager::GetLinks() const
{
    std::vector<SlideLinkRecord> aLinks;
    if (maLinks.empty())
        return aLinks;

    std::map<OUString, sal_Int32> aPageIndices;
    for (sal_uInt16 nIndex = 0, nCount = mrDoc.GetSdPageCount(PageKind::Standard); nIndex < nCount;
         ++nIndex)
    {
        if (const SdPage* pPage = mrDoc.GetSdPage(nIndex, PageKind::Standard))
            aPageIndices[pPage->GetGuid().getOUString()] = nIndex;
    }

    for (const SlideLinkRecord& rRecord : maLinks)
    {
        if (Holds(rRecord, aPageIndices))
            aLinks.push_back(rRecord);
    }

    // A link that spans a slide comes before one that starts on it, so a section comes before
    // its first slide.
    const auto aPosition = [&aPageIndices](const SlideLinkRecord& rRecord)
    {
        const auto it = aPageIndices.find(rRecord.maSlides.front());
        const sal_Int32 nIndex = it == aPageIndices.end() ? -1 : it->second;
        return std::make_pair(nIndex, rRecord.meKind == SlideLinkKind::Section ? 0 : 1);
    };
    std::stable_sort(aLinks.begin(), aLinks.end(),
                     [&aPosition](const SlideLinkRecord& rOne, const SlideLinkRecord& rOther)
                     { return aPosition(rOne) < aPosition(rOther); });
    return aLinks;
}

const SlideLinkRecord* SlideLinkManager::Find(SlideLinkKind eKind, std::u16string_view rId) const
{
    const OUString aId = SlideLink::NormaliseGuid(rId);
    if (aId.isEmpty())
        return nullptr;

    for (const SlideLinkRecord& rRecord : maLinks)
    {
        if (rRecord.meKind == eKind && rRecord.maId == aId)
            return Holds(rRecord) ? &rRecord : nullptr;
    }
    return nullptr;
}

OUString SlideLinkManager::GetParent(const SlideLinkRecord& rRecord) const
{
    if (rRecord.meKind != SlideLinkKind::Slide)
        return OUString();

    // Few sections span the slide, so the walk over the pages that Holds makes is the last test.
    for (const SlideLinkRecord& rCandidate : maLinks)
    {
        if (rCandidate.meKind == SlideLinkKind::Section
            && std::find(rCandidate.maSlides.begin(), rCandidate.maSlides.end(), rRecord.maId)
                   != rCandidate.maSlides.end()
            && Holds(rCandidate))
            return rCandidate.maId;
    }
    return OUString();
}

void SlideLinkManager::WriteLinks(tools::JsonWriter& rJsonWriter) const
{
    auto aLinksArray = rJsonWriter.startArray("links");
    for (const SlideLinkRecord& rRecord : GetLinks())
    {
        auto aLinkNode = rJsonWriter.startStruct();
        rJsonWriter.put("unit", GetSlideLinkKindName(rRecord.meKind));
        rJsonWriter.put("id", rRecord.maId);
        rJsonWriter.put("parent", GetParent(rRecord));
        rJsonWriter.put("source", rRecord.maSource);
        rJsonWriter.put("sourceId", rRecord.maSourceId);
        rJsonWriter.put("name", rRecord.maName);
        rJsonWriter.put("lastModifiedTime", rRecord.maLastModifiedTime);
        auto aSlidesArray = rJsonWriter.startArray("slides");
        for (const OUString& rSlide : rRecord.maSlides)
            rJsonWriter.putSimpleValue(rSlide);
    }
}

void SlideLinkManager::Put(std::vector<SlideLinkRecord> aRecords)
{
    for (SlideLinkRecord& rRecord : aRecords)
    {
        auto it
            = std::find_if(maLinks.begin(), maLinks.end(), [&rRecord](const SlideLinkRecord& rHeld)
                           { return isSameUnit(rHeld, rRecord); });
        if (it == maLinks.end())
            maLinks.push_back(std::move(rRecord));
        else
            *it = std::move(rRecord);
    }
}

void SlideLinkManager::DropRecordsOfGoneUnits()
{
    std::erase_if(maLinks, [this](const SlideLinkRecord& rRecord)
                  { return GetUnitSlides(rRecord).empty(); });
}

void SlideLinkManager::AddUndo(std::vector<SlideLinkRecord> aOldLinks, const OUString& rComment)
{
    if (mrDoc.IsUndoEnabled())
        mrDoc.AddUndo(std::make_unique<UndoSlideLinks>(mrDoc, rComment, std::move(aOldLinks)));
}

bool SlideLinkManager::Set(std::vector<SlideLinkRecord> aRecords, const OUString& rComment)
{
    std::vector<SlideLinkRecord> aValidated;
    for (const SlideLinkRecord& rRecord : aRecords)
    {
        std::optional<SlideLinkRecord> oRecord = Validate(rRecord);
        if (!oRecord)
            return false;
        aValidated.push_back(std::move(*oRecord));
    }

    std::vector<SlideLinkRecord> aOldLinks = maLinks;
    Put(std::move(aValidated));
    AddUndo(std::move(aOldLinks), rComment);
    mrDoc.SetChanged();
    NotifyLinksIfChanged();
    return true;
}

std::vector<SlideLinkRecord> SlideLinkManager::MakeSlideRecords(
    SdDrawDocument& rSource, const std::vector<OUString>& rReadNames, sal_Int32 nFirstSlide,
    sal_Int32 nSlideCount, const std::vector<sal_Int32>& rSourcePositions,
    const OUString& rSourceName, const OUString& rLastModifiedTime) const
{
    std::vector<SlideLinkRecord> aRecords;
    if (nSlideCount <= 0 || rReadNames.size() != static_cast<size_t>(nSlideCount))
        return aRecords;

    // The run names the position of every page it inserts or of none of them.
    if (!rSourcePositions.empty() && rSourcePositions.size() != static_cast<size_t>(nSlideCount))
        return aRecords;

    for (sal_Int32 nSlide = 0; nSlide < nSlideCount; ++nSlide)
    {
        const SdPage* pPage
            = mrDoc.GetSdPage(static_cast<sal_uInt16>(nFirstSlide + nSlide), PageKind::Standard);
        if (!pPage)
            continue;

        SlideLinkRecord aRecord;
        aRecord.meKind = SlideLinkKind::Slide;
        aRecord.maId = pPage->GetGuid().getOUString();
        aRecord.maSource = rSourceName;
        aRecord.maLastModifiedTime = rLastModifiedTime;
        aRecord.maSlides = { aRecord.maId };

        if (!rSourcePositions.empty())
        {
            // The page follows the position its slide holds in the source and no one slide, so
            // an update reads whichever slide stands there.
            aRecord.maSourceId = SlideLink::MakePositionName(rSourcePositions[nSlide]);
            aRecord.maName = aRecord.maSourceId;
            aRecords.push_back(std::move(aRecord));
            continue;
        }

        bool bIsMasterPage = false;
        const sal_uInt16 nRead = rSource.GetPageByName(rReadNames[nSlide], bIsMasterPage);
        if (nRead == SDRPAGE_NOTFOUND || bIsMasterPage)
            continue;

        const SdPage* pReadPage = dynamic_cast<const SdPage*>(rSource.GetPage(nRead));
        if (!pReadPage)
            continue;

        // The page follows the slide of the source it came from, which is the slide the file was
        // written from rather than the page it holds in the file, and records the name that slide
        // has of its own. A slide with no name of its own is recorded with none, and so is one
        // whose name the record cannot keep: the identifier finds the slide, and the name is the
        // fallback for a file that lists no identifiers.
        const OUString aOriginPage = SlideLink::GetOriginPage(*pReadPage);
        const OUString aSourceSlideName = aOriginPage.isEmpty() ? rReadNames[nSlide] : aOriginPage;
        aRecord.maSourceId = pReadPage->GetGuid().getOUString();
        aRecord.maName = getOwnRecordableName(aSourceSlideName);
        aRecords.push_back(std::move(aRecord));
    }
    return aRecords;
}

bool SlideLinkManager::LinkInsertedPages(SdDrawDocument& rSource,
                                         const std::vector<OUString>& rReadNames,
                                         sal_Int32 nFirstSlide, sal_Int32 nSlideCount,
                                         const std::vector<sal_Int32>& rSourcePositions,
                                         const OUString& rSourceName,
                                         const OUString& rLastModifiedTime)
{
    std::vector<SlideLinkRecord> aRecords
        = MakeSlideRecords(rSource, rReadNames, nFirstSlide, nSlideCount, rSourcePositions,
                           rSourceName, rLastModifiedTime);
    if (aRecords.empty())
        return false;

    return Set(std::move(aRecords), SdResId(STR_UNDO_INSERTPAGES));
}

bool SlideLinkManager::LinkInsertedSection(sal_Int32 nFirstSlide, sal_Int32 nSlideCount,
                                           const SlideLinkSourceSection& rSection,
                                           const OUString& rSourceName,
                                           const OUString& rLastModifiedTime)
{
    SlideSectionManager& rSections = mrDoc.GetSectionManager();
    const std::vector<SlideSection> aSnapshot = rSections.GetSectionsSnapshot();

    const OUString aName
        = rSection.maName.isEmpty() ? SdResId(STR_DEFAULT_SLIDE_SECTION_NAME) : rSection.maName;
    if (!rSections.AddInsertedSection(nFirstSlide, nSlideCount, aName))
        return false;

    const sal_Int32 nSection = rSections.GetSectionIndexForSlide(nFirstSlide);
    SlideLinkRecord aRecord;
    aRecord.meKind = SlideLinkKind::Section;
    aRecord.maId = rSections.GetSection(nSection).maId;
    aRecord.maSource = rSourceName;
    aRecord.maSourceId = rSection.maId;
    aRecord.maName = rSection.maName;
    aRecord.maLastModifiedTime = rLastModifiedTime;
    aRecord.maSlides = rSections.GetSectionPageGuids(nSection);

    if (!Set({ aRecord }, SdResId(STR_UNDO_LINK_SLIDE_SECTION)))
    {
        rSections.RestoreSectionsSnapshot(aSnapshot);
        return false;
    }
    return true;
}

std::optional<std::vector<OUString>>
SlideLinkManager::GetSectionReadNames(const SlideLinkRecord& rRecord, SdDrawDocument& rSource)
{
    const SlideSectionManager& rSections = rSource.GetSectionManager();
    const sal_Int32 nSection = rSections.GetSectionIndexForId(rRecord.maSourceId);
    if (nSection < 0)
    {
        SAL_WARN("sd", "slide link refresh: the file lists no section " << rRecord.maSourceId);
        return std::nullopt;
    }

    // An empty list of names would stand for every page of the file, so a section with no slide
    // reads nothing.
    std::vector<OUString> aNames;
    const sal_Int32 nStart = rSections.GetSection(nSection).mnStartIndex;
    const sal_Int32 nEnd = nStart + rSections.GetSectionSlideCount(nSection);
    for (sal_Int32 nPage = nStart; nPage < nEnd; ++nPage)
    {
        if (const SdPage* pPage
            = rSource.GetSdPage(static_cast<sal_uInt16>(nPage), PageKind::Standard))
            aNames.push_back(pPage->GetName());
    }
    if (aNames.empty())
        return std::nullopt;

    return aNames;
}

sal_Int32 SlideLinkManager::Refresh(SlideLinkKind eKind, std::u16string_view rId,
                                    const OUString& rSourceName, const OUString& rFileUrl,
                                    const OUString& rLastModifiedTime, const OUString& rName,
                                    std::vector<OString>* pNotUpdated)
{
    const SlideLinkRecord* pRecord = Find(eKind, rId);
    if (!pRecord || pRecord->maSource != rSourceName)
        return -1;

    if (!SlideLink::IsReadableSourceString(rName))
        return -1;

    // The list changes in the refresh, and the record says what the link was made with.
    std::vector<SlideLinkRecord> aSlides;
    std::vector<SlideLinkRecord> aSections;
    const bool bSection = eKind == SlideLinkKind::Section;
    (bSection ? aSections : aSlides).push_back(*pRecord);
    return RefreshRecords(aSlides, aSections, rFileUrl, rLastModifiedTime, rName,
                          bSection ? STR_UNDO_UPDATE_LINKED_SECTION
                                   : STR_UNDO_UPDATE_LINKED_SLIDES,
                          pNotUpdated);
}

sal_Int32 SlideLinkManager::RefreshSource(const OUString& rSourceName, const OUString& rFileUrl,
                                          const OUString& rLastModifiedTime,
                                          std::vector<OString>* pNotUpdated)
{
    // The links under no other link: every section, and every slide outside a linked section.
    // A section brings the slides it spans along.
    std::vector<SlideLinkRecord> aSections;
    std::vector<SlideLinkRecord> aSlides;
    for (const SlideLinkRecord& rRecord : GetLinks())
    {
        if (rRecord.maSource != rSourceName)
            continue;

        if (rRecord.meKind == SlideLinkKind::Section)
            aSections.push_back(rRecord);
        else if (GetParent(rRecord).isEmpty())
            aSlides.push_back(rRecord);
    }
    if (aSections.empty() && aSlides.empty())
        return -1;

    return RefreshRecords(aSlides, aSections, rFileUrl, rLastModifiedTime, OUString(),
                          STR_UNDO_UPDATE_LINKED_SLIDES, pNotUpdated);
}

sal_Int32 SlideLinkManager::RefreshRecords(const std::vector<SlideLinkRecord>& rSlides,
                                           const std::vector<SlideLinkRecord>& rSections,
                                           const OUString& rFileUrl,
                                           const OUString& rLastModifiedTime,
                                           const OUString& rName, TranslateId pUndoName,
                                           std::vector<OString>* pNotUpdated)
{
    if (!SlideLink::IsReadableSourceString(rLastModifiedTime))
        return -1;

    // The slides come from a file on this machine, so that a refresh needs no network.
    if (!SlideLink::IsLocalFile(rFileUrl))
    {
        SAL_WARN("sd", "slide link refresh: the slides are not staged in a file on this machine");
        return -1;
    }

    // The pages change in several steps below, and the links are announced once they are all
    // in place.
    const HoldAnnouncement aHold(*this);

    // A file staged under a name used before holds other slides this time, so the document reads
    // the file it is given now.
    mrDoc.CloseBookmarkDoc();
    SdDrawDocument* pSource = mrDoc.OpenBookmarkDoc(rFileUrl, /*bNoDialogs=*/true);
    if (!pSource)
        return -1;
    comphelper::ScopeGuard aCloseSource([this] { mrDoc.CloseBookmarkDoc(); });

    SfxUndoManager* pUndoManager = mrDoc.beginUndoAction(SdResId(pUndoName));
    comphelper::ScopeGuard aLeaveUndo([this, pUndoManager]
                                      { mrDoc.endUndoAction(/*bUndo=*/false, pUndoManager); });

    sal_Int32 nRefreshed = RefreshSlides(rSlides, *pSource, rLastModifiedTime, pNotUpdated);
    for (const SlideLinkRecord& rSection : rSections)
    {
        // A section the file holds nothing for keeps the slides it holds, like a slide the file
        // holds nothing for, and so does a section whose slides could not be read. Both are
        // reported with the slides.
        const std::optional<std::vector<OUString>> oNames = GetSectionReadNames(rSection, *pSource);
        const sal_Int32 nCount
            = oNames ? RefreshSection(rSection, *pSource, *oNames, rLastModifiedTime, rName) : 0;
        if (nCount > 0)
            nRefreshed += nCount;
        else
            addNotUpdated(pNotUpdated, rSection.maSlides);
    }
    return nRefreshed;
}

sal_Int32 SlideLinkManager::RefreshSlides(const std::vector<SlideLinkRecord>& rRecords,
                                          SdDrawDocument& rSource,
                                          const OUString& rLastModifiedTime,
                                          std::vector<OString>* pNotUpdated)
{
    /// One page to read again: where it stands, what it holds and what it reads.
    struct Target
    {
        sal_uInt16 mnIndex;
        OUString maId;
        OUString maSourcePage;
        OUString maRecordedName;
    };
    std::vector<Target> aTargets;
    for (const SlideLinkRecord& rRecord : rRecords)
    {
        const sal_Int32 nIndex = SlideLink::GetPageIndex(mrDoc, rRecord.maId);
        if (nIndex < 0)
            continue;

        const SdPage* pSourcePage = findSourcePage(rSource, rRecord);
        if (!pSourcePage)
        {
            SAL_WARN("sd", "slide link refresh: the file holds no slide "
                               << rRecord.maSourceId << " and none named " << rRecord.maName);
            addNotUpdated(pNotUpdated, rRecord.maSlides);
            continue;
        }

        aTargets.push_back({ static_cast<sal_uInt16>(nIndex), rRecord.maId, pSourcePage->GetName(),
                             getRefreshedName(rRecord, pSourcePage->GetName()) });
    }
    std::sort(aTargets.begin(), aTargets.end(), [](const Target& rOne, const Target& rOther)
              { return rOne.mnIndex < rOther.mnIndex; });

    std::vector<SlideLinkRecord> aOldLinks = maLinks;
    sal_Int32 nRefreshed = 0;

    // One resolution reads a run of slides into the pages from the position of its first one, so
    // the pages that sit next to each other are resolved together. Every resolution of its own
    // copies the styles of the file and sweeps the master pages of the whole document again.
    for (size_t nFirst = 0; nFirst < aTargets.size();)
    {
        size_t nPast = nFirst + 1;
        while (nPast < aTargets.size()
               && aTargets[nPast].mnIndex == aTargets[nPast - 1].mnIndex + 1)
            ++nPast;

        std::vector<OUString> aSourcePages;
        for (size_t nPosition = nFirst; nPosition < nPast; ++nPosition)
            aSourcePages.push_back(aTargets[nPosition].maSourcePage);

        // A resolution keeps the number of pages, so every position collected above still holds a
        // page.
        const sal_uInt16 nInsertPosition
            = mrDoc.GetSdPage(aTargets[nFirst].mnIndex, PageKind::Standard)->GetPageNum();
        mrDoc.ResolvePageLinks(aSourcePages, nInsertPosition, /*bNoDialogs=*/true,
                               /*bCopy=*/true);

        // A resolution puts the slide it read in place of the page at its position, and a page
        // read from a file holds an identifier of its own, so the identifier a position holds now
        // says whether that page was refreshed. The record takes the page read, the slide it names
        // now and the time its source was last modified, and the links above take the page in
        // place of the old one.
        for (size_t nPosition = nFirst; nPosition < nPast; ++nPosition)
        {
            const Target& rTarget = aTargets[nPosition];
            SdPage* pPage = mrDoc.GetSdPage(rTarget.mnIndex, PageKind::Standard);
            if (!pPage || pPage->GetGuid().getOUString() == rTarget.maId)
                continue;

            // The page read records the file it was read from, as a page linked to a file by its
            // path does. The link lives in the record, so the page is the document's own.
            SlideLink::ClearPageReference(*pPage);

            const OUString aNewId = pPage->GetGuid().getOUString();
            for (SlideLinkRecord& rLink : maLinks)
            {
                if (rLink.meKind == SlideLinkKind::Slide && rLink.maId == rTarget.maId)
                {
                    rLink.maId = aNewId;
                    rLink.maSlides = { aNewId };
                    rLink.maName = rTarget.maRecordedName;
                    rLink.maLastModifiedTime = rLastModifiedTime;
                }
                else if (rLink.meKind == SlideLinkKind::Section)
                    std::replace(rLink.maSlides.begin(), rLink.maSlides.end(), rTarget.maId,
                                 aNewId);
            }
            ++nRefreshed;
        }

        nFirst = nPast;
    }

    if (nRefreshed > 0)
    {
        AddUndo(std::move(aOldLinks), SdResId(STR_UNDO_UPDATE_LINKED_SLIDES));
        mrDoc.SetChanged();
    }
    NotifyLinksIfChanged();
    return nRefreshed;
}

sal_Int32 SlideLinkManager::RefreshSection(const SlideLinkRecord& rRecord, SdDrawDocument& rSource,
                                           const std::vector<OUString>& rReadNames,
                                           const OUString& rLastModifiedTime, const OUString& rName)
{
    SlideSectionManager& rSections = mrDoc.GetSectionManager();
    const sal_Int32 nSection = rSections.GetSectionIndexForId(rRecord.maId);
    rtl::Reference<SdXImpressDocument> xModel(mrDoc.getUnoModel());
    if (nSection < 0 || !xModel.is())
        return -1;

    std::vector<SlideLinkRecord> aOldLinks = maLinks;

    // The fresh slides go in after the old ones, so they join the section and the section always
    // holds slides. Then the old slides go. The slides keep the design of the source, as a
    // refreshed slide does.
    const sal_Int32 nStart = rSections.GetSection(nSection).mnStartIndex;
    const sal_Int32 nEnd = nStart + rSections.GetSectionSlideCount(nSection);
    const sal_uInt16 nInsertPosition = nEnd < mrDoc.GetSdPageCount(PageKind::Standard)
                                           ? static_cast<sal_uInt16>(2 * nEnd + 1)
                                           : 0xFFFF;
    const sal_Int32 nInsertedCount = SlideLink::InsertPages(
        mrDoc, rSource, rReadNames, InsertBookmarkOptions::ForSlideImport(/*bKeepDesign=*/true),
        nInsertPosition);
    // A read that brought no slide in replaces nothing.
    if (nInsertedCount <= 0)
        return -1;

    std::vector<SlideLinkRecord> aSlideRecords = MakeSlideRecords(
        rSource, rReadNames, nEnd, nInsertedCount, {}, rRecord.maSource, rLastModifiedTime);

    rtl::Reference<SdDrawPagesAccess> xPages = xModel->getSdDrawPages();
    for (sal_Int32 nSlide = nEnd - 1; nSlide >= nStart; --nSlide)
    {
        cpo::uno::Reference<css::drawing::XDrawPage> xPage(xPages->getByIndex(nSlide),
                                                           cpo::uno::UNO_QUERY);
        xPages->remove(xPage);
    }

    const sal_Int32 nUpdated = rSections.GetSectionIndexForId(rRecord.maId);
    if (nUpdated < 0)
    {
        SAL_WARN("sd", "slide link refresh: the section is gone with its old slides");
        return -1;
    }

    // The section follows the name of its source section until somebody renames it here. That
    // name is the one given, or else the one the file lists for the section, since the file
    // carries the sections of its pages under the names they have in the source.
    OUString aSourceSectionName = rName;
    if (aSourceSectionName.isEmpty())
    {
        const SlideSectionManager& rSourceSections = rSource.GetSectionManager();
        const sal_Int32 nSourceSection = rSourceSections.GetSectionIndexForId(rRecord.maSourceId);
        if (nSourceSection >= 0
            && SlideLink::IsReadableSourceString(rSourceSections.GetSection(nSourceSection).maName))
            aSourceSectionName = rSourceSections.GetSection(nSourceSection).maName;
    }
    if (aSourceSectionName.isEmpty())
        aSourceSectionName = rRecord.maName;
    const OUString& rCurrentName = rSections.GetSection(nUpdated).maName;
    if (rCurrentName == rRecord.maName && !aSourceSectionName.isEmpty()
        && rCurrentName != aSourceSectionName)
    {
        if (mrDoc.IsUndoEnabled())
            mrDoc.AddUndo(
                std::make_unique<UndoSlideSection>(mrDoc, SdResId(STR_UNDO_RENAME_SLIDE_SECTION)));
        rSections.RenameSection(nUpdated, aSourceSectionName);
    }

    // The old slides are gone with their records. The section spans the fresh slides, each linked
    // to its source slide on its own as well.
    DropRecordsOfGoneUnits();
    SlideLinkRecord aSection = rRecord;
    aSection.maName = aSourceSectionName;
    aSection.maLastModifiedTime = rLastModifiedTime;
    aSection.maSlides = rSections.GetSectionPageGuids(nUpdated);
    aSlideRecords.push_back(std::move(aSection));
    Put(std::move(aSlideRecords));
    SAL_WARN_IF(!Find(SlideLinkKind::Section, rRecord.maId), "sd",
                "the updated section could not be linked again");

    AddUndo(std::move(aOldLinks), SdResId(STR_UNDO_UPDATE_LINKED_SECTION));
    mrDoc.SetChanged();
    NotifyLinksIfChanged();
    return nInsertedCount;
}

bool SlideLinkManager::Break(SlideLinkKind eKind, std::u16string_view rId, bool bDeep)
{
    const SlideLinkRecord* pRecord = Find(eKind, rId);
    if (!pRecord)
        return false;

    const SlideLinkRecord aRecord = *pRecord;
    std::vector<SlideLinkRecord> aOldLinks = maLinks;

    std::erase_if(maLinks,
                  [&aRecord](const SlideLinkRecord& rHeld) { return isSameUnit(rHeld, aRecord); });

    // The slides a section spans keep their own links unless they are asked off as well.
    if (bDeep && aRecord.meKind == SlideLinkKind::Section)
    {
        std::erase_if(maLinks,
                      [&aRecord](const SlideLinkRecord& rHeld)
                      {
                          return rHeld.meKind == SlideLinkKind::Slide
                                 && std::find(aRecord.maSlides.begin(), aRecord.maSlides.end(),
                                              rHeld.maId)
                                        != aRecord.maSlides.end();
                      });
    }

    AddUndo(std::move(aOldLinks),
            SdResId(eKind == SlideLinkKind::Slide ? STR_UNDO_BREAK_SLIDE_LINK
                                                  : STR_UNDO_UNLINK_SLIDE_SECTION));
    mrDoc.SetChanged();
    NotifyLinksIfChanged();
    return true;
}

std::vector<OUString> SlideLinkManager::GetHoldingKeys() const
{
    std::vector<OUString> aKeys;
    for (const SlideLinkRecord& rRecord : GetLinks())
        aKeys.push_back(makeKey(rRecord));
    return aKeys;
}

void SlideLinkManager::NotifyLinksIfChanged()
{
    // An operation that holds the announcement back announces when it ends.
    if (mnAnnouncementsHeld > 0)
        return;

    std::vector<OUString> aKeys = GetHoldingKeys();
    if (aKeys == maAnnouncedLinks)
        return;

    maAnnouncedLinks = std::move(aKeys);
    mrDoc.NotifyViews(COKitCallbackType::SLIDE_LINKS_CHANGED, OString());
}

SlideLinkManager::HoldAnnouncement::HoldAnnouncement(SlideLinkManager& rManager)
    : mrManager(rManager)
{
    ++mrManager.mnAnnouncementsHeld;
}

SlideLinkManager::HoldAnnouncement::~HoldAnnouncement()
{
    if (--mrManager.mnAnnouncementsHeld == 0)
        mrManager.NotifyLinksIfChanged();
}

void SlideLinkManager::RestoreSnapshot(std::vector<SlideLinkRecord> aLinks)
{
    maLinks = std::move(aLinks);
    NotifyLinksIfChanged();
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
