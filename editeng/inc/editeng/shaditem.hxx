/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_SHADITEM_HXX
#define _SVX_SHADITEM_HXX

#include <tools/color.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/editengdllapi.h>

namespace rtl
{
    class OUString;
}

// class SvxShadowItem ---------------------------------------------------

/*  [Description]

    This item describes the shadow attribute (color, width and position).
*/

#define SHADOW_TOP      ((sal_uInt16)0)
#define SHADOW_BOTTOM   ((sal_uInt16)1)
#define SHADOW_LEFT     ((sal_uInt16)2)
#define SHADOW_RIGHT    ((sal_uInt16)3)

class EDITENG_DLLPUBLIC SvxShadowItem : public SfxEnumItemInterface
{
    Color               aShadowColor;
    sal_uInt16              nWidth;
    SvxShadowLocation   eLocation;
public:
    TYPEINFO();

    SvxShadowItem( const sal_uInt16 nId ,
                 const Color *pColor = 0, const sal_uInt16 nWidth = 100 /*5pt*/,
                 const SvxShadowLocation eLoc = SVX_SHADOW_NONE );

    inline SvxShadowItem& operator=( const SvxShadowItem& rFmtShadow );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual bool             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    const Color& GetColor() const { return aShadowColor;}
    void SetColor( const Color &rNew ) { aShadowColor = rNew; }

    sal_uInt16 GetWidth() const { return nWidth; }
    SvxShadowLocation GetLocation() const { return eLocation; }

    void SetWidth( sal_uInt16 nNew ) { nWidth = nNew; }
    void SetLocation( SvxShadowLocation eNew ) { eLocation = eNew; }

    // Calculate width of the shadow on the page.
    sal_uInt16 CalcShadowSpace( sal_uInt16 nShadow ) const;

    virtual sal_uInt16      GetValueCount() const;
    virtual rtl::OUString   GetValueTextByPos( sal_uInt16 nPos ) const;
    virtual sal_uInt16      GetEnumValue() const;
    virtual void            SetEnumValue( sal_uInt16 nNewVal );
};

inline SvxShadowItem &SvxShadowItem::operator=( const SvxShadowItem& rFmtShadow )
{
    aShadowColor = rFmtShadow.aShadowColor;
    nWidth = rFmtShadow.GetWidth();
    eLocation = rFmtShadow.GetLocation();
    return *this;
}

#endif // #ifndef _SVX_SHADITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
