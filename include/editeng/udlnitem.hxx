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

#include <editeng/editengdllapi.h>
#include <svl/eitem.hxx>
#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <docmodel/color/ComplexColor.hxx>

// class SvxTextLineItem ------------------------------------------------

/* Value container for underline and overline font effects */

class EDITENG_DLLPUBLIC SvxTextLineItem : public SfxEnumItem<FontLineStyle>
{
    Color maColor;
    model::ComplexColor maComplexColor;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxTextLineItem)
    SvxTextLineItem( const FontLineStyle eSt,
                     const sal_uInt16 nId);

    // "pure virtual Methods" from SfxPoolItem
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SvxTextLineItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual OUString        GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16      GetValueCount() const override;

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    using SfxEnumItem::SetValue;
    virtual bool            HasBoolValue() const override;
    virtual bool            GetBoolValue() const override;
    virtual void            SetBoolValue( bool bVal ) override;

    virtual bool            operator==( const SfxPoolItem& ) const override;

    // enum cast
    FontLineStyle           GetLineStyle() const
                                { return GetValue(); }
    void                    SetLineStyle( FontLineStyle eNew )
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        SetValue(eNew);
    }

    const Color& GetColor() const { return maColor; }
    void SetColor(const Color& rColor) { maColor = rColor; }

    const model::ComplexColor& getComplexColor() const
    {
        return maComplexColor;
    }

    void setComplexColor(model::ComplexColor const& rComplexColor)
    {
        ASSERT_CHANGE_REFCOUNTED_ITEM;
        maComplexColor = rComplexColor;
    }
};

// class SvxUnderlineItem ------------------------------------------------

/* Value container for underline font effects */

class EDITENG_DLLPUBLIC SvxUnderlineItem final : public SvxTextLineItem
{
protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;

public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxUnderlineItem)
    SvxUnderlineItem( const FontLineStyle eSt,
                      const sal_uInt16 nId );

    virtual SvxUnderlineItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const override;
};

// class SvxOverlineItem ------------------------------------------------

/* Value container for overline font effects */

class EDITENG_DLLPUBLIC SvxOverlineItem final : public SvxTextLineItem
{
protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;

public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxOverlineItem)
    SvxOverlineItem( const FontLineStyle eSt,
                     const sal_uInt16 nId );

    virtual SvxOverlineItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
