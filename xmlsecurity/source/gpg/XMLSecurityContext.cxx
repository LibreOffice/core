/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSecurityContext.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::crypto;

XMLSecurityContextGpg::XMLSecurityContextGpg()
    : m_nDefaultEnvIndex(-1)
{
}

XMLSecurityContextGpg::~XMLSecurityContextGpg() {}

sal_Int32 SAL_CALL XMLSecurityContextGpg::addSecurityEnvironment(
    const Reference<XSecurityEnvironment>& aSecurityEnvironment)
{
    if (!aSecurityEnvironment.is())
        throw RuntimeException(u"Invalid SecurityEnvironment given!"_ustr);

    m_vSecurityEnvironments.push_back(aSecurityEnvironment);
    return m_vSecurityEnvironments.size() - 1;
}

sal_Int32 SAL_CALL XMLSecurityContextGpg::getSecurityEnvironmentNumber()
{
    return m_vSecurityEnvironments.size();
}

Reference<XSecurityEnvironment>
    SAL_CALL XMLSecurityContextGpg::getSecurityEnvironmentByIndex(sal_Int32 index)
{
    if (index < 0 || o3tl::make_unsigned(index) >= m_vSecurityEnvironments.size())
        throw RuntimeException(u"Invalid index"_ustr);

    return m_vSecurityEnvironments[index];
}

Reference<XSecurityEnvironment> SAL_CALL XMLSecurityContextGpg::getSecurityEnvironment()
{
    if (m_nDefaultEnvIndex < 0
        || o3tl::make_unsigned(m_nDefaultEnvIndex) >= m_vSecurityEnvironments.size())
        throw RuntimeException(u"Invalid index"_ustr);

    return getSecurityEnvironmentByIndex(m_nDefaultEnvIndex);
}

sal_Int32 SAL_CALL XMLSecurityContextGpg::getDefaultSecurityEnvironmentIndex()
{
    return m_nDefaultEnvIndex;
}

void SAL_CALL XMLSecurityContextGpg::setDefaultSecurityEnvironmentIndex(sal_Int32 nDefaultEnvIndex)
{
    m_nDefaultEnvIndex = nDefaultEnvIndex;
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContextGpg::getImplementationName()
{
    return u"com.sun.star.xml.security.gpg.XMLSecurityContext_GpgImpl"_ustr;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContextGpg::supportsService(const OUString& serviceName)
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence<OUString> SAL_CALL XMLSecurityContextGpg::getSupportedServiceNames()
{
    return { u"com.sun.star.xml.crypto.XMLSecurityContext"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
