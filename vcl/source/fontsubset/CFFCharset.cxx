/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <font/CFFCharset.hxx>

#include <tools/stream.hxx>

namespace vcl::font
{
namespace
{
// Read the offsets of a CFF INDEX, leaving the stream at its data.
// An INDEX is: count (2), offSize (1), offsets (count + 1), data.
bool readIndexOffsets(SvStream& rStream, std::vector<sal_uInt32>& rOffsets)
{
    sal_uInt16 nCount(0);
    rStream.ReadUInt16(nCount);
    if (!rStream.good())
        return false;

    // An empty INDEX is just the count
    if (nCount == 0)
        return true;

    sal_uInt8 nOffSize(0);
    rStream.ReadUChar(nOffSize);
    if (nOffSize < 1 || nOffSize > 4)
        return false;

    rOffsets.reserve(nCount + 1);
    for (sal_uInt32 i = 0; i <= nCount; i++)
    {
        sal_uInt32 nOffset = 0;
        for (sal_uInt8 j = 0; j < nOffSize; j++)
        {
            sal_uInt8 nByte(0);
            rStream.ReadUChar(nByte);
            nOffset = (nOffset << 8) | nByte;
        }
        rOffsets.push_back(nOffset);
    }

    return rStream.good();
}

// Read a DICT operand. Returns false if this is not an operand.
bool readDictOperand(SvStream& rStream, sal_uInt8 nByte0, sal_Int32& rOperand)
{
    switch (nByte0)
    {
        case 28: // shortint
        {
            sal_Int16 nOperand(0);
            rStream.ReadInt16(nOperand);
            rOperand = nOperand;
            break;
        }
        case 29: // longint
        {
            sal_Int32 nOperand(0);
            rStream.ReadInt32(nOperand);
            rOperand = nOperand;
            break;
        }
        case 30: // real
        {
            // We have no use for real numbers, so just skip over it.
            sal_uInt8 nByte(0);
            do
                rStream.ReadUChar(nByte);
            while (rStream.good() && (nByte & 0x0f) != 0x0f && (nByte & 0xf0) != 0xf0);
            break;
        }
        default:
        {
            // 22 to 27, 31 and 255 are reserved.
            if (nByte0 < 32 || nByte0 == 255)
                return false;

            // 32 to 246 encode an integer in the byte itself.
            if (nByte0 <= 246)
            {
                rOperand = sal_Int32(nByte0) - 139;
                break;
            }

            // 247 to 254 encode an integer in the byte and the one after it.
            sal_uInt8 nByte1(0);
            rStream.ReadUChar(nByte1);
            if (nByte0 <= 250)
                rOperand = (sal_Int32(nByte0) - 247) * 256 + nByte1 + 108;
            else
                rOperand = -(sal_Int32(nByte0) - 251) * 256 - nByte1 - 108;
            break;
        }
    }

    return rStream.good();
}

struct TopDictEntries
{
    sal_Int32 nCharsetOffset = 0;
    sal_Int32 nCharStringsOffset = 0;
    bool bIsCID = false;
};

// Read the Top DICT of the font, collecting the charset (op 15) and CharStrings
// (op 17) offsets and whether the font is CID-keyed (has a ROS, op 12 30).
bool readTopDict(SvStream& rStream, TopDictEntries& rEntries)
{
    // CFF header: major (1), minor (1), hdrSize (1), offSize (1)
    sal_uInt8 nHeaderSize(0);
    rStream.SeekRel(2);
    rStream.ReadUChar(nHeaderSize);
    if (!rStream.good() || nHeaderSize > rStream.TellEnd())
        return false;
    rStream.Seek(nHeaderSize);

    // The Name INDEX comes before the Top DICT INDEX, skip over it. Offsets
    // are one-based and relative to the byte before the data.
    std::vector<sal_uInt32> aOffsets;
    if (!readIndexOffsets(rStream, aOffsets))
        return false;
    if (!aOffsets.empty())
    {
        sal_uInt64 nNameEnd = rStream.Tell() + aOffsets.back() - 1;
        if (aOffsets.back() < 1 || nNameEnd > rStream.TellEnd())
            return false;
        rStream.Seek(nNameEnd);
    }

    // The Top DICT of the font is the first element of the Top DICT INDEX.
    aOffsets.clear();
    if (!readIndexOffsets(rStream, aOffsets) || aOffsets.size() < 2)
        return false;

    sal_uInt64 nData = rStream.Tell() - 1;
    if (aOffsets[0] < 1 || aOffsets[1] < aOffsets[0] || nData + aOffsets[1] > rStream.TellEnd())
        return false;

    sal_uInt64 nEnd = nData + aOffsets[1];
    rStream.Seek(nData + aOffsets[0]);
    if (!rStream.good())
        return false;

    sal_Int32 nOperand = 0;
    while (rStream.Tell() < nEnd && rStream.good())
    {
        sal_uInt8 nByte0(0);
        rStream.ReadUChar(nByte0);

        // Operands are 22 and above, anything else is an operator
        if (nByte0 > 21)
        {
            if (!readDictOperand(rStream, nByte0, nOperand))
                return false;
            continue;
        }

        sal_uInt16 nOperator = nByte0;
        if (nByte0 == 12) // two-byte operator
        {
            sal_uInt8 nByte1(0);
            rStream.ReadUChar(nByte1);
            nOperator = (12 << 8) | nByte1;
        }

        switch (nOperator)
        {
            case 15: // charset
                rEntries.nCharsetOffset = nOperand;
                break;
            case 17: // CharStrings
                rEntries.nCharStringsOffset = nOperand;
                break;
            case (12 << 8) | 30: // ROS
                rEntries.bIsCID = true;
                break;
            default:
                break;
        }
    }

    return rStream.good();
}
}

bool ReadCFFGlyphCIDs(const sal_uInt8* pData, sal_uInt32 nLen, std::vector<sal_uInt16>& rCIDs)
{
    SvMemoryStream aStream(const_cast<sal_uInt8*>(pData), nLen, StreamMode::READ);
    // Font data are big endian.
    aStream.SetEndian(SvStreamEndian::BIG);

    TopDictEntries aEntries;
    if (!readTopDict(aStream, aEntries))
        return false;

    // Name-keyed fonts have no CIDs, their glyph IDs are used as such.
    if (!aEntries.bIsCID)
        return true;

    // The number of glyphs is the count of the CharStrings INDEX, a font
    // without one is broken.
    if (!aEntries.nCharStringsOffset)
        return false;
    aStream.Seek(aEntries.nCharStringsOffset);
    sal_uInt16 nGlyphs(0);
    aStream.ReadUInt16(nGlyphs);
    if (!aStream.good() || nGlyphs == 0)
        return false;

    // Offsets 0 to 2 are the predefined charsets, which CID-keyed fonts don’t
    // use, they always have a charset of their own.
    if (aEntries.nCharsetOffset <= 2)
        return false;
    aStream.Seek(aEntries.nCharsetOffset);
    sal_uInt8 nFormat(0);
    aStream.ReadUChar(nFormat);

    rCIDs.assign(nGlyphs, 0); // glyph 0 is always CID 0
    sal_uInt16 nGlyph = 1;
    if (nFormat == 0)
    {
        // An array of CIDs, one per glyph
        while (nGlyph < nGlyphs)
            aStream.ReadUInt16(rCIDs[nGlyph++]);
    }
    else if (nFormat == 1 || nFormat == 2)
    {
        // Ranges of consecutive CIDs, the number of the ones following the
        // first is one byte wide in format 1 and two in format 2.
        while (nGlyph < nGlyphs && aStream.good())
        {
            sal_uInt16 nFirst(0);
            aStream.ReadUInt16(nFirst);

            sal_uInt16 nLeft(0);
            if (nFormat == 1)
            {
                sal_uInt8 nByte(0);
                aStream.ReadUChar(nByte);
                nLeft = nByte;
            }
            else
            {
                aStream.ReadUInt16(nLeft);
            }

            if (nFirst + sal_uInt32(nLeft) > 0xFFFF)
                return false;

            for (sal_uInt32 i = 0; i <= nLeft && nGlyph < nGlyphs; i++)
                rCIDs[nGlyph++] = nFirst + i;
        }
    }
    else
        return false;

    if (!aStream.good())
    {
        rCIDs.clear();
        return false;
    }

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
