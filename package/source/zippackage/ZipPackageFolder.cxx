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

#include <ZipPackageFolder.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageStream.hxx>
#include <PackageConstants.hxx>
#include "ZipPackageFolderEnumeration.hxx"
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

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

bool ZipPackageFolder::LookForUnexpectedODF12Streams(
        std::u16string_view const aPath, bool const isWholesomeEncryption)
{
    bool bHasUnexpected = false;

    for (const auto& [rShortName, rInfo] : maContents)
    {
        if ( rInfo.bFolder )
        {
            if ( aPath == u"META-INF/" )
            {
                // META-INF is not allowed to contain subfolders
                bHasUnexpected = true;
            }
            else if (isWholesomeEncryption && rShortName != u"META-INF")
            {
                bHasUnexpected = true;
            }
            else
            {
                OUString sOwnPath = aPath + rShortName + "/";
                bHasUnexpected = rInfo.pFolder->LookForUnexpectedODF12Streams(sOwnPath, isWholesomeEncryption);
            }
        }
        else
        {
            if ( aPath == u"META-INF/" )
            {
                if ( rShortName != "manifest.xml"
                  && rShortName.indexOf( "signatures" ) == -1 )
                {
                    // a stream from META-INF with unexpected name
                    bHasUnexpected = true;
                }

                // streams from META-INF with expected names are allowed not to be registered in manifest.xml
            }
            else if (isWholesomeEncryption && rShortName != "mimetype" && rShortName != "encrypted-package")
            {
                bHasUnexpected = true;
            }
            else if ( !rInfo.pStream->IsFromManifest() )
            {
                // the stream is not in META-INF and is not registered in manifest.xml,
                // check whether it is an internal part of the package format
                if ( !aPath.empty() || rShortName != "mimetype" )
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

    for (const auto& [rShortName, rInfo] : maContents)
    {
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

    // XNameContainer
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const uno::Any& aElement )
{
    if (hasByName(aName))
        throw ElementExistException(THROW_WHERE );

    uno::Reference < XInterface > xRef;
    aElement >>= xRef;
    if ( !(aElement >>= xRef) )
        throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

    ZipPackageEntry* pEntry = dynamic_cast<ZipPackageFolder*>(xRef.get());
    if (!pEntry)
        pEntry = dynamic_cast<ZipPackageStream*>(xRef.get());
    if (!pEntry)
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
    return cppu::UnoType<XInterface>::get();
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
    return aIter->second;
}

uno::Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
{
    return uno::Any ( uno::Reference(cppu::getXWeak(doGetByName ( aName ).xPackageEntry.get())) );
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
        ::std::optional<sal_Int32> const oPBKDF2IterationCount,
        ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> const oArgon2Args,
        const rtlRandomPool &rRandomPool)
{
    uno::Sequence < PropertyValue > aPropSet (PKG_SIZE_NOENCR_MNFST);
    OUString sTempName = rPath + "/";

    if ( !GetMediaType().isEmpty() )
    {
        auto pPropSet = aPropSet.getArray();
        pPropSet[PKG_MNFST_MEDIATYPE].Name = "MediaType";
        pPropSet[PKG_MNFST_MEDIATYPE].Value <<= GetMediaType();
        pPropSet[PKG_MNFST_VERSION].Name = "Version";
        pPropSet[PKG_MNFST_VERSION].Value <<= GetVersion();
        pPropSet[PKG_MNFST_FULLPATH].Name = "FullPath";
        pPropSet[PKG_MNFST_FULLPATH].Value <<= sTempName;
    }
    else
        aPropSet.realloc( 0 );

    saveContents(sTempName, rManList, rZipOut, rEncryptionKey, oPBKDF2IterationCount, oArgon2Args, rRandomPool);

    // folder can have a mediatype only in package format
    if ( aPropSet.hasElements() && ( m_nFormat == embed::StorageFormats::PACKAGE ) )
        rManList.push_back( aPropSet );

    return true;
}

void ZipPackageFolder::saveContents(
        const OUString &rPath,
        std::vector < uno::Sequence < PropertyValue > > &rManList,
        ZipOutputStream & rZipOut,
        const uno::Sequence < sal_Int8 >& rEncryptionKey,
        ::std::optional<sal_Int32> const oPBKDF2IterationCount,
        ::std::optional<::std::tuple<sal_Int32, sal_Int32, sal_Int32>> const oArgon2Args,
        const rtlRandomPool &rRandomPool ) const
{
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
            throw uno::RuntimeException( THROW_WHERE );
        }
        catch ( IOException& )
        {
            throw uno::RuntimeException( THROW_WHERE );
        }
    }

    bool bMimeTypeStreamStored = false;
    OUString aMimeTypeStreamName(u"mimetype"_ustr);
    if ( m_nFormat == embed::StorageFormats::ZIP && rPath.isEmpty() )
    {
        // let the "mimetype" stream in root folder be stored as the first stream if it is zip format
        ContentHash::const_iterator aIter = maContents.find ( aMimeTypeStreamName );
        if ( aIter != maContents.end() && !(*aIter).second.bFolder )
        {
            bMimeTypeStreamStored = true;
            if (!aIter->second.pStream->saveChild(rPath + aIter->first, rManList, rZipOut,
                    rEncryptionKey, oPBKDF2IterationCount, oArgon2Args, rRandomPool))
            {
                throw uno::RuntimeException( THROW_WHERE );
            }
        }
    }

    for (const auto& [rShortName, rInfo] : maContents)
    {
        if ( !bMimeTypeStreamStored || rShortName != aMimeTypeStreamName )
        {
            if (rInfo.bFolder)
            {
                if (!rInfo.pFolder->saveChild(rPath + rShortName, rManList, rZipOut,
                        rEncryptionKey, oPBKDF2IterationCount, oArgon2Args, rRandomPool))
                {
                    throw uno::RuntimeException( THROW_WHERE );
                }
            }
            else
            {
                if (!rInfo.pStream->saveChild(rPath + rShortName, rManList, rZipOut,
                        rEncryptionKey, oPBKDF2IterationCount, oArgon2Args, rRandomPool))
                {
                    throw uno::RuntimeException( THROW_WHERE );
                }
            }
        }
    }
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
        throw UnknownPropertyException(aPropertyName);
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
{
    if ( PropertyName == "MediaType" )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException(THROW_WHERE );

        return uno::Any ( msMediaType );
    }
    else if ( PropertyName == "Version" )
        return uno::Any( m_sVersion );
    else if ( PropertyName == "Size" )
        return uno::Any ( aEntry.nSize );
    else
        throw UnknownPropertyException(PropertyName);
}

void ZipPackageFolder::doInsertByName ( ZipPackageEntry *pEntry, bool bSetParent )
{
    if ( pEntry->IsFolder() )
        maContents.emplace(pEntry->getName(), ZipContentInfo(static_cast<ZipPackageFolder*>(pEntry)));
    else
        maContents.emplace(pEntry->getName(), ZipContentInfo(static_cast<ZipPackageStream*>(pEntry)));
    if ( bSetParent )
        pEntry->setParent ( *this );
}

OUString ZipPackageFolder::getImplementationName()
{
    return u"ZipPackageFolder"_ustr;
}

uno::Sequence< OUString > ZipPackageFolder::getSupportedServiceNames()
{
    return { u"com.sun.star.packages.PackageFolder"_ustr };
}

sal_Bool SAL_CALL ZipPackageFolder::supportsService( OUString const & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}


ZipContentInfo::ZipContentInfo ( ZipPackageStream * pNewStream )
: xPackageEntry ( pNewStream )
, bFolder ( false )
, pStream ( pNewStream )
{
}

ZipContentInfo::ZipContentInfo ( ZipPackageFolder * pNewFolder )
: xPackageEntry ( pNewFolder )
, bFolder ( true )
, pFolder ( pNewFolder )
{
}

ZipContentInfo::ZipContentInfo( const ZipContentInfo& ) = default;
ZipContentInfo::ZipContentInfo( ZipContentInfo&& ) = default;
ZipContentInfo& ZipContentInfo::operator=( const ZipContentInfo& ) = default;
ZipContentInfo& ZipContentInfo::operator=( ZipContentInfo&& ) = default;

ZipContentInfo::~ZipContentInfo()
{
    if ( bFolder )
        pFolder->clearParent();
    else
        pStream->clearParent();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
