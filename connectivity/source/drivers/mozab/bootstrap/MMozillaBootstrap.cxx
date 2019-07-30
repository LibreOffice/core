/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>
#include "MMozillaBootstrap.hxx"
#include "MNSFolders.hxx"
#include "MNSProfileDiscover.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::mozilla;
using namespace connectivity::mozab;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;

static MozillaBootstrap *pMozillaBootstrap=nullptr;
static Reference<XMozillaBootstrap> xMozillaBootstrap;
extern "C" SAL_DLLPUBLIC_EXPORT void* OMozillaBootstrap_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& )
{
    if (!pMozillaBootstrap)
    {
        pMozillaBootstrap=new connectivity::mozab::MozillaBootstrap;
        pMozillaBootstrap->Init();
        xMozillaBootstrap = pMozillaBootstrap;
    }
    return  pMozillaBootstrap;
}

MozillaBootstrap::MozillaBootstrap()
    : OMozillaBootstrap_BASE(m_aMutex)
{
}

MozillaBootstrap::~MozillaBootstrap()
{
}

void MozillaBootstrap::Init()
{
    m_ProfileAccess.reset(new ProfileAccess);
    bootupProfile(css::mozilla::MozillaProductType_Mozilla,OUString());
}

void MozillaBootstrap::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OMozillaBootstrap_BASE::disposing();
}

OUString SAL_CALL MozillaBootstrap::getImplementationName(  )
{
    return MOZAB_MozillaBootstrap_IMPL_NAME;
}

sal_Bool SAL_CALL MozillaBootstrap::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL MozillaBootstrap::getSupportedServiceNames(  )
{
    // which service is supported
    // for more information @see com.sun.star.mozilla.MozillaBootstrap
    return { "com.sun.star.mozilla.MozillaBootstrap" };
}


// XProfileDiscover
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileCount( css::mozilla::MozillaProductType product)
{
    return m_ProfileAccess->getProfileCount(product);
}
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileList( css::mozilla::MozillaProductType product, css::uno::Sequence< OUString >& list )
{
    return m_ProfileAccess->getProfileList(product,list);
}
OUString SAL_CALL MozillaBootstrap::getDefaultProfile( css::mozilla::MozillaProductType product )
{
    return m_ProfileAccess->getDefaultProfile(product);
}
OUString SAL_CALL MozillaBootstrap::getProfilePath( css::mozilla::MozillaProductType product, const OUString& profileName )
{
    return m_ProfileAccess->getProfilePath(product,profileName);
}
sal_Bool SAL_CALL MozillaBootstrap::isProfileLocked( css::mozilla::MozillaProductType /*product*/, const OUString& /*profileName*/ )
{
    return true;
}
sal_Bool SAL_CALL MozillaBootstrap::getProfileExists( css::mozilla::MozillaProductType product, const OUString& profileName )
{
    return m_ProfileAccess->getProfileExists(product,profileName);
}

// XProfileManager
::sal_Int32 SAL_CALL MozillaBootstrap::bootupProfile( css::mozilla::MozillaProductType, const OUString& )
{
        return -1;
}
::sal_Int32 SAL_CALL MozillaBootstrap::shutdownProfile(  )
{
    return -1;
}
css::mozilla::MozillaProductType SAL_CALL MozillaBootstrap::getCurrentProduct(  )
{
    return css::mozilla::MozillaProductType_Default;
}
OUString SAL_CALL MozillaBootstrap::getCurrentProfile(  )
{
    return OUString();
}
sal_Bool SAL_CALL MozillaBootstrap::isCurrentProfileLocked(  )
{
    return true;
}
OUString SAL_CALL MozillaBootstrap::setCurrentProfile( css::mozilla::MozillaProductType, const OUString& )
{
    return OUString();
}

// XProxyRunner
::sal_Int32 SAL_CALL MozillaBootstrap::Run( const css::uno::Reference< css::mozilla::XCodeProxy >& )
{
    return -1;
}

static Reference< XInterface > createInstance( const Reference< XMultiServiceFactory >& rServiceManager )
{
        MozillaBootstrap * pBootstrap = static_cast<MozillaBootstrap*>(OMozillaBootstrap_CreateInstance(rServiceManager));
        return *pBootstrap;
}

extern "C" SAL_DLLPUBLIC_EXPORT void* mozbootstrap_component_getFactory(
                                        const sal_Char* pImplementationName,
                                        void* pServiceManager,
                                        void* /*pRegistryKey*/)
{
        void* pRet = nullptr;

        if (pServiceManager)
        {
                OUString aImplName( OUString::createFromAscii( pImplementationName ) );
                Reference< XSingleServiceFactory > xFactory;
                if ( aImplName == "com.sun.star.comp.mozilla.MozillaBootstrap" )
                {
                    Sequence<OUString> aSNS { "com.sun.star.mozilla.MozillaBootstrap" };

                    xFactory = ::cppu::createSingleFactory(
                        static_cast< XMultiServiceFactory* > ( pServiceManager),
                        aImplName, createInstance, aSNS );
                }
                if ( xFactory.is() )
                {
                    xFactory->acquire();
                    pRet = xFactory.get();
                }
        }

        return pRet;
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
