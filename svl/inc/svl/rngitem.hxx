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

#ifndef _SFXRNGITEM_HXX

#ifndef NUMTYPE

#define NUMTYPE sal_uInt16
#define SfxXRangeItem SfxRangeItem
#define SfxXRangesItem SfxUShortRangesItem
#include <svl/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem

#ifndef _SFXITEMS_HXX
#define NUMTYPE sal_uLong
#define SfxXRangeItem SfxULongRangeItem
#define SfxXRangesItem SfxULongRangesItem
#include <svl/rngitem.hxx>
#undef NUMTYPE
#undef SfxXRangeItem
#undef SfxXRangesItem
#endif

#define _SFXRNGITEM_HXX

#else
#include "svl/svldllapi.h"
#include <svl/poolitem.hxx>

class SvStream;

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxXRangeItem : public SfxPoolItem
{
private:
    NUMTYPE                     nFrom;
    NUMTYPE                     nTo;
public:
                                TYPEINFO();
                                SfxXRangeItem();
                                SfxXRangeItem( sal_uInt16 nWID, NUMTYPE nFrom, NUMTYPE nTo );
                                SfxXRangeItem( sal_uInt16 nWID, SvStream &rStream );
                                SfxXRangeItem( const SfxXRangeItem& rItem );
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    inline NUMTYPE&             From() { return nFrom; }
    inline NUMTYPE              From() const { return nFrom; }
    inline NUMTYPE&             To() { return nTo; }
    inline NUMTYPE              To() const { return nTo; }
    inline sal_Bool                 HasRange() const { return nTo>nFrom; }
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const;
};

// -----------------------------------------------------------------------

class SVL_DLLPUBLIC SfxXRangesItem : public SfxPoolItem
{
private:
    NUMTYPE*                    _pRanges;

public:
                                TYPEINFO();
                                SfxXRangesItem();
                                SfxXRangesItem( sal_uInt16 nWID, const NUMTYPE *pRanges );
                                SfxXRangesItem( sal_uInt16 nWID, SvStream &rStream );
                                SfxXRangesItem( const SfxXRangesItem& rItem );
    virtual                     ~SfxXRangesItem();
    virtual int                 operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*        Clone( SfxItemPool *pPool = 0 ) const;
    inline const NUMTYPE*       GetRanges() const { return _pRanges; }
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVersion ) const;
    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const;
};

#endif
#endif
