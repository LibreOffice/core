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
extern "C" SAL_DLLPUBLIC_EXPORT void*  SAL_CALL OMozillaBootstrap_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
{
    if (!pMozillaBootstrap)
    {
        pMozillaBootstrap=new connectivity::mozab::MozillaBootstrap( _rxFactory );
        pMozillaBootstrap->Init();
        xMozillaBootstrap = pMozillaBootstrap;
    }
    return  pMozillaBootstrap;
}

MozillaBootstrap::MozillaBootstrap(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : OMozillaBootstrap_BASE(m_aMutex), m_xMSFactory( _rxFactory ),
      m_ProfileAccess(nullptr), m_ProfileManager(nullptr)
{
}

MozillaBootstrap::~MozillaBootstrap()
{
}

void MozillaBootstrap::Init()
{
    m_ProfileAccess = new ProfileAccess();
    bootupProfile(::com::sun::star::mozilla::MozillaProductType_Mozilla,OUString());
}


void MozillaBootstrap::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OMozillaBootstrap_BASE::disposing();
}

// static ServiceInfo

OUString MozillaBootstrap::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString(MOZAB_MozillaBootstrap_IMPL_NAME);
}

Sequence< OUString > MozillaBootstrap::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.mozilla.MozillaBootstrap
    Sequence< OUString > aSNS( 1 );
    aSNS[0] = "com.sun.star.mozilla.MozillaBootstrap";
    return aSNS;
}


OUString SAL_CALL MozillaBootstrap::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL MozillaBootstrap::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL MozillaBootstrap::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}


// XProfileDiscover
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileCount( ::com::sun::star::mozilla::MozillaProductType product) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->getProfileCount(product);
}
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< OUString >& list ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->getProfileList(product,list);
}
OUString SAL_CALL MozillaBootstrap::getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->getDefaultProfile(product);
}
OUString SAL_CALL MozillaBootstrap::getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->getProfilePath(product,profileName);
}
sal_Bool SAL_CALL MozillaBootstrap::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->isProfileLocked(product,profileName);
}
sal_Bool SAL_CALL MozillaBootstrap::getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_ProfileAccess->getProfileExists(product,profileName);
}

// XProfileManager
::sal_Int32 SAL_CALL MozillaBootstrap::bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)product; /* avoid warning about unused parameter */
    (void)profileName; /* avoid warning about unused parameter */
        return -1;
}
::sal_Int32 SAL_CALL MozillaBootstrap::shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return -1;
}
::com::sun::star::mozilla::MozillaProductType SAL_CALL MozillaBootstrap::getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return ::com::sun::star::mozilla::MozillaProductType_Default;
}
OUString SAL_CALL MozillaBootstrap::getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString();
}
sal_Bool SAL_CALL MozillaBootstrap::isCurrentProfileLocked(  ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return true;
}
OUString SAL_CALL MozillaBootstrap::setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)product; /* avoid warning about unused parameter */
    (void)profileName; /* avoid warning about unused parameter */
    return OUString();
}

// XProxyRunner
::sal_Int32 SAL_CALL MozillaBootstrap::Run( const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode ) throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    (void)aCode; /* avoid warning about unused parameter */
    return -1;
}

static Reference< XInterface > SAL_CALL createInstance( const Reference< XMultiServiceFactory >& rServiceManager )
{
        MozillaBootstrap * pBootstrap = static_cast<MozillaBootstrap*>(OMozillaBootstrap_CreateInstance(rServiceManager));
        return *pBootstrap;
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL mozbootstrap_component_getFactory(
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
                    Sequence< OUString > aSNS( 1 );
                    aSNS[0] = "com.sun.star.mozilla.MozillaBootstrap";

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
