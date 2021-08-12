/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_OSX_RASTERCONTEXT_HXX
#define INCLUDED_VCL_INC_SKIA_OSX_RASTERCONTEXT_HXX

#include <tools/sk_app/WindowContext.h>

class AquaSkiaSalGraphicsImpl;

// RasterWindowContext_mac uses OpenGL internally, which
// we don't want, so make our own raster window context
// based on SkBitmap, and our code will handle things like flush.

class AquaSkiaWindowContextRaster : public sk_app::WindowContext
{
public:
    AquaSkiaWindowContextRaster(int w, int h, const sk_app::DisplayParams& params);
    virtual sk_sp<SkSurface> getBackbufferSurface() override { return mSurface; }
    // Not to be called, our mac code should be used.
    virtual void swapBuffers(const SkIRect* = nullptr) override { abort(); }
    virtual bool isValid() override { return mSurface.get(); };
    virtual void resize(int w, int h) override;
    virtual void setDisplayParams(const sk_app::DisplayParams& params) override;

protected:
    virtual bool isGpuContext() override { return false; }

private:
    void createSurface();
    sk_sp<SkSurface> mSurface;
};

#endif // INCLUDED_VCL_INC_SKIA_OSX_RASTERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
