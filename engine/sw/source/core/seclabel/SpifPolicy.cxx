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
#include <tools/XmlWalker.hxx>
#include <tools/stream.hxx>

#include <algorithm>

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
                    ++nSelected;
                ++nIdx;
            }
            if (nSelectable == 0)
                continue; // tag not applicable under this classification

            const bool bUnderMin = rTag.nMinSelection > 0 && nSelected < rTag.nMinSelection;
            const bool bOverMax = rTag.nMaxSelection >= 0 && nSelected > rTag.nMaxSelection;
            if (bUnderMin || bOverMax)
                aViolations.push_back(
                    { rTag.aName, rTag.nMinSelection, rTag.nMaxSelection, nSelected });
        }
    }
    return aViolations;
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

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
