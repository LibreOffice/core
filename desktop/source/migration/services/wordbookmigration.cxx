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

#include "wordbookmigration.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace migration
{
//.........................................................................


    static OUString sSourceSubDir( "/user/wordbook" );
    static OUString sTargetSubDir( "/user/wordbook" );


    // =============================================================================
    // component operations
    // =============================================================================

    OUString WordbookMigration_getImplementationName()
    {
        static OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static OUString aImplName( "com.sun.star.comp.desktop.migration.Wordbooks" );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< OUString > WordbookMigration_getSupportedServiceNames()
    {
        static Sequence< OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< OUString > aNames(1);
                aNames.getArray()[0] = OUString( "com.sun.star.migration.Wordbooks" );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // =============================================================================
    // WordbookMigration
    // =============================================================================

    WordbookMigration::WordbookMigration()
    {
    }

    // -----------------------------------------------------------------------------

    WordbookMigration::~WordbookMigration()
    {
    }

    // -----------------------------------------------------------------------------

    TStringVectorPtr WordbookMigration::getFiles( const OUString& rBaseURL ) const
    {
        TStringVectorPtr aResult( new TStringVector );
        ::osl::Directory aDir( rBaseURL);

        if ( aDir.open() == ::osl::FileBase::E_None )
        {
            // iterate over directory content
            TStringVector aSubDirs;
            ::osl::DirectoryItem aItem;
            while ( aDir.getNextItem( aItem ) == ::osl::FileBase::E_None )
            {
                ::osl::FileStatus aFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL );
                if ( aItem.getFileStatus( aFileStatus ) == ::osl::FileBase::E_None )
                {
                    if ( aFileStatus.getFileType() == ::osl::FileStatus::Directory )
                        aSubDirs.push_back( aFileStatus.getFileURL() );
                    else
                        aResult->push_back( aFileStatus.getFileURL() );
                }
            }

            // iterate recursive over subfolders
            TStringVector::const_iterator aI = aSubDirs.begin();
            while ( aI != aSubDirs.end() )
            {
                TStringVectorPtr aSubResult = getFiles( *aI );
                aResult->insert( aResult->end(), aSubResult->begin(), aSubResult->end() );
                ++aI;
            }
        }

        return aResult;
    }

    // -----------------------------------------------------------------------------

    ::osl::FileBase::RC WordbookMigration::checkAndCreateDirectory( INetURLObject& rDirURL )
    {
        ::osl::FileBase::RC aResult = ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        if ( aResult == ::osl::FileBase::E_NOENT )
        {
            INetURLObject aBaseURL( rDirURL );
            aBaseURL.removeSegment();
            checkAndCreateDirectory( aBaseURL );
            return ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        }
        else
        {
            return aResult;
        }
    }

#define MAX_HEADER_LENGTH 16
bool IsUserWordbook( const OUString& rFile )
{
    static const sal_Char*      pVerStr2    = "WBSWG2";
    static const sal_Char*      pVerStr5    = "WBSWG5";
    static const sal_Char*      pVerStr6    = "WBSWG6";
    static const sal_Char*      pVerOOo7    = "OOoUserDict1";

    bool bRet = false;
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( OUString(rFile), STREAM_STD_READ );
    if ( pStream && !pStream->GetError() )
    {
        sal_Size nSniffPos = pStream->Tell();
        static sal_Size nVerOOo7Len = sal::static_int_cast< sal_Size >(strlen( pVerOOo7 ));
        sal_Char pMagicHeader[MAX_HEADER_LENGTH];
        pMagicHeader[ nVerOOo7Len ] = '\0';
        if ((pStream->Read((void *) pMagicHeader, nVerOOo7Len) == nVerOOo7Len))
        {
            if ( !strcmp(pMagicHeader, pVerOOo7) )
                bRet = true;
            else
            {
                sal_uInt16 nLen;
                pStream->Seek (nSniffPos);
                *pStream >> nLen;
                if ( nLen < MAX_HEADER_LENGTH )
                {
                   pStream->Read(pMagicHeader, nLen);
                   pMagicHeader[nLen] = '\0';
                    if ( !strcmp(pMagicHeader, pVerStr2)
                     ||  !strcmp(pMagicHeader, pVerStr5)
                     ||  !strcmp(pMagicHeader, pVerStr6) )
                    bRet = true;
                }
            }
        }
    }

    delete pStream;
    return bRet;
}


    // -----------------------------------------------------------------------------

    void WordbookMigration::copyFiles()
    {
        OUString sTargetDir;
        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            sTargetDir += sTargetSubDir;
            TStringVectorPtr aFileList = getFiles( m_sSourceDir );
            TStringVector::const_iterator aI = aFileList->begin();
            while ( aI != aFileList->end() )
            {
                if (IsUserWordbook(*aI) )
                {
                    OUString sSourceLocalName = aI->copy( m_sSourceDir.getLength() );
                    OUString sTargetName = sTargetDir + sSourceLocalName;
                    INetURLObject aURL( sTargetName );
                    aURL.removeSegment();
                    checkAndCreateDirectory( aURL );
                    ::osl::FileBase::RC aResult = ::osl::File::copy( *aI, sTargetName );
                    if ( aResult != ::osl::FileBase::E_None )
                    {
                        OString aMsg( "WordbookMigration::copyFiles: cannot copy " );
                        aMsg += OUStringToOString( *aI, RTL_TEXTENCODING_UTF8 ) + " to "
                             +  OUStringToOString( sTargetName, RTL_TEXTENCODING_UTF8 );
                        OSL_FAIL( aMsg.getStr() );
                    }
                }
                ++aI;
            }
        }
        else
        {
            OSL_FAIL( "WordbookMigration::copyFiles: no user installation!" );
        }
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    OUString WordbookMigration::getImplementationName() throw (RuntimeException)
    {
        return WordbookMigration_getImplementationName();
    }

    // -----------------------------------------------------------------------------

    sal_Bool WordbookMigration::supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    // -----------------------------------------------------------------------------

    Sequence< OUString > WordbookMigration::getSupportedServiceNames() throw (RuntimeException)
    {
        return WordbookMigration_getSupportedServiceNames();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void WordbookMigration::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const Any* pIter = aArguments.getConstArray();
        const Any* pEnd = pIter + aArguments.getLength();
        for ( ; pIter != pEnd ; ++pIter )
        {
            beans::NamedValue aValue;
            *pIter >>= aValue;
            if ( aValue.Name == "UserData" )
            {
                if ( !(aValue.Value >>= m_sSourceDir) )
                {
                    OSL_FAIL( "WordbookMigration::initialize: argument UserData has wrong type!" );
                }
                m_sSourceDir += sSourceSubDir;
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // XJob
    // -----------------------------------------------------------------------------

    Any WordbookMigration::execute( const Sequence< beans::NamedValue >& )
        throw (lang::IllegalArgumentException, Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        copyFiles();

        return Any();
    }

    // =============================================================================
    // component operations
    // =============================================================================

    Reference< XInterface > SAL_CALL WordbookMigration_create(
        Reference< XComponentContext > const & )
        SAL_THROW(())
    {
        return static_cast< lang::XTypeProvider * >( new WordbookMigration() );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace migration
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
