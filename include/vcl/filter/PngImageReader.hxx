/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>

#include <vcl/dllapi.h>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/filter/ImportOutput.hxx>

#include <com/sun/star/uno/Reference.hxx>

constexpr sal_uInt64 PNG_SIGNATURE = 0x89504E470D0A1A0A;
constexpr sal_uInt32 PNG_IHDR_SIGNATURE = 0x49484452;
constexpr sal_uInt32 PNG_IDAT_SIGNATURE = 0x49444154;
constexpr sal_uInt32 PNG_PHYS_SIGNATURE = 0x70485973;
constexpr sal_uInt32 PNG_TRNS_SIGNATURE = 0x74524E53;
constexpr sal_uInt32 PNG_ACTL_SIGNATURE = 0x6163544C;
constexpr sal_uInt32 PNG_FCTL_SIGNATURE = 0x6663544C;
constexpr sal_uInt32 PNG_FDAT_SIGNATURE = 0x66644154;
constexpr sal_uInt32 PNG_IEND_SIGNATURE = 0x49454E44;
constexpr sal_uInt32 PNG_IEND_CRC = 0xAE426082;
constexpr int PNG_SIGNATURE_SIZE = 8;
constexpr int PNG_IHDR_SIZE = 13;
constexpr int PNG_TYPE_SIZE = 4;
constexpr int PNG_SIZE_SIZE = 4;
constexpr int PNG_CRC_SIZE = 4;
constexpr int PNG_IEND_SIZE = 0;

namespace com::sun::star::task
{
class XStatusIndicator;
}

class Bitmap;
class SvStream;

namespace vcl
{
class VCL_DLLPUBLIC PngImageReader
{
    SvStream& mrStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

public:
    PngImageReader(SvStream& rStream);

    // Returns true if image was successfully read without errors.
    // A usable bitmap may be returned even if there were errors (e.g. incomplete image).
    bool read(Bitmap& rBitmap);
    bool read(ImportOutput& rImportOutput);
    // Returns a bitmap without indicating if there were errors.
    Bitmap read();

    // Returns the contents of the msOG chunk (containing a Gif image), if it exists.
    // Does not change position in the stream.
    static BinaryDataContainer getMicrosoftGifChunk(SvStream& rStream);

    static bool isAPng(SvStream& rStream);
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
