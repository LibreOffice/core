/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_FILTER_PNGIMAGEREADER_HXX
#define INCLUDED_VCL_FILTER_PNGIMAGEREADER_HXX

#include <vcl/dllapi.h>

#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::task
{
class XStatusIndicator;
}

class BitmapEx;
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
    bool read(BitmapEx& rBitmap);
    // Returns a bitmap without indicating if there were errors.
    BitmapEx read();

    // Returns the contents of the msOG chunk (containing a Gif image), if it exists.
    // Does not change position in the stream.
    static std::unique_ptr<sal_uInt8[]> getMicrosoftGifChunk(SvStream& rStream,
                                                             sal_Int32* chunkSize = nullptr);
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
