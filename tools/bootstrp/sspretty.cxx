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
#include <stdio.h>


/*****************************************************************************/
#ifdef UNX
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/*****************************************************************************/
{
    if ( argc != 2 ) {
        fprintf( stdout, "\nsspretty.exe v1.0 (c) 2001\n\n" );
        fprintf( stdout, "Syntax:  sspretty filename\n" );
    }
    else {
        String aFileName( argv[ 1 ], RTL_TEXTENCODING_ASCII_US );
        InformationParser aParser;
        GenericInformationList *pList = aParser.Execute( aFileName );
        if ( pList )
            aParser.Save( aFileName, pList );
        else {
            fprintf( stderr, "Error reading input file!\n" );
            return 1;
        }
    }
    return 0;
}

