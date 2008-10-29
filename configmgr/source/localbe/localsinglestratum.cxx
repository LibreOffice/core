/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localsinglestratum.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_configmgr.hxx"

#include "localsinglestratum.hxx"
#include "localfilehelper.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_
#include "serviceinfohelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>

namespace configmgr { namespace localbe {

//==============================================================================

static inline rtl::OUString getDataSubPath()
{ return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/data")); }

static inline rtl::OUString getLocalisedDataSubPath()
{ return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/res")); }
//------------------------------------------------------------------------------

LocalSingleStratumBase::LocalSingleStratumBase(const uno::Reference<uno::XComponentContext>& xContext)
: cppu::ImplInheritanceHelper1< LocalStratumBase, backend::XSingleLayerStratum >(xContext)
{
}
//------------------------------------------------------------------------------

LocalSingleStratumBase::~LocalSingleStratumBase() {}

//------------------------------------------------------------------------------
uno::Reference<backend::XLayer> SAL_CALL
    LocalSingleStratumBase::getLayer( const rtl::OUString& aComponent, const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    return LocalStratumBase::getLayer(aComponent,aTimestamp);
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer> SAL_CALL
    LocalSingleStratumBase::getUpdatableLayer(const rtl::OUString& aComponent)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    if (aComponent.getLength() == 0){
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "LocalSingleStratum:getLayer - no component specified")),
                *this, 0) ;
    }
    rtl::OUString const componentSubPath = componentToPath(aComponent) + getDataSuffix();
    return createUpdatableFileLayer( componentSubPath) ;
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer> SAL_CALL
    LocalReadonlyStratum::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    failReadonly();
    return 0;
}
uno::Reference<backend::XUpdatableLayer> SAL_CALL
    LocalResourceStratum::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    failReadonly();
    return 0;
}
//------------------------------------------------------------------------------
static
inline
void impl_getLayerDataDirectory(rtl::OUString const & aLayerBaseUrl,
                                 rtl::OUString& aMainLayerUrl)
{
    aMainLayerUrl   = aLayerBaseUrl + getDataSubPath() ;
}
//------------------------------------------------------------------------------
static
inline
void impl_getLayerResDirectory(rtl::OUString const & aLayerBaseUrl,
                                 rtl::OUString& aSubLayerUrl)
{
    aSubLayerUrl    = aLayerBaseUrl + getLocalisedDataSubPath() ;
}
//------------------------------------------------------------------------------

void LocalSingleStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
    impl_getLayerDataDirectory(getBaseUrl(),aLayerUrl);
    impl_getLayerResDirectory(getBaseUrl(),aSubLayerUrl);
}
//------------------------------------------------------------------------------

void LocalDataStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
    impl_getLayerDataDirectory(getBaseUrl(),aLayerUrl);
    aSubLayerUrl = rtl::OUString();
}
//------------------------------------------------------------------------------

void LocalReadonlyStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
    impl_getLayerDataDirectory(getBaseUrl(),aLayerUrl);
    aSubLayerUrl = rtl::OUString();
}
//------------------------------------------------------------------------------

void LocalResourceStratum::adjustBaseURL(rtl::OUString& aBaseUrl)
{
    impl_getLayerResDirectory(aBaseUrl,aBaseUrl);
}
//------------------------------------------------------------------------------

void LocalResourceStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
   aLayerUrl = rtl::OUString();
   aSubLayerUrl = getBaseUrl();
   // impl_getLayerResDirectory(getBaseUrl(),aSubLayerUrl);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static const sal_Char * const kLegacyStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalSingleStratum" ;
static const sal_Char * const kDataStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalStratum" ;
static const sal_Char * const kReadonlyStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalReadonlyStratum" ;
static const sal_Char * const kResourceStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalResourceStratum" ;
static const sal_Char * const kBackendService =
                "com.sun.star.configuration.backend.SingleStratum" ;
static const sal_Char * const kLocalService =
                "com.sun.star.configuration.backend.LocalSingleStratum" ;

static sal_Char const * kServiceNames [] = { kLocalService, 0, kBackendService, 0 } ;
static const ServiceImplementationInfo kLegacyStratumServiceInfo   = { kLegacyStratumImplementation  , kServiceNames, kServiceNames + 2 } ;
static const ServiceImplementationInfo kDataStratumServiceInfo     = { kDataStratumImplementation    , kServiceNames, kServiceNames + 2 } ;
static const ServiceImplementationInfo kReadonlyStratumServiceInfo = { kReadonlyStratumImplementation, kServiceNames, kServiceNames + 2 } ;
static const ServiceImplementationInfo kResourceStratumServiceInfo = { kResourceStratumImplementation, kServiceNames, kServiceNames + 2 } ;

const ServiceRegistrationInfo *getLocalLegacyStratumServiceInfo()
{ return getRegistrationInfo(&kLegacyStratumServiceInfo) ; }

const ServiceRegistrationInfo *getLocalDataStratumServiceInfo()
{ return getRegistrationInfo(&kDataStratumServiceInfo) ; }

const ServiceRegistrationInfo *getLocalReadonlyStratumServiceInfo()
{ return getRegistrationInfo(&kReadonlyStratumServiceInfo) ; }

const ServiceRegistrationInfo *getLocalResourceStratumServiceInfo()
{ return getRegistrationInfo(&kResourceStratumServiceInfo) ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalLegacyStratum(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalSingleStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalDataStratum(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalDataStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalReadonlyStratum(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalReadonlyStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalResourceStratum(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalResourceStratum(xContext) ;
}
//------------------------------------------------------------------------------

const ServiceImplementationInfo * LocalSingleStratum::getServiceInfoData() const
{
    return &kLegacyStratumServiceInfo;
}
const ServiceImplementationInfo * LocalDataStratum::getServiceInfoData() const
{
    return &kDataStratumServiceInfo;
}
const ServiceImplementationInfo * LocalReadonlyStratum::getServiceInfoData() const
{
    return &kReadonlyStratumServiceInfo;
}
const ServiceImplementationInfo * LocalResourceStratum::getServiceInfoData() const
{
    return &kResourceStratumServiceInfo;
}
//------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

} } // configmgr.localsinglestratum
