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

#include <svl/poolitem.hxx>
#include <tools/color.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;

#define VERSION_USEAUTOCOLOR    1

/** SvxColorItem item describes a color.
*/
class EDITENG_DLLPUBLIC SvxColorItem : public SfxPoolItem
{
private:
    Color mColor;
    sal_Int16 maThemeIndex;
    sal_Int16 maTintShade;

public:
    TYPEINFO_OVERRIDE();

    explicit SvxColorItem(const sal_uInt16 nId);
    SvxColorItem(const Color& aColor, const sal_uInt16 nId);
    SvxColorItem(SvStream& rStream, const sal_uInt16 nId);
    SvxColorItem(const SvxColorItem& rCopy);
    virtual ~SvxColorItem();

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rPoolItem) const SAL_OVERRIDE;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const SAL_OVERRIDE;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) SAL_OVERRIDE;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileVersion) const SAL_OVERRIDE;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper* pIntlWrapper = 0) const SAL_OVERRIDE;

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const SAL_OVERRIDE;
    virtual SfxPoolItem* Create(SvStream& rStream, sal_uInt16 nVersion) const SAL_OVERRIDE;
    virtual SvStream& Store(SvStream& rStream, sal_uInt16 nVersion) const SAL_OVERRIDE;

    inline SvxColorItem& operator=(const SvxColorItem& rColor)
    {
        SetValue(rColor.GetValue());
        return *this;
    }

    const Color& GetValue() const
    {
        return mColor;
    }
    void SetValue(const Color& rNewColor);

    sal_Int16 GetThemeIndex() const
    {
        return maThemeIndex;
    }
    void SetThemeIndex(sal_Int16 nIndex)
    {
        maThemeIndex = nIndex;
    }

    sal_Int16 GetTintOrShade() const
    {
        return maTintShade;
    }
    void SetTintOrShade(sal_Int16 nTintOrShade)
    {
        maTintShade = nTintOrShade;
    }
};

// XXX: to be moved in a separate header.
class EDITENG_DLLPUBLIC SvxBackgroundColorItem : public SvxColorItem
{
    public:
        TYPEINFO_OVERRIDE();

        SvxBackgroundColorItem(const sal_uInt16 nId);
        SvxBackgroundColorItem(const Color& rCol, const sal_uInt16 nId);
        SvxBackgroundColorItem(SvStream& rStrm, const sal_uInt16 nId);
        SvxBackgroundColorItem(const SvxBackgroundColorItem& rCopy);

        virtual SfxPoolItem* Clone(SfxItemPool* pPool = 0) const SAL_OVERRIDE;
        virtual SvStream& Store(SvStream& rStream, sal_uInt16 nVersion) const SAL_OVERRIDE;
        virtual SfxPoolItem* Create(SvStream &, sal_uInt16) const SAL_OVERRIDE;
        virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const SAL_OVERRIDE;
        virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
