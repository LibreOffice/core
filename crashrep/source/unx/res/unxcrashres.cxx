/*************************************************************************
 *
 *  $RCSfile: unxcrashres.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-26 03:10:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
