/*************************************************************************
 *
 *  $RCSfile: tempfile.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:07:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <unotools/tempfile.hxx>
#include <tools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <rtl/ustring.hxx>
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif
#include <osl/file.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <stdio.h>

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
    osl::FileBase::RC nError = aDirectory.open();
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
        ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
        if ( pBroker )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentProviderManager > xManager =
                    pBroker->getContentProviderManagerInterface();

            // if parent given try to use it
            rtl::OUString aTmp( *pParent );

            // test for valid filename
            rtl::OUString aRet;
            ::osl::FileBase::getFileURLFromSystemPath(
                ::ucb::getSystemPathFromFileURL( xManager, aTmp ),
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
    sal_Int32 i = aName.Len();
    if( i>0 && aName.GetChar(i-1) != '/' )
        aName += '/';

    return aName;
}

void CreateTempName_Impl( String& rName, sal_Bool bKeep, sal_Bool bDir = sal_True )
{
    // add a suitable tempname
    // Prefix can have 5 chars, leaving 3 for numbers. 26 ** 3 == 17576
    // ER 13.07.00  why not radix 36 [0-9A-Z] ?!?
    const unsigned nRadix = 26;
    String aName( rName );
    aName += String::CreateFromAscii( "sv" );
    sal_Int32 i = aName.Len();

    rName.Erase();
    static unsigned long u = Time::GetSystemTicks();
    for ( unsigned long nOld = u; ++u != nOld; )
    {
        u %= (nRadix*nRadix*nRadix);
        String aTmp( aName );
        aTmp += String::CreateFromInt32( (sal_Int32) (unsigned) u, nRadix );
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
            FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
            if (  err == FileBase::E_None )
            {
                rName = aTmp;
                aFile.close();
                break;
            }
            else if ( err != FileBase::E_EXIST )
            {
                 // if f.e. name contains invalid chars stop trying to create files
                 break;
            }
        }
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

TempFile::TempFile( const String& rLeadingChars, const String* pExtension, const String* pParent, sal_Bool bDirectory )
    : pImp( new TempFile_Impl )
    , bKillingFileEnabled( sal_False )
{
    pImp->bIsDirectory = bDirectory;

    // get correct directory
    String aName = ConstructTempDir_Impl( pParent );

    // now use special naming scheme ( name takes leading chars and an index counting up from zero
    aName += rLeadingChars;
    for ( sal_Int32 i=0;; i++ )
    {
        String aTmp( aName );
        aTmp += String::CreateFromInt32( i );
        if ( pExtension )
            aTmp += *pExtension;
        else
            aTmp += String::CreateFromAscii( ".tmp" );
        if ( bDirectory )
        {
            FileBase::RC err = Directory::create( aTmp );
            if ( err == FileBase::E_None )
            {
                pImp->aName = aTmp;
                break;
            }
            else if ( err != FileBase::E_EXIST )
                // if f.e. name contains invalid chars stop trying to create dirs
                break;
        }
        else
        {
            File aFile( aTmp );
            FileBase::RC err = aFile.open(osl_File_OpenFlag_Create);
            if ( err == FileBase::E_None )
            {
                pImp->aName = aTmp;
                aFile.close();
                break;
            }
            else if ( err != FileBase::E_EXIST )
                // if f.e. name contains invalid chars stop trying to create dirs
                break;
        }
    }
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
            pImp->pStream = new SvFileStream( pImp->aName, eMode );
    }

    return pImp->pStream;
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
        rTempNameBase_Impl += ::rtl::OUString( '/' );

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

};
