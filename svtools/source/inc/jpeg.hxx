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



#ifndef _JPEG_HXX
#define _JPEG_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#include <svtools/fltcall.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#ifdef _JPEGPRIVATE

// --------
// - Enum -
// --------

enum ReadState
{
    JPEGREAD_OK,
    JPEGREAD_ERROR,
    JPEGREAD_NEED_MORE
};

// --------------
// - JPEGReader -
// --------------

class JPEGReader : public GraphicReader
{
    SvStream&           rIStm;
    Bitmap              aBmp;
    Bitmap              aBmp1;
    BitmapWriteAccess*  pAcc;
    BitmapWriteAccess*  pAcc1;
    void*               pBuffer;
    long                nLastPos;
    long                nFormerPos;
    long                nLastLines;
    sal_Bool            bSetLogSize;

    Graphic             CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );
    void                FillBitmap();

public:

    void*               CreateBitmap( void* JPEGCreateBitmapParam );

public:



                        JPEGReader( SvStream& rStm, void* pCallData, sal_Bool bSetLogSize );
    virtual             ~JPEGReader();


    ReadState           Read( Graphic& rGraphic );
};

// --------------
// - JPEGWriter -
// --------------

class JPEGWriter
{
    SvStream&           rOStm;
    Bitmap              aBmp;
    BitmapReadAccess*   pAcc;
    sal_uInt8*              pBuffer;
    sal_Bool                bNative;

    sal_Bool            bGreys;
    sal_Int32           nQuality;

    bool*               pExpWasGrey;

    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

    void*               GetScanline( long nY );

                        JPEGWriter( SvStream& rOStm, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData,
                                    bool* pExportWasGrey = NULL );
                        ~JPEGWriter() {};

    sal_Bool                Write( const Graphic& rGraphic );
};

#endif // _JPEGPRIVATE

// ---------------------
// - Import/ExportJPEG -
// ---------------------

sal_Bool ImportJPEG( SvStream& rStream, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags );

sal_Bool ExportJPEG( SvStream& rStream,
                 const Graphic& rGraphic,
                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData,
                 bool* pExportWasGrey = NULL
                );

#endif // _JPEG_HXX
