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

#ifndef INCLUDED_VCL_SOURCE_FILTER_JPEG_JPEGREADER_HXX
#define INCLUDED_VCL_SOURCE_FILTER_JPEG_JPEGREADER_HXX

#include <vcl/graph.hxx>
#include <vcl/fltcall.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

enum ReadState
{
    JPEGREAD_OK,
    JPEGREAD_ERROR,
    JPEGREAD_NEED_MORE
};

struct JPEGCreateBitmapParam
{
    unsigned long nWidth;
    unsigned long nHeight;
    unsigned long density_unit;
    unsigned long X_density;
    unsigned long Y_density;
    long     bGray;

    long     nAlignedWidth;  // these members will be filled by the
    bool     bTopDown;      // CreateBitmap method in svtools
};

class JPEGReader : public GraphicReader
{
    SvStream&           mrStream;
    Bitmap              maBmp;
    Bitmap              maBmp1;
    BitmapWriteAccess*  mpAcc;
    BitmapWriteAccess*  mpAcc1;
    unsigned char *     mpBuffer;
    long                mnLastPos;
    long                mnFormerPos;
    long                mnLastLines;
    bool                mbSetLogSize;

    Graphic CreateIntermediateGraphic( const Bitmap& rBitmap, long nLines );
    void    FillBitmap();

public:
            JPEGReader( SvStream& rStream, void* pCallData, bool bSetLogSize );
    virtual ~JPEGReader();

    ReadState   Read( Graphic& rGraphic );
    unsigned char * CreateBitmap( JPEGCreateBitmapParam& param );
};

#endif // INCLUDED_VCL_SOURCE_FILTER_JPEG_JPEGREADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
