/*************************************************************************
 *
 *  $RCSfile: inimgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:59 $
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
#if !defined( UNX ) && !defined( MAC )
#include <direct.h>
#else
#ifndef MAC
#include <sys/stat.h>
#else
#include <stat.h>
#endif
#endif
#include <stdlib.h>
#include <stdio.h>


#include "inimgr.hxx"
#include "appdef.hxx"
#include "fattr.hxx"

/****************************************************************************/
IniManager::IniManager( ByteString &rDir, ByteString &rLocalDir )
/****************************************************************************/
            : bUpdate( TRUE )
{
    sLocalPath = ByteString( getenv( "LOCALINI" ));
    if ( !sLocalPath.Len())
        sLocalPath = rLocalDir;

    sGlobalDir = rDir;
#if !defined( UNX ) && !defined( MAC )
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
#if !defined( UNX ) && !defined( MAC )
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

#if !defined( UNX ) && !defined( MAC )
    mkdir(( char * ) sLocalPath.GetBuffer());
#else
    mkdir( sLocalPath.GetBuffer(), 00777 );
#endif

    sGlobalDir = ByteString( _INIROOT );
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

#ifndef MAC
    sTmp.SearchAndReplace( sIni, sLocalPath );
#else
    sTmp.SearchAndReplace( sIni, GetIniRootOld() );
#endif

    while ( sTmp.SearchAndReplace( "\\\\", "\\" ) != STRING_NOTFOUND );
#ifdef MAC
    while ( sTmp.SearchAndReplace( "\\", ":" ) != STRING_NOTFOUND );
    while ( sTmp.SearchAndReplace( "::", ":" ) != STRING_NOTFOUND );
#endif
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
#ifdef MAC
        ByteString sLocal( getenv( "TEMP" ));
        sLocal += ByteString( ":localini" );
#else
        ByteString sLocal( getenv( "TMP" ));
        sLocal += ByteString( "\\localini" );
#endif
#endif

        sLocalPath = sLocal;
    }

    return sLocalPath;
}

/****************************************************************************/
void IniManager::ForceUpdate()
/****************************************************************************/
{
    UniString sUniGlobalDir( sGlobalDir, gsl_getSystemTextEncoding());
    DirEntry aPath( UniString( sGlobalDir, gsl_getSystemTextEncoding()));
    Dir aDir( aPath, FSYS_KIND_DIR | FSYS_KIND_FILE);

#ifndef UNX
#ifndef MAC
    sLocalPath.EraseTrailingChars( '\\' );
    sLocalPath += "\\";
#else
    sLocalPath.EraseTrailingChars( ':' );
    sLocalPath += ":";
#endif
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
                    BOOL bWorkaround = FALSE;

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
