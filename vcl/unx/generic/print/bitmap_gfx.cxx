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

#include <array>
#include <memory>
#include "psputil.hxx"

#include <unx/printergfx.hxx>
#include <vcl/strhelper.hxx>

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

    osl::File* const mpFile;
    sal_uInt32      mnColumn;
    sal_uInt32      mnOffset;
    OStringBuffer   mpFileBuffer;

public:

    explicit        HexEncoder (osl::File* pFile);
    virtual         ~HexEncoder () override;
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
    sal_uInt32 nOff = psp::getHexValueOf (nByte, mpFileBuffer);
    mnColumn += nOff;
    mnOffset += nOff;

    if (mnColumn >= nLineLength)
    {
        mnOffset += psp::appendStr ("\n", mpFileBuffer);
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
        WritePS (mpFile, mpFileBuffer.makeStringAndClear());
        mnOffset = 0;
    }
}

/* Ascii85 encoder, is abi compatible with HexEncoder but writes a ~> to
   indicate end of data EOD */

class Ascii85Encoder : public ByteEncoder
{
private:

    osl::File* const mpFile;
    sal_uInt32      mnByte;
    sal_uInt8       mpByteBuffer[4];

    sal_uInt32      mnColumn;
    sal_uInt32      mnOffset;
    OStringBuffer   mpFileBuffer;

    inline void     PutByte (sal_uInt8 nByte);
    inline void     PutEOD ();
    void            ConvertToAscii85 ();
    void            FlushLine ();

public:

    explicit        Ascii85Encoder (osl::File* pFile);
    virtual         ~Ascii85Encoder () override;
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
    // Add (4 - mnByte) zero padding bytes:
    if (mnByte < 4)
        std::memset (mpByteBuffer + mnByte, 0, (4 - mnByte) * sizeof(sal_uInt8));

    sal_uInt32 nByteValue =   mpByteBuffer[0] * 256 * 256 * 256
        + mpByteBuffer[1] * 256 * 256
        + mpByteBuffer[2] * 256
        + mpByteBuffer[3];

    if (nByteValue == 0 && mnByte == 4)
    {
        /* special case of 4 Bytes in row */
        mpFileBuffer.append('z');

        mnOffset += 1;
        mnColumn += 1;
    }
    else
    {
        /* real ascii85 encoding */

        // Of the up to 5 characters to be generated, do not generate the last (4 - mnByte) ones
        // that correspond to the (4 - mnByte) zero padding bytes added to the input:

        auto const pos = mpFileBuffer.getLength();
        if (mnByte == 4) {
            mpFileBuffer.insert(pos, char((nByteValue % 85) + 33));
        }
        nByteValue /= 85;
        if (mnByte >= 3) {
            mpFileBuffer.insert(pos, char((nByteValue % 85) + 33));
        }
        nByteValue /= 85;
        if (mnByte >= 2) {
            mpFileBuffer.insert(pos, char((nByteValue % 85) + 33));
        }
        nByteValue /= 85;
        mpFileBuffer.insert(pos, char((nByteValue % 85) + 33));
        nByteValue /= 85;
        mpFileBuffer.insert(pos, char((nByteValue % 85) + 33));

        mnColumn += (mnByte + 1);
        mnOffset += (mnByte + 1);

        /* insert a newline if necessary */
        if (mnColumn > nLineLength)
        {
            sal_uInt32 nEolOff = mnColumn - nLineLength;
            auto const posNl = pos + (mnByte + 1) - nEolOff;

            mpFileBuffer.insert(posNl, '\n');

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
        mnOffset += psp::appendStr ("\n", mpFileBuffer);
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
        WritePS (mpFile, mpFileBuffer.makeStringAndClear());
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

    std::array<LZWCTreeNode, 4096>
                    mpTable;    // LZW compression data
    LZWCTreeNode*   mpPrefix;   // the compression is as same as the TIFF compression
    static constexpr sal_uInt16 gnDataSize = 8;
    static constexpr sal_uInt16 gnClearCode = 1 << gnDataSize;
    static constexpr sal_uInt16 gnEOICode = gnClearCode + 1;
    sal_uInt16      mnTableSize;
    sal_uInt16      mnCodeSize;
    sal_uInt32      mnOffset;
    sal_uInt32      mdwShift;

    void            WriteBits (sal_uInt16 nCode, sal_uInt16 nCodeLen);

public:

    explicit LZWEncoder (osl::File* pOutputFile);
    virtual ~LZWEncoder () override;

    virtual void    EncodeByte (sal_uInt8 nByte) override;
};

LZWEncoder::LZWEncoder(osl::File* pOutputFile) :
        Ascii85Encoder (pOutputFile),
        mpPrefix(nullptr),
        mnTableSize(gnEOICode + 1),
        mnCodeSize(gnDataSize + 1),
        mnOffset(32),       // free bits in dwShift
        mdwShift(0)
{
    for (sal_uInt32 i = 0; i < 4096; i++)
    {
        mpTable[i].mpBrother    = nullptr;
        mpTable[i].mpFirstChild = nullptr;
        mpTable[i].mnCode       = i;
        mpTable[i].mnValue      = static_cast<sal_uInt8>(mpTable[i].mnCode);
    }

    WriteBits( gnClearCode, mnCodeSize );
}

LZWEncoder::~LZWEncoder()
{
    if (mpPrefix)
        WriteBits (mpPrefix->mnCode, mnCodeSize);

    WriteBits (gnEOICode, mnCodeSize);
}

void
LZWEncoder::WriteBits (sal_uInt16 nCode, sal_uInt16 nCodeLen)
{
    mdwShift |= (nCode << (mnOffset - nCodeLen));
    mnOffset -= nCodeLen;
    while (mnOffset < 24)
    {
        WriteAscii (static_cast<sal_uInt8>(mdwShift >> 24));
        mdwShift <<= 8;
        mnOffset += 8;
    }
    if (nCode == 257 && mnOffset != 32)
        WriteAscii (static_cast<sal_uInt8>(mdwShift >> 24));
}

void
LZWEncoder::EncodeByte (sal_uInt8 nByte )
{
    LZWCTreeNode*   p;
    sal_uInt16      i;
    sal_uInt8       nV;

    if (!mpPrefix)
    {
        mpPrefix = mpTable.data() + nByte;
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
                WriteBits (gnClearCode, mnCodeSize);

                for (i = 0; i < gnClearCode; i++)
                    mpTable[i].mpFirstChild = nullptr;

                mnCodeSize = gnDataSize + 1;
                mnTableSize = gnEOICode + 1;
            }
            else
            {
                if(mnTableSize == static_cast<sal_uInt16>((1 << mnCodeSize) - 1))
                    mnCodeSize++;

                p = mpTable.data() + (mnTableSize++);
                p->mpBrother = mpPrefix->mpFirstChild;
                mpPrefix->mpFirstChild = p;
                p->mnValue = nV;
                p->mpFirstChild = nullptr;
            }

            mpPrefix = mpTable.data() + nV;
        }
    }
}

/*
 *
 * bitmap handling routines
 *
 */

void
PrinterGfx::DrawBitmap (const tools::Rectangle& rDest, const tools::Rectangle& rSrc,
                        const PrinterBmp& rBitmap)
{
    double fScaleX = static_cast<double>(rDest.GetWidth());
    double fScaleY = static_cast<double>(rDest.GetHeight());
    if(rSrc.GetWidth() > 0)
    {
        fScaleX = static_cast<double>(rDest.GetWidth()) / static_cast<double>(rSrc.GetWidth());
    }
    if(rSrc.GetHeight() > 0)
    {
        fScaleY = static_cast<double>(rDest.GetHeight()) / static_cast<double>(rSrc.GetHeight());
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
PrinterGfx::DrawPS1GrayImage (const PrinterBmp& rBitmap, const tools::Rectangle& rArea)
{
    sal_uInt32 nWidth  = rArea.GetWidth();
    sal_uInt32 nHeight = rArea.GetHeight();

    OStringBuffer pGrayImage;

    // image header
    psp::getValueOf (nWidth,                           pGrayImage);
    psp::appendStr  (" ",                              pGrayImage);
    psp::getValueOf (nHeight,                          pGrayImage);
    psp::appendStr  (" 8 ",                            pGrayImage);
    psp::appendStr  ("[ 1 0 0 1 0 ",                   pGrayImage);
    psp::getValueOf (nHeight,                          pGrayImage);
    psp::appendStr  ("]",                              pGrayImage);
    psp::appendStr  (" {currentfile ",                 pGrayImage);
    psp::getValueOf (nWidth,                           pGrayImage);
    psp::appendStr  (" string readhexstring pop}\n",   pGrayImage);
    psp::appendStr  ("image\n",                        pGrayImage);

    WritePS (mpPageBody, pGrayImage.makeStringAndClear());

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
PrinterGfx::writePS2ImageHeader (const tools::Rectangle& rArea, psp::ImageType nType)
{
    OStringBuffer pImage;

    sal_Int32 nDictType = 0;
    switch (nType)
    {
        case psp::ImageType::TrueColorImage:  nDictType = 0; break;
        case psp::ImageType::PaletteImage:    nDictType = 1; break;
        case psp::ImageType::GrayScaleImage:  nDictType = 2; break;
        case psp::ImageType::MonochromeImage: nDictType = 3; break;
        default: break;
    }

    psp::getValueOf (rArea.GetWidth(),  pImage);
    psp::appendStr  (" ",               pImage);
    psp::getValueOf (rArea.GetHeight(), pImage);
    psp::appendStr  (" ",               pImage);
    psp::getValueOf (nDictType,         pImage);
    psp::appendStr  (" ",               pImage);
    psp::getValueOf (sal_Int32(1),      pImage); // nCompressType
    psp::appendStr  (" psp_imagedict image\n", pImage);

    WritePS (mpPageBody, pImage.makeStringAndClear());
}

void
PrinterGfx::writePS2Colorspace(const PrinterBmp& rBitmap, psp::ImageType nType)
{
    switch (nType)
    {
        case psp::ImageType::GrayScaleImage:

            WritePS (mpPageBody, "/DeviceGray setcolorspace\n");
            break;

        case psp::ImageType::TrueColorImage:

            WritePS (mpPageBody, "/DeviceRGB setcolorspace\n");
            break;

        case psp::ImageType::MonochromeImage:
        case psp::ImageType::PaletteImage:
        {

            OStringBuffer pImage;

            const sal_uInt32 nSize = rBitmap.GetPaletteEntryCount();

            psp::appendStr ("[/Indexed /DeviceRGB ", pImage);
            psp::getValueOf (nSize - 1, pImage);
            psp::appendStr ("\npsp_lzwstring\n", pImage);
            WritePS (mpPageBody, pImage.makeStringAndClear());

            std::unique_ptr<ByteEncoder> xEncoder(new LZWEncoder(mpPageBody));
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
PrinterGfx::DrawPS2GrayImage (const PrinterBmp& rBitmap, const tools::Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::ImageType::GrayScaleImage);
    writePS2ImageHeader(rArea, psp::ImageType::GrayScaleImage);

    std::unique_ptr<ByteEncoder> xEncoder(new LZWEncoder(mpPageBody));

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
PrinterGfx::DrawPS2MonoImage (const PrinterBmp& rBitmap, const tools::Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::ImageType::MonochromeImage);
    writePS2ImageHeader(rArea, psp::ImageType::MonochromeImage);

    std::unique_ptr<ByteEncoder> xEncoder(new LZWEncoder(mpPageBody));

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
PrinterGfx::DrawPS2PaletteImage (const PrinterBmp& rBitmap, const tools::Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::ImageType::PaletteImage);
    writePS2ImageHeader(rArea, psp::ImageType::PaletteImage);

    std::unique_ptr<ByteEncoder> xEncoder(new LZWEncoder(mpPageBody));

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
PrinterGfx::DrawPS2TrueColorImage (const PrinterBmp& rBitmap, const tools::Rectangle& rArea)
{
    writePS2Colorspace(rBitmap, psp::ImageType::TrueColorImage);
    writePS2ImageHeader(rArea, psp::ImageType::TrueColorImage);

    std::unique_ptr<ByteEncoder> xEncoder(new LZWEncoder(mpPageBody));

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
