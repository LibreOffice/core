/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/skia/SkiaHelper.hxx>

#if !HAVE_FEATURE_SKIA

namespace SkiaHelper
{
bool isVCLSkiaEnabled() { return false; }

} // namespace

#else

#include <rtl/bootstrap.hxx>
#include <vcl/svapp.hxx>
#include <desktop/crashreport.hxx>
#include <officecfg/Office/Common.hxx>
#include <watchdog.hxx>
#include <skia/zone.hxx>
#include <sal/log.hxx>
#include <driverblocklist.hxx>
#include <skia/utils.hxx>
#include <config_folders.h>

#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkSurface.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

namespace SkiaHelper
{
uint32_t vendorId = 0;

static OUString getBlacklistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/skia/skia_blacklist_vulkan.xml";
}

static bool isVulkanBlacklisted(const VkPhysicalDeviceProperties& props)
{
    static const char* const types[]
        = { "other", "integrated", "discrete", "virtual", "cpu", "??" }; // VkPhysicalDeviceType
    OUString driverVersion = OUString::number(props.driverVersion >> 22) + "."
                             + OUString::number((props.driverVersion >> 12) & 0x3ff) + "."
                             + OUString::number(props.driverVersion & 0xfff);
    vendorId = props.vendorID;
    OUString vendorIdStr = "0x" + OUString::number(props.vendorID, 16);
    OUString deviceIdStr = "0x" + OUString::number(props.deviceID, 16);
    SAL_INFO("vcl.skia",
             "Vulkan API version: "
                 << (props.apiVersion >> 22) << "." << ((props.apiVersion >> 12) & 0x3ff) << "."
                 << (props.apiVersion & 0xfff) << ", driver version: " << driverVersion
                 << ", vendor: " << vendorIdStr << ", device: " << deviceIdStr << ", type: "
                 << types[std::min<unsigned>(props.deviceType, SAL_N_ELEMENTS(types) - 1)]
                 << ", name: " << props.deviceName);
    return DriverBlocklist::IsDeviceBlocked(getBlacklistFile(), driverVersion, vendorIdStr,
                                            deviceIdStr);
}

static sk_app::VulkanWindowContext::SharedGrContext getTemporaryGrContext();

static void checkDeviceBlacklisted(bool blockDisable = false)
{
    static bool done = false;
    if (!done)
    {
        SkiaZone zone;

        switch (renderMethodToUse())
        {
            case RenderVulkan:
            {
                // First try if a GrContext already exists.
                sk_app::VulkanWindowContext::SharedGrContext grContext
                    = sk_app::VulkanWindowContext::getSharedGrContext();
                if (!grContext.getGrContext())
                {
                    // This function is called from isVclSkiaEnabled(), which
                    // may be called when deciding which X11 visual to use,
                    // and that visual is normally needed when creating
                    // Skia's VulkanWindowContext, which is needed for the GrContext.
                    // Avoid the loop by creating a temporary GrContext
                    // that will use the default X11 visual (that shouldn't matter
                    // for just finding out information about Vulkan) and destroying
                    // the temporary context will clean up again.
                    grContext = getTemporaryGrContext();
                }
                bool blacklisted = true; // assume the worst
                if (grContext.getGrContext()) // Vulkan was initialized properly
                {
                    blacklisted = isVulkanBlacklisted(
                        sk_app::VulkanWindowContext::getPhysDeviceProperties());
                    SAL_INFO("vcl.skia", "Vulkan blacklisted: " << blacklisted);
                }
                else
                    SAL_INFO("vcl.skia", "Vulkan could not be initialized");
                if (blacklisted && !blockDisable)
                    disableRenderMethod(RenderVulkan);
                break;
            }
            case RenderRaster:
                SAL_INFO("vcl.skia", "Using Skia raster mode");
                return; // software, never blacklisted
        }
        done = true;
    }
}

static bool skiaSupportedByBackend = false;
static bool supportsVCLSkia()
{
    if (!skiaSupportedByBackend)
    {
        SAL_INFO("vcl.skia", "Skia not supported by VCL backend, disabling");
        return false;
    }
    return getenv("SAL_DISABLESKIA") == nullptr;
}

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
        // don't actually block if blacklisted, but log it if enabled, and also get the vendor id
        checkDeviceBlacklisted(true);
    }
    else if (getenv("SAL_FORCEGL"))
    {
        // Skia usage is checked before GL usage, so if GL is forced (and Skia is not), do not
        // enable Skia in order to allow GL.
        bRet = false;
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
        if (strcmp(env, "vulkan") == 0)
        {
            methodToUse = RenderVulkan;
            return true;
        }
        SAL_WARN("vcl.skia", "Unrecognized value of SAL_SKIA");
        abort();
    }
    if (officecfg::Office::Common::VCL::ForceSkiaRaster::get())
    {
        methodToUse = RenderRaster;
        return true;
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

static std::unique_ptr<sk_app::WindowContext> (*createVulkanWindowContextFunction)(bool) = nullptr;
static void setCreateVulkanWindowContext(std::unique_ptr<sk_app::WindowContext> (*function)(bool))
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
    if (createVulkanWindowContextFunction == nullptr)
        return nullptr; // not initialized properly (e.g. used from a VCL backend with no Skia support)
    std::unique_ptr<sk_app::WindowContext> tmpContext = createVulkanWindowContextFunction(false);
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

static sk_app::VulkanWindowContext::SharedGrContext getTemporaryGrContext()
{
    if (createVulkanWindowContextFunction == nullptr)
        return sk_app::VulkanWindowContext::SharedGrContext();
    std::unique_ptr<sk_app::WindowContext> tmpContext = createVulkanWindowContextFunction(true);
    // Set up using the shared context created by the call above, if successful.
    return sk_app::VulkanWindowContext::getSharedGrContext();
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

sk_sp<SkImage> createSkImage(const SkBitmap& bitmap)
{
    SkiaZone zone;
    assert(bitmap.colorType() == kN32_SkColorType || bitmap.colorType() == kAlpha_8_SkColorType);
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            if (GrContext* grContext = getSharedGrContext())
            {
                sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(
                    grContext, SkBudgeted::kNo, bitmap.info().makeAlphaType(kPremul_SkAlphaType));
                assert(surface);
                SkPaint paint;
                paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
                surface->getCanvas()->drawBitmap(bitmap, 0, 0, &paint);
                return surface->makeImageSnapshot();
            }
            break;
        }
        default:
            break;
    }
    // Create raster image as a fallback.
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    assert(image);
    return image;
}

void cleanup()
{
    delete sharedGrContext;
    sharedGrContext = nullptr;
}

// Skia should not be used from VCL backends that do not actually support it, as there will be setup missing.
// The code here (that is in the vcl lib) needs a function for creating Vulkan context that is
// usually available only in the backend libs.
void prepareSkia(std::unique_ptr<sk_app::WindowContext> (*createVulkanWindowContext)(bool))
{
    setCreateVulkanWindowContext(createVulkanWindowContext);
    skiaSupportedByBackend = true;
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
