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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSBITMAPCOMPRESSOR_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_CACHE_SLSBITMAPCOMPRESSOR_HXX

#include <sal/types.h>
#include <memory>

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
    virtual std::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const = 0;

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
    virtual bool IsLossless() const = 0;

protected:
    ~BitmapCompressor() {}
};

/** Interface for preview bitmap replacements.  Each bitmap
    compressor/decompressor has to provide an implementation that is
    suitable to store the compressed bitmaps.
*/
class BitmapReplacement
{
public:
    virtual sal_Int32 GetMemorySize() const { return 0; }

protected:
    ~BitmapReplacement() {}
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
    virtual ~NoBitmapCompression() {}
    virtual std::shared_ptr<BitmapReplacement> Compress (const Bitmap& rpBitmap) const SAL_OVERRIDE;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const SAL_OVERRIDE;
    virtual bool IsLossless() const SAL_OVERRIDE;
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
    virtual ~CompressionByDeletion() {}
    virtual std::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const SAL_OVERRIDE;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const SAL_OVERRIDE;
    virtual bool IsLossless() const SAL_OVERRIDE;
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
    virtual ~ResolutionReduction() {}
    virtual std::shared_ptr<BitmapReplacement> Compress (const Bitmap& rpBitmap) const SAL_OVERRIDE;
    /** Scale the replacement bitmap up to the original size.
    */
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const SAL_OVERRIDE;
    virtual bool IsLossless() const SAL_OVERRIDE;
};

/** Compress preview bitmaps using the PNG format.
    This compressor is lossless.
*/
class PngCompression
    : public BitmapCompressor
{
    class PngReplacement;
public:
    virtual ~PngCompression() {}
    virtual std::shared_ptr<BitmapReplacement> Compress (const Bitmap& rBitmap) const SAL_OVERRIDE;
    virtual Bitmap Decompress (const BitmapReplacement& rBitmapData) const SAL_OVERRIDE;
    virtual bool IsLossless() const SAL_OVERRIDE;
};

} } } // end of namespace ::sd::slidesorter::cache

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
