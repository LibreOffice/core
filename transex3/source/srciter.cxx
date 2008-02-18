/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srciter.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:10:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_transex3.hxx"

#include "srciter.hxx"
#include <stdio.h>
#include <tools/fsys.hxx>

//
// class SourceTreeIterator
//

/*****************************************************************************/
SourceTreeIterator::SourceTreeIterator(
    const ByteString &rRootDirectory, const ByteString &rVersion , bool bLocal_in )
/*****************************************************************************/
                : bInExecute( FALSE ) , bLocal( bLocal_in )
{
    (void) rVersion ;

    if(!bLocal){
        rtl::OUString sRootDirectory( rRootDirectory.GetBuffer() , rRootDirectory.Len() , RTL_TEXTENCODING_UTF8 );
        aRootDirectory = transex::Directory( sRootDirectory );
    }
}

/*****************************************************************************/
SourceTreeIterator::~SourceTreeIterator()
/*****************************************************************************/
{
}

/*****************************************************************************/
void SourceTreeIterator::ExecuteDirectory( transex::Directory& aDirectory )
/*****************************************************************************/
{
    if ( bInExecute ) {
        rtl::OUString sDirName = aDirectory.getDirectoryName();

        static rtl::OUString WCARD1 ( rtl::OUString::createFromAscii( "unxlngi" ) );
        static rtl::OUString WCARD2 ( rtl::OUString::createFromAscii( "unxsoli" ) );
        static rtl::OUString WCARD3 ( rtl::OUString::createFromAscii( "wntmsci" ) );
        static rtl::OUString WCARD4 ( rtl::OUString::createFromAscii( "unxsols" ) );
        static rtl::OUString WCARD5 ( rtl::OUString::createFromAscii( "common" ) );
        static rtl::OUString WCARD6 ( rtl::OUString::createFromAscii( "unxmacx" ) );
        static rtl::OUString WCARD7 ( rtl::OUString::createFromAscii( "unxlngx" ) );
        static rtl::OUString WCARD8 ( rtl::OUString::createFromAscii( "unxsolsx" ) );
        static rtl::OUString WCARD9 ( rtl::OUString::createFromAscii( "unxsolsu" ) );
        static rtl::OUString WCARD10 ( rtl::OUString::createFromAscii( "wntmscx" ) );


        if( sDirName.indexOf( WCARD1 , 0 ) > -1 ||
            sDirName.indexOf( WCARD2 , 0 ) > -1 ||
            sDirName.indexOf( WCARD3 , 0 ) > -1 ||
            sDirName.indexOf( WCARD4 , 0 ) > -1 ||
            sDirName.indexOf( WCARD5 , 0 ) > -1 ||
            sDirName.indexOf( WCARD6 , 0 ) > -1 ||
            sDirName.indexOf( WCARD7 , 0 ) > -1 ||
            sDirName.indexOf( WCARD8 , 0 ) > -1 ||
            sDirName.indexOf( WCARD9 , 0 ) > -1 ||
            sDirName.indexOf( WCARD10 , 0 ) > -1
           )    return;
        //printf("**** %s \n", OUStringToOString( sDirName , RTL_TEXTENCODING_UTF8 , sDirName.getLength() ).getStr() );

        rtl::OUString sDirNameTmp = aDirectory.getFullName();
        ByteString sDirNameTmpB( OUStringToOString( sDirNameTmp , RTL_TEXTENCODING_UTF8 , sDirName.getLength() ).getStr() );

#ifdef WNT
        sDirNameTmpB.Append( ByteString("\\no_localization") );
#else
        sDirNameTmpB.Append( ByteString("/no_localization") );
#endif
        //printf("**** %s \n", OUStringToOString( sDirNameTmp , RTL_TEXTENCODING_UTF8 , sDirName.getLength() ).getStr() );

        DirEntry aDE( sDirNameTmpB.GetBuffer() );
        if( aDE.Exists() )
        {
            //printf("#### no_localization file found ... skipping");
            return;
        }

        aDirectory.setSkipLinks( bSkipLinks );
        aDirectory.readDirectory();
        OnExecuteDirectory( aDirectory.getFullName() );
        if ( aDirectory.getSubDirectories().size() )
            for ( ULONG i=0;i < aDirectory.getSubDirectories().size();i++ )
                ExecuteDirectory( aDirectory.getSubDirectories()[ i ] );
    }
}

/*****************************************************************************/
BOOL SourceTreeIterator::StartExecute()
/*****************************************************************************/
{

    bInExecute = TRUE;                  // FIXME
    ExecuteDirectory( aRootDirectory );

    if ( bInExecute ) {                 // FIXME
        bInExecute = FALSE;
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
void SourceTreeIterator::EndExecute()
/*****************************************************************************/
{
    bInExecute = FALSE;
}

/*****************************************************************************/
void SourceTreeIterator::OnExecuteDirectory( const rtl::OUString &rDirectory )
/*****************************************************************************/
{
    fprintf( stdout, "%s\n", rtl::OUStringToOString( rDirectory, RTL_TEXTENCODING_UTF8, rDirectory.getLength() ).getStr() );
}
