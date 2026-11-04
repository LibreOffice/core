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
#include <tools/Guid.hxx>
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
constexpr OUString gPresentationKindName = u"presentation"_ustr;


/// The name a record keeps for a unit of the source: rName itself, or none when the record
/// cannot keep it.
OUString getRecordableName(const OUString& rName)
{
    return SlideLink::IsReadableSourceString(rName) ? rName : OUString();
}

/// Whether rUnits names the unit rId.
bool contains(const std::vector<OUString>& rUnits, const OUString& rId)
{
    return std::find(rUnits.begin(), rUnits.end(), rId) != rUnits.end();
}

/// Where a link of kind eKind sorts among the links that start on the same slide: the one that
/// spans the most comes first.
int getKindRank(SlideLinkKind eKind)
{
    switch (eKind)
    {
        case SlideLinkKind::Presentation:
            return 0;
        case SlideLinkKind::Section:
            return 1;
        case SlideLinkKind::Slide:
            return 2;
    }
    return 3;
}

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
        case SlideLinkKind::Presentation:
            return gPresentationKindName;
    }
    return OUString();
}

std::optional<SlideLinkKind> ParseSlideLinkKind(std::u16string_view rName)
{
    if (rName == gSlideKindName)
        return SlideLinkKind::Slide;
    if (rName == gSectionKindName)
        return SlideLinkKind::Section;
    if (rName == gPresentationKindName)
        return SlideLinkKind::Presentation;
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
        case SlideLinkKind::Presentation:
        {
            const SlideSectionManager& rSections = mrDoc.GetSectionManager();
            std::vector<OUString> aSlides;
            for (const OUString& rSection : rRecord.maSections)
            {
                const sal_Int32 nSection = rSections.GetSectionIndexForId(rSection);
                if (nSection < 0)
                    return {};

                const std::vector<OUString> aSectionSlides
                    = rSections.GetSectionPageGuids(nSection);
                aSlides.insert(aSlides.end(), aSectionSlides.begin(), aSectionSlides.end());
            }
            return aSlides;
        }
    }
    return {};
}

bool SlideLinkManager::IsSectionRun(const std::vector<OUString>& rSections) const
{
    const SlideSectionManager& rManager = mrDoc.GetSectionManager();
    sal_Int32 nPrevious = -1;
    for (const OUString& rSection : rSections)
    {
        const sal_Int32 nSection = rManager.GetSectionIndexForId(rSection);
        if (nSection < 0 || (nPrevious >= 0 && nSection != nPrevious + 1))
            return false;
        nPrevious = nSection;
    }
    return !rSections.empty();
}

std::vector<OUString>
SlideLinkManager::GetSectionsOfSlides(const std::vector<OUString>& rSlides) const
{
    if (rSlides.empty())
        return {};

    const SlideSectionManager& rSections = mrDoc.GetSectionManager();
    const sal_Int32 nFirst = SlideLink::GetPageIndex(mrDoc, rSlides.front());
    if (nFirst < 0)
        return {};
    sal_Int32 nSection = rSections.GetSectionIndexForSlide(nFirst);
    if (nSection < 0 || rSections.GetSection(nSection).mnStartIndex != nFirst)
        return {};

    std::vector<OUString> aIds;
    sal_Int32 nCovered = 0;
    const sal_Int32 nSlideCount = static_cast<sal_Int32>(rSlides.size());
    while (nCovered < nSlideCount && nSection < rSections.GetSectionCount())
    {
        const OUString& rId = rSections.GetSection(nSection).maId;
        if (rId.isEmpty())
            return {};
        aIds.push_back(rId);
        nCovered += rSections.GetSectionSlideCount(nSection);
        ++nSection;
    }
    return nCovered == nSlideCount ? aIds : std::vector<OUString>();
}

bool SlideLinkManager::Holds(const SlideLinkRecord& rRecord) const
{
    if (rRecord.maSlides.empty() || rRecord.maSlides != GetUnitSlides(rRecord))
        return false;

    // The sections of a presentation stay one run in the order they were linked in. The slides
    // tell a section added or taken away, and the run tells a section moved within it.
    return rRecord.meKind != SlideLinkKind::Presentation || IsSectionRun(rRecord.maSections);
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

    for (OUString& rSlide : aRecord.maSlides)
    {
        rSlide = SlideLink::NormaliseGuid(rSlide);
        if (rSlide.isEmpty())
            return std::nullopt;
    }

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
            // A section follows one section of the source, or the slides of the source outside
            // any section when it names none.
            if (!rRecord.maSourceId.isEmpty())
            {
                aRecord.maSourceId = SlideLink::NormaliseGuid(rRecord.maSourceId);
                if (aRecord.maSourceId.isEmpty())
                    return std::nullopt;
            }
            break;
        case SlideLinkKind::Presentation:
            // The whole source is the unit, so the link names nothing in it. The sections are
            // the ones given, or else the ones that hold the slides, as for a link read from
            // the file.
            if (!rRecord.maSourceId.isEmpty())
                return std::nullopt;
            for (OUString& rSection : aRecord.maSections)
            {
                rSection = SlideLink::NormaliseGuid(rSection);
                if (rSection.isEmpty())
                    return std::nullopt;
            }
            if (aRecord.maSections.empty())
                aRecord.maSections = GetSectionsOfSlides(aRecord.maSlides);
            if (aRecord.maSections.empty())
                return std::nullopt;
            break;
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

    // A link that spans a slide comes before one that starts on it, so a presentation comes
    // before its first section and a section before its first slide.
    const auto aPosition = [&aPageIndices](const SlideLinkRecord& rRecord)
    {
        const auto it = aPageIndices.find(rRecord.maSlides.front());
        const sal_Int32 nIndex = it == aPageIndices.end() ? -1 : it->second;
        return std::make_pair(nIndex, getKindRank(rRecord.meKind));
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
    if (rRecord.meKind == SlideLinkKind::Presentation)
        return OUString();

    // Few links span the unit, so the walk over the pages that Holds makes is the last test. A
    // slide is under the section that spans it, and under the presentation when no linked
    // section does.
    const auto aFindSpanning = [this, &rRecord](SlideLinkKind eKind)
    {
        for (const SlideLinkRecord& rCandidate : maLinks)
        {
            if (rCandidate.meKind != eKind)
                continue;

            const std::vector<OUString>& rUnits = rRecord.meKind == SlideLinkKind::Slide
                                                      ? rCandidate.maSlides
                                                      : rCandidate.maSections;
            if (contains(rUnits, rRecord.maId) && Holds(rCandidate))
                return rCandidate.maId;
        }
        return OUString();
    };

    if (rRecord.meKind == SlideLinkKind::Slide)
    {
        const OUString aSection = aFindSpanning(SlideLinkKind::Section);
        if (!aSection.isEmpty())
            return aSection;
    }
    return aFindSpanning(SlideLinkKind::Presentation);
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

bool SlideLinkManager::LinkInsertedPresentation(SdDrawDocument& rSource, sal_Int32 nFirstSlide,
                                                sal_Int32 nSlideCount, const OUString& rFileName,
                                                const OUString& rSourceName,
                                                const OUString& rLastModifiedTime)
{
    // The sections of the source go over the slides read from it, which are every page of the
    // source in order, so the runs of the source add up to the slides.
    const std::vector<SlideSectionRun> aRuns
        = rSource.GetSectionManager().GetSlideRuns(rFileName);
    SlideSectionManager& rSections = mrDoc.GetSectionManager();
    const std::vector<SlideSection> aSnapshot = rSections.GetSectionsSnapshot();
    const std::vector<OUString> aSectionIds
        = rSections.AddInsertedSections(nFirstSlide, nSlideCount, aRuns);
    if (aSectionIds.empty())
        return false;

    // One record per section the source identifies, one for the run of its slides outside any
    // section, which names no section, and one over them all. A section the source does not
    // identify is linked to nothing.
    std::vector<SlideLinkRecord> aRecords;
    for (size_t nRun = 0; nRun < aRuns.size(); ++nRun)
    {
        const OUString aSourceId = SlideLink::NormaliseGuid(aRuns[nRun].maId);
        if (aSourceId.isEmpty() && !aRuns[nRun].mbLoose)
            continue;

        SlideLinkRecord aRecord;
        aRecord.meKind = SlideLinkKind::Section;
        aRecord.maId = aSectionIds[nRun];
        aRecord.maSource = rSourceName;
        aRecord.maSourceId = aSourceId;
        aRecord.maName = getRecordableName(aRuns[nRun].maName);
        aRecord.maLastModifiedTime = rLastModifiedTime;
        aRecord.maSlides
            = rSections.GetSectionPageGuids(rSections.GetSectionIndexForId(aRecord.maId));
        aRecords.push_back(std::move(aRecord));
    }

    SlideLinkRecord aPresentation;
    aPresentation.meKind = SlideLinkKind::Presentation;
    aPresentation.maId = tools::Guid(tools::Guid::Generate).getOUString();
    aPresentation.maSource = rSourceName;
    aPresentation.maName = rFileName;
    aPresentation.maLastModifiedTime = rLastModifiedTime;
    aPresentation.maSections = aSectionIds;
    aPresentation.maSlides = GetUnitSlides(aPresentation);
    aRecords.push_back(std::move(aPresentation));

    if (!Set(std::move(aRecords), SdResId(STR_UNDO_LINK_SLIDE_PRESENTATION)))
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
    const sal_Int32 nPageCount = rSource.GetSdPageCount(PageKind::Standard);
    sal_Int32 nStart = 0;
    sal_Int32 nEnd = 0;
    if (rRecord.maSourceId.isEmpty())
    {
        // The pages of the file outside any section sit before its first section.
        nEnd = rSections.GetSectionCount() > 0
                   ? std::min(rSections.GetSection(0).mnStartIndex, nPageCount)
                   : nPageCount;
    }
    else
    {
        const sal_Int32 nSection = rSections.GetSectionIndexForId(rRecord.maSourceId);
        if (nSection < 0)
        {
            SAL_WARN("sd",
                     "slide link refresh: the file lists no section " << rRecord.maSourceId);
            return std::nullopt;
        }
        nStart = rSections.GetSection(nSection).mnStartIndex;
        nEnd = nStart + rSections.GetSectionSlideCount(nSection);
    }

    // An empty list of names would stand for every page of the file, so a section with no slide
    // reads nothing.
    std::vector<OUString> aNames;
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
    std::vector<SlideLinkRecord> aPresentations;
    TranslateId pUndoName = STR_UNDO_UPDATE_LINKED_SLIDES;
    switch (eKind)
    {
        case SlideLinkKind::Slide:
            aSlides.push_back(*pRecord);
            break;
        case SlideLinkKind::Section:
            aSections.push_back(*pRecord);
            pUndoName = STR_UNDO_UPDATE_LINKED_SECTION;
            break;
        case SlideLinkKind::Presentation:
            aPresentations.push_back(*pRecord);
            pUndoName = STR_UNDO_UPDATE_LINKED_PRESENTATION;
            break;
    }
    return RefreshRecords(aSlides, aSections, aPresentations, rFileUrl, rLastModifiedTime, rName,
                          pUndoName, pNotUpdated);
}

sal_Int32 SlideLinkManager::RefreshSource(const OUString& rSourceName, const OUString& rFileUrl,
                                          const OUString& rLastModifiedTime,
                                          std::vector<OString>* pNotUpdated)
{
    // The links under no other link: every presentation, every section outside a linked
    // presentation, and every slide outside a linked section or presentation. A link brings the
    // links it spans along.
    std::vector<SlideLinkRecord> aSlides;
    std::vector<SlideLinkRecord> aSections;
    std::vector<SlideLinkRecord> aPresentations;
    for (const SlideLinkRecord& rRecord : GetLinks())
    {
        if (rRecord.maSource != rSourceName || !GetParent(rRecord).isEmpty())
            continue;

        switch (rRecord.meKind)
        {
            case SlideLinkKind::Slide:
                aSlides.push_back(rRecord);
                break;
            case SlideLinkKind::Section:
                aSections.push_back(rRecord);
                break;
            case SlideLinkKind::Presentation:
                aPresentations.push_back(rRecord);
                break;
        }
    }
    if (aSlides.empty() && aSections.empty() && aPresentations.empty())
        return -1;

    return RefreshRecords(aSlides, aSections, aPresentations, rFileUrl, rLastModifiedTime,
                          OUString(), STR_UNDO_UPDATE_LINKED_SLIDES, pNotUpdated);
}

sal_Int32 SlideLinkManager::RefreshRecords(const std::vector<SlideLinkRecord>& rSlides,
                                           const std::vector<SlideLinkRecord>& rSections,
                                           const std::vector<SlideLinkRecord>& rPresentations,
                                           const OUString& rFileUrl,
                                           const OUString& rLastModifiedTime, const OUString& rName,
                                           TranslateId pUndoName, std::vector<OString>* pNotUpdated)
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
    for (const SlideLinkRecord& rPresentation : rPresentations)
    {
        // A presentation whose pages could not be read keeps its slides, reported the same way.
        const sal_Int32 nCount
            = RefreshPresentation(rPresentation, *pSource, rLastModifiedTime, rName);
        if (nCount > 0)
            nRefreshed += nCount;
        else
            addNotUpdated(pNotUpdated, rPresentation.maSlides);
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
        // now and the time its source was last modified, and the section and the presentation
        // above take the page in place of the old one.
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
                else if (rLink.meKind != SlideLinkKind::Slide)
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
    const OUString aPresentation = GetParent(rRecord);

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

    // The presentation that held the section spans the fresh slides in place of the old ones.
    for (SlideLinkRecord& rLink : maLinks)
    {
        if (rLink.meKind == SlideLinkKind::Presentation && rLink.maId == aPresentation)
            rLink.maSlides = GetUnitSlides(rLink);
    }

    AddUndo(std::move(aOldLinks), SdResId(STR_UNDO_UPDATE_LINKED_SECTION));
    mrDoc.SetChanged();
    NotifyLinksIfChanged();
    return nInsertedCount;
}

sal_Int32 SlideLinkManager::RefreshPresentation(const SlideLinkRecord& rRecord,
                                                SdDrawDocument& rSource,
                                                const OUString& rLastModifiedTime,
                                                const OUString& rName)
{
    SlideSectionManager& rSections = mrDoc.GetSectionManager();
    rtl::Reference<SdXImpressDocument> xModel(mrDoc.getUnoModel());
    const sal_Int32 nFirstSection
        = rRecord.maSections.empty() ? -1
                                     : rSections.GetSectionIndexForId(rRecord.maSections.front());
    if (nFirstSection < 0 || !xModel.is())
        return -1;

    // The file is read whole: every page, in the sections the file lists, with the pages outside
    // any section first.
    const OUString aFileName = rName.isEmpty() ? rRecord.maName : rName;
    const std::vector<SlideSectionRun> aRuns
        = rSource.GetSectionManager().GetSlideRuns(aFileName);
    std::vector<OUString> aReadNames;
    for (sal_uInt16 nPage = 0, nCount = rSource.GetSdPageCount(PageKind::Standard); nPage < nCount;
         ++nPage)
    {
        if (const SdPage* pPage = rSource.GetSdPage(nPage, PageKind::Standard))
            aReadNames.push_back(pPage->GetName());
    }
    if (aRuns.empty() || aReadNames.empty())
        return -1;

    std::vector<SlideLinkRecord> aOldLinks = maLinks;
    const std::vector<SlideSection> aOldSections = rSections.GetSectionsSnapshot();

    // The fresh slides go in after the old ones, so they join the last section of the
    // presentation and the run always holds slides. Then the old slides go, which empties the
    // other sections of the presentation, and the sections of the file are built over the fresh
    // slides. The slides keep the design of the source, as a refreshed slide does.
    const sal_Int32 nStart = rSections.GetSection(nFirstSection).mnStartIndex;
    const sal_Int32 nEnd = nStart + static_cast<sal_Int32>(rRecord.maSlides.size());
    const sal_uInt16 nInsertPosition = nEnd < mrDoc.GetSdPageCount(PageKind::Standard)
                                           ? static_cast<sal_uInt16>(2 * nEnd + 1)
                                           : 0xFFFF;
    const sal_Int32 nInsertedCount = SlideLink::InsertPages(
        mrDoc, rSource, aReadNames, InsertBookmarkOptions::ForSlideImport(/*bKeepDesign=*/true),
        nInsertPosition);
    if (nInsertedCount <= 0)
        return -1;

    std::vector<SlideLinkRecord> aSlideRecords = MakeSlideRecords(
        rSource, aReadNames, nEnd, nInsertedCount, {}, rRecord.maSource, rLastModifiedTime);

    rtl::Reference<SdDrawPagesAccess> xPages = xModel->getSdDrawPages();
    for (sal_Int32 nSlide = nEnd - 1; nSlide >= nStart; --nSlide)
    {
        cpo::uno::Reference<css::drawing::XDrawPage> xPage(xPages->getByIndex(nSlide),
                                                           cpo::uno::UNO_QUERY);
        xPages->remove(xPage);
    }

    // A section of the file that is linked under the presentation keeps the identifier it has
    // here, the section of the pages outside any section among them, which names no section of
    // the source. A kept section follows the name of its source until somebody renames it
    // here. Any other section of the file is new here, a section the file lists with no
    // identifier among them, and any section of the presentation the file no longer lists goes.
    const auto aFindOldSection = [&aOldSections](const OUString& rId) -> const SlideSection*
    {
        for (const SlideSection& rSection : aOldSections)
            if (rSection.maId == rId)
                return &rSection;
        return nullptr;
    };
    const auto aFindOldLink
        = [&aOldLinks, &rRecord](const SlideSectionRun& rRun) -> const SlideLinkRecord*
    {
        const OUString aSourceId = SlideLink::NormaliseGuid(rRun.maId);
        if (aSourceId.isEmpty() && !rRun.mbLoose)
            return nullptr;

        for (const SlideLinkRecord& rLink : aOldLinks)
        {
            if (rLink.meKind == SlideLinkKind::Section && rLink.maSourceId == aSourceId
                && contains(rRecord.maSections, rLink.maId))
                return &rLink;
        }
        return nullptr;
    };

    const std::vector<SlideSection> aSectionsBetween = rSections.GetSectionsSnapshot();
    std::vector<SlideSection> aNewSections;
    for (const SlideSection& rSection : aSectionsBetween)
    {
        if (rSection.mnStartIndex < nStart)
            aNewSections.push_back(rSection);
    }

    std::vector<OUString> aPresentationSections;
    std::vector<SlideLinkRecord> aSectionRecords;
    sal_Int32 nRunStart = nStart;
    for (const SlideSectionRun& rRun : aRuns)
    {
        SlideSection aSection;
        aSection.mnStartIndex = nRunStart;
        aSection.maName = rRun.maName;
        nRunStart += rRun.mnSlideCount;

        const OUString aSourceId = SlideLink::NormaliseGuid(rRun.maId);
        const SlideLinkRecord* pOldLink = aFindOldLink(rRun);
        const SlideSection* pKept = pOldLink ? aFindOldSection(pOldLink->maId) : nullptr;
        if (pKept)
        {
            aSection.maId = pKept->maId;
            if (pKept->maName != pOldLink->maName)
                aSection.maName = pKept->maName;
        }
        else
            aSection.maId = tools::Guid(tools::Guid::Generate).getOUString();

        if (rRun.mbLoose || !aSourceId.isEmpty())
        {
            SlideLinkRecord aLink;
            aLink.meKind = SlideLinkKind::Section;
            aLink.maId = aSection.maId;
            aLink.maSource = rRecord.maSource;
            aLink.maSourceId = aSourceId;
            aLink.maName = getRecordableName(rRun.maName);
            aLink.maLastModifiedTime = rLastModifiedTime;
            aSectionRecords.push_back(std::move(aLink));
        }
        aPresentationSections.push_back(aSection.maId);
        aNewSections.push_back(std::move(aSection));
    }
    for (const SlideSection& rSection : aSectionsBetween)
    {
        if (rSection.mnStartIndex >= nStart + nInsertedCount)
            aNewSections.push_back(rSection);
    }
    rSections.RestoreSectionsSnapshot(aNewSections);
    if (mrDoc.IsUndoEnabled())
        mrDoc.AddUndo(std::make_unique<UndoSlideSection>(
            mrDoc, SdResId(STR_UNDO_UPDATE_LINKED_PRESENTATION), aSectionsBetween));

    // The old slides are gone with their records, and so is every section the file no longer
    // lists. The presentation spans the fresh sections and slides, each linked on its own as
    // well.
    DropRecordsOfGoneUnits();
    for (SlideLinkRecord& rLink : aSectionRecords)
        rLink.maSlides = rSections.GetSectionPageGuids(rSections.GetSectionIndexForId(rLink.maId));
    SlideLinkRecord aPresentation = rRecord;
    aPresentation.maName = aFileName;
    aPresentation.maLastModifiedTime = rLastModifiedTime;
    aPresentation.maSections = std::move(aPresentationSections);
    aPresentation.maSlides = GetUnitSlides(aPresentation);
    aSlideRecords.insert(aSlideRecords.end(), aSectionRecords.begin(), aSectionRecords.end());
    aSlideRecords.push_back(std::move(aPresentation));
    Put(std::move(aSlideRecords));
    SAL_WARN_IF(!Find(SlideLinkKind::Presentation, rRecord.maId), "sd",
                "the updated presentation could not be linked again");

    AddUndo(std::move(aOldLinks), SdResId(STR_UNDO_UPDATE_LINKED_PRESENTATION));
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

    // The sections and the slides a link spans keep their own links unless they are asked off
    // as well.
    if (bDeep && aRecord.meKind != SlideLinkKind::Slide)
    {
        std::erase_if(maLinks,
                      [&aRecord](const SlideLinkRecord& rHeld)
                      {
                          switch (rHeld.meKind)
                          {
                              case SlideLinkKind::Slide:
                                  return contains(aRecord.maSlides, rHeld.maId);
                              case SlideLinkKind::Section:
                                  return contains(aRecord.maSections, rHeld.maId);
                              case SlideLinkKind::Presentation:
                                  return false;
                          }
                          return false;
                      });
    }

    TranslateId pUndoName = STR_UNDO_BREAK_SLIDE_LINK;
    switch (eKind)
    {
        case SlideLinkKind::Slide:
            break;
        case SlideLinkKind::Section:
            pUndoName = STR_UNDO_UNLINK_SLIDE_SECTION;
            break;
        case SlideLinkKind::Presentation:
            pUndoName = STR_UNDO_UNLINK_SLIDE_PRESENTATION;
            break;
    }
    AddUndo(std::move(aOldLinks), SdResId(pUndoName));
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
