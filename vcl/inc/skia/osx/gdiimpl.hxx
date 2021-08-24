/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SKIA_OSX_GDIIMPL_HXX
#define INCLUDED_VCL_INC_SKIA_OSX_GDIIMPL_HXX

#include <vcl/dllapi.h>

#include <quartz/salgdi.h>

#include <skia/gdiimpl.hxx>
#include <skia/utils.hxx>

#include <SkFontMgr.h>

class VCL_PLUGIN_PUBLIC AquaSkiaSalGraphicsImpl final : public SkiaSalGraphicsImpl,
                                                        public AquaGraphicsBackendBase
{
public:
    AquaSkiaSalGraphicsImpl(AquaSalGraphics& rParent, AquaSharedAttributes& rShared);
    virtual ~AquaSkiaSalGraphicsImpl() override;

    virtual void DeInit() override;
    virtual void freeResources() override;
    //    virtual void Flush() override;

    virtual void UpdateGeometryProvider(SalGeometryProvider* provider) override
    {
        setProvider(provider);
    }
    static void prepareSkia();

    virtual bool drawNativeControl(ControlType nType, ControlPart nPart,
                                   const tools::Rectangle& rControlRegion, ControlState nState,
                                   const ImplControlValue& aValue) override;

    virtual void drawTextLayout(const GenericSalLayout& layout) override;

    virtual void Flush() override;
    virtual void Flush(const tools::Rectangle&) override;

private:
    virtual void createWindowSurfaceInternal(bool forceRaster = false) override;
    virtual void destroyWindowSurfaceInternal() override;
    virtual void performFlush() override;
    void flushToScreenRaster(const SkIRect& rect);
    void flushToScreenMetal(const SkIRect& rect);
    static inline sk_sp<SkFontMgr> fontManager;
    // This one is used only for Metal, and only indirectly.
    std::unique_ptr<sk_app::WindowContext> mWindowContext;
};

#endif // INCLUDED_VCL_INC_SKIA_OSX_GDIIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
