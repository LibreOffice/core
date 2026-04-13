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
 *  The data is persisted via the document's InteropGrabBag
 *  (key "OOXSectionList") for PPTX/ODP round-trip compatibility.
 *
 *  Sections are sorted by mnStartIndex. Each section spans from
 *  its start index to the next section's start (or end of deck).
 */
class SD_DLLPUBLIC SlideSectionManager
{
public:
    explicit SlideSectionManager(SdDrawDocument& rDoc);

    /// Load sections from the document's InteropGrabBag.
    void LoadFromGrabBag() const;

    /// Save sections back to the document's InteropGrabBag.
    void SaveToGrabBag();

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

    bool IsLoaded() const { return mbLoaded; }

    /// Mark section data as stale; next access will reload from grab bag.
    void Invalidate();

private:
    SdDrawDocument& mrDoc;
    mutable std::vector<SlideSection> maSections;
    mutable bool mbLoaded = false;

    void EnsureLoaded() const;
};

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
