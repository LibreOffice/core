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

#ifndef INCLUDED_INCLUDE_VCL_IMPLIMAGETREE_HXX
#define INCLUDED_INCLUDE_VCL_IMPLIMAGETREE_HXX

#include <sal/config.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/dllapi.h>

namespace com { namespace sun { namespace star { namespace container {
    class XNameAccess;
} } } }

class ImplImageTree {
public:
    VCL_DLLPUBLIC static ImplImageTree & get();

    ImplImageTree();

    ~ImplImageTree();

    VCL_DLLPUBLIC OUString getImageUrl(
        OUString const & name, OUString const & style, OUString const & lang);

    bool loadImage(
        OUString const & name, OUString const & style,
        BitmapEx & bitmap, bool localized = false, bool loadMissing = false );

    bool loadDefaultImage(
        OUString const & style,
        BitmapEx& bitmap);

    /** a crude form of life cycle control (called from DeInitVCL; otherwise,
     *  if the ImplImageTree singleton were destroyed during exit that would
     *  be too late for the destructors of the bitmaps in maIconCache)*/
    void shutDown();

    css::uno::Reference< css::container::XNameAccess > getNameAccess();

private:
    ImplImageTree(const ImplImageTree&) = delete;
    ImplImageTree& operator=(const ImplImageTree&) = delete;

    typedef std::unordered_map<OUString, std::pair<bool, BitmapEx>, OUStringHash> IconCache;
    typedef std::unordered_map<OUString, OUString, OUStringHash> IconLinkHash;

    struct IconSet {
        OUString maURL;
        css::uno::Reference<css::container::XNameAccess> maNameAccess;
        IconCache maIconCache;
        IconLinkHash maLinkHash;

        IconSet() {}
        IconSet(const OUString &aURL) : maURL(aURL) {}
    };

    /// Map between the theme name(s) and the content.
    typedef std::unordered_map<OUString, IconSet, OUStringHash> StyleIconSet;

    /// Remember all the (used) icon styles and individual icons in them.
    StyleIconSet maIconSet;

    /// Style used for the current operations; switches switch several times during fallback search.
    OUString maCurrentStyle;

    bool doLoadImage(
        OUString const & name, OUString const & style,
        BitmapEx & bitmap, bool localized);

    bool checkPathAccess();

    void setStyle(OUString const & style );

    void createStyle();

    bool iconCacheLookup( OUString const & name, bool localized, BitmapEx & bitmap );

    bool findImage(std::vector< OUString > const & paths, BitmapEx & bitmap );

    void loadImageLinks();

    void parseLinkFile(std::shared_ptr<SvStream> stream);

    /// Return name of a real .png according to links.txt.
    OUString const & getRealImageName(OUString const & name);

    /** Return name of the fallback style for the provided one.

        Must not be cyclic :-)  The last theme in the chain returns an empty string.
    */
    static OUString fallbackStyle(const OUString &style);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
