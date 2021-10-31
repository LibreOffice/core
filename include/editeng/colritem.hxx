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
#ifndef INCLUDED_EDITENG_COLRITEM_HXX
#define INCLUDED_EDITENG_COLRITEM_HXX

#include <sfx2/objsh.hxx>
#include <sfx2/ColorSets.hxx>
#include <svl/poolitem.hxx>
#include <tools/color.hxx>
#include <editeng/editengdllapi.h>

#include <sal/log.hxx>

#define VERSION_USEAUTOCOLOR 1


/** ThemeColorData holds theme color related
    data of a color item eg. SvxColorItem
*/
class ThemeColorData
{
public:
    ThemeColorData() = default;
    ThemeColorData(const ColorSets* pColorSets) : mpColorSets(pColorSets){}
    bool operator==(const ThemeColorData& rOther) const;
    /** Calculates and returns the theme color from current ThemeColorData

        @returns std::nullopt if color item that owns the ThemeColorData
                 doesn't need a color update, returns calculated color otherwise.

    */
    std::optional<Color> getThemeColorIfNeedsUpdate() const;
    void setTintOrShade(sal_Int16 nTintShade);
    sal_Int16 getTintOrShade() const;
    void setThemeColorIndex(sal_Int16 nThemeColorIndex);
    sal_Int16 getThemeColorIndex() const;
    void setVirtualThemeColorSet(sal_Int32 nVirtualThemeColorSetIndex);
    sal_Int32 getVirtualThemeColorSetIndex() const;

private:
    void RecalculateOnNextGet();
    /** Specifies which color of the current theme color set
        is the base color for this item.

        If it is -1, this item isn't a theme color.
     */
    sal_Int16 mnThemeColorIndex = -1;
    /** Tint or Shade value that will be applied
        on top of the base color
     */
    sal_Int16 mnTintShade = 0;
    /** Cached base color value used to determine whether
        a recalculation of theme color is required.

        Mutable since the base color is cached on the getThemeColorIfNeedsUpdate
    */
    mutable Color maCachedBaseColor;
    /** Pointer to the ColorSets of the document.

        Mutable since the correct value initialized on getThemeColorIfNeedsUpdate
        if it is indeed a theme color
     */
    mutable const ColorSets* mpColorSets = nullptr;
    // mutable since marked as false after recalculation in getThemeColorIfNeedsUpdate
    mutable bool mbRecalculateColor = true;

    std::weak_ptr<VirtualThemeColorSet> mpVirtualThemeColorSet = std::weak_ptr<VirtualThemeColorSet>();
};

/** SvxColorItem item describes a color.
*/
class EDITENG_DLLPUBLIC SvxColorItem final : public SfxPoolItem
{
private:
    // Marked mutable since the color gets recalculated on GetValue if
    // it is a ThemeColor and a recalculation is needed.
    mutable Color mColor;
    /** ThemeColorData that is used to calculate and update mColor
        if color item is a theme color.
    */
    ThemeColorData maThemeColorData;

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxColorItem(const sal_uInt16 nId);
    SvxColorItem(const Color& aColor, const sal_uInt16 nId);
    SvxColorItem(const Color& aColor, const ColorSets* pColorSets, const sal_uInt16 nId);
    virtual ~SvxColorItem() override;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rPoolItem) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric, MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper& rIntlWrapper) const override;

    virtual SvxColorItem* Clone(SfxItemPool* pPool = nullptr) const override;
    SvxColorItem(SvxColorItem const &) = default; // SfxPoolItem copy function dichotomy

    const Color& GetValue() const;

    void SetValue(const Color& rNewColor);

    // Getters and setters for maThemeColorData
    sal_Int16 GetThemeIndex() const;
    void SetThemeIndex(sal_Int16 nIndex);
    sal_Int16 GetTintOrShade() const;
    void SetTintOrShade(sal_Int16 nTintOrShade);

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
