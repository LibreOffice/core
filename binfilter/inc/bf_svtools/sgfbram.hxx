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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

namespace binfilter
{

#define SgfBitImag0   1 /* Bitmap                      */
#define SgfBitImag1   4 /* Bitmap                      */
#define SgfBitImag2   5 /* Bitmap                      */
#define SgfBitImgMo   6 /* Monochrome Bitmap           */
#define SgfSimpVect   2 /* Einfaches Vectorformat      */
#define SgfPostScrp   3 /* Postscript file             */
#define SgfStarDraw   7 /* StarDraw SGV-Datei          */
#define SgfDontKnow 255 /* Unbekannt oder kein SGF/SGV */

// Konstanten fr SgfHeader.SwGrCol
#define SgfBlckWhit 1 /* Schwarz/Weiá Bild                  Ä¿ SimpVector,  */
#define SgfGrayscal 2 /* Bild mit Graustufen                 ³ StarDraw und */
#define Sgf16Colors 3 /* Farbbild (16 Farben)               ÄÙ Bit Image    */
#define SgfVectFarb 4 /* Farben fr Linien verwenden        Ä¿              */
#define SgfVectGray 5 /* Graustufen fr Linien verwenden     ³ Nur fr      */
#define SgfVectWdth 6 /* Strichst„rken fr Linien verwenden ÄÙ SimpVector   */


#define SgfHeaderSize 42
class SgfHeader
{
public:
    UINT16 Magic;
    UINT16 Version;
    UINT16 Typ;
    UINT16 Xsize;
    UINT16 Ysize;
    INT16  Xoffs;
    INT16  Yoffs;
    UINT16 Planes;    // Layer
    UINT16 SwGrCol;
    char   Autor[10];
    char   Programm[10];
    UINT16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (38 mod 4 =2) !

    UINT32 GetOffset();
    friend SvStream& operator>>(SvStream& rIStream, SgfHeader& rHead);
    BOOL   ChkMagic();
};

#define SgfEntrySize 22
class SgfEntry
{
public:
    UINT16 Typ;
    UINT16 iFrei;
    UINT16 lFreiLo,lFreiHi;
    char   cFrei[10];
    UINT16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (18 mod 4 =2) !

    UINT32 GetOffset();
    friend SvStream& operator>>(SvStream& rIStream, SgfEntry& rEntr);
};

#define SgfVectorSize 10
class SgfVector
{
public:
    UINT16 Flag;
    INT16 x;
    INT16 y;
    UINT16 OfsLo,OfsHi; // DWord-Allignment ist notwendig (6 mod 4 =2) !

    friend SvStream& operator>>(SvStream& rIStream, SgfVector& rEntr);
};

extern long SgfVectXofs;
extern long SgfVectYofs;
extern long SgfVectXmul;
extern long SgfVectYmul;
extern long SgfVectXdiv;
extern long SgfVectYdiv;
extern BOOL SgfVectScal;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Windows BMP /////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#define BmpFileHeaderSize 14
class BmpFileHeader
{
public:
    UINT16 Typ;            // = "BM"
    UINT16 SizeLo,SizeHi;  // Filesize in Bytes
    UINT16 Reserve1;       // Reserviert
    UINT16 Reserve2;       // Reserviert
    UINT16 OfsLo,OfsHi;    // Offset?

    void   SetSize(UINT32 Size);
    void   SetOfs(UINT32 Size);
    UINT32 GetOfs();
    friend SvStream& operator<<(SvStream& rOStream, BmpFileHeader& rHead);
};

#define BmpInfoHeaderSize 40
class BmpInfoHeader
{
public:
    UINT32 Size;       // Gr”áe des BmpInfoHeaders
    INT32  Width;      // Breite in Pixel
    INT32  Hight;      // H”he in Pixel
    UINT16 Planes;     // Anzahl der Planes (immer 1)
    UINT16 PixBits;    // Anzahl der Bit je Pixel (1,4,8,oder 24)
    UINT32 Compress;   // Datenkompression
    UINT32 ImgSize;    // Gr”áe der Images in Bytes. Ohne Kompression ist auch 0 erlaubt.
    INT32  xDpmm;      // Dot per Meter (0 ist erlaubt)
    INT32  yDpmm;      // Dot per Meter (0 ist erlaubt)
    UINT32 ColUsed;    // Anzahl der verwendeten Farben (0=alle)
    UINT32 ColMust;    // Anzahl der wichtigen Farben (0=alle)

    friend SvStream& operator<<(SvStream& rOStream, BmpInfoHeader& rHead);
};

#define RGBQuadSize 4
class RGBQuad {
private:
    BYTE Red;
    BYTE Grn;
    BYTE Blu;
    BYTE Fil;
public:
    RGBQuad(BYTE R, BYTE G, BYTE B) { Red=R; Grn=G; Blu=B; Fil=0; }
};

}


#endif //_SGFBRAM_HXX
