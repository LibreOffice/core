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
#include <com/sun/star/uno/Sequence.hxx>
#include <vector>
#include "sddllapi.h"

class SdDrawDocument;

namespace sd
{
struct SlideSection
{
    OUString maName;
    OUString maId; // GUID string, e.g. "{00F38008-1855-4575-B10E-A14737D1CCA5}"
    sal_Int32 mnStartIndex; // 0-based index of first slide in this section
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
    css::uno::Sequence<css::beans::PropertyValue> GetSectionsAsPropertyValues() const;

    /// Set sections from a UNO PropertyValue sequence (from UNO property / import).
    void
    SetSectionsFromPropertyValues(const css::uno::Sequence<css::beans::PropertyValue>& rSections);

    sal_Int32 GetSectionCount() const;
    const SlideSection& GetSection(sal_Int32 nIndex) const;

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

    /// Rename a section.
    void RenameSection(sal_Int32 nSectionIndex, const OUString& rNewName);

    /// Move a section (and its slides) to a new position.
    void MoveSection(sal_Int32 nOldIndex, sal_Int32 nNewIndex);

    /// Get a snapshot of all sections (for undo support).
    std::vector<SlideSection> GetSectionsSnapshot() const;

    /// Replace all sections with the given vector (for undo support).
    /// Does not move any slides.
    void RestoreSectionsSnapshot(const std::vector<SlideSection>& rSections);

private:
    SdDrawDocument& mrDoc;
    std::vector<SlideSection> maSections;

    /// Push the sections via KIT_CALLBACK_STATE_CHANGED for .uno:SlideSections
    /// so COKit clients update without polling after async UNO dispatch.
    void NotifySectionsChanged();
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
