/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_l10ntools.hxx"

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
                : bInExecute( sal_False ) , bLocal( bLocal_in )
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

        static rtl::OUString WCARD1 ( rtl::OUString::createFromAscii( "unxlng" ) );
        static rtl::OUString WCARD2 ( rtl::OUString::createFromAscii( "unxsol" ) );
        static rtl::OUString WCARD3 ( rtl::OUString::createFromAscii( "wntmsc" ) );
        static rtl::OUString WCARD4 ( rtl::OUString::createFromAscii( "common" ) );
        static rtl::OUString WCARD5 ( rtl::OUString::createFromAscii( "unxmac" ) );
        static rtl::OUString WCARD6 ( rtl::OUString::createFromAscii( "unxubt" ) );
        static rtl::OUString WCARD7 ( rtl::OUString::createFromAscii( ".svn" ) );
        static rtl::OUString WCARD8 ( rtl::OUString::createFromAscii( ".hg" ) );


        if( sDirName.indexOf( WCARD1 , 0 ) > -1 ||
            sDirName.indexOf( WCARD2 , 0 ) > -1 ||
            sDirName.indexOf( WCARD3 , 0 ) > -1 ||
            sDirName.indexOf( WCARD4 , 0 ) > -1 ||
            sDirName.indexOf( WCARD5 , 0 ) > -1 ||
            sDirName.indexOf( WCARD6 , 0 ) > -1 ||
            sDirName.indexOf( WCARD7 , 0 ) > -1 ||
            sDirName.indexOf( WCARD8 , 0 ) > -1
           )    return;
        //printf("**** %s \n", OUStringToOString( sDirName , RTL_TEXTENCODING_UTF8 , sDirName.getLength() ).getStr() );

        rtl::OUString sDirNameTmp = aDirectory.getFullName();
        ByteString sDirNameTmpB( rtl::OUStringToOString( sDirNameTmp , RTL_TEXTENCODING_UTF8 , sDirName.getLength() ).getStr() );

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
            for ( sal_uLong i=0;i < aDirectory.getSubDirectories().size();i++ )
                ExecuteDirectory( aDirectory.getSubDirectories()[ i ] );
    }
}

/*****************************************************************************/
sal_Bool SourceTreeIterator::StartExecute()
/*****************************************************************************/
{

    bInExecute = sal_True;                  // FIXME
    ExecuteDirectory( aRootDirectory );

    if ( bInExecute ) {                 // FIXME
        bInExecute = sal_False;
        return sal_True;
    }
    return sal_False;
}

/*****************************************************************************/
void SourceTreeIterator::EndExecute()
/*****************************************************************************/
{
    bInExecute = sal_False;
}

/*****************************************************************************/
void SourceTreeIterator::OnExecuteDirectory( const rtl::OUString &rDirectory )
/*****************************************************************************/
{
    fprintf( stdout, "%s\n", rtl::OUStringToOString( rDirectory, RTL_TEXTENCODING_UTF8, rDirectory.getLength() ).getStr() );
}
