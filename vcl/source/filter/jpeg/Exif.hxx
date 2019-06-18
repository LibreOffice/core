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

#ifndef INCLUDED_VCL_SOURCE_FILTER_JPEG_EXIF_HXX
#define INCLUDED_VCL_SOURCE_FILTER_JPEG_EXIF_HXX

#include <tools/stream.hxx>

enum Orientation {
    TOP_LEFT        = 1,
    TOP_RIGHT       = 2,
    BOTTOM_RIGHT    = 3,
    BOTTOM_LEFT     = 4,
    LEFT_TOP        = 5,
    RIGHT_TOP       = 6,
    RIGHT_BOTTOM    = 7,
    LEFT_BOTTOM     = 8
};

enum Tag {
    ORIENTATION         = 0x0112
};

class Exif final
{
private:
    Orientation maOrientation;
    bool mbExifPresent;

    bool processJpeg(SvStream& rStream, bool bSetValue);
    bool processExif(SvStream& rStream, sal_uInt16 aLength, bool bSetValue);
    void processIFD(sal_uInt8* pExifData, sal_uInt16 aLength, sal_uInt16 aOffset, sal_uInt16 aNumberOfTags, bool bSetValue, bool bLittleEndian);

    struct ExifIFD {
        sal_uInt8 tag[2];
        sal_uInt8 type[2];
        sal_uInt8 count[4];
        sal_uInt8 offset[4];
    };

    struct TiffHeader {
        sal_uInt16 const byteOrder;
        sal_uInt16 tagAlign;
        sal_uInt32 offset;
    };

    static Orientation convertToOrientation(sal_Int32 value);

public:
    Exif();
    ~Exif();

    bool hasExif() const { return mbExifPresent;}

    Orientation getOrientation() const { return maOrientation;}
    sal_Int32 getRotation() const;

    void setOrientation(Orientation orientation);

    bool read(SvStream& rStream);
    void write(SvStream& rStream);

};

#endif // INCLUDED_VCL_SOURCE_FILTER_JPEG_EXIF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
