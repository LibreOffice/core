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
#include <vcl/bitmapex.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/implimagetree.hxx>
#include <vcl/pngread.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcldemo-debug.hxx>

#include <vcl/BitmapProcessor.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/pngwrite.hxx>

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
    if (osl::FileBase::E_None == eRC)
        return true;
    return false;
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

void loadImageFromStream(std::shared_ptr<SvStream> const & xStream, OUString const & rPath, BitmapEx & rBitmap, const ImageLoadFlags eFlags)
{
    static bool bIconsForDarkTheme = !!getenv("VCL_ICONS_FOR_DARK_THEME");

    bool bConvertToDarkTheme = bIconsForDarkTheme;
    if (eFlags & ImageLoadFlags::IgnoreDarkTheme)
        bConvertToDarkTheme = false;

    float aScaleFactor = Application::GetDefaultDevice()->GetDPIScaleFactor();
    if (eFlags & ImageLoadFlags::IgnoreScalingFactor)
        aScaleFactor = 1.0f;

    if (rPath.endsWith(".png"))
    {
        vcl::PNGReader aPNGReader(*xStream);
        aPNGReader.SetIgnoreGammaChunk( true );
        rBitmap = aPNGReader.Read();
    }
    else if (rPath.endsWith(".svg"))
    {
        vcl::bitmap::loadFromSvg(*xStream.get(), rPath, rBitmap, double(aScaleFactor));
        if (bConvertToDarkTheme)
            rBitmap = BitmapProcessor::createLightImage(rBitmap);
        return;
    }
    else
    {
        ReadDIBBitmapEx(rBitmap, *xStream);
    }

    if (bConvertToDarkTheme)
        rBitmap = BitmapProcessor::createLightImage(rBitmap);

    if (aScaleFactor > 1.0f)
        rBitmap.Scale(double(aScaleFactor), double(aScaleFactor), BmpScaleFlag::Fast);
}

}

ImplImageTree & ImplImageTree::get() {
    static ImplImageTree s_ImplImageTree;
    return s_ImplImageTree;
}

ImplImageTree::ImplImageTree()
{
}

ImplImageTree::~ImplImageTree()
{
}

std::vector<OUString> ImplImageTree::getPaths(OUString const & name, LanguageTag& rLanguageTag)
{
    std::vector<OUString> sPaths;

    sal_Int32 pos = name.lastIndexOf('/');
    if (pos != -1)
    {
        for (OUString& rFallback : rLanguageTag.getFallbackStrings(true))
        {
            OUString aFallbackName = getNameNoExtension(getRealImageName(createPath(name, pos, rFallback)));
            sPaths.push_back(aFallbackName + ".png");
            sPaths.push_back(aFallbackName + ".svg");
        }
    }

    OUString aRealName = getNameNoExtension(getRealImageName(name));
    sPaths.push_back(aRealName + ".png");
    sPaths.push_back(aRealName + ".svg");

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
            SAL_INFO("vcl", "exception " << e.Message);
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
            if (doLoadImage(name, aStyle, rBitmap, localized, eFlags))
                return true;
        }
        catch (css::uno::RuntimeException &)
        {}

        aStyle = fallbackStyle(aStyle);
    }
    return false;
}

bool ImplImageTree::loadDefaultImage(OUString const & style, BitmapEx& bitmap, const ImageLoadFlags eFlags)
{
    return doLoadImage("res/grafikde.png", style, bitmap, false, eFlags);
}

OUString createVariant(const ImageLoadFlags eFlags)
{
    static bool bIconsForDarkTheme = !!getenv("VCL_ICONS_FOR_DARK_THEME");

    bool bConvertToDarkTheme = bIconsForDarkTheme;
    if (eFlags & ImageLoadFlags::IgnoreDarkTheme)
        bConvertToDarkTheme = false;

    sal_Int32 aScalePercentage = Application::GetDefaultDevice()->GetDPIScalePercentage();
    if (eFlags & ImageLoadFlags::IgnoreScalingFactor)
        aScalePercentage = 100;

    OUString aVariant;
    if (aScalePercentage == 100 && !bConvertToDarkTheme)
        return aVariant;

    aVariant = OUString::number(aScalePercentage);

    if (bConvertToDarkTheme)
        aVariant += "-dark";

    return aVariant;
}

bool loadDiskCachedVersion(OUString const & sStyle, OUString const & sVariant, OUString const & sName, BitmapEx & rBitmapEx)
{
    OUString sUrl(getIconCacheUrl(sStyle, sVariant, sName));
    if (!urlExists(sUrl))
        return false;
    SvFileStream aFileStream(sUrl, StreamMode::READ);
    vcl::PNGReader aPNGReader(aFileStream);
    aPNGReader.SetIgnoreGammaChunk( true );
    rBitmapEx = aPNGReader.Read();
    return true;
}

void cacheBitmapToDisk(OUString const & sStyle, OUString const & sVariant, OUString const & sName, BitmapEx & rBitmapEx)
{
    OUString sUrl(createIconCacheUrl(sStyle, sVariant, sName));
    vcl::PNGWriter aWriter(rBitmapEx);
    try
    {
        SvFileStream aStream(sUrl, StreamMode::WRITE);
        aWriter.Write(aStream);
        aStream.Close();
    }
    catch (...)
    {}
}

bool ImplImageTree::doLoadImage(OUString const & name, OUString const & style, BitmapEx & bitmap, bool localized, const ImageLoadFlags eFlags)
{
    setStyle(style);

    if (iconCacheLookup(name, localized, eFlags, bitmap))
        return true;

    if (!bitmap.IsEmpty())
        bitmap.SetEmpty();

    LanguageTag aLanguageTag = Application::GetSettings().GetUILanguageTag();

    std::vector<OUString> paths = getPaths(name, aLanguageTag);

    bool found = false;
    try {
        found = findImage(paths, bitmap, eFlags);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        SAL_INFO("vcl", "ImplImageTree::doLoadImage exception " << e.Message);
    }

    if (found)
    {
        OUString aVariant = createVariant(eFlags);
        if (!aVariant.isEmpty())
            cacheBitmapToDisk(style, aVariant, name, bitmap);
        getCurrentIconSet().maIconCache[name] = std::make_pair(localized, bitmap);
    }

    return found;
}

void ImplImageTree::shutDown()
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
        OSL_ASSERT(ok); (void) ok;
        sThemeUrl = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE) + ".zip";

    }
    else
        sThemeUrl += "images";

    if (!urlExists(sThemeUrl))
        return;

    maIconSets[maCurrentStyle] = IconSet(sThemeUrl);

    loadImageLinks();
}

bool ImplImageTree::iconCacheLookup(OUString const & name, bool localized, const ImageLoadFlags eFlags, BitmapEx & bitmap)
{
    IconCache& rIconCache = getCurrentIconSet().maIconCache;

    IconCache::iterator i(rIconCache.find(getRealImageName(name)));
    if (i != rIconCache.end() && i->second.first == localized)
    {
        bitmap = i->second.second;
        return true;
    }

    OUString aVariant = createVariant(eFlags);
    if (!aVariant.isEmpty() && loadDiskCachedVersion(maCurrentStyle, aVariant, name, bitmap))
        return true;

    return false;
}

bool ImplImageTree::findImage(std::vector<OUString> const & paths, BitmapEx & bitmap, const ImageLoadFlags eFlags)
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

            loadImageFromStream(wrapStream(aStream), rPath, bitmap, eFlags);
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
        SAL_INFO("vcl", "ImplImageTree::zip file location exception " << e.Message << " for " << rIconSet.maURL);
        return false;
    }
    return rNameAccess.is();
}

css::uno::Reference<css::container::XNameAccess> ImplImageTree::getNameAccess()
{
    checkPathAccess();
    return getCurrentIconSet().maNameAccess;
}

/// Recursively dump all names ...
css::uno::Sequence<OUString> ImageTree_getAllImageNames()
{
    css::uno::Reference<css::container::XNameAccess> xRef(ImplImageTree::get().getNameAccess());

    return xRef->getElementNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
