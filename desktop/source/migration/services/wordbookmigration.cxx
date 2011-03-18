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
#include "precompiled_desktop.hxx"
#include "wordbookmigration.hxx"
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


//.........................................................................
namespace migration
{
//.........................................................................


    static ::rtl::OUString sSourceSubDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/wordbook" ) );
    static ::rtl::OUString sTargetSubDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/wordbook" ) );
    static ::rtl::OUString sBaseName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/wordbook" ) );
    static ::rtl::OUString sSuffix = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dic" ) );


    // =============================================================================
    // component operations
    // =============================================================================

    ::rtl::OUString WordbookMigration_getImplementationName()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.desktop.migration.Wordbooks" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > WordbookMigration_getSupportedServiceNames()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.migration.Wordbooks" ) );
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

    TStringVectorPtr WordbookMigration::getFiles( const ::rtl::OUString& rBaseURL ) const
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
                ::osl::FileStatus aFileStatus( FileStatusMask_Type | FileStatusMask_FileURL );
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
bool IsUserWordbook( const ::rtl::OUString& rFile )
{
    static const sal_Char*      pVerStr2    = "WBSWG2";
    static const sal_Char*      pVerStr5    = "WBSWG5";
    static const sal_Char*      pVerStr6    = "WBSWG6";
    static const sal_Char*      pVerOOo7    = "OOoUserDict1";

    bool bRet = false;
    SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( String(rFile), STREAM_STD_READ );
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
        ::rtl::OUString sTargetDir;
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
                    ::rtl::OUString sSourceLocalName = aI->copy( m_sSourceDir.getLength() );
                    ::rtl::OUString sTargetName = sTargetDir + sSourceLocalName;
                    INetURLObject aURL( sTargetName );
                    aURL.removeSegment();
                    checkAndCreateDirectory( aURL );
                    ::osl::FileBase::RC aResult = ::osl::File::copy( *aI, sTargetName );
                    if ( aResult != ::osl::FileBase::E_None )
                    {
                        ::rtl::OString aMsg( "WordbookMigration::copyFiles: cannot copy " );
                        aMsg += ::rtl::OUStringToOString( *aI, RTL_TEXTENCODING_UTF8 ) + " to "
                             +  ::rtl::OUStringToOString( sTargetName, RTL_TEXTENCODING_UTF8 );
                        OSL_ENSURE( sal_False, aMsg.getStr() );
                    }
                }
                ++aI;
            }
        }
        else
        {
            OSL_ENSURE( sal_False, "WordbookMigration::copyFiles: no user installation!" );
        }
    }

    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString WordbookMigration::getImplementationName() throw (RuntimeException)
    {
        return WordbookMigration_getImplementationName();
    }

    // -----------------------------------------------------------------------------

    sal_Bool WordbookMigration::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > WordbookMigration::getSupportedServiceNames() throw (RuntimeException)
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
            if ( aValue.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "UserData" ) ) )
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
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new WordbookMigration() );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace migration
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
