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

#ifndef _GIFREAD_HXX
#define _GIFREAD_HXX

#include <vcl/graph.hxx>
#include <vcl/bmpacc.hxx>

#ifdef _GIFPRIVATE

// ---------
// - Enums -
// ---------

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

// ------------------------------------------------------------------------

enum ReadState
{
    GIFREAD_OK,
    GIFREAD_ERROR,
    GIFREAD_NEED_MORE
};

// -------------
// - GIFReader -
// -------------

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
    HPBYTE              pSrcBuf;
    GIFLZWDecompressor* pDecomp;
    BitmapWriteAccess*  pAcc8;
    BitmapWriteAccess*  pAcc1;
    long                nYAcc;
    long                nLastPos;
    sal_uInt32          nLogWidth100;
    sal_uInt32          nLogHeight100;
    sal_uInt16              nTimer;
    sal_uInt16              nGlobalWidth;           // maximale Bildbreite aus Header
    sal_uInt16              nGlobalHeight;          // maximale Bildhoehe aus Header
    sal_uInt16              nImageWidth;            // maximale Bildbreite aus Header
    sal_uInt16              nImageHeight;           // maximale Bildhoehe aus Header
    sal_uInt16              nImagePosX;
    sal_uInt16              nImagePosY;
    sal_uInt16              nImageX;                // maximale Bildbreite aus Header
    sal_uInt16              nImageY;                // maximale Bildhoehe aus Header
    sal_uInt16              nLastImageY;
    sal_uInt16              nLastInterCount;
    sal_uInt16              nLoops;
    GIFAction           eActAction;
    sal_Bool                bStatus;
    sal_Bool                bGCTransparent;         // Ob das Bild Transparent ist, wenn ja:
    sal_Bool                bInterlaced;
    sal_Bool                bOverreadBlock;
    sal_Bool                bImGraphicReady;
    sal_Bool                bGlobalPalette;
    sal_uInt8               nBackgroundColor;       // Hintergrundfarbe
    sal_uInt8               nGCTransparentIndex;    // Pixel von diesem Index sind durchsichtig
    sal_uInt8               nGCDisposalMethod;      // 'Disposal Method' (siehe GIF-Doku)
    sal_uInt8               cTransIndex1;
    sal_uInt8               cNonTransIndex1;

    void                ReadPaletteEntries( BitmapPalette* pPal, sal_uLong nCount );
    void                ClearImageExtensions();
    sal_Bool                CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal, sal_Bool bWatchForBackgroundColor );
    sal_Bool                ReadGlobalHeader();
    sal_Bool                ReadExtension();
    sal_Bool                ReadLocalHeader();
    sal_uLong               ReadNextBlock();
    void                FillImages( HPBYTE pBytes, sal_uLong nCount );
    void                CreateNewBitmaps();
    sal_Bool                ProcessGIF();

public:

    ReadState           ReadGIF( Graphic& rGraphic );
    const Graphic&      GetIntermediateGraphic();

                        GIFReader( SvStream& rStm );
    virtual             ~GIFReader();
};

#endif // _GIFPRIVATE

// -------------
// - ImportGIF -
// -------------

 sal_Bool ImportGIF( SvStream& rStream, Graphic& rGraphic );

#endif // _GIFREAD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
