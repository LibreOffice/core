/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/dllapi.h>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>

#pragma once

namespace vcl
{
class VCL_DLLPUBLIC PngImageWriter
{
    SvStream& mrStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

    sal_Int32 mnCompressionLevel;
    bool mbInterlaced;

public:
    PngImageWriter(SvStream& rStream);

    virtual ~PngImageWriter() {}

    void setParameters(css::uno::Sequence<css::beans::PropertyValue> const& rParameters)
    {
        for (auto const& rValue : rParameters)
        {
            if (rValue.Name == "Compression")
                rValue.Value >>= mnCompressionLevel;
            else if (rValue.Name == "Interlaced")
                rValue.Value >>= mbInterlaced;
        }
    }
    bool write(BitmapEx& rBitmap);
};

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
