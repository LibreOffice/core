/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>

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
#include <SkGraphics.h>
#include <GrDirectContext.h>
#include <skia_compiler.hxx>
#include <skia_opts.hxx>

#ifdef DBG_UTIL
#include <fstream>
#endif

namespace SkiaHelper
{
static OUString getDenylistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/skia/skia_denylist_vulkan.xml";
}

static uint32_t driverVersion = 0;
uint32_t vendorId = 0;

static OUString versionAsString(uint32_t version)
{
    return OUString::number(version >> 22) + "." + OUString::number((version >> 12) & 0x3ff) + "."
           + OUString::number(version & 0xfff);
}

static std::string_view vendorAsString(uint32_t vendor)
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

static void writeToLog(SvStream& stream, const char* key, std::u16string_view value)
{
    writeToLog(stream, key, OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr());
}

// Note that this function also logs system information about Vulkan.
static bool isVulkanDenylisted(const VkPhysicalDeviceProperties& props)
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

    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE | StreamMode::TRUNC);
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
    bool denylisted
        = DriverBlocklist::IsDeviceBlocked(getDenylistFile(), DriverBlocklist::VersionType::Vulkan,
                                           driverVersionString, vendorIdStr, deviceIdStr);
    writeToLog(logFile, "Denylisted", denylisted ? "yes" : "no");
    return denylisted;
}

static void writeSkiaRasterInfo()
{
    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE | StreamMode::TRUNC);
    writeToLog(logFile, "RenderMethod", "raster");
    // Log compiler, Skia works best when compiled with Clang.
    writeToLog(logFile, "Compiler", skia_compiler_name());
}

static sk_app::VulkanWindowContext::SharedGrDirectContext getTemporaryGrDirectContext();

static void checkDeviceDenylisted(bool blockDisable = false)
{
    static bool done = false;
    if (done)
        return;

    SkiaZone zone;

    switch (renderMethodToUse())
    {
        case RenderVulkan:
        {
            // First try if a GrDirectContext already exists.
            sk_app::VulkanWindowContext::SharedGrDirectContext grDirectContext
                = sk_app::VulkanWindowContext::getSharedGrDirectContext();
            if (!grDirectContext.getGrDirectContext())
            {
                // This function is called from isVclSkiaEnabled(), which
                // may be called when deciding which X11 visual to use,
                // and that visual is normally needed when creating
                // Skia's VulkanWindowContext, which is needed for the GrDirectContext.
                // Avoid the loop by creating a temporary GrDirectContext
                // that will use the default X11 visual (that shouldn't matter
                // for just finding out information about Vulkan) and destroying
                // the temporary context will clean up again.
                grDirectContext = getTemporaryGrDirectContext();
            }
            bool denylisted = true; // assume the worst
            if (grDirectContext.getGrDirectContext()) // Vulkan was initialized properly
            {
                denylisted
                    = isVulkanDenylisted(sk_app::VulkanWindowContext::getPhysDeviceProperties());
                SAL_INFO("vcl.skia", "Vulkan denylisted: " << denylisted);
            }
            else
                SAL_INFO("vcl.skia", "Vulkan could not be initialized");
            if (denylisted && !blockDisable)
            {
                disableRenderMethod(RenderVulkan);
                writeSkiaRasterInfo();
            }
            break;
        }
        case RenderRaster:
            SAL_INFO("vcl.skia", "Using Skia raster mode");
            writeSkiaRasterInfo();
            return; // software, never denylisted
    }
    done = true;
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
     *  * SAL_FORCESKIA forces Skia if disabled by UI options or denylisted
     *  * SAL_DISABLESKIA avoids the use of Skia regardless of any option
     */

    bSet = true;
    bForceSkia = !!getenv("SAL_FORCESKIA") || officecfg::Office::Common::VCL::ForceSkia::get();

    bool bRet = false;
    bool bSupportsVCLSkia = supportsVCLSkia();
    if (bForceSkia && bSupportsVCLSkia)
    {
        bRet = true;
        SkGraphics::Init();
        SkLoOpts::Init();
        // don't actually block if denylisted, but log it if enabled, and also get the vendor id
        checkDeviceDenylisted(true);
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
        {
            SkGraphics::Init();
            SkLoOpts::Init();
            checkDeviceDenylisted(); // switch to raster if driver is denylisted
        }

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

static sk_app::VulkanWindowContext::SharedGrDirectContext* sharedGrDirectContext;

static std::unique_ptr<sk_app::WindowContext> (*createVulkanWindowContextFunction)(bool) = nullptr;
static void setCreateVulkanWindowContext(std::unique_ptr<sk_app::WindowContext> (*function)(bool))
{
    createVulkanWindowContextFunction = function;
}

GrDirectContext* getSharedGrDirectContext()
{
    SkiaZone zone;
    assert(renderMethodToUse() == RenderVulkan);
    if (sharedGrDirectContext)
        return sharedGrDirectContext->getGrDirectContext();
    // TODO mutex?
    // Set up the shared GrDirectContext from Skia's (patched) VulkanWindowContext, if it's been
    // already set up.
    sk_app::VulkanWindowContext::SharedGrDirectContext context
        = sk_app::VulkanWindowContext::getSharedGrDirectContext();
    GrDirectContext* grDirectContext = context.getGrDirectContext();
    if (grDirectContext)
    {
        sharedGrDirectContext = new sk_app::VulkanWindowContext::SharedGrDirectContext(context);
        return grDirectContext;
    }
    static bool done = false;
    if (done)
        return nullptr;
    done = true;
    if (createVulkanWindowContextFunction == nullptr)
        return nullptr; // not initialized properly (e.g. used from a VCL backend with no Skia support)
    std::unique_ptr<sk_app::WindowContext> tmpContext = createVulkanWindowContextFunction(false);
    // Set up using the shared context created by the call above, if successful.
    context = sk_app::VulkanWindowContext::getSharedGrDirectContext();
    grDirectContext = context.getGrDirectContext();
    if (grDirectContext)
    {
        sharedGrDirectContext = new sk_app::VulkanWindowContext::SharedGrDirectContext(context);
        return grDirectContext;
    }
    disableRenderMethod(RenderVulkan);
    return nullptr;
}

static sk_app::VulkanWindowContext::SharedGrDirectContext getTemporaryGrDirectContext()
{
    if (createVulkanWindowContextFunction == nullptr)
        return sk_app::VulkanWindowContext::SharedGrDirectContext();
    std::unique_ptr<sk_app::WindowContext> tmpContext = createVulkanWindowContextFunction(true);
    // Set up using the shared context created by the call above, if successful.
    return sk_app::VulkanWindowContext::getSharedGrDirectContext();
}

sk_sp<SkSurface> createSkSurface(int width, int height, SkColorType type, SkAlphaType alpha)
{
    SkiaZone zone;
    assert(type == kN32_SkColorType || type == kAlpha_8_SkColorType);
    sk_sp<SkSurface> surface;
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            if (GrDirectContext* grDirectContext = getSharedGrDirectContext())
            {
                surface = SkSurface::MakeRenderTarget(grDirectContext, SkBudgeted::kNo,
                                                      SkImageInfo::Make(width, height, type, alpha),
                                                      0, surfaceProps());
                if (surface)
                {
#ifdef DBG_UTIL
                    prefillSurface(surface);
#endif
                    return surface;
                }
                SAL_WARN("vcl.skia",
                         "cannot create Vulkan GPU offscreen surface, falling back to Raster");
            }
            break;
        }
        default:
            break;
    }
    // Create raster surface as a fallback.
    surface = SkSurface::MakeRaster(SkImageInfo::Make(width, height, type, alpha), surfaceProps());
    assert(surface);
    if (surface)
    {
#ifdef DBG_UTIL
        prefillSurface(surface);
#endif
        return surface;
    }
    // In non-debug builds we could return SkSurface::MakeNull() and try to cope with the situation,
    // but that can lead to unnoticed data loss, so better fail clearly.
    abort();
}

sk_sp<SkImage> createSkImage(const SkBitmap& bitmap)
{
    SkiaZone zone;
    assert(bitmap.colorType() == kN32_SkColorType || bitmap.colorType() == kAlpha_8_SkColorType);
    switch (SkiaHelper::renderMethodToUse())
    {
        case SkiaHelper::RenderVulkan:
        {
            if (GrDirectContext* grDirectContext = getSharedGrDirectContext())
            {
                sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(
                    grDirectContext, SkBudgeted::kNo,
                    bitmap.info().makeAlphaType(kPremul_SkAlphaType), 0, surfaceProps());
                if (surface)
                {
                    SkPaint paint;
                    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
                    surface->getCanvas()->drawBitmap(bitmap, 0, 0, &paint);
                    return makeCheckedImageSnapshot(surface);
                }
                // Try to fall back in non-debug builds.
                SAL_WARN("vcl.skia",
                         "cannot create Vulkan GPU offscreen surface, falling back to Raster");
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

sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface)
{
    sk_sp<SkImage> ret = surface->makeImageSnapshot();
    assert(ret);
    if (ret)
        return ret;
    abort();
}

sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface, const SkIRect& bounds)
{
    sk_sp<SkImage> ret = surface->makeImageSnapshot(bounds);
    assert(ret);
    if (ret)
        return ret;
    abort();
}

namespace
{
// Image cache, for saving results of complex operations such as drawTransformedBitmap().
struct ImageCacheItem
{
    OString key;
    sk_sp<SkImage> image;
    tools::Long size; // cost of the item
};
} //namespace

// LRU cache, last item is the least recently used. Hopefully there won't be that many items
// to require a hash/map. Using o3tl::lru_cache would be simpler, but it doesn't support
// calculating cost of each item.
static std::list<ImageCacheItem>* imageCache = nullptr;
static tools::Long imageCacheSize = 0; // sum of all ImageCacheItem.size

void addCachedImage(const OString& key, sk_sp<SkImage> image)
{
    static bool disabled = getenv("SAL_DISABLE_SKIA_CACHE") != nullptr;
    if (disabled)
        return;
    if (imageCache == nullptr)
        imageCache = new std::list<ImageCacheItem>;
    tools::Long size = static_cast<tools::Long>(image->width()) * image->height()
                       * SkColorTypeBytesPerPixel(image->imageInfo().colorType());
    imageCache->push_front({ key, image, size });
    imageCacheSize += size;
    SAL_INFO("vcl.skia.trace", "addcachedimage " << image << " :" << size << "/" << imageCacheSize);
    const tools::Long maxSize = maxImageCacheSize();
    while (imageCacheSize > maxSize)
    {
        assert(!imageCache->empty());
        imageCacheSize -= imageCache->back().size;
        SAL_INFO("vcl.skia.trace", "least used removal " << imageCache->back().image << ":"
                                                         << imageCache->back().size);
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

tools::Long maxImageCacheSize()
{
    // Defaults to 4x 2000px 32bpp images, 64MiB.
    return officecfg::Office::Common::Cache::Skia::ImageCacheSize::get();
}

void cleanup()
{
    delete sharedGrDirectContext;
    sharedGrDirectContext = nullptr;
    delete imageCache;
    imageCache = nullptr;
    imageCacheSize = 0;
}

static SkSurfaceProps commonSurfaceProps;
const SkSurfaceProps* surfaceProps() { return &commonSurfaceProps; }

void setPixelGeometry(SkPixelGeometry pixelGeometry)
{
    commonSurfaceProps = SkSurfaceProps(commonSurfaceProps.flags(), pixelGeometry);
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
void prefillSurface(const sk_sp<SkSurface>& surface)
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
    dump(makeCheckedImageSnapshot(surface), file);
}

void dump(const sk_sp<SkImage>& image, const char* file)
{
    sk_sp<SkData> data = image->encodeToData(SkEncodedImageFormat::kPNG, 1);
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}

#endif

} // namespace

#endif // HAVE_FEATURE_SKIA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
