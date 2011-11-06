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
#include "precompiled_tools.hxx"
#if !defined( UNX )
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include <stdlib.h>
#include <stdio.h>


#include "bootstrp/inimgr.hxx"
#include "bootstrp/appdef.hxx"

/****************************************************************************/
IniManager::IniManager( ByteString &rDir, ByteString &rLocalDir )
/****************************************************************************/
            : bUpdate( sal_True )
{
    sLocalPath = ByteString( getenv( "LOCALINI" ));
    if ( !sLocalPath.Len())
        sLocalPath = rLocalDir;

    sGlobalDir = rDir;
#if !defined( UNX ) && !defined( OS2 )
    mkdir(( char * ) sLocalPath.GetBuffer());
#else
    mkdir( sLocalPath.GetBuffer() ,00777 );
#endif
}

/****************************************************************************/
IniManager::IniManager( ByteString &rDir )
/****************************************************************************/
            : bUpdate( sal_True )
{
    sLocalPath = GetLocalIni();
    sGlobalDir = rDir;
#if !defined( UNX ) && !defined( OS2 )
    mkdir(( char * ) sLocalPath.GetBuffer());
#else
    mkdir( sLocalPath.GetBuffer() ,00777 );
#endif
}

/****************************************************************************/
IniManager::IniManager()
/****************************************************************************/
            : bUpdate( sal_True )
{
    sLocalPath = GetLocalIni();

#if !defined( UNX ) && !defined( OS2 )
    mkdir(( char * ) sLocalPath.GetBuffer());
#else
    mkdir( sLocalPath.GetBuffer(), 00777 );
#endif

    sGlobalDir = GetGlobalIni();
}

/****************************************************************************/
ByteString IniManager::ToLocal( ByteString &rPath )
/****************************************************************************/
{
    ByteString sTmp( rPath );
#if !defined( UNX )
    ByteString sUnc( _INI_UNC );
    sUnc.ToUpperAscii();
    ByteString sOldUnc( _INI_UNC_OLD );
    sOldUnc.ToUpperAscii();
    sTmp.ToUpperAscii();

    sTmp.SearchAndReplace( sUnc, _INI_DRV );
    sTmp.SearchAndReplace( sOldUnc, _INI_DRV );
    sTmp.ToUpperAscii();

    ByteString sIni( sGlobalDir );
    sIni.ToUpperAscii();

    sTmp.SearchAndReplace( sIni, sLocalPath );

    while ( sTmp.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND ) ;
#else
    sTmp.SearchAndReplace( sGlobalDir, sLocalPath );

    ByteString sOldGlobalDir( GetIniRootOld() );
    sTmp.SearchAndReplace( sOldGlobalDir, sLocalPath );

    while ( sTmp.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND ) ;
#endif

    return sTmp;
}

/****************************************************************************/
ByteString IniManager::GetLocalIni()
/****************************************************************************/
{
    ByteString sLocalPath = ByteString( getenv( "LOCALINI" ));

    if ( !sLocalPath.Len()) {
#ifdef UNX
        ByteString sLocal( getenv( "HOME" ));
        sLocal += ByteString( "/localini" );
#else
        ByteString sLocal( getenv( "TMP" ));
        sLocal += ByteString( "\\localini" );
#endif

        sLocalPath = sLocal;
    }

    return sLocalPath;
}

/****************************************************************************/
ByteString IniManager::GetGlobalIni()
/****************************************************************************/
{
    ByteString sGlobalPath = ByteString( GetEnv( "GLOBALINI" ));

    if ( !sGlobalPath.Len())
        sGlobalPath = ByteString( _INIROOT );

    return sGlobalPath;
}

/****************************************************************************/
void IniManager::ForceUpdate()
/****************************************************************************/
{
    UniString sUniGlobalDir( sGlobalDir, gsl_getSystemTextEncoding());
    DirEntry aPath( UniString( sGlobalDir, gsl_getSystemTextEncoding()));
    Dir aDir( aPath, FSYS_KIND_DIR | FSYS_KIND_FILE);

#ifndef UNX
    sLocalPath.EraseTrailingChars( '\\' );
    sLocalPath += "\\";
#else
    sLocalPath.EraseTrailingChars( '/' );
    sLocalPath += "/";
#endif

    for ( sal_uInt16 i=0; i < aDir.Count(); i++ ) {
        ByteString sEntry( aDir[i].GetName(), gsl_getSystemTextEncoding());
        if (( sEntry != "." ) &&
            ( sEntry != ".." ))
        {
            if ( !FileStat( aDir[i] ).IsKind( FSYS_KIND_DIR )) {
                ByteString sSrc( aDir[i].GetFull(), gsl_getSystemTextEncoding());
                ByteString sDestination( sLocalPath );
                sDestination += sEntry;

                UniString sUniDestination( sDestination, gsl_getSystemTextEncoding());
                DirEntry aDestEntry( sUniDestination );
                FileStat aDestStat( aDestEntry );
                FileStat aSrcStat( aDir[i] );

                if (( !aDestEntry.Exists() ) ||
                    ( aSrcStat.IsYounger( aDestStat )))
                {
                    FileCopier aFileCopier( aDir[ i ], aDestEntry );
                    aFileCopier.Execute();

                    while ( !aDestEntry.Exists())
                        aFileCopier.Execute();
                }
            }
        }
    }
}

/****************************************************************************/
void IniManager::Update()
/****************************************************************************/
{
    if ( bUpdate )
    {
        ForceUpdate();
        bUpdate = sal_False;
    }
}
