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

#pragma once

#include <sal/config.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/ImageTree.hxx>

namespace com::sun::star::container {
    class XNameAccess;
}

namespace com::sun::star::io {
    class XInputStream;
}

struct ImageRequestParameters
{
    OUString msName;
    OUString msStyle;
    BitmapEx& mrBitmap;
    bool mbLocalized;
    ImageLoadFlags meFlags;
    bool mbWriteImageToCache;
    sal_Int32 mnScalePercentage;

    ImageRequestParameters(OUString aName, OUString aStyle, BitmapEx& rBitmap, bool bLocalized,
                           ImageLoadFlags eFlags, sal_Int32 nScalePercentage)
        : msName(std::move(aName))
        , msStyle(std::move(aStyle))
        , mrBitmap(rBitmap)
        , mbLocalized(bLocalized)
        , meFlags(eFlags)
        , mbWriteImageToCache(false)
        , mnScalePercentage(nScalePercentage)
    {}

    bool convertToDarkTheme();
    sal_Int32 scalePercentage();
};

class ImplImageTree
{
public:
    ImplImageTree();
    ~ImplImageTree();

    OUString getImageUrl(
        OUString const & name, OUString const & style, OUString const & lang);

    css::uno::Reference<css::io::XInputStream> getImageXInputStream(OUString const & rName,
        OUString const & rStyle, OUString const & rLang);

    std::shared_ptr<SvMemoryStream> getImageStream(
        OUString const & rName, OUString const & rStyle, OUString const & rLang);

    bool loadImage(
        OUString const & name, OUString const & style,
        BitmapEx & bitmap, bool localized,
        const ImageLoadFlags eFlags,
        sal_Int32 nScalePercentage = -1);

    /** a crude form of life cycle control (called from DeInitVCL; otherwise,
     *  if the ImplImageTree singleton were destroyed during exit that would
     *  be too late for the destructors of the bitmaps in maIconCache)*/
    void shutdown();

    css::uno::Reference< css::container::XNameAccess > const & getNameAccess();

private:
    ImplImageTree(const ImplImageTree&) = delete;
    ImplImageTree& operator=(const ImplImageTree&) = delete;

    typedef std::unordered_map<OUString, std::pair<bool,BitmapEx>> IconCache;
    typedef std::unordered_map<sal_Int32, IconCache> ScaledIconCache;
    typedef std::unordered_map<OUString, OUString> IconLinkHash;

    struct IconSet
    {
        OUString maURL;
        css::uno::Reference<css::container::XNameAccess> maNameAccess;
        ScaledIconCache maScaledIconCaches;
        IconLinkHash maLinkHash;

        IconSet()
        {
            maLinkHash.reserve(50);
        }

        IconSet(OUString aURL)
            : maURL(std::move(aURL))
        {
            maLinkHash.reserve(50);
        }
    };

    /// Remember all the (used) icon styles and individual icons in them.
    /// Map between the theme name(s) and the content.
    std::unordered_map<OUString, IconSet> maIconSets;

    /// Style used for the current operations; switches switch several times during fallback search.
    OUString maCurrentStyle;

    IconSet& getCurrentIconSet()
    {
        return maIconSets[maCurrentStyle];
    }

    bool doLoadImage(ImageRequestParameters& rParameters);

    std::vector<OUString> getPaths(OUString const & name, LanguageTag const & rLanguageTag);

    bool checkPathAccess();

    void setStyle(OUString const & rStyle);

    void createStyle();

    IconCache &getIconCache(const ImageRequestParameters& rParameters);

    bool iconCacheLookup(ImageRequestParameters& rParameters);

    bool findImage(std::vector<OUString> const & rPaths, ImageRequestParameters& rParameters);

    void loadImageLinks();

    void parseLinkFile(std::shared_ptr<SvStream> const & aStream);

    /// Return name of a real .png according to links.txt.
    OUString const & getRealImageName(OUString const & rName);


    /** Return name of the fallback style for the provided one.

        Must not be cyclic :-)  The last theme in the chain returns an empty string.
    */
    static OUString fallbackStyle(std::u16string_view rStyle);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
