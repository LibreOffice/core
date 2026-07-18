/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include "MMozillaBootstrap.hxx"
#include "MNSProfileDiscover.hxx"

using namespace com::sun::star::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::mozilla;
using namespace connectivity::mozab;

using ::com::sun::star::uno::Reference;
using ::cpo::uno::Sequence;

MozillaBootstrap::MozillaBootstrap()
    : OMozillaBootstrap_BASE(m_aMutex)
{
    m_ProfileAccess.reset(new ProfileAccess);
    bootupProfile(css::mozilla::MozillaProductType_Mozilla,OUString());
}

MozillaBootstrap::~MozillaBootstrap()
{
}

void MozillaBootstrap::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OMozillaBootstrap_BASE::disposing();
}

OUString MozillaBootstrap::getImplementationName(  )
{
    return u"com.sun.star.comp.mozilla.MozillaBootstrap"_ustr;
}

bool MozillaBootstrap::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > MozillaBootstrap::getSupportedServiceNames(  )
{
    // which service is supported
    // for more information @see com.sun.star.mozilla.MozillaBootstrap
    return { u"com.sun.star.mozilla.MozillaBootstrap"_ustr };
}


// XProfileDiscover
::sal_Int32 MozillaBootstrap::getProfileCount( css::mozilla::MozillaProductType product)
{
    return m_ProfileAccess->getProfileCount(product);
}
::sal_Int32 MozillaBootstrap::getProfileList( css::mozilla::MozillaProductType product, cpo::uno::Sequence< OUString >& list )
{
    return m_ProfileAccess->getProfileList(product,list);
}
OUString MozillaBootstrap::getDefaultProfile( css::mozilla::MozillaProductType product )
{
    return m_ProfileAccess->getDefaultProfile(product);
}
OUString MozillaBootstrap::getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName )
{
    return m_ProfileAccess->getProfilePath(product,profileName);
}
bool MozillaBootstrap::isProfileLocked( css::mozilla::MozillaProductType /*product*/, const OUString& /*profileName*/ )
{
    return true;
}
bool MozillaBootstrap::getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName )
{
    return m_ProfileAccess->getProfileExists(product,profileName);
}

// XProfileManager
::sal_Int32 MozillaBootstrap::bootupProfile( css::mozilla::MozillaProductType, const OUString& )
{
        return -1;
}
::sal_Int32 MozillaBootstrap::shutdownProfile(  )
{
    return -1;
}
css::mozilla::MozillaProductType MozillaBootstrap::getCurrentProduct(  )
{
    return css::mozilla::MozillaProductType_Default;
}
OUString MozillaBootstrap::getCurrentProfile(  )
{
    return OUString();
}
bool MozillaBootstrap::isCurrentProfileLocked(  )
{
    return true;
}
OUString MozillaBootstrap::setCurrentProfile( css::mozilla::MozillaProductType, const OUString& )
{
    return OUString();
}

// XProxyRunner
::sal_Int32 MozillaBootstrap::Run( const css::uno::Reference< css::mozilla::XCodeProxy >& )
{
    return -1;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_moz_MozillaBootstrap_get_implementation(
    css::uno::XComponentContext* , cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new connectivity::mozab::MozillaBootstrap());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
