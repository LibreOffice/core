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

#ifndef INCLUDED_VCL_SOURCE_FILTER_IGIF_GIFREAD_HXX
#define INCLUDED_VCL_SOURCE_FILTER_IGIF_GIFREAD_HXX

#include <vcl/graphic.hxx>
#include <vcl/bitmapaccess.hxx>

#ifdef _GIFPRIVATE

enum GIFAction
{
    GLOBAL_HEADER_READING,
    MARKER_READING,
    EXTENSION_READING,
    LOCAL_HEADER_READING,
    FIRST_BLOCK_READING,
    NEXT_BLOCK_READING,
    ABORT_READING,
    END_READING
};

enum ReadState
{
    GIFREAD_OK,
    GIFREAD_ERROR,
    GIFREAD_NEED_MORE
};

class GIFLZWDecompressor;

class SvStream;

class GIFReader : public GraphicReader
{
    Graphic             aImGraphic;
    Animation           aAnimation;
    Bitmap              aBmp8;
    Bitmap              aBmp1;
    BitmapPalette       aGPalette;
    BitmapPalette       aLPalette;
    SvStream&           rIStm;
    sal_uInt8*          pSrcBuf;
    GIFLZWDecompressor* pDecomp;
    BitmapWriteAccess*  pAcc8;
    BitmapWriteAccess*  pAcc1;
    long                nYAcc;
    long                nLastPos;
    sal_uInt32          nLogWidth100;
    sal_uInt32          nLogHeight100;
    sal_uInt16          nTimer;
    sal_uInt16          nGlobalWidth;           // maximum imagewidth from header
    sal_uInt16          nGlobalHeight;          // maximum imageheight from header
    sal_uInt16          nImageWidth;            // maximum screenwidth from header
    sal_uInt16          nImageHeight;           // maximum screenheight from header
    sal_uInt16          nImagePosX;
    sal_uInt16          nImagePosY;
    sal_uInt16          nImageX;                // maximum screenwidth from header
    sal_uInt16          nImageY;                // maximum screenheight from header
    sal_uInt16          nLastImageY;
    sal_uInt16          nLastInterCount;
    sal_uInt16          nLoops;
    GIFAction           eActAction;
    bool                bStatus;
    bool                bGCTransparent;         // is the image transparent, if yes:
    bool                bInterlaced;
    bool                bOverreadBlock;
    bool                bImGraphicReady;
    bool                bGlobalPalette;
    sal_uInt8           nBackgroundColor;       // backgroundcolour
    sal_uInt8           nGCTransparentIndex;    // pixels of this index are transparant
    sal_uInt8           nGCDisposalMethod;      // 'Disposal Method' (see GIF docs)
    sal_uInt8           cTransIndex1;
    sal_uInt8           cNonTransIndex1;

    void                ReadPaletteEntries( BitmapPalette* pPal, sal_uLong nCount );
    void                ClearImageExtensions();
    bool                CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal, bool bWatchForBackgroundColor );
    bool                ReadGlobalHeader();
    bool                ReadExtension();
    bool                ReadLocalHeader();
    sal_uLong           ReadNextBlock();
    void                FillImages( sal_uInt8* pBytes, sal_uLong nCount );
    void                CreateNewBitmaps();
    bool                ProcessGIF();

public:

    ReadState           ReadGIF( Graphic& rGraphic );
    const Graphic&      GetIntermediateGraphic();

    explicit            GIFReader( SvStream& rStm );
    virtual             ~GIFReader();
};

#endif // _GIFPRIVATE

VCL_DLLPUBLIC bool ImportGIF( SvStream& rStream, Graphic& rGraphic );

#endif // INCLUDED_VCL_SOURCE_FILTER_IGIF_GIFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
