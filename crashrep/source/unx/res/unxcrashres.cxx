/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unxcrashres.cxx,v $
 * $Revision: 1.5 $
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
 ************************************************************************/#include <tools/config.hxx>
#include <rtl/string.hxx>
#include <osl/thread.h>

#include <hash_map>
#include <string>
#include <cstdio>
#include <cerrno>

using namespace std;
using namespace rtl;

void filterValue( ByteString& rValue, const OString& rGroup, const ByteString& rKey )
{
    USHORT nStartPos = rValue.Search( '"' );
    USHORT nStopPos = rValue.SearchBackward( '"' );
    if( nStartPos == STRING_NOTFOUND || nStopPos == STRING_NOTFOUND )
    {
        fprintf( stderr, "Error: invalid key in [%s] in key \"%s\"\n",
                 rGroup.getStr(),
                 rKey.GetBuffer() );
        exit( 1 );
    }
    rValue.Erase( nStopPos );
    rValue.Erase( 0, nStartPos+1 );
}

int main( int argc, char** argv )
{
    if( argc != 3 )
    {
        fprintf( stderr, "USAGE: unxcrashres <in_lng_file> <out_file_prefix>\n" );
        exit( 1 );
    }

    ByteString tmp_argv1( argv[1] );
    Config aConfig( String( tmp_argv1, osl_getThreadTextEncoding() ) );
    hash_map< ByteString, hash_map< OString, OString, OStringHash >, OStringHash > aFiles;

    for( USHORT i = 0; i < aConfig.GetGroupCount(); i++ )
    {
        aConfig.SetGroup( aConfig.GetGroupName( i ) );
        OString aGroup = aConfig.GetGroupName( i );
        USHORT nKeys = aConfig.GetKeyCount();
        for( USHORT n = 0; n < nKeys; n++ )
        {
            ByteString aKey = aConfig.GetKeyName( n );
            ByteString aValue = aConfig.ReadKey( aKey );
            // tailor key
            filterValue( aValue, aGroup, aKey );

            aFiles[aKey][aGroup] = ByteString( aValue );
        }
    }

    for( hash_map< ByteString, hash_map< OString, OString, OStringHash >, OStringHash >::const_iterator lang_it = aFiles.begin(); lang_it != aFiles.end(); ++lang_it )
    {
        ByteString aFile( argv[2] );
        aFile.Append( '.' );
        aFile.Append( lang_it->first );
        FILE* fp = fopen( aFile.GetBuffer(), "w" );
        if( ! fp )
        {
            fprintf( stderr, "Error: could not open \"%s\" for writing: %s\n",
                     aFile.GetBuffer(), strerror( errno ) );
            exit(1);
        }
        for( hash_map< OString, OString, OStringHash >::const_iterator line_it = lang_it->second.begin(); line_it != lang_it->second.end(); ++line_it )
        {
            fprintf( fp, "%s=%s\n", line_it->first.getStr(), line_it->second.getStr() );
        }
        fclose( fp );
    }

    return 0;
}
