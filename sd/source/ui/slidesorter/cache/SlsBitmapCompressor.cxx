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

#include "SlsBitmapCompressor.hxx"

#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/pngread.hxx>
#include <vcl/pngwrite.hxx>

namespace sd { namespace slidesorter { namespace cache {

//===== NoBitmapCompression ===================================================

/** This dummy replacement simply stores a shared pointer to the original
    preview bitmap.
*/
class NoBitmapCompression::DummyReplacement
    : public BitmapReplacement
{
public:
    BitmapEx const maPreview;

    explicit DummyReplacement (const BitmapEx& rPreview) : maPreview(rPreview)  { }
    virtual ~DummyReplacement() {}
    virtual sal_Int32 GetMemorySize() const override { return maPreview.GetSizeBytes(); }
};

std::shared_ptr<BitmapReplacement> NoBitmapCompression::Compress (const BitmapEx& rBitmap) const
{
    return std::shared_ptr<BitmapReplacement>(new DummyReplacement(rBitmap));
}

BitmapEx NoBitmapCompression::Decompress (const BitmapReplacement& rBitmapData) const
{
    return dynamic_cast<const DummyReplacement&>(rBitmapData).maPreview;
}

bool NoBitmapCompression::IsLossless() const
{
    return true;
}

//===== CompressionByDeletion =================================================

std::shared_ptr<BitmapReplacement> CompressionByDeletion::Compress (const BitmapEx& ) const
{
    return std::shared_ptr<BitmapReplacement>();
}

BitmapEx CompressionByDeletion::Decompress (const BitmapReplacement& ) const
{
    // Return a NULL pointer.  This will eventually lead to a request for
    // the creation of a new one.
    return BitmapEx();
}

bool CompressionByDeletion::IsLossless() const
{
    return false;
}

//===== ResolutionReduction ===================================================

/** Store a scaled down bitmap together with the original size.
*/
class ResolutionReduction::ResolutionReducedReplacement : public BitmapReplacement
{
public:
    BitmapEx maPreview;
    Size maOriginalSize;

    virtual ~ResolutionReducedReplacement();
    virtual sal_Int32 GetMemorySize() const override;
};

ResolutionReduction::ResolutionReducedReplacement::~ResolutionReducedReplacement()
{
}

sal_Int32 ResolutionReduction::ResolutionReducedReplacement::GetMemorySize() const
{
    return maPreview.GetSizeBytes();
}

std::shared_ptr<BitmapReplacement> ResolutionReduction::Compress (
    const BitmapEx& rBitmap) const
{
    ResolutionReducedReplacement* pResult = new ResolutionReducedReplacement;
    pResult->maPreview = rBitmap;
    Size aSize (rBitmap.GetSizePixel());
    pResult->maOriginalSize = aSize;
    if (aSize.Width()>0 && aSize.Width()<mnWidth)
    {
        int nHeight = aSize.Height() * mnWidth / aSize.Width() ;
        pResult->maPreview.Scale(Size(mnWidth,nHeight));
    }

    return std::shared_ptr<BitmapReplacement>(pResult);
}

BitmapEx ResolutionReduction::Decompress (const BitmapReplacement& rBitmapData) const
{
    BitmapEx aResult;

    const ResolutionReducedReplacement* pData (
        dynamic_cast<const ResolutionReducedReplacement*>(&rBitmapData));

    if ( pData && ! pData->maPreview.IsEmpty())
    {
        aResult = pData->maPreview;
        if (pData->maOriginalSize.Width() > mnWidth)
            aResult.Scale(pData->maOriginalSize);
    }

    return aResult;
}

bool ResolutionReduction::IsLossless() const
{
    return false;
}

//===== PNGCompression ========================================================

class PngCompression::PngReplacement : public BitmapReplacement
{
public:
    void* mpData;
    sal_Int32 mnDataSize;
    PngReplacement()
        : mpData(nullptr),
          mnDataSize(0)
    {}
    virtual ~PngReplacement()
    {
        delete [] static_cast<char*>(mpData);
    }
    virtual sal_Int32 GetMemorySize() const override
    {
        return mnDataSize;
    }
};

std::shared_ptr<BitmapReplacement> PngCompression::Compress (const BitmapEx& rBitmap) const
{
    vcl::PNGWriter aWriter(rBitmap);
    SvMemoryStream aStream (32768, 32768);
    aWriter.Write(aStream);

    PngReplacement* pResult = new PngReplacement();
    pResult->mnDataSize = aStream.Tell();
    pResult->mpData = new char[pResult->mnDataSize];
    memcpy(pResult->mpData, aStream.GetData(), pResult->mnDataSize);

    return std::shared_ptr<BitmapReplacement>(pResult);
}

BitmapEx PngCompression::Decompress (
    const BitmapReplacement& rBitmapData) const
{
    BitmapEx aResult;
    const PngReplacement* pData (dynamic_cast<const PngReplacement*>(&rBitmapData));
    if (pData != nullptr)
    {
        SvMemoryStream aStream (pData->mpData, pData->mnDataSize, StreamMode::READ);
        vcl::PNGReader aReader (aStream);
        aResult = aReader.Read().GetBitmap();
    }

    return aResult;
}

bool PngCompression::IsLossless() const
{
    return true;
}

} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
