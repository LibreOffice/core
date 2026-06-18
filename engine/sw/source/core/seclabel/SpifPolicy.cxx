/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SpifPolicy.hxx>

#include <tools/XmlWalker.hxx>
#include <tools/stream.hxx>

namespace sw::seclabel
{
namespace
{
OUString toOU(std::string_view rStr) { return OStringToOUString(rStr, RTL_TEXTENCODING_UTF8); }

SpifTagCategory parseTagCategory(tools::XmlWalker& rWalker)
{
    SpifTagCategory aCategory;
    aCategory.aName = toOU(rWalker.attribute("name"_ostr));
    aCategory.nLacv = rWalker.attribute("lacv"_ostr).toInt64();
    aCategory.bObsolete = rWalker.attribute("obsolete"_ostr) == "true";
    return aCategory;
}

SpifCategoryTag parseCategoryTag(tools::XmlWalker& rWalker)
{
    SpifCategoryTag aTag;
    aTag.aName = toOU(rWalker.attribute("name"_ostr));
    aTag.aTagType = toOU(rWalker.attribute("tagType"_ostr));
    aTag.aEnumType = toOU(rWalker.attribute("enumType"_ostr));
    aTag.bSingleSelection = rWalker.attribute("singleSelection"_ostr) == "true";

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

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
