/*************************************************************************
 *
 *  $RCSfile: localsinglebackend.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 13:30:21 $
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

LocalSingleBackend::LocalSingleBackend(
        const uno::Reference<uno::XComponentContext>& xContext)
        : SingleBackendBase(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY) {
}
//------------------------------------------------------------------------------

LocalSingleBackend::~LocalSingleBackend(void) {}
//------------------------------------------------------------------------------
static inline bool isValidFileURL (rtl::OUString const& _sFileURL)
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
osl::DirectoryItem::RC implNormalizeURL(OUString & _sURL, osl::DirectoryItem& aDirItem)
{
    using namespace osl;

    OSL_PRECOND(aDirItem.is(), "Opened DirItem required");

    static const sal_uInt32 cFileStatusMask = FileStatusMask_FileURL;

    FileStatus aFileStatus(cFileStatusMask);

    DirectoryItem::RC rc = aDirItem.getFileStatus(aFileStatus);

    if (rc == DirectoryItem::E_None)
    {
        OUString aNormalizedURL = aFileStatus.getFileURL();

        if (aNormalizedURL.getLength() != 0)
            _sURL = aNormalizedURL;
        else
            rc = DirectoryItem::E_INVAL;
    }
    return rc;
}

// ---------------------------------------------------------------------------------------
static
bool normalizeURL(OUString & _sURL, backend::XBackendEntities * pContext)
{
    using namespace osl;

    if (_sURL.getLength() == 0)
        return false;

    DirectoryItem aDirItem;

    DirectoryItem::RC rc = DirectoryItem::get(_sURL, aDirItem);

    if (rc == DirectoryItem::E_None)
        rc = implNormalizeURL(_sURL,aDirItem);

    switch (rc)
    {
    case DirectoryItem::E_None:  return true;

    case DirectoryItem::E_NOENT: return false;

    case DirectoryItem::E_ACCES:
        {
            rtl::OUStringBuffer msg;
            msg.appendAscii("LocalBackend: Cannot normalize URL \"" );
            msg.append(_sURL);
            msg.appendAscii("\" - InsufficientAccess");
            throw backend::InsufficientAccessRightsException(msg.makeStringAndClear(),pContext,uno::Any());
        }
    default:
        {
            rtl::OUStringBuffer msg;
            msg.appendAscii("LocalBackend: Cannot normalize URL \"" );
            msg.append(_sURL);
            msg.appendAscii("\" - ").append(FileHelper::createOSLErrorString(rc));
            throw backend::BackendAccessException(msg.makeStringAndClear(),pContext,uno::Any());
        }
    }
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
static inline bool checkOptionalArg(rtl::OUString& aArg)
{
    if (aArg.getLength() && aArg[0] == sal_Unicode('?'))
    {
        aArg = aArg.copy(1);
        return true;
    }
    else
    {
        return false;
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

    // Setting: schema diretory(ies)
    uno::Any const aSchemaDataSetting = context->getValueByName(kSchemaDataUrl);
    rtl::OUString schemas;

    if (aSchemaDataSetting >>= schemas)
    {
        fillFromBlankSeparated(schemas, mSchemaDataUrls) ;
    }
    else
    {
        aSchemaDataSetting >>=  mSchemaDataUrls;
    }
    //validate SchemaDataUrls
    sal_Int32 nExistingSchemaLocations = 0;
    for (sal_Int32 j = 0; j < mSchemaDataUrls.getLength(); ++j)
    {
        bool bOptional = checkOptionalArg(mSchemaDataUrls[j]);

        validateFileURL(mSchemaDataUrls[j]);
        //NormalizeURL
        implEnsureAbsoluteURL(mSchemaDataUrls[j]);
        normalizeURL(mSchemaDataUrls[j],this);

        if (!bOptional)
            checkFileExists(mSchemaDataUrls[j]);

        else if(!FileHelper::fileExists(mSchemaDataUrls[j]))
            continue; // skip the directory check

        checkIfDirectory(mSchemaDataUrls[j]);
        ++nExistingSchemaLocations;
    }
    if (0 == nExistingSchemaLocations)
    {
        rtl::OUString sMsg = rtl::OUString::createFromAscii("LocalBackend: No schema directories found");
        throw backend::BackendSetupException(sMsg,*this, uno::Any()) ;
    }

    // Setting: default layer(s)
    uno::Any const aDefaultDataSetting = context->getValueByName(kDefaultDataUrl);
    rtl::OUString defaults;

    if (aDefaultDataSetting >>= defaults)
    {
        fillFromBlankSeparated(defaults, mDefaultDataUrls) ;
    }
    else
    {
        aDefaultDataSetting >>= mDefaultDataUrls ;
    }
    //validate DefaultDataUrls
    for (sal_Int32 ix = 0; ix < mDefaultDataUrls.getLength(); ++ix)
    {
        validateFileURL(mDefaultDataUrls[ix]);
        //NormalizeURL
        implEnsureAbsoluteURL(mDefaultDataUrls[ix]);
        normalizeURL(mDefaultDataUrls[ix],this);
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
            normalizeURL(mUserDataUrl,this);
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
        normalizeURL(mUserDataUrl,this);
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

const sal_Int32 k_UserLayerEntity = 0;
const sal_Int32 k_InvalidEntity         = k_UserLayerEntity - 1;
const sal_Int32 k_DefaultEntityOffset   = k_UserLayerEntity + 1;
//------------------------------------------------------------------------------
static inline bool isValidEntity(sal_Int32 ent)
{
    return ent > k_InvalidEntity;
}
//------------------------------------------------------------------------------
static inline sal_Int32 indexToEntity(sal_Int32 ix)
{
    OSL_ASSERT(0 <= ix);
    return ix + k_DefaultEntityOffset;
}
static inline sal_Int32 entityToIndex(sal_Int32 ent)
{
    OSL_ASSERT(k_DefaultEntityOffset <= ent);
    return ent - k_DefaultEntityOffset;
}
//------------------------------------------------------------------------------


/**
  Transforms a file url into a layer id. The layer id will
  contain the URL passed plus an integer indicating which
  layer the URL points to. If the integer is 0, the URL
  is a user layer, otherwise it is one of the default layers.

  @param aFileUrl   URL to encode
  @param aIndex     index of the layer concerned (0 = user, other = default)
  @return   layer id
  */
static
rtl::OUString urlToLayerId(const rtl::OUString& aFileUrl,sal_Int32 aIndex)
{
    rtl::OUStringBuffer id ;

    OSL_ASSERT(isValidEntity(aIndex));
    if (aIndex)
        id.append(aIndex).appendAscii(" ",1);     // non-user layers
    else
        id.appendAscii("U ",2); // user layer

    id.append(aFileUrl) ;
    return id.makeStringAndClear() ;
}

static
inline
bool layerIdToUrl( const rtl::OUString& aLayerId,
                    rtl::OUString& aFileUrl,
                    sal_Int32& aIndex)
{
    sal_Int32 const sep = aLayerId.indexOf(sal_Unicode(' ')) ;

    if (sep < 0) return false;

    // detect user layer id
    if (aLayerId[0] == sal_Unicode('U'))
    {
        if (sep != 1) return false;
        aIndex   = 0;
    }
    else
    {
        aIndex   = aLayerId.copy(0, sep).toInt32() ;
        if (0 == aIndex || !isValidEntity(aIndex)) return false;

        OSL_ENSURE( aLayerId.copy(0, sep).equals(OUString::valueOf(aIndex)),
                    "Invalid layer id was not detected");
    }
    aFileUrl = aLayerId.copy(sep + 1);

    return true;
}
//------------------------------------------------------------------------------
sal_Int32 LocalSingleBackend::resolveLayerId(const rtl::OUString& aLayerId,
                                                  rtl::OUString& aFileUrl)
{
    sal_Int32 nIndex = k_InvalidEntity;
    if (!layerIdToUrl(aLayerId,aFileUrl,nIndex))
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid layer id: "));

        // layer id is always the second parameter
        throw lang::IllegalArgumentException(sMsg.concat(aLayerId), *this, 2);
    }
    OSL_ASSERT(isValidEntity(nIndex));
    return nIndex;
}
//------------------------------------------------------------------------------

sal_Int32 LocalSingleBackend::findEntity(const rtl::OUString& aEntity)
{
    if (aEntity.getLength() == 0)
    {
        return k_InvalidEntity;
    }

    // quick check for OwnerEntity first
    if (aEntity.equals(mUserDataUrl))
    {
        return k_UserLayerEntity;
    }

    OUString sNormalizedEntityUrl(aEntity);
    normalizeURL(sNormalizedEntityUrl,this);

    for (sal_Int32 ix = 0; ix < mDefaultDataUrls.getLength(); ++ix)
    {
        OUString sNormalizedDefaultUrl(mDefaultDataUrls[ix]);
        normalizeURL(sNormalizedDefaultUrl,this);

        if (sNormalizedEntityUrl.equals(sNormalizedDefaultUrl))
        {
            // found it
            return indexToEntity(ix);
        }
    }

    //Try normalized version of mUserDataUrl
    OUString sNormalizedUserUrl(mUserDataUrl);
    normalizeURL(sNormalizedUserUrl,this);
    if (sNormalizedEntityUrl.equals(sNormalizedUserUrl))
    {
        return k_UserLayerEntity;
    }

    // not found
    return k_InvalidEntity;
}
//------------------------------------------------------------------------------

static const rtl::OUString kDataSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcu")) ;

uno::Sequence<rtl::OUString> SAL_CALL LocalSingleBackend::listLayerIds(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    sal_Int32 nEntity = findEntity(aEntity);
    if ( !isValidEntity(nEntity) )
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Unknown entity: "));

        throw lang::IllegalArgumentException(sMsg.concat(aEntity), *this, 2);
    }

    sal_Int32 nDefLayers = (k_UserLayerEntity == nEntity)
                            ? mDefaultDataUrls.getLength()
                            : entityToIndex(nEntity);

    OSL_ASSERT(0 <= nDefLayers && nDefLayers <= mDefaultDataUrls.getLength());

    rtl::OUString const componentSubPath = componentToPath(aComponent) + kDataSuffix ;

    uno::Sequence<rtl::OUString> aLayerIds(nDefLayers + 1) ;
    OSL_ASSERT(0 < aLayerIds.getLength());

    // First, the defaults...
    for (sal_Int32 ix = 0; ix < nDefLayers ; ++ ix)
    {
        aLayerIds[ix] = urlToLayerId(componentSubPath, indexToEntity(ix)) ;
    }
    // Then the entity data.
    aLayerIds [nDefLayers] = urlToLayerId(componentSubPath, nEntity) ;

    return  aLayerIds;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSingleBackend::getUpdateLayerId(
        const rtl::OUString& aComponent, const rtl::OUString& aEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,
            uno::RuntimeException)
{
    sal_Int32 nEntity = findEntity(aEntity);
    if ( !isValidEntity(nEntity) )
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Unknown entity for update: "));

        throw lang::IllegalArgumentException(sMsg.concat(aEntity), *this, 2);
    }

    return urlToLayerId(componentToPath(aComponent) + kDataSuffix, nEntity) ;
}
//------------------------------------------------------------------------------

OUString SAL_CALL LocalSingleBackend::getOwnerEntity()
    throw (uno::RuntimeException)
{
    return mUserDataUrl ;
}
//------------------------------------------------------------------------------

OUString SAL_CALL LocalSingleBackend::getAdminEntity()
    throw (uno::RuntimeException)
{
    return mDefaultDataUrls.getLength() > 0 ? mDefaultDataUrls[0] : mUserDataUrl;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleBackend::supportsEntity( const OUString& aEntity )
    throw (backend::BackendAccessException, uno::RuntimeException)
{
    return isValidEntity(findEntity(aEntity)) ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSingleBackend::isEqualEntity(const OUString& aEntity, const OUString& aOtherEntity)
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
    normalizeURL(aNormalizedEntity,this);

    OUString aNormalizedOther(aOtherEntity);
    normalizeURL(aNormalizedOther,this);

    return aNormalizedEntity == aNormalizedOther;
}
//------------------------------------------------------------------------------

sal_Bool LocalSingleBackend::isMoreRecent(const rtl::OUString& aFileUrl,
                                          sal_Int32 aLayerIndex,
                                          const rtl::OUString& aTimestamp) {
    rtl::OUString layerUrl ;
    rtl::OUString subLayerUrl ;

    getLayerDirectories(aLayerIndex, layerUrl, subLayerUrl) ;

    return (BasicLocalFileLayer::getTimestamp(layerUrl + aFileUrl).compareTo(
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

    sal_Int32 defaultIndex = resolveLayerId(aLayerId, fileUrl) ;

    if (!isMoreRecent(fileUrl, defaultIndex, aTimestamp)) { return NULL ; }

    uno::Reference<backend::XUpdatableLayer> xLayer = getFileLayer(fileUrl,defaultIndex);
    uno::Reference<backend::XLayer> xResult = xLayer.get();
    return xResult;
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

static const rtl::OUString kSchemaSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcs")) ;
static const rtl::OUString kXMLSchemaParser(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.xml.SchemaParser")) ;

uno::Reference<backend::XSchema> SAL_CALL
    LocalSingleBackend::getComponentSchema(const rtl::OUString& aComponent)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    rtl::OUString subPath = componentToPath(aComponent) ;

    osl::File * schemaFile = NULL;
    OUString errorMessage;
    bool bInsufficientAccess = false;
    for (sal_Int32 ix = 0; ix < mSchemaDataUrls.getLength(); ++ix)
    {
        rtl::OUStringBuffer schemaUrl(mSchemaDataUrls[ix]) ;

        schemaUrl.append(subPath).append(kSchemaSuffix) ;

        OUString const aFileUrl = schemaUrl.makeStringAndClear();

        std::auto_ptr<osl::File> checkFile( new osl::File(aFileUrl) );
        osl::File::RC rc = checkFile->open(OpenFlag_Read) ;

        if (rc == osl::File::E_None)
        {
            schemaFile = checkFile.release();
            break;
        }
        else if (rc != osl::File::E_NOENT)
        {
            if (rc == osl::File::E_ACCES)
                bInsufficientAccess =true;

            // accumulate error messages
            rtl::OUStringBuffer sMsg(errorMessage);
            if (errorMessage.getLength())
                sMsg.appendAscii("LocalFile SchemaSupplier - Error accessing schema: ");

            sMsg.appendAscii("\n- Cannot open input file \"");
            sMsg.append(aFileUrl);
            sMsg.appendAscii("\" : ");
            sMsg.append(FileHelper::createOSLErrorString(rc));

            errorMessage = sMsg.makeStringAndClear();
        }
    }

    if (NULL == schemaFile)
    {
        if (errorMessage.getLength() != 0)
        {
            // a real error occured
            io::IOException ioe(errorMessage,*this);

            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LocalFileLayer - Cannot readData: ").append(errorMessage);

            if (bInsufficientAccess)
                throw backend::InsufficientAccessRightsException(sMsg.makeStringAndClear(),*this,uno::makeAny(ioe));
            else
                throw backend::BackendAccessException(sMsg.makeStringAndClear(),*this,uno::makeAny(ioe));
        }
        // simply not found
        return NULL;
    }

    uno::Sequence<uno::Any> arguments(1) ;
    uno::Reference<io::XInputStream> stream( new OSLInputStreamWrapper(schemaFile, true) );

    arguments [0] <<= stream ;
    uno::Reference<backend::XSchema> schema(
            mFactory->createInstanceWithArguments(kXMLSchemaParser, arguments),
            uno::UNO_QUERY) ;

    if (!schema.is())
    {
        throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot instantiate Schema Parser for ")) + aComponent,
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
    OSL_ASSERT(isValidEntity(aLayerIndex));
    OUString aLayerBaseUrl = (aLayerIndex == k_UserLayerEntity ? mUserDataUrl : mDefaultDataUrls [entityToIndex(aLayerIndex)]) ;

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


uno::Reference<backend::XUpdatableLayer> LocalSingleBackend::getFileLayer(const rtl::OUString& aLayerId)
    throw (lang::IllegalArgumentException)
{
    rtl::OUString fileUrl ;
    sal_Int32 defaultIndex = resolveLayerId(aLayerId, fileUrl) ;

    return getFileLayer(fileUrl, defaultIndex) ;
}
//------------------------------------------------------------------------------

uno::Reference<backend::XUpdatableLayer> LocalSingleBackend::getFileLayer(
                    const rtl::OUString& aComponent, sal_Int32 aLayerIndex) {
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(aLayerIndex, layerPath, subLayerPath) ;
    return createLocalFileLayer(mFactory, layerPath, aComponent, subLayerPath) ;
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
     if (!isValidFileURL( aFileURL))
     {
         rtl::OUStringBuffer sMsg;
         sMsg.appendAscii(" Not a Valid File URL: \"");
         sMsg.append(aFileURL);
         sMsg.appendAscii("\"");

         // todo: get configmgrrc url from context
          throw com::sun::star::configuration::InvalidBootstrapFileException(
            sMsg.makeStringAndClear(),*this, OUString() ) ;
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
