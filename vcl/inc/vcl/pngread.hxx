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



#ifndef _SV_PNGREAD_HXX
#define _SV_PNGREAD_HXX

#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <vector>

// -------------
// - PNGReader -
// -------------

namespace vcl
{
    class PNGReaderImpl;

    class VCL_DLLPUBLIC PNGReader
    {
        PNGReaderImpl*          mpImpl;

    public:

        /* the PNG chunks are read within the c'tor, so the stream will
        be positioned at the end of the PNG */
        explicit PNGReader( SvStream& rStm );
        ~PNGReader();

        /* an empty preview size hint (=default) will read the whole image
        */
        BitmapEx                        Read( const Size& i_rPreviewHint = Size() );

        // retrieve every chunk that resides inside the PNG
        struct ChunkData
        {
            sal_uInt32                  nType;
            std::vector< sal_uInt8 >    aData;
        };
        const std::vector< ChunkData >& GetChunks() const;

        void SetIgnoreGammaChunk( sal_Bool b );
    };
}

#endif // _SV_PNGREAD_HXX
