/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_BITMAP_COMPRESSOR_HXX
#define SD_SLIDESORTER_BITMAP_COMPRESSOR_HXX

#include <sal/types.h>
#include <tools/gen.hxx>
#include <boost/shared_ptr.hpp>


class Bitmap;

namespace sd { namespace slidesorter { namespace cache {

class BitmapReplacement;


/** This interface class provides the minimal method set for classes that
    implement the compression and decompression of preview bitmaps.
*/
class BitmapCompressor
{
public:
    /** Compress the given bitmap into a replacement format that is specific
        to the compressor class.
    */
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const = 0;

    /** Decompress the given replacement data into a preview bitmap.
        Depending on the compression technique the returned bitmap may
        differ from the original bitmap given to the Compress() method.  It
        may even of the wrong size or empty or the NULL pointer.  It is the
        task of the caller to create a new preview bitmap if the returned
        one is not as desired.
    */
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData)const=0;

    /** Return whether the compression and decompression is lossless.  This
        value is used by the caller of Decompress() to decide whether to use
        the returned bitmap as is or if a new preview has to be created.
    */
    virtual bool IsLossless (void) const = 0;
};



/** Interface for preview bitmap replacements.  Each bitmap
    compressor/decompressor has to provide an implementation that is
    suitable to store the compressed bitmaps.
*/
class BitmapReplacement
{
public:
    virtual sal_Int32 GetMemorySize (void) const { return 0; }
};




/** This is one trivial bitmap compressor.  It stores bitmaps unmodified
    instead of compressing them.
    This compressor is lossless.
*/
class NoBitmapCompression
    : public BitmapCompressor
{
    class DummyReplacement;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (const Bitmap& rpBitmap) const;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** This is another trivial bitmap compressor.  Instead of compressing a
    bitmap, it throws the bitmap away.  Its Decompress() method returns a
    NULL pointer.  The caller has to create a new preview bitmap instead.
    This compressor clearly is not lossless.
*/
class CompressionByDeletion
    : public BitmapCompressor
{
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** Compress a preview bitmap by reducing its resolution.  While the aspect
    ratio is maintained the horizontal resolution is scaled down to 100
    pixels.
    This compressor is not lossless.
*/
class ResolutionReduction
    : public BitmapCompressor
{
    class ResolutionReducedReplacement;
    static const sal_Int32 mnWidth = 100;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (const Bitmap& rpBitmap) const;
    /** Scale the replacement bitmap up to the original size.
    */
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};




/** Compress preview bitmaps using the PNG format.
    This compressor is lossless.
*/
class PngCompression
    : public BitmapCompressor
{
    class PngReplacement;
public:
    virtual ::boost::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const;
    virtual bool IsLossless (void) const;
};


} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
