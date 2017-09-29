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

#include "sal/config.h"

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/packages/zip/ZipFileAccess.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/processfactory.hxx"
#include "osl/file.hxx"
#include "osl/diagnose.h"
#include "rtl/bootstrap.hxx"
#include "rtl/uri.hxx"

#include "tools/stream.hxx"
#include "tools/urlobj.hxx"
#include "implimagetree.hxx"

#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/pngread.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <vcl/BitmapTools.hxx>
#include <vcl/pngwrite.hxx>

#include "BitmapProcessor.hxx"

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

OUString getIconThemeFolderUrl()
{
    OUString sUrl("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/config/");
    rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}

OUString getIconCacheUrl(OUString const & sStyle, OUString const & sVariant, OUString const & sName)
{
    OUString sUrl("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    sUrl += sStyle + "/" + sVariant + "/" + sName;
    rtl::Bootstrap::expandMacros(sUrl);
    return sUrl;
}

OUString createIconCacheUrl(OUString const & sStyle, OUString const & sVariant, OUString const & sName)
{
    OUString sUrl(getIconCacheUrl(sStyle, sVariant, sName));
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

std::shared_ptr<SvStream> wrapStream(css::uno::Reference< css::io::XInputStream > const & stream)
{
    // This could use SvInputStream instead if that did not have a broken
    // SeekPos implementation for an XInputStream that is not also XSeekable
    // (cf. "@@@" at tags/DEV300_m37/svtools/source/misc1/strmadpt.cxx@264807
    // l. 593):
    OSL_ASSERT(stream.is());
    std::shared_ptr<SvStream> s(std::make_shared<SvMemoryStream>());
    for (;;)
    {
        sal_Int32 const size = 2048;
        css::uno::Sequence< sal_Int8 > data(size);
        sal_Int32 n = stream->readBytes(data, size);
        s->WriteBytes(data.getConstArray(), n);
        if (n < size)
            break;
    }
    s->Seek(0);
    return s;
}

void loadImageFromStream(std::shared_ptr<SvStream> const & xStream, OUString const & rPath, ImageRequestParameters& rParameters)
{
    bool bConvertToDarkTheme = rParameters.convertToDarkTheme();
    sal_Int32 aScalePercentage = rParameters.scalePercentage();

    if (rPath.endsWith(".png"))
    {
        vcl::PNGReader aPNGReader(*xStream);
        aPNGReader.SetIgnoreGammaChunk(true);
        rParameters.mrBitmap = aPNGReader.Read();
    }
    else if (rPath.endsWith(".svg"))
    {
        vcl::bitmap::loadFromSvg(*xStream.get(), rPath, rParameters.mrBitmap, aScalePercentage / 100.0);
        if (bConvertToDarkTheme)
            rParameters.mrBitmap = BitmapProcessor::createLightImage(rParameters.mrBitmap);
        return;
    }
    else
    {
        ReadDIBBitmapEx(rParameters.mrBitmap, *xStream);
    }

    if (bConvertToDarkTheme)
        rParameters.mrBitmap = BitmapProcessor::createLightImage(rParameters.mrBitmap);

    if (aScalePercentage > 100)
    {
        double aScaleFactor(aScalePercentage / 100.0);
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
                const css::uno::Reference<css::container::XNameAccess>& rNameAccess = rIconSet.maNameAccess;

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
        catch (const css::uno::Exception & e)
        {
            SAL_INFO("vcl", e);
        }

        aStyle = fallbackStyle(aStyle);
    }
    return OUString();
}

OUString ImplImageTree::fallbackStyle(const OUString& rsStyle)
{
    OUString sResult;

    if (rsStyle == "galaxy")
        sResult = "";
    else if (rsStyle == "industrial" || rsStyle == "tango" || rsStyle == "breeze")
        sResult = "galaxy";
    else if (rsStyle == "sifr" || rsStyle == "breeze_dark")
        sResult = "breeze";
    else if (rsStyle == "sifr_dark" )
        sResult = "breeze_dark";
    else if (rsStyle == "helpimg")
        sResult = "";
    else
        sResult = "tango";

    return sResult;
}

bool ImplImageTree::loadImage(OUString const & name, OUString const & style, BitmapEx & rBitmap, bool localized, const ImageLoadFlags eFlags)
{
    OUString aStyle(style);
    while (!aStyle.isEmpty())
    {
        try
        {
            ImageRequestParameters aParameters(name, aStyle, rBitmap, localized, eFlags);
            if (doLoadImage(aParameters))
                return true;
        }
        catch (css::uno::RuntimeException &)
        {}

        aStyle = fallbackStyle(aStyle);
    }
    return false;
}

OUString createVariant(ImageRequestParameters& rParameters)
{
    bool bConvertToDarkTheme = rParameters.convertToDarkTheme();
    sal_Int32 aScalePercentage = rParameters.scalePercentage();

    OUString aVariant;
    if (aScalePercentage == 100 && !bConvertToDarkTheme)
        return aVariant;

    aVariant = OUString::number(aScalePercentage);

    if (bConvertToDarkTheme)
        aVariant += "-dark";

    return aVariant;
}

bool loadDiskCachedVersion(OUString const & sVariant, ImageRequestParameters& rParameters)
{
    OUString sUrl(getIconCacheUrl(rParameters.msStyle, sVariant, rParameters.msName));
    if (!urlExists(sUrl))
        return false;
    SvFileStream aFileStream(sUrl, StreamMode::READ);
    vcl::PNGReader aPNGReader(aFileStream);
    aPNGReader.SetIgnoreGammaChunk( true );
    rParameters.mrBitmap = aPNGReader.Read();
    return true;
}

void cacheBitmapToDisk(OUString const & sVariant, ImageRequestParameters const & rParameters)
{
    OUString sUrl(createIconCacheUrl(rParameters.msStyle, sVariant, rParameters.msName));
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

bool ImplImageTree::doLoadImage(ImageRequestParameters& rParameters)
{
    setStyle(rParameters.msStyle);

    if (iconCacheLookup(rParameters))
        return true;

    if (!rParameters.mrBitmap.IsEmpty())
        rParameters.mrBitmap.SetEmpty();

    LanguageTag aLanguageTag = Application::GetSettings().GetUILanguageTag();

    std::vector<OUString> paths = getPaths(rParameters.msName, aLanguageTag);

    bool bFound = false;

    try
    {
        bFound = findImage(paths, rParameters);
    }
    catch (css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception& e)
    {
        SAL_INFO("vcl", "ImplImageTree::doLoadImage " << e);
    }

    if (bFound)
    {
        OUString aVariant = createVariant(rParameters);
        if (!aVariant.isEmpty())
            cacheBitmapToDisk(aVariant, rParameters);
        getCurrentIconSet().maIconCache[rParameters.msName] = std::make_pair(rParameters.mbLocalized, rParameters.mrBitmap);
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

void ImplImageTree::createStyle()
{
    if (maIconSets.find(maCurrentStyle) != maIconSets.end())
        return;

    OUString sThemeUrl;

    if (maCurrentStyle != "default")
    {
        INetURLObject aUrl(getIconThemeFolderUrl());
        OSL_ASSERT(!aUrl.HasError());

        bool ok = aUrl.Append("images_" + maCurrentStyle, INetURLObject::EncodeMechanism::All);
        OSL_ASSERT(ok);
        sThemeUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE) + ".zip";

    }
    else
        sThemeUrl += "images";

    if (!urlExists(sThemeUrl))
        return;

    maIconSets[maCurrentStyle] = IconSet(sThemeUrl);

    loadImageLinks();
}

bool ImplImageTree::iconCacheLookup(ImageRequestParameters& rParameters)
{
    IconCache& rIconCache = getCurrentIconSet().maIconCache;

    IconCache::iterator i(rIconCache.find(getRealImageName(rParameters.msName)));
    if (i != rIconCache.end() && i->second.first == rParameters.mbLocalized)
    {
        rParameters.mrBitmap = i->second.second;
        return true;
    }

    OUString aVariant = createVariant(rParameters);
    return !aVariant.isEmpty() && loadDiskCachedVersion(aVariant, rParameters);
}

bool ImplImageTree::findImage(std::vector<OUString> const & paths, ImageRequestParameters& rParameters)
{
    if (!checkPathAccess())
        return false;

    const css::uno::Reference<css::container::XNameAccess>& rNameAccess = getCurrentIconSet().maNameAccess;

    for (const OUString& rPath : paths)
    {
        if (rNameAccess->hasByName(rPath))
        {
            css::uno::Reference<css::io::XInputStream> aStream;
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

    const css::uno::Reference<css::container::XNameAccess> &rNameAccess = getCurrentIconSet().maNameAccess;

    if (rNameAccess->hasByName(aLinkFilename))
    {
        css::uno::Reference< css::io::XInputStream > s;
        bool ok = rNameAccess->getByName(aLinkFilename) >>= s;
        assert(ok);
        (void)ok; // prevent unused warning in release build

        parseLinkFile( wrapStream(s) );
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
        if ( aLine.isEmpty() )
            continue;

        sal_Int32 nIndex = 0;
        aLink = OStringToOUString( aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8 );
        aOriginal = OStringToOUString( aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8 );

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

OUString const & ImplImageTree::getRealImageName(OUString const & name)
{
    IconLinkHash &rLinkHash = maIconSets[maCurrentStyle].maLinkHash;

    IconLinkHash::iterator it(rLinkHash.find(name));
    if (it == rLinkHash.end())
        return name;

    return it->second;
}

bool ImplImageTree::checkPathAccess()
{
    IconSet& rIconSet = getCurrentIconSet();
    css::uno::Reference<css::container::XNameAccess> &rNameAccess = rIconSet.maNameAccess;
    if (rNameAccess.is())
        return true;

    try
    {
        rNameAccess = css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), rIconSet.maURL);
    }
    catch (const css::uno::RuntimeException &) {
        throw;
    }
    catch (const css::uno::Exception & e) {
        SAL_INFO("vcl", "ImplImageTree::zip file location " << e << " for " << rIconSet.maURL);
        return false;
    }
    return rNameAccess.is();
}

css::uno::Reference<css::container::XNameAccess> ImplImageTree::getNameAccess()
{
    checkPathAccess();
    return getCurrentIconSet().maNameAccess;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
