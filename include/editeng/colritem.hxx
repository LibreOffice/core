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

public:
    static SfxPoolItem* CreateDefault();

    explicit SvxColorItem(const sal_uInt16 nId);
    SvxColorItem(const Color& aColor, const sal_uInt16 nId);
    SvxColorItem(SvStream& rStream, const sal_uInt16 nId);
    virtual ~SvxColorItem() override;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool operator==(const SfxPoolItem& rPoolItem) const override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
    virtual sal_uInt16 GetVersion(sal_uInt16 nFileVersion) const override;

    virtual bool GetPresentation(SfxItemPresentation ePres,
                                 MapUnit eCoreMetric, MapUnit ePresMetric,
                                 OUString &rText, const IntlWrapper& rIntlWrapper) const override;

    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
    virtual SfxPoolItem* Create(SvStream& rStream, sal_uInt16 nVersion) const override;
    virtual SvStream& Store(SvStream& rStream, sal_uInt16 nVersion) const override;

    SvxColorItem& operator=(const SvxColorItem& rColor)
    {
        SetValue(rColor.GetValue());
        return *this;
    }
    /*TODO: SfxPoolItem copy function dichotomy*/SvxColorItem(SvxColorItem const &) = default;

    const Color& GetValue() const
    {
        return mColor;
    }
    void SetValue(const Color& rNewColor);
};

// XXX: to be moved in a separate header.
class EDITENG_DLLPUBLIC SvxBackgroundColorItem : public SvxColorItem
{
    public:
        static SfxPoolItem* CreateDefault();

        SvxBackgroundColorItem(const sal_uInt16 nId);
        SvxBackgroundColorItem(const Color& rCol, const sal_uInt16 nId);

        virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;
        virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
        virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
