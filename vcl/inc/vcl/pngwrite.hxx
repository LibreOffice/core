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



#ifndef _SV_PNGWRITE_HXX
#define _SV_PNGWRITE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <vector>

// -------------
// - PNGWriter -
// -------------

namespace vcl
{
    class PNGWriterImpl;

    class VCL_DLLPUBLIC PNGWriter
    {
        PNGWriterImpl*          mpImpl;

    public:

        explicit PNGWriter( const BitmapEx&,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData = NULL );
        ~PNGWriter();

        sal_Bool Write( SvStream& rStm );

        // additional method to be able to modify all chunk before they are stored
        struct ChunkData
        {
            sal_uInt32                  nType;
            std::vector< sal_uInt8 >    aData;
        };
        std::vector< vcl::PNGWriter::ChunkData >& GetChunks();
    };
}

#endif // _SV_PNGWRITE_HXX
