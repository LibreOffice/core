/*************************************************************************
 *
 *  $RCSfile: localsinglebackend.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:20 $
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

#include "localsinglebackend.hxx"

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

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalSingleBackend::LocalSingleBackend(
        const uno::Reference<uno::XComponentContext>& xContext)
        : SingleBackendBase(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY) {
}
//------------------------------------------------------------------------------

LocalSingleBackend::~LocalSingleBackend(void) {}
//------------------------------------------------------------------------------
static inline bool isValidFileURL (rtl::OUString& _sFileURL)
{
    using osl::File;

    rtl::OUString sSystemPath;
    return _sFileURL.getLength() && (File::E_None == File::getSystemPathFromFileURL(_sFileURL, sSystemPath));
}
// ---------------------------------------------------------------------------------------
static
bool implEnsureAbsoluteURL(rtl::OUString & _rsURL) // also strips embedded dots etc.
{
    using osl::File;

    rtl::OUString sBasePath = _rsURL;
    OSL_VERIFY(osl_Process_E_None == osl_getProcessWorkingDir(&sBasePath.pData));

    rtl::OUString sAbsolute;
    if ( File::E_None == File::getAbsoluteFileURL(sBasePath, _rsURL, sAbsolute))
    {
        _rsURL = sAbsolute;
        return true;
    }
    else
    {
        OSL_ENSURE(false, "Could not get absolute file URL for valid URL");
        return false;
    }
}
// ---------------------------------------------------------------------------------------

static
bool implNormalizeURL(OUString & _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    OSL_PRECOND(aDirItem.is(), "Opened DirItem required");

    static const sal_uInt32 cFileStatusMask = FileStatusMask_FileURL;

    FileStatus aFileStatus(cFileStatusMask);

    if (aDirItem.getFileStatus(aFileStatus) != DirectoryItem::E_None)
        return false;

    OUString aNormalizedURL = aFileStatus.getFileURL();

    if (aNormalizedURL.getLength() == 0)
        return false;

    _sURL = aNormalizedURL;
    return true;
}

// ---------------------------------------------------------------------------------------
static
bool normalizeURL(OUString & _sURL)
{
    using namespace osl;

    DirectoryItem aDirItem;

    bool bResult = DirectoryItem::get(_sURL, aDirItem) == DirectoryItem::E_None;

    if ( bResult )
        bResult = implNormalizeURL(_sURL,aDirItem);

    return bResult;
}
// ---------------------------------------------------------------------------------------
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

static const rtl::OUString kSchemaDataUrl(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"SchemaDataUrl")) ;
static const rtl::OUString kDefaultDataUrl(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"DefaultLayerUrls")) ;
static const rtl::OUString kUserDataUrl(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"UserLayerUrl")) ;
static const rtl::OUString kEntity(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"EntityLayer")) ;

static const rtl::OUString kAdminModeFlag(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_ADMINFLAG)) ;

void SAL_CALL LocalSingleBackend::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backend::CannotConnectException,
           backend::BackendSetupException)
{
    if (aParameters.getLength() == 0) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to SingleBackend")),
                *this, 0) ;
    }
    uno::Reference<uno::XComponentContext> context ;

    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i) {
        if (aParameters [i] >>= context) { break ; }
    }

    // Setting: schema
    context->getValueByName(kSchemaDataUrl) >>= mSchemaDataUrl;
    //validate SchemaDataUrl
    validateFileURL(mSchemaDataUrl);
    //NormalizeURL
    implEnsureAbsoluteURL(mSchemaDataUrl);
    normalizeURL(mSchemaDataUrl);

    checkFileExists(mSchemaDataUrl);
    checkIfDirectory(mSchemaDataUrl);

    // Setting: default layer(s)
    uno::Any aDefaultDataSetting = context->getValueByName(kDefaultDataUrl);
    rtl::OUString defaults;

    if (context->getValueByName(kDefaultDataUrl) >>= defaults)
    {
        fillFromBlankSeparated(defaults, mDefaultDataUrls) ;
    }
    else
    {
        context->getValueByName(kDefaultDataUrl) >>= mDefaultDataUrls ;
    }
    //validate DefaultDataUrls
    for (sal_Int32 ix = 0; ix < mDefaultDataUrls.getLength(); ++ix)
    {
        validateFileURL(mDefaultDataUrls[ix]);
        //NormalizeURL
        implEnsureAbsoluteURL(mDefaultDataUrls[ix]);
        normalizeURL(mDefaultDataUrls[ix]);
        if(FileHelper::fileExists(mDefaultDataUrls[ix]))
        {
            checkIfDirectory(mDefaultDataUrls[ix]);
        }
    }

    // Setting: admin mode tag
    sal_Bool bAdminMode = false;
    context->getValueByName(kAdminModeFlag) >>= bAdminMode;

    if (bAdminMode)
    {
        // find given entity
        if ( (context->getValueByName(kEntity) >>= mUserDataUrl) && mUserDataUrl.getLength() )
        {
            //Validate UserDataUrl
            validateFileURL(mUserDataUrl);
            //NormalizeURL
            implEnsureAbsoluteURL(mUserDataUrl);
            normalizeURL(mUserDataUrl);
            if(FileHelper::fileExists(mUserDataUrl))
            {
                checkIfDirectory(mUserDataUrl);
            }

            for (sal_Int32 ix = 0; ix < mDefaultDataUrls.getLength(); ++ix)
            {
                if (mDefaultDataUrls.getConstArray()[ix].equals(mUserDataUrl))
                {
                    mDefaultDataUrls.realloc(ix);
                    // this is the last round through the loop
                }
            }
        }
        else if (mDefaultDataUrls.getLength()) // administrate first default layer
        {
            mUserDataUrl = mDefaultDataUrls[0];
            mDefaultDataUrls.realloc(0);
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
        //Validate UserDataUrl
        validateFileURL(mUserDataUrl);
        //GetAbsolsoluteURL
        implEnsureAbsoluteURL(mUserDataUrl);
        normalizeURL(mUserDataUrl);
        if(FileHelper::fileExists(mUserDataUrl))
        {
            checkIfDirectory(mUserDataUrl);
        }
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
    if (aEntity.getLength() > 0 && !aEntity.equals(mUserDataUrl)) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Can only access user data")),
                *this, 1) ;
    }
    sal_Int32 nbLayers = mDefaultDataUrls.getLength() + 1 ;
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
    sal_Bool bFoundDefaultLayer = sal_False;
    sal_Int32 nIndex = 0;

    if ((aEntity.getLength() > 0) && aEntity.equals(mUserDataUrl))
    {
        nIndex=-1;
    }
    else
    {
        OUString sTempEntityUrl(aEntity);
        normalizeURL(sTempEntityUrl);
        for (sal_Int32 ix = 0; ix < mDefaultDataUrls.getLength()&&!bFoundDefaultLayer; ix++)
        {
            OUString sTempDefaultUrl(mDefaultDataUrls[ix]);
            normalizeURL(sTempDefaultUrl);
            if (sTempEntityUrl.equals(sTempDefaultUrl))
            {
                bFoundDefaultLayer = sal_True;
                nIndex = ix;
            }
        }
        if (!bFoundDefaultLayer)
        {
            //Try normalized version of mUserDataUrl
            OUString sTempUserUrl(mUserDataUrl);
            normalizeURL(sTempUserUrl);
            if (sTempUserUrl.equals(sTempEntityUrl))
            {
                nIndex=-1;
            }
            else
            {
                rtl::OUStringBuffer sMsg;
                sMsg.appendAscii(" Cannot update data: Invalid Layer URL: \"");
                sMsg.append(aEntity);
                sMsg.appendAscii("\"");
                throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),
                                                     *this, 1) ;
            }
        }
    }
    return urlToLayerId(componentToPath(aComponent) + kDataSuffix, nIndex) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getOwnId(void)
    throw (uno::RuntimeException)
{
    return mUserDataUrl ;
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
                FileHelper::createOSLErrorString(errorCode),
                *this, uno::Any()) ;
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

static
inline
void impl_getLayerSubDirectories(rtl::OUString const & aLayerBaseUrl,
                             rtl::OUString& aMainLayerUrl,
                             rtl::OUString& aSubLayerUrl)
{
    aMainLayerUrl   = aLayerBaseUrl + kDataSubPath ;
    aSubLayerUrl    = aLayerBaseUrl + kLocalisedDataSubPath ;
}
//------------------------------------------------------------------------------

void LocalSingleBackend::getLayerSubDirectories(rtl::OUString const & aLayerBaseUrl,
                             rtl::OUString& aMainLayerUrl,
                             rtl::OUString& aSubLayerUrl)
{
    impl_getLayerSubDirectories(aLayerBaseUrl,aMainLayerUrl,aSubLayerUrl);
}
//------------------------------------------------------------------------------

void LocalSingleBackend::getLayerDirectories(sal_Int32 aLayerIndex,
                                             rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl)
{
    OUString aLayerBaseUrl = (aLayerIndex == -1 ? mUserDataUrl : mDefaultDataUrls [aLayerIndex]) ;

    impl_getLayerSubDirectories(aLayerBaseUrl,aLayerUrl,aSubLayerUrl);
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> LocalSingleBackend::createSimpleLayer(
                                                  const uno::Reference<lang::XMultiServiceFactory>& xFactory,
                                                  rtl::OUString const & aComponentUrl)
{
    SimpleLocalFileLayer * pLayer = new SimpleLocalFileLayer(xFactory, aComponentUrl);
    return pLayer;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> LocalSingleBackend::createSimpleLayer(
                                                  const uno::Reference<lang::XMultiServiceFactory>& xFactory,
                                                  rtl::OUString const & aLayerBaseUrl,
                                                  rtl::OUString const & aComponent)
{
    rtl::OUString aLayerUrl, aSubLayerUrl;
    impl_getLayerSubDirectories(aLayerBaseUrl,aLayerUrl,aSubLayerUrl);

    SimpleLocalFileLayer * pLayer = new SimpleLocalFileLayer(xFactory, aLayerUrl, componentToPath(aComponent));
    return pLayer;
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

static const sal_Char * const kImplementation =
                "com.sun.star.comp.configuration.backend.LocalSingleBackend" ;
static const sal_Char * const kBackendService =
                "com.sun.star.configuration.backend.SingleBackend" ;
static const sal_Char * const kLocalService =
                "com.sun.star.configuration.backend.LocalSingleBackend" ;

static AsciiServiceName kServiceNames [] = { kLocalService, 0, kBackendService, 0 } ;
static const ServiceImplementationInfo kServiceInfo = { kImplementation, kServiceNames, kServiceNames + 2 } ;

const ServiceRegistrationInfo *getLocalBackendServiceInfo()
{ return getRegistrationInfo(&kServiceInfo) ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalBackend(const CreationContext& xContext) {
    return *new LocalSingleBackend(xContext) ;
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
    return ServiceInfoHelper(&kServiceInfo).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleBackend::supportsService(
                                        const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(&kServiceInfo).supportsService(aServiceName);
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocalSingleBackend::getServices(void)
{
    return ServiceInfoHelper(&kServiceInfo).getSupportedServiceNames() ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL LocalSingleBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getSupportedServiceNames() ;
}

// ---------------------------------------------------------------------------------------

void LocalSingleBackend::validateFileURL(rtl::OUString& aFileURL)
{
     rtl::OUStringBuffer sMsg;
     sMsg.appendAscii(" Not a Valid File URL: \"");
     sMsg.append(aFileURL);
     sMsg.appendAscii("\"");
     if (!isValidFileURL( aFileURL))
     {
          throw com::sun::star::configuration::InvalidBootstrapFileException(
            sMsg.makeStringAndClear(),*this, aFileURL ) ;
     }
}
//------------------------------------------------------------------------------
void LocalSingleBackend::checkFileExists(rtl::OUString& aFileURL)
     throw (backend::CannotConnectException)
{
    rtl::OUStringBuffer sMsg;
    sMsg.appendAscii(" No Such File or Directory: \"");
    sMsg.append(aFileURL);
    sMsg.appendAscii("\"");
    if (!FileHelper::fileExists(aFileURL))
    {
        throw backend::CannotConnectException(sMsg.makeStringAndClear(), *this, uno::Any()) ;
    }
}
//------------------------------------------------------------------------------
void LocalSingleBackend::checkIfDirectory(rtl::OUString& aFileURL)
    throw (backend::BackendSetupException)
{
    rtl::OUStringBuffer sMsg;
    sMsg.appendAscii(" File:\"");
    sMsg.append(aFileURL);
    sMsg.appendAscii("\" Must be a Directory\"");
    if (!FileHelper::dirExists(aFileURL))
    {
        throw backend::BackendSetupException(sMsg.makeStringAndClear(),*this, uno::Any()) ;
    }
}

} } // configmgr.localbe
