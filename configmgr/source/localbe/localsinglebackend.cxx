/*************************************************************************
 *
 *  $RCSfile: localsinglebackend.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: jb $ $Date: 2002-07-14 16:49:41 $
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

static void fillFromBlankSeparated(const rtl::OUString& aList,
                                   uno::Sequence<rtl::OUString>& aTarget) {
    std::vector<rtl::OUString> tokens ;
    sal_Int32 nextToken = 0 ;

    do {
        tokens.push_back(aList.getToken(0, ' ', nextToken)) ;
    } while (nextToken >= 0) ;
    if (tokens.size() > 0) {
        aTarget.realloc(tokens.size()) ;
        std::vector<rtl::OUString>::const_iterator token ;
        sal_Int32 i = 0 ;

        for (token = tokens.begin() ; token != tokens.end() ; ++ token) {
            aTarget [i ++] = *token ;
        }
    }
}
//------------------------------------------------------------------------------

static const rtl::OUString kMetaConfPrefix(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.bootstrap.")) ;
static const rtl::OUString kSchemaDataUrl(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SchemaDataUrl"))) ;
static const rtl::OUString kDefaultDataUrl(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultLayerUrls"))) ;
static const rtl::OUString kUserDataUrl(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UserLayerUrl"))) ;
static const rtl::OUString kEntity(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EntityLayer"))) ;

static const rtl::OUString kAdminMode(kMetaConfPrefix +
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_session_class_"))) ;
static const rtl::OUString kAdminModeValue(
        RTL_CONSTASCII_USTRINGPARAM("adminconfiguration")) ;

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

    // Setting: schema
    context->getValueByName(kSchemaDataUrl) >>= mSchemaDataUrl;

    // Setting: default layer(s)
    uno::Any aDefaultDataSetting = context->getValueByName(kDefaultDataUrl);
    rtl::OUString defaults;

    if (context->getValueByName(kDefaultDataUrl) >>= defaults)
    {
        fillFromBlankSeparated(defaults, mDefaultDataUrl) ;
    }
    else
    {
        context->getValueByName(kDefaultDataUrl) >>= mDefaultDataUrl ;
    }

    // Setting: admin mode tag
    rtl::OUString adminModeSelector ;
    bool bAdminMode =
            (context->getValueByName(kAdminMode) >>= adminModeSelector) &&
             adminModeSelector.equalsIgnoreAsciiCase(kAdminModeValue) ;

    if (bAdminMode)
    {
        // find given entity
        if ( (context->getValueByName(kEntity) >>= mOwnId) && mOwnId.getLength() )
        {
            for (sal_Int32 ix = 0; ix < mDefaultDataUrl.getLength(); ++ix)
            {
                if (mDefaultDataUrl.getConstArray()[ix].equals(mOwnId))
                {
                    mDefaultDataUrl.realloc(ix);
                    // this is the last round through the loop
                }
            }
            mUserDataUrl = mOwnId;
        }
        else if (sal_Int32 nLen = mDefaultDataUrl.getLength()) // administrate last default layer
        {
            --nLen;
            mUserDataUrl = mOwnId = mDefaultDataUrl[nLen];
            mDefaultDataUrl.realloc(nLen);
        }
        else
        {
            OSL_ENSURE(false, "Cannot find target entity for admin mode - fallback to normal mode");
            bAdminMode = false;
        }
    }

    if (!bAdminMode)
    {
        context->getValueByName(kUserDataUrl) >>= mUserDataUrl ;
        mOwnId = mUserDataUrl;
    }
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

/**
  Transforms a file url into a layer id. The layer id will
  contain the URL passed plus an integer indicating which
  layer the URL points to. If the integer is -1, the URL
  is a user layer, otherwise it is one of the default layers.

  @param aFileUrl   URL to encode
  @param aIndex     index of the layer concerned (-1 = user, other = default)
  @return   layer id
  */
static rtl::OUString urlToLayerId(const rtl::OUString& aFileUrl,
                                  sal_Int32 aIndex) {
    rtl::OUStringBuffer retCode ;

    retCode.append(aIndex).appendAscii(" ").append(aFileUrl) ;
    return retCode.makeStringAndClear() ;
}

static void layerIdToUrl(const rtl::OUString& aLayerId,
                         rtl::OUString& aFileUrl,
                         sal_Int32& aIndex) {
    sal_Int32 space = aLayerId.indexOf(' ') ;

    if (space == -1) {
        aFileUrl = aLayerId ;
        aIndex = -1 ;
    }
    else {
        aFileUrl = aLayerId.copy(space + 1) ;
        aIndex = aLayerId.copy(0, space).toInt32() ;
    }
}
//------------------------------------------------------------------------------

static const rtl::OUString kDataSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcu")) ;

uno::Sequence<rtl::OUString> SAL_CALL LocalSingleBackend::listLayerIds(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    if (aEntity.getLength() > 0 && !aEntity.equals(mOwnId)) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Can only access user data")),
                *this, 1) ;
    }
    sal_Int32 nbLayers = mDefaultDataUrl.getLength() + 1 ;
    uno::Sequence<rtl::OUString> retCode(nbLayers) ;
    rtl::OUString componentSubPath = componentToPath(aComponent) + kDataSuffix ;

    // First, the defaults...
    for (sal_Int32 i = 0 ; i < nbLayers - 1 ; ++ i) {
        retCode [i] = urlToLayerId(componentSubPath, i) ;
    }
    // Then the user delta.
    retCode [nbLayers - 1] = urlToLayerId(componentSubPath, -1) ;
    return retCode ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getUpdateLayerId(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    if (aEntity.getLength() > 0 && !aEntity.equals(mOwnId)) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Can only access user data")),
                *this, 1) ;
    }
    return urlToLayerId(componentToPath(aComponent) + kDataSuffix, -1) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getOwnId(void)
    throw (uno::RuntimeException)
{
    return mOwnId ;
}
//------------------------------------------------------------------------------

sal_Bool LocalSingleBackend::isMoreRecent(const rtl::OUString& aFileUrl,
                                          sal_Int32 aLayerIndex,
                                          const rtl::OUString& aTimestamp) {
    rtl::OUString layerUrl ;
    rtl::OUString subLayerUrl ;

    getLayerDirectories(aLayerIndex, layerUrl, subLayerUrl) ;
    return (LocalFileLayer::getTimestamp(layerUrl + aFileUrl).compareTo(
                                                            aTimestamp) > 0) ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kDataSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/data")) ;
static const rtl::OUString kLocalisedDataSubPath(
                                        RTL_CONSTASCII_USTRINGPARAM("/res")) ;

uno::Reference<backend::XLayer> SAL_CALL LocalSingleBackend::getLayer(
        const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString fileUrl ;
    sal_Int32 defaultIndex = -1 ;

    layerIdToUrl(aLayerId, fileUrl, defaultIndex) ;
    if (!isMoreRecent(fileUrl, defaultIndex, aTimestamp)) { return NULL ; }
    uno::Reference<backend::XCompositeLayer> layer = getFileLayer(fileUrl,
                                                                defaultIndex) ;

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
    return getFileLayer(aLayerId) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayerHandler> SAL_CALL
LocalSingleBackend::getWriteHandler(const rtl::OUString& aLayerId)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    LocalFileLayer *layer = getFileLayer(aLayerId) ;
    uno::Reference<backend::XUpdatableLayer> xLayer(layer);

    uno::Reference<backend::XLayerHandler> retCode = layer->createLayerWriter() ;

    return retCode ;
}
//------------------------------------------------------------------------------

static const rtl::OUString kSchemaSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcs")) ;
static const rtl::OUString kXMLSchemaParser(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.xml.SchemaParser")) ;

uno::Reference<backend::XSchema> SAL_CALL LocalSingleBackend::getSchema(
                                              const rtl::OUString& aComponent)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    rtl::OUString subPath = componentToPath(aComponent) ;
    rtl::OUStringBuffer schemaUrl(mSchemaDataUrl) ;

    schemaUrl.append(subPath).append(kSchemaSuffix) ;
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

LocalFileLayer *LocalSingleBackend::getFileLayer(const rtl::OUString& aLayerId)
    throw (lang::IllegalArgumentException)
{
    rtl::OUString fileUrl ;
    sal_Int32 defaultIndex = -1 ;

    layerIdToUrl(aLayerId, fileUrl, defaultIndex) ;
    return getFileLayer(fileUrl, defaultIndex) ;
}
//------------------------------------------------------------------------------

LocalFileLayer *LocalSingleBackend::getFileLayer(
                    const rtl::OUString& aComponent, sal_Int32 aLayerIndex) {
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(aLayerIndex, layerPath, subLayerPath) ;
    return new LocalFileLayer(mFactory, layerPath, aComponent, subLayerPath) ;
}
//------------------------------------------------------------------------------

void LocalSingleBackend::getLayerDirectories(sal_Int32 aLayerIndex,
                                             rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl)
{
    OUString const & aLayerBaseUrl = (aLayerIndex == -1 ? mUserDataUrl : mDefaultDataUrl [aLayerIndex]) ;

    aLayerUrl       = aLayerBaseUrl + kDataSubPath ;
    aSubLayerUrl    = aLayerBaseUrl + kLocalisedDataSubPath ;
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
