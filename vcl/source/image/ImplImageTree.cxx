/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include <sal/config.h>
#include <sal/log.hxx>

#include <deque>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <osl/file.hxx>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>

#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <implimagetree.hxx>

#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/IconThemeScanner.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pngwrite.hxx>

#include <BitmapLightenFilter.hxx>

using namespace css;

bool ImageRequestParameters::convertToDarkTheme()
{
    static bool bIconsForDarkTheme = !!getenv("VCL_ICONS_FOR_DARK_THEME");

    bool bConvertToDarkTheme = false;
    if (!(meFlags & ImageLoadFlags::IgnoreDarkTheme))
        bConvertToDarkTheme = bIconsForDarkTheme;

    return bConvertToDarkTheme;
}

sal_Int32 ImageRequestParameters::scalePercentage()
{
    sal_Int32 aScalePercentage = 100;
    if (!(meFlags & ImageLoadFlags::IgnoreScalingFactor))
        aScalePercentage = Application::GetDefaultDevice()->GetDPIScalePercentage();
    else if (mnScalePercentage > 0)
        aScalePercentage = mnScalePercentage;
    return aScalePercentage;
}

namespace
{

OUString convertLcTo32Path(OUString const & rPath)
{
    OUString aResult;
    if (rPath.lastIndexOf('/') != -1)
    {
        sal_Int32 nCopyFrom = rPath.lastIndexOf('/') + 1;
        OUString sFile = rPath.copy(nCopyFrom);
        OUString sDir = rPath.copy(0, rPath.lastIndexOf('/'));
        if (!sFile.isEmpty() && sFile.startsWith("lc_"))
        {
            aResult = sDir + "/32/" + sFile.copy(3);
        }
    }
    return aResult;
}

OUString createPath(OUString const & name, sal_Int32 pos, OUString const & locale)
{
    return name.copy(0, pos + 1) + locale + name.copy(pos);
}

OUString getIconCacheUrl(OUString const & sVariant, ImageRequestParameters const & rParameters)
{
    OUString sUrl("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    sUrl += rParameters.msStyle + "/" + sVariant + "/" + rParameters.msName;
    rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}

OUString createIconCacheUrl(OUString const & sVariant, ImageRequestParameters const & rParameters)
{
    OUString sUrl(getIconCacheUrl(sVariant, rParameters));
    OUString sDir = sUrl.copy(0, sUrl.lastIndexOf('/'));
    osl::Directory::createPath(sDir);
    return sUrl;
}

bool urlExists(OUString const & sUrl)
{
    osl::File aFile(sUrl);
    osl::FileBase::RC eRC = aFile.open(osl_File_OpenFlag_Read);
    return osl::FileBase::E_None == eRC;
}

OUString getNameNoExtension(OUString const & sName)
{
    sal_Int32 nDotPosition = sName.lastIndexOf('.');
    return sName.copy(0, nDotPosition);
}

std::shared_ptr<SvMemoryStream> wrapStream(uno::Reference<io::XInputStream> const & rInputStream)
{
    // This could use SvInputStream instead if that did not have a broken
    // SeekPos implementation for an XInputStream that is not also XSeekable
    // (cf. "@@@" at tags/DEV300_m37/svtools/source/misc1/strmadpt.cxx@264807
    // l. 593):
    OSL_ASSERT(rInputStream.is());
    std::shared_ptr<SvMemoryStream> aMemoryStream(std::make_shared<SvMemoryStream>());
    for (;;)
    {
        const sal_Int32 nSize(2048);
        uno::Sequence<sal_Int8> aData(nSize);
        sal_Int32 nRead = rInputStream->readBytes(aData, nSize);
        aMemoryStream->WriteBytes(aData.getConstArray(), nRead);
        if (nRead < nSize)
            break;
    }
    aMemoryStream->Seek(0);
    return aMemoryStream;
}

void loadImageFromStream(std::shared_ptr<SvStream> const & xStream, OUString const & rPath, ImageRequestParameters& rParameters)
{
    bool bConvertToDarkTheme = rParameters.convertToDarkTheme();
    sal_Int32 aScalePercentage = rParameters.scalePercentage();

    if (rPath.endsWith(".png"))
    {
        vcl::PngImageReader aPNGReader(*xStream);
        aPNGReader.read(rParameters.mrBitmap);
    }
    else if (rPath.endsWith(".svg"))
    {
        rParameters.mbWriteImageToCache = true; // We always want to cache a SVG image
        vcl::bitmap::loadFromSvg(*xStream, rPath, rParameters.mrBitmap, aScalePercentage / 100.0);

        if (bConvertToDarkTheme)
            BitmapFilter::Filter(rParameters.mrBitmap, BitmapLightenFilter());

        return;
    }
    else
    {
        ReadDIBBitmapEx(rParameters.mrBitmap, *xStream);
    }

    if (bConvertToDarkTheme)
    {
        rParameters.mbWriteImageToCache = true; // Cache the dark variant
        BitmapFilter::Filter(rParameters.mrBitmap, BitmapLightenFilter());
    }

    if (aScalePercentage > 100)
    {
        rParameters.mbWriteImageToCache = true; // Cache the scaled variant
        double aScaleFactor(aScalePercentage / 100.0);
        // when scaling use the full 24bit RGB values
        rParameters.mrBitmap.Convert(BmpConversion::N24Bit);
        rParameters.mrBitmap.Scale(aScaleFactor, aScaleFactor, BmpScaleFlag::Fast);
    }
}

} // end anonymous namespace

ImplImageTree::ImplImageTree()
{
}

ImplImageTree::~ImplImageTree()
{
}

std::vector<OUString> ImplImageTree::getPaths(OUString const & name, LanguageTag const & rLanguageTag)
{
    std::vector<OUString> sPaths;

    sal_Int32 pos = name.lastIndexOf('/');
    if (pos != -1)
    {
        for (OUString& rFallback : rLanguageTag.getFallbackStrings(true))
        {
            OUString aFallbackName = getNameNoExtension(getRealImageName(createPath(name, pos, rFallback)));
            sPaths.emplace_back(aFallbackName + ".png");
            sPaths.emplace_back(aFallbackName + ".svg");
        }
    }

    OUString aRealName = getNameNoExtension(getRealImageName(name));
    sPaths.emplace_back(aRealName + ".png");
    sPaths.emplace_back(aRealName + ".svg");

    return sPaths;
}

OUString ImplImageTree::getImageUrl(OUString const & rName, OUString const & rStyle, OUString const & rLang)
{
    OUString aStyle(rStyle);

    while (!aStyle.isEmpty())
    {
        try
        {
            setStyle(aStyle);

            if (checkPathAccess())
            {
                IconSet& rIconSet = getCurrentIconSet();
                const uno::Reference<container::XNameAccess> & rNameAccess = rIconSet.maNameAccess;

                LanguageTag aLanguageTag(rLang);

                for (OUString& rPath: getPaths(rName, aLanguageTag))
                {
                    if (rNameAccess->hasByName(rPath))
                    {
                        return "vnd.sun.star.zip://"
                            + rtl::Uri::encode(rIconSet.maURL, rtl_UriCharClassRegName,
                                               rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8)
                            + "/" + rPath;
                    }
                }
            }
        }
        catch (const uno::Exception & e)
        {
            SAL_INFO("vcl", e);
        }

        aStyle = fallbackStyle(aStyle);
    }
    return OUString();
}

std::shared_ptr<SvMemoryStream> ImplImageTree::getImageStream(OUString const & rName, OUString const & rStyle, OUString const & rLang)
{
    OUString aStyle(rStyle);

    while (!aStyle.isEmpty())
    {
        try
        {
            setStyle(aStyle);

            if (checkPathAccess())
            {
                IconSet& rIconSet = getCurrentIconSet();
                const uno::Reference<container::XNameAccess>& rNameAccess = rIconSet.maNameAccess;

                LanguageTag aLanguageTag(rLang);

                for (OUString& rPath: getPaths(rName, aLanguageTag))
                {
                    if (rNameAccess->hasByName(rPath))
                    {
                        uno::Reference<io::XInputStream> aStream;
                        bool ok = rNameAccess->getByName(rPath) >>= aStream;
                        assert(ok);
                        (void)ok; // prevent unused warning in release build
                        return wrapStream(aStream);
                    }
                }
            }
        }
        catch (const uno::Exception & e)
        {
            SAL_INFO("vcl", e);
        }

        aStyle = fallbackStyle(aStyle);
    }
    return std::shared_ptr<SvMemoryStream>();
}

OUString ImplImageTree::fallbackStyle(const OUString& rsStyle)
{
    OUString sResult;

    if (rsStyle == "colibre" || rsStyle == "helpimg")
        sResult = "";
    else if (rsStyle == "sifr" || rsStyle == "breeze_dark")
        sResult = "breeze";
    else if (rsStyle == "sifr_dark" )
        sResult = "breeze_dark";
    else
        sResult = "colibre";

    return sResult;
}

bool ImplImageTree::loadImage(OUString const & rName, OUString const & rStyle, BitmapEx & rBitmap, bool localized,
                              const ImageLoadFlags eFlags, sal_Int32 nScalePercentage)
{
    OUString aCurrentStyle(rStyle);
    while (!aCurrentStyle.isEmpty())
    {
        try
        {
            ImageRequestParameters aParameters(rName, aCurrentStyle, rBitmap, localized, eFlags, nScalePercentage);
            if (doLoadImage(aParameters))
                return true;
        }
        catch (uno::RuntimeException &)
        {}

        aCurrentStyle = fallbackStyle(aCurrentStyle);
    }
    return false;
}

namespace
{

OUString createVariant(ImageRequestParameters& rParameters)
{
    bool bConvertToDarkTheme = rParameters.convertToDarkTheme();
    sal_Int32 aScalePercentage = rParameters.scalePercentage();

    OUString aVariant = OUString::number(aScalePercentage);

    if (bConvertToDarkTheme)
        aVariant += "-dark";

    return aVariant;
}

bool loadDiskCachedVersion(OUString const & sVariant, ImageRequestParameters& rParameters)
{
    OUString sUrl(getIconCacheUrl(sVariant, rParameters));
    if (!urlExists(sUrl))
        return false;
    SvFileStream aFileStream(sUrl, StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    aPNGReader.read(rParameters.mrBitmap);
    return true;
}

void cacheBitmapToDisk(OUString const & sVariant, ImageRequestParameters const & rParameters)
{
    OUString sUrl(createIconCacheUrl(sVariant, rParameters));
    vcl::PNGWriter aWriter(rParameters.mrBitmap);
    try
    {
        SvFileStream aStream(sUrl, StreamMode::WRITE);
        aWriter.Write(aStream);
        aStream.Close();
    }
    catch (...)
    {}
}

} // end anonymous namespace

bool ImplImageTree::doLoadImage(ImageRequestParameters& rParameters)
{
    setStyle(rParameters.msStyle);

    if (iconCacheLookup(rParameters))
        return true;

    OUString aVariant = createVariant(rParameters);
    if (loadDiskCachedVersion(aVariant, rParameters))
        return true;

    if (!rParameters.mrBitmap.IsEmpty())
        rParameters.mrBitmap.SetEmpty();

    LanguageTag aLanguageTag = Application::GetSettings().GetUILanguageTag();

    std::vector<OUString> aPaths = getPaths(rParameters.msName, aLanguageTag);

    bool bFound = false;

    try
    {
        bFound = findImage(aPaths, rParameters);
    }
    catch (uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception& e)
    {
        SAL_INFO("vcl", "ImplImageTree::doLoadImage exception: " << e);
    }

    if (bFound)
    {
        if (rParameters.mbWriteImageToCache)
        {
            cacheBitmapToDisk(aVariant, rParameters);
        }
        getIconCache(rParameters)[rParameters.msName] = std::make_pair(rParameters.mbLocalized, rParameters.mrBitmap);
    }

    return bFound;
}

void ImplImageTree::shutdown()
{
    maCurrentStyle.clear();
    maIconSets.clear();
}

void ImplImageTree::setStyle(OUString const & style)
{
    assert(!style.isEmpty());
    if (style != maCurrentStyle)
    {
        maCurrentStyle = style;
        createStyle();
    }
}

/**
 * The vcldemo app doesn't set up all the config stuff that the main app does, so we need another
 * way of finding the cursor images.
 */
static bool isVclDemo()
{
    static const bool bVclDemoOverride = std::getenv("LIBO_VCL_DEMO") != nullptr;
    return bVclDemoOverride;
}

void ImplImageTree::createStyle()
{
    if (maIconSets.find(maCurrentStyle) != maIconSets.end())
        return;

    OUString sThemeUrl;

    if (isVclDemo())
    {
        static OUString s_workingDir;
        if (!s_workingDir.getLength())
            osl_getProcessWorkingDir( &s_workingDir.pData );

        if (maCurrentStyle == "default")
            sThemeUrl = s_workingDir + "/icon-themes/colibre-svg";
        else
            sThemeUrl = s_workingDir + "/icon-themes/" + maCurrentStyle;
    }
    else if (maCurrentStyle != "default")
    {
        OUString paths = vcl::IconThemeScanner::GetStandardIconThemePath();
        std::deque<OUString> aPaths;
        sal_Int32 nIndex = 0;
        do
        {
            aPaths.push_front(paths.getToken(0, ';', nIndex));
        }
        while (nIndex >= 0);

        for (const auto& path : aPaths)
        {
            INetURLObject aUrl(path);
            OSL_ASSERT(!aUrl.HasError());

            bool ok = aUrl.Append("images_" + maCurrentStyle, INetURLObject::EncodeMechanism::All);
            OSL_ASSERT(ok);
            sThemeUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE) + ".zip";
            if (urlExists(sThemeUrl))
                break;
            sThemeUrl.clear();
        }

        if (sThemeUrl.isEmpty())
            return;
    }
    else
    {
        sThemeUrl += "images";
        if (!urlExists(sThemeUrl))
            return;
    }

    maIconSets[maCurrentStyle] = IconSet(sThemeUrl);

    loadImageLinks();
}

/// Find an icon cache for the right scale factor
ImplImageTree::IconCache &ImplImageTree::getIconCache(const ImageRequestParameters& rParameters)
{
    IconSet &rSet = getCurrentIconSet();
    auto it = rSet.maScaledIconCaches.find(rParameters.mnScalePercentage);
    if ( it != rSet.maScaledIconCaches.end() )
        return *it->second.get();
    rSet.maScaledIconCaches[rParameters.mnScalePercentage] = std::unique_ptr<IconCache>(new IconCache);
    return *rSet.maScaledIconCaches[rParameters.mnScalePercentage].get();
}

bool ImplImageTree::iconCacheLookup(ImageRequestParameters& rParameters)
{
    IconCache& rIconCache = getIconCache(rParameters);

    IconCache::iterator i(rIconCache.find(getRealImageName(rParameters.msName)));
    if (i != rIconCache.end() && i->second.first == rParameters.mbLocalized)
    {
        rParameters.mrBitmap = i->second.second;
        return true;
    }

    return false;
}

bool ImplImageTree::findImage(std::vector<OUString> const & rPaths, ImageRequestParameters& rParameters)
{
    if (!checkPathAccess())
        return false;

    uno::Reference<container::XNameAccess> const & rNameAccess = getCurrentIconSet().maNameAccess;

    for (OUString const & rPath : rPaths)
    {
        if (rNameAccess->hasByName(rPath))
        {
            uno::Reference<io::XInputStream> aStream;
            bool ok = rNameAccess->getByName(rPath) >>= aStream;
            assert(ok);
            (void)ok; // prevent unused warning in release build

            loadImageFromStream(wrapStream(aStream), rPath, rParameters);

            return true;
        }
    }
    return false;
}

void ImplImageTree::loadImageLinks()
{
    const OUString aLinkFilename("links.txt");

    if (!checkPathAccess())
        return;

    const uno::Reference<container::XNameAccess> &rNameAccess = getCurrentIconSet().maNameAccess;

    if (rNameAccess->hasByName(aLinkFilename))
    {
        uno::Reference<io::XInputStream> xStream;
        bool ok = rNameAccess->getByName(aLinkFilename) >>= xStream;
        assert(ok);
        (void)ok; // prevent unused warning in release build

        parseLinkFile(wrapStream(xStream));
        return;
    }
}

void ImplImageTree::parseLinkFile(std::shared_ptr<SvStream> const & xStream)
{
    OString aLine;
    OUString aLink, aOriginal;
    int nLineNo = 0;
    while (xStream->ReadLine(aLine))
    {
        ++nLineNo;
        if (aLine.isEmpty())
            continue;

        sal_Int32 nIndex = 0;
        aLink = OStringToOUString(aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8);
        aOriginal = OStringToOUString(aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8);

        // skip comments, or incomplete entries
        if (aLink.isEmpty() || aLink[0] == '#' || aOriginal.isEmpty())
        {
            if (aLink.isEmpty() || aOriginal.isEmpty())
                SAL_WARN("vcl", "ImplImageTree::parseLinkFile: icon links.txt parse error, incomplete link at line " << nLineNo);
            continue;
        }

        getCurrentIconSet().maLinkHash[aLink] = aOriginal;

        OUString aOriginal32 = convertLcTo32Path(aOriginal);
        OUString aLink32 = convertLcTo32Path(aLink);

        if (!aOriginal32.isEmpty() && !aLink32.isEmpty())
            getCurrentIconSet().maLinkHash[aLink32] = aOriginal32;
    }
}

OUString const & ImplImageTree::getRealImageName(OUString const & rIconName)
{
    IconLinkHash & rLinkHash = maIconSets[maCurrentStyle].maLinkHash;

    OUString sNameWithNoExtension = getNameNoExtension(rIconName);

    IconLinkHash::iterator it;

    // PNG is priority
    it = rLinkHash.find(sNameWithNoExtension + ".png");
    if (it != rLinkHash.end())
        return it->second;

    // also check SVG name
    it = rLinkHash.find(sNameWithNoExtension + ".svg");
    if (it != rLinkHash.end())
        return it->second;

    // neither was found so just return the original name
    return rIconName;
}

class FolderFileAccess : public ::cppu::WeakImplHelper<css::container::XNameAccess>
{
public:
    uno::Reference< uno::XComponentContext > mxContext;
    OUString maURL;
    FolderFileAccess(uno::Reference< uno::XComponentContext > const & context, OUString const & url)
        : mxContext(context), maURL(url) {}
    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override { return cppu::UnoType<io::XInputStream>::get(); }
    virtual sal_Bool SAL_CALL hasElements() override { return true; }
    // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        uno::Reference< io::XInputStream > xInputStream = ucb::SimpleFileAccess::create(mxContext)->openFileRead( maURL + "/" + aName );
        return css::uno::Any(xInputStream);
    }
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override
    {
        return {};
    }
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        osl::File aBaseFile(maURL + "/" + aName);
        return osl::File::E_None == aBaseFile.open(osl_File_OpenFlag_Read);
    }
};

bool ImplImageTree::checkPathAccess()
{
    IconSet& rIconSet = getCurrentIconSet();
    uno::Reference<container::XNameAccess> & rNameAccess = rIconSet.maNameAccess;
    if (rNameAccess.is())
        return true;

    try
    {
        if (isVclDemo())
            rNameAccess = new FolderFileAccess(comphelper::getProcessComponentContext(), rIconSet.maURL);
        else
            rNameAccess = packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), rIconSet.maURL);
    }
    catch (const uno::RuntimeException &)
    {
        throw;
    }
    catch (const uno::Exception & e)
    {
        SAL_INFO("vcl", "ImplImageTree::zip file location " << e << " for " << rIconSet.maURL);
        return false;
    }
    return rNameAccess.is();
}

uno::Reference<container::XNameAccess> const & ImplImageTree::getNameAccess()
{
    (void)checkPathAccess();
    return getCurrentIconSet().maNameAccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
