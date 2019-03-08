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
#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>

#ifndef INCLUDED_VCL_FILTER_PNGIMAGEREADER_HXX
#define INCLUDED_VCL_FILTER_PNGIMAGEREADER_HXX

namespace vcl
{
class VCL_DLLPUBLIC PngImageReader
{
    SvStream& mrStream;
    css::uno::Reference<css::task::XStatusIndicator> mxStatusIndicator;

public:
    PngImageReader(SvStream& rStream);

    bool read(BitmapEx& rBitmap);
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
