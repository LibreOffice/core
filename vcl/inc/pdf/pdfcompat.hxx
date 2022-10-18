/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_features.h>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <vcl/vectorgraphicdata.hxx>

namespace vcl::pdf
{
#if HAVE_FEATURE_PDFIUM

/// Convert to inch, then assume 96 DPI.
inline double pointToPixel(const double fPoint, const double fResolutionDPI)
{
    return fPoint * fResolutionDPI / 72.;
}

/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream, sal_uInt64 nPos, sal_uInt64 nSize);

/// Converts to highest supported format version (currently 1.6).
/// Usually used to deal with missing referenced objects in the
/// source pdf stream.
bool convertToHighestSupported(SvStream& rInStream, SvStream& rOutStream);

#endif // HAVE_FEATURE_PDFIUM

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream);

VectorGraphicDataArray createVectorGraphicDataArray(SvStream& rStream);

} // end of vcl::filter::ipdf namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
