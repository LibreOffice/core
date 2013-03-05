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

#ifndef _SGFBRAM_HXX
#define _SGFBRAM_HXX

#include <tools/solar.h>

#define SgfBitImag0   1 /* Bitmap                      */
#define SgfBitImag1   4 /* Bitmap                      */
#define SgfBitImag2   5 /* Bitmap                      */
#define SgfBitImgMo   6 /* Monochrome Bitmap           */
#define SgfSimpVect   2 /* Einfaches Vectorformat      */
#define SgfPostScrp   3 /* Postscript file             */
#define SgfStarDraw   7 /* StarDraw SGV-Datei          */
#define SgfDontKnow 255 /* Unbekannt oder kein SGF/SGV */

// Konstanten fr SgfHeader.SwGrCol
#define SgfBlckWhit 1 /* Schwarz/Wei?Bild                  Ä¿ SimpVector,  */
#define SgfGrayscal 2 /* Bild mit Graustufen                 ?StarDraw und */
#define Sgf16Colors 3 /* Farbbild (16 Farben)               ÄÙ Bit Image    */
#define SgfVectFarb 4 /* Farben fr Linien verwenden        Ä¿              */
#define SgfVectGray 5 /* Graustufen fr Linien verwenden     ?Nur fr      */
#define SgfVectWdth 6 /* Strichst„rken fr Linien verwenden ÄÙ SimpVector   */


#define SgfHeaderSize 42
class SgfHeader
{
public:
    sal_uInt16 Magic;
    sal_uInt16 Version;
    sal_uInt16 Typ;
    sal_uInt16 Xsize;
    sal_uInt16 Ysize;
    sal_Int16  Xoffs;
    sal_Int16  Yoffs;
    sal_uInt16 Planes;    // Layer
    sal_uInt16 SwGrCol;
    char   Autor[10];
    char   Programm[10];
    sal_uInt16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (38 mod 4 =2) !

    sal_uInt32 GetOffset();
    friend SvStream& operator>>(SvStream& rIStream, SgfHeader& rHead);
    sal_Bool   ChkMagic();
};

#define SgfEntrySize 22
class SgfEntry
{
public:
    sal_uInt16 Typ;
    sal_uInt16 iFrei;
    sal_uInt16 lFreiLo,lFreiHi;
    char   cFrei[10];
    sal_uInt16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (18 mod 4 =2) !

    sal_uInt32 GetOffset();
    friend SvStream& operator>>(SvStream& rIStream, SgfEntry& rEntr);
};

#define SgfVectorSize 10
class SgfVector
{
public:
    sal_uInt16 Flag;
    sal_Int16 x;
    sal_Int16 y;
    sal_uInt16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (6 mod 4 =2) !

    friend SvStream& operator>>(SvStream& rIStream, SgfVector& rEntr);
};

extern long SgfVectXofs;
extern long SgfVectYofs;
extern long SgfVectXmul;
extern long SgfVectYmul;
extern long SgfVectXdiv;
extern long SgfVectYdiv;
extern sal_Bool SgfVectScal;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows BMP /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#define BmpFileHeaderSize 14
class BmpFileHeader
{
public:
    sal_uInt16 Typ;            // = "BM"
    sal_uInt16 SizeLo,SizeHi;  // Filesize in Bytes
    sal_uInt16 Reserve1;       // Reserviert
    sal_uInt16 Reserve2;       // Reserviert
    sal_uInt16 OfsLo,OfsHi;    // Offset?

    void   SetSize(sal_uInt32 Size);
    void   SetOfs(sal_uInt32 Size);
    sal_uInt32 GetOfs();
    friend SvStream& operator<<(SvStream& rOStream, BmpFileHeader& rHead);
};

#define BmpInfoHeaderSize 40
class BmpInfoHeader
{
public:
    sal_uInt32 Size;       // Gr”áe des BmpInfoHeaders
    sal_Int32  Width;      // Breite in Pixel
    sal_Int32  Hight;      // H”he in Pixel
    sal_uInt16 Planes;     // Anzahl der Planes (immer 1)
    sal_uInt16 PixBits;    // Anzahl der Bit je Pixel (1,4,8,oder 24)
    sal_uInt32 Compress;   // Datenkompression
    sal_uInt32 ImgSize;    // Gr”áe der Images in Bytes. Ohne Kompression ist auch 0 erlaubt.
    sal_Int32  xDpmm;      // Dot per Meter (0 ist erlaubt)
    sal_Int32  yDpmm;      // Dot per Meter (0 ist erlaubt)
    sal_uInt32 ColUsed;    // Anzahl der verwendeten Farben (0=alle)
    sal_uInt32 ColMust;    // Anzahl der wichtigen Farben (0=alle)

    friend SvStream& operator<<(SvStream& rOStream, BmpInfoHeader& rHead);
};

#define RGBQuadSize 4
class RGBQuad {
private:
    sal_uInt8 Red;
    sal_uInt8 Grn;
    sal_uInt8 Blu;
    sal_uInt8 Fil;
public:
    RGBQuad(sal_uInt8 R, sal_uInt8 G, sal_uInt8 B) { Red=R; Grn=G; Blu=B; Fil=0; }
};

#endif //_SGFBRAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
