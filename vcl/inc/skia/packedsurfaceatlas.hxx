/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_SKIA_PACKEDSURFACEATLAS_HXX
#define INCLUDED_VCL_INC_SKIA_PACKEDSURFACEATLAS_HXX

#include <memory>

#include <SkSurface.h>

#include <vcl/dllapi.h>
#include <tools/gen.hxx>

/**
 * SkSurface that is actually packed in a larger SkSurface atlas.
 *
 * In Skia's case we draw into SkSurface (as that's what's GPU-backed),
 * but then for using the result we need to get an associated SkImage.
 * The use of SkSurface::makeImageSnapshot() complicates things in two ways:
 * - it does data copy if we want a sub-rectangle, so we need to pass a reference
 *   and the geometry wanted
 * - it does a snapshot of the state, meaning any further drawing into the SkSurface
 *   would detach by creating a copy, so we need to pass around the SkSurface
 *   reference and then create only a temporary SkImage for the whole SkSurface
 *   (which should be almost a no-op)
 */
class VCL_DLLPUBLIC SkiaPackedSurface
{
public:
    sk_sp<SkSurface> mSurface;
    tools::Rectangle mRect; // the area in the surface that is this "surface"
    SkiaPackedSurface(const sk_sp<SkSurface>& surface, const tools::Rectangle& rect)
        : mSurface(surface)
        , mRect(rect)
    {
    }
    SkiaPackedSurface() = default;
};

/**
 * Pack Skia "surfaces" into one surface atlas.
 *
 * This is based on algorithm described in [1] and is an
 * adaptation of "texture atlas generator" from [2].
 *
 * [1]: http://www.blackpawn.com/texts/lightmaps/
 * [2]: https://github.com/lukaszdk/texture-atlas-generator
 *
 */
class VCL_DLLPUBLIC SkiaPackedSurfaceAtlasManager final
{
    struct PackedSurface;
    std::vector<std::unique_ptr<PackedSurface>> maPackedSurfaces;

    int const mnSurfaceWidth;
    int const mnSurfaceHeight;

    void CreateNewSurface();

    SkiaPackedSurfaceAtlasManager(const SkiaPackedSurfaceAtlasManager&) = delete;
    SkiaPackedSurfaceAtlasManager& operator=(const SkiaPackedSurfaceAtlasManager&) = delete;

public:
    /**
     * nSurfaceWidth and nSurfaceHeight are the dimensions of the common surface(s)
     */
    SkiaPackedSurfaceAtlasManager(int nSurfaceWidth, int nSurfaceHeight);
    ~SkiaPackedSurfaceAtlasManager();

    SkiaPackedSurface Reserve(int nWidth, int nHeight);
    std::vector<sk_sp<SkSurface>> ReduceSurfaceNumber(int nMaxNumberOfSurfaces);
};

#endif // INCLUDED_VCL_INC_SKIA_PACKEDSURFACEATLAS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
