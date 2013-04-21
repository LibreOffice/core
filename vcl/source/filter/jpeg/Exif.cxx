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
    maOrientation(TOP_LEFT)
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
            return 0;
    }
    return 0;
}

bool Exif::read(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    bool result = processJpegStream(rStream, false);
    rStream.Seek( nStreamPosition );

    return result;
}

bool Exif::write(SvStream& rStream)
{
    sal_Int32 nStreamPosition = rStream.Tell();
    bool result = processJpegStream(rStream, true);
    rStream.Seek( nStreamPosition );

    return result;
}

bool Exif::processJpegStream(SvStream& rStream, bool bSetValue)
{
    sal_uInt32  aMagic32;
    sal_uInt16  aMagic16;
    sal_uInt16  aLength;

    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
    rStream >> aMagic32;

    // Compare JPEG magic bytes
    if( 0xffd8ff00 != ( aMagic32 & 0xffffff00 ) )
    {
        return false;
    }

    rStream >> aLength;
    if (aLength < 8)
    {
        return false;
    }
    aLength -= 8;

    rStream >> aMagic32;
    rStream >> aMagic16;

    // Compare EXIF magic bytes
    if( 0x45786966 != aMagic32 || 0x0000 != aMagic16)
    {
        return false;
    }

    sal_uInt8* exifData = new sal_uInt8[aLength];
    sal_uInt32 aExifDataBeginPosition = rStream.Tell();
    rStream.Read(exifData, aLength);

    sal_uInt16 offset;
    offset = exifData[5];
    offset <<= 8;
    offset += exifData[4];

    sal_uInt16 numberOfTags;
    numberOfTags = exifData[offset+1];
    numberOfTags <<= 8;
    numberOfTags += exifData[offset];

    offset += 2;

    ExifIFD* ifd = NULL;

    while (offset <= aLength - 12 && numberOfTags > 0) {
        ifd = (ExifIFD*) &exifData[offset];

        if (ifd->tag == Tag::ORIENTATION)
        {
            if(bSetValue)
            {
                ifd->tag = Tag::ORIENTATION;
                ifd->type = 3;
                ifd->count = 1;
                ifd->offset = maOrientation;
            }
            else
            {
                maOrientation = convertToOrientation(ifd->offset);
            }
        }

        numberOfTags--;
        offset += 12;
    }

    if (bSetValue)
    {
        rStream.Seek(aExifDataBeginPosition);
        rStream.Write(exifData, aLength);
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
