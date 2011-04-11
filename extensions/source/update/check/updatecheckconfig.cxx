/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_extensions.hxx"


#include "updatecheckconfig.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
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

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

#define LAST_CHECK              "LastCheck"
#define VERSION_FOUND           "UpdateVersionFound"
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
#define EXTENSION_PREFIX        "Extension_"

#define PROPERTY_VERSION        UNISTRING("Version")

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

rtl::OUString
UpdateCheckROModel::getStringValue(const sal_Char * pStr) const
{
    uno::Any aAny( m_aNameAccess.getValue(pStr) );
    rtl::OUString aRet;

    aAny >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

rtl::OUString UpdateCheckROModel::getLocalFileName() const
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

rtl::OUString
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

    rtl::OString aStr(RELEASE_NOTE);
    for(sal_Int32 n=1; n < 6; ++n )
    {
        rtl::OUString aUStr = getStringValue(aStr + rtl::OString::valueOf(n));
        if( aUStr.getLength() > 0 )
            rInfo.ReleaseNotes.push_back(ReleaseNote((sal_Int8) n, aUStr));
    }
}


//------------------------------------------------------------------------------

rtl::OUString UpdateCheckConfig::getDesktopDirectory()
{
    rtl::OUString aRet;

#ifdef WNT
    WCHAR szPath[MAX_PATH];

    if( ! FAILED( SHGetSpecialFolderPathW( NULL, szPath, CSIDL_DESKTOPDIRECTORY, true ) ) )
    {
        aRet = rtl::OUString( reinterpret_cast< sal_Unicode * >(szPath) );
        osl::FileBase::getFileURLFromSystemPath( aRet, aRet );
    }
#else
    // This should become a desktop specific setting in some system backend ..
    rtl::OUString aHomeDir;
    osl::Security().getHomeDir( aHomeDir );
    aRet = aHomeDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/Desktop"));

    // Set path to home directory when there is no /Desktop directory
    osl::Directory aDocumentsDir( aRet );
    if( osl::FileBase::E_None != aDocumentsDir.open() )
        aRet = aHomeDir;
#endif

    return aRet;
}

//------------------------------------------------------------------------------

rtl::OUString UpdateCheckConfig::getAllUsersDirectory()
{
    rtl::OUString aRet;

#ifdef WNT
    WCHAR szPath[MAX_PATH];

    if( ! FAILED( SHGetSpecialFolderPathW( NULL, szPath, CSIDL_COMMON_DOCUMENTS, true ) ) )
    {
        aRet = rtl::OUString( reinterpret_cast< sal_Unicode * >(szPath) );
        osl::FileBase::RC rc;
        rc = osl::FileBase::getFileURLFromSystemPath( aRet, aRet );
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
    if( !xContext.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckConfig: empty component context" ),
            uno::Reference< uno::XInterface >() );

    uno::Reference< lang::XMultiComponentFactory > xServiceManager(xContext->getServiceManager());

    if( !xServiceManager.is() )
        throw uno::RuntimeException(
            UNISTRING( "UpdateCheckConfig: unable to obtain service manager from component context" ),
            uno::Reference< uno::XInterface >() );

    uno::Reference< lang::XMultiServiceFactory > xConfigProvider(
        xServiceManager->createInstanceWithContext( UNISTRING( "com.sun.star.configuration.ConfigurationProvider" ), xContext ),
        uno::UNO_QUERY_THROW);

    beans::PropertyValue aProperty;
    aProperty.Name  = UNISTRING( "nodepath" );
    aProperty.Value = uno::makeAny( UNISTRING("org.openoffice.Office.Jobs/Jobs/UpdateCheck/Arguments") );

    uno::Sequence< uno::Any > aArgumentList( 1 );
    aArgumentList[0] = uno::makeAny( aProperty );

    uno::Reference< container::XNameContainer > xContainer(
        xConfigProvider->createInstanceWithArguments(
            UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess"), aArgumentList ),
        uno::UNO_QUERY_THROW );

    aProperty.Value = uno::makeAny( UNISTRING("/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/IgnoredUpdates") );
    aArgumentList[0] = uno::makeAny( aProperty );
    uno::Reference< container::XNameContainer > xIgnoredExt( xConfigProvider->createInstanceWithArguments( UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess"), aArgumentList ), uno::UNO_QUERY_THROW );

    aProperty.Value = uno::makeAny( UNISTRING("/org.openoffice.Office.ExtensionManager/ExtensionUpdateData/AvailableUpdates") );
    aArgumentList[0] = uno::makeAny( aProperty );
    uno::Reference< container::XNameContainer > xUpdateAvail( xConfigProvider->createInstanceWithArguments( UNISTRING("com.sun.star.configuration.ConfigurationUpdateAccess"), aArgumentList ), uno::UNO_QUERY_THROW );

    return new UpdateCheckConfig( xContainer, xUpdateAvail, xIgnoredExt, rListener );
}

//------------------------------------------------------------------------------

bool
UpdateCheckConfig::isAutoCheckEnabled() const
{
    sal_Bool nValue = sal_False;
    const_cast < UpdateCheckConfig *> (this)->getByName( UNISTRING( AUTOCHECK_ENABLED ) ) >>= nValue;
    return nValue;
}

//------------------------------------------------------------------------------

bool
UpdateCheckConfig::isAutoDownloadEnabled() const
{
    sal_Bool nValue = sal_False;
    const_cast < UpdateCheckConfig *> (this)->getByName( UNISTRING( AUTODOWNLOAD_ENABLED ) ) >>= nValue;
    return nValue;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckConfig::getUpdateEntryVersion() const
{
    rtl::OUString aValue;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( UNISTRING( OLD_VERSION ) ) >>= aValue;

    return aValue;
}

//------------------------------------------------------------------------------

sal_Int64
UpdateCheckConfig::getLastChecked() const
{
    sal_Int64 nValue = 0;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( UNISTRING( LAST_CHECK ) ) >>= nValue;

    return nValue;
}

//------------------------------------------------------------------------------

sal_Int64
UpdateCheckConfig::getCheckInterval() const
{
    sal_Int64 nValue = 0;

    // getByName is defined as non const in XNameAccess
    const_cast < UpdateCheckConfig *> (this)->getByName( UNISTRING( CHECK_INTERVAL ) ) >>= nValue;

    return nValue;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckConfig::getLocalFileName() const
{
    rtl::OUString aName = UNISTRING(LOCAL_FILE);
    rtl::OUString aRet;

    if( m_xContainer->hasByName(aName) )
        m_xContainer->getByName(aName) >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckConfig::getDownloadDestination() const
{
    rtl::OUString aName = UNISTRING(DOWNLOAD_DESTINATION);
    rtl::OUString aRet;

    const_cast <UpdateCheckConfig *> (this)->getByName(aName) >>= aRet;

    return aRet;
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::storeLocalFileName(const rtl::OUString& rLocalFileName, sal_Int64 nFileSize)
{
    const sal_uInt8 nItems = 2;
    const rtl::OUString aNameList[nItems] = { UNISTRING(LOCAL_FILE), UNISTRING(DOWNLOAD_SIZE) };
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
    const rtl::OUString aNameList[nItems] = { UNISTRING(LOCAL_FILE), UNISTRING(DOWNLOAD_SIZE) };

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
    replaceByName(UNISTRING(DOWNLOAD_PAUSED) , uno::makeAny(paused));
    commitChanges();
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::updateLastChecked()
{
    TimeValue systime;
    osl_getSystemTime(&systime);

    sal_Int64 lastCheck = systime.Seconds;

    replaceByName(UNISTRING(LAST_CHECK), uno::makeAny(lastCheck));
}

//------------------------------------------------------------------------------

void
UpdateCheckConfig::storeUpdateFound( const UpdateInfo& rInfo, const rtl::OUString& aCurrentBuild)

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

    rtl::OUString aName;
    for( sal_uInt32 n=0; n < nUpdateEntryProperties; ++n )
    {
        aName = rtl::OUString::createFromAscii(aUpdateEntryProperties[n]);

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
    rtl::OUString aName;

    for( sal_uInt32 n=0; n < nUpdateEntryProperties; ++n )
    {
        aName = rtl::OUString::createFromAscii(aUpdateEntryProperties[n]);

        try {
            if( m_xContainer->hasByName(aName) )
                m_xContainer->removeByName(aName);
        } catch(const lang::WrappedTargetException& ) {
            // Can not remove value, probably in share layer
            OSL_ASSERT(false);
            m_xContainer->replaceByName(aName, uno::makeAny(rtl::OUString()));
        }
    }

    /* As we have removed UpdateVersionFound from the shared configuration
     * existing entries in the user layer do not have a oor operation and
     * thus are completly ignored (which also means they can not be removed).
     */

    commitChanges();
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString >
UpdateCheckConfig::getServiceNames()
{
    uno::Sequence< rtl::OUString > aServiceList(1);
    aServiceList[0] = UNISTRING( "com.sun.star.setup.UpdateCheckConfig");
    return aServiceList;
}

//------------------------------------------------------------------------------

rtl::OUString
UpdateCheckConfig::getImplName()
{
    return UNISTRING( "vnd.sun.UpdateCheckConfig");
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
UpdateCheckConfig::getByName( const ::rtl::OUString& aName )
    throw (container::NoSuchElementException, lang::WrappedTargetException,  uno::RuntimeException)
{
    uno::Any aValue = m_xContainer->getByName( aName );

    // Provide dynamic default value
    if( aName.equalsAscii(DOWNLOAD_DESTINATION) )
    {
        rtl::OUString aStr;
        aValue >>= aStr;

        if( aStr.getLength() == 0 )
            aValue = uno::makeAny(getDesktopDirectory());
    }

    return aValue;
}

//------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL
UpdateCheckConfig::getElementNames(  ) throw (uno::RuntimeException)
{
    return m_xContainer->getElementNames();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::hasByName( const ::rtl::OUString& aName ) throw (uno::RuntimeException)
{
    return m_xContainer->hasByName( aName );
}

//------------------------------------------------------------------------------

void SAL_CALL
UpdateCheckConfig::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
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
            rtl::OUString aString;

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
bool UpdateCheckConfig::storeExtensionVersion( const rtl::OUString& rExtensionName,
                                               const rtl::OUString& rVersion )
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
        ::rtl::OUString aIgnoredVersion;
        uno::Any aValue( uno::Reference< beans::XPropertySet >( m_xIgnoredUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aValue >>= aIgnoredVersion;
        if ( aIgnoredVersion.getLength() == 0 ) // no version means ignore all updates
            bNotify = false;
        else if ( aIgnoredVersion == rVersion ) // the user wanted to ignore this update
            bNotify = false;
    }

    commitChanges();

    return bNotify;
}

//------------------------------------------------------------------------------
bool UpdateCheckConfig::checkExtensionVersion( const rtl::OUString& rExtensionName,
                                               const rtl::OUString& rVersion )
{
    if ( m_xAvailableUpdates->hasByName( rExtensionName ) )
    {
        ::rtl::OUString aStoredVersion;
        uno::Any aValue( uno::Reference< beans::XPropertySet >( m_xAvailableUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
        aValue >>= aStoredVersion;

        if ( m_xIgnoredUpdates->hasByName( rExtensionName ) )
        {
            ::rtl::OUString aIgnoredVersion;
            uno::Any aValue2( uno::Reference< beans::XPropertySet >( m_xIgnoredUpdates->getByName( rExtensionName ), uno::UNO_QUERY_THROW )->getPropertyValue( PROPERTY_VERSION ) );
            aValue2 >>= aIgnoredVersion;
            if ( aIgnoredVersion.getLength() == 0 ) // no version means ignore all updates
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
rtl::OUString UpdateCheckConfig::getSubVersion( const rtl::OUString& rVersion,
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

bool UpdateCheckConfig::isVersionGreater( const rtl::OUString& rVersion1,
                                          const rtl::OUString& rVersion2 )
{
    for ( sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0; )
    {
        ::rtl::OUString sSub1( getSubVersion( rVersion1, &i1 ) );
        ::rtl::OUString sSub2( getSubVersion( rVersion2, &i2 ) );

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

rtl::OUString SAL_CALL
UpdateCheckConfig::getImplementationName()  throw (uno::RuntimeException)
{
    return getImplName();
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL
UpdateCheckConfig::supportsService(rtl::OUString const & serviceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > aServiceNameList = getServiceNames();

    for( sal_Int32 n=0; n < aServiceNameList.getLength(); n++ )
        if( aServiceNameList[n].equals(serviceName) )
            return sal_True;

    return sal_False;
}

//------------------------------------------------------------------------------

uno::Sequence< rtl::OUString > SAL_CALL
UpdateCheckConfig::getSupportedServiceNames()  throw (uno::RuntimeException)
{
    return getServiceNames();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
