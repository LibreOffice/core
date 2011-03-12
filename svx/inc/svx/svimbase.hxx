/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVIMBASE_HXX
#define _SVIMBASE_HXX

#include <vcl/bitmap.hxx>
#include <vcl/salbtype.hxx>


// -----------
// - Defines -
// -----------

#define _SVHUGE

// ----------------
// - SimDepthType -
// ----------------

enum SimDepthType
{
    SIM_DEPTH_1,
    SIM_DEPTH_4,
    SIM_DEPTH_8,
    SIM_DEPTH_24
};

// ------------
// - SimColor -
// ------------

struct SimColor
{
    sal_uInt8 cBlue;
    sal_uInt8 cGreen;
    sal_uInt8 cRed;
};

// ------------------------------------------------------------------------

inline sal_Bool operator==( const SimColor& rCol1, const SimColor& rCol2 )
{
    return ( ( rCol1.cRed == rCol2.cRed ) &&
             ( rCol1.cGreen == rCol2.cGreen ) &&
             ( rCol1.cBlue == rCol2.cBlue ) );
}

// --------------
// - SimPalette -
// --------------

struct SimPalette
{
    sal_uIntPtr     nColors;
    SimColor    aColorArray[ 256 ];
};

// ---------------
// - SvImageBase -
// ---------------

class SfxViewFrame;
class SfxProgress;

class SvImageBase
{
private:

    Bitmap              aOutBitmap;
    HPBYTE              pArray1;
    HPBYTE              pArray2;
    HPBYTE              pArray3;
    HPBYTE              pArray4;
    HPBYTE              pOrgArray;
    HPBYTE              pDestArray;
    SimPalette*         pPal1;
    SimPalette*         pPal2;
    SimPalette*         pPal3;
    SimPalette*         pPal4;
    sal_uIntPtr             nWidth1;
    sal_uIntPtr             nWidth2;
    sal_uIntPtr             nWidth3;
    sal_uIntPtr             nWidth4;
    sal_uIntPtr             nHeight1;
    sal_uIntPtr             nHeight2;
    sal_uIntPtr             nHeight3;
    sal_uIntPtr             nHeight4;
    sal_uIntPtr             nAlignedWidth1;
    sal_uIntPtr             nAlignedWidth2;
    sal_uIntPtr             nAlignedWidth3;
    sal_uIntPtr             nAlignedWidth4;
    sal_uIntPtr             nWhichOrg;
    SimDepthType        eOrgDepth;
    SimDepthType        eUndoDepth;
    SimDepthType        eRedoDepth;
    sal_Bool                bIsUndo;
    sal_Bool                bIsRedo;
    sal_Bool                bCreateUndo;
    sal_Bool                bValid;
    sal_Bool                bDitherAll;

    HPBYTE              BitmapToArray24( const Bitmap& rBitmap, sal_uIntPtr* pWidth,
                                         sal_uIntPtr* pHeight, sal_uIntPtr* pAlignedWidth,
                                         SfxViewFrame *pFrame = NULL );
    sal_Bool                Array24ToBitmap( HPBYTE pArray, Bitmap &rBitmap,
                                         const sal_uIntPtr nWidth, const sal_uIntPtr nHeight,
                                         const sal_uIntPtr nColorCount = 256,
                                         sal_uIntPtr nLast = 0, SfxProgress* pProgress = NULL );

    Bitmap              CreateSaveBitmap( const SimDepthType eDepth, SfxViewFrame *pFrame = NULL );

    HPBYTE              CreateArray24( sal_uIntPtr nWidth, sal_uIntPtr nHeight );
    void                DeleteArray( HPBYTE pArray );

                        SvImageBase(const SvImageBase& rSvImageBase);
    const SvImageBase&  operator=(const SvImageBase& rSvImageBase);

public:

                        SvImageBase();
                        SvImageBase( const Bitmap& rBitmap,
                                     const sal_uIntPtr nColorCount = 256,
                                     sal_uIntPtr nLast = 0, SfxProgress* pProgress = NULL );
                        ~SvImageBase();

    sal_Bool                IsValid() { return bValid; }

    sal_uIntPtr             GetOrgWidth() const { return nWhichOrg == 1 ? nWidth1 : nWidth2; }
    sal_uIntPtr             GetDestWidth() const { return nWhichOrg == 1 ? nWidth2 : nWidth1; }

    sal_uIntPtr             GetOrgHeight() const { return nWhichOrg == 1 ? nHeight1 : nHeight2; }
    sal_uIntPtr             GetDestHeight() const { return nWhichOrg == 1 ? nHeight2 : nHeight1; }

    sal_uIntPtr             GetOrgAlignedWidth() const { return nWhichOrg == 1 ? nAlignedWidth1 : nAlignedWidth2; }
    sal_uIntPtr             GetDestAlignedWidth() const { return nWhichOrg == 1 ? nAlignedWidth2 : nAlignedWidth1; }

    sal_uIntPtr             GetOrgAlignedSize() const { return GetOrgAlignedWidth() * GetOrgHeight(); }
    sal_uIntPtr             GetDestAlignedSize() const { return GetDestAlignedWidth() * GetDestHeight(); }

    // Get and set the color depth of the origin picture
    SimDepthType        GetDepth() const { return eOrgDepth; }
    void                SetDepth( const SimDepthType eDepth ) { eOrgDepth = eDepth; }

    // Get and set the color depth after Undo and Redo
    SimDepthType        GetUndoDepth() const { return eUndoDepth; }
    void                SetUndoDepth(const SimDepthType eDepth) { eUndoDepth = eDepth; }

    SimDepthType        GetRedoDepth() const { return eRedoDepth; }
    void                SetRedoDepth(const SimDepthType eDepth) { eRedoDepth = eDepth; }

    sal_Bool                BeginProcessing( sal_Bool bUndo = sal_True );
    void                EndProcessing();

    sal_Bool                BeginProcessingExt(sal_uIntPtr nWidth, sal_uIntPtr nHeight, sal_Bool bUndo = sal_True);
    void                EndProcessingExt() { EndProcessing(); }

    HPBYTE              GetOrgPointer() { return pOrgArray; }
    HPBYTE              GetDestPointer() { return pDestArray; }

    sal_Bool                CreateOutBitmap( const sal_uIntPtr nColorCount = 256, sal_uIntPtr nLast = 0,
                                         SfxProgress* pProgress = NULL );

    sal_Bool                DoUndo( SfxProgress* pProgress = NULL );
    sal_Bool                DoRedo( SfxProgress* pProgress = NULL );

    const Bitmap&       GetOutBitmap() const;

    Bitmap              GetSaveBitmap();

    SimPalette*         GetOrgPalette() const { return nWhichOrg == 1 ? pPal1 : pPal2; }
    SimPalette*         GetDestPalette() const { return nWhichOrg == 1 ? pPal2 : pPal1; }
};

// ----------------
// - DitherBitmap -
// ----------------

sal_Bool DitherBitmap( Bitmap& rBitmap, sal_Bool bDitherAlways = sal_False );

#endif // _SVIMBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
