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
#include "precompiled_unotools.hxx"

#include <unotools/tempfile.hxx>
#include <tools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
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
        aParent += rtl::OUString::createFromAscii( "/" );

    if( 0 == aParent.compareToAscii( "file://" ) )
        aParent = rtl::OUString::createFromAscii( "file:///" );

    return aParent;
}

sal_Bool ensuredir( const rtl::OUString& rUnqPath )
{
    rtl::OUString aPath;
    if ( rUnqPath.getLength() < 1 )
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

#define TMPNAME_SIZE  ( 1 + 5 + 5 + 4 + 1 )
String ConstructTempDir_Impl( const String* pParent )
{
    String aName;
    if ( pParent && pParent->Len() )
    {
        ::ucbhelper::ContentBroker* pBroker = ::ucbhelper::ContentBroker::get();
        if ( pBroker )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                    pBroker->getContentProviderManagerInterface();

            // if parent given try to use it
            rtl::OUString aTmp( *pParent );

            // test for valid filename
            rtl::OUString aRet;
            ::osl::FileBase::getFileURLFromSystemPath(
                ::ucbhelper::getSystemPathFromFileURL( xManager, aTmp ),
                aRet );
            if ( aRet.getLength() )
            {
                ::osl::DirectoryItem aItem;
                sal_Int32 i = aRet.getLength();
                if ( aRet[i-1] == '/' )
                    i--;

                if ( DirectoryItem::get( ::rtl::OUString( aRet, i ), aItem ) == FileBase::E_None )
                    aName = aRet;
            }
        }
        else
        {
            DBG_WARNING( "::unotools::TempFile : UCB not present or not initialized!" );
        }
    }

    if ( !aName.Len() )
    {
        ::rtl::OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        if (rTempNameBase_Impl.getLength() == 0)
        {
            ::rtl::OUString ustrTempDirURL;
            ::osl::FileBase::RC rc = ::osl::File::getTempDirURL(
                ustrTempDirURL );
            if (rc == ::osl::FileBase::E_None)
                rTempNameBase_Impl = ustrTempDirURL;
        }
        // if no parent or invalid parent : use default directory
        DBG_ASSERT( rTempNameBase_Impl.getLength(), "No TempDir!" );
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
    aName += String::CreateFromAscii( "sv" );

    rName.Erase();
    unsigned long nSeed = Time::GetSystemTicks() % nMax;
    for ( unsigned long u = nSeed; ++u != nSeed; )
    {
        u %= nMax;
        String aTmp( aName );
        aTmp += String::CreateFromInt64( static_cast<sal_Int64>(u), nRadix );
        aTmp += String::CreateFromAscii( ".tmp" );

        if ( bDir )
        {
            FileBase::RC err = Directory::create( aTmp );
            if (  err == FileBase::E_None )
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
#ifdef UNX
/* RW permission for the user only! */
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
                 FileStatus aTmpStatus( FileStatusMask_Type );
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
            aTmp += String::CreateFromAscii( ".tmp" );
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
            if ( err == FileBase::E_None )
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
                FileStatus aTmpStatus( FileStatusMask_Type );
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
        String aTmp;
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
        rTempNameBase_Impl += String( '/' );

        TempFile aBase( NULL, sal_True );
        if ( aBase.IsValid() )
            // use it in case of success
            rTempNameBase_Impl = aBase.pImp->aName;

        // return system path of used directory
        FileBase::getSystemPathFromFileURL( rTempNameBase_Impl, aTmp );
    }

    return aTmp;
}

String TempFile::GetTempNameBaseDirectory()
{
    const ::rtl::OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
    if ( !rTempNameBase_Impl.getLength() )
        return String();

    rtl::OUString aTmp;
    FileBase::getSystemPathFromFileURL( rTempNameBase_Impl, aTmp );
    return aTmp;
}

}
