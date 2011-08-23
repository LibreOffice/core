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

class SvStream;

namespace binfilter
{

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

class GIFReader : public GraphicReader
{
    Graphic				aImGraphic;
    Animation			aAnimation;
    Bitmap				aBmp8;
    Bitmap				aBmp1;
    BitmapPalette		aGPalette;
    BitmapPalette		aLPalette;
    SvStream&			rIStm;
    void*				pCallerData;
    HPBYTE				pSrcBuf;
    GIFLZWDecompressor*	pDecomp;
    BitmapWriteAccess*	pAcc8;
    BitmapWriteAccess*	pAcc1;
    long				nYAcc;
    long				nLastPos;
    sal_uInt32			nLogWidth100;
    sal_uInt32			nLogHeight100;
    USHORT				nTimer;
    USHORT				nGlobalWidth;			// maximale Bildbreite aus Header
    USHORT				nGlobalHeight;			// maximale Bildhoehe aus Header
    USHORT				nImageWidth;			// maximale Bildbreite aus Header
    USHORT				nImageHeight;			// maximale Bildhoehe aus Header
    USHORT				nImagePosX;
    USHORT				nImagePosY;
    USHORT				nImageX;				// maximale Bildbreite aus Header
    USHORT				nImageY;				// maximale Bildhoehe aus Header
    USHORT				nLastImageY;
    USHORT				nLastInterCount;
    USHORT				nLoops;
    GIFAction			eActAction;
    BOOL				bStatus;
    BOOL				bGCTransparent;			// Ob das Bild Transparent ist, wenn ja:
    BOOL				bInterlaced;
    BOOL				bOverreadBlock;
    BOOL				bImGraphicReady;
    BOOL				bGlobalPalette;
    BYTE				nBackgroundColor;		// Hintergrundfarbe
    BYTE				nGCTransparentIndex;	// Pixel von diesem Index sind durchsichtig
    BYTE				nGCDisposalMethod;		// 'Disposal Method' (siehe GIF-Doku)
    BYTE				cTransIndex1;
    BYTE				cNonTransIndex1;

    void				ReadPaletteEntries( BitmapPalette* pPal, ULONG nCount );
    void				ClearImageExtensions();
    BOOL				CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal, BOOL bWatchForBackgroundColor );
    BOOL				ReadGlobalHeader();
    BOOL				ReadExtension();
    BOOL				ReadLocalHeader();
    ULONG				ReadNextBlock();
    void				FillImages( HPBYTE pBytes, ULONG nCount );
    void				CreateNewBitmaps();
    BOOL				ProcessGIF();

public:

    ReadState			ReadGIF( Graphic& rGraphic );
    const Graphic&		GetIntermediateGraphic();

                        GIFReader( SvStream& rStm );
    virtual				~GIFReader();
};

#endif // _GIFPRIVATE

// -------------
// - ImportGIF -
// -------------

 BOOL ImportGIF( SvStream& rStream, Graphic& rGraphic );

 }

#endif // _GIFREAD_HXX
