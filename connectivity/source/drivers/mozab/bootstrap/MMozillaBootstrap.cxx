/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MMozillaBootstrap.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "MMozillaBootstrap.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::mozilla;
using namespace connectivity::mozab;
#include <MNSFolders.hxx>
#include "MNSProfileDiscover.hxx"
#include "MNSProfileManager.hxx"
#include "MNSRunnable.hxx"
#include <MNSInit.hxx>

static MozillaBootstrap *pMozillaBootstrap=NULL;
static Reference<XMozillaBootstrap> xMozillaBootstrap;
extern "C" void*  SAL_CALL OMozillaBootstrap_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
{
    if (!pMozillaBootstrap)
    {
        pMozillaBootstrap=new connectivity::mozab::MozillaBootstrap( _rxFactory );
        pMozillaBootstrap->Init();
        xMozillaBootstrap = pMozillaBootstrap;
    }
    return  pMozillaBootstrap;
}

// --------------------------------------------------------------------------------


// --------------------------------------------------------------------------------
MozillaBootstrap::MozillaBootstrap(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : OMozillaBootstrap_BASE(m_aMutex), m_xMSFactory( _rxFactory )
{
}
// -----------------------------------------------------------------------------
MozillaBootstrap::~MozillaBootstrap()
{
}
// -----------------------------------------------------------------------------
void MozillaBootstrap::Init()
{
    sal_Bool aProfileExists=sal_False;
    //This must be call before any mozilla code
    MNS_Init(aProfileExists);

    m_ProfileManager = new ProfileManager();
    m_ProfileAccess = new ProfileAccess();
    bootupProfile(::com::sun::star::mozilla::MozillaProductType_Mozilla,rtl::OUString());
}

// --------------------------------------------------------------------------------
void MozillaBootstrap::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    OMozillaBootstrap_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString MozillaBootstrap::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii(MOZAB_MozillaBootstrap_IMPL_NAME);
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > MozillaBootstrap::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    // which service is supported
    // for more information @see com.sun.star.mozilla.MozillaBootstrap
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap"));
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL MozillaBootstrap::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL MozillaBootstrap::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL MozillaBootstrap::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}


// XProfileDiscover
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileCount( ::com::sun::star::mozilla::MozillaProductType product) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->getProfileCount(product);
}
::sal_Int32 SAL_CALL MozillaBootstrap::getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< ::rtl::OUString >& list ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->getProfileList(product,list);
}
::rtl::OUString SAL_CALL MozillaBootstrap::getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->getDefaultProfile(product);
}
::rtl::OUString SAL_CALL MozillaBootstrap::getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->getProfilePath(product,profileName);
}
::sal_Bool SAL_CALL MozillaBootstrap::isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->isProfileLocked(product,profileName);
}
::sal_Bool SAL_CALL MozillaBootstrap::getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileAccess->getProfileExists(product,profileName);
}

// XProfileManager
::sal_Int32 SAL_CALL MozillaBootstrap::bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileManager->bootupProfile(product,profileName);
}
::sal_Int32 SAL_CALL MozillaBootstrap::shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileManager->shutdownProfile();
}
::com::sun::star::mozilla::MozillaProductType SAL_CALL MozillaBootstrap::getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileManager->getCurrentProduct();
}
::rtl::OUString SAL_CALL MozillaBootstrap::getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileManager->getCurrentProfile();
}
::sal_Bool SAL_CALL MozillaBootstrap::isCurrentProfileLocked(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return isProfileLocked(getCurrentProduct(),m_ProfileManager->getCurrentProfile());
}
::rtl::OUString SAL_CALL MozillaBootstrap::setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException)
{
    return m_ProfileManager->setCurrentProfile(product,profileName);
}

// XProxyRunner
::sal_Int32 SAL_CALL MozillaBootstrap::Run( const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode ) throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString profileName = aCode->getProfileName();
    ::rtl::OUString currProfileName = getCurrentProfile();
    ::com::sun::star::mozilla::MozillaProductType currProduct = getCurrentProduct();

     //if client provides a profileName, we will use it
    if (profileName.getLength()
             && ( aCode->getProductType() != currProduct  || !profileName.equals(currProfileName)) )
        setCurrentProfile(aCode->getProductType(),profileName);
       MNSRunnable xRunnable;

    return xRunnable.StartProxy(aCode);;
}

