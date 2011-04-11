/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
