/*************************************************************************
 *
 *  $RCSfile: localsinglebackend.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-07-03 13:41:08 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#include "localsinglebackend.hxx"
#endif // CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_

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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_
#include <com/sun/star/uno/XCurrentContext.hpp>
#endif // _COM_SUN_STAR_UNO_XCURRENTCONTEXT_HPP_

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalSingleBackend::LocalSingleBackend(
        const uno::Reference<lang::XMultiServiceFactory>& aFactory)
: SingleBackendBase(mMutex), mFactory(aFactory) {
}
//------------------------------------------------------------------------------

LocalSingleBackend::~LocalSingleBackend(void) {}
//------------------------------------------------------------------------------

static const rtl::OUString kMetaConfPrefix(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.bootstrap.")) ;
static const rtl::OUString kSharedDataUrl(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SharedDataUrl"))) ;
static const rtl::OUString kUserDataUrl(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserDataUrl"))) ;
static const rtl::OUString kUserName(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserName"))) ;

void SAL_CALL LocalSingleBackend::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception)
{
    if (aParameters.getLength() == 0) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to SingleBackend")),
                *this, 0) ;
    }
    uno::Reference<uno::XCurrentContext> context ;

    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i) {
        if (aParameters [i] >>= context) { break ; }
    }
    context->getValueByName(kSharedDataUrl) >>= mSharedDataUrl ;
    context->getValueByName(kUserDataUrl) >>= mUserDataUrl ;
    context->getValueByName(kUserName) >>= mOwnId ;
}
//------------------------------------------------------------------------------

static sal_Unicode kComponentSeparator = '.' ;
static sal_Unicode kPathSeparator = '/' ;

static rtl::OUString componentToPath(const rtl::OUString& aComponent) {
    rtl::OUStringBuffer retCode ;

    retCode.append(kPathSeparator) ;
    retCode.append(aComponent.replace(kComponentSeparator, kPathSeparator)) ;
    return retCode.makeStringAndClear() ;
}
//------------------------------------------------------------------------------

static sal_Unicode kDefaultLayerMark = 'd' ;
static sal_Unicode kUserLayerMark = 'u' ;

static rtl::OUString urlToLayerId(const rtl::OUString& aFileUrl,
                                  sal_Bool bIsDefault) {
    rtl::OUStringBuffer retCode ;

    retCode.append(bIsDefault ? kDefaultLayerMark : kUserLayerMark) ;
    retCode.append(aFileUrl) ;
    return retCode.makeStringAndClear() ;
}

static void layerIdToUrl(const rtl::OUString& aLayerId,
                         rtl::OUString& aFileUrl,
                         sal_Bool& bIsDefault) {
    bIsDefault = ((static_cast<const sal_Unicode *>(aLayerId)) [0] ==
                                                            kDefaultLayerMark) ;
    aFileUrl = aLayerId.copy(1) ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kDefaultsSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/data")) ;
static const rtl::OUString kLocalisedDefaultsSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/res")) ;
static const rtl::OUString kDataSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcu")) ;

uno::Sequence<rtl::OUString> SAL_CALL LocalSingleBackend::listLayerIds(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    if (!aEntity.equals(mOwnId)) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Can only access user data")),
                *this, 1) ;
    }
    uno::Sequence<rtl::OUString> retCode(2) ;
    rtl::OUString componentSubPath = componentToPath(aComponent) + kDataSuffix ;
    // First, the defaults...
    retCode [0] = urlToLayerId(componentSubPath, sal_True) ;
    // Then the user delta.
    retCode [1] = urlToLayerId(componentSubPath, sal_False) ;
    return retCode ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getUpdateLayerId(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    if (!aEntity.equals(mOwnId)) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Can only access user data")),
                *this, 1) ;
    }
    return urlToLayerId(componentToPath(aComponent) + kDataSuffix, sal_False) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getOwnId(void)
    throw (uno::RuntimeException)
{
    return mOwnId ;
}
//------------------------------------------------------------------------------

/**
  Tests whether a file defined by its URL is more recent than a
  date provided as a timestamp.
  The timestamp is formatted YYYYMMDDhhmmssZ.

  @param aFileUrl   URL of the file to test
  @param aTimestamp timestamp to test against
  @return   sal_True if the file is more recent, sal_False otherwise
  */
static sal_Bool isMoreRecent(const rtl::OUString& aFileUrl,
                             const rtl::OUString& aTimestamp) {
    return (LocalFileLayer::getTimestamp(aFileUrl).compareTo(aTimestamp) > 0) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL LocalSingleBackend::getLayer(
        const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString fileUrl ;
    sal_Bool bIsDefault = sal_False ;

    layerIdToUrl(aLayerId, fileUrl, bIsDefault) ;
    if (!isMoreRecent(fileUrl, aTimestamp)) { return NULL ; }
    uno::Reference<backend::XCompositeLayer> layer ;

    if (bIsDefault) {
        layer = new LocalFileLayer(mFactory,
                mSharedDataUrl + kDefaultsSubPath,
                fileUrl,
                mSharedDataUrl + kLocalisedDefaultsSubPath) ;
    }
    else {
        layer = new LocalFileLayer(mFactory,
                mUserDataUrl, fileUrl, rtl::OUString()) ;
    }
    return uno::Reference<backend::XLayer>::query(layer) ;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
LocalSingleBackend::getLayers(const uno::Sequence<rtl::OUString>& aLayerIds,
                              const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    uno::Sequence<uno::Reference<backend::XLayer> >
                                                retCode(aLayerIds.getLength()) ;

    for (sal_Int32 i = 0 ; i < aLayerIds.getLength() ; ++ i) {
        retCode [i] = getLayer(aLayerIds [i], aTimestamp) ;
    }
    return retCode ;
}
//------------------------------------------------------------------------------

uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
LocalSingleBackend::getMultipleLayers(
                        const uno::Sequence<rtl::OUString>& aLayerIds,
                        const uno::Sequence<rtl::OUString>& aTimestamps)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    if (aLayerIds.getLength() != aTimestamps.getLength()) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Not enough or too many timestamps")),
                *this, 0) ;
    }
    uno::Sequence<uno::Reference<backend::XLayer> >
                                                retCode(aLayerIds.getLength()) ;

    for (sal_Int32 i = 0 ; i < aLayerIds.getLength() ; ++ i) {
        retCode [i] = getLayer(aLayerIds [i], aTimestamps [i]) ;
    }
    return retCode ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> SAL_CALL
LocalSingleBackend::getUpdatableLayer(const rtl::OUString& aLayerId)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString fileUrl ;
    sal_Bool bIsDefault = sal_False ;

    layerIdToUrl(aLayerId, fileUrl, bIsDefault) ;
    if (bIsDefault) {
        throw backend::BackendAccessException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot update default layer")),
                *this) ;
    }
    return new LocalFileLayer(mFactory, mUserDataUrl,
                              fileUrl, rtl::OUString()) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayerHandler> SAL_CALL
LocalSingleBackend::getWriteHandler(const rtl::OUString& aLayerId)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString fileUrl ;
    sal_Bool bIsDefault = sal_False ;

    layerIdToUrl(aLayerId, fileUrl, bIsDefault) ;
    if (bIsDefault) {
        throw backend::BackendAccessException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot update default layer")),
                *this) ;
    }
    LocalFileLayer layer(mFactory, mUserDataUrl, fileUrl, rtl::OUString()) ;

    return layer.getLayerWriter() ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kSchemaSubPath(
                                    RTL_CONSTASCII_USTRINGPARAM("/schema")) ;
static const rtl::OUString kSchemaSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcs")) ;
static const rtl::OUString kXMLSchemaParser(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.xml.SchemaParser")) ;

uno::Reference<backend::XSchema> SAL_CALL LocalSingleBackend::getSchema(
                                              const rtl::OUString& aComponent)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString subPath = componentToPath(aComponent) ;
    rtl::OUStringBuffer schemaUrl(mSharedDataUrl) ;

    schemaUrl.append(kSchemaSubPath).append(subPath).append(kSchemaSuffix) ;
    osl::File *schemaFile = new osl::File(schemaUrl.makeStringAndClear()) ;
    osl::FileBase::RC errorCode = schemaFile->open(OpenFlag_Read) ;

    if (errorCode != osl_File_E_None) {
        throw backend::BackendAccessException(
                FileHelper::createOSLErrorString(errorCode), *this) ;
    }
    uno::Sequence<uno::Any> arguments(1) ;
    uno::Reference<io::XInputStream> stream =
                            new OSLInputStreamWrapper(schemaFile, sal_True) ;

    arguments [0] <<= stream ;
    uno::Reference<backend::XSchema> schema(
            mFactory->createInstanceWithArguments(kXMLSchemaParser, arguments),
            uno::UNO_QUERY) ;

    if (!schema.is()) {
        throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot instantiate Schema")),
                *this) ;
    }
    return schema ;
}
//------------------------------------------------------------------------------

static const sal_Char *kImplementation =
                "com.sun.star.comp.configuration.backend.LocalSingleBackend" ;
static const sal_Char *kService =
                "com.sun.star.configuration.backend.SingleBackend" ;

static AsciiServiceName kServiceNames [] = { kService, 0 } ;
static const ServiceInfo kServiceInfo = { kImplementation, kServiceNames } ;

const ServiceInfo *getLocalBackendServiceInfo(void) { return &kServiceInfo ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalBackend(const CreationContext& aServiceManager) {
    return *new LocalSingleBackend(aServiceManager) ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kImplementationName(
                            RTL_CONSTASCII_USTRINGPARAM(kImplementation)) ;

rtl::OUString SAL_CALL LocalSingleBackend::getName(void) {
    return kImplementationName ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getName() ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kSingleBackendServiceName(
                            RTL_CONSTASCII_USTRINGPARAM(kService)) ;

sal_Bool SAL_CALL LocalSingleBackend::supportsService(
                                        const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return aServiceName.equals(kSingleBackendServiceName) ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocalSingleBackend::getServices(void) {
    return uno::Sequence<rtl::OUString>(&kSingleBackendServiceName, 1) ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL LocalSingleBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getServices() ;
}
//------------------------------------------------------------------------------

} } // configmgr.localbe
