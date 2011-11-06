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

