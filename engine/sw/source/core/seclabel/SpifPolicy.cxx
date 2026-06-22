/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SpifPolicy.hxx>

#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <tools/XmlWalker.hxx>
#include <tools/stream.hxx>

#include <algorithm>
#include <set>
#include <vector>

namespace sw::seclabel
{
namespace
{
OUString toOU(std::string_view rStr) { return OStringToOUString(rStr, RTL_TEXTENCODING_UTF8); }

// SPIF "selection" type: an integer or "unbounded". -1 represents unbounded.
sal_Int32 parseSelection(std::string_view rValue)
{
    if (rValue.empty() || rValue == "unbounded")
        return -1;
    return o3tl::toInt32(rValue);
}

SpifCategoryRef parseCategoryRef(tools::XmlWalker& rWalker)
{
    SpifCategoryRef aRef;
    aRef.aTagSetRef = toOU(rWalker.attribute("tagSetRef"_ostr));
    aRef.aTagType = toOU(rWalker.attribute("tagType"_ostr));
    aRef.nLacv = rWalker.attribute("lacv"_ostr).toInt64();
    aRef.bAll = rWalker.attribute("all"_ostr) == "true";
    return aRef;
}

SpifTagCategory parseTagCategory(tools::XmlWalker& rWalker)
{
    SpifTagCategory aCategory;
    aCategory.aName = toOU(rWalker.attribute("name"_ostr));
    aCategory.nLacv = rWalker.attribute("lacv"_ostr).toInt64();
    aCategory.bObsolete = rWalker.attribute("obsolete"_ostr) == "true";
    aCategory.aRequiredClass = toOU(rWalker.attribute("requiredClass"_ostr));

    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "excludedClass")
            aCategory.aExcludedClasses.push_back(toOU(rWalker.content()));
        else if (rWalker.name() == "excludedCategory")
            aCategory.aExcludedCategories.push_back(parseCategoryRef(rWalker));
        else if (rWalker.name() == "requiredCategory")
        {
            SpifRequiredCategory aRequired;
            aRequired.aOperation = toOU(rWalker.attribute("operation"_ostr));
            rWalker.children();
            while (rWalker.isValid())
            {
                if (rWalker.name() == "categoryGroup")
                    aRequired.aCategories.push_back(parseCategoryRef(rWalker));
                rWalker.next();
            }
            rWalker.parent();
            aCategory.aRequiredCategories.push_back(aRequired);
        }
        rWalker.next();
    }
    rWalker.parent();
    return aCategory;
}

SpifCategoryTag parseCategoryTag(tools::XmlWalker& rWalker)
{
    SpifCategoryTag aTag;
    aTag.aName = toOU(rWalker.attribute("name"_ostr));
    aTag.aTagType = toOU(rWalker.attribute("tagType"_ostr));
    aTag.aEnumType = toOU(rWalker.attribute("enumType"_ostr));
    aTag.bSingleSelection = rWalker.attribute("singleSelection"_ostr) == "true";
    aTag.nMinSelection = parseSelection(rWalker.attribute("minSelection"_ostr));
    aTag.nMaxSelection = parseSelection(rWalker.attribute("maxSelection"_ostr));

    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "tagCategory")
            aTag.aCategories.push_back(parseTagCategory(rWalker));
        else if (rWalker.name() == "markingQualifier")
        {
            rWalker.children();
            while (rWalker.isValid())
            {
                if (rWalker.name() == "qualifier")
                {
                    const OString aCode = rWalker.attribute("qualifierCode"_ostr);
                    const OUString aPhrase = toOU(rWalker.attribute("markingQualifier"_ostr));
                    if (aCode == "prefix")
                        aTag.aMarkingPrefix = aPhrase;
                    else if (aCode == "separator")
                        aTag.aMarkingSeparator = aPhrase;
                    else if (aCode == "suffix")
                        aTag.aMarkingSuffix = aPhrase;
                }
                rWalker.next();
            }
            rWalker.parent();
        }
        rWalker.next();
    }
    rWalker.parent();
    return aTag;
}

SpifCategoryTagSet parseTagSet(tools::XmlWalker& rWalker)
{
    SpifCategoryTagSet aSet;
    aSet.aName = toOU(rWalker.attribute("name"_ostr));
    aSet.aId = toOU(rWalker.attribute("id"_ostr));

    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "securityCategoryTag")
            aSet.aTags.push_back(parseCategoryTag(rWalker));
        rWalker.next();
    }
    rWalker.parent();
    return aSet;
}

// Whether the referenced category (tag-set name + lacv, or all of the tag set) is
// among the selected categories.
bool isRefSelected(const SpifCategoryRef& rRef,
                   const std::set<std::pair<OUString, sal_Int64>>& rSelected)
{
    if (rRef.bAll)
    {
        for (const auto& rEntry : rSelected)
            if (rEntry.first == rRef.aTagSetRef)
                return true;
        return false;
    }
    return rSelected.count({ rRef.aTagSetRef, rRef.nLacv }) != 0;
}

// Map a SPIF tag's type to a STANAG 4774 Category/@Type.
OUString mapTagType(const SpifCategoryTag& rTag)
{
    if (rTag.aTagType == u"restrictive"_ustr)
        return u"RESTRICTIVE"_ustr;
    if (rTag.aTagType == u"permissive"_ustr)
        return u"PERMISSIVE"_ustr;
    if (rTag.aTagType == u"enumerated"_ustr)
        return rTag.aEnumType == u"restrictive"_ustr ? u"RESTRICTIVE"_ustr : u"PERMISSIVE"_ustr;
    return u"INFORMATIVE"_ustr; // tagType7 / notApplicable
}
}

// SPIF elements are namespace-prefixed (spif:...); XmlWalker::name() yields the
// local name, so the prefix is ignored here.
bool SpifPolicy::parse(SvStream& rStream)
{
    aName.clear();
    aId.clear();
    aClassifications.clear();
    aTagSets.clear();

    tools::XmlWalker aWalker;
    if (!aWalker.open(&rStream))
        return false;

    if (aWalker.name() != "SPIF")
        return false;

    aWalker.children();
    while (aWalker.isValid())
    {
        if (aWalker.name() == "securityPolicyId")
        {
            aName = toOU(aWalker.attribute("name"_ostr));
            aId = toOU(aWalker.attribute("id"_ostr));
        }
        else if (aWalker.name() == "securityClassifications")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "securityClassification")
                {
                    SpifClassification aClass;
                    aClass.aName = toOU(aWalker.attribute("name"_ostr));
                    aClass.aColor = toOU(aWalker.attribute("color"_ostr));
                    aClass.nLacv = aWalker.attribute("lacv"_ostr).toInt32();
                    aClass.nHierarchy = aWalker.attribute("hierarchy"_ostr).toInt32();
                    aClass.bObsolete = aWalker.attribute("obsolete"_ostr) == "true";
                    aClassifications.push_back(aClass);
                }
                aWalker.next();
            }
            aWalker.parent();
        }
        else if (aWalker.name() == "securityCategoryTagSets")
        {
            aWalker.children();
            while (aWalker.isValid())
            {
                if (aWalker.name() == "securityCategoryTagSet")
                    aTagSets.push_back(parseTagSet(aWalker));
                aWalker.next();
            }
            aWalker.parent();
        }
        aWalker.next();
    }
    aWalker.parent();

    return true;
}

OUString SpifPolicy::buildMarking(const OUString& rClassification,
                                  const std::vector<bool>& rSelected) const
{
    // First cut: classification, then per tag with selected values
    // separator + prefix + space-joined names + suffix. Display modifiers
    // (noNameDisplay/suppressClassName) and multi-level marking data: TODO.
    OUString aMarking = rClassification;
    size_t nIdx = 0;
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            OUString aValues;
            for (const auto& rCategory : rTag.aCategories)
            {
                // Walk only the categories the dialog shows for this classification,
                // so rSelected (built from the filtered rows) stays index-aligned.
                if (!rCategory.isSelectable(rClassification))
                    continue;
                if (nIdx < rSelected.size() && rSelected[nIdx])
                {
                    if (!aValues.isEmpty())
                        aValues += u" "_ustr;
                    aValues += rCategory.aName;
                }
                ++nIdx;
            }
            if (!aValues.isEmpty())
                aMarking
                    += rTag.aMarkingSeparator + rTag.aMarkingPrefix + aValues + rTag.aMarkingSuffix;
        }
    }
    return aMarking;
}

std::vector<SpifViolation> SpifPolicy::validate(const OUString& rClassification,
                                                const std::vector<bool>& rSelected) const
{
    std::vector<SpifViolation> aViolations;

    // First pass: record selected categories (tag-set name + lacv) and check the
    // per-tag min/max selection counts.
    std::set<std::pair<OUString, sal_Int64>> aSelectedSet;
    size_t nIdx = 0;
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            sal_Int32 nSelectable = 0;
            sal_Int32 nSelected = 0;
            for (const auto& rCategory : rTag.aCategories)
            {
                if (!rCategory.isSelectable(rClassification))
                    continue;
                ++nSelectable;
                if (nIdx < rSelected.size() && rSelected[nIdx])
                {
                    ++nSelected;
                    aSelectedSet.emplace(rTagSet.aName, rCategory.nLacv);
                }
                ++nIdx;
            }
            if (nSelectable == 0)
                continue; // tag not applicable under this classification

            if (rTag.nMinSelection > 0 && nSelected < rTag.nMinSelection)
                aViolations.push_back({ SpifViolationType::MinSelection, rTag.aName,
                                        rTag.nMinSelection, rTag.nMaxSelection, nSelected });
            if (rTag.nMaxSelection >= 0 && nSelected > rTag.nMaxSelection)
                aViolations.push_back({ SpifViolationType::MaxSelection, rTag.aName,
                                        rTag.nMinSelection, rTag.nMaxSelection, nSelected });
        }
    }

    // Second pass: for each selected category, check excludedCategory and
    // requiredCategory against the selected set.
    nIdx = 0;
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            for (const auto& rCategory : rTag.aCategories)
            {
                if (!rCategory.isSelectable(rClassification))
                    continue;
                const bool bSelected = nIdx < rSelected.size() && rSelected[nIdx];
                ++nIdx;
                if (!bSelected)
                    continue;

                for (const auto& rRef : rCategory.aExcludedCategories)
                    if (isRefSelected(rRef, aSelectedSet))
                        aViolations.push_back(
                            { SpifViolationType::ExcludedCategory, rCategory.aName, -1, -1, 0 });

                for (const auto& rReq : rCategory.aRequiredCategories)
                {
                    sal_Int32 nRefsSelected = 0;
                    for (const auto& rRef : rReq.aCategories)
                        if (isRefSelected(rRef, aSelectedSet))
                            ++nRefsSelected;
                    const sal_Int32 nTotal = static_cast<sal_Int32>(rReq.aCategories.size());
                    bool bOk = true;
                    if (rReq.aOperation == u"onlyOne"_ustr)
                        bOk = nRefsSelected == 1;
                    else if (rReq.aOperation == u"oneOrMore"_ustr)
                        bOk = nRefsSelected >= 1;
                    else if (rReq.aOperation == u"all"_ustr)
                        bOk = nRefsSelected == nTotal;
                    if (!bOk)
                        aViolations.push_back(
                            { SpifViolationType::RequiredCategory, rCategory.aName, -1, -1, 0 });
                }
            }
        }
    }
    return aViolations;
}

bool SpifPolicy::matchesLabel(const StanagLabel& rLabel) const
{
    if (aId.isEmpty())
        return false;
    OUString sLabelId;
    if (!rLabel.aPolicyId.startsWithIgnoreAsciiCase(u"urn:oid:", &sLabelId))
        sLabelId = rLabel.aPolicyId;
    return sLabelId == aId;
}

bool SpifPolicySet::loadFile(const OUString& rFileUrl)
{
    SvFileStream aStream(rFileUrl, StreamMode::READ);
    if (!aStream.IsOpen())
        return false;
    SpifPolicy aPolicy;
    if (!aPolicy.parse(aStream))
        return false;
    aPolicies.push_back(std::move(aPolicy));
    return true;
}

void SpifPolicySet::loadFromDir(const OUString& rDirUrl)
{
    osl::Directory aDir(rDirUrl);
    if (aDir.open() != osl::FileBase::E_None)
        return;

    // Collect the *.xml entries first so loading order is deterministic (the
    // directory's own iteration order is not).
    std::vector<OUString> aFiles;
    osl::DirectoryItem aItem;
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        osl::FileStatus aStatus(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL);
        if (aItem.getFileStatus(aStatus) != osl::FileBase::E_None)
            continue;
        if (aStatus.getFileType() == osl::FileStatus::Directory)
            continue;
        const OUString sUrl = aStatus.getFileURL();
        if (sUrl.endsWithIgnoreAsciiCase(u".xml"))
            aFiles.push_back(sUrl);
    }
    std::sort(aFiles.begin(), aFiles.end());

    for (const auto& rUrl : aFiles)
        loadFile(rUrl);
}

const SpifPolicy* SpifPolicySet::findByLabel(const StanagLabel& rLabel) const
{
    for (const auto& rPolicy : aPolicies)
    {
        if (rPolicy.matchesLabel(rLabel))
            return &rPolicy;
    }
    return nullptr;
}

bool SpifTagCategory::isSelectable(const OUString& rClassification) const
{
    if (bObsolete)
        return false;
    if (!aRequiredClass.isEmpty() && aRequiredClass != rClassification)
        return false;
    return std::find(aExcludedClasses.begin(), aExcludedClasses.end(), rClassification)
           == aExcludedClasses.end();
}

StanagLabel SpifPolicy::buildLabel(const OUString& rClassification,
                                   const std::vector<bool>& rSelected,
                                   const OUString& rCreationDateTime,
                                   const OUString& rReviewDateTime) const
{
    StanagLabel aLabel;
    aLabel.aPolicyName = aName;
    if (!aId.isEmpty())
        aLabel.aPolicyId = u"urn:oid:"_ustr + aId;
    aLabel.aClassification = rClassification;
    aLabel.aCreationDateTime = rCreationDateTime;
    aLabel.aReviewDateTime = rReviewDateTime;

    size_t nIdx = 0;
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            StanagCategory aCategory;
            aCategory.aTagName = rTag.aName;
            aCategory.aType = mapTagType(rTag);
            for (const auto& rCat : rTag.aCategories)
            {
                if (!rCat.isSelectable(rClassification))
                    continue;
                if (nIdx < rSelected.size() && rSelected[nIdx])
                    aCategory.aValues.push_back(rCat.aName);
                ++nIdx;
            }
            if (!aCategory.aValues.empty())
                aLabel.aCategories.push_back(aCategory);
        }
    }
    return aLabel;
}

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
