/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/dllapi.h>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vector>

#pragma once

namespace vcl
{
// Similar to png_unknown_chunk
struct PngChunk
{
    std::array<uint8_t, 5> name;
    std::vector<sal_uInt8> data;
    size_t size;
};
class VCL_DLLPUBLIC PngImageWriter
{
    SvStream& mrStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

    sal_Int32 mnCompressionLevel;
    bool mbInterlaced;
    std::vector<PngChunk> maAdditionalChunks;

public:
    PngImageWriter(SvStream& rStream);

    virtual ~PngImageWriter() = default;

    void setParameters(css::uno::Sequence<css::beans::PropertyValue> const& rParameters);
    bool write(const BitmapEx& rBitmap);
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
