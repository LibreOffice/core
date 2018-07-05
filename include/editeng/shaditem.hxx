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
#ifndef INCLUDED_EDITENG_SHADITEM_HXX
#define INCLUDED_EDITENG_SHADITEM_HXX

#include <tools/color.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>


// class SvxShadowItem ---------------------------------------------------

/*  [Description]

    This item describes the shadow attribute (color, width and position).
*/
enum class SvxShadowItemSide { TOP, BOTTOM, LEFT, RIGHT };

class EDITENG_DLLPUBLIC SvxShadowItem : public SfxEnumItemInterface
{
    Color               aShadowColor;
    sal_uInt16              nWidth;
    SvxShadowLocation   eLocation;
public:
    static SfxPoolItem* CreateDefault();

    explicit SvxShadowItem( const sal_uInt16 nId ,
                 const Color *pColor = nullptr, const sal_uInt16 nWidth = 100 /*5pt*/,
                 const SvxShadowLocation eLoc = SvxShadowLocation::NONE );

    inline SvxShadowItem& operator=( const SvxShadowItem& rFmtShadow );
    /*TODO: SfxPoolItem copy function dichotomy*/SvxShadowItem(SvxShadowItem const &) = default;

    // "pure virtual Methods" from SfxPoolItem
    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual void             ScaleMetrics( long nMult, long nDiv ) override;
    virtual bool             HasMetrics() const override;

    const Color& GetColor() const { return aShadowColor;}
    void SetColor( const Color &rNew ) { aShadowColor = rNew; }

    sal_uInt16 GetWidth() const { return nWidth; }
    SvxShadowLocation GetLocation() const { return eLocation; }

    void SetWidth( sal_uInt16 nNew ) { nWidth = nNew; }
    void SetLocation( SvxShadowLocation eNew ) { eLocation = eNew; }

    // Calculate width of the shadow on the page.
    sal_uInt16 CalcShadowSpace( SvxShadowItemSide nShadow ) const;

    virtual sal_uInt16      GetValueCount() const override;
    virtual sal_uInt16      GetEnumValue() const override;
    virtual void            SetEnumValue( sal_uInt16 nNewVal ) override;
    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

inline SvxShadowItem &SvxShadowItem::operator=( const SvxShadowItem& rFmtShadow )
{
    aShadowColor = rFmtShadow.aShadowColor;
    nWidth = rFmtShadow.GetWidth();
    eLocation = rFmtShadow.GetLocation();
    return *this;
}

#endif // INCLUDED_EDITENG_SHADITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
