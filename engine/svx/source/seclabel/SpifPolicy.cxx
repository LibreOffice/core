/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/seclabel/SpifPolicy.hxx>

#include <config_folders.h>

#include <comphelper/kit.hxx>
#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/XmlWalker.hxx>
#include <tools/stream.hxx>
#include <unotools/pathoptions.hxx>

#include <algorithm>
#include <set>
#include <vector>

namespace svx::seclabel
{
namespace
{
// TODO dev stopgap: fixed policy path. Replaced by WOPI provisioning (Phase F).
// The file sits beside the sample TSCP policies in the installation.
OUString getDevPolicyUrl()
{
    OUString sUrl(u"$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER
                  "/classification/spif-collabora.xml"_ustr);
    rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}

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

// A parsed markingData element: its phrase and the display codes we honour.
struct MarkingData
{
    OUString aPhrase;
    bool bNoNameDisplay = false;
    bool bSuppressClassName = false;
    bool bReplacePolicy = false;
};

// Parse a markingData element (walker positioned on it): @phrase plus its <code>s.
// noMarkingDisplay needs no flag: output uses the name, which is already the default.
MarkingData parseMarkingData(tools::XmlWalker& rWalker)
{
    MarkingData aData;
    aData.aPhrase = toOU(rWalker.attribute("phrase"_ostr));
    rWalker.children();
    while (rWalker.isValid())
    {
        if (rWalker.name() == "code")
        {
            const OString aCode = rWalker.content();
            if (aCode == "noNameDisplay")
                aData.bNoNameDisplay = true;
            else if (aCode == "suppressClassName")
                aData.bSuppressClassName = true;
            else if (aCode == "replacePolicy")
                aData.bReplacePolicy = true;
        }
        rWalker.next();
    }
    rWalker.parent();
    return aData;
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
        if (rWalker.name() == "markingData")
        {
            // A value may carry several markingData; accumulate rather than clobber
            // (e.g. a noNameDisplay phrase plus a replacePolicy phrase).
            const MarkingData aData = parseMarkingData(rWalker);
            if (aData.bNoNameDisplay)
            {
                aCategory.aMarkingPhrase = aData.aPhrase;
                aCategory.bNoNameDisplay = true;
            }
            if (aData.bReplacePolicy)
                aCategory.aReplacePolicyPhrase = aData.aPhrase;
        }
        else if (rWalker.name() == "excludedClass")
        {
            // The text node carries whatever whitespace the policy is formatted
            // with; isSelectable() compares it to a classification name exactly,
            // so untrimmed content would let an excluded category through.
            const OString aExcluded = rWalker.content();
            aCategory.aExcludedClasses.push_back(toOU(o3tl::trim(aExcluded)));
        }
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
            const OString aMarkingCode = rWalker.attribute("markingCode"_ostr);
            if (aMarkingCode == "waterMark")
                aTag.bWatermark = true;
            else if (aMarkingCode == "documentStart")
                aTag.bDocumentStart = true;
            else if (aMarkingCode == "documentEnd")
                aTag.bDocumentEnd = true;
            else if (aMarkingCode == "portionMarking")
                aTag.bPortionMarking = true;
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

// Whether a marking fragment begins with punctuation that attaches to the preceding
// word without a space (a comma, or the "-STAFF" style Administrative join).
bool leadsWithHugChar(std::u16string_view rText)
{
    if (rText.empty())
        return false;
    const sal_Unicode c = rText.front();
    return c == ',' || c == ';' || c == '.' || c == ':' || c == '-';
}

// Normalize a value separator for the marking: exactly one trailing space, and one
// leading space unless it hugs the preceding word (so "//" -> " // " but "," -> ", ").
// An empty separator falls back to a single space.
OUString normalizeSeparator(std::u16string_view rSep)
{
    const std::u16string_view aTrimmed = o3tl::trim(rSep);
    if (aTrimmed.empty())
        return u" "_ustr;
    return (leadsWithHugChar(aTrimmed) ? OUString() : u" "_ustr) + aTrimmed + u" "_ustr;
}

// Append a marking fragment, preceded by a single space unless the output is empty or
// the fragment hugs (begins with attaching punctuation, e.g. "-STAFF").
void appendPart(OUStringBuffer& rOut, std::u16string_view rPart)
{
    if (rPart.empty())
        return;
    if (!rOut.isEmpty() && !leadsWithHugChar(rPart))
        rOut.append(u" ");
    rOut.append(rPart);
}

// The policy tag category with this name within a tag, or nullptr.
const SpifTagCategory* findTagCategory(const SpifCategoryTag& rTag, std::u16string_view rName)
{
    for (const auto& rCategory : rTag.aCategories)
    {
        if (rCategory.aName == rName)
            return &rCategory;
    }
    return nullptr;
}

// The policy tag with this name across all tag sets, or nullptr.
const SpifCategoryTag* findTag(const std::vector<SpifCategoryTagSet>& rSets,
                               std::u16string_view rName)
{
    for (const auto& rSet : rSets)
    {
        for (const auto& rTag : rSet.aTags)
        {
            if (rTag.aName == rName)
                return &rTag;
        }
    }
    return nullptr;
}

// The marking text for a resolved tag category: its phrase when noNameDisplay (which
// may be empty -> the value is suppressed), else the value name itself.
OUString categoryDisplay(const SpifTagCategory* pCategory, const OUString& rValueName)
{
    if (pCategory && pCategory->bNoNameDisplay)
        return pCategory->aMarkingPhrase;
    return rValueName;
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
                    aClass.nLacv = aWalker.attribute("lacv"_ostr).toInt64();
                    aClass.nHierarchy = aWalker.attribute("hierarchy"_ostr).toInt64();
                    aClass.bObsolete = aWalker.attribute("obsolete"_ostr) == "true";
                    aWalker.children();
                    while (aWalker.isValid())
                    {
                        if (aWalker.name() == "markingData")
                        {
                            // Accumulate across markingData (e.g. TOP SECRET carries a
                            // replacePolicy=COSMIC alongside its display markingData).
                            const MarkingData aData = parseMarkingData(aWalker);
                            if (aData.bNoNameDisplay)
                            {
                                aClass.aMarkingPhrase = aData.aPhrase;
                                aClass.bNoNameDisplay = true;
                            }
                            if (aData.bSuppressClassName)
                                aClass.bSuppressClassName = true;
                            if (aData.bReplacePolicy)
                                aClass.aReplacePolicyPhrase = aData.aPhrase;
                        }
                        aWalker.next();
                    }
                    aWalker.parent();
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

OUString SpifPolicy::deriveMarking(const StanagLabel& rLabel) const
{
    OUStringBuffer aOut;

    const SpifClassification* pClass = nullptr;
    for (const auto& rCandidate : aClassifications)
    {
        if (rCandidate.aName == rLabel.aClassification)
        {
            pClass = &rCandidate;
            break;
        }
    }

    // Ownership prefix. The marking always leads with the policy name
    // (securityPolicyId/@name); a selected value's replacePolicy phrase (e.g. Context ->
    // "NATO/EAPC") replaces it, and the classification's replacePolicy (e.g. TOP SECRET
    // -> "COSMIC") overrides that in turn.
    OUString aOwnership = aName;
    for (const auto& rCat : rLabel.aCategories)
    {
        const SpifCategoryTag* pTag = findTag(aTagSets, rCat.aTagName);
        if (!pTag)
            continue;
        for (const auto& rValue : rCat.aValues)
        {
            const SpifTagCategory* pTagCat = findTagCategory(*pTag, rValue);
            if (pTagCat && !pTagCat->aReplacePolicyPhrase.isEmpty())
                aOwnership = pTagCat->aReplacePolicyPhrase;
        }
    }
    if (pClass && !pClass->aReplacePolicyPhrase.isEmpty())
        aOwnership = pClass->aReplacePolicyPhrase;
    appendPart(aOut, aOwnership);

    // Classification: its phrase when noNameDisplay, else the name; suppressClassName
    // drops it entirely. An unknown classification renders verbatim.
    OUString aClassText;
    if (!pClass)
        aClassText = rLabel.aClassification;
    else if (!pClass->bSuppressClassName)
        aClassText = (pClass->bNoNameDisplay && !pClass->aMarkingPhrase.isEmpty())
                         ? pClass->aMarkingPhrase
                         : rLabel.aClassification;
    appendPart(aOut, aClassText);

    // Category groups, in policy tag-set order; the label carries the selected values
    // keyed by tag name. Each group is prefix + values(joined by the normalized
    // separator) + suffix; groups join to what precedes them with a single space.
    // Values that fed the ownership prefix (replacePolicy) are not rendered here.
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            const StanagCategory* pCat = nullptr;
            for (const auto& rCandidate : rLabel.aCategories)
            {
                if (rCandidate.aTagName == rTag.aName)
                {
                    pCat = &rCandidate;
                    break;
                }
            }
            if (!pCat)
                continue;

            const OUString aSeparator = normalizeSeparator(rTag.aMarkingSeparator);
            OUStringBuffer aValues;
            for (const auto& rValue : pCat->aValues)
            {
                const SpifTagCategory* pTagCat = findTagCategory(rTag, rValue);
                if (pTagCat && !pTagCat->aReplacePolicyPhrase.isEmpty())
                    continue; // fed the ownership prefix; not shown as a value
                const OUString aText = categoryDisplay(pTagCat, rValue);
                if (aText.isEmpty())
                    continue;
                if (!aValues.isEmpty())
                    aValues.append(aSeparator);
                aValues.append(aText);
            }
            if (aValues.isEmpty())
                continue;

            const OUString aGroup
                = rTag.aMarkingPrefix + aValues.makeStringAndClear() + rTag.aMarkingSuffix;
            appendPart(aOut, aGroup);
        }
    }

    return aOut.makeStringAndClear();
}

bool SpifPolicy::anySelectedTag(const OUString& rClassification,
                                const std::vector<bool>& rSelected,
                                bool SpifCategoryTag::*pFlag) const
{
    size_t nIdx = 0;
    for (const auto& rTagSet : aTagSets)
    {
        for (const auto& rTag : rTagSet.aTags)
        {
            for (const auto& rCategory : rTag.aCategories)
            {
                if (!rCategory.isSelectable(rClassification))
                    continue;
                if (rTag.*pFlag && nIdx < rSelected.size() && rSelected[nIdx])
                    return true;
                ++nIdx;
            }
        }
    }
    return false;
}

bool SpifPolicy::wantsWatermark(const OUString& rClassification,
                                const std::vector<bool>& rSelected) const
{
    return anySelectedTag(rClassification, rSelected, &SpifCategoryTag::bWatermark);
}

bool SpifPolicy::wantsDocumentStart(const OUString& rClassification,
                                    const std::vector<bool>& rSelected) const
{
    return anySelectedTag(rClassification, rSelected, &SpifCategoryTag::bDocumentStart);
}

bool SpifPolicy::wantsDocumentEnd(const OUString& rClassification,
                                  const std::vector<bool>& rSelected) const
{
    return anySelectedTag(rClassification, rSelected, &SpifCategoryTag::bDocumentEnd);
}

bool SpifPolicy::wantsPortionMarking(const OUString& rClassification,
                                     const std::vector<bool>& rSelected) const
{
    return anySelectedTag(rClassification, rSelected, &SpifCategoryTag::bPortionMarking);
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

    // Two files declaring the same policy OID are the same policy: the later load
    // replaces the earlier one rather than listing it twice. Load order is what gives
    // the per-user tree precedence over the system one (see loadProvisioned), and
    // replacing in place keeps the listing order stable while it does so.
    if (!aPolicy.aId.isEmpty())
    {
        auto it = std::find_if(aPolicies.begin(), aPolicies.end(),
                               [&aPolicy](const SpifPolicy& rExisting)
                               { return rExisting.aId == aPolicy.aId; });
        if (it != aPolicies.end())
        {
            *it = std::move(aPolicy);
            return true;
        }
    }

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

void SpifPolicySet::loadProvisioned()
{
    // Both provisioned sets, in increasing precedence (a repeated OID replaces, see
    // loadFile). The system set first: every *.xml the host's administrator published
    // org-wide, which the client made readable inside the sandbox (in COOL the kit
    // mounts the WOPI host's shared presets there; the group dir is <system>/spif).
    OUString aSystemDir = comphelper::COKit::getSystemConfigDir();
    if (!aSystemDir.isEmpty())
    {
        if (!aSystemDir.endsWith("/"))
            aSystemDir += "/";
        loadFromDir(aSystemDir + "spif");
    }

    // Then this user's own set, from the jail's user config dir under spif/.
    // $(userurl) resolves to that config root.
    loadFromDir(SvtPathOptions().SubstituteVariable(u"$(userurl)/spif"_ustr));

    if (!empty())
        return;

    loadFile(getDevPolicyUrl());
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

} // namespace svx::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
