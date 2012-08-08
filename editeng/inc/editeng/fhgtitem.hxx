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
#ifndef _SVX_FHGTITEM_HXX
#define _SVX_FHGTITEM_HXX

#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFontHeightItem -----------------------------------------------

// Warning: twips values

/*  [Description]

    This item describes the font height
*/

#define FONTHEIGHT_16_VERSION   ((sal_uInt16)0x0001)
#define FONTHEIGHT_UNIT_VERSION ((sal_uInt16)0x0002)

class EDITENG_DLLPUBLIC SvxFontHeightItem : public SfxPoolItem
{
    sal_uInt32  nHeight;
    sal_uInt16  nProp;              // default 100%
    SfxMapUnit ePropUnit;       // Percent, Twip, ...
public:
    TYPEINFO();

    SvxFontHeightItem( const sal_uLong nSz /*= 240*/, const sal_uInt16 nPropHeight /*= 100*/,
                       const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual sal_uInt16           GetVersion( sal_uInt16 nItemVersion) const;
    virtual bool                 ScaleMetrics( long nMult, long nDiv );
    virtual bool                 HasMetrics() const;

    inline SvxFontHeightItem& operator=(const SvxFontHeightItem& rSize)
        {
            SetHeightValue( rSize.GetHeight() );
            SetProp( rSize.GetProp(), ePropUnit );
            return *this;
        }

    void SetHeight( sal_uInt32 nNewHeight, const sal_uInt16 nNewProp = 100,
                     SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE );

    void SetHeight( sal_uInt32 nNewHeight, sal_uInt16 nNewProp,
                     SfxMapUnit eUnit, SfxMapUnit eCoreUnit );

    sal_uInt32 GetHeight() const { return nHeight; }

    void SetHeightValue( sal_uInt32 nNewHeight )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nHeight = nNewHeight;
        }

    void SetProp( const sal_uInt16 nNewProp,
                    SfxMapUnit eUnit = SFX_MAPUNIT_RELATIVE )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
            ePropUnit = eUnit;
        }

    sal_uInt16 GetProp() const { return nProp; }

    SfxMapUnit GetPropUnit() const { return ePropUnit;  }   // Percent, Twip, ...
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
