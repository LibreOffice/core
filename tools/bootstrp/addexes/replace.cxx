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

#include <stdio.h>
#include <tools/string.hxx>


/****************************************************************************/
#if defined UNX
int main( int argc, char *argv[] )
#else
int _cdecl main( int argc, char *argv[] )
#endif
/****************************************************************************/
{
    if ( argc < 4 )
    {
        fprintf( stderr, "ERROR: too few parameters. \n\n");
        fprintf( stderr, "usage: txtrep.exe EnvironmentVariable Searchstring replacestring\n");
        return 1;
    }
    ByteString aText( getenv( argv[ 1 ] ));
    if ( aText.Len() == 0 )
    {
        fprintf( stderr, "ERROR: Variable not set. \n\n");
        fprintf( stderr, "usage: txtrep.exe EnvironmentVariable Searchstring replacestring\n");
        return 2;
    }
    ByteString aSearch( argv[ 2 ] );
    ByteString aReplace( argv[ 3 ] );

    ByteString aUpperText( aText );
    aUpperText.ToUpperAscii();


    sal_uIntPtr nIndex;
    aSearch.ToUpperAscii();

    nIndex = aUpperText.Search( aSearch.GetBuffer(), 0);
    while ( nIndex != STRING_NOTFOUND )
    {
        aText.Replace( nIndex, aSearch.Len(), aReplace.GetBuffer());
        aUpperText.Replace( nIndex, aSearch.Len(), aReplace.GetBuffer());
        nIndex = aUpperText.Search( aSearch.GetBuffer(), nIndex + aReplace.Len());
    }

    fprintf( stdout, "%s\n", aText.GetBuffer());
    return 0;
}
