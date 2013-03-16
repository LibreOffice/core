/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _JPEG_HXX
#define _JPEG_HXX

#include <vcl/graph.hxx>
#include <vcl/fltcall.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#ifdef _JPEGPRIVATE

enum ReadState
{
    JPEGREAD_OK,
    JPEGREAD_ERROR,
    JPEGREAD_NEED_MORE
};

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

sal_Bool ImportJPEG( SvStream& rStream, Graphic& rGraphic, void* pCallerData, sal_Int32 nImportFlags );

sal_Bool ExportJPEG( SvStream& rStream,
                 const Graphic& rGraphic,
                 const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData,
                 bool* pExportWasGrey = NULL
                );

#endif // _JPEG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
