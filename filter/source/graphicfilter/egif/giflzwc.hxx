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



#ifndef _GIFLZWC_HXX
#define _GIFLZWC_HXX

#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

// --------------------
// - GIFLZWCompressor -
// --------------------

class   GIFImageDataOutputStream;
struct  GIFLZWCTreeNode;

class GIFLZWCompressor
{
private:

    GIFImageDataOutputStream*   pIDOS;
    GIFLZWCTreeNode*            pTable;
    GIFLZWCTreeNode*            pPrefix;
    sal_uInt16                      nDataSize;
    sal_uInt16                      nClearCode;
    sal_uInt16                      nEOICode;
    sal_uInt16                      nTableSize;
    sal_uInt16                      nCodeSize;

public:

                                GIFLZWCompressor();
                                ~GIFLZWCompressor();

    void                        StartCompression( SvStream& rGIF, sal_uInt16 nPixelSize );
    void                        Compress( HPBYTE pSrc, sal_uLong nSize );
    void                        EndCompression();
};

#endif // _GIFLZWC_HXX
