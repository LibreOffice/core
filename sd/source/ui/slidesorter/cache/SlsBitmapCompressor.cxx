/*************************************************************************
 *
 *  $RCSfile: SlsBitmapCompressor.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 10:47:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
    virtual sal_Int32 GetMemorySize (void) const
    {
        return mpPreview->GetSizeBytes();
    }
};




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
    const ::boost::shared_ptr<BitmapEx>& rpBitmap) const
{
    return ::boost::shared_ptr<BitmapReplacement>();
}




::boost::shared_ptr<BitmapEx> CompressionByDeletion::Decompress (
    const BitmapReplacement& rBitmapData) const
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

    virtual sal_Int32 GetMemorySize (void) const
    {
        if (mpPreview.get() != NULL)
            return mpPreview->GetSizeBytes();
        else
            return 0;
    }
};




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
        delete [] mpData;
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

    sal_Int32 nRatio ((100L * (ULONG)pResult->GetSizeBytes()) / (ULONG)pData->mnDataSize);

    return ::boost::shared_ptr<BitmapEx>(pResult);
}




bool PngCompression::IsLossless (void) const
{
    return true;
}




} } } // end of namespace ::sd::slidesorter::cache
