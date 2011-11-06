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

    // Farbtiefe des Ausgangsbildes ermitteln und setzen
    SimDepthType        GetDepth() const { return eOrgDepth; }
    void                SetDepth( const SimDepthType eDepth ) { eOrgDepth = eDepth; }

    // Farbtiefen nach Undo und Redo ermitteln und setzen
    SimDepthType        GetUndoDepth() const { return eUndoDepth; }
    void                SetUndoDepth(const SimDepthType eDepth) { eUndoDepth = eDepth; }

    SimDepthType        GetRedoDepth() const { return eRedoDepth; }
    void                SetRedoDepth(const SimDepthType eDepth) { eRedoDepth = eDepth; }

    // Vor- und Ruecklauf der Bildverarbeitung
    sal_Bool                BeginProcessing( sal_Bool bUndo = sal_True );
    void                EndProcessing();

    sal_Bool                BeginProcessingExt(sal_uIntPtr nWidth, sal_uIntPtr nHeight, sal_Bool bUndo = sal_True);
    void                EndProcessingExt() { EndProcessing(); }

    // Zeiger auf Arrays zur Verfuegung stellen
    HPBYTE              GetOrgPointer() { return pOrgArray; }
    HPBYTE              GetDestPointer() { return pDestArray; }

    // DIB-Erzeugung fuer Anzeige
    sal_Bool                CreateOutBitmap( const sal_uIntPtr nColorCount = 256, sal_uIntPtr nLast = 0,
                                         SfxProgress* pProgress = NULL );

    // Undo-Verwaltung
    sal_Bool                DoUndo( SfxProgress* pProgress = NULL );
    sal_Bool                DoRedo( SfxProgress* pProgress = NULL );

    // DIB-Rueckgabe fuer Anzeige
    const Bitmap&       GetOutBitmap() const;

    // DIB-Rueckgabe fuer Speicherung
    Bitmap              GetSaveBitmap();

    // Palette besorgen
    SimPalette*         GetOrgPalette() const { return nWhichOrg == 1 ? pPal1 : pPal2; }
    SimPalette*         GetDestPalette() const { return nWhichOrg == 1 ? pPal2 : pPal1; }
};

// ----------------
// - DitherBitmap -
// ----------------

sal_Bool DitherBitmap( Bitmap& rBitmap, sal_Bool bDitherAlways = sal_False );

#endif // _SVIMBASE_HXX
