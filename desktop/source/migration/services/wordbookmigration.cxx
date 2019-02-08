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
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace migration
{
    // component operations


    OUString WordbookMigration_getImplementationName()
    {
        return OUString( "com.sun.star.comp.desktop.migration.Wordbooks" );
    }


    Sequence< OUString > WordbookMigration_getSupportedServiceNames()
    {
        return { "com.sun.star.migration.Wordbooks" };
    }


    // WordbookMigration


    WordbookMigration::WordbookMigration()
    {
    }


    WordbookMigration::~WordbookMigration()
    {
    }


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
            for (auto const& subDir : aSubDirs)
            {
                TStringVectorPtr aSubResult = getFiles(subDir);
                aResult->insert( aResult->end(), aSubResult->begin(), aSubResult->end() );
            }
        }

        return aResult;
    }


    void WordbookMigration::checkAndCreateDirectory( INetURLObject const & rDirURL )
    {
        ::osl::FileBase::RC aResult = ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
        if ( aResult == ::osl::FileBase::E_NOENT )
        {
            INetURLObject aBaseURL( rDirURL );
            aBaseURL.removeSegment();
            checkAndCreateDirectory( aBaseURL );
            ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
        }
    }

#define MAX_HEADER_LENGTH 16
static bool IsUserWordbook( const OUString& rFile )
{
    bool bRet = false;
    std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( rFile, StreamMode::STD_READ );
    if ( pStream && !pStream->GetError() )
    {
        static const sal_Char* const pVerOOo7    = "OOoUserDict1";
        sal_uInt64 const nSniffPos = pStream->Tell();
        static std::size_t nVerOOo7Len = sal::static_int_cast< std::size_t >(strlen( pVerOOo7 ));
        sal_Char pMagicHeader[MAX_HEADER_LENGTH];
        pMagicHeader[ nVerOOo7Len ] = '\0';
        if (pStream->ReadBytes(static_cast<void *>(pMagicHeader), nVerOOo7Len) == nVerOOo7Len)
        {
            if ( !strcmp(pMagicHeader, pVerOOo7) )
                bRet = true;
            else
            {
                sal_uInt16 nLen;
                pStream->Seek (nSniffPos);
                pStream->ReadUInt16( nLen );
                if ( nLen < MAX_HEADER_LENGTH )
                {
                    pStream->ReadBytes(pMagicHeader, nLen);
                    pMagicHeader[nLen] = '\0';
                    if ( !strcmp(pMagicHeader, "WBSWG2")
                     ||  !strcmp(pMagicHeader, "WBSWG5")
                     ||  !strcmp(pMagicHeader, "WBSWG6") )
                    bRet = true;
                }
            }
        }
    }

    return bRet;
}


    void WordbookMigration::copyFiles()
    {
        OUString sTargetDir;
        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            sTargetDir += "/user/wordbook";
            TStringVectorPtr aFileList = getFiles( m_sSourceDir );
            for (auto const& elem : *aFileList)
            {
                if (IsUserWordbook(elem) )
                {
                    OUString sSourceLocalName = elem.copy( m_sSourceDir.getLength() );
                    OUString sTargetName = sTargetDir + sSourceLocalName;
                    INetURLObject aURL( sTargetName );
                    aURL.removeSegment();
                    checkAndCreateDirectory( aURL );
                    ::osl::FileBase::RC aResult = ::osl::File::copy( elem, sTargetName );
                    if ( aResult != ::osl::FileBase::E_None )
                    {
                        SAL_WARN( "desktop", "WordbookMigration::copyFiles: cannot copy "
                                     << elem << " to " << sTargetName);
                    }
                }
            }
        }
        else
        {
            OSL_FAIL( "WordbookMigration::copyFiles: no user installation!" );
        }
    }


    // XServiceInfo


    OUString WordbookMigration::getImplementationName()
    {
        return WordbookMigration_getImplementationName();
    }


    sal_Bool WordbookMigration::supportsService(OUString const & ServiceName)
    {
        return cppu::supportsService(this, ServiceName);
    }


    Sequence< OUString > WordbookMigration::getSupportedServiceNames()
    {
        return WordbookMigration_getSupportedServiceNames();
    }


    // XInitialization


    void WordbookMigration::initialize( const Sequence< Any >& aArguments )
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
                m_sSourceDir += "/user/wordbook";
                break;
            }
        }
    }


    // XJob


    Any WordbookMigration::execute( const Sequence< beans::NamedValue >& )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        copyFiles();

        return Any();
    }


    // component operations


    Reference< XInterface > WordbookMigration_create(
        Reference< XComponentContext > const & )
    {
        return static_cast< lang::XTypeProvider * >( new WordbookMigration() );
    }


}   // namespace migration


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
