/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <cpo/uno/Sequence.hxx>
#include <string_view>
#include <vector>
#include "sddllapi.h"

#include <COKit/COKit.hxx>

class SdDrawDocument;

namespace tools
{
class JsonWriter;
}

namespace sd
{
/** The section of another presentation a section is linked to.

    A linked section holds a copy of the slides of its source section, one slide for each, in the
    order the source holds them. An empty maSourceReference means the link names no source, and
    every other field is then empty too. A link that names a source holds for its section only while
    the section spans the slides of maPageGuids, in that order.
*/
struct SlideSectionLink
{
    /// The source document, in the vnd.collabora.slide-source: form a linked page records.
    OUString maSourceReference;
    /// The braced identifier of the section in the source document.
    OUString maSourceSectionId;
    /// The name the source section had when its slides were last read.
    OUString maSourceSectionName;
    /// The time the source document was last modified when the slides were last read, as the
    /// integrator reported it. Empty when no time was reported.
    OUString maSourceModifiedTime;
    /// The braced identifier of the source slide each slide of the section was read from, one entry
    /// for each slide of the section, in slide order.
    std::vector<OUString> maSourcePageGuids;
    /// The braced identifier of each slide of this document the section held when it was linked,
    /// in slide order, one entry for each entry of maSourcePageGuids. The section holds a copy of
    /// its source section while it spans these slides in this order. This list is not saved,
    /// since a loaded section takes the slides it spans on load.
    std::vector<OUString> maPageGuids;

    /// Whether the link names a source section.
    bool IsLinked() const { return !maSourceReference.isEmpty(); }
};

struct SlideSection
{
    OUString maName;
    OUString maId; // GUID string, e.g. "{00F38008-1855-4575-B10E-A14737D1CCA5}"
    sal_Int32 mnStartIndex; // 0-based index of first slide in this section
    SlideSectionLink maLink;
};

/** Manages slide sections for a presentation document.
 *
 *  Sections group consecutive slides under a named header.
 *  The data is persisted via the "SlideSections" UNO property
 *  on the document model for PPTX/ODP round-trip compatibility.
 *
 *  Sections are sorted by mnStartIndex. Each section spans from
 *  its start index to the next section's start (or end of deck).
 */
class SD_DLLPUBLIC SlideSectionManager
{
public:
    explicit SlideSectionManager(SdDrawDocument& rDoc);

    /// Get sections as a UNO PropertyValue sequence (for UNO property / export).
    cpo::uno::Sequence<css::beans::PropertyValue> GetSectionsAsPropertyValues() const;

    /// Set sections from a UNO PropertyValue sequence (from UNO property / import).
    void
    SetSectionsFromPropertyValues(const cpo::uno::Sequence<css::beans::PropertyValue>& rSections);

    sal_Int32 GetSectionCount() const;
    const SlideSection& GetSection(sal_Int32 nIndex) const;

    /// The number of slides the section at nSectionIndex spans.
    sal_Int32 GetSectionSlideCount(sal_Int32 nSectionIndex) const;

    /** The link of the section at nSectionIndex to a section of another presentation, or nullptr
        when the section has none.

        A section stops holding a copy of its source section when a slide is inserted into it,
        removed from it or moved within it, or when the section is split or merged, so it has no
        link while it spans other slides than it was linked with. Undoing the change makes the
        section span those slides again, and gives it its link back.
    */
    const SlideSectionLink* GetSectionLink(sal_Int32 nSectionIndex) const;

    /** Link the section at nSectionIndex to the source section rLink names, or take the link off
        when rLink is not linked. The slides the section spans now are the slides the link holds.

        rLink must name a source document, a source section by a valid identifier, and one valid
        source slide identifier for each slide of the section, which spans at least one slide. Its
        identifiers are kept in braced form. The change is one undo action.

        @return true when the section holds rLink afterwards, false when rLink was refused or
                nSectionIndex names no section.
    */
    bool SetSectionLink(sal_Int32 nSectionIndex, const SlideSectionLink& rLink);

    /** Add a section named rName for the nSlideCount slides from nStartSlideIndex on, linked to
        the source section rLink names, or to none when rLink is not linked. The sections and the
        link each record an undo action of their own.

        The slides that follow keep a section apart from the new one. A section that starts at
        nStartSlideIndex starts after the new slides instead. Slides that follow inside the section
        the new slides went into get a section of their own with the name of that section, and
        slides that follow in no section get a section with the default name.

        @return true when the section was added, and linked when rLink is linked, false when
                nothing changed.
    */
    bool AddInsertedSection(sal_Int32 nStartSlideIndex, sal_Int32 nSlideCount,
                            const OUString& rName, const SlideSectionLink& rLink);

    /** Announce the slide link list as changed when the sections that hold a copy of their source
        section are not the ones they were at the last announcement. The linked sections are part
        of that list, and a section stops holding its copy when the slides it spans change.
    */
    void NotifyLinksIfChanged();

    /** Record that the slide with the identifier rOldPageGuid was replaced by a slide read from the
        same source slide, which holds the identifier rNewPageGuid, so a linked section that holds
        the old slide holds the new one in its place.
    */
    void ReplaceLinkedPage(const OUString& rOldPageGuid, const OUString& rNewPageGuid);

    /// Write the sections as a JSON array under the given key. Each entry
    /// carries the section's name, its id when it has one, the 0-based
    /// index of its first slide and the number of slides it spans.
    void WriteSectionsJson(::tools::JsonWriter& rJsonWriter, std::string_view aArrayName) const;

    /// The index of the section with the identifier rId, in braced form, or -1 when no section
    /// holds it.
    sal_Int32 GetSectionIndexForId(std::u16string_view rId) const;

    /// Returns the section index that contains the given slide, or -1.
    sal_Int32 GetSectionIndexForSlide(sal_Int32 nSlideIndex) const;

    /// Returns true if nSlideIndex is the first slide of a section.
    bool IsSectionStart(sal_Int32 nSlideIndex) const;

    /// Returns the start indices of all sections (for the layouter).
    std::vector<sal_Int32> GetSectionStartIndices() const;

    /// Add a new section starting before the given slide index.
    void AddSection(sal_Int32 nStartSlideIndex, const OUString& rName);

    /// Remove a section; its slides merge into the previous section.
    void RemoveSection(sal_Int32 nSectionIndex);

    /// Delete every slide belonging to nSectionIndex via the document's
    /// XDrawPages, iterating in reverse so that the indices of the slides
    /// still to be deleted stay valid.  Never deletes the last remaining
    /// slide in the document.  The caller owns undo grouping and view
    /// refresh; the now-empty section metadata is auto-erased by
    /// OnSlideRemoved as the slides are removed.
    void RemoveSectionSlides(sal_Int32 nSectionIndex);

    /// Rename a section.
    void RenameSection(sal_Int32 nSectionIndex, const OUString& rNewName);

    /// Move a section (and its slides) to a new position.
    void MoveSection(sal_Int32 nOldIndex, sal_Int32 nNewIndex);

    /// Get a snapshot of all sections (for undo support).
    std::vector<SlideSection> GetSectionsSnapshot() const;

    /// Replace all sections with the given vector (for undo support).
    /// Does not move any slides.
    void RestoreSectionsSnapshot(const std::vector<SlideSection>& rSections);

    /// Shift section anchors after a slide is inserted; a slide at a
    /// section boundary stays in the preceding section.
    void OnSlideInserted(sal_Int32 nNewStandardIndex);

    /// Shift section anchors after a slide is removed; zero-slide
    /// sections are dropped.
    void OnSlideRemoved(sal_Int32 nOldStandardIndex);

private:
    SdDrawDocument& mrDoc;
    std::vector<SlideSection> maSections;
    /// The identifiers of the sections that held a copy of their source section when the slide link
    /// list was last announced as changed, in document order.
    std::vector<OUString> maAnnouncedLinks;

    /// The identifiers of the sections that hold a copy of their source section now, in document
    /// order.
    std::vector<OUString> GetLinkedSectionIds() const;

    /// The braced identifiers of the slides the section at nSectionIndex spans, in slide order.
    std::vector<OUString> GetSectionPageGuids(sal_Int32 nSectionIndex) const;

    /// rLink with its identifiers in braced form, holding the slides the section at nSectionIndex
    /// spans now, or a link that is not linked when rLink is not one that section can hold.
    SlideSectionLink ValidateLink(const SlideSectionLink& rLink, sal_Int32 nSectionIndex) const;

    /// Push the sections via COKitCallbackType::STATE_CHANGED for .uno:SlideSections
    /// so COKit clients update without polling after async UNO dispatch.
    void NotifySectionsChanged();
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
