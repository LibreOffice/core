/*************************************************************************
 *
 *  $RCSfile: svimbase.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:01 $
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

#ifndef _SVIMBASE_HXX
#define _SVIMBASE_HXX

#ifdef VCL

#ifndef _SV_BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX //autogen
#include <vcl/salbtype.hxx>
#endif


// -----------
// - Defines -
// -----------

#ifdef WIN
#define _SVHUGE huge
#else
#define _SVHUGE
#endif

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
    BYTE cBlue;
    BYTE cGreen;
    BYTE cRed;
};

// ------------------------------------------------------------------------

inline BOOL operator==( const SimColor& rCol1, const SimColor& rCol2 )
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
    ULONG       nColors;
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
    ULONG               nWidth1;
    ULONG               nWidth2;
    ULONG               nWidth3;
    ULONG               nWidth4;
    ULONG               nHeight1;
    ULONG               nHeight2;
    ULONG               nHeight3;
    ULONG               nHeight4;
    ULONG               nAlignedWidth1;
    ULONG               nAlignedWidth2;
    ULONG               nAlignedWidth3;
    ULONG               nAlignedWidth4;
    ULONG               nWhichOrg;
    SimDepthType        eOrgDepth;
    SimDepthType        eUndoDepth;
    SimDepthType        eRedoDepth;
    BOOL                bIsUndo;
    BOOL                bIsRedo;
    BOOL                bCreateUndo;
    BOOL                bValid;
    BOOL                bDitherAll;

    HPBYTE              BitmapToArray24( const Bitmap& rBitmap, ULONG* pWidth,
                                         ULONG* pHeight, ULONG* pAlignedWidth,
                                         SfxViewFrame *pFrame = NULL );
    BOOL                Array24ToBitmap( HPBYTE pArray, Bitmap &rBitmap,
                                         const ULONG nWidth, const ULONG nHeight,
                                         const ULONG nColorCount = 256,
                                         ULONG nLast = 0, SfxProgress* pProgress = NULL );

    Bitmap              CreateSaveBitmap( const SimDepthType eDepth, SfxViewFrame *pFrame = NULL );

    HPBYTE              CreateArray24( ULONG nWidth, ULONG nHeight );
    void                DeleteArray( HPBYTE pArray );

                        SvImageBase(const SvImageBase& rSvImageBase);
    const SvImageBase&  operator=(const SvImageBase& rSvImageBase);

public:

                        SvImageBase();
                        SvImageBase( const Bitmap& rBitmap,
                                     const ULONG nColorCount = 256,
                                     ULONG nLast = 0, SfxProgress* pProgress = NULL );
                        ~SvImageBase();

    BOOL                IsValid() { return bValid; }

    ULONG               GetOrgWidth() const { return nWhichOrg == 1 ? nWidth1 : nWidth2; }
    ULONG               GetDestWidth() const { return nWhichOrg == 1 ? nWidth2 : nWidth1; }

    ULONG               GetOrgHeight() const { return nWhichOrg == 1 ? nHeight1 : nHeight2; }
    ULONG               GetDestHeight() const { return nWhichOrg == 1 ? nHeight2 : nHeight1; }

    ULONG               GetOrgAlignedWidth() const { return nWhichOrg == 1 ? nAlignedWidth1 : nAlignedWidth2; }
    ULONG               GetDestAlignedWidth() const { return nWhichOrg == 1 ? nAlignedWidth2 : nAlignedWidth1; }

    ULONG               GetOrgAlignedSize() const { return GetOrgAlignedWidth() * GetOrgHeight(); }
    ULONG               GetDestAlignedSize() const { return GetDestAlignedWidth() * GetDestHeight(); }

    // Farbtiefe des Ausgangsbildes ermitteln und setzen
    SimDepthType        GetDepth() const { return eOrgDepth; }
    void                SetDepth( const SimDepthType eDepth ) { eOrgDepth = eDepth; }

    // Farbtiefen nach Undo und Redo ermitteln und setzen
    SimDepthType        GetUndoDepth() const { return eUndoDepth; }
    void                SetUndoDepth(const SimDepthType eDepth) { eUndoDepth = eDepth; }

    SimDepthType        GetRedoDepth() const { return eRedoDepth; }
    void                SetRedoDepth(const SimDepthType eDepth) { eRedoDepth = eDepth; }

    // Vor- und Ruecklauf der Bildverarbeitung
    BOOL                BeginProcessing( BOOL bUndo = TRUE );
    void                EndProcessing();

    BOOL                BeginProcessingExt(ULONG nWidth, ULONG nHeight, BOOL bUndo = TRUE);
    void                EndProcessingExt() { EndProcessing(); }

    // Zeiger auf Arrays zur Verfuegung stellen
    HPBYTE              GetOrgPointer() { return pOrgArray; }
    HPBYTE              GetDestPointer() { return pDestArray; }

    // DIB-Erzeugung fuer Anzeige
    BOOL                CreateOutBitmap( const ULONG nColorCount = 256, ULONG nLast = 0,
                                         SfxProgress* pProgress = NULL );

    // Undo-Verwaltung
    BOOL                DoUndo( SfxProgress* pProgress = NULL );
    BOOL                DoRedo( SfxProgress* pProgress = NULL );

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

BOOL DitherBitmap( Bitmap& rBitmap, BOOL bDitherAlways = FALSE );

#else // VCL

#ifndef _BITMAP_HXX //autogen
#include <vcl/bitmap.hxx>
#endif
#ifndef _PAL_HXX
#include <vcl/pal.hxx>
#endif
#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif


#undef GetPrinter
#undef SetPrinter

#ifndef _SVHUGE
#ifdef WIN
#define _SVHUGE huge
#else
#define _SVHUGE
#endif // WIN
#endif // _SVHUGE


class SfxViewFrame;
class SfxProgress;


/******************************************************************************/


enum SimDepthType
{
    SIM_DEPTH_1,
    SIM_DEPTH_4,
    SIM_DEPTH_8,
    SIM_DEPTH_24
};


/******************************************************************************/


struct SimColor
{
    BYTE cBlue;
    BYTE cGreen;
    BYTE cRed;
};


BOOL operator==( const SimColor& rCol1, const SimColor& rCol2 );


/******************************************************************************/


struct SimPalette
{
    ULONG       nColors;
    SimColor    aColorArray[ 256 ];
};


/******************************************************************************/


class SvImageBase
{
#if defined(WIN) || defined(WNT)

    HGLOBAL         hArray1;
    HGLOBAL         hArray2;
    HGLOBAL         hArray3;
    HGLOBAL         hArray4;


protected:

    HGLOBAL         BitmapToArray24(const Bitmap& rBitmap, ULONG* pWidth,
                                    ULONG* pHeight, ULONG* pAlignedWidth,
                                    SfxViewFrame* pFrame = NULL);
    BOOL            Array24ToBitmap(HGLOBAL hArray, Bitmap &rBitmap,
                                    const ULONG nWidth, const ULONG nHeight,
                                    const ULONG nColorCount = 256,
                                    ULONG nLast = 0,
                                    SfxProgress* pProgress = NULL);

    HGLOBAL         CreateArray24(ULONG nWidth, ULONG nHeight);
    void            DeleteArray(HGLOBAL hArray);

    //  DIB-Funktionen, die noch nicht frei in SV zur Verfuegung stehen
    HBITMAP         DDBtoDIB(HBITMAP hBitmap, HPALETTE hPalette);
    ULONG           PaletteSize(void* p);

    // RLE-Dekomprimierung
    BOOL            DecompressRLE4(BYTE _SVHUGE* pSrc, BYTE _SVHUGE* pDst,
                                   ULONG nWidth, BITMAPINFO* pBMI);

    BOOL            DecompressRLE8(BYTE _SVHUGE* pSrc, BYTE _SVHUGE* pDst,
                                   ULONG nWidth, BITMAPINFO* pBMI);

#endif // WIN || WNT


/******************************************************************************/


#if defined(OS2) || defined(UNX) || defined(MAC)

    BYTE _SVHUGE*   pArray1;
    BYTE _SVHUGE*   pArray2;
    BYTE _SVHUGE*   pArray3;
    BYTE _SVHUGE*   pArray4;


protected:

    BYTE _SVHUGE*   BitmapToArray24(const Bitmap& rBitmap, ULONG* pWidth,
                                    ULONG* pHeight, ULONG* pAlignedWidth,
                                    SfxViewFrame *pFrame = NULL);
    BOOL            Array24ToBitmap(BYTE _SVHUGE*, Bitmap &rBitmap,
                                    const ULONG nWidth, const ULONG nHeight,
                                    const ULONG nColorCount = 256,
                                    ULONG nLast = 0,
                                    SfxProgress* pProgress = NULL);


    BYTE _SVHUGE*   CreateArray24(ULONG nWidth, ULONG nHeight);
    void            DeleteArray(BYTE _SVHUGE* pArray);

#if defined OS2

    // RLE-Dekomprimierung
    BOOL            DecompressRLE4(BYTE* pSrc, BYTE* pDst,
                                   ULONG nWidth, BmpPaletteEntryOS2* pPal);

    BOOL            DecompressRLE8(BYTE* pSrc, BYTE* pDst,
                                   ULONG nWidth, BmpPaletteEntryOS2* pPal);

#endif // OS2

#endif // OS2 || UNX || MAC


/******************************************************************************/


private:
    Palette             aDithPal;

    BYTE _SVHUGE*       pOrgArray;
    BYTE _SVHUGE*       pDestArray;
    BYTE _SVHUGE*       pUndoArray;
    BYTE _SVHUGE*       pMergeArray;

    Bitmap*             pOutBitmap;

    SimPalette*         pPal1;
    SimPalette*         pPal2;
    SimPalette*         pPal3;
    SimPalette*         pPal4;

    long                nDummy1;
    long                nDummy2;
    long                nDummy3;
    long                nDummy4;

    ULONG               nWidth1;
    ULONG               nWidth2;
    ULONG               nWidth3;
    ULONG               nWidth4;

    ULONG               nHeight1;
    ULONG               nHeight2;
    ULONG               nHeight3;
    ULONG               nHeight4;

    ULONG               nAlignedWidth1;
    ULONG               nAlignedWidth2;
    ULONG               nAlignedWidth3;
    ULONG               nAlignedWidth4;

    USHORT              nWhichOrg;
    BOOL                bIsUndo;
    BOOL                bIsRedo;
    BOOL                bCreateUndo;

    SimDepthType        eOrgDepth;
    SimDepthType        eUndoDepth;
    SimDepthType        eRedoDepth;

    BOOL                bValid;

    BOOL                bDitherAll;


    // !!! steht nicht zur Verfuegung !!!
                        SvImageBase(const SvImageBase& rSvImageBase);
    const SvImageBase&  operator=(const SvImageBase& rSvImageBase);


public:

    SvImageBase();
    SvImageBase(const Bitmap& rBitmap, const ULONG nColorCount = 256, ULONG nLast = 0, SfxProgress* pProgress = NULL);
    ~SvImageBase();

    // alles korrekt erzeugt?
    BOOL        IsValid() { return bValid; }

    // Allgemeine Informationsfunktionen (Bildgroesse etc.)
    ULONG       GetOrgWidth() const     { return nWhichOrg == 1 ? nWidth1 : nWidth2; }
    ULONG       GetDestWidth() const    { return nWhichOrg == 1 ? nWidth2 : nWidth1; }
    ULONG       GetUndoWidth() const    { return nWidth3; }
    ULONG       GetMergeWidth() const   { return nWidth4; }

    ULONG       GetOrgHeight() const    { return nWhichOrg == 1 ? nHeight1 : nHeight2; }
    ULONG       GetDestHeight() const   { return nWhichOrg == 1 ? nHeight2 : nHeight1; }
    ULONG       GetUndoHeight() const   { return nHeight3; }
    ULONG       GetMergeHeight() const  { return nHeight4; }

    ULONG       GetOrgAlignedWidth() const   { return nWhichOrg == 1 ? nAlignedWidth1 : nAlignedWidth2; }
    ULONG       GetDestAlignedWidth() const  { return nWhichOrg == 1 ? nAlignedWidth2 : nAlignedWidth1; }
    ULONG       GetUndoAlignedWidth() const  { return nAlignedWidth3; }
    ULONG       GetMergeAlignedWidth() const { return nAlignedWidth4; }

    ULONG       GetOrgAlignedSize() const   { return GetOrgAlignedWidth() * GetOrgHeight(); }
    ULONG       GetDestAlignedSize() const  { return GetDestAlignedWidth() * GetDestHeight(); }
    ULONG       GetUndoAlignedSize() const  { return GetUndoAlignedWidth() * GetUndoHeight(); }
    ULONG       GetMergeAlignedSize() const { return GetMergeAlignedWidth() * GetMergeHeight(); }

    // Farbtiefe des Ausgangsbildes ermitteln und setzen
    SimDepthType    GetDepth() const { return eOrgDepth; }
    void            SetDepth(const SimDepthType eDepth) { eOrgDepth = eDepth; }

    // Farbtiefen nach Undo und Redo ermitteln und setzen
    SimDepthType    GetUndoDepth() const { return eUndoDepth; }
    SimDepthType    GetRedoDepth() const { return eRedoDepth; }
    void            SetUndoDepth(const SimDepthType eDepth) { eUndoDepth = eDepth; }
    void            SetRedoDepth(const SimDepthType eDepth) { eRedoDepth = eDepth; }

    // Vor- und Ruecklauf der Bildverarbeitung
    BOOL        BeginProcessing(BOOL bUndo = TRUE);
    void        EndProcessing();

    BOOL        BeginProcessingExt(ULONG nWidth, ULONG nHeight, BOOL bUndo = TRUE);
    void        EndProcessingExt();

    BOOL        BeginProcessingMerge(BOOL bUndo = TRUE);
    void        EndProcessingMerge();

    // Zeiger auf Arrays zur Verfuegung stellen
    BYTE _SVHUGE*   GetOrgPointer() { return pOrgArray; }
    BYTE _SVHUGE*   GetDestPointer() { return pDestArray; }
    BYTE _SVHUGE*   GetUndoPointer() { return pUndoArray; }
    BYTE _SVHUGE*   GetMergePointer() { return pMergeArray; }

    // Zeiger auf Ursprungs-Array zur Verfuegung stellen,
    // wenn dieses nicht veraendert werden soll
#if defined(WIN) || defined(WNT)
    HGLOBAL         GetOrgPointerUnchanged() { return (nWhichOrg == 1 ? hArray1 : hArray2); }
#endif // WIN || WNT

#if defined(OS2) || defined(UNX) || defined(MAC)
    BYTE _SVHUGE*   GetOrgPointerUnchanged() { return (nWhichOrg == 1 ? pArray1 : pArray2); }
#endif // OS2 || UNX || MAC

    // Setzen des Ausgangs-DIB's
    void            SetOrgBitmap(const Bitmap& rBitmap) { pOutBitmap = new Bitmap(rBitmap); }

    // DIB-Erzeugung fuer Anzeige
    BOOL            CreateOutBitmap(const ULONG nColorCount = 256, ULONG nLast = 0, SfxProgress* pProgress = NULL );

    // DIB-Erzeugung fuer Speicherung
    Bitmap          CreateSaveBitmap(const SimDepthType eDepth, SfxViewFrame *pFrame = NULL );

    // Undo-Verwaltung
    BOOL            DoUndo( SfxProgress* pProgress = NULL );
    BOOL            DoRedo( SfxProgress* pProgress = NULL );

    // DIB-Rueckgabe fuer Anzeige
    Bitmap&         GetOutBitmap() const;

    // DIB-Rueckgabe fuer Speicherung
    Bitmap          GetSaveBitmap() { return CreateSaveBitmap( eOrgDepth ); }

    // Palette besorgen
    SimPalette*     GetOrgPalette() const { return nWhichOrg == 1 ? pPal1 : pPal2; }
    SimPalette*     GetDestPalette() const { return nWhichOrg == 1 ? pPal2 : pPal1; }
};

// ----------------
// - DitherBitmap -
// ----------------

BOOL DitherBitmap( Bitmap& rBitmap );

#endif // VCL
#endif // _SVIMBASE_HXX
