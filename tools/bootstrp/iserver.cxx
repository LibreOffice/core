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
#include <tools/iparser.hxx>
#include <tools/geninfo.hxx>
#include "bootstrp/appdef.hxx"
#include <stdio.h>


/*****************************************************************************/
#ifdef UNX
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( argc == 1 ) {
        fprintf( stdout, "\ni_server.exe v2.0 (c) 2000\n\n" );
        fprintf( stdout, "Syntax:  i_server -i accesspath [-l] [-d database] \n" );
        fprintf( stdout, "Example: - i_server -i vcl364/settings/now\n" );
        fprintf( stdout, "           returns value of settings \"now\" of version \"vcl364\"\n" );
        fprintf( stdout, "         - i_server -i vcl364/settings -l\n" );
        fprintf( stdout, "           returns a list of all settings of version \"vcl364\"\n" );
    }
    else {
        sal_Bool bError = sal_False;
        sal_Bool bList = sal_False;
        ByteString sInfo( "" );
        ByteString sDataBase( GetDefStandList());

        sal_Bool bGetNow = sal_False;

        int nCount = 1;
        while (( nCount < argc ) &&
            ( !bError ))
        {
            if ( ByteString( argv[nCount] ).ToUpperAscii() == "-I" ) {
                // requestet info path
                nCount++;
                if( nCount < argc ) {
                    sInfo = ByteString( argv[nCount] );
                    nCount++;
                }
                else bError = sal_True;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-D" ) {
                // requestet info path
                nCount++;
                if( nCount < argc ) {
                    sDataBase = ByteString( argv[nCount] );
                    nCount++;
                }
                else bError = sal_True;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-L" ) {
                // request list of childs
                nCount++;
                bList = sal_True;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-N" ) {
                // request list of childs
                nCount++;
                bGetNow = sal_True;
            }
            else {
                bError = sal_True;
            }
        }

        if ( !bError ) {
            InformationParser aParser( REPLACE_VARIABLES );
            ByteString sStandList( sDataBase );
            String s = String( sStandList, gsl_getSystemTextEncoding());
            GenericInformationList *pList = aParser.Execute( s );
            if ( !pList )
                return 1;

            if ( sInfo.Len()) {
                GenericInformation *pInfo = pList->GetInfo( sInfo, sal_True );

                if ( pInfo ) {
                    ByteString sValue( pInfo->GetValue());
                    // show the info and its value
                    fprintf( stdout, "%s %s\n", pInfo->GetBuffer(), sValue.GetBuffer());
                    if ( bList ) {
                        GenericInformationList *pList = pInfo->GetSubList();
                        if ( pList ) {
                            // show whole list of childs and their values
                            for( sal_uIntPtr i = 0; i < pList->Count(); i++ ) {
                                GenericInformation *pInfo = pList->GetObject( i );
                                ByteString sValue( pInfo->GetValue());
                                fprintf( stdout, "    %s %s\n", pInfo->GetBuffer(), sValue.GetBuffer());
                            }
                        }
                    }
                    return 0;
                }
                return 1;
            }
            else {
                // show whole list of childs and their values
                for( sal_uIntPtr i = 0; i < pList->Count(); i++ ) {
                    GenericInformation *pInfo = pList->GetObject( i );
                    if ( bGetNow ) {
                        ByteString sPath( "settings/now" );
                        GenericInformation *pSubInfo = pInfo->GetSubInfo( sPath, sal_True );
                        if ( pSubInfo && pSubInfo->GetValue() == "_TRUE" )
                            fprintf( stdout, "%s\n", pInfo->GetBuffer());
                    }
                    else {
                        ByteString sValue( pInfo->GetValue());
                        fprintf( stdout, "    %s %s\n", pInfo->GetBuffer(), sValue.GetBuffer());
                    }
                }
                return 0;
            }
        }
        else
            fprintf( stderr, "%s: Fehler in der Kommandozeile!", argv[0] );
            // command line arror !!!
    }

    return 1;
}

