/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inimgr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:58:41 $
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
            : bUpdate( TRUE )
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
            : bUpdate( TRUE )
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
            : bUpdate( TRUE )
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

    while ( sTmp.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND );
#else
    sTmp.SearchAndReplace( sGlobalDir, sLocalPath );

    ByteString sOldGlobalDir( GetIniRootOld() );
    sTmp.SearchAndReplace( sOldGlobalDir, sLocalPath );

    while ( sTmp.SearchAndReplace( "//", "/" ) != STRING_NOTFOUND );
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

    for ( USHORT i=0; i < aDir.Count(); i++ ) {
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
        bUpdate = FALSE;
    }
}
