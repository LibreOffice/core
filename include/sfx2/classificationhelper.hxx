/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_CLASSIFICATIONHELPER_HXX
#define INCLUDED_SFX2_CLASSIFICATIONHELPER_HXX

#include <memory>
#include <vector>

#include <com/sun/star/document/XDocumentProperties.hpp>

#include <rtl/ustring.hxx>
#include <sfx2/dllapi.h>
#include <sfx2/infobar.hxx>

class SfxObjectShell;
class SfxViewFrame;
namespace basegfx
{
class BColor;
}

/// Return code of SfxClassificationHelper::CheckPaste().
enum class SfxClassificationCheckPasteResult
{
    None = 1,
    TargetDocNotClassified = 2,
    DocClassificationTooLow = 3
};

/// Specifies a policy type, to be used with SetBACName(). Getters always use IntellectualProperty for now.
enum class SfxClassificationPolicyType
{
    ExportControl = 1,
    NationalSecurity = 2,
    IntellectualProperty = 3
};

/// Shared code to handle Business Authorization Identification and Labeling Scheme (BAILS) properties.
class SFX2_DLLPUBLIC SfxClassificationHelper
{
    class Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    /// Does the document have any BAILS properties?
    static bool IsClassified(const css::uno::Reference<css::document::XDocumentProperties>& xDocumentProperties);
    /// Checks if pasting from xSource to xDestination would leak information.
    static SfxClassificationCheckPasteResult CheckPaste(const css::uno::Reference<css::document::XDocumentProperties>& xSource,
            const css::uno::Reference<css::document::XDocumentProperties>& xDestination);
    /// Wrapper around CheckPaste(): informs the user if necessary and finds out if the paste can be continued or not.
    static bool ShowPasteInfo(SfxClassificationCheckPasteResult eResult);

    SfxClassificationHelper(const css::uno::Reference<css::document::XDocumentProperties>& xDocumentProperties, bool bUseLocalizedPolicy = true);
    ~SfxClassificationHelper();
    /// Get the currently selected category for eType.
    const OUString& GetBACName(SfxClassificationPolicyType eType);
    /// Return all possible valid category names, based on the policy.
    std::vector<OUString> GetBACNames();
    /// Return all possible valid category identifiers, based on the policy.
    std::vector<OUString> GetBACIdentifiers();
    /// Get the currently selected category abbreviation for eType. Returns full name if no abbreviation defined.
    const OUString& GetAbbreviatedBACName(const OUString& sFullName);
    /// Return all possible valid abbreviated category names, based on the policy.
    std::vector<OUString> GetAbbreviatedBACNames();
    /// Setting this sets all the other properties, based on the policy.
    void SetBACName(const OUString& rName, SfxClassificationPolicyType eType);
    /// Returns the class with the higher priority (based on sensitivity).
    OUString GetHigherClass(const OUString& first, const OUString& second);
    /// If GetImpactScale() and GetImpactLevel*() will return something meaningful.
    bool HasImpactLevel();
    InfoBarType GetImpactLevelType();
    /// Larger value means more confidential.
    sal_Int32 GetImpactLevel();
    /// Comparing the GetImpactLevel() result is only meaningful when the impact scale is the same.
    OUString GetImpactScale();
    OUString GetDocumentWatermark();
    /// The selected category has some content for the document header.
    bool HasDocumentHeader();
    /// The selected category has some content for the document footer.
    bool HasDocumentFooter();
    void UpdateInfobar(SfxViewFrame& rViewFrame);

    const std::vector<OUString> GetMarkings();
    const std::vector<OUString> GetIntellectualPropertyParts();
    const std::vector<OUString> GetIntellectualPropertyPartNumbers();

    /// Does a best-effort conversion of rType to SfxClassificationPolicyType.
    static SfxClassificationPolicyType stringToPolicyType(const OUString& rType);
    /// Returns the string representation of a SfxClassificationPolicyType element.
    static const OUString& policyTypeToString(SfxClassificationPolicyType eType);

    /// Brief text located at the top of each document's pages.
    static const OUString& PROP_DOCHEADER();
    /// Brief text located at the bottom of each document's pages.
    static const OUString& PROP_DOCFOOTER();
    /// Brief text formatted as a watermark on each document's page.
    static const OUString& PROP_DOCWATERMARK();
    /// Get the property prefix for the IntellectualProperty policy type.
    static const OUString& PROP_PREFIX_INTELLECTUALPROPERTY();

    static SfxClassificationPolicyType getPolicyType();
};

namespace sfx
{

/// Specifies the origin: either defined by the BAF policy or manual via. the advanced classification dialog
enum class ClassificationCreationOrigin
{
    NONE,
    BAF_POLICY,
    MANUAL
};

class ClassificationKeyCreator
{
private:
    const SfxClassificationPolicyType m_ePolicyType;
    const OUString m_sPolicy;
    sal_Int32 m_nTextNumber;
    sal_Int32 m_nIPPartNumber;
    sal_Int32 m_nMarkingNumber;

    OUString getPolicyKey() const
    {
        return m_sPolicy;
    }
public:
    ClassificationKeyCreator(SfxClassificationPolicyType ePolicyType)
        : m_ePolicyType(ePolicyType)
        , m_sPolicy(SfxClassificationHelper::policyTypeToString(m_ePolicyType))
        , m_nTextNumber(1)
        , m_nIPPartNumber(1)
        , m_nMarkingNumber(1)
    {}

    OUString makeTextKey() const
    {
        return getPolicyKey() + "Text";
    }

    OUString makeNumberedTextKey()
    {
        return makeTextKey() + ":n" + OUString::number(m_nTextNumber++);
    }

    bool isMarkingTextKey(OUString const & aKey) const
    {
        return aKey.startsWith(makeTextKey());
    }

    OUString makeCategoryNameKey() const
    {
        return getPolicyKey() + "BusinessAuthorizationCategory:Name";
    }

    bool isCategoryNameKey(OUString const & aKey) const
    {
        return aKey.startsWith(makeCategoryNameKey());
    }

    OUString makeCategoryIdentifierKey() const
    {
        return getPolicyKey() + "BusinessAuthorizationCategory:Identifier";
    }

    bool isCategoryIdentifierKey(OUString const & aKey) const
    {
        return aKey.startsWith(makeCategoryIdentifierKey());
    }

    OUString makeMarkingKey() const
    {
        return getPolicyKey() + "Marking";
    }

    OUString makeNumberedMarkingKey()
    {
        return makeMarkingKey() + ":n" + OUString::number(m_nMarkingNumber++);
    }

    bool isMarkingKey(OUString const & aKey) const
    {
        return aKey.startsWith(makeMarkingKey());
    }

    OUString makeIntellectualPropertyPartKey() const
    {
        return getPolicyKey() + "IntellectualPropertyPart";
    }

    OUString makeNumberedIntellectualPropertyPartKey()
    {
        return makeIntellectualPropertyPartKey() + ":n" + OUString::number(m_nIPPartNumber++);
    }

    bool isIntellectualPropertyPartKey(OUString const & aKey) const
    {
        return aKey.startsWith(makeIntellectualPropertyPartKey());
    }

    OUString makeFullTextualRepresentationKey() const
    {
        return getPolicyKey() + "FullTexturalRepresentation";
    }

    /// Classification creation origin key
    OUString makeCreationOriginKey() const
    {
        return getPolicyKey() + "CreationOrigin";
    }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
