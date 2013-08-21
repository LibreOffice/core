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

#include "updatecheckconfig.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/security.hxx>
#include <osl/time.h>
#include <osl/file.hxx>
#include <sal/macros.h>

#ifdef WNT
#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#pragma warning(disable: 4917)
#endif
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif

namespace container = com::sun::star::container ;
namespace beans = com::sun::star::beans ;
namespace lang = com::sun::star::lang ;
namespace util = com::sun::star::util ;
namespace uno = com::sun::star::uno ;

#define LAST_CHECK              "LastCheck"
#define UPDATE_VERSION          "UpdateVersion"
#define UPDATE_BUILDID          "UpdateBuildId"
#define UPDATE_DESCRIPTION      "UpdateDescription"
#define DOWNLOAD_URL            "DownloadURL"
#define IS_DIRECT_DOWNLOAD      "IsDirectDownload"
#define OLD_VERSION             "UpdateFoundFor"
#define AUTOCHECK_ENABLED       "AutoCheckEnabled"
#define AUTODOWNLOAD_ENABLED    "AutoDownloadEnabled"
#define CHECK_INTERVAL          "CheckInterval"
#define LOCAL_FILE              "LocalFile"
#define DOWNLOAD_SIZE           "DownloadSize"
#define DOWNLOAD_PAUSED         "DownloadPaused"
#define DOWNLOAD_DESTINATION    "DownloadDestination"
#define RELEASE_NOTE            "ReleaseNote"

#define PROPERTY_VERSION        "Version"

static const sal_Char * const aUpdateEntryProperties[] = {
    UPDATE_VERSION,
    UPDATE_BUILDID,
    UPDATE_DESCRIPTION,
    DOWNLOAD_URL,
    IS_DIRECT_DOWNLOAD,
    RELEASE_NOTE"1",
    RELEASE_NOTE"2",
    RELEASE_NOTE"3",
    RELEASE_NOTE"4",
    RELEASE_NOTE"5",
    OLD_VERSION
};

static const sal_uInt32 nUpdateEntryProperties = SAL_N_ELEMENTS(aUpdateEntryProperties);

//------------------------------------------------------------------------------

NamedValueByNameAccess::~NamedValueByNameAccess()
{
}

//------------------------------------------------------------------------------

::com::sun::star::uno::Any
NamedValueByNameAccess::getValue(const sal_Char * pName)
{
    const sal_Int32 nLen = m_rValues.getLength();
    for( sal_Int32 n=0; n < nLen; ++n )
    {
        if( m_rValues[n].Name.equalsAscii( pName ) )
            return m_rValues[n].Value;
    }
    return ::com::sun::star::uno::Any();
}


//------------------------------------------------------------------------------

bool
UpdateCheckROModel::isAutoCheckEnabled() const
{
    return sal_True == m_aNameAccess.getValue(AUTOCHECK_ENABLED).get< sal_Bool >();
}

//------------------------------------------------------------------------------

bool
UpdateCheckROModel::isDownloadPaused() const
{
    return sal_True == m_aNameAccess.getValue(DOWNLOAD_PAUSED).get< sal_Bool >();
}

//------------------------------------------------------------------------------

OUString
UpdateCheckROModel::getStringValue(const sal_Char * pStr) const
{
    uno::Any aAny( m_aNameAccess.getValue(pStr) );
    OUString aRet;

    aAny >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

OUString UpdateCheckROModel::getLocalFileName() const
{
    return getStringValue(LOCAL_FILE);
};

//------------------------------------------------------------------------------

sal_Int64 UpdateCheckROModel::getDownloadSize() const
{
    uno::Any aAny( m_aNameAccess.getValue(DOWNLOAD_SIZE) );
    sal_Int64 nRet = -1;

    aAny >>= nRet;
    return nRet;
};

//------------------------------------------------------------------------------

OUString
UpdateCheckROModel::getUpdateEntryVersion() const
{
    return getStringValue(OLD_VERSION);
}

//------------------------------------------------------------------------------

void
UpdateCheckROModel::getUpdateEntry(UpdateInfo& rInfo) const
{
    rInfo.BuildId = getStringValue(UPDATE_BUILDID);
    rInfo.Version = getStringValue(UPDATE_VERSION);
    rInfo.Description = getStringValue(UPDATE_DESCRIPTION);

    sal_Bool isDirectDownload = sal_False;
    m_aNameAccess.getValue(IS_DIRECT_DOWNLOAD) >>= isDirectDownload;

    rInfo.Sources.push_back( DownloadSource( isDirectDownload, getStringValue(DOWNLOAD_URL) ) );

    OString aStr(RELEASE_NOTE);
    for(sal_Int32 n=1; n < 6; ++n )
    {
        OUString aUStr = getStringValue(
            OString(aStr + OString::number(n)).getStr());
        if( !aUStr.isEmpty() )
            rInfo.ReleaseNotes.push_back(ReleaseNote((sal_Int8) n, aUStr));
    }
}


//------------------------------------------------------------------------------

OUString UpdateCheckConfig::getDesktopDirectory()
{
    OUString aRet;

#ifdef WNT
    WCHAR szPath[MAX_PATH];

    if( ! FAILED( SHGetSpecialFolderPathW( NULL, szPath, CSIDL_DESKTOPDIRECTORY, true ) ) )
    {
        aRet = OUString( reinterpret_cast< sal_Unicode * >(szPath) );
        osl::FileBase::getFileURLFromSystemPath( aRet, aRet );
    }
#else
    // This should become a desktop specific setting in some system backend ..
    OUString aHomeDir;
    osl::Security().getHomeDir( aHomeDir );
    aRet = aHomeDir + OUString("/Desktop");

    // Set path to home directory when there is no /Desktop directory
    osl::Directory aDocumentsDir( aRet );
    if( osl::FileBase::E_None != aDocumentsDir.open() )
        aRet = aHomeDir;
#endif

    return aRet;
}

//------------------------------------------------------------------------------

OUString UpdateCheckConfig::getAllUsersDirectory()
{
    OUString aRet;

#ifdef WNT
    WCHAR szPath[MAX_PATH];

    if( ! FAILED( SHGetSpecialFolderPathW( NULL, szPath, CSIDL_COMMON_DOCUMENTS, true ) ) )
    {
        aRet = OUString( reinterpret_cast< sal_Unicode * >(szPath) );
        osl::FileBase::getFileURLFromSystemPath( aRet, aRet );
    }
#else
    osl::FileBase::getTempDirURL(aRet);
#endif

    return aRet;
}

//------------------------------------------------------------------------------
UpdateCheckConfig::UpdateCheckConfig( const uno::Reference<container::XNameContainer>& xContainer,
                                      const uno::Reference<container::XNameContainer>& xAvailableUpdates,
                                      const uno::Reference<container::XNameContainer>& xIgnoredUpdates,
                                      const ::rtl::Reference< UpdateCheckConfigListener >& rListener ) :
    m_xContainer( xContainer ),
    m_xAvailableUpdates( xAvailableUpdates ),
    m_xIgnoredUpdates( xIgnoredUpdates ),
    m_rListener( rListener )
{}

//------------------------------------------------------------------------------
UpdateCheckConfig::~UpdateCheckConfig()
{}

//------------------------------------------------------------------------------

::rtl::Reference< UpdateCheckConfig >
UpdateCheckConfig::get(
    const uno::Reference<uno::XComponentContext>& xContext,
    const ::rtl::Reference< UpdateCheckConfigListener >& rListener)
{
    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
        com::sun::star::configuration::theDefaultProvider::get( xContext ) );

    beans::PropertyValue aProperty;
    aProperty.Name  = "nodepath";
    aProperty.Value = uno::makeAny( OUString("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameContainer > xContainer(
        xConfigProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationUpdateAccess", aArgumentList ),
        uno::UNO_QUERY_THROW );

    aProperty.Value = uno::makeAny( OUString("/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/IgnoredUpdates") );
    aArgumentList[0] = uno::makeAny( aProperty );
    uno::Reference< container::XNameContainer > xIgnoredExt( xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationUpdateAccess", aArgumentList ), uno::UNO_QUERY_THROW );

    aProperty.Value = uno::makeAny( OUString("/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/AvailableUpdates") );
    aArgumentList[0] = uno::makeAny( aProperty );
    uno::Reference< container::XNameContainer > xUpdateAvail( xConfigProvider->createInstanceWithArguments( "com.sun.star.configuration.ConfigurationUpdateAccess", aArgumentList ), uno::UNO_QUERY_THROW );

    return new UpdateCheckConfig( xContainer, xUpdateAvail, xIgnoredExt, rListener );
}

//------------------------------------------------------------------------------

bool
UpdateCheckConfig::isAutoCheckEnabled() const
{
    sal_Bool nValue = sal_False;
    const_cast < UpdateCheckConfig *> (this)->getByName( AUTOCHECK_ENABLED ) >>= nValue;
    return nValue;
}

//------------------------------------------------------------------------------

bool
UpdateCheckConfig::isAutoDownloadEnabled() const
{
    sal_Bool nValue = sal_False;
    const_cast < UpdateCheckConfig *> (this)->getByName( AUTODOWNLOAD_ENABLED ) >>= nValue;
    return nValue;
}

//------------------------------------------------------------------------------

OUString
UpdateCheckConfig::getUpdateEntryVersion() const
{
    OUString aValue;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( OLD_VERSION ) >>= aValue;

    return aValue;
}

//------------------------------------------------------------------------------

sal_Int64
UpdateCheckConfig::getLastChecked() const
{
    sal_Int64 nValue = 0;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( LAST_CHECK ) >>= nValue;

    return nValue;
}

//------------------------------------------------------------------------------

sal_Int64
UpdateCheckConfig::getCheckInterval() const
{
    sal_Int64 nValue = 0;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( CHECK_INTERVAL ) >>= nValue;

    return nValue;
}

//------------------------------------------------------------------------------

OUString
UpdateCheckConfig::getLocalFileName() const
{
    OUString aName = LOCAL_FILE;
    OUString aRet;

    if( m_xContainer->hasByName(aName) )
        m_xContainer->getByName(aName) >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

OUString
UpdateCheckConfig::getDownloadDestination() const
{
    OUString aName = DOWNLOAD_DESTINATION;
    OUString aRet;

    const_cast <UpdateCheckConfig *> (this)->getByName(aName) >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::storeLocalFileName(const OUString& rLocalFileName, sal_Int64 nFileSize)
{
    const sal_uInt8 nItems = 2;
    const OUString aNameList[nItems] = { OUString(LOCAL_FILE), OUString(DOWNLOAD_SIZE) };
    const uno::Any aValueList[nItems] = { uno::makeAny(rLocalFileName), uno::makeAny(nFileSize) };

    for( sal_uInt8 i=0; i < nItems; ++i )
    {
        if( m_xContainer->hasByName(aNameList[i]) )
            m_xContainer->replaceByName(aNameList[i], aValueList[i]);
        else
            m_xContainer->insertByName(aNameList[i], aValueList[i]);
    }

    commitChanges();
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::clearLocalFileName()
{
    const sal_uInt8 nItems = 2;
    const OUString aNameList[nItems] = { OUString(LOCAL_FILE), OUString(DOWNLOAD_SIZE) };

    for( sal_uInt8 i=0; i < nItems; ++i )
    {
        if( m_xContainer->hasByName(aNameList[i]) )
            m_xContainer->removeByName(aNameList[i]);
    }

    commitChanges();
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::storeDownloadPaused(bool paused)
{
    replaceByName(DOWNLOAD_PAUSED , uno::makeAny(paused));
    commitChanges();
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::updateLastChecked()
{
    TimeValue systime;
    osl_getSystemTime(&systime);

    sal_Int64 lastCheck = systime.Seconds;

    replaceByName(LAST_CHECK, uno::makeAny(lastCheck));
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::storeUpdateFound( const UpdateInfo& rInfo, const OUString& aCurrentBuild)

{
    bool autoDownloadEnabled = isAutoDownloadEnabled();

    uno::Any aValues[nUpdateEntryProperties] =
    {
        uno::makeAny(rInfo.Version),
        uno::makeAny(rInfo.BuildId),
        uno::makeAny(rInfo.Description),
        uno::makeAny(rInfo.Sources[0].URL),
        uno::makeAny(rInfo.Sources[0].IsDirect ? sal_True : sal_False),
        uno::makeAny(getReleaseNote(rInfo, 1, autoDownloadEnabled) ),
        uno::makeAny(getReleaseNote(rInfo, 2, autoDownloadEnabled) ),
        uno::makeAny(getReleaseNote(rInfo, 3, autoDownloadEnabled) ),
        uno::makeAny(getReleaseNote(rInfo, 4, autoDownloadEnabled) ),
        uno::makeAny(getReleaseNote(rInfo, 5, autoDownloadEnabled) ),
        uno::makeAny(aCurrentBuild)
    };

    OUString aName;
    for( sal_uInt32 n=0; n < nUpdateEntryProperties; ++n )
    {
        aName = OUString::createFromAscii(aUpdateEntryProperties[n]);

        if( m_xContainer->hasByName(aName) )
            m_xContainer->replaceByName(aName, aValues[n]);
        else
            m_xContainer->insertByName(aName,aValues[n]);
    }

    commitChanges();
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::clearUpdateFound()
{
    OUString aName;

    for( sal_uInt32 n=0; n < nUpdateEntryProperties; ++n )
    {
        aName = OUString::createFromAscii(aUpdateEntryProperties[n]);

        try {
            if( m_xContainer->hasByName(aName) )
                m_xContainer->removeByName(aName);
        } catch(const lang::WrappedTargetException& ) {
            // Can not remove value, probably in share layer
            OSL_ASSERT(false);
            m_xContainer->replaceByName(aName, uno::makeAny(OUString()));
        }
    }

    /* As we have removed UpdateVersionFound from the shared configuration
     * existing entries in the user layer do not have a oor operation and
     * thus are completely ignored (which also means they can not be removed).
     */

    commitChanges();
}

//------------------------------------------------------------------------------

uno::Sequence< OUString >
UpdateCheckConfig::getServiceNames()
{
    uno::Sequence< OUString > aServiceList(1);
    aServiceList[0] = "com.sun.star.setup.UpdateCheckConfig";
    return aServiceList;
}

//------------------------------------------------------------------------------

OUString
UpdateCheckConfig::getImplName()
{
    return OUString("vnd.sun.UpdateCheckConfig");
}

//------------------------------------------------------------------------------

uno::Type SAL_CALL
UpdateCheckConfig::getElementType() throw (uno::RuntimeException)
{
    return m_xContainer->getElementType();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasElements() throw (uno::RuntimeException)
{
    return m_xContainer->hasElements();
}

//------------------------------------------------------------------------------

uno::Any SAL_CALL
UpdateCheckConfig::getByName( const OUString& aName )
    throw (container::NoSuchElementException, lang::WrappedTargetException,  uno::RuntimeException)
{
    uno::Any aValue = m_xContainer->getByName( aName );

    // Provide dynamic default value
    if( aName.equalsAscii(DOWNLOAD_DESTINATION) )
    {
        OUString aStr;
        aValue >>= aStr;

        if( aStr.isEmpty() )
            aValue = uno::makeAny(getDesktopDirectory());
    }

    return aValue;
}

//------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL
UpdateCheckConfig::getElementNames(  ) throw (uno::RuntimeException)
{
    return m_xContainer->getElementNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasByName( const OUString& aName ) throw (uno::RuntimeException)
{
    return m_xContainer->hasByName( aName );
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateCheckConfig::replaceByName( const OUString& aName, const uno::Any& aElement )
    throw (lang::IllegalArgumentException, container::NoSuchElementException,
           lang::WrappedTargetException, uno::RuntimeException)
{
    return m_xContainer->replaceByName( aName, aElement );
}

//------------------------------------------------------------------------------
// XChangesBatch

void SAL_CALL
UpdateCheckConfig::commitChanges()
    throw (lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xContainer, uno::UNO_QUERY);
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
    {
        util::ChangesSet aChangesSet = xChangesBatch->getPendingChanges();
        xChangesBatch->commitChanges();

        if( m_rListener.is() )
        {
            const sal_Int32 nChanges = aChangesSet.getLength();
            OUString aString;

            for( sal_Int32 i=0; i<nChanges; ++i )
            {
                aChangesSet[i].Accessor >>= aString;

                // FIXME: use non IgnoreAsciiCase version as soon as it becomes available
                if( aString.endsWithIgnoreAsciiCaseAsciiL(AUTOCHECK_ENABLED "']", sizeof(AUTOCHECK_ENABLED)+1) )
                {
                    sal_Bool bEnabled = sal_False;
                    aChangesSet[i].Element >>= bEnabled;
                    m_rListener->autoCheckStatusChanged(sal_True == bEnabled);
                }
                // FIXME: use non IgnoreAsciiCase version as soon as it becomes available
                else if( aString.endsWithIgnoreAsciiCaseAsciiL(CHECK_INTERVAL "']", sizeof(CHECK_INTERVAL)+1) )
                {
                    m_rListener->autoCheckIntervalChanged();
                }
            }
        }
    }

    xChangesBatch = uno::Reference< util::XChangesBatch > ( m_xAvailableUpdates, uno::UNO_QUERY );
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
    {
        util::ChangesSet aChangesSet = xChangesBatch->getPendingChanges();
        xChangesBatch->commitChanges();
    }
    xChangesBatch = uno::Reference< util::XChangesBatch > ( m_xIgnoredUpdates, uno::UNO_QUERY );
    if( xChangesBatch.is() && xChangesBatch->hasPendingChanges() )
    {
        util::ChangesSet aChangesSet = xChangesBatch->getPendingChanges();
        xChangesBatch->commitChanges();
    }
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasPendingChanges(  ) throw (uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xContainer, uno::UNO_QUERY);
    if( xChangesBatch.is() )
        return xChangesBatch->hasPendingChanges();

    return sal_False;
}

//------------------------------------------------------------------------------

uno::Sequence< util::ElementChange > SAL_CALL
UpdateCheckConfig::getPendingChanges(  ) throw (uno::RuntimeException)
{
    uno::Reference< util::XChangesBatch > xChangesBatch(m_xContainer, uno::UNO_QUERY);
    if( xChangesBatch.is() )
        return xChangesBatch->getPendingChanges();

    return uno::Sequence< util::ElementChange >();
}

//------------------------------------------------------------------------------
bool UpdateCheckConfig::storeExtensionVersion( const OUString& rExtensionName,
                                               const OUString& rVersion )
{
    bool bNotify = true;

    if ( m_xAvailableUpdates->hasByName( rExtensionName ) )
        uno::Reference< beans::XPropertySet >( m_xAvailableUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->setPropertyValue( PROPERTY_VERSION, uno::Any( rVersion ) );
    else
    {
        uno::Reference< beans::XPropertySet > elem( uno::Reference< lang::XSingleServiceFactory >( m_xAvailableUpdates, uno::UNO_QUERY_THROW )->createInstance(), uno::UNO_QUERY_THROW );
        elem->setPropertyValue( PROPERTY_VERSION, uno::Any( rVersion ) );
        m_xAvailableUpdates->insertByName( rExtensionName, uno::Any( elem ) );
    }

    if ( m_xIgnoredUpdates->hasByName( rExtensionName ) )
    {
        OUString aIgnoredVersion;
        uno::Any aValue( uno::Reference< beans::XPropertySet >( m_xIgnoredUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aValue >>= aIgnoredVersion;
        if ( aIgnoredVersion.isEmpty() ) // no version means ignore all updates
            bNotify = false;
        else if ( aIgnoredVersion == rVersion ) // the user wanted to ignore this update
            bNotify = false;
    }

    commitChanges();

    return bNotify;
}

//------------------------------------------------------------------------------
bool UpdateCheckConfig::checkExtensionVersion( const OUString& rExtensionName,
                                               const OUString& rVersion )
{
    if ( m_xAvailableUpdates->hasByName( rExtensionName ) )
    {
        OUString aStoredVersion;
        uno::Any aValue( uno::Reference< beans::XPropertySet >( m_xAvailableUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aValue >>= aStoredVersion;

        if ( m_xIgnoredUpdates->hasByName( rExtensionName ) )
        {
            OUString aIgnoredVersion;
            uno::Any aValue2( uno::Reference< beans::XPropertySet >( m_xIgnoredUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
            aValue2 >>= aIgnoredVersion;
            if ( aIgnoredVersion.isEmpty() ) // no version means ignore all updates
                return false;
            else if ( aIgnoredVersion == aStoredVersion ) // the user wanted to ignore this update
                return false;
            // TODO: else delete ignored entry?
        }
        if ( isVersionGreater( rVersion, aStoredVersion ) )
            return true;
        else
        {
            m_xAvailableUpdates->removeByName( rExtensionName );
            commitChanges();
        }
    }

    return false;
}

//------------------------------------------------------------------------------
OUString UpdateCheckConfig::getSubVersion( const OUString& rVersion,
                                                sal_Int32 *nIndex )
{
    while ( *nIndex < rVersion.getLength() && rVersion[*nIndex] == '0')
    {
        ++*nIndex;
    }

    return rVersion.getToken( 0, '.', *nIndex );
}

//------------------------------------------------------------------------------
// checks if the second version string is greater than the first one

bool UpdateCheckConfig::isVersionGreater( const OUString& rVersion1,
                                          const OUString& rVersion2 )
{
    for ( sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0; )
    {
        OUString sSub1( getSubVersion( rVersion1, &i1 ) );
        OUString sSub2( getSubVersion( rVersion2, &i2 ) );

        if ( sSub1.getLength() < sSub2.getLength() ) {
            return true;
        } else if ( sSub1.getLength() > sSub2.getLength() ) {
            return false;
        } else if ( sSub1 < sSub2 ) {
            return true;
        } else if ( sSub1 > sSub2 ) {
            return false;
        }
    }
    return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

OUString SAL_CALL
UpdateCheckConfig::getImplementationName()  throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::supportsService(OUString const & serviceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL
UpdateCheckConfig::getSupportedServiceNames()  throw (uno::RuntimeException)
{
    return getServiceNames();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
