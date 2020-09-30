/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SKIA_INTERFACE_HXX
#define INCLUDED_VCL_SKIA_INTERFACE_HXX

#include <vcl/dllapi.h>

#include <config_features.h>

#if HAVE_FEATURE_SKIA

class SkPaint;
class SkMatrix;
class BitmapEx;
namespace vcl
{
class Region;
}

/**
Interface class giving direct access to Skia features of an output device.
*/
class VCL_DLLPUBLIC SkiaOutDevInterface
{
public:
    // Calls will abort if FailIfSlow is passed and they cannot be fast.
    // This allows e.g. vclcanvas to handle call locally with caching.
    enum SlowHandling
    {
        FailIfSlow,
        ProceedIfSlow
    };
    virtual ~SkiaOutDevInterface();

    // Sets the given clip region. If IsNull(), resets clipping.
    virtual bool setClipRegion(const vcl::Region&) = 0;
    // Return the current clip region.
    virtual const vcl::Region& getClipRegion() const = 0;

    // Draw the bitmap with the given paint, matrix, clip region and alpha modulation.
    virtual bool drawBitmap(const BitmapEx& bitmap, const SkPaint& paint, const SkMatrix& matrix,
                            double alphaModulation, SlowHandling slowHandling = ProceedIfSlow)
        = 0;
};

#endif // HAVE_FEATURE_SKIA

#endif // INCLUDED_VCL_SKIA_INTERFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
