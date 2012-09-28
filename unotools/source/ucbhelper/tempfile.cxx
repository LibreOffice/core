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
#include <tools/tempfile.hxx>
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
        : public rtl::Static< ::rtl::OUString, TempNameBase_Impl > {};
}

namespace utl
{

struct TempFile_Impl
{
    String      aName;
    String      aURL;
    SvStream*   pStream;
    sal_Bool    bIsDirectory;

                TempFile_Impl()
                    : pStream(0)
                    {}
};

rtl::OUString getParentName( const rtl::OUString& aFileName )
{
    sal_Int32 lastIndex = aFileName.lastIndexOf( sal_Unicode('/') );
    rtl::OUString aParent = aFileName.copy( 0,lastIndex );

    if( aParent[ aParent.getLength()-1] == sal_Unicode(':') && aParent.getLength() == 6 )
        aParent += rtl::OUString("/");

    if( 0 == aParent.compareToAscii( "file://" ) )
        aParent = rtl::OUString("file:///");

    return aParent;
}

sal_Bool ensuredir( const rtl::OUString& rUnqPath )
{
    rtl::OUString aPath;
    if ( rUnqPath.isEmpty() )
        return sal_False;

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
        return sal_True;

    // try to create the directory
    nError = osl::Directory::create( aPath );
    sal_Bool  bSuccess = ( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
    if( !bSuccess )
    {
        // perhaps parent(s) don't exist
        rtl::OUString aParentDir = getParentName( aPath );
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

String ConstructTempDir_Impl( const String* pParent )
{
    String aName;
    if ( pParent && pParent->Len() )
    {
        com::sun::star::uno::Reference<
            com::sun::star::ucb::XUniversalContentBroker > pBroker(
                com::sun::star::ucb::UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() ) );

        // if parent given try to use it
        rtl::OUString aTmp( *pParent );

        // test for valid filename
        rtl::OUString aRet;
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

    if ( !aName.Len() )
    {
        ::rtl::OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        if (rTempNameBase_Impl.isEmpty())
        {
            ::rtl::OUString ustrTempDirURL;
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
    xub_StrLen i = aName.Len();
    if( i>0 && aName.GetChar(i-1) != '/' )
        aName += '/';

    return aName;
}

void CreateTempName_Impl( String& rName, sal_Bool bKeep, sal_Bool bDir = sal_True )
{
    // add a suitable tempname
    // 36 ** 6 == 2176782336
    unsigned const nRadix = 36;
    unsigned long const nMax = (nRadix*nRadix*nRadix*nRadix*nRadix*nRadix);
    String aName( rName );
    aName += rtl::OUString( "lu" );

    rName.Erase();
    static unsigned long u = Time::GetSystemTicks() % nMax;
    for ( unsigned long nSeed = u; ++u != nSeed; )
    {
        u %= nMax;
        String aTmp( aName );
        aTmp += rtl::OUString::valueOf(static_cast<sal_Int64>(u), nRadix);
        aTmp += rtl::OUString( ".tmp" );

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

void lcl_createName(TempFile_Impl& _rImpl,const String& rLeadingChars,sal_Bool _bStartWithZero, const String* pExtension, const String* pParent, sal_Bool bDirectory)
{
    _rImpl.bIsDirectory = bDirectory;

    // get correct directory
    String aName = ConstructTempDir_Impl( pParent );

    sal_Bool bUseNumber = _bStartWithZero;
    // now use special naming scheme ( name takes leading chars and an index counting up from zero
    aName += rLeadingChars;
    for ( sal_Int32 i=0;; i++ )
    {
        String aTmp( aName );
        if ( bUseNumber )
            aTmp += String::CreateFromInt32( i );
        bUseNumber = sal_True;
        if ( pExtension )
            aTmp += *pExtension;
        else
            aTmp += rtl::OUString( ".tmp" );
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
            aTmp += String::CreateFromInt32( i );
    }
}


String TempFile::CreateTempName( const String* pParent )
{
    // get correct directory
    String aName = ConstructTempDir_Impl( pParent );

    // get TempFile name with default naming scheme
    CreateTempName_Impl( aName, sal_False );

    // convert to file URL
    rtl::OUString aTmp;
    if ( aName.Len() )
        FileBase::getSystemPathFromFileURL( aName, aTmp );
    return aTmp;
}

TempFile::TempFile( const String* pParent, sal_Bool bDirectory )
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    pImp->bIsDirectory = bDirectory;

    // get correct directory
    pImp->aName = ConstructTempDir_Impl( pParent );

    // get TempFile with default naming scheme
    CreateTempName_Impl( pImp->aName, sal_True, bDirectory );
}

TempFile::TempFile( const String& rLeadingChars, const String* pExtension, const String* pParent, sal_Bool bDirectory)
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    lcl_createName(*pImp,rLeadingChars,sal_True, pExtension, pParent, bDirectory);
}
TempFile::TempFile( const String& rLeadingChars,sal_Bool _bStartWithZero, const String* pExtension, const String* pParent, sal_Bool bDirectory)
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    lcl_createName(*pImp,rLeadingChars,_bStartWithZero, pExtension, pParent, bDirectory);
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

sal_Bool TempFile::IsValid() const
{
    return pImp->aName.Len() != 0;
}

String TempFile::GetFileName() const
{
    rtl::OUString aTmp;
    FileBase::getSystemPathFromFileURL( pImp->aName, aTmp );
    return aTmp;
}

String TempFile::GetURL() const
{
    if ( !pImp->aURL.Len() )
    {
        rtl::OUString aTmp;
        LocalFileHelper::ConvertPhysicalNameToURL( GetFileName(), aTmp );
        pImp->aURL = aTmp;
    }

    return pImp->aURL;
}

SvStream* TempFile::GetStream( StreamMode eMode )
{
    if ( !pImp->pStream )
    {
        if ( GetURL().Len() )
            pImp->pStream = UcbStreamHelper::CreateStream( pImp->aURL, eMode, sal_True /* bFileExists */ );
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

String TempFile::SetTempNameBaseDirectory( const String &rBaseName )
{
    if( !rBaseName.Len() )
        return String();

    rtl::OUString aUnqPath( rBaseName );

    // remove trailing slash
    if ( rBaseName.GetChar( rBaseName.Len() - 1 ) == sal_Unicode( '/' ) )
        aUnqPath = rBaseName.Copy( 0, rBaseName.Len() - 1 );

    // try to create the directory
    sal_Bool bRet = sal_False;
    osl::FileBase::RC err = osl::Directory::create( aUnqPath );
    if ( err != FileBase::E_None && err != FileBase::E_EXIST )
        // perhaps parent(s) don't exist
        bRet = ensuredir( aUnqPath );
    else
        bRet = sal_True;

    // failure to create base directory means returning an empty string
    rtl::OUString aTmp;
    if ( bRet )
    {
        // append own internal directory
        bRet = sal_True;
        ::rtl::OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        rTempNameBase_Impl = rBaseName;
        rTempNameBase_Impl += rtl::OUString('/');

        TempFile aBase( NULL, sal_True );
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
