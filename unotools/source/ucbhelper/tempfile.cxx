/*************************************************************************
 *
 *  $RCSfile: tempfile.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-12 15:59:19 $
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

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <stdio.h>

using namespace osl;

namespace utl
{

static ::rtl::OUString aTempNameBase_Impl;

struct TempFile_Impl
{
    String      aName;
    sal_Bool    bIsDirectory;
};

#define TMPNAME_SIZE  ( 1 + 5 + 5 + 4 + 1 )
String ConstructTempDir_Impl( const String* pParent )
{
    String aName;
    if ( pParent && pParent->Len() )
    {
        // if parent given try to use it
        rtl::OUString aTmp( *pParent );
        rtl::OUString aRet;

        // test for valid filename
        if ( FileBase::getNormalizedPathFromFileURL( aTmp, aRet ) == FileBase::E_None )
        {
            ::osl::DirectoryItem aItem;
            sal_Int32 i = aRet.getLength();
            if ( aRet[i-1] == '/' )
                i--;

            if ( DirectoryItem::get( ::rtl::OUString( aRet, i ), aItem ) == FileBase::E_None )
                aName = aRet;
        }
    }

    if ( !aName.Len() )
    {
        // if no parent or invalid parent : use default directory
        DBG_ASSERT( aTempNameBase_Impl.getLength(), "No TempDir!" );
        aName = aTempNameBase_Impl;
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
        FileBase::getSystemPathFromNormalizedPath( aName, aTmp );
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
    FileBase::getSystemPathFromNormalizedPath( pImp->aName, aTmp );
    return aTmp;
}

String TempFile::GetURL() const
{
    rtl::OUString aTmp;
    FileBase::getFileURLFromNormalizedPath( pImp->aName, aTmp );
    return aTmp;
}

String TempFile::ConvertURLToPhysicalName( const String& rName )
{
    rtl::OUString aTmp;
    FileBase::getFileURLFromNormalizedPath( rName, aTmp );
    return aTmp;
}


String TempFile::SetTempNameBaseDirectory( const String &rBaseName )
{
    String aName( rBaseName );

    FileBase::RC err= Directory::create( aName );
    if ( err == FileBase::E_None || err == FileBase::E_EXIST )
    {
        aTempNameBase_Impl = aName;
        aTempNameBase_Impl += ::rtl::OUString( '/' );

        TempFile aBase( NULL, sal_True );
        if ( aBase.IsValid() )
            aTempNameBase_Impl = aBase.pImp->aName;
    }

    rtl::OUString aTmp;
    FileBase::getSystemPathFromNormalizedPath( aTempNameBase_Impl, aTmp );
    return aTmp;
}

String TempFile::GetTempNameBaseDirectory()
{
    if ( !aTempNameBase_Impl.getLength() )
        return String();

    rtl::OUString aTmp;
    FileBase::getSystemPathFromNormalizedPath( aTempNameBase_Impl, aTmp );
    return aTmp;
}

};
