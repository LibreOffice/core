/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/LintImageCompressor.hxx>

#include <algorithm>
#include <cstddef>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/fileformat.h>
#include <comphelper/propertyvalue.hxx>
#include <cpo/uno/Sequence.hxx>
#include <o3tl/unit_conversion.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/mapmod.hxx>

namespace sd::lint
{
namespace
{
/** Compression level the PNG encoding runs at, the level the image compression dialog starts at. */
constexpr sal_Int32 gnPNGCompression = 6;

} // end of anonymous namespace

tools::Long getTargetPixelCount(tools::Long nLogicLength, sal_Int32 nTargetDPI)
{
    if (nLogicLength <= 0 || nTargetDPI <= 0)
        return 0;

    const double fLengthInInches
        = o3tl::convert(double(nLogicLength), o3tl::Length::mm100, o3tl::Length::in);
    return tools::Long(fLengthInInches * double(nTargetDPI));
}

LintCompressedImage compressGraphic(const Graphic& rGraphic, sal_uInt64 nSourceBytes,
                                    const Size& rLogicSize, sal_Int32 nTargetDPI,
                                    sal_Int32 nJPEGQuality)
{
    LintCompressedImage aResult;

    if (nTargetDPI <= 0)
        return aResult;

    Bitmap aBitmap = rGraphic.GetBitmap();
    const Size aPixelSize = aBitmap.GetSizePixel();
    if (aPixelSize.Width() <= 0 || aPixelSize.Height() <= 0)
        return aResult;

    const tools::Long nTargetWidth = getTargetPixelCount(rLogicSize.Width(), nTargetDPI);
    const tools::Long nTargetHeight = getTargetPixelCount(rLogicSize.Height(), nTargetDPI);
    if (nTargetWidth <= 0 || nTargetHeight <= 0)
        return aResult;

    // An image that already carries fewer pixels than the target asks for keeps the pixels it has.
    const Size aScaledSize(std::min(nTargetWidth, aPixelSize.Width()),
                           std::min(nTargetHeight, aPixelSize.Height()));

    // The default scaling is the one the rest of the program draws a bitmap with. It gets through a
    // slide-sized photograph in a fraction of a second, which fits a single turn of the idle loop
    // the measurement runs in.
    if (aScaledSize != aPixelSize && !aBitmap.Scale(aScaledSize, BmpScaleFlag::Default))
        return aResult;

    const bool bTransparent = rGraphic.IsTransparent() || aBitmap.HasAlpha();

    // JPEG has no way to hold transparency, so a bitmap that carries any goes out as PNG and
    // every opaque bitmap as JPEG.
    const OUString sFormatName = bTransparent ? u"png"_ustr : u"jpg"_ustr;

    const cpo::uno::Sequence<css::beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue(u"Interlaced"_ustr, sal_Int32(0)),
        comphelper::makePropertyValue(u"Compression"_ustr, gnPNGCompression),
        comphelper::makePropertyValue(u"Quality"_ustr, nJPEGQuality)
    };

    // The export works the size of the image out from the unit the bitmap says it prefers. A
    // bitmap that prefers hundredths of a millimetre has that figure worked out by arithmetic
    // alone, with no output device to ask, so the size the image is drawn at goes in as the size it
    // prefers. The resolution fields of the encoding then hold the resolution the image really
    // comes out at.
    aBitmap.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
    aBitmap.SetPrefSize(rLogicSize);

    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    const sal_uInt16 nFormat = rFilter.GetExportFormatNumberForShortName(sFormatName);

    SvMemoryStream aMemoryStream;
    aMemoryStream.SetVersion(SOFFICE_FILEFORMAT_CURRENT);
    if (rFilter.ExportGraphic(Graphic(aBitmap), u"none", aMemoryStream, nFormat, &aFilterData)
        != ERRCODE_NONE)
        return aResult;

    const sal_uInt64 nBytes = aMemoryStream.TellEnd();
    if (nBytes == 0)
        return aResult;

    // The encoded bytes are kept as they are. Turning them into a graphic again is left to the fix
    // step, so a scan that only wants the figure pays for the encoding alone.
    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aResult.maData = BinaryDataContainer(aMemoryStream, std::size_t(nBytes));
    if (aResult.maData.isEmpty())
        return aResult;

    aResult.mbSmaller = nSourceBytes > 0 && nBytes < nSourceBytes;

    return aResult;
}

Graphic importCompressedImage(const LintCompressedImage& rCompressed)
{
    if (rCompressed.maData.isEmpty())
        return Graphic();

    // The stream reads the bytes where they already lie, so the import costs no copy of them.
    SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(rCompressed.maData.getData()),
                                 rCompressed.maData.getSize(), StreamMode::READ);
    aMemoryStream.SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    Graphic aGraphic;
    if (GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, u"import", aMemoryStream)
        != ERRCODE_NONE)
        return Graphic();

    return aGraphic;
}

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
