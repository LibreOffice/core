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



#ifndef _XBMREAD_HXX
#define _XBMREAD_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#ifdef _XBMPRIVATE

// ---------
// - Enums -
// ---------

enum XBMFormat
{
    XBM10,
    XBM11
};

enum ReadState
{
    XBMREAD_OK,
    XBMREAD_ERROR,
    XBMREAD_NEED_MORE
};

// -------------
// - XBMReader -
// -------------

class XBMReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp1;
    BitmapWriteAccess*  pAcc1;
    short*              pHexTable;
    BitmapColor         aWhite;
    BitmapColor         aBlack;
    long                nLastPos;
    long                nWidth;
    long                nHeight;
    sal_Bool                bStatus;

    void                InitTable();
    ByteString          FindTokenLine( SvStream* pInStm, const char* pTok1,
                                       const char* pTok2 = NULL, const char* pTok3 = NULL );
    long                ParseDefine( const sal_Char* pDefine );
    sal_Bool                ParseData( SvStream* pInStm, const ByteString& aLastLine, XBMFormat eFormat );


public:

                        XBMReader( SvStream& rStm );
    virtual             ~XBMReader();

    ReadState           ReadXBM( Graphic& rGraphic );
};

#endif // _XBMPRIVATE

// -------------
// - ImportXBM -
// -------------

sal_Bool ImportXBM( SvStream& rStream, Graphic& rGraphic );

#endif // _XBMREAD_HXX
