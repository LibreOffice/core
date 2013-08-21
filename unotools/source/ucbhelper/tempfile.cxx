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

#include "sal/config.h"

#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>
#include <osl/file.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <stdio.h>

#ifdef UNX
#include <sys/stat.h>
#endif

using namespace osl;

namespace
{
    struct TempNameBase_Impl
        : public rtl::Static< OUString, TempNameBase_Impl > {};
}

namespace utl
{

struct TempFile_Impl
{
    OUString    aName;
    OUString    aURL;
    SvStream*   pStream;
    bool        bIsDirectory;

                TempFile_Impl()
                    : pStream(0)
                    {}
};

OUString getParentName( const OUString& aFileName )
{
    sal_Int32 lastIndex = aFileName.lastIndexOf( sal_Unicode('/') );
    OUString aParent = aFileName.copy( 0, lastIndex );

    if( aParent[ aParent.getLength()-1] == sal_Unicode(':') && aParent.getLength() == 6 )
        aParent += OUString("/");

    if( 0 == aParent.compareToAscii( "file://" ) )
        aParent = OUString("file:///");

    return aParent;
}

bool ensuredir( const OUString& rUnqPath )
{
    OUString aPath;
    if ( rUnqPath.isEmpty() )
        return false;

    // remove trailing slash
    if ( rUnqPath[ rUnqPath.getLength() - 1 ] == sal_Unicode( '/' ) )
        aPath = rUnqPath.copy( 0, rUnqPath.getLength() - 1 );
    else
        aPath = rUnqPath;

    // HACK: create directory on a mount point with nobrowse option
    // returns ENOSYS in any case !!
    osl::Directory aDirectory( aPath );
#ifdef UNX
/* RW permission for the user only! */
 mode_t old_mode = umask(077);
#endif
    osl::FileBase::RC nError = aDirectory.open();
#ifdef UNX
umask(old_mode);
#endif
    aDirectory.close();
    if( nError == osl::File::E_None )
        return true;

    // try to create the directory
    nError = osl::Directory::create( aPath );
    bool  bSuccess = ( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
    if( !bSuccess )
    {
        // perhaps parent(s) don't exist
        OUString aParentDir = getParentName( aPath );
        if ( aParentDir != aPath )
        {
            bSuccess = ensuredir( getParentName( aPath ) );

            // After parent directory structure exists try it one's more
            if ( bSuccess )
            {
                // Parent directory exists, retry creation of directory
                nError = osl::Directory::create( aPath );
                bSuccess =( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
            }
        }
    }

    return bSuccess;
}

OUString ConstructTempDir_Impl( const OUString* pParent )
{
    OUString aName;
    if ( pParent && !pParent->isEmpty() )
    {
        com::sun::star::uno::Reference<
            com::sun::star::ucb::XUniversalContentBroker > pBroker(
                com::sun::star::ucb::UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() ) );

        // if parent given try to use it
        OUString aTmp( *pParent );

        // test for valid filename
        OUString aRet;
        ::osl::FileBase::getFileURLFromSystemPath(
            ::ucbhelper::getSystemPathFromFileURL( pBroker, aTmp ),
            aRet );
        if ( !aRet.isEmpty() )
        {
            ::osl::DirectoryItem aItem;
            sal_Int32 i = aRet.getLength();
            if ( aRet[i-1] == '/' )
                i--;

            if ( DirectoryItem::get( aRet.copy(0, i), aItem ) == FileBase::E_None )
                aName = aRet;
        }
    }

    if ( aName.isEmpty() )
    {
        OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        if (rTempNameBase_Impl.isEmpty())
        {
            OUString ustrTempDirURL;
            ::osl::FileBase::RC rc = ::osl::File::getTempDirURL(
                ustrTempDirURL );
            if (rc == ::osl::FileBase::E_None)
                rTempNameBase_Impl = ustrTempDirURL;
        }
        // if no parent or invalid parent : use default directory
        DBG_ASSERT( !rTempNameBase_Impl.isEmpty(), "No TempDir!" );
        aName = rTempNameBase_Impl;
        ensuredir( aName );
    }

    // Make sure that directory ends with a separator
    sal_Int32 i = aName.getLength();
    if( i>0 && aName[i-1] != '/' )
        aName += "/";

    return aName;
}

void CreateTempName_Impl( OUString& rName, bool bKeep, bool bDir = true )
{
    // add a suitable tempname
    // 36 ** 6 == 2176782336
    unsigned const nRadix = 36;
    unsigned long const nMax = (nRadix*nRadix*nRadix*nRadix*nRadix*nRadix);
    OUString aName = rName + "lu";

    rName = "";
    static unsigned long u = Time::GetSystemTicks() % nMax;
    for ( unsigned long nSeed = u; ++u != nSeed; )
    {
        u %= nMax;
        OUString aTmp( aName );
        aTmp += OUString::number(u, nRadix);
        aTmp += ".tmp" ;

        if ( bDir )
        {
#ifdef UNX /* RW permission for the user only! */
            mode_t old_mode = umask(077);
#endif
            FileBase::RC err = Directory::create( aTmp );
#ifdef UNX
            umask(old_mode);
#endif
            if ( err == FileBase::E_None )
            {
                // !bKeep: only for creating a name, not a file or directory
                if ( bKeep || Directory::remove( aTmp ) == FileBase::E_None )
                    rName = aTmp;
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                // if f.e. name contains invalid chars stop trying to create dirs
                break;
            }
        }
        else
        {
            DBG_ASSERT( bKeep, "Too expensive, use directory for creating name!" );
            File aFile( aTmp );
#ifdef UNX /* RW permission for the user only! */
            mode_t old_mode = umask(077);
#endif
            FileBase::RC err = aFile.open( osl_File_OpenFlag_Create | osl_File_OpenFlag_NoLock );
#ifdef UNX
            umask(old_mode);
#endif
            if (  err == FileBase::E_None )
            {
                rName = aTmp;
                aFile.close();
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                 // if f.e. name contains invalid chars stop trying to create files
                 // but if there is a folder with such name proceed further

                 DirectoryItem aTmpItem;
                 FileStatus aTmpStatus( osl_FileStatus_Mask_Type );
                 if ( DirectoryItem::get( aTmp, aTmpItem ) != FileBase::E_None
                   || aTmpItem.getFileStatus( aTmpStatus ) != FileBase::E_None
                   || aTmpStatus.getFileType() != FileStatus::Directory )
                     break;
            }
        }
    }
}

void lcl_createName(TempFile_Impl& _rImpl,const OUString& rLeadingChars, bool _bStartWithZero,
                    const OUString* pExtension, const OUString* pParent, bool bDirectory)
{
    _rImpl.bIsDirectory = bDirectory;

    // get correct directory
    OUString aName = ConstructTempDir_Impl( pParent );

    bool bUseNumber = _bStartWithZero;
    // now use special naming scheme ( name takes leading chars and an index counting up from zero
    aName += rLeadingChars;
    for ( sal_Int32 i=0;; i++ )
    {
        OUString aTmp( aName );
        if ( bUseNumber )
            aTmp += OUString::number( i );
        bUseNumber = true;
        if ( pExtension )
            aTmp += *pExtension;
        else
            aTmp += ".tmp" ;
        if ( bDirectory )
        {
            FileBase::RC err = Directory::create( aTmp );
            if ( err == FileBase::E_None )
            {
                _rImpl.aName = aTmp;
                break;
            }
            else if ( err != FileBase::E_EXIST )
                // if f.e. name contains invalid chars stop trying to create dirs
                break;
        }
        else
        {
            File aFile( aTmp );
#ifdef UNX
/* RW permission for the user only! */
 mode_t old_mode = umask(077);
#endif
            FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
#ifdef UNX
umask(old_mode);
#endif
            if ( err == FileBase::E_None || err == FileBase::E_NOLCK )
            {
                _rImpl.aName = aTmp;
                aFile.close();
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                // if f.e. name contains invalid chars stop trying to create dirs
                // but if there is a folder with such name proceed further

                DirectoryItem aTmpItem;
                FileStatus aTmpStatus( osl_FileStatus_Mask_Type );
                if ( DirectoryItem::get( aTmp, aTmpItem ) != FileBase::E_None
                  || aTmpItem.getFileStatus( aTmpStatus ) != FileBase::E_None
                  || aTmpStatus.getFileType() != FileStatus::Directory )
                    break;
            }
        }
        if ( !_bStartWithZero )
            aTmp += OUString::number( i );
    }
}


OUString TempFile::CreateTempName( const OUString* pParent )
{
    // get correct directory
    OUString aName = ConstructTempDir_Impl( pParent );

    // get TempFile name with default naming scheme
    CreateTempName_Impl( aName, false );

    // convert to file URL
    OUString aTmp;
    if ( !aName.isEmpty() )
        FileBase::getSystemPathFromFileURL( aName, aTmp );
    return aTmp;
}

TempFile::TempFile( const OUString* pParent, bool bDirectory )
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( false )
{
    pImp->bIsDirectory = bDirectory;

    // get correct directory
    pImp->aName = ConstructTempDir_Impl( pParent );

    // get TempFile with default naming scheme
    CreateTempName_Impl( pImp->aName, true, bDirectory );
}

TempFile::TempFile( const OUString& rLeadingChars, const OUString* pExtension, const OUString* pParent, bool bDirectory)
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( false )
{
    lcl_createName(*pImp, rLeadingChars, true, pExtension, pParent, bDirectory);
}
TempFile::TempFile( const OUString& rLeadingChars, bool _bStartWithZero, const OUString* pExtension, const OUString* pParent, bool bDirectory)
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( false )
{
    lcl_createName(*pImp, rLeadingChars, _bStartWithZero, pExtension, pParent, bDirectory);
}

TempFile::~TempFile()
{
    delete pImp->pStream;
    if ( bKillingFileEnabled )
    {
        if ( pImp->bIsDirectory )
        {
            // at the moment no recursiv algorithm present
            Directory::remove( pImp->aName );
        }
        else
        {
            File::remove( pImp->aName );
        }
    }

    delete pImp;
}

bool TempFile::IsValid() const
{
    return !(pImp->aName.isEmpty());
}

OUString TempFile::GetFileName() const
{
    OUString aTmp;
    FileBase::getSystemPathFromFileURL( pImp->aName, aTmp );
    return aTmp;
}

OUString TempFile::GetURL() const
{
    if ( pImp->aURL.isEmpty() )
    {
        OUString aTmp;
        LocalFileHelper::ConvertPhysicalNameToURL( GetFileName(), aTmp );
        pImp->aURL = aTmp;
    }

    return pImp->aURL;
}

SvStream* TempFile::GetStream( StreamMode eMode )
{
    if ( !pImp->pStream )
    {
        if ( !GetURL().isEmpty() )
            pImp->pStream = UcbStreamHelper::CreateStream( pImp->aURL, eMode, true /* bFileExists */ );
        else
            pImp->pStream = new SvMemoryStream( eMode );
    }

    return pImp->pStream;
}

void TempFile::CloseStream()
{
    if ( pImp->pStream )
    {
        delete pImp->pStream;
        pImp->pStream = NULL;
    }
}

OUString TempFile::SetTempNameBaseDirectory( const OUString &rBaseName )
{
    if( rBaseName.isEmpty() )
        return OUString();

    OUString aUnqPath( rBaseName );

    // remove trailing slash
    if ( rBaseName[ rBaseName.getLength() - 1 ] == sal_Unicode( '/' ) )
        aUnqPath = rBaseName.copy( 0, rBaseName.getLength() - 1 );

    // try to create the directory
    bool bRet = false;
    osl::FileBase::RC err = osl::Directory::create( aUnqPath );
    if ( err != FileBase::E_None && err != FileBase::E_EXIST )
        // perhaps parent(s) don't exist
        bRet = ensuredir( aUnqPath );
    else
        bRet = true;

    // failure to create base directory means returning an empty string
    OUString aTmp;
    if ( bRet )
    {
        // append own internal directory
        bRet = true;
        OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        rTempNameBase_Impl = rBaseName;
        rTempNameBase_Impl += OUString('/');

        TempFile aBase( NULL, true );
        if ( aBase.IsValid() )
            // use it in case of success
            rTempNameBase_Impl = aBase.pImp->aName;

        // return system path of used directory
        FileBase::getSystemPathFromFileURL( rTempNameBase_Impl, aTmp );
    }

    return aTmp;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
