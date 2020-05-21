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
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <list>

#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkSurface.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

namespace SkiaHelper
{
static OUString getBlacklistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/skia/skia_blacklist_vulkan.xml";
}

static uint32_t driverVersion = 0;
uint32_t vendorId = 0;

static OUString versionAsString(uint32_t version)
{
    return OUString::number(version >> 22) + "." + OUString::number((version >> 12) & 0x3ff) + "."
           + OUString::number(version & 0xfff);
}

static OUStringLiteral vendorAsString(uint32_t vendor)
{
    return DriverBlocklist::GetVendorNameFromId(vendor);
}

static OUString getCacheFolder()
{
    OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER
                 "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    rtl::Bootstrap::expandMacros(url);
    osl::Directory::create(url);
    return url;
}

static void writeToLog(SvStream& stream, const char* key, const char* value)
{
    stream.WriteCharPtr(key);
    stream.WriteCharPtr(": ");
    stream.WriteCharPtr(value);
    stream.WriteChar('\n');
}

static void writeToLog(SvStream& stream, const char* key, const OUString& value)
{
    writeToLog(stream, key, OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr());
}

// Note that this function also logs system information about Vulkan.
static bool isVulkanBlacklisted(const VkPhysicalDeviceProperties& props)
{
    static const char* const types[]
        = { "other", "integrated", "discrete", "virtual", "cpu", "??" }; // VkPhysicalDeviceType
    driverVersion = props.driverVersion;
    vendorId = props.vendorID;
    OUString vendorIdStr = "0x" + OUString::number(props.vendorID, 16);
    OUString deviceIdStr = "0x" + OUString::number(props.deviceID, 16);
    OUString driverVersionString = versionAsString(driverVersion);
    OUString apiVersion = versionAsString(props.apiVersion);
    const char* deviceType = types[std::min<unsigned>(props.deviceType, SAL_N_ELEMENTS(types) - 1)];

    CrashReporter::addKeyValue("VulkanVendor", vendorIdStr, CrashReporter::AddItem);
    CrashReporter::addKeyValue("VulkanDevice", deviceIdStr, CrashReporter::AddItem);
    CrashReporter::addKeyValue("VulkanAPI", apiVersion, CrashReporter::AddItem);
    CrashReporter::addKeyValue("VulkanDriver", driverVersionString, CrashReporter::AddItem);
    CrashReporter::addKeyValue("VulkanDeviceType", OUString::createFromAscii(deviceType),
                               CrashReporter::AddItem);
    CrashReporter::addKeyValue("VulkanDeviceName", OUString::createFromAscii(props.deviceName),
                               CrashReporter::Write);

    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE);
    writeToLog(logFile, "RenderMethod", "vulkan");
    writeToLog(logFile, "Vendor", vendorIdStr);
    writeToLog(logFile, "Device", deviceIdStr);
    writeToLog(logFile, "API", apiVersion);
    writeToLog(logFile, "Driver", driverVersionString);
    writeToLog(logFile, "DeviceType", deviceType);
    writeToLog(logFile, "DeviceName", props.deviceName);

    SAL_INFO("vcl.skia",
             "Vulkan API version: " << apiVersion << ", driver version: " << driverVersionString
                                    << ", vendor: " << vendorIdStr << " ("
                                    << vendorAsString(vendorId) << "), device: " << deviceIdStr
                                    << ", type: " << deviceType << ", name: " << props.deviceName);
    bool blacklisted = DriverBlocklist::IsDeviceBlocked(getBlacklistFile(), driverVersionString,
                                                        vendorIdStr, deviceIdStr);
    writeToLog(logFile, "Blacklisted", blacklisted ? "yes" : "no");
    return blacklisted;
}

static void writeSkiaRasterInfo()
{
    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE);
    writeToLog(logFile, "RenderMethod", "raster");
    // Log compiler, Skia works best when compiled with Clang.
#if defined __clang__
    writeToLog(logFile, "Compiler", "Clang");
#elif defined __GNUC__
    writeToLog(logFile, "Compiler", "GCC");
#elif defined _MSC_VER
    writeToLog(logFile, "Compiler", "MSVC");
#else
    writeToLog(logFile, "Compiler", "?");
#endif
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
                {
                    disableRenderMethod(RenderVulkan);
                    writeSkiaRasterInfo();
                }
                break;
            }
            case RenderRaster:
                SAL_INFO("vcl.skia", "Using Skia raster mode");
                writeSkiaRasterInfo();
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

namespace
{
// Image cache, for saving results of complex operations such as drawTransformedBitmap().
struct ImageCacheItem
{
    OString key;
    sk_sp<SkImage> image;
    int size; // cost of the item
};
} //namespace

// LRU cache, last item is the least recently used. Hopefully there won't be that many items
// to require a hash/map. Using o3tl::lru_cache would be simpler, but it doesn't support
// calculating cost of each item.
static std::list<ImageCacheItem>* imageCache = nullptr;
static int imageCacheSize = 0; // sum of all ImageCacheItem.size

void addCachedImage(const OString& key, sk_sp<SkImage> image)
{
    static bool disabled = getenv("SAL_DISABLE_SKIA_CACHE") != nullptr;
    if (disabled)
        return;
    if (imageCache == nullptr)
        imageCache = new std::list<ImageCacheItem>;
    int size = image->width() * image->height()
               * SkColorTypeBytesPerPixel(image->imageInfo().colorType());
    imageCache->push_front({ key, image, size });
    imageCacheSize += size;
    SAL_INFO("vcl.skia.trace", "addcachedimage " << image << " :" << size << "/" << imageCacheSize);
    const int MAX_CACHE_SIZE = 4 * 1000 * 1000 * 4; // 4x 1000px 32bpp images, 16MiB
    while (imageCacheSize > MAX_CACHE_SIZE)
    {
        assert(!imageCache->empty());
        imageCacheSize -= imageCache->back().size;
        SAL_INFO("vcl.skia.trace",
                 "least used removal " << image << ":" << imageCache->back().size);
        imageCache->pop_back();
    }
}

sk_sp<SkImage> findCachedImage(const OString& key)
{
    if (imageCache != nullptr)
    {
        for (auto it = imageCache->begin(); it != imageCache->end(); ++it)
        {
            if (it->key == key)
            {
                sk_sp<SkImage> ret = it->image;
                SAL_INFO("vcl.skia.trace",
                         "findcachedimage " << key << " : " << it->image << " found");
                imageCache->splice(imageCache->begin(), *imageCache, it);
                return ret;
            }
        }
    }
    SAL_INFO("vcl.skia.trace", "findcachedimage " << key << " not found");
    return nullptr;
}

void removeCachedImage(sk_sp<SkImage> image)
{
    if (imageCache == nullptr)
        return;
    for (auto it = imageCache->begin(); it != imageCache->end();)
    {
        if (it->image == image)
        {
            imageCacheSize -= it->size;
            assert(imageCacheSize >= 0);
            it = imageCache->erase(it);
        }
        else
            ++it;
    }
}

void cleanup()
{
    delete sharedGrContext;
    sharedGrContext = nullptr;
    delete imageCache;
    imageCache = nullptr;
    imageCacheSize = 0;
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
