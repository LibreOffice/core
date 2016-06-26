/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/graph.hxx>
#include <vcl/dllapi.h>
#include <com/sun/star/task/XStatusIndicator.hpp>

#ifndef INCLUDED_VCL_SOURCE_FILTER_PNG_PNGREADER_HXX
#define INCLUDED_VCL_SOURCE_FILTER_PNG_PNGREADER_HXX

namespace vcl
{

class VCL_DLLPUBLIC PngReader
{
    SvStream& mrStream;
    css::uno::Reference< css::task::XStatusIndicator > mxStatusIndicator;

public:
    PngReader(SvStream& rStream);

    virtual ~PngReader()
    {}

    bool Read(BitmapEx& rBitmap);
    bool Read(Graphic& rGraphic);
};

} // namespace vcl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
