#include <tools/config.hxx>
#include <rtl/string.hxx>
#include <osl/thread.h>
#include <tools/l2txtenc.hxx>

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
    hash_map< int, hash_map< OString, OString, OStringHash > > aFiles;

    for( USHORT i = 0; i < aConfig.GetGroupCount(); i++ )
    {
        aConfig.SetGroup( aConfig.GetGroupName( i ) );
        OString aGroup = aConfig.GetGroupName( i );
        USHORT nKeys = aConfig.GetKeyCount();
        for( USHORT n = 0; n < nKeys; n++ )
        {
            ByteString aKey = aConfig.GetKeyName( n );
            ByteString aValue = aConfig.ReadKey( aKey );
            int nRes = aKey.ToInt32();
            // tailor key
            filterValue( aValue, aGroup, aKey );

            aFiles[nRes][aGroup] = ByteString( String( aValue, Langcode2TextEncoding( (USHORT)nRes ) ), RTL_TEXTENCODING_UTF8 );
        }
    }

    for( hash_map< int, hash_map< OString, OString, OStringHash > >::const_iterator lang_it = aFiles.begin(); lang_it != aFiles.end(); ++lang_it )
    {
        ByteString aFile( argv[2] );
        aFile.Append( '.' );
        if( lang_it->first < 10 )
            aFile.Append( '0' );
        aFile.Append( ByteString::CreateFromInt32( lang_it->first ) );
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
