/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVX_SECLABEL_SPIFPOLICY_HXX
#define INCLUDED_SVX_SECLABEL_SPIFPOLICY_HXX

#include <svx/svxdllapi.h>
#include <svx/seclabel/StanagLabel.hxx>
#include <rtl/ustring.hxx>
#include <vector>

class SvStream;

namespace svx::seclabel
{
/// A classification level defined by a SPIF policy (xmlspif.org).
struct SpifClassification
{
    OUString aName; ///< securityClassification/@name; default marking phrase
    OUString aColor; ///< @color: W3C name or #RRGGBB; may be empty
    sal_Int64 nLacv = 0; ///< @lacv (label and certificate value; may exceed 32 bits)
    sal_Int64 nHierarchy = 0; ///< @hierarchy (dominance ordering; may exceed 32 bits)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
    OUString aMarkingPhrase; ///< markingData @phrase (shown when noNameDisplay)
    bool bNoNameDisplay = false; ///< markingData code: show phrase, not the name
    bool bSuppressClassName = false; ///< markingData code: omit the class from the marking
    OUString aReplacePolicyPhrase; ///< markingData replacePolicy @phrase (e.g. COSMIC); replaces ownership
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
struct SVXCORE_DLLPUBLIC SpifTagCategory
{
    OUString aName; ///< tagCategory/@name
    sal_Int64 nLacv = 0; ///< @lacv (may exceed 32 bits)
    bool bObsolete = false; ///< @obsolete: not for newly created labels
    OUString aRequiredClass; ///< @requiredClass: classification required when this category is used
    OUString aMarkingPhrase; ///< markingData @phrase (shown when noNameDisplay)
    bool bNoNameDisplay = false; ///< markingData code: show phrase, not the category name
    OUString aReplacePolicyPhrase; ///< markingData replacePolicy @phrase (e.g. NATO/EAPC); replaces ownership
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
    bool bWatermark = false; ///< markingQualifier @markingCode=waterMark
    bool bDocumentStart = false; ///< @markingCode=documentStart (cover marking)
    bool bDocumentEnd = false; ///< @markingCode=documentEnd (end-page marking)
    bool bPortionMarking = false; ///< @markingCode=portionMarking (mark a selected portion)
    std::vector<SpifTagCategory> aCategories;
};

/// A named set of category tags (e.g. "Release Categories").
struct SpifCategoryTagSet
{
    OUString aName; ///< securityCategoryTagSet/@name
    OUString aId; ///< @id (OID)
    std::vector<SpifCategoryTag> aTags;
};

/// The kind of constraint a SpifViolation reports.
enum class SpifViolationType
{
    MinSelection, ///< fewer than minSelection categories chosen in a tag
    MaxSelection, ///< more than maxSelection categories chosen in a tag
    ExcludedCategory, ///< a selected category excludes another selected category
    RequiredCategory, ///< a selected category's requiredCategory operation is unmet
};

/// A constraint violation found by SpifPolicy::validate.
struct SpifViolation
{
    SpifViolationType eType = SpifViolationType::MinSelection;
    OUString aName; ///< the tag (min/max) or category (required/excluded) at fault
    sal_Int32 nMinSelection = -1; ///< required minimum (-1 if none)
    sal_Int32 nMaxSelection = -1; ///< allowed maximum (-1 if none)
    sal_Int32 nSelected = 0; ///< how many were selected (min/max)
};

/// A parsed SPIF policy: policy identifier, classifications and category tag
/// sets. Privacy marks, marking rules and relationships follow.
class SVXCORE_DLLPUBLIC SpifPolicy
{
public:
    OUString aName; ///< securityPolicyId/@name
    OUString aId; ///< securityPolicyId/@id (OID)
    std::vector<SpifClassification> aClassifications;
    std::vector<SpifCategoryTagSet> aTagSets;

    /// Parse a SPIF document from rStream into this instance. Returns false if
    /// the stream is not a SPIF document.
    bool parse(SvStream& rStream);

    /// Derive the ADatP-4774.2-conformant visual marking from a label (the label
    /// is the authoritative source; ADatP defers rendering to this SPIF). An
    /// ownership prefix (a `replacePolicy` phrase, e.g. `NATO/EAPC` or `COSMIC`)
    /// leads when the policy specifies one. Each category group is `markingQualifier`
    /// prefix + values joined by `separator` (normalized to single surrounding
    /// spaces) + suffix; groups render in policy tag-set order, joined by a space;
    /// values keep the label's order (no sort). Value/classification text is the
    /// `markingData` phrase when `noNameDisplay`, else the name; `suppressClassName`
    /// drops the classification.
    OUString deriveMarking(const StanagLabel& rLabel) const;

    /// Whether the selection calls for a watermark / a cover (documentStart) /
    /// an end-page (documentEnd) marking: true if any selected category belongs to
    /// a tag carrying that markingQualifier markingCode. rSelected is indexed as in
    /// buildLabel (selectable categories in tag-set/tag/category order). These
    /// placements are policy-driven only.
    bool wantsWatermark(const OUString& rClassification,
                        const std::vector<bool>& rSelected) const;
    bool wantsDocumentStart(const OUString& rClassification,
                            const std::vector<bool>& rSelected) const;
    bool wantsDocumentEnd(const OUString& rClassification,
                          const std::vector<bool>& rSelected) const;
    bool wantsPortionMarking(const OUString& rClassification,
                             const std::vector<bool>& rSelected) const;

    /// Check selection-count constraints (minSelection/maxSelection per tag) for
    /// the given classification and selection. rSelected is indexed as in buildLabel (selectable categories in
    /// tag-set/tag/category order). Returns
    /// one entry per violating tag; empty means valid.
    std::vector<SpifViolation> validate(const OUString& rClassification,
                                        const std::vector<bool>& rSelected) const;

    /// Build a STANAG 4774 label from the selection. rSelected is indexed over the
    /// selectable categories in tag-set/tag/category order (the dialog's filtered
    /// rows); the timestamps are passed through to the label.
    StanagLabel buildLabel(const OUString& rClassification, const std::vector<bool>& rSelected,
                           const OUString& rCreationDateTime,
                           const OUString& rReviewDateTime) const;

    /// Whether this policy is the one rLabel was created under, matched by OID
    /// (the label's PolicyIdentifier/@URI against this policy's securityPolicyId/@id,
    /// tolerating an optional "urn:oid:" prefix). Drives the choice between
    /// structured edit (match) and the foreign-policy read-only/re-label flow.
    bool matchesLabel(const StanagLabel& rLabel) const;

private:
    /// True if any selected category belongs to a tag whose pFlag member is set
    /// (shared by the wants* placement queries). rSelected is indexed as in buildLabel.
    bool anySelectedTag(const OUString& rClassification, const std::vector<bool>& rSelected,
                        bool SpifCategoryTag::*pFlag) const;
};

/// The SPIF policies provisioned for this session (org + user). The label dialog
/// offers all of them; an existing label is matched to one by OID.
class SVXCORE_DLLPUBLIC SpifPolicySet
{
public:
    std::vector<SpifPolicy> aPolicies;

    /// Parse rFileUrl as a SPIF policy and append it on success, replacing an
    /// already-loaded policy that declares the same OID. Returns false if it is
    /// unreadable or not a SPIF document.
    bool loadFile(const OUString& rFileUrl);

    /// Parse every *.xml directly under rDirUrl as a SPIF policy, appending those
    /// that parse, in filename order. A missing directory or non-SPIF files are skipped.
    void loadFromDir(const OUString& rDirUrl);

    /// Load the policies provisioned for this session: every *.xml the WOPI host synced
    /// into the system config tree the client mounted for us (COKit::getSystemConfigDir()
    /// + /spif, the administrator's org-wide set) and into the jail's user config dir
    /// under spif/ ($(userurl)/spif, this user's own set), the latter winning on a
    /// repeated policy OID. Falls back to the dev stopgap policy shipped in the
    /// installation when neither yields anything. The single provisioning source
    /// shared by the dialog and the on-load banner.
    void loadProvisioned();

    /// The provisioned policy rLabel was created under (first OID match), or nullptr.
    const SpifPolicy* findByLabel(const StanagLabel& rLabel) const;

    bool empty() const { return aPolicies.empty(); }
};

} // namespace svx::seclabel

#endif // INCLUDED_SVX_SECLABEL_SPIFPOLICY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
