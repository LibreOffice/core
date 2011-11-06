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
#include <stdio.h>

#include <tools/stream.hxx>
#include <tools/zcodec.hxx>

void show_usage()
{
    fputs("ztool usage: ztool <source> <dest>\n", stderr);
}

int
#ifdef WNT
__cdecl
#endif
main( int argc, char **argv )
{
    if ( argc != 3 )
    {
        show_usage();
        return -1;
    }
    UniString aInName = UniString::CreateFromAscii( argv[1] );
    UniString aOutName = UniString::CreateFromAscii( argv[2] );
    SvFileStream aInFile( aInName, STREAM_READ );
    SvFileStream aOutFile( aOutName, STREAM_WRITE | STREAM_TRUNC );

    ZCodec aCompressor;
    aCompressor.BeginCompression();
    aCompressor.Compress( aInFile, aOutFile );
    aCompressor.EndCompression();

    return 0;
}


