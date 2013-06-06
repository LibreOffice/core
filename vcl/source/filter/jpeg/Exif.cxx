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

Exif::Exif() :
    maOrientation(TOP_LEFT),
    mbExifPresent(false)
{}

Exif::~Exif()
{}

Orientation Exif::getOrientation() {
    return maOrientation;
}

void Exif::setOrientation(Orientation aOrientation) {
    maOrientation = aOrientation;
}

Orientation Exif::convertToOrientation(sal_Int32 value)
{
    switch(value) {
        case 1: return TOP_LEFT;
        case 2: return TOP_RIGHT;
        case 3: return BOTTOM_RIGHT;
        case 4: return BOTTOM_LEFT;
        case 5: return LEFT_TOP;
        case 6: return RIGHT_TOP;
        case 7: return RIGHT_BOTTOM;
        case 8: return LEFT_BOTTOM;
    }
    return TOP_LEFT;
}

sal_Int32 Exif::getRotation()
{
    switch(maOrientation) {
        case TOP_LEFT:
            return 0;
        case BOTTOM_RIGHT:
            return 1800;
        case RIGHT_TOP:
            return 2700;
        case LEFT_BOTTOM:
            return 900;
        default:
            break;
    }
    return 0;
}

bool Exif::hasExif()
{
    return mbExifPresent;
}

bool Exif::read(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    bool result = processJpeg(rStream, false);
    rStream.Seek( nStreamPosition );

    return result;
}

bool Exif::write(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    bool result = processJpeg(rStream, true);
    rStream.Seek( nStreamPosition );

    return result;
}

bool Exif::processJpeg(SvStream& rStream, bool bSetValue)
{
    sal_uInt16  aMagic16;
    sal_uInt16  aLength;

    rStream.Seek(STREAM_SEEK_TO_END);
    sal_uInt32 aSize = rStream.Tell();
    rStream.Seek(STREAM_SEEK_TO_BEGIN);

    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStream >> aMagic16;

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
            rStream >> aMarker;
            if (aMarker != 0xFF)
            {
                break;
            }
            if (aCount >= 6)
            {
                return false;
            }
        }

        rStream >> aLength;

        if (aLength < 8)
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

bool Exif::processIFD(sal_uInt8* pExifData, sal_uInt16 aLength, sal_uInt16 aOffset, sal_uInt16 aNumberOfTags, bool bSetValue)
{
    ExifIFD* ifd = NULL;

    while (aOffset <= aLength - 12 && aNumberOfTags > 0)
    {
        ifd = (ExifIFD*) &pExifData[aOffset];

        if (ifd->tag == ORIENTATION)
        {
            if(bSetValue)
            {
                ifd->tag = ORIENTATION;
                ifd->type = 3;
                ifd->count = 1;
                ifd->offset = maOrientation;
            }
            else
            {
                maOrientation = convertToOrientation(ifd->offset);
            }
        }

        aNumberOfTags--;
        aOffset += 12;
    }
    return true;
}

bool Exif::processExif(SvStream& rStream, sal_uInt16 aSectionLength, bool bSetValue)
{
    sal_uInt32  aMagic32;
    sal_uInt16  aMagic16;

    rStream >> aMagic32;
    rStream >> aMagic16;

    // Compare EXIF magic bytes
    if( 0x45786966 != aMagic32 || 0x0000 != aMagic16)
    {
        return false;
    }

    sal_uInt16 aLength = aSectionLength - 6; // Length = Section - Header

    sal_uInt8* aExifData = new sal_uInt8[aLength];
    sal_uInt32 aExifDataBeginPosition = rStream.Tell();

    rStream.Read(aExifData, aLength);

    // Exif detected
    mbExifPresent = true;

    TiffHeader* aTiffHeader = (TiffHeader*) &aExifData[0];

    if( 0x4949 != aTiffHeader->byteOrder || 0x002A != aTiffHeader->tagAlign )
    {
        delete[] aExifData;
        return false;
    }

    sal_uInt16 aOffset = aTiffHeader->offset;

    sal_uInt16 aNumberOfTags = aExifData[aOffset];
    aNumberOfTags = aExifData[aOffset + 1];
    aNumberOfTags <<= 8;
    aNumberOfTags += aExifData[aOffset];

    processIFD(aExifData, aLength, aOffset+2, aNumberOfTags, bSetValue);

    if (bSetValue)
    {
        rStream.Seek(aExifDataBeginPosition);
        rStream.Write(aExifData, aLength);
    }

    delete[] aExifData;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
