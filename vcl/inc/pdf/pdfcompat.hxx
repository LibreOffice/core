/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/unit_conversion.hxx>
#include <tools/stream.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>

namespace vcl::pdf
{
/// Convert to inch, then apply custom resolution.
inline double pointToPixel(const double fPoint, const double fResolutionDPI)
{
    return o3tl::convert(fPoint, o3tl::Length::pt, o3tl::Length::in) * fResolutionDPI;
}

/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream);

/// Converts to highest supported format version (currently 1.6).
/// Usually used to deal with missing referenced objects in the
/// source pdf stream.
/// The conversion takes place if either the stream is encrypted, or 'bForce' is true
bool convertToHighestSupported(
    SvStream& rInStream, SvStream& rOutStream,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler, bool bForce,
    bool& bEncrypted);

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(
    SvStream& rInStream, SvStream& rOutStream,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler,
    bool& bEncrypted);

BinaryDataContainer createBinaryDataContainer(
    SvStream& rStream, bool& bEncrypted,
    const css::uno::Reference<css::task::XInteractionHandler>& xInteractionHandler = nullptr);

} // end of vcl::filter::ipdf namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
