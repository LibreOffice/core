/*************************************************************************
 *
 *  $RCSfile: iserver.cxx,v $
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
#include "iparser.hxx"
#include "geninfo.hxx"
#include "appdef.hxx"
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
        BOOL bError = FALSE;
        BOOL bList = FALSE;
        ByteString sInfo( "" );
        ByteString sDataBase( GetDefStandList());

        BOOL bGetNow = FALSE;

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
                else bError = TRUE;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-D" ) {
                // requestet info path
                nCount++;
                if( nCount < argc ) {
                    sDataBase = ByteString( argv[nCount] );
                    nCount++;
                }
                else bError = TRUE;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-L" ) {
                // request list of childs
                nCount++;
                bList = TRUE;
            }
            else if ( ByteString( argv[nCount] ).ToUpperAscii() == "-N" ) {
                // request list of childs
                nCount++;
                bGetNow = TRUE;
            }
            else {
                bError = TRUE;
            }
        }

        if ( !bError ) {
            InformationParser aParser( REPLACE_VARIABLES );
            ByteString sStandList( sDataBase );
            GenericInformationList *pList = aParser.Execute( String( sStandList, gsl_getSystemTextEncoding()));
            if ( !pList )
                return 1;

            if ( sInfo.Len()) {
                GenericInformation *pInfo = pList->GetInfo( sInfo, TRUE );

                if ( pInfo ) {
                    ByteString sValue( pInfo->GetValue());
                    // show the info and its value
                    fprintf( stdout, "%s %s\n", pInfo->GetBuffer(), sValue.GetBuffer());
                    if ( bList ) {
                        GenericInformationList *pList = pInfo->GetSubList();
                        if ( pList ) {
                            // show whole list of childs and their values
                            for( ULONG i = 0; i < pList->Count(); i++ ) {
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
                for( ULONG i = 0; i < pList->Count(); i++ ) {
                    GenericInformation *pInfo = pList->GetObject( i );
                    if ( bGetNow ) {
                        ByteString sPath( "settings/now" );
                        GenericInformation *pSubInfo = pInfo->GetSubInfo( sPath, TRUE );
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

