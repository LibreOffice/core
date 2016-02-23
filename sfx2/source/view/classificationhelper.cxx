/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/classificationhelper.hxx>

#include <map>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <sfx2/objsh.hxx>
#include <o3tl/make_unique.hxx>

using namespace com::sun::star;

/// Implementation details of SfxClassificationHelper.
struct SfxClassificationHelper::Impl
{
    std::map<OUString, OUString> m_aLabels;
};

bool SfxClassificationHelper::IsClassified(SfxObjectShell& rObjectShell)
{
    uno::Reference<document::XDocumentProperties> xDocumentProperties = rObjectShell.getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return false;

    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties = xPropertySet->getPropertySetInfo()->getProperties();
    for (const beans::Property& rProperty : aProperties)
    {
        if (rProperty.Name.startsWith("urn:bails:"))
            return true;
    }

    return false;
}

SfxClassificationHelper::SfxClassificationHelper(SfxObjectShell& rObjectShell)
    : m_pImpl(o3tl::make_unique<Impl>())
{
    uno::Reference<document::XDocumentProperties> xDocumentProperties = rObjectShell.getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    if (!xPropertyContainer.is())
        return;

    uno::Reference<beans::XPropertySet> xPropertySet(xPropertyContainer, uno::UNO_QUERY);
    uno::Sequence<beans::Property> aProperties = xPropertySet->getPropertySetInfo()->getProperties();
    for (const beans::Property& rProperty : aProperties)
    {
        if (!rProperty.Name.startsWith("urn:bails:"))
            continue;

        uno::Any aAny = xPropertySet->getPropertyValue(rProperty.Name);
        OUString aValue;
        if (aAny >>= aValue)
            m_pImpl->m_aLabels[rProperty.Name] = aValue;
    }
}

SfxClassificationHelper::~SfxClassificationHelper()
{
}

OUString SfxClassificationHelper::GetBACName()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:BusinessAuthorizationCategory:Name");
    if (it != m_pImpl->m_aLabels.end())
        return it->second;

    return OUString();
}

bool SfxClassificationHelper::HasImpactLevel()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Scale");
    if (it == m_pImpl->m_aLabels.end())
        return false;

    it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    if (it == m_pImpl->m_aLabels.end())
        return false;

    return true;
}

basegfx::BColor SfxClassificationHelper::GetImpactLevelColor()
{
    basegfx::BColor aRet;

    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Scale");
    if (it == m_pImpl->m_aLabels.end())
        return aRet;
    OUString aScale = it->second;

    it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    if (it == m_pImpl->m_aLabels.end())
        return aRet;
    OUString aLevel = it->second;

    // The spec defines two valid scale values: FIPS-199 and UK-Cabinet.
    if (aScale == "UK-Cabinet")
    {
        static std::map<OUString, basegfx::BColor> aColors;
        if (aColors.empty())
        {
            // Green -> brown -> orange -> red.
            aColors["0"] = basegfx::BColor(0.0, 0.5, 0.0);
            aColors["1"] = basegfx::BColor(0.5, 0.5, 0.0);
            aColors["2"] = basegfx::BColor(1.0, 0.5, 0.0);
            aColors["3"] = basegfx::BColor(0.5, 0.0, 0.0);
        }
        std::map<OUString, basegfx::BColor>::iterator itColor = aColors.find(aLevel);
        if (itColor == aColors.end())
            return aRet;
        aRet = itColor->second;
    }
    else if (aScale == "FIPS-199")
    {
        static std::map<OUString, basegfx::BColor> aColors;
        if (aColors.empty())
        {
            // Green -> orange -> red.
            aColors["Low"] = basegfx::BColor(0.0, 0.5, 0.0);
            aColors["Moderate"] = basegfx::BColor(1.0, 0.5, 0.0);
            aColors["High"] = basegfx::BColor(0.5, 0.0, 0.0);
        }
        std::map<OUString, basegfx::BColor>::iterator itColor = aColors.find(aLevel);
        if (itColor == aColors.end())
            return aRet;
        aRet = itColor->second;
    }

    return aRet;
}

OUString SfxClassificationHelper::GetDocumentWatermark()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Marking:document-watermark");
    if (it != m_pImpl->m_aLabels.end())
        return it->second;

    return OUString();
}

void SfxClassificationHelper::SetBACName(const OUString& /*rName*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
