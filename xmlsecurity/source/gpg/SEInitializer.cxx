/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/supportsservice.hxx>
#include <gpg/SEInitializer.hxx>
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <global.h>

namespace com::sun::star::uno
{
class XComponentContext;
}

using namespace css;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::crypto;

SEInitializerGpg::SEInitializerGpg()
{
    // Also init GpgME while we're at it
    GpgME::initializeLibrary();
}

SEInitializerGpg::~SEInitializerGpg() {}

/* XSEInitializer */
Reference<XXMLSecurityContext> SAL_CALL SEInitializerGpg::createSecurityContext(const OUString&)
{
    try
    {
        /* Build XML Security Context */
        Reference<XXMLSecurityContext> xSecCtx(new XMLSecurityContextGpg());

        Reference<XSecurityEnvironment> xSecEnv(new SecurityEnvironmentGpg());
        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex(n);
        return xSecCtx;
    }
    catch (const uno::Exception&)
    {
        return nullptr;
    }
}

void SAL_CALL SEInitializerGpg::freeSecurityContext(const uno::Reference<XXMLSecurityContext>&) {}

/* XServiceInfo */
sal_Bool SAL_CALL SEInitializerGpg::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL SEInitializerGpg::getSupportedServiceNames()
{
    return { u"com.sun.star.xml.crypto.GPGSEInitializer"_ustr };
}

OUString SAL_CALL SEInitializerGpg::getImplementationName()
{
    return u"com.sun.star.xml.security.SEInitializer_Gpg"_ustr;
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_security_SEInitializer_Gpg_get_implementation(
    uno::XComponentContext* /*pCtx*/, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new SEInitializerGpg());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
