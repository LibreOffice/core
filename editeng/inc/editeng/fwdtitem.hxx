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
#ifndef _SVX_FWDTITEM_HXX
#define _SVX_FWDTITEM_HXX

#include <svl/poolitem.hxx>


// class SvxFontWidthItem -----------------------------------------------


/*  [Description]

    This item describes the font width.
*/

class SvxFontWidthItem : public SfxPoolItem
{
    sal_uInt16  nWidth;         // 0 = default
    sal_uInt16  nProp;          // default 100%
public:
    TYPEINFO();

    SvxFontWidthItem(   const sal_uInt16 nSz /*= 0*/,
                        const sal_uInt16 nPropWidth /*= 100*/,
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
    virtual bool             ScaleMetrics( long nMult, long nDiv );
    virtual bool             HasMetrics() const;

    inline SvxFontWidthItem& operator=(const SvxFontWidthItem& rItem )
        {
            SetWidthValue( rItem.GetWidth() );
            SetProp( rItem.GetProp() );
            return *this;
        }

    void SetWidth( sal_uInt16 nNewWidth, const sal_uInt16 nNewProp = 100 )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = sal_uInt16(( (sal_uInt32)nNewWidth * nNewProp ) / 100 );
            nProp = nNewProp;
        }

    sal_uInt16 GetWidth() const { return nWidth; }

    void SetWidthValue( sal_uInt16 nNewWidth )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nWidth = nNewWidth;
        }

    void SetProp( const sal_uInt16 nNewProp )
        {
            DBG_ASSERT( GetRefCount() == 0, "SetValue() with pooled item" );
            nProp = nNewProp;
        }

    sal_uInt16 GetProp() const { return nProp; }
};





#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
