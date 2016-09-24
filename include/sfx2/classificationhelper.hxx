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

    SfxClassificationHelper(const css::uno::Reference<css::document::XDocumentProperties>& xDocumentProperties);
    ~SfxClassificationHelper();
    /// Get the currently selected category for eType.
    const OUString& GetBACName(SfxClassificationPolicyType eType);
    /// Return all possible valid category names, based on the policy.
    std::vector<OUString> GetBACNames();
    /// Setting this sets all the other properties, based on the policy.
    void SetBACName(const OUString& rName, SfxClassificationPolicyType eType);
    /// If GetImpactScale() and GetImpactLevel*() will return something meaningful.
    bool HasImpactLevel();
    basegfx::BColor GetImpactLevelColor();
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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
