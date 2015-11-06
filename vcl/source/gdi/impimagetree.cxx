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
#include "com/sun/star/uno/Any.hxx"
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
#include "vcl/bitmapex.hxx"
#include <vcl/dibtools.hxx>
#include <vcl/implimagetree.hxx>
#include "vcl/pngread.hxx"
#include "vcl/settings.hxx"
#include "vcl/svapp.hxx"
#include <vcldemo-debug.hxx>

#include <vcl/BitmapProcessor.hxx>

using namespace css;

namespace {

static OUString createPath(OUString const & name, sal_Int32 pos, OUString const & locale)
{
    return name.copy(0, pos + 1) + locale + name.copy(pos);
}

static std::shared_ptr<SvStream> wrapStream(css::uno::Reference< css::io::XInputStream > const & stream)
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
        s->Write(data.getConstArray(), n);
        if (n < size)
            break;
    }
    s->Seek(0);
    return s;
}

static void loadImageFromStream(std::shared_ptr<SvStream> xStream, OUString const & rPath, BitmapEx & rBitmap)
{
    if (rPath.endsWith(".png"))
    {
        vcl::PNGReader aPNGReader(*xStream);
        aPNGReader.SetIgnoreGammaChunk( true );
        rBitmap = aPNGReader.Read();
    }
    else
    {
        ReadDIBBitmapEx(rBitmap, *xStream);
    }
}

}

ImplImageTree & ImplImageTree::get() {
    static ImplImageTree instance;
    return instance;
}

ImplImageTree::ImplImageTree()
{
}

ImplImageTree::~ImplImageTree()
{
}

OUString ImplImageTree::getImageUrl(
    OUString const & name, OUString const & style, OUString const & lang)
{
    OUString aStyle(style);
    while (!aStyle.isEmpty())
    {
        try {
            setStyle(aStyle);

            std::vector< OUString > paths;
            paths.push_back(getRealImageName(name));

            if (!lang.isEmpty())
            {
                sal_Int32 pos = name.lastIndexOf('/');
                if (pos != -1)
                {
                    std::vector<OUString> aFallbacks(
                        LanguageTag(lang).getFallbackStrings(true));
                    for (std::vector< OUString >::reverse_iterator it( aFallbacks.rbegin());
                         it != aFallbacks.rend(); ++it)
                    {
                        paths.push_back( getRealImageName( createPath(name, pos, *it) ) );
                    }
                }
            }

            try {
                if (checkPathAccess()) {
                    const uno::Reference<container::XNameAccess> &rNameAccess = maIconSet[maCurrentStyle].maNameAccess;

                    for (std::vector<OUString>::const_reverse_iterator j(paths.rbegin()); j != paths.rend(); ++j)
                    {
                        if (rNameAccess->hasByName(*j))
                        {
                            return "vnd.sun.star.zip://"
                                + rtl::Uri::encode(
                                    maIconSet[maCurrentStyle].maURL + ".zip",
                                    rtl_UriCharClassRegName,
                                    rtl_UriEncodeIgnoreEscapes,
                                    RTL_TEXTENCODING_UTF8)
                                + "/" + *j;
                                // assuming *j contains no problematic chars
                        }
                    }
                }
            } catch (css::uno::RuntimeException &) {
                throw;
            } catch (const css::uno::Exception & e) {
                SAL_INFO("vcl", "exception " << e.Message);
            }
        }
        catch (css::uno::RuntimeException &) {}

        aStyle = fallbackStyle(aStyle);
    }
    return OUString();
}

OUString ImplImageTree::fallbackStyle(const OUString &style)
{
    if (style == "galaxy")
        return OUString();
    else if (style == "industrial")
        return OUString("galaxy");
    else if (style == "tango")
        return OUString("galaxy");
    else if (style == "breeze")
        return OUString("galaxy");
    else if (style == "sifr")
        return OUString("breeze");

    return OUString("tango");
}

bool ImplImageTree::loadImage(OUString const & name, OUString const & style, BitmapEx & bitmap,
    bool localized, bool loadMissing)
{
    OUString aStyle(style);
    while (!aStyle.isEmpty())
    {
        try {
            if (doLoadImage(name, aStyle, bitmap, localized))
            {
                static bool bIconsForDarkTheme = !!getenv("VCL_ICONS_FOR_DARK_THEME");
                if (bIconsForDarkTheme)
                    bitmap = BitmapProcessor::createLightImage(bitmap);
                return true;
            }
        }
        catch (css::uno::RuntimeException &) {}

        aStyle = fallbackStyle(aStyle);
    }

    if (!loadMissing)
        return false;

    SAL_INFO("vcl", "ImplImageTree::loadImage couldn't load \"" << name << "\", fetching default image");

    return loadDefaultImage(style, bitmap);
}

bool ImplImageTree::loadDefaultImage(OUString const & style, BitmapEx& bitmap)
{
    return doLoadImage(
        "res/grafikde.png",
        style, bitmap, false);
}

bool ImplImageTree::doLoadImage(OUString const & name, OUString const & style, BitmapEx & bitmap,
    bool localized)
{
    setStyle(style);
    if (iconCacheLookup(name, localized, bitmap))
        return true;

    if (!bitmap.IsEmpty())
        bitmap.SetEmpty();

    std::vector< OUString > paths;
    paths.push_back(getRealImageName(name));

    if (localized)
    {
        sal_Int32 pos = name.lastIndexOf('/');
        if (pos != -1)
        {
            // findImage() uses a reverse iterator, so push in reverse order.
            std::vector< OUString > aFallbacks( Application::GetSettings().GetUILanguageTag().getFallbackStrings(true));
            for (std::vector< OUString >::reverse_iterator it( aFallbacks.rbegin());
                    it != aFallbacks.rend(); ++it)
            {
                paths.push_back( getRealImageName( createPath(name, pos, *it) ) );
            }
        }
    }

    bool found = false;
    try {
        found = findImage(paths, bitmap);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        SAL_INFO("vcl", "ImplImageTree::doLoadImage exception " << e.Message);
    }

    if (found)
        maIconSet[maCurrentStyle].maIconCache[name] = std::make_pair(localized, bitmap);

    return found;
}

void ImplImageTree::shutDown()
{
    maCurrentStyle.clear();
    for (StyleIconSet::iterator it = maIconSet.begin(); it != maIconSet.end(); ++it)
    {
        it->second.maIconCache.clear();
        it->second.maLinkHash.clear();
    }
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
    if (maIconSet.find(maCurrentStyle) != maIconSet.end())
        return;

    OUString url( "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/config/" );
    rtl::Bootstrap::expandMacros(url);
    if (maCurrentStyle != "default")
    {
        INetURLObject u(url);
        OSL_ASSERT(!u.HasError());
        bool ok = u.Append("images_" + maCurrentStyle, INetURLObject::ENCODE_ALL);
        OSL_ASSERT(ok); (void) ok;
        url = u.GetMainURL(INetURLObject::NO_DECODE);
    }
    else
        url += "images";

    maIconSet[maCurrentStyle] = IconSet(url);

    loadImageLinks();
}

bool ImplImageTree::iconCacheLookup(OUString const & name, bool localized, BitmapEx & bitmap)
{
    IconCache &rIconCache = maIconSet[maCurrentStyle].maIconCache;

    IconCache::iterator i(rIconCache.find(getRealImageName(name)));
    if (i != rIconCache.end() && i->second.first == localized)
    {
        bitmap = i->second.second;
        return true;
    }
    return false;
}

bool ImplImageTree::findImage(std::vector<OUString> const & paths, BitmapEx & bitmap)
{
    if (!checkPathAccess())
        return false;

    const uno::Reference<container::XNameAccess> &rNameAccess = maIconSet[maCurrentStyle].maNameAccess;

    for (std::vector<OUString>::const_reverse_iterator j(paths.rbegin()); j != paths.rend(); ++j)
    {
        if (rNameAccess->hasByName(*j))
        {
            css::uno::Reference< css::io::XInputStream > s;
            bool ok = rNameAccess->getByName(*j) >>= s;
            assert(ok);
            (void)ok; // prevent unused warning in release build

            loadImageFromStream( wrapStream(s), *j, bitmap );
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

    const uno::Reference<container::XNameAccess> &rNameAccess = maIconSet[maCurrentStyle].maNameAccess;

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

void ImplImageTree::parseLinkFile(std::shared_ptr<SvStream> xStream)
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

        maIconSet[maCurrentStyle].maLinkHash[aLink] = aOriginal;
    }
}

OUString const & ImplImageTree::getRealImageName(OUString const & name)
{
    IconLinkHash &rLinkHash = maIconSet[maCurrentStyle].maLinkHash;

    IconLinkHash::iterator it(rLinkHash.find(name));
    if (it == rLinkHash.end())
        return name;

    return it->second;
}

bool ImplImageTree::checkPathAccess()
{
    uno::Reference<container::XNameAccess> &rNameAccess = maIconSet[maCurrentStyle].maNameAccess;
    if (rNameAccess.is())
        return true;

    try {
        rNameAccess = css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), maIconSet[maCurrentStyle].maURL + ".zip");
    }
    catch (const css::uno::RuntimeException &) {
        throw;
    }
    catch (const css::uno::Exception & e) {
        SAL_INFO("vcl", "ImplImageTree::zip file location exception " << e.Message << " for " << maIconSet[maCurrentStyle].maURL);
        return false;
    }
    return rNameAccess.is();
}

css::uno::Reference<css::container::XNameAccess> ImplImageTree::getNameAccess()
{
    checkPathAccess();
    return maIconSet[maCurrentStyle].maNameAccess;
}

/// Recursively dump all names ...
css::uno::Sequence<OUString> ImageTree_getAllImageNames()
{
    css::uno::Reference<css::container::XNameAccess> xRef(ImplImageTree::get().getNameAccess());

    return xRef->getElementNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
