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

#include "Exif.hxx"
#include <memory>
#include <osl/endian.h>
#include <tools/stream.hxx>

Exif::Exif() :
    maOrientation(exif::TOP_LEFT),
    mbExifPresent(false)
{}

Exif::~Exif()
{}

void Exif::setOrientation(exif::Orientation aOrientation) {
    maOrientation = aOrientation;
}

exif::Orientation Exif::convertToOrientation(sal_Int32 value)
{
    switch(value) {
        case 1: return exif::TOP_LEFT;
        case 2: return exif::TOP_RIGHT;
        case 3: return exif::BOTTOM_RIGHT;
        case 4: return exif::BOTTOM_LEFT;
        case 5: return exif::LEFT_TOP;
        case 6: return exif::RIGHT_TOP;
        case 7: return exif::RIGHT_BOTTOM;
        case 8: return exif::LEFT_BOTTOM;
    }
    return exif::TOP_LEFT;
}

Degree10 Exif::getRotation() const
{
    switch(maOrientation) {
        case exif::TOP_LEFT:
            return 0_deg10;
        case exif::BOTTOM_RIGHT:
            return 1800_deg10;
        case exif::RIGHT_TOP:
            return 2700_deg10;
        case exif::LEFT_BOTTOM:
            return 900_deg10;
        default:
            break;
    }
    return 0_deg10;
}

bool Exif::read(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    bool result = processJpeg(rStream, false);
    rStream.Seek( nStreamPosition );

    return result;
}

void Exif::write(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    processJpeg(rStream, true);
    rStream.Seek( nStreamPosition );
}

bool Exif::processJpeg(SvStream& rStream, bool bSetValue)
{
    sal_uInt16  aMagic16;
    sal_uInt16  aLength;

    sal_uInt32 aSize = rStream.TellEnd();
    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    rStream.SetEndian( SvStreamEndian::BIG );
    rStream.ReadUInt16( aMagic16 );

    // Compare JPEG magic bytes
    if( 0xFFD8 != aMagic16 )
    {
        return false;
    }

    sal_uInt32 aPreviousPosition = STREAM_SEEK_TO_BEGIN;

    while(true)
    {
        sal_uInt8 aMarker = 0xD9;
        sal_Int32 aCount;

        for (aCount = 0; aCount < 7; aCount++)
        {
            rStream.ReadUChar( aMarker );
            if (aMarker != 0xFF)
            {
                break;
            }
            if (aCount >= 6)
            {
                return false;
            }
        }

        rStream.ReadUInt16( aLength );

        if (aLength < 8 || aLength > rStream.remainingSize())
        {
            return false;
        }

        if (aMarker == 0xE1)
        {
            return processExif(rStream, aLength, bSetValue);
        }
        else if (aMarker == 0xD9)
        {
            return false;
        }
        else
        {
            sal_uInt32 aCurrentPosition = rStream.SeekRel(aLength-1);
            if (aCurrentPosition == aPreviousPosition || aCurrentPosition > aSize)
            {
                return false;
            }
            aPreviousPosition = aCurrentPosition;
        }
    }
    return false;
}

namespace {

sal_uInt16 read16(sal_uInt8 const (& data)[2], bool littleEndian) {
    if (littleEndian) {
        return data[0] | (sal_uInt16(data[1]) << 8);
    } else {
        return data[1] | (sal_uInt16(data[0]) << 8);
    }
}

void write16(sal_uInt16 value, sal_uInt8 (& data)[2], bool littleEndian) {
    if (littleEndian) {
        data[0] = value & 0xFF;
        data[1] = value >> 8;
    } else {
        data[1] = value & 0xFF;
        data[0] = value >> 8;
    }
}

void write32(sal_uInt32 value, sal_uInt8 (& data)[4], bool littleEndian) {
    if (littleEndian) {
        data[0] = value & 0xFF;
        data[1] = (value >> 8) & 0xFF;
        data[2] = (value >> 16) & 0xFF;
        data[3] = value >> 24;
    } else {
        data[3] = value & 0xFF;
        data[2] = (value >> 8) & 0xFF;
        data[1] = (value >> 16) & 0xFF;
        data[0] = value >> 24;
    }
}

}

void Exif::processIFD(sal_uInt8* pExifData, sal_uInt16 aLength, sal_uInt16 aOffset, sal_uInt16 aNumberOfTags, bool bSetValue, bool littleEndian)
{
    ExifIFD* ifd = nullptr;

    while (aOffset <= aLength - 12 && aNumberOfTags > 0)
    {
        ifd = reinterpret_cast<ExifIFD*>(&pExifData[aOffset]);
        sal_uInt16 tag = read16(ifd->tag, littleEndian);

        if (tag == ORIENTATION)
        {
            if(bSetValue)
            {
                write16(3, ifd->type, littleEndian);
                write32(1, ifd->count, littleEndian);
                write16(
                    maOrientation, reinterpret_cast<sal_uInt8 (&)[2]>(ifd->offset), littleEndian);
            }
            else
            {
                sal_uInt16 nIfdOffset = read16(
                    reinterpret_cast<sal_uInt8 (&)[2]>(ifd->offset), littleEndian);
                maOrientation = convertToOrientation(nIfdOffset);
            }
        }

        aNumberOfTags--;
        aOffset += 12;
    }
}

bool Exif::processExif(SvStream& rStream, sal_uInt16 aSectionLength, bool bSetValue)
{
    sal_uInt32  aMagic32;
    sal_uInt16  aMagic16;

    rStream.ReadUInt32( aMagic32 );
    rStream.ReadUInt16( aMagic16 );

    // Compare EXIF magic bytes
    if( 0x45786966 != aMagic32 || 0x0000 != aMagic16)
    {
        return false;
    }

    sal_uInt16 aLength = aSectionLength - 6; // Length = Section - Header

    std::unique_ptr<sal_uInt8[]> aExifData(new sal_uInt8[aLength]);
    sal_uInt32 aExifDataBeginPosition = rStream.Tell();

    rStream.ReadBytes(aExifData.get(), aLength);

    // Exif detected
    mbExifPresent = true;

    TiffHeader* aTiffHeader = reinterpret_cast<TiffHeader*>(&aExifData[0]);

    bool bIntel = aTiffHeader->byteOrder == 0x4949;      //little-endian
    bool bMotorola = aTiffHeader->byteOrder == 0x4D4D;   //big-endian

    if (!bIntel && !bMotorola)
    {
        return false;
    }

    bool bSwap = false;

#ifdef OSL_BIGENDIAN
    if (bIntel)
        bSwap = true;
#else
    if (bMotorola)
        bSwap = true;
#endif

    if (bSwap)
    {
        aTiffHeader->tagAlign = OSL_SWAPWORD(aTiffHeader->tagAlign);
        aTiffHeader->offset = OSL_SWAPDWORD(aTiffHeader->offset);
    }

    if (aTiffHeader->tagAlign != 0x002A) // TIFF tag
    {
        return false;
    }

    sal_uInt16 aOffset = aTiffHeader->offset;

    sal_uInt16 aNumberOfTags = aExifData[aOffset];
    if (bSwap)
    {
        aNumberOfTags = ((aExifData[aOffset] << 8) | aExifData[aOffset+1]);
    }

    processIFD(aExifData.get(), aLength, aOffset+2, aNumberOfTags, bSetValue, bIntel);

    if (bSetValue)
    {
        rStream.Seek(aExifDataBeginPosition);
        rStream.WriteBytes(aExifData.get(), aLength);
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
