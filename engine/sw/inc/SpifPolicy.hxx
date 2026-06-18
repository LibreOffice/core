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

/// A selectable value within a category tag (e.g. "CANADA" under "Releasable To").
struct SpifTagCategory
{
    OUString aName; ///< tagCategory/@name
    sal_Int64 nLacv = 0; ///< @lacv (may exceed 32 bits)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
};

/// A category tag (e.g. "Releasable To") and its selectable values.
struct SpifCategoryTag
{
    OUString aName; ///< securityCategoryTag/@name
    OUString aTagType; ///< @tagType: enumerated/restrictive/permissive/tagType7/notApplicable
    OUString aEnumType; ///< @enumType when tagType=enumerated: permissive/restrictive
    bool bSingleSelection = false; ///< @singleSelection
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
};

} // namespace sw::seclabel

#endif // INCLUDED_SW_INC_SPIFPOLICY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
