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

#include "psputil.hxx"

#include "generic/printergfx.hxx"
#include "vcl/strhelper.hxx"

namespace psp {

const sal_uInt32 nLineLength = 80;
const sal_uInt32 nBufferSize = 16384;

/*
 *
 * Bitmap compression / Hex encoding / Ascii85 Encoding
 *
 */

PrinterBmp::~PrinterBmp()
{
}

/* virtual base class */

class ByteEncoder
{
private:

public:

    virtual void    EncodeByte (sal_uInt8 nByte) = 0;
    virtual         ~ByteEncoder () = 0;
};

ByteEncoder::~ByteEncoder()
{
}

/* HexEncoder */

class HexEncoder : public ByteEncoder
{
private:

    osl::File*      mpFile;
    sal_uInt32      mnColumn;
    sal_uInt32      mnOffset;
    sal_Char        mpFileBuffer[nBufferSize + 16];

public:

    explicit        HexEncoder (osl::File* pFile);
    virtual         ~HexEncoder ();
    void            WriteAscii (sal_uInt8 nByte);
    virtual void    EncodeByte (sal_uInt8 nByte) override;
    void            FlushLine ();
};

HexEncoder::HexEncoder (osl::File* pFile) :
        mpFile (pFile),
        mnColumn (0),
        mnOffset (0)
{}

HexEncoder::~HexEncoder ()
{
    FlushLine ();
    if (mnColumn > 0)
        WritePS (mpFile, "\n");
}

void
HexEncoder::WriteAscii (sal_uInt8 nByte)
{
    sal_uInt32 nOff = psp::getHexValueOf (nByte, mpFileBuffer + mnOffset);
    mnColumn += nOff;
    mnOffset += nOff;

    if (mnColumn >= nLineLength)
    {
        mnOffset += psp::appendStr ("\n", mpFileBuffer + mnOffset);
        mnColumn = 0;
    }
    if (mnOffset >= nBufferSize)
        FlushLine ();
}

void
HexEncoder::EncodeByte (sal_uInt8 nByte)
{
    WriteAscii (nByte);
}

void
HexEncoder::FlushLine ()
{
    if (mnOffset > 0)
    {
        WritePS (mpFile, mpFileBuffer, mnOffset);
        mnOffset = 0;
    }
}

/* Ascii85 encoder, is abi compatible with HexEncoder but writes a ~> to
   indicate end of data EOD */

class Ascii85Encoder : public ByteEncoder
{
private:

    osl::File*      mpFile;
    sal_uInt32      mnByte;
    sal_uInt8       mpByteBuffer[4];

    sal_uInt32      mnColumn;
    sal_uInt32      mnOffset;
    sal_Char        mpFileBuffer[nBufferSize + 16];

    inline void     PutByte (sal_uInt8 nByte);
    inline void     PutEOD ();
    void            ConvertToAscii85 ();
    void            FlushLine ();

public:

    explicit        Ascii85Encoder (osl::File* pFile);
    virtual         ~Ascii85Encoder ();
    virtual void    EncodeByte (sal_uInt8 nByte) override;
    void            WriteAscii (sal_uInt8 nByte);
};

Ascii85Encoder::Ascii85Encoder (osl::File* pFile) :
        mpFile (pFile),
        mnByte (0),
        mnColumn (0),
        mnOffset (0)
{}

inline void
Ascii85Encoder::PutByte (sal_uInt8 nByte)
{
    mpByteBuffer [mnByte++] = nByte;
}

inline void
Ascii85Encoder::PutEOD ()
{
    WritePS (mpFile, "~>\n");
}

void
Ascii85Encoder::ConvertToAscii85 ()
{
    if (mnByte < 4)
        std::memset (mpByteBuffer + mnByte, 0, (4 - mnByte) * sizeof(sal_uInt8));

    sal_uInt32 nByteValue =   mpByteBuffer[0] * 256 * 256 * 256
        + mpByteBuffer[1] * 256 * 256
        + mpByteBuffer[2] * 256
        + mpByteBuffer[3];

    if (nByteValue == 0 && mnByte == 4)
    {
        /* special case of 4 Bytes in row */
        mpFileBuffer [mnOffset] = 'z';

        mnOffset += 1;
        mnColumn += 1;
    }
    else
    {
        /* real ascii85 encoding */
        mpFileBuffer [mnOffset + 4] = (nByteValue % 85) + 33;
        nByteValue /= 85;
        mpFileBuffer [mnOffset + 3] = (nByteValue % 85) + 33;
        nByteValue /= 85;
        mpFileBuffer [mnOffset + 2] = (nByteValue % 85) + 33;
        nByteValue /= 85;
        mpFileBuffer [mnOffset + 1] = (nByteValue % 85) + 33;
        nByteValue /= 85;
        mpFileBuffer [mnOffset + 0] = (nByteValue % 85) + 33;

        mnColumn += (mnByte + 1);
        mnOffset += (mnByte + 1);

        /* insert a newline if necessary */
        if (mnColumn > nLineLength)
        {
            sal_uInt32 nEolOff = mnColumn - nLineLength;
            sal_uInt32 nBufOff = mnOffset - nEolOff;

            std::memmove (mpFileBuffer + nBufOff + 1, mpFileBuffer + nBufOff, nEolOff);
            mpFileBuffer[ nBufOff ] = '\n';

            mnOffset++;
            mnColumn = nEolOff;
        }
    }

    mnByte = 0;
}

void
Ascii85Encoder::WriteAscii (sal_uInt8 nByte)
{
    PutByte (nByte);
    if (mnByte == 4)
        ConvertToAscii85 ();

    if (mnColumn >= nLineLength)
    {
        mnOffset += psp::appendStr ("\n", mpFileBuffer + mnOffset);
        mnColumn = 0;
    }
    if (mnOffset >= nBufferSize)
        FlushLine ();
}

void
Ascii85Encoder::EncodeByte (sal_uInt8 nByte)
{
    WriteAscii (nByte);
}

void
Ascii85Encoder::FlushLine ()
{
    if (mnOffset > 0)
    {
        WritePS (mpFile, mpFileBuffer, mnOffset);
        mnOffset = 0;
    }
}

Ascii85Encoder::~Ascii85Encoder ()
{
    if (mnByte > 0)
        ConvertToAscii85 ();
    if (mnOffset > 0)
        FlushLine ();
    PutEOD ();
}

/* LZW encoder */

class LZWEncoder : public Ascii85Encoder
{
private:

    struct LZWCTreeNode
    {
        LZWCTreeNode*   mpBrother;      // next node with same parent
        LZWCTreeNode*   mpFirstChild;   // first son
        sal_uInt16      mnCode;         // code for the string
        sal_uInt16      mnValue;        // pixelvalue
    };

    LZWCTreeNode*   mpTable;    // LZW compression data
    LZWCTreeNode*   mpPrefix;   // the compression is as same as the TIFF compression
    sal_uInt16      mnDataSize;
    sal_uInt16      mnClearCode;
    sal_uInt16      mnEOICode;
    sal_uInt16      mnTableSize;
    sal_uInt16      mnCodeSize;
    sal_uInt32      mnOffset;
    sal_uInt32      mdwShift;

    void            WriteBits (sal_uInt16 nCode, sal_uInt16 nCodeLen);

public:

    explicit LZWEncoder (osl::File* pOutputFile);
    virtual ~LZWEncoder ();

    virtual void    EncodeByte (sal_uInt8 nByte) override;
};

LZWEncoder::LZWEncoder(osl::File* pOutputFile) :
        Ascii85Encoder (pOutputFile)
{
    mnDataSize  = 8;

    mnClearCode = 1 << mnDataSize;
    mnEOICode   = mnClearCode + 1;
    mnTableSize = mnEOICode   + 1;
    mnCodeSize  = mnDataSize  + 1;

    mnOffset    = 32;   // free bits in dwShift
    mdwShift    = 0;

    mpTable = new LZWCTreeNode[ 4096 ];

    for (sal_uInt32 i = 0; i < 4096; i++)
    {
        mpTable[i].mpBrother    = nullptr;
        mpTable[i].mpFirstChild = nullptr;
        mpTable[i].mnCode       = i;
        mpTable[i].mnValue      = (sal_uInt8)mpTable[i].mnCode;
    }

    mpPrefix = nullptr;

    WriteBits( mnClearCode, mnCodeSize );
}

LZWEncoder::~LZWEncoder()
{
    if (mpPrefix)
        WriteBits (mpPrefix->mnCode, mnCodeSize);

    WriteBits (mnEOICode, mnCodeSize);

    delete[] mpTable;
}

void
LZWEncoder::WriteBits (sal_uInt16 nCode, sal_uInt16 nCodeLen)
{
    mdwShift |= (nCode << (mnOffset - nCodeLen));
    mnOffset -= nCodeLen;
    while (mnOffset < 24)
    {
        WriteAscii ((sal_uInt8)(mdwShift >> 24));
        mdwShift <<= 8;
        mnOffset += 8;
    }
    if (nCode == 257 && mnOffset != 32)
        WriteAscii ((sal_uInt8)(mdwShift >> 24));
}

void
LZWEncoder::EncodeByte (sal_uInt8 nByte )
{
    LZWCTreeNode*   p;
    sal_uInt16      i;
    sal_uInt8       nV;

    if (!mpPrefix)
    {
        mpPrefix = mpTable + nByte;
    }
    else
    {
        nV = nByte;
        for (p = mpPrefix->mpFirstChild; p != nullptr; p = p->mpBrother)
        {
            if (p->mnValue == nV)
                break;
        }

        if (p != nullptr)
        {
            mpPrefix = p;
        }
        else
        {
            WriteBits (mpPrefix->mnCode, mnCodeSize);

            if (mnTableSize == 409)
            {
                WriteBits (mnClearCode, mnCodeSize);

                for (i = 0; i < mnClearCode; i++)
                    mpTable[i].mpFirstChild = nullptr;

                mnCodeSize = mnDataSize + 1;
                mnTableSize = mnEOICode + 1;
            }
            else
            {
                if(mnTableSize == (sal_uInt16)((1 << mnCodeSize) - 1))
                    mnCodeSize++;

                p = mpTable + (mnTableSize++);
                p->mpBrother = mpPrefix->mpFirstChild;
                mpPrefix->mpFirstChild = p;
                p->mnValue = nV;
                p->mpFirstChild = nullptr;
            }

            mpPrefix = mpTable + nV;
        }
    }
}

/*
 *
 * bitmap handling routines
 *
 */

void
PrinterGfx::DrawBitmap (const Rectangle& rDest, const Rectangle& rSrc,
                        const PrinterBmp& rBitmap)
{
    double fScaleX = (double)rDest.GetWidth();
    double fScaleY = (double)rDest.GetHeight();
    if(rSrc.GetWidth() > 0)
    {
        fScaleX = (double)rDest.GetWidth() / (double)rSrc.GetWidth();
    }
    if(rSrc.GetHeight() > 0)
    {
        fScaleY = (double)rDest.GetHeight() / (double)rSrc.GetHeight();
    }
    PSGSave ();
    PSTranslate (rDest.BottomLeft());
    PSScale (fScaleX, fScaleY);

    if (mnPSLevel >= 2)
    {
        if (rBitmap.GetDepth() == 1)
        {
            DrawPS2MonoImage (rBitmap, rSrc);
        }
        else
        if (rBitmap.GetDepth() ==  8 && mbColor)
        {
            // if the palette is larger than the image itself print it as a truecolor
            // image to save diskspace. This is important for printing transparent
            // bitmaps that are disassembled into small pieces
            sal_Int32 nImageSz   = rSrc.GetWidth() * rSrc.GetHeight();
            sal_Int32 nPaletteSz = rBitmap.GetPaletteEntryCount();
            if ((nImageSz < nPaletteSz) || (nImageSz < 24) )
                DrawPS2TrueColorImage (rBitmap, rSrc);
            else
                DrawPS2PaletteImage (rBitmap, rSrc);
        }
        else
        if (rBitmap.GetDepth() == 24 && mbColor)
        {
            DrawPS2TrueColorImage (rBitmap, rSrc);
        }
        else
        {
            DrawPS2GrayImage (rBitmap, rSrc);
        }
    }
    else
    {
        DrawPS1GrayImage (rBitmap, rSrc);
    }

    PSGRestore ();
}

/*
 *
 * Implementation: PS Level 1
 *
 */

void
PrinterGfx::DrawPS1GrayImage (const PrinterBmp& rBitmap, const Rectangle& rArea)
{
    sal_uInt32 nWidth  = rArea.GetWidth();
    sal_uInt32 nHeight = rArea.GetHeight();

    sal_Char  pGrayImage [512];
    sal_Int32 nChar = 0;

    // image header
    nChar += psp::getValueOf (nWidth,                           pGrayImage + nChar);
    nChar += psp::appendStr  (" ",                              pGrayImage + nChar);
    nChar += psp::getValueOf (nHeight,                          pGrayImage + nChar);
    nChar += psp::appendStr  (" 8 ",                            pGrayImage + nChar);
    nChar += psp::appendStr  ("[ 1 0 0 1 0 ",                   pGrayImage + nChar);
    nChar += psp::getValueOf (nHeight,                          pGrayImage + nChar);
    nChar += psp::appendStr  ("]",                              pGrayImage + nChar);
    nChar += psp::appendStr  (" {currentfile ",                 pGrayImage + nChar);
    nChar += psp::getValueOf (nWidth,                           pGrayImage + nChar);
    nChar += psp::appendStr  (" string readhexstring pop}\n",   pGrayImage + nChar);
    nChar += psp::appendStr  ("image\n",                        pGrayImage + nChar);

    WritePS (mpPageBody, pGrayImage, nChar);

    // image body
    std::unique_ptr<HexEncoder> xEncoder(new HexEncoder (mpPageBody));

    for (long nRow = rArea.Top(); nRow <= rArea.Bottom(); nRow++)
    {
        for (long nColumn = rArea.Left(); nColumn <= rArea.Right(); nColumn++)
        {
            unsigned char nByte = rBitmap.GetPixelGray (nRow, nColumn);
            xEncoder->EncodeByte (nByte);
        }
    }

    xEncoder.reset();

    WritePS (mpPageBody, "\n");
}

/*
 *
 * Implementation: PS Level 2
 *
 */

void
PrinterGfx::writePS2ImageHeader (const Rectangle& rArea, psp::ImageType nType)
{
    sal_Int32 nChar = 0;
    sal_Char  pImage [512];

    sal_Int32 nDictType = 0;
    switch (nType)
    {
        case psp::TrueColorImage:  nDictType = 0; break;
        case psp::PaletteImage:    nDictType = 1; break;
        case psp::GrayScaleImage:  nDictType = 2; break;
        case psp::MonochromeImage: nDictType = 3; break;
        default: break;
    }
    sal_Int32 nCompressType = mbCompressBmp ? 1 : 0;

    nChar += psp::getValueOf (rArea.GetWidth(),  pImage + nChar);
    nChar += psp::appendStr  (" ",               pImage + nChar);
    nChar += psp::getValueOf (rArea.GetHeight(), pImage + nChar);
    nChar += psp::appendStr  (" ",               pImage + nChar);
    nChar += psp::getValueOf (nDictType,         pImage + nChar);
    nChar += psp::appendStr  (" ",               pImage + nChar);
    nChar += psp::getValueOf (nCompressType,     pImage + nChar);
    nChar += psp::appendStr  (" psp_imagedict image\n", pImage + nChar);

    WritePS (mpPageBody, pImage, nChar);
}

void
PrinterGfx::writePS2Colorspace(const PrinterBmp& rBitmap, psp::ImageType nType)
{
    switch (nType)
    {
        case psp::GrayScaleImage:

            WritePS (mpPageBody, "/DeviceGray setcolorspace\n");
            break;

        case psp::TrueColorImage:

            WritePS (mpPageBody, "/DeviceRGB setcolorspace\n");
            break;

        case psp::MonochromeImage:
        case psp::PaletteImage:
        {

            sal_Int32 nChar = 0;
            sal_Char  pImage [4096];

            const sal_uInt32 nSize = rBitmap.GetPaletteEntryCount();

            nChar += psp::appendStr ("[/Indexed /DeviceRGB ", pImage + nChar);
            nChar += psp::getValueOf (nSize - 1, pImage + nChar);
            if (mbCompressBmp)
                nChar += psp::appendStr ("\npsp_lzwstring\n", pImage + nChar);
            else
                nChar += psp::appendStr ("\npsp_ascii85string\n", pImage + nChar);
            WritePS (mpPageBody, pImage, nChar);

            std::unique_ptr<ByteEncoder> xEncoder(mbCompressBmp ? new LZWEncoder(mpPageBody)
                                                    : new Ascii85Encoder(mpPageBody));
            for (sal_uInt32 i = 0; i < nSize; i++)
            {
                PrinterColor aColor = rBitmap.GetPaletteColor(i);

                xEncoder->EncodeByte (aColor.GetRed());
                xEncoder->EncodeByte (aColor.GetGreen());
                xEncoder->EncodeByte (aColor.GetBlue());
            }
            xEncoder.reset();

            WritePS (mpPageBody, "pop ] setcolorspace\n");
        }
        break;
        default: break;
    }
}

void
PrinterGfx::DrawPS2GrayImage (const PrinterBmp& rBitmap, const Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::GrayScaleImage);
    writePS2ImageHeader(rArea, psp::GrayScaleImage);

    std::unique_ptr<ByteEncoder> xEncoder(mbCompressBmp ? new LZWEncoder(mpPageBody)
                                            : new Ascii85Encoder(mpPageBody));

    for (long nRow = rArea.Top(); nRow <= rArea.Bottom(); nRow++)
    {
        for (long nColumn = rArea.Left(); nColumn <= rArea.Right(); nColumn++)
        {
            unsigned char nByte = rBitmap.GetPixelGray (nRow, nColumn);
            xEncoder->EncodeByte (nByte);
        }
    }
}

void
PrinterGfx::DrawPS2MonoImage (const PrinterBmp& rBitmap, const Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::MonochromeImage);
    writePS2ImageHeader(rArea, psp::MonochromeImage);

    std::unique_ptr<ByteEncoder> xEncoder(mbCompressBmp ? new LZWEncoder(mpPageBody)
                                            : new Ascii85Encoder(mpPageBody));

    for (long nRow = rArea.Top(); nRow <= rArea.Bottom(); nRow++)
    {
        long      nBitPos = 0;
        unsigned char nByte   = 0;

        for (long nColumn = rArea.Left(); nColumn <= rArea.Right(); nColumn++)
        {
            unsigned char nBit   = rBitmap.GetPixelIdx (nRow, nColumn);
            nByte |= nBit << (7 - nBitPos);

            if (++nBitPos == 8)
            {
                xEncoder->EncodeByte (nByte);
                nBitPos = 0;
                nByte   = 0;
            }
        }
        // keep the row byte aligned
        if (nBitPos != 0)
            xEncoder->EncodeByte (nByte);
    }
}

void
PrinterGfx::DrawPS2PaletteImage (const PrinterBmp& rBitmap, const Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::PaletteImage);
    writePS2ImageHeader(rArea, psp::PaletteImage);

    std::unique_ptr<ByteEncoder> xEncoder(mbCompressBmp ? new LZWEncoder(mpPageBody)
                                            : new Ascii85Encoder(mpPageBody));

    for (long nRow = rArea.Top(); nRow <= rArea.Bottom(); nRow++)
    {
        for (long nColumn = rArea.Left(); nColumn <= rArea.Right(); nColumn++)
        {
            unsigned char nByte = rBitmap.GetPixelIdx (nRow, nColumn);
            xEncoder->EncodeByte (nByte);
        }
    }
}

void
PrinterGfx::DrawPS2TrueColorImage (const PrinterBmp& rBitmap, const Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::TrueColorImage);
    writePS2ImageHeader(rArea, psp::TrueColorImage);

    std::unique_ptr<ByteEncoder> xEncoder(mbCompressBmp ? new LZWEncoder(mpPageBody)
                                            : new Ascii85Encoder(mpPageBody));

    for (long nRow = rArea.Top(); nRow <= rArea.Bottom(); nRow++)
    {
        for (long nColumn = rArea.Left(); nColumn <= rArea.Right(); nColumn++)
        {
            PrinterColor aColor = rBitmap.GetPixelRGB (nRow, nColumn);
            xEncoder->EncodeByte (aColor.GetRed());
            xEncoder->EncodeByte (aColor.GetGreen());
            xEncoder->EncodeByte (aColor.GetBlue());
        }
    }
}

} /* namespace psp */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
