/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>

#include <array>
#include <stdint.h>
#include <vector>

#pragma once

class Graphic;
class SvStream;
namespace com::sun::star::beans
{
struct PropertyValue;
}
namespace com::sun::star::task
{
class XStatusIndicator;
}
namespace com::sun::star::uno
{
template <class E> class Sequence;
}

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
    bool mbInterlaced, mbTranslucent;
    std::vector<PngChunk> maAdditionalChunks;

public:
    PngImageWriter(SvStream& rStream);

    void setParameters(css::uno::Sequence<css::beans::PropertyValue> const& rParameters);
    bool write(const Graphic& rGraphic);
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
