/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_SPIFPOLICY_HXX
#define INCLUDED_SW_INC_SPIFPOLICY_HXX

#include "swdllapi.h"
#include <rtl/ustring.hxx>
#include <vector>

class SvStream;

namespace sw::seclabel
{
/// A classification level defined by a SPIF policy (xmlspif.org).
struct SpifClassification
{
    OUString aName; ///< securityClassification/@name; default marking phrase
    OUString aColor; ///< @color: W3C name or #RRGGBB; may be empty
    sal_Int32 nLacv = 0; ///< @lacv (label and certificate value)
    sal_Int32 nHierarchy = 0; ///< @hierarchy (dominance ordering)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
};

/// A reference to a category in a tag set, by tag-set name + lacv (or all of them).
/// Used by required/excluded category relationships (optionalCategoryData).
struct SpifCategoryRef
{
    OUString aTagSetRef; ///< tagSetRef: the referenced tag set's name
    OUString aTagType; ///< @tagType of the reference
    sal_Int64 nLacv = 0; ///< @lacv of the referenced category (when !bAll)
    bool bAll = false; ///< @all: all categories of the referenced tag set
};

/// A required-category group: `aOperation` (onlyOne/oneOrMore/all) of the referenced
/// categories must be selected when the owning category is selected.
struct SpifRequiredCategory
{
    OUString aOperation; ///< operation: onlyOne / oneOrMore / all
    std::vector<SpifCategoryRef> aCategories;
};

/// A selectable value within a category tag (e.g. "CANADA" under "Releasable To").
struct SW_DLLPUBLIC SpifTagCategory
{
    OUString aName; ///< tagCategory/@name
    sal_Int64 nLacv = 0; ///< @lacv (may exceed 32 bits)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
    OUString aRequiredClass; ///< @requiredClass: classification required when this category is used
    std::vector<OUString> aExcludedClasses; ///< excludedClass: classifications this category excludes
    std::vector<SpifCategoryRef> aExcludedCategories; ///< excludedCategory: categories this excludes
    std::vector<SpifRequiredCategory> aRequiredCategories; ///< requiredCategory groups

    /// Whether this category may be selected for a NEW label under rClassification
    /// (not obsolete, not excluded, and requiredClass, if set, matches).
    bool isSelectable(const OUString& rClassification) const;
};

/// A category tag (e.g. "Releasable To") and its selectable values.
struct SpifCategoryTag
{
    OUString aName; ///< securityCategoryTag/@name
    OUString aTagType; ///< @tagType: enumerated/restrictive/permissive/tagType7/notApplicable
    OUString aEnumType; ///< @enumType when tagType=enumerated: permissive/restrictive
    bool bSingleSelection = false; ///< @singleSelection
    sal_Int32 nMinSelection = -1; ///< @minSelection; -1 = unbounded (no minimum)
    sal_Int32 nMaxSelection = -1; ///< @maxSelection; -1 = unbounded (no maximum)
    OUString aMarkingPrefix; ///< markingQualifier qualifier @qualifierCode=prefix
    OUString aMarkingSeparator; ///< qualifierCode=separator (precedes the values)
    OUString aMarkingSuffix; ///< qualifierCode=suffix
    std::vector<SpifTagCategory> aCategories;
};

/// A named set of category tags (e.g. "Release Categories").
struct SpifCategoryTagSet
{
    OUString aName; ///< securityCategoryTagSet/@name
    OUString aId; ///< @id (OID)
    std::vector<SpifCategoryTag> aTags;
};

/// A selection-constraint violation found by SpifPolicy::validate.
struct SpifViolation
{
    OUString aTagName; ///< the tag whose selection count is out of range
    sal_Int32 nMinSelection = -1; ///< required minimum (-1 if none)
    sal_Int32 nMaxSelection = -1; ///< allowed maximum (-1 if none)
    sal_Int32 nSelected = 0; ///< how many were selected
};

/// A parsed SPIF policy: policy identifier, classifications and category tag
/// sets. Privacy marks, marking rules and relationships follow.
class SW_DLLPUBLIC SpifPolicy
{
public:
    OUString aName; ///< securityPolicyId/@name
    OUString aId; ///< securityPolicyId/@id (OID)
    std::vector<SpifClassification> aClassifications;
    std::vector<SpifCategoryTagSet> aTagSets;

    /// Parse a SPIF document from rStream into this instance. Returns false if
    /// the stream is not a SPIF document.
    bool parse(SvStream& rStream);

    /// Derive the marking string for the given classification and selected
    /// categories. rSelected is indexed in tag-set/tag/category order (matching
    /// the order categories appear across aTagSets).
    OUString buildMarking(const OUString& rClassification,
                          const std::vector<bool>& rSelected) const;

    /// Check selection-count constraints (minSelection/maxSelection per tag) for
    /// the given classification and selection. rSelected is indexed as in
    /// buildMarking (selectable categories in tag-set/tag/category order). Returns
    /// one entry per violating tag; empty means valid.
    std::vector<SpifViolation> validate(const OUString& rClassification,
                                        const std::vector<bool>& rSelected) const;
};

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SPIFPOLICY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
