/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_X11_GDIIMPL_HXX
#define INCLUDED_VCL_INC_SKIA_X11_GDIIMPL_HXX

#include <vcl/dllapi.h>

#include <unx/salgdi.h>
#include <unx/x11/x11gdiimpl.h>
#include <skia/gdiimpl.hxx>
#include <skia/utils.hxx>

class VCL_PLUGIN_PUBLIC X11SkiaSalGraphicsImpl final : public SkiaSalGraphicsImpl,
                                                       public X11GraphicsImpl
{
private:
    X11SalGraphics& mX11Parent;

public:
    X11SkiaSalGraphicsImpl(X11SalGraphics& rParent);
    virtual ~X11SkiaSalGraphicsImpl() override;

    virtual void Init() override;
    virtual void DeInit() override;
    virtual void freeResources() override;
    virtual void Flush() override;

    static void prepareSkia();

private:
    virtual void createWindowContext(bool forceRaster = false) override;
    virtual void performFlush() override;
    virtual bool avoidRecreateByResize() const override;
    static std::unique_ptr<sk_app::WindowContext>
    createWindowContext(Display* display, Drawable drawable, const XVisualInfo* visual, int width,
                        int height, SkiaHelper::RenderMethod renderMethod, bool temporary);
    friend std::unique_ptr<sk_app::WindowContext> createVulkanWindowContext(bool);
};

#endif // INCLUDED_VCL_INC_SKIA_X11_GDIIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
