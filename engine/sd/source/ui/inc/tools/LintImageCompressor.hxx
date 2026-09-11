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

#pragma once

#include <sal/types.h>
#include <tools/gen.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/graph.hxx>

#include <sddllapi.h>

namespace sd::lint
{
/** An image encoded again at a lower resolution, held as the encoded bytes themselves. The bytes
    are empty when the encoding did not work out. */
struct LintCompressedImage
{
    BinaryDataContainer maData;

    /** True when the new encoding takes fewer bytes than the data the graphic was built from. */
    bool mbSmaller = false;

    /** How many bytes the new encoding takes. */
    sal_uInt64 getByteCount() const { return sal_uInt64(maData.getSize()); }
};

/** Number of pixels the target resolution asks for over the given length in hundredths of a
    millimetre. Returns zero when the length or the resolution leaves nothing to work with. */
tools::Long getTargetPixelCount(tools::Long nLogicLength, sal_Int32 nTargetDPI);

/** Scales a bitmap graphic down to the pixel count the target resolution asks for at the size it is
    drawn at, and encodes it again. nSourceBytes is how many bytes the image takes as the document
    holds it today, which is what decides whether the new encoding came out smaller. rLogicSize is
    the drawing size in hundredths of a millimetre, nTargetDPI the wanted resolution in dots per
    inch, and nJPEGQuality a value from 1 to 100 that the JPEG encoding uses. An image already at
    or below the target keeps every pixel it has. */
SD_DLLPUBLIC LintCompressedImage compressGraphic(const Graphic& rGraphic, sal_uInt64 nSourceBytes,
                                                 const Size& rLogicSize, sal_Int32 nTargetDPI,
                                                 sal_Int32 nJPEGQuality);

/** Turns the encoded bytes back into a graphic. The graphic comes out of the graphic filter, so it
    carries the encoded data it was built from and a document that holds it keeps that data. An
    empty encoding gives an empty graphic. */
Graphic importCompressedImage(const LintCompressedImage& rCompressed);

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
