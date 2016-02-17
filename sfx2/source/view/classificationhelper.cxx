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

OUString SfxClassificationHelper::GetImpactLevel()
{
    std::map<OUString, OUString>::iterator it = m_pImpl->m_aLabels.find("urn:bails:IntellectualProperty:Impact:Level:Confidentiality");
    if (it != m_pImpl->m_aLabels.end())
        return it->second;

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
