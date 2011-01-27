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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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
    ::boost::shared_ptr<BitmapEx> mpPreview;
    Size maOriginalSize;

    DummyReplacement (const ::boost::shared_ptr<BitmapEx>& rpPreview) : mpPreview(rpPreview)
    {
    }

    virtual ~DummyReplacement();

    virtual sal_Int32 GetMemorySize (void) const;
};

NoBitmapCompression::DummyReplacement::~DummyReplacement()
{
}

sal_Int32 NoBitmapCompression::DummyReplacement::GetMemorySize (void) const
{
    return mpPreview->GetSizeBytes();
}

::boost::shared_ptr<BitmapReplacement> NoBitmapCompression::Compress (
    const ::boost::shared_ptr<BitmapEx>& rpBitmap) const
{
    return ::boost::shared_ptr<BitmapReplacement>(new DummyReplacement(rpBitmap));
}

::boost::shared_ptr<BitmapEx> NoBitmapCompression::Decompress (
    const BitmapReplacement& rBitmapData) const
{
    return dynamic_cast<const DummyReplacement&>(rBitmapData).mpPreview;
}




bool NoBitmapCompression::IsLossless (void) const
{
    return true;
}




//===== CompressionByDeletion =================================================

::boost::shared_ptr<BitmapReplacement> CompressionByDeletion::Compress (
    const ::boost::shared_ptr<BitmapEx>& ) const
{
    return ::boost::shared_ptr<BitmapReplacement>();
}




::boost::shared_ptr<BitmapEx> CompressionByDeletion::Decompress (
    const BitmapReplacement& ) const
{
    // Return a NULL pointer.  This will eventually lead to a request for
    // the creation of a new one.
    return ::boost::shared_ptr<BitmapEx>();
}




bool CompressionByDeletion::IsLossless (void) const
{
    return false;
}




//===== ResolutionReduction ===================================================

/** Store a scaled down bitmap together with the original size.
*/
class ResolutionReduction::ResolutionReducedReplacement : public BitmapReplacement
{
public:
    ::boost::shared_ptr<BitmapEx> mpPreview;
    Size maOriginalSize;

    virtual ~ResolutionReducedReplacement();

    virtual sal_Int32 GetMemorySize (void) const;
};

ResolutionReduction::ResolutionReducedReplacement::~ResolutionReducedReplacement()
{
}

sal_Int32 ResolutionReduction::ResolutionReducedReplacement::GetMemorySize (void) const
{
    if (mpPreview.get() != NULL)
        return mpPreview->GetSizeBytes();
    else
        return 0;
}

::boost::shared_ptr<BitmapReplacement> ResolutionReduction::Compress (
    const ::boost::shared_ptr<BitmapEx>& rpBitmap) const
{
    ResolutionReducedReplacement* pResult = new ResolutionReducedReplacement();
    pResult->mpPreview.reset(new BitmapEx(*rpBitmap));
    Size aSize (rpBitmap->GetSizePixel());
    pResult->maOriginalSize = aSize;
    if (aSize.Width()>0 && aSize.Width()<mnWidth)
    {
        int nHeight = aSize.Height() * mnWidth / aSize.Width() ;
        pResult->mpPreview->Scale(Size(mnWidth,nHeight));
    }

    return ::boost::shared_ptr<BitmapReplacement>(pResult);
}




::boost::shared_ptr<BitmapEx> ResolutionReduction::Decompress (
    const BitmapReplacement& rBitmapData) const
{
    ::boost::shared_ptr<BitmapEx> pResult;

    const ResolutionReducedReplacement* pData (
        dynamic_cast<const ResolutionReducedReplacement*>(&rBitmapData));

    if (pData->mpPreview.get() != NULL)
    {
        pResult.reset(new BitmapEx(*pData->mpPreview));
        if (pData->maOriginalSize.Width() > mnWidth)
            pResult->Scale(pData->maOriginalSize);
    }

    return pResult;
}




bool ResolutionReduction::IsLossless (void) const
{
    return false;
}




//===== PNGCompression ========================================================


class PngCompression::PngReplacement : public BitmapReplacement
{
public:
    void* mpData;
    sal_Int32 mnDataSize;
    Size maImageSize;
    PngReplacement (void)
        : mpData(NULL),
          mnDataSize(0),
          maImageSize(0,0)
    {}
    virtual ~PngReplacement (void)
    {
        delete [] (char*)mpData;
    }
    virtual sal_Int32 GetMemorySize (void) const
    {
        return mnDataSize;
    }
};




::boost::shared_ptr<BitmapReplacement> PngCompression::Compress (
    const ::boost::shared_ptr<BitmapEx>& rpBitmap) const
{
    ::vcl::PNGWriter aWriter (*rpBitmap);
    SvMemoryStream aStream (32768, 32768);
    aWriter.Write(aStream);

    PngReplacement* pResult = new PngReplacement();
    pResult->maImageSize = rpBitmap->GetSizePixel();
    pResult->mnDataSize = aStream.Tell();
    pResult->mpData = new char[pResult->mnDataSize];
    memcpy(pResult->mpData, aStream.GetData(), pResult->mnDataSize);

    return ::boost::shared_ptr<BitmapReplacement>(pResult);
}




::boost::shared_ptr<BitmapEx> PngCompression::Decompress (
    const BitmapReplacement& rBitmapData) const
{
    BitmapEx* pResult = NULL;
    const PngReplacement* pData (dynamic_cast<const PngReplacement*>(&rBitmapData));
    if (pData != NULL)
    {
        SvMemoryStream aStream (pData->mpData, pData->mnDataSize, STREAM_READ);
        ::vcl::PNGReader aReader (aStream);
        pResult = new BitmapEx(aReader.Read());
    }

    return ::boost::shared_ptr<BitmapEx>(pResult);
}




bool PngCompression::IsLossless (void) const
{
    return true;
}




} } } // end of namespace ::sd::slidesorter::cache

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
