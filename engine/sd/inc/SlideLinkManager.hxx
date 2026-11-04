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

#include <map>
#include <optional>
#include <string_view>
#include <vector>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <cpo/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/resmgr.hxx>

#include "sddllapi.h"

namespace tools
{
class JsonWriter;
}
class SdDrawDocument;

namespace sd
{
/// The unit of a source document a link holds a copy of.
enum class SlideLinkKind
{
    Slide,
    Section,
    Presentation
};

/// The name a kind goes by outside the document: "slide", "section" or "presentation".
SD_DLLPUBLIC OUString GetSlideLinkKindName(SlideLinkKind eKind);

/// The kind named rName, or nothing when rName names no kind.
SD_DLLPUBLIC std::optional<SlideLinkKind> ParseSlideLinkKind(std::u16string_view rName);

/** A run of consecutive slides of this document that holds a copy of one unit of a source
    document: one slide of the source, one section of it, or the whole presentation.

    Every identifier is a GUID in braced form, the form a slide keeps its own identifier in.
*/
struct SlideLinkRecord
{
    SlideLinkKind meKind = SlideLinkKind::Slide;
    /// The unit of this document that holds the copy: the identifier of the page for a slide, the
    /// identifier of the section for a section, an identifier of its own for a presentation.
    OUString maId;
    /// The source document, as the user knows it: the persistent link the storage resolves.
    OUString maSource;
    /// The unit in the source. For a slide, the identifier of the source slide, or the name a
    /// slide holds by its position when the link follows a position of the source rather than one
    /// slide. For a section, the identifier of the source section, or empty for a section that
    /// holds the slides of the source outside any section. Empty for a presentation, which is
    /// the whole source.
    OUString maSourceId;
    /// The name of the unit in the source when it was last read: the name of the source slide,
    /// empty for a slide that has no name of its own, the name of the source section, or the name
    /// of the source document for a presentation and for a section of the slides outside any
    /// section.
    OUString maName;
    /// The time the source document was last modified when the unit was last read, as the
    /// integrator reported it. Empty when no time was reported.
    OUString maLastModifiedTime;
    /// The identifiers of the slides of this document the link spans, in slide order: the one page
    /// of a slide, the pages of the section, the pages of every section of the presentation.
    std::vector<OUString> maSlides;
    /// The identifiers of the sections of this document a presentation spans, in section order,
    /// one run of consecutive sections. Empty for a slide and for a section. The sections are
    /// not saved with the link: a presentation read from the file spans the sections that hold
    /// the slides it lists.
    std::vector<OUString> maSections;
};

/// The section of a source document a run of inserted pages makes up.
struct SlideLinkSourceSection
{
    /// The identifier of the section in the source, in braced form.
    OUString maId;
    /// The name of the section in the source.
    OUString maName;
};

/** Owns every link of the document to a unit of a source document, whatever its kind.

    A link holds while its span is what it was made with: the one page for a slide, the same
    slides in the same order for a section, the same sections in the same order spanning the
    same slides for a presentation. A link whose span changed is reported no more and is not
    saved, and an undo that gives the span back makes it hold again. Links nest: the slides of a
    linked section are linked to their source slides on their own, under the section, and the
    sections of a linked presentation are linked to their source sections on their own, under
    the presentation. A slide losing its own link does not end the section's, and a section
    losing its own link does not end the presentation's.

    The links are saved with the document as the SlideLinks property of the model, a sequence of
    one property value sequence per link that holds, with the fields of SlideLinkRecord under
    the names Unit, Id, Source, SourceId, Name, LastModifiedTime and Slides.
*/
class SD_DLLPUBLIC SlideLinkManager
{
public:
    explicit SlideLinkManager(SdDrawDocument& rDoc);

    /// The links that hold, as the SlideLinks property holds them.
    cpo::uno::Sequence<css::beans::PropertyValue> GetLinksAsPropertyValues() const;

    /** Takes the links of the SlideLinks property in place of every link held now.

        A link that does not describe the slides it finds, or that holds a value of a shape the
        record cannot keep, is dropped.
    */
    void SetLinksFromPropertyValues(const cpo::uno::Sequence<css::beans::PropertyValue>& rLinks);

    /** The links that hold, in document order: a presentation before the sections it spans, a
        section before the slides it spans, and the links of one kind by the position of their
        first slide.
    */
    std::vector<SlideLinkRecord> GetLinks() const;

    /// The link of kind eKind to the unit rId of this document, when it holds.
    const SlideLinkRecord* Find(SlideLinkKind eKind, std::u16string_view rId) const;

    /** The identifier of the innermost holding link that spans the slides of rRecord, empty for
        none: for a slide, the section that spans it, or else the presentation that spans it; for
        a section, the presentation that spans it.
    */
    OUString GetParent(const SlideLinkRecord& rRecord) const;

    /** Writes the links that hold:

        {"links":[{"unit":"<slide|section|presentation>","id":"<id>","parent":"<id>",
                   "source":"<source>","sourceId":"<id>","name":"<name>",
                   "lastModifiedTime":"<time>","slides":["<part>",...]},...]}

        One flat array in document order, a presentation before its sections and a section
        before its slides. A part is the identifier the page holds now, and the parent is the
        innermost link that spans the unit, or empty.
    */
    void WriteLinks(tools::JsonWriter& rJsonWriter) const;

    /** Makes every link of rRecords hold, in place of the link of the same kind and unit when
        there is one. One undo action with the comment rComment.

        A record is taken as it is given when it holds now: its slides are the slides its unit
        spans, its identifiers are valid, and its name and time are readable. The identifiers
        are kept in braced form.

        @return true when every record was taken, false when one was refused and none was taken.
    */
    bool Set(std::vector<SlideLinkRecord> aRecords, const OUString& rComment);

    /** Links the nSlideCount slides from nFirstSlide on, read from the pages rReadNames of
        rSource in that order, each to the slide of rSourceName it came from.

        rSourcePositions, when it names any, names the position each page's slide holds in the
        source, counted from one, one for every page: a page given a position follows that
        position rather than one slide. Otherwise each page follows the identifier of the slide it
        was read from, and records the name that slide holds in the writing document. One undo
        action.
    */
    bool LinkInsertedPages(SdDrawDocument& rSource, const std::vector<OUString>& rReadNames,
                           sal_Int32 nFirstSlide, sal_Int32 nSlideCount,
                           const std::vector<sal_Int32>& rSourcePositions,
                           const OUString& rSourceName, const OUString& rLastModifiedTime);

    /** Adds a section for the nSlideCount slides from nFirstSlide on, named after rSection,
        linked to that section of rSourceName. The slides are linked already, each to its source
        slide. The section and its link each record an undo action.

        @return true when the section was added and linked, false when nothing changed.
    */
    bool LinkInsertedSection(sal_Int32 nFirstSlide, sal_Int32 nSlideCount,
                             const SlideLinkSourceSection& rSection, const OUString& rSourceName,
                             const OUString& rLastModifiedTime);

    /** Adds the sections of rSource over the nSlideCount slides from nFirstSlide on, which were
        read from every page of rSource in order, and links them and the run as a whole to
        rSourceName, the presentation named rFileName there.

        One section goes in per section of rSource, named after it and linked to it when rSource
        identifies it. The slides of rSource that sit in no section go into one section named
        rFileName at the start, linked to nothing of its own. The slides are linked already,
        each to its source slide. The sections and the links each record an undo action.

        @return true when the sections were added and the presentation linked, false when
                nothing changed.
    */
    bool LinkInsertedPresentation(SdDrawDocument& rSource, sal_Int32 nFirstSlide,
                                  sal_Int32 nSlideCount, const OUString& rFileName,
                                  const OUString& rSourceName, const OUString& rLastModifiedTime);

    /** Replaces the span of one link with what the file at rFileUrl holds for its unit.

        rFileUrl is a file on this machine, named by a file: URL, holding a presentation the
        source wrote out. A slide link takes the slide it follows: by identifier, by position, or
        by name. A section link takes the slides of the section the file lists under the
        identifier of the source section, in order, or the pages of the file outside any
        section for a section that names no section of the source. A presentation link takes
        every page and every section of the file: a section linked under the presentation
        already, the one of the pages outside any section among them, is read again in place and
        keeps its identifier here, a section new to the file comes in, and a section the file no
        longer lists goes. Each page read is a page of its own, so the identifiers of the slides
        change, and a section and a presentation keep their own identifiers. A section takes the
        name rName when its own name is still the name the source section had, a presentation
        records rName as the name of the source document and gives it to the section of the
        pages outside any section the same way, and rLastModifiedTime is recorded either way. A
        unit the file holds nothing for, or whose slides could not be read, keeps the slides it
        holds. One undo step.

        @param pNotUpdated takes the identifier of every slide the file held nothing for or
               could not read.
        @return the number of slides replaced, or -1 for a link that does not hold or is not
                linked to rSourceName, a file that is no local file, a name or time that is not
                readable, or a file that could not be read.
    */
    sal_Int32 Refresh(SlideLinkKind eKind, std::u16string_view rId, const OUString& rSourceName,
                      const OUString& rFileUrl, const OUString& rLastModifiedTime,
                      const OUString& rName, std::vector<OString>* pNotUpdated = nullptr);

    /** Refresh for every link of rSourceName that is under no other link, from one file, in one
        undo step.

        @return the number of slides replaced, or -1 when no link of the source holds, when the
                file is no local file, or when it could not be read.
    */
    sal_Int32 RefreshSource(const OUString& rSourceName, const OUString& rFileUrl,
                            const OUString& rLastModifiedTime,
                            std::vector<OString>* pNotUpdated = nullptr);

    /** Takes the link of kind eKind to the unit rId off. The unit keeps its slides and its
        sections and becomes the document's own. With bDeep, every link beneath comes off as
        well. One undo action, and undoing it gives them all back.

        @return true when the link held and is off now.
    */
    bool Break(SlideLinkKind eKind, std::u16string_view rId, bool bDeep);

    /** Announces the link list as changed when the links that hold are not the ones they were
        at the last announcement.
    */
    void NotifyLinksIfChanged();

    /// Every record, holding or not, as it is.
    std::vector<SlideLinkRecord> GetSnapshot() const { return maLinks; }

    /// Puts the records of a snapshot back in place of every record held now.
    void RestoreSnapshot(std::vector<SlideLinkRecord> aLinks);

    /** Holds the announcement of the links back while it lives, for an operation that changes
        pages and records in several steps, and announces once when the last hold ends.
    */
    class HoldAnnouncement
    {
    public:
        explicit HoldAnnouncement(SlideLinkManager& rManager);
        ~HoldAnnouncement();
        HoldAnnouncement(const HoldAnnouncement&) = delete;
        HoldAnnouncement& operator=(const HoldAnnouncement&) = delete;

    private:
        SlideLinkManager& mrManager;
    };

private:
    SdDrawDocument& mrDoc;
    std::vector<SlideLinkRecord> maLinks;
    /// The kind and identifier of each link that held when the link list was last announced as
    /// changed, in document order.
    std::vector<OUString> maAnnouncedLinks;
    /// How many operations hold the announcement back now. Zero when nothing does.
    sal_Int32 mnAnnouncementsHeld = 0;

    /// Whether rRecord spans the slides its unit holds now, and for a presentation whether its
    /// sections are still one run in the order it lists them.
    bool Holds(const SlideLinkRecord& rRecord) const;

    /// Holds, with the index of every page of the document by identifier in rPageIndices.
    bool Holds(const SlideLinkRecord& rRecord,
               const std::map<OUString, sal_Int32>& rPageIndices) const;

    /// The identifiers of the slides the unit of rRecord spans now, empty when the unit is gone:
    /// the page, the section, or any section of the presentation.
    std::vector<OUString> GetUnitSlides(const SlideLinkRecord& rRecord) const;

    /// Whether the sections rSections sit next to each other in the document in that order.
    bool IsSectionRun(const std::vector<OUString>& rSections) const;

    /// The identifiers of the sections that make up the run of slides rSlides, in order: the
    /// first section starts on the first slide and the last ends on the last. Empty when the
    /// slides are not whole sections, or when one of those sections has no identifier.
    std::vector<OUString> GetSectionsOfSlides(const std::vector<OUString>& rSlides) const;

    /// rRecord with its identifiers in braced form, or nothing when rRecord is one that cannot
    /// hold. A presentation that lists no sections takes the sections that hold its slides.
    std::optional<SlideLinkRecord> Validate(const SlideLinkRecord& rRecord) const;

    /// The kind and identifier of every link that holds, in document order.
    std::vector<OUString> GetHoldingKeys() const;

    /// The records of the slides nFirstSlide on, each linked to the slide of rSource it was read
    /// from. See LinkInsertedPages.
    std::vector<SlideLinkRecord> MakeSlideRecords(SdDrawDocument& rSource,
                                                  const std::vector<OUString>& rReadNames,
                                                  sal_Int32 nFirstSlide, sal_Int32 nSlideCount,
                                                  const std::vector<sal_Int32>& rSourcePositions,
                                                  const OUString& rSourceName,
                                                  const OUString& rLastModifiedTime) const;

    /// Puts rRecords in place of the records of the same kind and unit, or adds them.
    void Put(std::vector<SlideLinkRecord> aRecords);

    /// Drops every record whose unit is gone from the document.
    void DropRecordsOfGoneUnits();

    /// Records an undo action that puts aOldLinks back, when undo is on.
    void AddUndo(std::vector<SlideLinkRecord> aOldLinks, const OUString& rComment);

    /** Refresh for the slide links rSlides, the section links rSections and the presentation
        links rPresentations from the file at rFileUrl, in one undo step named pUndoName. rName
        is the name the one unit refreshed has in the source, or empty for the name the file
        lists for a section and the name a presentation recorded. A unit the file holds nothing
        for, or whose slides could not be read, keeps its slides and has them reported in
        pNotUpdated.

        @return the number of slides replaced, or -1 when the file is no local file, when the
                time is not readable, or when the file could not be read.
    */
    sal_Int32 RefreshRecords(const std::vector<SlideLinkRecord>& rSlides,
                             const std::vector<SlideLinkRecord>& rSections,
                             const std::vector<SlideLinkRecord>& rPresentations,
                             const OUString& rFileUrl, const OUString& rLastModifiedTime,
                             const OUString& rName, TranslateId pUndoName,
                             std::vector<OString>* pNotUpdated);

    /// Reads the source slide of each slide link of rRecords from rSource, in runs of consecutive
    /// pages. Returns how many slides were replaced.
    sal_Int32 RefreshSlides(const std::vector<SlideLinkRecord>& rRecords, SdDrawDocument& rSource,
                            const OUString& rLastModifiedTime, std::vector<OString>* pNotUpdated);

    /// Gives the section of rRecord the slides rReadNames of rSource, and the name rName, or the
    /// name rSource lists for the section when rName is empty. Returns how many slides it holds
    /// afterwards, or -1 when the slides could not be read.
    sal_Int32 RefreshSection(const SlideLinkRecord& rRecord, SdDrawDocument& rSource,
                             const std::vector<OUString>& rReadNames,
                             const OUString& rLastModifiedTime, const OUString& rName);

    /// Gives the presentation of rRecord every page and every section of rSource, and the name
    /// rName, or the name it recorded when rName is empty. Returns how many slides it holds
    /// afterwards, or -1 when the pages could not be read.
    sal_Int32 RefreshPresentation(const SlideLinkRecord& rRecord, SdDrawDocument& rSource,
                                  const OUString& rLastModifiedTime, const OUString& rName);

    /// The pages of rSource the section link rRecord reads: the pages of the section the file
    /// lists under the source section identifier, or the pages before the first section for a
    /// link that names no section. Nothing when the file lists no such section or there is no
    /// such page.
    static std::optional<std::vector<OUString>> GetSectionReadNames(const SlideLinkRecord& rRecord,
                                                                    SdDrawDocument& rSource);
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
