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

#include <osl/endian.h>
#include <vcl/graph.hxx>
#include <vcl/fltcall.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

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
    ORIENTATION         = 0x0112,
    X_RESOLUTION        = 0x011a,
    Y_RESOLUTION        = 0x011b,
    EXIF_OFFSET         = 0x8769,
    INTEROP_OFFSET      = 0xa005
};

class Exif
{
private:
    Orientation maOrientation;
    bool mbExifPresent;

    bool processJpeg(SvStream& rStream, bool bSetValue);
    bool processExif(SvStream& rStream, sal_uInt16 aLength, bool bSetValue);
    bool processIFD(sal_uInt8* pExifData, sal_uInt16 aLength, sal_uInt16 aOffset, sal_uInt16 aNumberOfTags, bool bSetValue, bool bMoto);

    struct ExifIFD {
        sal_uInt16 tag;
        sal_uInt16 type;
        sal_uInt32 count;
        sal_uInt32 offset;
    };

    struct TiffHeader {
        sal_uInt16 byteOrder;
        sal_uInt16 tagAlign;
        sal_uInt32 offset;
    };

    Orientation convertToOrientation(sal_Int32 value);

public :
    Exif();
    virtual ~Exif();

    bool hasExif();

    Orientation getOrientation();
    sal_Int32 getRotation();

    void setOrientation(Orientation orientation);

    bool read(SvStream& rStream);
    bool write(SvStream& rStream);

};

#endif // INCLUDED_VCL_SOURCE_FILTER_JPEG_EXIF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
