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
#include <o3tl/lru_map.hxx>

#include <SkCanvas.h>
#include <SkPaint.h>
#include <SkSurface.h>
#include <SkGraphics.h>
#include <GrDirectContext.h>
#include <SkRuntimeEffect.h>
#include <SkOpts_spi.h>
#include <skia_compiler.hxx>
#include <skia_opts.hxx>
#include <tools/sk_app/VulkanWindowContext.h>
#include <tools/sk_app/MetalWindowContext.h>
#include <fstream>

namespace SkiaHelper
{
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

uint32_t vendorId = 0;

#ifdef SK_VULKAN
static void writeToLog(SvStream& stream, const char* key, std::u16string_view value)
{
    writeToLog(stream, key, OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr());
}

static OUString getDenylistFile()
{
    OUString url("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER);
    rtl::Bootstrap::expandMacros(url);

    return url + "/skia/skia_denylist_vulkan.xml";
}

static uint32_t driverVersion = 0;

static OUString versionAsString(uint32_t version)
{
    return OUString::number(version >> 22) + "." + OUString::number((version >> 12) & 0x3ff) + "."
           + OUString::number(version & 0xfff);
}

static std::string_view vendorAsString(uint32_t vendor)
{
    return DriverBlocklist::GetVendorNameFromId(vendor);
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
#endif

#ifdef SK_METAL
static void writeSkiaMetalInfo()
{
    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE | StreamMode::TRUNC);
    writeToLog(logFile, "RenderMethod", "metal");
}
#endif

static void writeSkiaRasterInfo()
{
    SvFileStream logFile(getCacheFolder() + "/skia.log", StreamMode::WRITE | StreamMode::TRUNC);
    writeToLog(logFile, "RenderMethod", "raster");
    // Log compiler, Skia works best when compiled with Clang.
    writeToLog(logFile, "Compiler", skia_compiler_name());
}

#if defined(SK_VULKAN) || defined(SK_METAL)
static std::unique_ptr<sk_app::WindowContext> getTemporaryWindowContext();
#endif

static void checkDeviceDenylisted(bool blockDisable = false)
{
    static bool done = false;
    if (done)
        return;

    SkiaZone zone;

    bool useRaster = false;
    switch (renderMethodToUse())
    {
        case RenderVulkan:
        {
#ifdef SK_VULKAN
            // First try if a GrDirectContext already exists.
            std::unique_ptr<sk_app::WindowContext> temporaryWindowContext;
            GrDirectContext* grDirectContext
                = sk_app::VulkanWindowContext::getSharedGrDirectContext();
            if (!grDirectContext)
            {
                // This function is called from isVclSkiaEnabled(), which
                // may be called when deciding which X11 visual to use,
                // and that visual is normally needed when creating
                // Skia's VulkanWindowContext, which is needed for the GrDirectContext.
                // Avoid the loop by creating a temporary WindowContext
                // that will use the default X11 visual (that shouldn't matter
                // for just finding out information about Vulkan) and destroying
                // the temporary context will clean up again.
                temporaryWindowContext = getTemporaryWindowContext();
                grDirectContext = sk_app::VulkanWindowContext::getSharedGrDirectContext();
            }
            bool denylisted = true; // assume the worst
            if (grDirectContext) // Vulkan was initialized properly
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
                useRaster = true;
            }
#else
            SAL_WARN("vcl.skia", "Vulkan support not built in");
            (void)blockDisable;
            useRaster = true;
#endif
            break;
        }
        case RenderMetal:
        {
#ifdef SK_METAL
            // First try if a GrDirectContext already exists.
            std::unique_ptr<sk_app::WindowContext> temporaryWindowContext;
            GrDirectContext* grDirectContext = sk_app::getMetalSharedGrDirectContext();
            if (!grDirectContext)
            {
                // Create a temporary window context just to get the GrDirectContext,
                // as an initial test of Metal functionality.
                temporaryWindowContext = getTemporaryWindowContext();
                grDirectContext = sk_app::getMetalSharedGrDirectContext();
            }
            if (grDirectContext) // Metal was initialized properly
            {
                // Try to assume Metal always works, given that Mac doesn't have such as wide range of HW vendors as PC.
                // If there turns out to be problems, handle it similarly to Vulkan.
                SAL_INFO("vcl.skia", "Using Skia Metal mode");
                writeSkiaMetalInfo();
            }
            else
            {
                SAL_INFO("vcl.skia", "Metal could not be initialized");
                disableRenderMethod(RenderMetal);
                useRaster = true;
            }
#else
            SAL_WARN("vcl.skia", "Metal support not built in");
            useRaster = true;
#endif
            break;
        }
        case RenderRaster:
            useRaster = true;
            break;
    }
    if (useRaster)
    {
        SAL_INFO("vcl.skia", "Using Skia raster mode");
        // software, never denylisted
        writeSkiaRasterInfo();
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
#ifdef MACOSX
        if (strcmp(env, "metal") == 0)
        {
            methodToUse = RenderMetal;
            return true;
        }
#else
        if (strcmp(env, "vulkan") == 0)
        {
            methodToUse = RenderVulkan;
            return true;
        }
#endif
        SAL_WARN("vcl.skia", "Unrecognized value of SAL_SKIA");
        abort();
    }
    methodToUse = RenderRaster;
    if (officecfg::Office::Common::VCL::ForceSkiaRaster::get())
        return true;
#ifdef SK_METAL
    methodToUse = RenderMetal;
#endif
#ifdef SK_VULKAN
    methodToUse = RenderVulkan;
#endif
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

// If needed, we'll allocate one extra window context so that we have a valid GrDirectContext
// from Vulkan/MetalWindowContext.
static std::unique_ptr<sk_app::WindowContext> sharedWindowContext;

static std::unique_ptr<sk_app::WindowContext> (*createGpuWindowContextFunction)(bool) = nullptr;
static void setCreateGpuWindowContext(std::unique_ptr<sk_app::WindowContext> (*function)(bool))
{
    createGpuWindowContextFunction = function;
}

GrDirectContext* getSharedGrDirectContext()
{
    SkiaZone zone;
    assert(renderMethodToUse() != RenderRaster);
    if (sharedWindowContext)
        return sharedWindowContext->directContext();
    // TODO mutex?
    // Set up the shared GrDirectContext from Skia's (patched) Vulkan/MetalWindowContext, if it's been
    // already set up.
    switch (renderMethodToUse())
    {
        case RenderVulkan:
#ifdef SK_VULKAN
            if (GrDirectContext* context = sk_app::VulkanWindowContext::getSharedGrDirectContext())
                return context;
#endif
            break;
        case RenderMetal:
#ifdef SK_METAL
            if (GrDirectContext* context = sk_app::getMetalSharedGrDirectContext())
                return context;
#endif
            break;
        case RenderRaster:
            abort();
    }
    static bool done = false;
    if (done)
        return nullptr;
    done = true;
    if (createGpuWindowContextFunction == nullptr)
        return nullptr; // not initialized properly (e.g. used from a VCL backend with no Skia support)
    sharedWindowContext = createGpuWindowContextFunction(false);
    GrDirectContext* grDirectContext
        = sharedWindowContext ? sharedWindowContext->directContext() : nullptr;
    if (grDirectContext)
        return grDirectContext;
    SAL_WARN_IF(renderMethodToUse() == RenderVulkan, "vcl.skia",
                "Cannot create Vulkan GPU context, falling back to Raster");
    SAL_WARN_IF(renderMethodToUse() == RenderMetal, "vcl.skia",
                "Cannot create Metal GPU context, falling back to Raster");
    disableRenderMethod(renderMethodToUse());
    return nullptr;
}

#if defined(SK_VULKAN) || defined(SK_METAL)
static std::unique_ptr<sk_app::WindowContext> getTemporaryWindowContext()
{
    if (createGpuWindowContextFunction == nullptr)
        return nullptr;
    return createGpuWindowContextFunction(true);
}
#endif

sk_sp<SkSurface> createSkSurface(int width, int height, SkColorType type, SkAlphaType alpha)
{
    SkiaZone zone;
    assert(type == kN32_SkColorType || type == kAlpha_8_SkColorType);
    sk_sp<SkSurface> surface;
    switch (renderMethodToUse())
    {
        case RenderVulkan:
        case RenderMetal:
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
                SAL_WARN_IF(renderMethodToUse() == RenderVulkan, "vcl.skia",
                            "Cannot create Vulkan GPU offscreen surface, falling back to Raster");
                SAL_WARN_IF(renderMethodToUse() == RenderMetal, "vcl.skia",
                            "Cannot create Metal GPU offscreen surface, falling back to Raster");
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
    switch (renderMethodToUse())
    {
        case RenderVulkan:
        case RenderMetal:
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
                    surface->getCanvas()->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(),
                                                    &paint);
                    return makeCheckedImageSnapshot(surface);
                }
                // Try to fall back in non-debug builds.
                SAL_WARN_IF(renderMethodToUse() == RenderVulkan, "vcl.skia",
                            "Cannot create Vulkan GPU offscreen surface, falling back to Raster");
                SAL_WARN_IF(renderMethodToUse() == RenderMetal, "vcl.skia",
                            "Cannot create Metal GPU offscreen surface, falling back to Raster");
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
// to require a hash/map. Using o3tl::lru_map would be simpler, but it doesn't support
// calculating cost of each item.
static std::list<ImageCacheItem> imageCache;
static tools::Long imageCacheSize = 0; // sum of all ImageCacheItem.size

void addCachedImage(const OString& key, sk_sp<SkImage> image)
{
    static bool disabled = getenv("SAL_DISABLE_SKIA_CACHE") != nullptr;
    if (disabled)
        return;
    tools::Long size = static_cast<tools::Long>(image->width()) * image->height()
                       * SkColorTypeBytesPerPixel(image->imageInfo().colorType());
    imageCache.push_front({ key, image, size });
    imageCacheSize += size;
    SAL_INFO("vcl.skia.trace", "addcachedimage " << image << " :" << size << "/" << imageCacheSize);
    const tools::Long maxSize = maxImageCacheSize();
    while (imageCacheSize > maxSize)
    {
        assert(!imageCache.empty());
        imageCacheSize -= imageCache.back().size;
        SAL_INFO("vcl.skia.trace",
                 "least used removal " << imageCache.back().image << ":" << imageCache.back().size);
        imageCache.pop_back();
    }
}

sk_sp<SkImage> findCachedImage(const OString& key)
{
    for (auto it = imageCache.begin(); it != imageCache.end(); ++it)
    {
        if (it->key == key)
        {
            sk_sp<SkImage> ret = it->image;
            SAL_INFO("vcl.skia.trace", "findcachedimage " << key << " : " << it->image << " found");
            imageCache.splice(imageCache.begin(), imageCache, it);
            return ret;
        }
    }
    SAL_INFO("vcl.skia.trace", "findcachedimage " << key << " not found");
    return nullptr;
}

void removeCachedImage(sk_sp<SkImage> image)
{
    for (auto it = imageCache.begin(); it != imageCache.end();)
    {
        if (it->image == image)
        {
            imageCacheSize -= it->size;
            assert(imageCacheSize >= 0);
            it = imageCache.erase(it);
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

static o3tl::lru_map<uint32_t, uint32_t> checksumCache(256);

static uint32_t computeSkPixmapChecksum(const SkPixmap& pixmap)
{
    // Use uint32_t because that's what SkOpts::hash_fn() returns.
    static_assert(std::is_same_v<uint32_t, decltype(SkOpts::hash_fn(nullptr, 0, 0))>);
    const size_t dataRowBytes = pixmap.width() << pixmap.shiftPerPixel();
    if (dataRowBytes == pixmap.rowBytes())
        return SkOpts::hash_fn(pixmap.addr(), pixmap.height() * dataRowBytes, 0);
    uint32_t sum = 0;
    for (int row = 0; row < pixmap.height(); ++row)
        sum = SkOpts::hash_fn(pixmap.addr(0, row), dataRowBytes, sum);
    return sum;
}

uint32_t getSkImageChecksum(sk_sp<SkImage> image)
{
    // Cache the checksums based on the uniqueID() (which should stay the same
    // for the same image), because it may be still somewhat expensive.
    uint32_t id = image->uniqueID();
    auto it = checksumCache.find(id);
    if (it != checksumCache.end())
        return it->second;
    SkPixmap pixmap;
    if (!image->peekPixels(&pixmap))
        abort(); // Fetching of GPU-based pixels is expensive, and shouldn't(?) be needed anyway.
    uint32_t checksum = computeSkPixmapChecksum(pixmap);
    checksumCache.insert({ id, checksum });
    return checksum;
}

static sk_sp<SkBlender> invertBlender;
static sk_sp<SkBlender> xorBlender;

// This does the invert operation, i.e. result = color(255-R,255-G,255-B,A).
void setBlenderInvert(SkPaint* paint)
{
    if (!invertBlender)
    {
        // Note that the colors are premultiplied, so '1 - dst.r' must be
        // written as 'dst.a - dst.r', since premultiplied R is in the range (0-A).
        const char* const diff = R"(
            vec4 main( vec4 src, vec4 dst )
            {
                return vec4( dst.a - dst.r, dst.a - dst.g, dst.a - dst.b, dst.a );
            }
        )";
        auto effect = SkRuntimeEffect::MakeForBlender(SkString(diff));
        if (!effect.effect)
        {
            SAL_WARN("vcl.skia",
                     "SKRuntimeEffect::MakeForBlender failed: " << effect.errorText.c_str());
            abort();
        }
        invertBlender = effect.effect->makeBlender(nullptr);
    }
    paint->setBlender(invertBlender);
}

// This does the xor operation, i.e. bitwise xor of RGB values of both colors.
void setBlenderXor(SkPaint* paint)
{
    if (!xorBlender)
    {
        // Note that the colors are premultiplied, converting to 0-255 range
        // must also unpremultiply.
        const char* const diff = R"(
            vec4 main( vec4 src, vec4 dst )
            {
                return vec4(
                    float(int(src.r * src.a * 255.0) ^ int(dst.r * dst.a * 255.0)) / 255.0 / dst.a,
                    float(int(src.g * src.a * 255.0) ^ int(dst.g * dst.a * 255.0)) / 255.0 / dst.a,
                    float(int(src.b * src.a * 255.0) ^ int(dst.b * dst.a * 255.0)) / 255.0 / dst.a,
                    dst.a );
            }
        )";
        SkRuntimeEffect::Options opts;
        // Skia does not allow binary operators in the default ES2Strict mode, but that's only
        // because of OpenGL support. We don't use OpenGL, and it's safe for all modes that we do use.
        // https://groups.google.com/g/skia-discuss/c/EPLuQbg64Kc/m/2uDXFIGhAwAJ
        opts.enforceES2Restrictions = false;
        auto effect = SkRuntimeEffect::MakeForBlender(SkString(diff), opts);
        if (!effect.effect)
        {
            SAL_WARN("vcl.skia",
                     "SKRuntimeEffect::MakeForBlender failed: " << effect.errorText.c_str());
            abort();
        }
        xorBlender = effect.effect->makeBlender(nullptr);
    }
    paint->setBlender(xorBlender);
}

void cleanup()
{
    sharedWindowContext.reset();
    imageCache.clear();
    imageCacheSize = 0;
    invertBlender.reset();
    xorBlender.reset();
}

static SkSurfaceProps commonSurfaceProps;
const SkSurfaceProps* surfaceProps() { return &commonSurfaceProps; }

void setPixelGeometry(SkPixelGeometry pixelGeometry)
{
    commonSurfaceProps = SkSurfaceProps(commonSurfaceProps.flags(), pixelGeometry);
}

// Skia should not be used from VCL backends that do not actually support it, as there will be setup missing.
// The code here (that is in the vcl lib) needs a function for creating Vulkan/Metal context that is
// usually available only in the backend libs.
void prepareSkia(std::unique_ptr<sk_app::WindowContext> (*createGpuWindowContext)(bool))
{
    setCreateGpuWindowContext(createGpuWindowContext);
    skiaSupportedByBackend = true;
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
    paint.setShader(
        bitmap.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions()));
    surface->getCanvas()->drawPaint(paint);
}
#endif

} // namespace

#endif // HAVE_FEATURE_SKIA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
