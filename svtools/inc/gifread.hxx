/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gifread.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:38:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    USHORT              nTimer;
    USHORT              nGlobalWidth;           // maximale Bildbreite aus Header
    USHORT              nGlobalHeight;          // maximale Bildhoehe aus Header
    USHORT              nImageWidth;            // maximale Bildbreite aus Header
    USHORT              nImageHeight;           // maximale Bildhoehe aus Header
    USHORT              nImagePosX;
    USHORT              nImagePosY;
    USHORT              nImageX;                // maximale Bildbreite aus Header
    USHORT              nImageY;                // maximale Bildhoehe aus Header
    USHORT              nLastImageY;
    USHORT              nLastInterCount;
    USHORT              nLoops;
    GIFAction           eActAction;
    BOOL                bStatus;
    BOOL                bGCTransparent;         // Ob das Bild Transparent ist, wenn ja:
    BOOL                bInterlaced;
    BOOL                bOverreadBlock;
    BOOL                bImGraphicReady;
    BOOL                bGlobalPalette;
    BYTE                nBackgroundColor;       // Hintergrundfarbe
    BYTE                nGCTransparentIndex;    // Pixel von diesem Index sind durchsichtig
    BYTE                nGCDisposalMethod;      // 'Disposal Method' (siehe GIF-Doku)
    BYTE                cTransIndex1;
    BYTE                cNonTransIndex1;

    void                ReadPaletteEntries( BitmapPalette* pPal, ULONG nCount );
    void                ClearImageExtensions();
    BOOL                CreateBitmaps( long nWidth, long nHeight, BitmapPalette* pPal, BOOL bWatchForBackgroundColor );
    BOOL                ReadGlobalHeader();
    BOOL                ReadExtension();
    BOOL                ReadLocalHeader();
    ULONG               ReadNextBlock();
    void                FillImages( HPBYTE pBytes, ULONG nCount );
    void                CreateNewBitmaps();
    BOOL                ProcessGIF();

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

 BOOL ImportGIF( SvStream& rStream, Graphic& rGraphic );

#endif // _GIFREAD_HXX
