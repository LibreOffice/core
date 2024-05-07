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

#include "basicmigration.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace migration
{


    #define sSourceUserBasic "/user/basic"
    #define sTargetUserBasic "/user/__basic_80"


    // BasicMigration


    BasicMigration::BasicMigration()
    {
    }


    BasicMigration::~BasicMigration()
    {
    }


    TStringVectorPtr BasicMigration::getFiles( const OUString& rBaseURL ) const
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


    void BasicMigration::checkAndCreateDirectory( INetURLObject const & rDirURL )
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


    void BasicMigration::copyFiles()
    {
        OUString sTargetDir;
        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            sTargetDir += sTargetUserBasic;
            TStringVectorPtr aFileList = getFiles( m_sSourceDir );
            for (auto const& elem : *aFileList)
            {
                std::u16string_view sLocalName = elem.subView( m_sSourceDir.getLength() );
                OUString sTargetName = sTargetDir + sLocalName;
                INetURLObject aURL( sTargetName );
                aURL.removeSegment();
                checkAndCreateDirectory( aURL );
                ::osl::FileBase::RC aResult = ::osl::File::copy( elem, sTargetName );
                if ( aResult != ::osl::FileBase::E_None )
                {
                    SAL_WARN( "desktop", "BasicMigration::copyFiles: cannot copy "
                                << elem << " to " << sTargetName );
                }
            }
        }
        else
        {
            OSL_FAIL( "BasicMigration::copyFiles: no user installation!" );
        }
    }


    // XServiceInfo


    OUString BasicMigration::getImplementationName()
    {
        return u"com.sun.star.comp.desktop.migration.Basic"_ustr;
    }


    sal_Bool BasicMigration::supportsService(OUString const & ServiceName)
    {
        return cppu::supportsService(this, ServiceName);
    }


    Sequence< OUString > BasicMigration::getSupportedServiceNames()
    {
        return { u"com.sun.star.migration.Basic"_ustr };
    }


    // XInitialization


    void BasicMigration::initialize( const Sequence< Any >& aArguments )
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
                    OSL_FAIL( "BasicMigration::initialize: argument UserData has wrong type!" );
                }
                m_sSourceDir += sSourceUserBasic;
                break;
            }
        }
    }


    // XJob


    Any BasicMigration::execute( const Sequence< beans::NamedValue >& )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        copyFiles();

        return Any();
    }


}   // namespace migration

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_BasicMigration_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new migration::BasicMigration());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
