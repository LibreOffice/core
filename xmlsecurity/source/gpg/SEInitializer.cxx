/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_gpgme.h>

#include <gpg/SEInitializer.hxx>
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <gpgme.h>
#include <context.h>

using namespace css;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::crypto;


SEInitializerGpg::SEInitializerGpg()
{
    // Also init GpgME while we're at it
    GpgME::initializeLibrary();
}

SEInitializerGpg::~SEInitializerGpg()
{
}

/* XSEInitializer */
Reference< XXMLSecurityContext > SAL_CALL SEInitializerGpg::createSecurityContext( const OUString& )
{
    try
    {
        /* Build XML Security Context */
        Reference< XXMLSecurityContext > xSecCtx(new XMLSecurityContextGpg());

        Reference< XSecurityEnvironment > xSecEnv(new SecurityEnvironmentGpg());
        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex( n );
        return xSecCtx;
    }
    catch( const uno::Exception& )
    {
        return nullptr;
    }
}

void SAL_CALL SEInitializerGpg::freeSecurityContext( const uno::Reference< XXMLSecurityContext >& )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
