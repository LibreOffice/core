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

#include <svx/numvset.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;
using namespace com::sun::star::container;
using namespace com::sun::star::style;

class SVX_DLLPUBLIC SvxBmpNumIconView
{
public:
    static vcl::Font& GetDefaultBulletFont();

    static void PopulateIconView(
        weld::IconView* pIconView,
        NumberingPageType ePageType,
        Size previewSize,
        const Sequence<Sequence<PropertyValue>>& rNumSettings = Sequence<Sequence<PropertyValue>>(),
        const Sequence<Reference<XIndexAccess>>& rOutlineSettings = Sequence<Reference<XIndexAccess>>(),
        Reference<XNumberingFormatter> const& xFormatter = nullptr,
        const Locale& rLocale = Locale());

    static VclPtr<VirtualDevice> CreatePreviewFromUserDraw(
        NumberingPageType ePageType,
        sal_Int32 nIndex,
        Size previewSize,
        const Sequence<Sequence<PropertyValue>>& rNumSettings = Sequence<Sequence<PropertyValue>>(),
        const Sequence<Reference<XIndexAccess>>& rOutlineSettings = Sequence<Reference<XIndexAccess>>(),
        Reference<XNumberingFormatter> const& xFormatter = nullptr,
        const Locale& rLocale = Locale(),
        const std::vector<std::pair<OUString, OUString>>& rCustomBullets = std::vector<std::pair<OUString, OUString>>());

    static VclPtr<VirtualDevice> CreateCustomBulletPreview(const OUString& rBulletChar, const OUString& rFontName);
    static OUString GetNumberingDescription(NumberingPageType ePageType, sal_Int32 nIndex);

    static void SetNumberingSettings(
        weld::IconView* mxIconView,
        Size previewSize,
        const Sequence<Sequence<PropertyValue>>& aNum,
        Reference<XNumberingFormatter> const& xFormat,
        const Locale& rLocale,
        std::vector<std::pair<OUString, OUString>> maCustomBullets = std::vector<std::pair<OUString, OUString>>());

    static void SetOutlineNumberingSettings(
        weld::IconView* mxIconView,
        Size previewSize,
        const Sequence<Reference<XIndexAccess>>& rOutline,
        Reference<XNumberingFormatter> const& xFormat,
        const Locale& rLocale,
        std::vector<std::pair<OUString, OUString>> maCustomBullets = std::vector<std::pair<OUString, OUString>>());

    static VclPtr<VirtualDevice> CreateBitmapBulletPreview(sal_uInt32 nGalleryIndex);
    static void PopulateBitmapIconView(weld::IconView* pIconView);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
