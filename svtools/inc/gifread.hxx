/*************************************************************************
 *
 *  $RCSfile: gifread.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _GIFREAD_HXX
#define _GIFREAD_HXX

#ifdef VCL

#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _BMPACC_HXX
#include <vcl/bmpacc.hxx>
#endif

#else // VCL

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _FLTDEFS_HXX
#include "fltdefs.hxx"
#endif

#endif // VCL

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

#ifdef VCL

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
    ULONG               nLogWidth100;
    ULONG               nLogHeight100;
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

                        GIFReader( SvStream& rStm, void* pCallData );
    virtual             ~GIFReader();
};

#else // VCL

class GIFReader : public GraphicReader
{
    Graphic             aImGraphic;
    Animation           aAnimation;
    SvStream&           rIStm;
    void*               pCallerData;
    ULONG*              pGPalette;              // Format: 0x00RRGGBB, Groesse des Feldes: immer 256
    BYTE*               pPalEntryFlag;
    PDIBBYTE            pDIB;
    PDIBBYTE            pDIBBytes;
    PDIBBYTE            pMonoDIB;
    PDIBBYTE            pMonoDIBBytes;
    PDIBBYTE            pSrcBuf;
    PDIBBYTE            pRow8;
    PDIBBYTE            pRow1;
    PDIBBYTE            pFile;
    PDIBBYTE            pMonoFile;
    GIFLZWDecompressor* pDecomp;
    long                nLastPos;
    long                nWidthAl8;
    long                nWidthAl1;
    long                nTotal;
    long                nMonoTotal;
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
    USHORT              nGPaletteSize;          // Anzahl der belegten Eintraege in pPalette
    USHORT              nBlackIndex;
    USHORT              nLastInterCount;
    USHORT              nLoops;
    GIFAction           eActAction;
    BOOL                bStatic;                // zeigt an, ob die GIF-Grafik animiert oder statisch ist
    BOOL                bStatus;
    BOOL                bDestTransparent;
    BOOL                bLocalDestTransparent;
    BOOL                bGCTransparent;         // Ob das Bild Transparent ist, wenn ja:
    BOOL                bInterlaced;
    BOOL                bOverreadBlock;
    BOOL                bImGraphicReady;
    BOOL                bGlobalPalette;
    BOOL                bGIF89;
    BYTE                nBackgroundColor;       // Hintergrundfarbe
    BYTE                nGCTransparentIndex;    // Pixel von diesem Index sind durchsichtig
    BYTE                nGCDisposalMethod;      // 'Disposal Method' (siehe GIF-Doku)

    USHORT              ReadPaletteEntries( ULONG* pPal, USHORT nNumEntries );
    void                ClearImageExtensions();
    BOOL                CreateBitmaps( long nWidth, long nHeight, ULONG* pDIBPal,
                                       USHORT nPalCount, BOOL bWatchForBackgroundColor );
    BOOL                ReadGlobalHeader();
    BOOL                ReadExtension();
    BOOL                ReadLocalHeader();
    USHORT              ReadNextBlock();
    void                FillImages( PDIBBYTE pBytes, ULONG nCount );
    void                CreateNewBitmaps();
    BOOL                ProcessGIF();

public:

    ReadState           ReadGIF( Graphic& rGraphic );
    const Graphic&      GetIntermediateGraphic();

                        GIFReader( SvStream& rStm, void* pCallData );
    virtual             ~GIFReader();
};

#endif // VCL
#endif // _GIFPRIVATE

// -------------
// - ImportGIF -
// -------------

 BOOL ImportGIF( SvStream& rStream, Graphic& rGraphic, void* pCallerData );

#endif // _GIFREAD_HXX
