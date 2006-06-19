/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localsinglestratum.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:25:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "localsinglestratum.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#include "localfilehelper.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif // CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif

namespace configmgr { namespace localbe {

//==============================================================================

static inline rtl::OUString getDataSubPath()
{ return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/data")); }

static inline rtl::OUString getLocalisedDataSubPath()
{ return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/res")); }
//------------------------------------------------------------------------------

LocalSingleStratumBase::LocalSingleStratumBase(const uno::Reference<uno::XComponentContext>& xContext)
: SingleStratumImplBase(xContext)
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
    aSubLayerUrl = OUString();
}
//------------------------------------------------------------------------------

void LocalReadonlyStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
    impl_getLayerDataDirectory(getBaseUrl(),aLayerUrl);
    aSubLayerUrl = OUString();
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
   aLayerUrl = OUString();
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

static AsciiServiceName kServiceNames [] = { kLocalService, 0, kBackendService, 0 } ;
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
instantiateLocalLegacyStratum(const CreationContext& xContext) {
    return *new LocalSingleStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalDataStratum(const CreationContext& xContext) {
    return *new LocalDataStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalReadonlyStratum(const CreationContext& xContext) {
    return *new LocalReadonlyStratum(xContext) ;
}

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalResourceStratum(const CreationContext& xContext) {
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
