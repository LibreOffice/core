/*************************************************************************
 *
 *  $RCSfile: localsinglestratum.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 13:24:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "localsinglestratum.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#include "localfilehelper.hxx"
#endif

#ifndef CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
#include "localfilelayer.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

#ifndef _CONFIGMGR_OSLSTREAM_HXX_
#include "oslstream.hxx"
#endif // _CONFIGMGR_OSLSTREAM_HXX_

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif // CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#include <memory>

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalSingleStratumBase::LocalSingleStratumBase(
        const uno::Reference<uno::XComponentContext>& xContext)
        : SingleBackendBase(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY) {
}
//------------------------------------------------------------------------------

LocalSingleStratumBase::~LocalSingleStratumBase(void) {}

//------------------------------------------------------------------------------
void SAL_CALL LocalSingleStratumBase::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backend::CannotConnectException,
           backend::BackendSetupException)
{



    if (aParameters.getLength() == 0) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to LocalSingleStratum")),
                *this, 0) ;
    }


    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i) {
        if (aParameters [i] >>= mStrataDataUrl )
        { break ; }

    }

    //OSL_TRACE ( " Url is %s " ,rtl::OUStringToOString(mStrataDataUrl, RTL_TEXTENCODING_ASCII_US).getStr());
    //validate DefaultDataUrls
    bool bOptional = checkOptionalArg(mStrataDataUrl);
    validateFileURL(mStrataDataUrl, *this);
    //NormalizeURL
    implEnsureAbsoluteURL(mStrataDataUrl);
    normalizeURL(mStrataDataUrl,*this, true);

    if(FileHelper::fileExists(mStrataDataUrl))
    {
        checkIfDirectory(mStrataDataUrl, *this);
    }
}

//------------------------------------------------------------------------------

sal_Bool LocalSingleStratumBase::isMoreRecent(const rtl::OUString& aFileUrl,
                                              const rtl::OUString& aTimestamp) {
    rtl::OUString layerUrl ;
    rtl::OUString subLayerUrl ;

    getLayerDirectories(layerUrl, subLayerUrl) ;

    return layerUrl.getLength() == 0 ||
        BasicLocalFileLayer::getTimestamp(layerUrl + aFileUrl).compareTo( aTimestamp) != 0;
}
//------------------------------------------------------------------------------

static const rtl::OUString kDataSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcu")) ;
static const rtl::OUString kDataSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/data")) ;
static const rtl::OUString kLocalisedDataSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/res")) ;


uno::Reference<backend::XLayer> SAL_CALL
    LocalSingleStratumBase::getLayer( const rtl::OUString& aComponent, const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{

    if (aComponent.getLength() == 0){
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "LocalStratum:getLayer - no component specified")),
                *this, 0) ;
    }
    rtl::OUString const componentSubPath = componentToPath(aComponent) + kDataSuffix ;
    if (!isMoreRecent(componentSubPath, aTimestamp)) { return NULL ; }

    return createReadonlyFileLayer(componentSubPath);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

OUString SAL_CALL LocalSingleStratumBase::getOwnerEntity()
    throw (uno::RuntimeException)
{
    return mStrataDataUrl ;
}
//------------------------------------------------------------------------------

OUString SAL_CALL LocalSingleStratumBase::getAdminEntity()
    throw (uno::RuntimeException)
{
    return OUString();
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleStratumBase::supportsEntity( const OUString& aEntity )
    throw (backend::BackendAccessException, uno::RuntimeException)
{
    if(mStrataDataUrl.getLength() == 0)
    {
        return false;
    }
    if (aEntity.getLength() == 0)
    {
      return false;
    }
    return isEqualEntity(mStrataDataUrl,aEntity);
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleStratumBase::isEqualEntity(const OUString& aEntity, const OUString& aOtherEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (aEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 1);
    }
    if (aOtherEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 2);
    }
    OUString aNormalizedEntity(aEntity);
    normalizeURL(aNormalizedEntity,*this);

    OUString aNormalizedOther(aOtherEntity);
    normalizeURL(aNormalizedOther,*this);

    return aNormalizedEntity == aNormalizedOther;
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
    rtl::OUString const componentSubPath = componentToPath(aComponent) + kDataSuffix ;
    return createUpdatableFileLayer( componentSubPath) ;
}
//------------------------------------------------------------------------------
static
void failReadonly(backend::XSingleLayerStratum * pContext,const rtl::OUString & aBaseUrl)
{
    rtl::OUStringBuffer aMessage;
    aMessage.appendAscii("Configurations - ")
            .appendAscii("Cannot get update access to layer: ")
            .appendAscii("Local file-based stratum at ")
            .append(aBaseUrl)
            .appendAscii(" is readonly.");
    throw lang::NoSupportException(aMessage.makeStringAndClear(),pContext);
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer> SAL_CALL
    LocalReadonlyStratum::getUpdatableLayer(const rtl::OUString& aComponent)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    failReadonly(this,getBaseUrl());
    return 0;
}
uno::Reference<backend::XUpdatableLayer> SAL_CALL
    LocalResourceStratum::getUpdatableLayer(const rtl::OUString& aComponent)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    failReadonly(this,getBaseUrl());
    return 0;
}
//------------------------------------------------------------------------------
static
inline
void impl_getLayerDataDirectory(rtl::OUString const & aLayerBaseUrl,
                                 rtl::OUString& aMainLayerUrl)
{
    aMainLayerUrl   = aLayerBaseUrl + kDataSubPath ;
}
//------------------------------------------------------------------------------
static
inline
void impl_getLayerResDirectory(rtl::OUString const & aLayerBaseUrl,
                                 rtl::OUString& aSubLayerUrl)
{
    aSubLayerUrl    = aLayerBaseUrl + kLocalisedDataSubPath ;
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

void LocalResourceStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
   aLayerUrl = OUString();
   impl_getLayerResDirectory(getBaseUrl(),aSubLayerUrl);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uno::Reference<backend::XLayer>
    LocalSingleStratumBase::createReadonlyFileLayer(const rtl::OUString& aComponent)
     throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createReadonlyLocalFileLayer(mFactory, layerPath, aComponent, subLayerPath) ;
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer>
    LocalSingleStratumBase::createUpdatableFileLayer(const rtl::OUString& aComponent)
         throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createUpdatableLocalFileLayer(mFactory, layerPath, aComponent, subLayerPath) ;
}
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

rtl::OUString SAL_CALL LocalSingleStratumBase::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleStratumBase::supportsService(
                                        const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(getServiceInfoData()).supportsService(aServiceName);
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL LocalSingleStratumBase::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getSupportedServiceNames() ;
}

// ---------------------------------------------------------------------------------------

} } // configmgr.localsinglestratum
