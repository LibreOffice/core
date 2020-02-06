/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/skia/SkiaHelper.hxx>

#include <vcl/svapp.hxx>
#include <desktop/crashreport.hxx>
#include <officecfg/Office/Common.hxx>
#include <watchdog.hxx>
#include <skia/zone.hxx>
#include <sal/log.hxx>

#if !HAVE_FEATURE_SKIA

namespace SkiaHelper
{
bool isVCLSkiaEnabled() { return false; }

} // namespace

#else

#include <skia/utils.hxx>

#include <SkSurface.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

namespace SkiaHelper
{
static bool isVulkanBlacklisted(const VkPhysicalDeviceProperties& props)
{
    static const char* const types[]
        = { "other", "integrated", "discrete", "virtual", "cpu", "??" }; // VkPhysicalDeviceType
    SAL_INFO("vcl.skia",
             "Vulkan API version: "
                 << (props.apiVersion >> 22) << "." << ((props.apiVersion >> 12) & 0x3ff) << "."
                 << (props.apiVersion & 0xfff) << ", driver version: " << std::hex
                 << props.driverVersion << ", vendor:" << props.vendorID
                 << ", device: " << props.deviceID << std::dec
                 << ", type: " << types[std::min<unsigned>(props.deviceType, SAL_N_ELEMENTS(types))]
                 << ", name: " << props.deviceName);
    return false;
}

static void checkDeviceBlacklisted()
{
    static bool done = false;
    if (!done)
    {
        SkiaZone zone;

        switch (renderMethodToUse())
        {
            case RenderVulkan:
            {
                GrContext* grContext = SkiaHelper::getSharedGrContext();
                bool blacklisted = true; // assume the worst
                if (grContext) // Vulkan was initialized properly
                {
                    blacklisted = isVulkanBlacklisted(
                        sk_app::VulkanWindowContext::getPhysDeviceProperties());
                    SAL_INFO("vcl.skia", "Vulkan blacklisted: " << blacklisted);
                }
                else
                    SAL_INFO("vcl.skia", "Vulkan could not be initialized");
                if (blacklisted)
                    disableRenderMethod(RenderVulkan);
                break;
            }
            case RenderRaster:
                return; // software, never blacklisted
        }
        done = true;
    }
}

static bool supportsVCLSkia() { return !getenv("SAL_DISABLESKIA"); }

bool isVCLSkiaEnabled()
{
    /**
     * The !bSet part should only be called once! Changing the results in the same
     * run will mix Skia and normal rendering.
     */

    static bool bSet = false;
    static bool bEnable = false;
    static bool bForceSkia = false;

    // No hardware rendering, so no Skia
    // TODO SKIA
    if (Application::IsBitmapRendering())
        return false;

    if (bSet)
    {
        return bForceSkia || bEnable;
    }

    /*
     * There are a number of cases that these environment variables cover:
     *  * SAL_FORCESKIA forces Skia independent of any other option
     *  * SAL_DISABLESKIA avoids the use of Skia if SAL_FORCESKIA is not set
     */

    bSet = true;
    bForceSkia = !!getenv("SAL_FORCESKIA") || officecfg::Office::Common::VCL::ForceSkia::get();

    bool bRet = false;
    bool bSupportsVCLSkia = supportsVCLSkia();
    if (bForceSkia)
    {
        bRet = true;
    }
    else if (bSupportsVCLSkia)
    {
        static bool bEnableSkiaEnv = !!getenv("SAL_ENABLESKIA");

        bEnable = bEnableSkiaEnv;

        if (officecfg::Office::Common::VCL::UseSkia::get())
            bEnable = true;

        // Force disable in safe mode
        if (Application::IsSafeModeEnabled())
            bEnable = false;

        if (bEnable)
            checkDeviceBlacklisted(); // switch to raster if driver is blacklisted

        bRet = bEnable;
    }

    if (bRet)
        WatchdogThread::start();

    CrashReporter::addKeyValue("UseSkia", OUString::boolean(bRet), CrashReporter::Write);

    return bRet;
}

static RenderMethod methodToUse = RenderRaster;

static bool initRenderMethodToUse()
{
    if (const char* env = getenv("SAL_SKIA"))
    {
        if (strcmp(env, "raster") == 0)
        {
            methodToUse = RenderRaster;
            return true;
        }
    }
    methodToUse = RenderVulkan;
    return true;
}

RenderMethod renderMethodToUse()
{
    static bool methodToUseInited = initRenderMethodToUse();
    if (methodToUseInited) // Used just to ensure thread-safe one-time init.
        return methodToUse;
    abort();
}

void disableRenderMethod(RenderMethod method)
{
    if (renderMethodToUse() != method)
        return;
    // Choose a fallback, right now always raster.
    methodToUse = RenderRaster;
}

static sk_app::VulkanWindowContext::SharedGrContext* sharedGrContext;

static std::unique_ptr<sk_app::WindowContext> (*createVulkanWindowContextFunction)() = nullptr;
void setCreateVulkanWindowContext(std::unique_ptr<sk_app::WindowContext> (*function)())
{
    createVulkanWindowContextFunction = function;
}

GrContext* getSharedGrContext()
{
    SkiaZone zone;
    assert(renderMethodToUse() == RenderVulkan);
    if (sharedGrContext)
        return sharedGrContext->getGrContext();
    // TODO mutex?
    // Set up the shared GrContext from Skia's (patched) VulkanWindowContext, if it's been
    // already set up.
    sk_app::VulkanWindowContext::SharedGrContext context
        = sk_app::VulkanWindowContext::getSharedGrContext();
    GrContext* grContext = context.getGrContext();
    if (grContext)
    {
        sharedGrContext = new sk_app::VulkanWindowContext::SharedGrContext(context);
        return grContext;
    }
    static bool done = false;
    if (done)
        return nullptr;
    done = true;
    if (!createVulkanWindowContextFunction)
        return nullptr;
    std::unique_ptr<sk_app::WindowContext> tmpContext = createVulkanWindowContextFunction();
    // Set up using the shared context created by the call above, if successful.
    context = sk_app::VulkanWindowContext::getSharedGrContext();
    grContext = context.getGrContext();
    if (grContext)
    {
        sharedGrContext = new sk_app::VulkanWindowContext::SharedGrContext(context);
        return grContext;
    }
    disableRenderMethod(RenderVulkan);
    return nullptr;
}

sk_sp<SkSurface> createSkSurface(int width, int height, SkColorType type)
{
    SkiaZone zone;
    assert(type == kN32_SkColorType || type == kAlpha_8_SkColorType);
    sk_sp<SkSurface> surface;
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            if (GrContext* grContext = getSharedGrContext())
            {
                surface = SkSurface::MakeRenderTarget(
                    grContext, SkBudgeted::kNo,
                    SkImageInfo::Make(width, height, type, kPremul_SkAlphaType));
                assert(surface);
#ifdef DBG_UTIL
                prefillSurface(surface);
#endif
                return surface;
            }
            break;
        }
        default:
            break;
    }
    // Create raster surface as a fallback.
    surface = SkSurface::MakeRaster(SkImageInfo::Make(width, height, type, kPremul_SkAlphaType));
    assert(surface);
#ifdef DBG_UTIL
    prefillSurface(surface);
#endif
    return surface;
}

void cleanup()
{
    delete sharedGrContext;
    sharedGrContext = nullptr;
}

#ifdef DBG_UTIL
void prefillSurface(sk_sp<SkSurface>& surface)
{
    // Pre-fill the surface with deterministic garbage.
    SkBitmap bitmap;
    bitmap.allocN32Pixels(2, 2);
    SkPMColor* scanline;
    scanline = bitmap.getAddr32(0, 0);
    *scanline++ = SkPreMultiplyARGB(0xFF, 0xBF, 0x80, 0x40);
    *scanline++ = SkPreMultiplyARGB(0xFF, 0x40, 0x80, 0xBF);
    scanline = bitmap.getAddr32(0, 1);
    *scanline++ = SkPreMultiplyARGB(0xFF, 0xE3, 0x5C, 0x13);
    *scanline++ = SkPreMultiplyARGB(0xFF, 0x13, 0x5C, 0xE3);
    bitmap.setImmutable();
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    paint.setShader(bitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat));
    surface->getCanvas()->drawPaint(paint);
}

void dump(const SkBitmap& bitmap, const char* file) { dump(SkImage::MakeFromBitmap(bitmap), file); }

void dump(const sk_sp<SkSurface>& surface, const char* file)
{
    surface->getCanvas()->flush();
    dump(surface->makeImageSnapshot(), file);
}

void dump(const sk_sp<SkImage>& image, const char* file)
{
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}

#endif

} // namespace

#endif // HAVE_FEATURE_SKIA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
