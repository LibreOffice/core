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

#ifndef INCLUDED_VCL_SOURCE_FILTER_SGFBRAM_HXX
#define INCLUDED_VCL_SOURCE_FILTER_SGFBRAM_HXX

#define SgfBitImag0   1 /* Bitmap                  */
#define SgfBitImag1   4 /* Bitmap                  */
#define SgfBitImag2   5 /* Bitmap                  */
#define SgfBitImgMo   6 /* Monochrome bitmap       */
#define SgfSimpVect   2 /* Simple vectorformat     */
#define SgfPostScrp   3 /* Postscript file         */
#define SgfStarDraw   7 /* StarDraw SGV-file       */
#define SgfDontKnow 255 /* Unknown or no SGF/SGV   */

// constants for SgfHeader.SwGrCol
#define SgfBlckWhit 1 /* black/white image                  SimpVector,  */
#define SgfGrayscal 2 /* image with greyscale               StarDraw and */
#define Sgf16Colors 3 /* colour image  (16 colours)         Bit Image    */
#define SgfVectFarb 4 /* use colours in lines                            */
#define SgfVectGray 5 /* use greyscale for lines only for                */
#define SgfVectWdth 6 /* use line widths for lines          SimpVector   */

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
    sal_uInt16 OfsLo,OfsHi; // DWord allignment is necessary (38 mod 4 =2) !

    sal_uInt32 GetOffset();
    friend SvStream& ReadSgfHeader(SvStream& rIStream, SgfHeader& rHead);
    bool   ChkMagic();
    SgfHeader();
};

#define SgfEntrySize 22
class SgfEntry
{
public:
    sal_uInt16 Typ;
    sal_uInt16 iFrei;
    sal_uInt16 lFreiLo,lFreiHi;
    char   cFrei[10];
    sal_uInt16 OfsLo,OfsHi; // DWord allignment is necessary (18 mod 4 =2) !

    sal_uInt32 GetOffset();
    friend SvStream& ReadSgfEntry(SvStream& rIStream, SgfEntry& rEntr);
    SgfEntry();
};

#define SgfVectorSize 10
class SgfVector
{
public:
    sal_uInt16 Flag;
    sal_Int16 x;
    sal_Int16 y;
    sal_uInt16 OfsLo,OfsHi; // DWord allignment is necessary (6 mod 4 =2) !

    friend SvStream& ReadSgfVector(SvStream& rIStream, SgfVector& rEntr);
};

extern long SgfVectXofs;
extern long SgfVectYofs;
extern long SgfVectXmul;
extern long SgfVectYmul;
extern long SgfVectXdiv;
extern long SgfVectYdiv;
extern bool SgfVectScal;

#define BmpFileHeaderSize 14
class BmpFileHeader
{
public:
    sal_uInt16 Typ;            // = "BM"
    sal_uInt16 SizeLo,SizeHi;  // filesize in bytes
    sal_uInt16 Reserve1;       // reserved
    sal_uInt16 Reserve2;       // reserved
    sal_uInt16 OfsLo,OfsHi;    // Offset?

    void   SetSize(sal_uInt32 Size);
    void   SetOfs(sal_uInt32 Size);
    sal_uInt32 GetOfs();
    friend SvStream& WriteBmpFileHeader(SvStream& rOStream, BmpFileHeader& rHead);
};

#define BmpInfoHeaderSize 40
class BmpInfoHeader
{
public:
    sal_uInt32 Size;       // size of BmpInfoHeaders
    sal_Int32  Width;      // width in Pixel
    sal_Int32  Hight;      // height in Pixel
    sal_uInt16 Planes;     // number of planes (always 1)
    sal_uInt16 PixBits;    // number of bits per pixel (1,4,8 or 24)
    sal_uInt32 Compress;   // datakompression
    sal_uInt32 ImgSize;    // size of  image in bytes. Without compression also 0 is allowed.
    sal_Int32  xDpmm;      // Dot per Meter (0 is allowed)
    sal_Int32  yDpmm;      // Dot per Meter (0 is allowed)
    sal_uInt32 ColUsed;    // number of colours used (0=all
    sal_uInt32 ColMust;    // number of important colours (0=all)

    friend SvStream& WriteBmpInfoHeader(SvStream& rOStream, BmpInfoHeader& rHead);
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

#endif // INCLUDED_VCL_SOURCE_FILTER_SGFBRAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
