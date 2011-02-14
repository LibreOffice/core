/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _GIFREAD_HXX
#define _GIFREAD_HXX

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

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
    void*               pCallerData;
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
