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
struct SlideSection
{
    OUString maName;
    OUString maId; // GUID string, e.g. "{00F38008-1855-4575-B10E-A14737D1CCA5}"
    sal_Int32 mnStartIndex; // 0-based index of first slide in this section
};

/// A run of consecutive slides that one section spans, or that sit before the first section.
struct SlideSectionRun
{
    /// How many slides the run holds, at least one.
    sal_Int32 mnSlideCount = 0;
    /// The name of the section, or the name given to the slides before the first section.
    OUString maName;
    /// The identifier of the section, empty for the slides before the first section and for a
    /// section that has none.
    OUString maId;
    /// Whether the run is the slides before the first section, which belong to no section.
    bool mbLoose = false;
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

    /// The braced identifiers of the slides the section at nSectionIndex spans, in slide order.
    /// Empty when nSectionIndex names no section.
    std::vector<OUString> GetSectionPageGuids(sal_Int32 nSectionIndex) const;

    /** Add a section named rName for the nSlideCount slides from nStartSlideIndex on. One undo
        action.

        The slides that follow keep a section apart from the new one. A section that starts at
        nStartSlideIndex starts after the new slides instead. Slides that follow inside the section
        the new slides went into get a section of their own with the name of that section, and
        slides that follow in no section get a section with the default name.

        @return true when the section was added, false when nothing changed.
    */
    bool AddInsertedSection(sal_Int32 nStartSlideIndex, sal_Int32 nSlideCount,
                            const OUString& rName);

    /** Adds one section per run of rRuns over the nSlideCount slides from nStartSlideIndex on,
        the runs one after the other, each named after its run. The runs add up to nSlideCount,
        or nothing changes. One undo action. The slides that follow keep a section apart from
        the new ones, as for AddInsertedSection.

        @return the identifiers of the sections added, in order, or none when nothing changed.
    */
    std::vector<OUString> AddInsertedSections(sal_Int32 nStartSlideIndex, sal_Int32 nSlideCount,
                                              const std::vector<SlideSectionRun>& rRuns);

    /** The slides of the document as runs, in slide order: the slides before the first section
        as one loose run named rLooseName, or the default name of a section when rLooseName is
        empty, with no identifier, then one run per section that spans a slide, with the name
        and the identifier of that section.
    */
    std::vector<SlideSectionRun> GetSlideRuns(const OUString& rLooseName) const;

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

    /// Push the sections via COKitCallbackType::STATE_CHANGED for .uno:SlideSections
    /// so COKit clients update without polling after async UNO dispatch.
    void NotifySectionsChanged();
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
