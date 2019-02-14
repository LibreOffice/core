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

#include <string.h>

#include <ZipPackageFolder.hxx>
#include <ZipFile.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageStream.hxx>
#include <PackageConstants.hxx>
#include "ZipPackageFolderEnumeration.hxx"
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <rtl/digest.h>
#include "ContentInfo.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <EncryptedDataHeader.hxx>
#include <rtl/instance.hxx>

using namespace com::sun::star;
using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace cppu;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

namespace { struct lcl_CachedImplId : public rtl::Static< cppu::OImplementationId, lcl_CachedImplId > {}; }

ZipPackageFolder::ZipPackageFolder( const css::uno::Reference < css::uno::XComponentContext >& xContext,
                                    sal_Int32 nFormat,
                                    bool bAllowRemoveOnInsert )
{
    m_xContext = xContext;
    m_nFormat = nFormat;
    mbAllowRemoveOnInsert = bAllowRemoveOnInsert;
    SetFolder ( true );
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = STORED;
    aEntry.nTime        = -1;
    aEntry.nCrc         = 0;
    aEntry.nCompressedSize = 0;
    aEntry.nSize        = 0;
    aEntry.nOffset      = -1;
}

ZipPackageFolder::~ZipPackageFolder()
{
}

bool ZipPackageFolder::LookForUnexpectedODF12Streams( const OUString& aPath )
{
    bool bHasUnexpected = false;

    for (const auto& [rShortName, rxInfo] : maContents)
    {
        const ZipContentInfo &rInfo = *rxInfo;

        if ( rInfo.bFolder )
        {
            if ( aPath == "META-INF/" )
            {
                // META-INF is not allowed to contain subfolders
                bHasUnexpected = true;
            }
            else
            {
                OUString sOwnPath = aPath + rShortName + "/";
                bHasUnexpected = rInfo.pFolder->LookForUnexpectedODF12Streams( sOwnPath );
            }
        }
        else
        {
            if ( aPath == "META-INF/" )
            {
                if ( rShortName != "manifest.xml"
                  && rShortName.indexOf( "signatures" ) == -1 )
                {
                    // a stream from META-INF with unexpected name
                    bHasUnexpected = true;
                }

                // streams from META-INF with expected names are allowed not to be registered in manifest.xml
            }
            else if ( !rInfo.pStream->IsFromManifest() )
            {
                // the stream is not in META-INF and is not registered in manifest.xml,
                // check whether it is an internal part of the package format
                if ( !aPath.isEmpty() || rShortName != "mimetype" )
                {
                    // if it is not "mimetype" from the root it is not a part of the package
                    bHasUnexpected = true;
                }
            }
        }

        if (bHasUnexpected)
            break;
    }

    return bHasUnexpected;
}

void ZipPackageFolder::setChildStreamsTypeByExtension( const beans::StringPair& aPair )
{
    OUString aExt;
    if ( aPair.First.toChar() == '.' )
        aExt = aPair.First;
    else
        aExt = "." + aPair.First;

    for (const auto& [rShortName, rxInfo] : maContents)
    {
        const ZipContentInfo &rInfo = *rxInfo;

        if ( rInfo.bFolder )
            rInfo.pFolder->setChildStreamsTypeByExtension( aPair );
        else
        {
            sal_Int32 nPathLength = rShortName.getLength();
            sal_Int32 nExtLength = aExt.getLength();
            if ( nPathLength >= nExtLength && rShortName.match( aExt, nPathLength - nExtLength ) )
                rInfo.pStream->SetMediaType( aPair.Second );
        }
    }
}

css::uno::Sequence < sal_Int8 > ZipPackageFolder::static_getImplementationId()
{
    return lcl_CachedImplId::get().getImplementationId();
}

    // XNameContainer
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const uno::Any& aElement )
{
    if (hasByName(aName))
        throw ElementExistException(THROW_WHERE );

    uno::Reference < XUnoTunnel > xRef;
    aElement >>= xRef;
    if ( !(aElement >>= xRef) )
        throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

    sal_Int64 nTest;
    ZipPackageEntry *pEntry;
    if ( ( nTest = xRef->getSomething ( ZipPackageFolder::static_getImplementationId() ) ) != 0 )
    {
        ZipPackageFolder *pFolder = reinterpret_cast < ZipPackageFolder * > ( nTest );
        pEntry = static_cast < ZipPackageEntry * > ( pFolder );
    }
    else if ( ( nTest = xRef->getSomething ( ZipPackageStream::static_getImplementationId() ) ) != 0 )
    {
        ZipPackageStream *pStream = reinterpret_cast < ZipPackageStream * > ( nTest );
        pEntry = static_cast < ZipPackageEntry * > ( pStream );
    }
    else
       throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

    if (pEntry->getName() != aName )
        pEntry->setName (aName);
    doInsertByName ( pEntry, true );
}

void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException(THROW_WHERE );
    maContents.erase( aIter );
}
    // XEnumerationAccess
uno::Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
{
    return uno::Reference < XEnumeration> (new ZipPackageFolderEnumeration(maContents));
}
    // XElementAccess
uno::Type SAL_CALL ZipPackageFolder::getElementType(  )
{
    return cppu::UnoType<XUnoTunnel>::get();
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
{
    return !maContents.empty();
}
    // XNameAccess
ZipContentInfo& ZipPackageFolder::doGetByName( const OUString& aName )
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException(THROW_WHERE );
    return *aIter->second;
}

uno::Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
{
    return uno::makeAny ( doGetByName ( aName ).xTunnel );
}
uno::Sequence< OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
{
    return comphelper::mapKeysToSequence(maContents);
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const OUString& aName )
{
    return maContents.find ( aName ) != maContents.end ();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const uno::Any& aElement )
{
    if ( !hasByName( aName ) )
        throw NoSuchElementException(THROW_WHERE );

    removeByName( aName );
    insertByName(aName, aElement);
}

bool ZipPackageFolder::saveChild(
        const OUString &rPath,
        std::vector < uno::Sequence < PropertyValue > > &rManList,
        ZipOutputStream & rZipOut,
        const uno::Sequence < sal_Int8 >& rEncryptionKey,
        sal_Int32 nPBKDF2IterationCount,
        const rtlRandomPool &rRandomPool)
{
    const OUString sMediaTypeProperty ("MediaType");
    const OUString sVersionProperty ("Version");
    const OUString sFullPathProperty ("FullPath");

    uno::Sequence < PropertyValue > aPropSet (PKG_SIZE_NOENCR_MNFST);
    OUString sTempName = rPath + "/";

    if ( !GetMediaType().isEmpty() )
    {
        aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
        aPropSet[PKG_MNFST_MEDIATYPE].Value <<= GetMediaType();
        aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
        aPropSet[PKG_MNFST_VERSION].Value <<= GetVersion();
        aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
        aPropSet[PKG_MNFST_FULLPATH].Value <<= sTempName;
    }
    else
        aPropSet.realloc( 0 );

    saveContents( sTempName, rManList, rZipOut, rEncryptionKey, nPBKDF2IterationCount, rRandomPool);

    // folder can have a mediatype only in package format
    if ( aPropSet.getLength() && ( m_nFormat == embed::StorageFormats::PACKAGE ) )
        rManList.push_back( aPropSet );

    return true;
}

void ZipPackageFolder::saveContents(
        const OUString &rPath,
        std::vector < uno::Sequence < PropertyValue > > &rManList,
        ZipOutputStream & rZipOut,
        const uno::Sequence < sal_Int8 >& rEncryptionKey,
        sal_Int32 nPBKDF2IterationCount,
        const rtlRandomPool &rRandomPool ) const
{
    bool bWritingFailed = false;

    if ( maContents.empty() && !rPath.isEmpty() && m_nFormat != embed::StorageFormats::OFOPXML )
    {
        // it is an empty subfolder, use workaround to store it
        ZipEntry* pTempEntry = new ZipEntry(aEntry);
        pTempEntry->nPathLen = static_cast<sal_Int16>( OUStringToOString( rPath, RTL_TEXTENCODING_UTF8 ).getLength() );
        pTempEntry->nExtraLen = -1;
        pTempEntry->sPath = rPath;

        try
        {
            ZipOutputStream::setEntry(pTempEntry);
            rZipOut.writeLOC(pTempEntry);
            rZipOut.rawCloseEntry();
        }
        catch ( ZipException& )
        {
            bWritingFailed = true;
        }
        catch ( IOException& )
        {
            bWritingFailed = true;
        }
    }

    bool bMimeTypeStreamStored = false;
    OUString aMimeTypeStreamName("mimetype");
    if ( m_nFormat == embed::StorageFormats::ZIP && rPath.isEmpty() )
    {
        // let the "mimetype" stream in root folder be stored as the first stream if it is zip format
        ContentHash::const_iterator aIter = maContents.find ( aMimeTypeStreamName );
        if ( aIter != maContents.end() && !(*aIter).second->bFolder )
        {
            bMimeTypeStreamStored = true;
            bWritingFailed = !aIter->second->pStream->saveChild(
                rPath + aIter->first, rManList, rZipOut, rEncryptionKey, nPBKDF2IterationCount, rRandomPool );
        }
    }

    for (const auto& [rShortName, rxInfo] : maContents)
    {
        const ZipContentInfo &rInfo = *rxInfo;

        if ( !bMimeTypeStreamStored || rShortName != aMimeTypeStreamName )
        {
            if (rInfo.bFolder)
            {
                bWritingFailed = !rInfo.pFolder->saveChild(
                    rPath + rShortName, rManList, rZipOut, rEncryptionKey, nPBKDF2IterationCount, rRandomPool );
            }
            else
            {
                bWritingFailed = !rInfo.pStream->saveChild(
                    rPath + rShortName, rManList, rZipOut, rEncryptionKey, nPBKDF2IterationCount, rRandomPool );
            }
        }
    }

    if( bWritingFailed )
        throw uno::RuntimeException(THROW_WHERE );
}

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == memcmp(static_getImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
{
    if ( aPropertyName == "MediaType" )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException(THROW_WHERE );

        aValue >>= msMediaType;
    }
    else if ( aPropertyName == "Version" )
        aValue >>= m_sVersion;
    else if ( aPropertyName == "Size" )
        aValue >>= aEntry.nSize;
    else
        throw UnknownPropertyException(THROW_WHERE );
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
{
    if ( PropertyName == "MediaType" )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException(THROW_WHERE );

        return uno::makeAny ( msMediaType );
    }
    else if ( PropertyName == "Version" )
        return uno::makeAny( m_sVersion );
    else if ( PropertyName == "Size" )
        return uno::makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException(THROW_WHERE );
}

void ZipPackageFolder::doInsertByName ( ZipPackageEntry *pEntry, bool bSetParent )
{
    if ( pEntry->IsFolder() )
        maContents[pEntry->getName()] = std::make_unique<ZipContentInfo>(static_cast<ZipPackageFolder*>(pEntry));
    else
        maContents[pEntry->getName()] = std::make_unique<ZipContentInfo>(static_cast<ZipPackageStream*>(pEntry));
    if ( bSetParent )
        pEntry->setParent ( *this );
}

OUString ZipPackageFolder::getImplementationName()
{
    return OUString("ZipPackageFolder");
}

uno::Sequence< OUString > ZipPackageFolder::getSupportedServiceNames()
{
    uno::Sequence< OUString > aNames { "com.sun.star.packages.PackageFolder" };
    return aNames;
}

sal_Bool SAL_CALL ZipPackageFolder::supportsService( OUString const & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
